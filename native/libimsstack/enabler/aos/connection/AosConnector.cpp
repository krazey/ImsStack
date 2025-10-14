/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ServiceTrace.h"
#include "ServiceTimer.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceEvent.h"
#include "ServiceUtil.h"
#include "CarrierConfig.h"
#include "IIpcan.h"
#include "IAosService.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosApplication.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosConnection.h"
#include "interface/IAosConnectorListener.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegistration.h"
#include "connection/AosConnector.h"
#include "provider/AosProvider.h"
#include "provider/AosUtil.h"

__IMS_TRACE_TAG_AOS__;

#define APPPROFILE strTag.GetStr()

PUBLIC
AosConnector::AosConnector(IN IAosAppContext* piAppContext) :
        m_piAppContext(piAppContext),
        m_piConnection(IMS_NULL),
        m_piService(IMS_NULL),
        m_piPcscf(IMS_NULL),
        m_piIpv6Timer(IMS_NULL),
        m_piStopDelayTimer(IMS_NULL),
        m_piReadyRecoveryTimer(IMS_NULL),
        m_piPcoWaitingTimer(IMS_NULL),
        m_piListener(IMS_NULL),
        m_nState(STATE_IDLE),
        m_nPendingFeature(PENDING_NONE),
        m_nReadyRecoveryCount(0),
        m_bPcscfConfigured(IMS_FALSE),
        m_bDataConnected(IMS_FALSE),
        m_bCrossSimConnected(IMS_FALSE),
        m_bEmergencyType(IMS_FALSE),
        m_bIsTerminating(IMS_FALSE),
        m_bIsPcscfChangeIgnored(IMS_FALSE)
{
    strTag.Sprintf("%d:%s", m_piAppContext->GetSlotId(), m_piAppContext->GetProfileId().GetStr());

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosConnector = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosConnector), this);

    m_piConnection = m_piAppContext->GetConnection();
    if (m_piConnection != IMS_NULL)
    {
        m_piConnection->SetListener(this);
        SetEmergencyType(m_piConnection->GetConnectionType() == NetworkPolicy::APN_EMERGENCY);
    }

    m_piPcscf = m_piAppContext->GetPcscf();
    if (m_piPcscf != IMS_NULL)
    {
        m_piPcscf->SetListener(this);
    }

    m_piService = AosProvider::GetInstance()->GetService(m_piAppContext->GetSlotId());
    if (m_piService != IMS_NULL)
    {
        m_piService->AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
    }

    m_pUtil = AosUtil::GetInstance();
}

PUBLIC VIRTUAL AosConnector::~AosConnector()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosConnector = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosConnector), this);
}

PUBLIC VIRTUAL IMS_BOOL AosConnector::Start()
{
    A_IMS_TRACE_I(APPPROFILE, "Start :: ready (%s) , connector is pending (%s)",
            _TRACE_B_(m_nState == STATE_READY), _TRACE_B_(IsPending()), 0);

    StopTimer(TIMER_STOP_DELAY);

    if (m_nState == STATE_READY)
    {
        A_IMS_TRACE_D(APPPROFILE, "Start :: already ready", 0, 0, 0);

        if (m_bIsPcscfChangeIgnored)
        {
            m_piPcscf->CheckAndProcessChangeFromPco();
            m_bIsPcscfChangeIgnored = IMS_FALSE;
        }

        Notify(LISTENER_TYPE_ACTIVATED);
        return IMS_FALSE;
    }

    if (IsPending())
    {
        A_IMS_TRACE_D(APPPROFILE, "Start :: connector is pending", 0, 0, 0);
        return IMS_FALSE;
    }

    m_piConnection->Activate();
    return IMS_TRUE;
}

PUBLIC VIRTUAL void AosConnector::Stop()
{
    m_piConnection->Deactivate();
    CleanAll();
    Notify(LISTENER_TYPE_DEACTIVATED);
}

PUBLIC VIRTUAL void AosConnector::Stop(IN IMS_SINT32 nDelayTimeSec)
{
    A_IMS_TRACE_I(APPPROFILE, "Stop :: delay (%d)", nDelayTimeSec, 0, 0);

    if (nDelayTimeSec <= 0)
    {
        Stop();
        return;
    }

    if (m_piStopDelayTimer != IMS_NULL)
    {
        return;
    }

    StartTimer(TIMER_STOP_DELAY, nDelayTimeSec * 1000);
}

PUBLIC VIRTUAL void AosConnector::SetListener(IN IAosConnectorListener* piListener)
{
    m_piListener = piListener;
}

PUBLIC VIRTUAL IMS_BOOL AosConnector::IsReady() const
{
    if (m_piStopDelayTimer != IMS_NULL)
    {
        return IMS_FALSE;
    }

    return (m_nState == STATE_READY);
}

PUBLIC VIRTUAL void AosConnector::ResetReadyRecovery()
{
    m_nReadyRecoveryCount = 0;
    if (IsTimerRunning(TIMER_READY_RECOVERY))
    {
        ProcessReadyRecoveryTimerExpired();
    }
}

PUBLIC VIRTUAL IMS_BOOL AosConnector::IsCrossSimConnected() const
{
    return m_bCrossSimConnected;
}

PUBLIC VIRTUAL IMS_BOOL AosConnector::ProcessPendingPcscfChange()
{
    if (m_bIsPcscfChangeIgnored)
    {
        m_bIsPcscfChangeIgnored = IMS_FALSE;
        if (m_piPcscf->CheckAndProcessChangeFromPco())
        {
            Notify(LISTENER_TYPE_UPDATED, REASON_PCSCF_CHANGED);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PROTECTED
void AosConnector::ClearPending()
{
    m_nPendingFeature = PENDING_NONE;
}

PROTECTED
void AosConnector::SetState(IN IMS_UINT32 nState)
{
    if (nState == this->m_nState)
    {
        return;
    }

    A_IMS_TRACE_I(APPPROFILE, "SetState :: OLD(%s) -> NEW(%s)",
            (this->m_nState == STATE_READY) ? "STATE_READY" : "STATE_IDLE",
            (nState == STATE_READY) ? "STATE_READY" : "STATE_IDLE", 0);

    this->m_nState = nState;

    if (nState == STATE_READY)
    {
        m_nReadyRecoveryCount = 0;

        if (IsTimerRunning(TIMER_READY_RECOVERY))
        {
            StopTimer(TIMER_READY_RECOVERY);
        }
    }
}

PROTECTED
void AosConnector::SetDataConnected(IN IMS_BOOL bConnected)
{
    m_bDataConnected = bConnected;
}

PROTECTED
void AosConnector::SetEmergencyType(IN IMS_BOOL bEmergency)
{
    m_bEmergencyType = bEmergency;
}

PROTECTED
void AosConnector::SetPcscfConfigured(IN IMS_BOOL bConfigured)
{
    m_bPcscfConfigured = bConfigured;
}

PROTECTED
IMS_BOOL AosConnector::IsDataConnected() const
{
    return m_bDataConnected;
}

PROTECTED
IMS_BOOL AosConnector::IsEmergencyType() const
{
    return m_bEmergencyType;
}

PROTECTED
IMS_BOOL AosConnector::IsPcscfChangeAvailable() const
{
    IMS_UINT32 nAppState = m_piAppContext->GetApp()->GetAppState();

    switch (nAppState)
    {
        case IAosApplication::STATE_CONNECTING:  // FALL-THROUGH
        case IAosApplication::STATE_CONNECTED:   // FALL-THROUGH
        case IAosApplication::STATE_UPDATING:
            return IMS_TRUE;

        default:
            return IMS_FALSE;
    }
}

PROTECTED
IMS_BOOL AosConnector::IsPcscfConfigured() const
{
    return m_bPcscfConfigured;
}

PROTECTED
IMS_BOOL AosConnector::IsPcoWaitingRequired() const
{
    IMS_BOOL bResult = IMS_FALSE;

    if (!IsCarrierSignalPcoEnabled())
    {
        return bResult;
    }

    IMS_SINT32 nSlotId = m_piAppContext->GetSlotId();
    if (GET_N_CONFIG(nSlotId) != IMS_NULL && GET_N_CONFIG(nSlotId)->IsSupportLimitedAdminSmsMode())
    {
        bResult = m_piConnection->GetCarrierSignalPcoValue() == PCO_INVALID_VALUE;
    }

    A_IMS_TRACE_D(APPPROFILE, "IsPcoWaitingRequired : %s", _TRACE_B_(bResult), 0, 0);
    return bResult;
}

PROTECTED
IMS_BOOL AosConnector::IsCarrierSignalPcoEnabled() const
{
    IMS_SINT32 nSlotId = m_piAppContext->GetSlotId();
    return (UtilService::GetUtilService()->GetPrivateProperty()->GetPersistentInt(
                    ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST, nSlotId) == 1);
}

PROTECTED
IMS_BOOL AosConnector::IsPending() const
{
    return (m_nPendingFeature != PENDING_NONE);
}

PROTECTED
IMS_BOOL AosConnector::IsTerminating() const
{
    return m_bIsTerminating;
}

PROTECTED
IMS_BOOL AosConnector::IsTimerRunning(IN IMS_UINT32 nType) const
{
    if (nType == TIMER_IPV6)
    {
        return (m_piIpv6Timer != IMS_NULL);
    }

    if (nType == TIMER_STOP_DELAY)
    {
        return (m_piStopDelayTimer != IMS_NULL);
    }

    if (nType == TIMER_READY_RECOVERY)
    {
        return (m_piReadyRecoveryTimer != IMS_NULL);
    }
    if (nType == TIMER_PCO_WAITING)
    {
        return (m_piPcoWaitingTimer != IMS_NULL);
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AosConnector::IsDataConnectedWithoutPending() const
{
    if (IsPending())
    {
        return IMS_FALSE;
    }

    if (!IsDataConnected())
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AosConnector::IsIpv6PcscfUnavailable() const
{
    if (!m_piPcscf->IsSinglePcoScheme())
    {
        return IMS_FALSE;
    }

    const AStringArray& objPcscfs = m_piConnection->GetPcscfAddress(IpAddress::IPV6);
    for (IMS_SINT32 nAt = 0; nAt < objPcscfs.GetCount(); nAt++)
    {
        const AString& strPcscf = objPcscfs.GetElementAt(nAt);
        IpAddress objIpa;
        if (objIpa.Parse(strPcscf) && !objIpa.IsAnyAddress())
        {
            return IMS_FALSE;
        }
    }
    return IMS_TRUE;
}

PROTECTED
void AosConnector::CheckReadyRecoveryAndSetTimer()
{
    if (m_piConnection->GetConnectionType() == NetworkPolicy::APN_IMS)
    {
        if (m_piPcscf->IsSinglePcoScheme() && !m_piPcscf->IsAsyncDnsDiscovery())
        {
            HandleInvalidPcscfAddress();
        }
    }
}

PROTECTED
IMS_BOOL AosConnector::CheckIpChangedForEmergency()
{
    if (!IsEmergencyType())
    {
        return IMS_FALSE;
    }

    IAosRegistration* piRegistration = m_piAppContext->GetRegistration();
    if (piRegistration->GetState() == IAosRegistration::STATE_OFFLINE)
    {
        return IMS_FALSE;
    }

    AString strIpa;
    IMS_UINT32 nNa = 0;
    piRegistration->GetProperty(IAosRegistration::PROPERTY_LOCAL_ADDRESS, nNa, strIpa);

    IpAddress objIpa(strIpa);
    if (objIpa.IsUnknownAddress())
    {
        return IMS_FALSE;
    }

    const IpAddress& objCurrIpa = m_piConnection->GetLocalAddress(
            (objIpa.IsIPv6Address()) ? IpAddress::IPV6 : IpAddress::IPV4);

    return objIpa.Equals(objCurrIpa);
}

PROTECTED
IMS_BOOL AosConnector::CheckIpaAndProcessReadyRecovery()
{
    if (m_piConnection->GetConnectionType() == NetworkPolicy::APN_IMS)
    {
        IMS_BOOL bIsReady = IMS_FALSE;

        const AStringArray& objPcscfs = m_piPcscf->GetPcscfs();

        if (objPcscfs.GetFirstElement().GetLength() > 0)
        {
            IpAddress objPcscf;

            bIsReady = IMS_TRUE;
            if (objPcscf.Parse(objPcscfs.GetFirstElement()))
            {
                const IpAddress& objIpa = m_piConnection->GetLocalAddress(
                        (objPcscf.IsIPv6Address()) ? IpAddress::IPV6 : IpAddress::IPV4);

                if (objPcscf.IsNoneAddress() || objIpa.IsNoneAddress())
                {
                    bIsReady = IMS_FALSE;
                }
            }
            else
            {
                bIsReady = IMS_FALSE;
            }
        }

        if (!bIsReady)
        {
            HandleInvalidPcscfAddress();
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PROTECTED
void AosConnector::HandleInvalidPcscfAddress()
{
    A_IMS_TRACE_D(
            APPPROFILE, "HandleInvalidPcscfAddress::fail count (%d)", m_nReadyRecoveryCount, 0, 0);
    IMS_SINT32 nSlotId = m_piAppContext->GetSlotId();
    if (GET_N_CONFIG(nSlotId) == IMS_NULL)
    {
        A_IMS_TRACE_D(APPPROFILE, "Can not get configurations for PCSCF recovery", 0, 0, 0);
        return;
    }

    m_nReadyRecoveryCount++;
    IMS_SINT32 nMaxRetryCnt = GET_N_CONFIG(nSlotId)->GetPcscfRecoveryMaxRetryCnt();
    if (m_nReadyRecoveryCount <= nMaxRetryCnt)
    {
        StartTimer(TIMER_READY_RECOVERY, GET_N_CONFIG(nSlotId)->GetPcscfRecoveryWaitTime() * 1000);
        return;
    }

    IMS_SINT32 nBaseTime = GET_N_CONFIG(nSlotId)->GetPcscfRecoveryBaseTime();
    IMS_SINT32 nMaxTime = GET_N_CONFIG(nSlotId)->GetPcscfRecoveryMaxTime();
    if (nBaseTime == 0 || nMaxTime == 0)
    {
        Notify(LISTENER_TYPE_DEACTIVATED, REASON_PCSCF_DISCOVERY_FAILED);
        return;
    }

    IMS_SINT32 nDeterminedWaitTime = m_pUtil->WaitTimeForFlowRecovery(
            nBaseTime, nMaxTime, m_nReadyRecoveryCount - nMaxRetryCnt);
    StartTimer(TIMER_READY_RECOVERY, nDeterminedWaitTime * 1000);
}

PROTECTED
IMS_BOOL AosConnector::SelectIpVersion()
{
    A_IMS_TRACE_I(APPPROFILE, "SelectIpVersion", 0, 0, 0);

    // Check if local address and PCSCF address are in the same IP version.

    const IpAddress& objLocalAddr = m_piConnection->GetLocalAddress();

    if (objLocalAddr.IsUnknownAddress())
    {
        A_IMS_TRACE_I(APPPROFILE, "local address is unknown", 0, 0, 0);
        return IMS_FALSE;
    }

    const AStringArray& objPcscfs = m_piPcscf->GetPcscfs();

    if (objPcscfs.GetCount() <= 0)
    {
        A_IMS_TRACE_I(APPPROFILE, "no pcscf address exists", 0, 0, 0);
        return IMS_FALSE;
    }

    const AString& strPcscf = objPcscfs.GetElementAt(0);

    if (strPcscf == IMS_NULL)
    {
        A_IMS_TRACE_I(APPPROFILE, "pcscf address at index (0) is null", 0, 0, 0);
        return IMS_FALSE;
    }

    if (strPcscf.GetLength() == 0)
    {
        A_IMS_TRACE_I(APPPROFILE, "pcscf address at index (0) is empty", 0, 0, 0);
        return IMS_FALSE;
    }

    IpAddress objPcscfAddress(strPcscf);

    if (objPcscfAddress.IsUnknownAddress())
    {
        A_IMS_TRACE_I(APPPROFILE, "pcscf address forming failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bPcscfIpv6 = objPcscfAddress.IsIPv6Address();

    if (bPcscfIpv6 != objLocalAddr.IsIPv6Address())
    {
        A_IMS_TRACE_I(APPPROFILE, "change default ip version", 0, 0, 0);

        const IpAddress& objIpa =
                m_piConnection->GetLocalAddress((bPcscfIpv6) ? IpAddress::IPV6 : IpAddress::IPV4);

        if (bPcscfIpv6 != objIpa.IsIPv6Address())
        {
            A_IMS_TRACE_I(APPPROFILE, "selecting ip version is failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PROTECTED
void AosConnector::Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nReason /* = REASON_NONE */)
{
    if (m_piListener == IMS_NULL)
    {
        return;
    }

    switch (nType)
    {
        case LISTENER_TYPE_ACTIVATED:
            m_piListener->Connector_Activated();
            break;

        case LISTENER_TYPE_DEACTIVATED:
            m_piListener->Connector_Deactivated(nReason);
            break;

        case LISTENER_TYPE_UPDATED:
            m_piListener->Connector_Updated(nReason);
            break;

        default:
            break;
    }
}

PROTECTED VIRTUAL void AosConnector::CleanAll()
{
    ClearTimers();
    ClearPending();
    SetPcscfConfigured(IMS_FALSE);
    SetDataConnected(IMS_FALSE);
    m_bIsPcscfChangeIgnored = IMS_FALSE;

    SetState(STATE_IDLE);
}

PROTECTED VIRTUAL IMS_BOOL AosConnector::ConfigurePcscf()
{
    m_piPcscf->Configure();
    SetPcscfConfigured(m_piPcscf->IsConfigured());

    return IsPcscfConfigured();
}

PROTECTED VIRTUAL void AosConnector::ProcessIpv6TimerExpired()
{
    A_IMS_TRACE_I(APPPROFILE, "ProcessIpv6TimerExpired", 0, 0, 0);

    StopTimer(TIMER_IPV6);

    m_pUtil->RemoveFeature(PENDING_IPV6_DELAY, m_nPendingFeature);

    if (IsPcoWaitingRequired())
    {
        m_pUtil->AddFeature(PENDING_PCO_WAITING, m_nPendingFeature);
        StartTimer(TIMER_PCO_WAITING, WAITING_PCO_VALUE_TIMEOUT_MILLIS);
        return;
    }

    if (!IsReady() && IsDataConnectedWithoutPending())
    {
        ProcessCheckingPcscfAndIpa();
    }
}

PROTECTED VIRTUAL void AosConnector::ProcessStopDelayTimerExpired()
{
    A_IMS_TRACE_I(APPPROFILE, "ProcessStopDelayTimerExpired", 0, 0, 0);

    StopTimer(TIMER_STOP_DELAY);

    Stop();
}

PROTECTED VIRTUAL void AosConnector::ProcessReadyRecoveryTimerExpired()
{
    A_IMS_TRACE_I(APPPROFILE, "ProcessReadyRecoveryTimerExpired", 0, 0, 0);

    StopTimer(TIMER_READY_RECOVERY);
    if (!IsTimerRunning(TIMER_STOP_DELAY))
    {
        Stop();
    }
}

PROTECTED VIRTUAL void AosConnector::ProcessPcoWaitingTimerExpired()
{
    A_IMS_TRACE_I(APPPROFILE, "ProcessPcoWaitingTimerExpired", 0, 0, 0);
    StopTimer(TIMER_PCO_WAITING);

    m_pUtil->RemoveFeature(PENDING_PCO_WAITING, m_nPendingFeature);

    if (!IsReady() && IsDataConnectedWithoutPending())
    {
        ProcessCheckingPcscfAndIpa();
    }
}

PROTECTED VIRTUAL void AosConnector::ProcessCheckingPcscfAndIpa()
{
    A_IMS_TRACE_I(APPPROFILE, "ProcessCheckingPcscfAndIpa", 0, 0, 0);
    if (ConfigurePcscf())
    {
        if (CheckIpaAndProcessReadyRecovery())
        {
            SetState(STATE_READY);
            Notify(LISTENER_TYPE_ACTIVATED);
        }
        return;
    }

    if (IsEmergencyType())
    {
        CleanAll();
        Notify(LISTENER_TYPE_DEACTIVATED, REASON_FAILED);
        return;
    }

    A_IMS_TRACE_I(APPPROFILE, "p-cscf is not configured", 0, 0, 0);
    m_pUtil->AddFeature(PENDING_PCSCF_CONFIG_READY, m_nPendingFeature);
    CheckReadyRecoveryAndSetTimer();
}

PROTECTED VIRTUAL void AosConnector::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_IPV6:
            ppiTimer = &m_piIpv6Timer;
            break;

        case TIMER_STOP_DELAY:
            ppiTimer = &m_piStopDelayTimer;
            break;

        case TIMER_READY_RECOVERY:
            ppiTimer = &m_piReadyRecoveryTimer;
            break;

        case TIMER_PCO_WAITING:
            ppiTimer = &m_piPcoWaitingTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer != IMS_NULL)
    {
        StopTimer(nType);
    }

    *ppiTimer = m_pUtil->StartTimer(nDuration, this, TimerToString(nType));
}

PROTECTED VIRTUAL void AosConnector::StopTimer(IN IMS_UINT32 nType)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_IPV6:
            ppiTimer = &m_piIpv6Timer;
            break;

        case TIMER_STOP_DELAY:
            ppiTimer = &m_piStopDelayTimer;
            break;

        case TIMER_READY_RECOVERY:
            ppiTimer = &m_piReadyRecoveryTimer;
            break;

        case TIMER_PCO_WAITING:
            ppiTimer = &m_piPcoWaitingTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer == IMS_NULL)
        return;

    m_pUtil->StopTimer(*ppiTimer, TimerToString(nType));
}

PROTECTED VIRTUAL void AosConnector::ClearTimers()
{
    StopTimer(TIMER_IPV6);

    if ((IsTerminating() == IMS_FALSE) && (m_piStopDelayTimer != IMS_NULL))
    {
        ProcessStopDelayTimerExpired();
    }
    else
    {
        StopTimer(TIMER_STOP_DELAY);
    }

    StopTimer(TIMER_READY_RECOVERY);
    StopTimer(TIMER_PCO_WAITING);
}

PROTECTED VIRTUAL void AosConnector::AosConnection_StateChanged(IN IMS_UINT32 nDataState)
{
    A_IMS_TRACE_I(APPPROFILE, "AosConnection_StateChanged :: state(%d)", nDataState, 0, 0);

    if (nDataState == IAosConnection::STATE_ACTIVE)
    {
        if (m_nState == STATE_READY)
        {
            return;
        }

        if ((m_piConnection->GetIpcanCategory() == IIpcan::CATEGORY_MOBILE) &&
                m_piConnection->IsIpv6Preferred())
        {
            // check the local IP Address version
            IpAddress objIpAddress = m_piConnection->GetLocalAddress();
            IMS_BOOL bLocalIpv4 = objIpAddress.IsIPv4Address();

            if (!IsDataConnected() && !IsIpv6PcscfUnavailable() && bLocalIpv4)
            {
                A_IMS_TRACE_I(APPPROFILE, "wait for obtaining IPv6 address", 0, 0, 0);

                SetDataConnected(IMS_TRUE);
                m_pUtil->AddFeature(PENDING_IPV6_DELAY, m_nPendingFeature);
                StartTimer(TIMER_IPV6, IPV6_ADDRESS_WAIT_TIME_SEC * 1000);
                return;
            }
        }

        SetDataConnected(IMS_TRUE);

        if (IsPcoWaitingRequired())
        {
            m_pUtil->AddFeature(PENDING_PCO_WAITING, m_nPendingFeature);
            StartTimer(TIMER_PCO_WAITING, WAITING_PCO_VALUE_TIMEOUT_MILLIS);
            return;
        }

        if (IsPending())
        {
            A_IMS_TRACE_I(APPPROFILE, "connection is pending (%d)", m_nPendingFeature, 0, 0);
            return;
        }

        ProcessCheckingPcscfAndIpa();
    }
    else  // STATE_IDLE, STATE_ACTIVATING
    {
        CleanAll();
        Notify(LISTENER_TYPE_DEACTIVATED, REASON_DISCONNECTED);
    }
}

PROTECTED VIRTUAL void AosConnector::AosConnection_IpChanged()
{
    A_IMS_TRACE_I(APPPROFILE, "AosConnection_IpChanged", 0, 0, 0);

    if (IsReady())
    {
        if (CheckIpChangedForEmergency() &&
                !(GET_N_CONFIG(m_piAppContext->GetSlotId())->IsEmergencyCallbackModeSupported()))
        {
            A_IMS_TRACE_I(APPPROFILE, "AosConnection_IpChanged :: ip change is ignored", 0, 0, 0);
            return;
        }

        if (SelectIpVersion())
        {
            Notify(LISTENER_TYPE_UPDATED, REASON_IP_CHANGED);
        }
        else
        {
            SetState(STATE_IDLE);
            Notify(LISTENER_TYPE_DEACTIVATED, REASON_IP_CHANGED);
        }

        return;
    }

    // STATE_IDLE
    if (IsDataConnected())
    {
        if (m_pUtil->IsFeatureOn(PENDING_IPV6_DELAY, m_nPendingFeature))
        {
            if (!m_piConnection->GetLocalAddress().IsIPv6Address())
            {
                A_IMS_TRACE_I(APPPROFILE, "AosConnection_IpChanged :: wait for timer expiration", 0,
                        0, 0);
                return;
            }

            m_pUtil->RemoveFeature(PENDING_IPV6_DELAY, m_nPendingFeature);
            StopTimer(TIMER_IPV6);
        }

        if (m_piPcscf->IsAsyncDnsDiscovery() && IsPending())
        {
            return;
        }

        if (!ConfigurePcscf())
        {
            A_IMS_TRACE_I(
                    APPPROFILE, "AosConnection_IpChanged :: p-cscf is not configured", 0, 0, 0);
            m_pUtil->AddFeature(PENDING_PCSCF_CONFIG_READY, m_nPendingFeature);
            CheckReadyRecoveryAndSetTimer();
            return;
        }
        else
        {
            m_pUtil->RemoveFeature(PENDING_PCSCF_CONFIG_READY, m_nPendingFeature);
        }

        if (CheckIpaAndProcessReadyRecovery())
        {
            SetState(STATE_READY);
            Notify(LISTENER_TYPE_ACTIVATED);
        }
    }
}

PROTECTED VIRTUAL void AosConnector::AosConnection_IpcanCatChanged()
{
    A_IMS_TRACE_I(APPPROFILE, "AosConnection_IpcanCatChanged", 0, 0, 0);

    Notify(LISTENER_TYPE_UPDATED, REASON_IPCAN_CAT_CHANGED);
}

PROTECTED VIRTUAL void AosConnector::AosConnection_PcscfChanged()
{
    A_IMS_TRACE_I(APPPROFILE, "AosConnection_PcscfChanged", 0, 0, 0);

    if (!IsPcscfChangeAvailable())
    {
        return;
    }

    IMS_SINT32 nSlotId = m_piAppContext->GetSlotId();
    const IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(nSlotId);
    if (!IsEmergencyType() &&
        GET_N_CONFIG(nSlotId)->ShouldKeepExistingPcscfOnPcscfChangeDuringTheCall() &&
        m_piAppContext->GetApp()->IsOn() &&
        (piCallTracker != IMS_NULL && piCallTracker->IsNormalCallActive()))
    {
        A_IMS_TRACE_D(APPPROFILE, "AosConnection_PcscfChanged :: ignore in incall mode", 0, 0,
                0);

        m_bIsPcscfChangeIgnored = IMS_TRUE;
        return;
    }

    if (!m_piPcscf->CheckAndProcessChangeFromPco())
    {
        A_IMS_TRACE_D(APPPROFILE, "AosConnection_PcscfChanged :: ignore", 0, 0, 0);
        return;
    }

    Notify(LISTENER_TYPE_UPDATED, REASON_PCSCF_CHANGED);
}

PROTECTED VIRTUAL void AosConnector::AosConnection_ConnectionFailed()
{
    A_IMS_TRACE_I(APPPROFILE, "AosConnection_ConnectionFailed", 0, 0, 0);

    CleanAll();
    Notify(LISTENER_TYPE_DEACTIVATED, REASON_PERMANENTLY_FAILED);
}

PROTECTED VIRTUAL void AosConnector::Pcscf_NotifyResult(IN IMS_BOOL bResult)
{
    A_IMS_TRACE_I(APPPROFILE, "Pcscf_NotifyResult :: result(%s)", (bResult) ? "SUCCESS" : "FAILURE",
            0, 0);

    if (bResult)
    {
        m_pUtil->RemoveFeature(PENDING_PCSCF_CONFIG_READY, m_nPendingFeature);

        if (!IsReady() && IsDataConnectedWithoutPending() && CheckIpaAndProcessReadyRecovery())
        {
            SetState(STATE_READY);
            Notify(LISTENER_TYPE_ACTIVATED);
        }
    }
}

PROTECTED VIRTUAL void AosConnector::ServicePhone_PcoValueChanged(IN IMS_SINT32 nValue)
{
    A_IMS_TRACE_I(APPPROFILE, "ServicePhone_PcoValueChanged :: nValue(%d)", nValue, 0, 0);

    StopTimer(TIMER_PCO_WAITING);
    m_pUtil->RemoveFeature(PENDING_PCO_WAITING, m_nPendingFeature);

    m_piConnection->SetCarrierSignalPcoValue(nValue);

    if (!IsReady())
    {
        if (IsDataConnectedWithoutPending())
        {
            ProcessCheckingPcscfAndIpa();
        }
        return;
    }

    if (m_piAppContext->GetRegistration()->GetState() == IAosRegistration::STATE_DEREGISTERING)
    {
        return;
    }

    IMS_BOOL bIsLimited = m_piConnection->IsLimitedServicePcoValue();
    IMS_BOOL bLimitedMode =
            m_piAppContext->GetRegistration()->GetMode() == IAosRegistration::MODE_LIMITED;
    if (bIsLimited != bLimitedMode)
    {
        Notify(LISTENER_TYPE_DEACTIVATED, REASON_LIMITED_SERVICE_PCO);
    }
}

PROTECTED VIRTUAL void AosConnector::ServicePhone_CrossSimStatusChanged(
        IN IMS_BOOL bCrossSimConnected)
{
    if (IsEmergencyType())
    {
        return;
    }

    m_bCrossSimConnected = bCrossSimConnected;
}

PROTECTED VIRTUAL void AosConnector::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piIpv6Timer)
    {
        ProcessIpv6TimerExpired();
        return;
    }

    if (piTimer == m_piStopDelayTimer)
    {
        ProcessStopDelayTimerExpired();
        return;
    }

    if (piTimer == m_piReadyRecoveryTimer)
    {
        ProcessReadyRecoveryTimerExpired();
        return;
    }

    if (piTimer == m_piPcoWaitingTimer)
    {
        ProcessPcoWaitingTimerExpired();
        return;
    }
}

PROTECTED VIRTUAL void AosConnector::CleanUp()
{
    A_IMS_TRACE_I(APPPROFILE, "CleanUp", 0, 0, 0);

    m_bIsTerminating = IMS_TRUE;

    CleanAll();

    if (m_piService != IMS_NULL)
    {
        m_piService->RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
    }

    if (m_piPcscf != IMS_NULL)
    {
        m_piPcscf->SetListener(IMS_NULL);
    }

    if (m_piConnection != IMS_NULL)
    {
        m_piConnection->RemoveListener(this);
    }
}

PROTECTED GLOBAL const IMS_CHAR* AosConnector::TimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TIMER_IPV6:
            return "TIMER_IPV6";

        case TIMER_STOP_DELAY:
            return "TIMER_STOP_DELAY";

        case TIMER_READY_RECOVERY:
            return "TIMER_READY_RECOVERY";

        case TIMER_PCO_WAITING:
            return "TIMER_PCO_WAITING";

        default:
            return "__INVALID__";
    }
}
