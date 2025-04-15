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
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "ServicePhoneInfo.h"
#include "ServiceTimer.h"
#include "ServiceEvent.h"
#include "ServiceNetworkPolicy.h"
#include "CarrierConfig.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosConnection.h"
#include "interface/IAosNetTrackerListener.h"

#include "provider/AosNConfiguration.h"
#include "provider/AosProvider.h"
#include "provider/AosUtil.h"

#include "network/AosNetTracker.h"

__IMS_TRACE_TAG_AOS__;

#define CNXID m_strTag.GetStr()

PUBLIC
AosNetTracker::AosNetTracker(IN IAosAppContext* piAppContext) :
        m_nCnxPolicy(0),
        m_nCnxPolicyInRoaming(0),
        m_piNetWatcherInfo(IMS_NULL),
        m_piWifiWatcher(IMS_NULL),
        m_piConnection(piAppContext->GetConnection()),
        m_pUtil(IMS_NULL),
        m_piAosNConfig(GET_N_CONFIG(piAppContext->GetSlotId())),
        m_nSlotId(piAppContext->GetSlotId()),
        m_nNetServiceType(NW_REPORT_SRV_NOSRV),
        m_nNetRadioType(NW_REPORT_RADIO_NOSRV),
        m_nChangingRat(NW_REPORT_RADIO_NOSRV),
        m_nNetVoiceRadioType(NW_REPORT_RADIO_NOSRV),
        m_nNetChangingVoiceRadioType(NW_REPORT_RADIO_NOSRV),
        m_bIsNetAvailable(IMS_FALSE),
        m_bIsRoaming(IMS_FALSE),
        m_bIsEpdgEnabled(IMS_FALSE),
        m_bIsWifiConnected(IMS_FALSE),
        m_bIsDataConnected(IMS_FALSE),
        m_nFeature(FEATURE_NONE),
        m_nServiceInTime(SERVICE_IN_TIME_MILLI_SEC),
        m_nServiceOutTime(SERVICE_OUT_TIME_MILLI_SEC),
        m_nRatTime(0),
        m_nVoiceRatGuardTime(0),
        m_piServiceInTimer(IMS_NULL),
        m_piServiceOutTimer(IMS_NULL),
        m_piRatTimer(IMS_NULL),
        m_piVoiceRatTimer(IMS_NULL)
{
    m_strTag.Sprintf("%d:%s.cnx", m_nSlotId, piAppContext->GetProfileId().GetStr());

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosNetTracker = %" PFLS_u "/%" PFLS_x, m_strTag.GetStr(),
            sizeof(AosNetTracker), this);
}

PUBLIC VIRTUAL AosNetTracker::~AosNetTracker()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosNetTracker = %" PFLS_u "/%" PFLS_x, m_strTag.GetStr(),
            sizeof(AosNetTracker), this);

    // Should not use the AosAppContext that is destroyed

    ClearTimers();
    m_objListeners.Clear();

    if (m_piWifiWatcher != IMS_NULL)
    {
        m_piWifiWatcher->RemoveObserver(this);
        m_piWifiWatcher = IMS_NULL;
    }

    if (m_piConnection != IMS_NULL)
    {
        m_piConnection->RemoveListener(this);
        m_piConnection = IMS_NULL;
    }

    if (m_piNetWatcherInfo != IMS_NULL)
    {
        m_piNetWatcherInfo->RemoveObserver(this);
        m_piNetWatcherInfo = IMS_NULL;
    }

    if (m_piAosNConfig != IMS_NULL)
    {
        m_piAosNConfig->RemoveListener(this);
        m_piAosNConfig = IMS_NULL;
    }

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_nSlotId);
}

PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsServiceIn(IN IMS_UINT32 nType /* = TYPE_DEFAULT */)
{
    if (nType == TYPE_WLAN)
    {
        return IsWifiConnected();
    }

    if (nType == TYPE_MOBILE)
    {
        return m_bIsNetAvailable;
    }

    if (IsWlanEnabled())
    {
        return m_bIsNetAvailable || IsWifiConnected();
    }

    if (IsEpdgEnabled())
    {
        return IMS_TRUE;
    }

    return m_bIsNetAvailable;
}

PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsDataIn()
{
    return IsServiceAvailable(GetAccessPolicy(), m_nNetServiceType);
}

PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsNetworkIn()
{
    return IsRadioTechAvailable(GetAccessPolicy(), m_nNetRadioType);
}

PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsEmergencyAttach()
{
    return m_piNetWatcherInfo->IsEmergencyOnly();
}

PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsSuspended()
{
    if (IsEpdgEnabled())
    {
        return IMS_FALSE;
    }

    if (IsDataConnected() && !IsServiceIn(TYPE_MOBILE))
    {
        A_IMS_TRACE_I(CNXID, "Network Suspended", 0, 0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsSessionContinuitySupported()
{
    IMS_UINT32 nPolicy = GetAccessPolicy();

    return ((nPolicy & NW_REPORT_RADIO_WLAN) &&
            ((nPolicy & NW_REPORT_RADIO_LTE) || (nPolicy & NW_REPORT_RADIO_NR)));
}

PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsServiceTimerRunning()
{
    return ((m_piServiceInTimer != IMS_NULL) || (m_piServiceOutTimer != IMS_NULL));
}

PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsImsVoiceCallSupported()
{
    return m_piNetWatcherInfo->IsImsVoiceCallSupported();
}

PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsRoaming()
{
    return m_piNetWatcherInfo->GetRoamingState() > 0;
}

PUBLIC VIRTUAL void AosNetTracker::SetListener(IN IAosNetTrackerListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosNetTrackerListener* pTmpListener = m_objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            A_IMS_TRACE_D(CNXID, "SetListener() - Listener (%" PFLS_x ") is already set",
                    piListener, 0, 0);
            return;
        }
    }

    m_objListeners.Append(piListener);

    A_IMS_TRACE_D(CNXID, "SetListener() - Listener (%" PFLS_x ") is set", piListener, 0, 0);

    piListener->NetTracker_StatusChanged();
}

PUBLIC VIRTUAL void AosNetTracker::RemoveListener(IN IAosNetTrackerListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosNetTrackerListener* pTmpListener = m_objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objListeners.RemoveAt(i);

            A_IMS_TRACE_D(
                    CNXID, "RemoveListener - Listener (%" PFLS_x ") is removed", piListener, 0, 0);
            return;
        }
    }
}

PUBLIC VIRTUAL IMS_UINT32 AosNetTracker::GetMobileChangingNetworkType()
{
    return m_nChangingRat;
}

PUBLIC VIRTUAL IMS_UINT32 AosNetTracker::GetMobileNetworkType()
{
    return m_nNetRadioType;
}

PUBLIC VIRTUAL IMS_UINT32 AosNetTracker::GetMobileNetworkRegistrationRejectCause()
{
    return m_piNetWatcherInfo->GetNetworkRegistrationRejectCause();
}

PUBLIC VIRTUAL IMS_SINT32 AosNetTracker::GetMobileVoiceServiceState()
{
    return m_piNetWatcherInfo->GetNetVoiceServiceType();
}

PUBLIC VIRTUAL IMS_UINT32 AosNetTracker::GetMobileVoiceNetworkType()
{
    A_IMS_TRACE_D(CNXID, "GetMobileVoiceNetworkType :: (%s)",
            RadioTypeToString(m_nNetVoiceRadioType), 0, 0);

    return m_nNetVoiceRadioType;
}

PUBLIC VIRTUAL IMS_UINT32 AosNetTracker::GetNetworkType()
{
    if (IsEpdgEnabled())
    {
        return NW_REPORT_RADIO_WLAN;
    }

    return m_nNetRadioType;
}

PUBLIC VIRTUAL void AosNetTracker::SetRatGuardTime(IN IMS_UINT32 nGuardTime)
{
    m_nRatTime = nGuardTime;

    // Here it uses the same value for RAT change and voice RAT change by default.
    m_nVoiceRatGuardTime = nGuardTime;

    if (m_nRatTime > 0)
    {
        m_nFeature |= FEATURE_RAT_GUARD;
        m_nFeature |= FEATURE_VOICE_RAT_GUARD;

        /*
          When SetRatGuardTime() is enabled, m_nChangingRat must be initialized to m_nNetRadioType.
          When FEATURE_RAT_GUARD is not enabled, it doesn't update m_nChangingRat.
          When FEATURE_RAT_GUARD become enabled,
          it's possible m_nChangingRat and m_nNetRadioType are different.
          If RAT changes but the new RAT is the same are m_nChangingRat,
          then, the change will not be notified.
        */
        m_nChangingRat = m_nNetRadioType;
        m_nNetChangingVoiceRadioType = m_nNetVoiceRadioType;
    }
    else
    {
        m_nFeature &= ~(FEATURE_RAT_GUARD);
        m_nFeature &= ~(FEATURE_VOICE_RAT_GUARD);
    }

    A_IMS_TRACE_I(CNXID, "SetRatGuardTime() :: Feature(%s)", FeaturesToString().GetStr(), 0, 0);
}

PUBLIC VIRTUAL void AosNetTracker::SetSrvOutGuardTime(IN IMS_UINT32 nGuardTime)
{
    m_nServiceOutTime = nGuardTime;

    if (m_nServiceOutTime > 0)
    {
        m_nFeature |= FEATURE_OUT_GUARD;
    }
    else
    {
        m_nFeature &= ~(FEATURE_OUT_GUARD);
    }

    A_IMS_TRACE_I(CNXID, "SetSrvOutGuardTime() :: Feature(%s)", FeaturesToString().GetStr(), 0, 0);
}

PUBLIC VIRTUAL void AosNetTracker::SetSrvInGuardTime(IN IMS_UINT32 nGuardTime)
{
    m_nServiceInTime = nGuardTime;

    if (m_nServiceInTime > 0)
    {
        m_nFeature |= FEATURE_IN_GUARD;
    }
    else
    {
        m_nFeature &= ~(FEATURE_IN_GUARD);
    }

    A_IMS_TRACE_I(CNXID, "SetSrvInGuardTime() :: Feature(%s)", FeaturesToString().GetStr(), 0, 0);
}

PUBLIC VIRTUAL void AosNetTracker::NetworkWatcher_NotifyStatus(IN INetworkWatcher* piNetWatcherInfo)
{
    if (m_piNetWatcherInfo != piNetWatcherInfo)
    {
        return;
    }

    A_IMS_TRACE_D(CNXID, "NotifyNetWatcherStatus", 0, 0, 0);

    if (m_nFeature != FEATURE_NONE)
    {
        ProcessNetworkChanged(REASON_NET_STATE_CHANGED);
        ProcessVoiceNetworkChanged();
        return;
    }

    if (UpdateNetworkStatus())
    {
        Notify();
    }
}

PUBLIC VIRTUAL void AosNetTracker::WifiWatcher_NotifyStateChanged(IN IWifiWatcher* piWifiWatcher)
{
    if (m_piWifiWatcher != piWifiWatcher)
    {
        return;
    }

    IMS_BOOL bCurrConnected =
            (m_piWifiWatcher->GetState() == IWifiWatcher::STATE_CONNECTED) ? IMS_TRUE : IMS_FALSE;

    if (IsWifiConnected() != bCurrConnected)
    {
        SetWifiConnected(bCurrConnected);
        Notify();
    }
}

PROTECTED VIRTUAL void AosNetTracker::NConfiguration_NotifyConfigChanged()
{
    A_IMS_TRACE_I(CNXID, "NConfiguration_NotifyConfigChanged :: changed", 0, 0, 0);

    if (m_piAosNConfig != IMS_NULL)
    {
        IMS_BOOL bWlanEnabled = IsWlanEnabled();
        InitConfig();
        Notify();

        if (IsWlanEnabled() != bWlanEnabled)
        {
            UpdateWifiObserver();
        }
    }
}

PUBLIC VIRTUAL void AosNetTracker::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    A_IMS_TRACE_I(CNXID, "Event_NotifyEvent :: E(%X)/W(%d)/L(%d)", nEvent, nWParam, nLParam);

    switch (nEvent)
    {
        case IMS_EVENT_ROAMING_STATE:
        {
            IMS_BOOL bCurrRoaming = (nWParam == IMS_ROAMING_STATE_OFF)
                    ? (nLParam == IMS_ROAMING_STATE_ON)
                    : IMS_TRUE;

            if (m_bIsRoaming != bCurrRoaming)
            {
                m_bIsRoaming = bCurrRoaming;

                if (m_nFeature != FEATURE_NONE)
                {
                    ProcessNetworkChanged(REASON_ROAMING_SATAE_CHANGED);
                    ProcessVoiceNetworkChanged();
                }
                else
                {
                    UpdateNetworkStatus();
                    Notify();
                }
            }
            break;
        }
        default:
            // No operation
            break;
    }
}

PROTECTED
void AosNetTracker::Init()
{
    if (m_piAosNConfig != IMS_NULL)
    {
        InitConfig();
        m_piAosNConfig->SetListener(this);
    }
    InitObject();
    UpdateNetworkStatus();
}

PRIVATE
void AosNetTracker::InitConfig()
{
    m_nCnxPolicy = 0;
    m_nCnxPolicyInRoaming = 0;

    if (IsCnxTypeEqual(NetworkPolicy::APN_IMS))
    {
        m_nCnxPolicy |= NW_REPORT_SRV_SRV;

        InitCnxPolicy(m_piAosNConfig->GetSupportedRats());

        if (m_piAosNConfig->IsSmsOverImsSupported())
        {
            InitCnxPolicy(m_piAosNConfig->GetSmsOverImsSupportedRats());
        }

        if (IsVonrSupported())
        {
            m_nCnxPolicy |= NW_REPORT_RADIO_NR;
        }

        if (IsRoamingAccessPolicyRequired())
        {
            m_nCnxPolicyInRoaming |= NW_REPORT_SRV_SRV;
            InitRoamingCnxPolicy(m_piAosNConfig->GetSupportedRoamingRats());

            A_IMS_TRACE_I(
                    CNXID, "InitConfig :: m_nCnxPolicyInRoaming(%X)", m_nCnxPolicyInRoaming, 0, 0);
            IMS_EVENT_AddListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_nSlotId);
        }
        else
        {
            IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_nSlotId);
        }
    }
    else if (IsCnxTypeEqual(NetworkPolicy::APN_EMERGENCY))
    {
        m_nCnxPolicy = 0xFFFFFFFF & (~NW_REPORT_RADIO_WLAN);
        ImsVector<IMS_SINT32>& objRats = m_piAosNConfig->GetEmergencyOverImsSupportedRats();
        if (objRats.Contains(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN))
        {
            m_nCnxPolicy |= NW_REPORT_RADIO_WLAN;
        }
    }
    else if (IsCnxTypeEqual(NetworkPolicy::APN_WIFI))
    {
        m_nCnxPolicy = 0x01000000;
    }

    A_IMS_TRACE_I(CNXID, "InitConfig :: nPolicy(%X)", m_nCnxPolicy, 0, 0);

    if (m_nServiceInTime > 0)
    {
        m_nFeature |= FEATURE_IN_GUARD;
    }

    if (m_nServiceOutTime > 0)
    {
        m_nFeature |= FEATURE_OUT_GUARD;
    }
}

PRIVATE
void AosNetTracker::InitCnxPolicy(IN ImsVector<IMS_SINT32>& objRats)
{
    for (int i = 0; i < objRats.GetSize(); i++)
    {
        if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN)
        {
            m_nCnxPolicy |= NW_REPORT_RADIO_LTE;
        }
        else if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN)
        {
            m_nCnxPolicy |= NW_REPORT_RADIO_WCDMA | NW_REPORT_RADIO_HSPA;
        }
        else if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN)
        {
            m_nCnxPolicy |= NW_REPORT_RADIO_GSM | NW_REPORT_RADIO_EDGE;
        }
        else if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN)
        {
            m_nCnxPolicy |= NW_REPORT_RADIO_WLAN;
        }
    }
}

PRIVATE
void AosNetTracker::InitRoamingCnxPolicy(IN ImsVector<IMS_SINT32>& objRoamingRats)
{
    for (int i = 0; i < objRoamingRats.GetSize(); i++)
    {
        if (objRoamingRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN)
        {
            m_nCnxPolicyInRoaming |= NW_REPORT_RADIO_NR;
        }
        else if (objRoamingRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN)
        {
            m_nCnxPolicyInRoaming |= NW_REPORT_RADIO_LTE;
        }
        else if (objRoamingRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN)
        {
            m_nCnxPolicyInRoaming |= NW_REPORT_RADIO_WCDMA | NW_REPORT_RADIO_HSPA;
        }
        else if (objRoamingRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN)
        {
            m_nCnxPolicyInRoaming |= NW_REPORT_RADIO_GSM | NW_REPORT_RADIO_EDGE;
        }
        else if (objRoamingRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN)
        {
            m_nCnxPolicyInRoaming |= NW_REPORT_RADIO_WLAN;
        }
    }
}

PRIVATE
void AosNetTracker::InitObject()
{
    A_IMS_TRACE_D(CNXID, "InitObject", 0, 0, 0);

    m_pUtil = AosUtil::GetInstance();

    m_piNetWatcherInfo = PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_nSlotId);
    if (m_piNetWatcherInfo != IMS_NULL)
    {
        m_piNetWatcherInfo->RegisterObserver(this);
        if (IsRoamingAccessPolicyRequired())
        {
            m_bIsRoaming = m_piNetWatcherInfo->GetRoamingState() ? IMS_TRUE : IMS_FALSE;
            A_IMS_TRACE_I(
                    CNXID, "InitObject :: m_bIsRoaming(%s)", m_bIsRoaming ? "TRUE" : "FALSE", 0, 0);
        }
    }

    if (IsCnxTypeEqual(NetworkPolicy::APN_IMS) || IsCnxTypeEqual(NetworkPolicy::APN_EMERGENCY))
    {
        if (IsCnxTypeEqual(NetworkPolicy::APN_IMS))
        {
            m_piConnection->SetListener(this);
        }
        UpdateWifiObserver();
    }
    else if (IsCnxTypeEqual(NetworkPolicy::APN_WIFI))
    {
        A_IMS_TRACE_I(CNXID, "InitObject :: apn is wifi", 0, 0, 0);
        IMS_EVENT_SendEventForSlotId(IMS_EVENT_WIFI_SERVICE, IMS_WIFI_ON, 0, m_nSlotId);
    }
}

PRIVATE
IMS_BOOL AosNetTracker::UpdateNetworkStatus()
{
    IMS_BOOL bOldNetAvailable = m_bIsNetAvailable;
    IMS_UINT32 nOldRadioType = m_nNetRadioType;
    IMS_UINT32 nOldVoiceRadioType = m_nNetVoiceRadioType;

    A_IMS_TRACE_I(CNXID, "Old Status :: service(%s), radio(%s), availability(%s)",
            ServiceTypeToString(m_nNetServiceType), RadioTypeToString(m_nNetRadioType),
            (m_bIsNetAvailable) ? "IN" : "OUT");

    GetStatus(m_nNetServiceType, m_nNetRadioType, m_bIsNetAvailable);

    if (m_piNetWatcherInfo != IMS_NULL)
    {
        m_nNetVoiceRadioType = m_piNetWatcherInfo->GetNetVoiceRadioTechType();
    }

    IMS_BOOL bIsChanged = (bOldNetAvailable != m_bIsNetAvailable) ||
            (nOldRadioType != m_nNetRadioType) || (nOldVoiceRadioType != m_nNetVoiceRadioType);

    return bIsChanged;
}

PRIVATE
void AosNetTracker::Notify()
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosNetTrackerListener* pListener = m_objListeners.GetAt(i);

        if (pListener == IMS_NULL)
        {
            continue;
        }

        pListener->NetTracker_StatusChanged();
    }
}

PRIVATE
void AosNetTracker::GetStatus(
        OUT IMS_SINT32& nService, OUT IMS_UINT32& nRadioTech, OUT IMS_BOOL& bIsIn)
{
    nService = m_piNetWatcherInfo->GetNetServiceType(
            AString::ConstNull(), m_piConnection->GetConnectionType());
    nRadioTech = m_piNetWatcherInfo->GetNetRadioTechType(
            AString::ConstNull(), m_piConnection->GetConnectionType());

    bIsIn = (IsServiceAvailable(GetAccessPolicy(), nService) &&
                    IsRadioTechAvailable(GetAccessPolicy(), nRadioTech))
            ? IMS_TRUE
            : IMS_FALSE;

    A_IMS_TRACE_I(CNXID, "GetStatus :: service(%s), radio(%s), availability(%s)",
            ServiceTypeToString(nService), RadioTypeToString(nRadioTech), (bIsIn) ? "IN" : "OUT");
}

PRIVATE
IMS_UINT32 AosNetTracker::GetAccessPolicy() const
{
    if (IsRoamingAccessPolicyRequired())
    {
        A_IMS_TRACE_I(CNXID, "GetAccessPolicy :: m_bIsRoaming(%s)", m_bIsRoaming ? "TRUE" : "FALSE",
                0, 0);
        return m_bIsRoaming ? m_nCnxPolicyInRoaming : m_nCnxPolicy;
    }

    return m_nCnxPolicy;
}

PRIVATE
void AosNetTracker::ProcessNetworkChanged(IMS_SINT32 nReason)
{
    IMS_SINT32 nCurrService = NW_REPORT_SRV_NOSRV;
    IMS_UINT32 nCurrRat = NW_REPORT_RADIO_NOSRV;
    IMS_BOOL bCurrIN = IMS_FALSE;
    IMS_BOOL bNotify = IMS_FALSE;

    GetStatus(nCurrService, nCurrRat, bCurrIN);

    if ((nReason != REASON_ROAMING_SATAE_CHANGED) && !IsNetworkChanged(nCurrRat, nCurrService))
    {
        return;
    }

    // Service Change
    if (nCurrService != m_nNetServiceType)
    {
        m_nNetServiceType = nCurrService;
    }

    // Rat Change
    if (m_pUtil->IsFeatureOn(FEATURE_RAT_GUARD, m_nFeature))
    {
        if (m_nNetRadioType != nCurrRat)
        {
            if (m_nChangingRat != nCurrRat)
            {
                m_nChangingRat = nCurrRat;
                StartTimer(TIMER_RAT_GUARD, m_nRatTime);
            }
        }
        else
        {
            m_nChangingRat = nCurrRat;
            StopTimer(TIMER_RAT_GUARD);
        }
    }
    else
    {
        if (m_nNetRadioType != nCurrRat)
        {
            m_nNetRadioType = nCurrRat;
            bNotify = IMS_TRUE;
        }
    }

    // In/Out Change
    if (!m_pUtil->IsFeatureOn(FEATURE_IN_GUARD, m_nFeature) &&
            !m_pUtil->IsFeatureOn(FEATURE_OUT_GUARD, m_nFeature))
    {
        if (bCurrIN != m_bIsNetAvailable)
        {
            m_bIsNetAvailable = bCurrIN;
            bNotify = IMS_TRUE;
        }
    }
    else
    {
        if (bCurrIN != m_bIsNetAvailable)
        {
            if (m_bIsNetAvailable)
            {
                // IN -> OUT
                if (m_pUtil->IsFeatureOn(FEATURE_OUT_GUARD, m_nFeature))
                {
                    if (m_piServiceOutTimer == IMS_NULL)
                    {
                        StartTimer(TIMER_OUT_GUARD, m_nServiceOutTime);
                    }
                }
                else
                {
                    m_bIsNetAvailable = bCurrIN;
                    bNotify = IMS_TRUE;
                }
            }
            else
            {
                // OUT -> IN
                if (m_pUtil->IsFeatureOn(FEATURE_IN_GUARD, m_nFeature))
                {
                    if (m_piServiceInTimer == IMS_NULL)
                    {
                        StartTimer(TIMER_IN_GUARD, m_nServiceInTime);
                    }
                }
                else
                {
                    m_bIsNetAvailable = bCurrIN;
                    bNotify = IMS_TRUE;
                }
            }
        }
        else
        {
            StopTimer(TIMER_IN_GUARD);
            StopTimer(TIMER_OUT_GUARD);
        }
    }

    if (bNotify)
    {
        Notify();
    }
}

PRIVATE
void AosNetTracker::ProcessVoiceNetworkChanged()
{
    IMS_UINT32 nCurrVoiceRat = (m_piNetWatcherInfo != IMS_NULL)
            ? m_piNetWatcherInfo->GetNetVoiceRadioTechType()
            : NW_REPORT_RADIO_NOSRV;

    A_IMS_TRACE_I(CNXID, "nCurrVoiceRat (%s)", RadioTypeToString(nCurrVoiceRat), 0, 0);

    if (m_pUtil->IsFeatureOn(FEATURE_VOICE_RAT_GUARD, m_nFeature))
    {
        if (nCurrVoiceRat == m_nNetVoiceRadioType)
        {
            m_nNetChangingVoiceRadioType = nCurrVoiceRat;
            StopTimer(TIMER_VOICE_RAT_GUARD);
        }
        else
        {
            if (m_nNetChangingVoiceRadioType == nCurrVoiceRat)
            {
                return;
            }

            m_nNetChangingVoiceRadioType = nCurrVoiceRat;
            StartTimer(TIMER_VOICE_RAT_GUARD, m_nVoiceRatGuardTime);
        }
    }
    else
    {
        // If it doesn't support gaurding timer, update and notify the change.
        if (nCurrVoiceRat != m_nNetVoiceRadioType)
        {
            m_nNetVoiceRadioType = nCurrVoiceRat;
            Notify();
        }
    }
}

PRIVATE
IMS_BOOL AosNetTracker::IsRadioTechAvailable(IN IMS_UINT32 nPolicy, IN IMS_UINT32 nRadioTech)
{
    return (nPolicy & nRadioTech & 0xffff0000);
}

PRIVATE
IMS_BOOL AosNetTracker::IsServiceAvailable(IN IMS_UINT32 nPolicy, IN IMS_UINT32 nService)
{
    return (nPolicy & nService & 0x000000ff);
}

PRIVATE
IMS_BOOL AosNetTracker::IsNetworkChanged(IN IMS_UINT32 nCurrRat, IN IMS_SINT32 nCurrService)
{
    if (m_pUtil->IsFeatureOn(FEATURE_RAT_GUARD, m_nFeature))
    {
        if ((m_nNetServiceType == nCurrService) && (m_nNetRadioType == nCurrRat) &&
                (m_nChangingRat == nCurrRat))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if ((m_nNetServiceType == nCurrService) && (m_nNetRadioType == nCurrRat))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AosNetTracker::IsDataConnected() const
{
    return m_bIsDataConnected;
}

PRIVATE
IMS_BOOL AosNetTracker::IsEpdgEnabled() const
{
    return m_bIsEpdgEnabled;
}

PRIVATE
IMS_BOOL AosNetTracker::IsCnxTypeEqual(IN IMS_SINT32 nType) const
{
    return (m_piConnection->GetConnectionType() == nType);
}

PRIVATE
IMS_BOOL AosNetTracker::IsWlanEnabled() const
{
    return (GetAccessPolicy() & NW_REPORT_RADIO_WLAN);
}

PRIVATE
IMS_BOOL AosNetTracker::IsWifiConnected() const
{
    return m_bIsWifiConnected;
}

PRIVATE
IMS_BOOL AosNetTracker::IsVonrSupported()
{
    return m_piAosNConfig->IsImsOverNrEnabled();
}

PRIVATE
IMS_BOOL AosNetTracker::IsRoamingAccessPolicyRequired() const
{
    return ((m_piAosNConfig->GetSupportedRoamingRats()).GetSize() > 0);
}

PRIVATE
void AosNetTracker::SetDataConnected(IN IMS_BOOL bConnected)
{
    if (m_bIsDataConnected != bConnected)
    {
        A_IMS_TRACE_I(CNXID, "SetDataConnected :: connected (%s)", _TRACE_B_(bConnected), 0, 0);
    }

    m_bIsDataConnected = bConnected;
}

PRIVATE
void AosNetTracker::SetEpdgEnabled(IN IMS_BOOL bEnabled)
{
    if (m_bIsEpdgEnabled != bEnabled)
    {
        A_IMS_TRACE_I(CNXID, "SetEpdgEnabled :: enabled (%s)", _TRACE_B_(bEnabled), 0, 0);
    }

    m_bIsEpdgEnabled = bEnabled;
}

PRIVATE
void AosNetTracker::SetWifiConnected(IN IMS_BOOL bConnected)
{
    if (m_bIsWifiConnected != bConnected)
    {
        A_IMS_TRACE_I(CNXID, "SetWifiConnected :: connected (%s)", _TRACE_B_(bConnected), 0, 0);
    }

    m_bIsWifiConnected = bConnected;
}

PRIVATE
void AosNetTracker::UpdateWifiObserver()
{
    IMS_SINT32 nCnxType = m_piConnection->GetConnectionType();
    if ((nCnxType != NetworkPolicy::APN_IMS) && (nCnxType != NetworkPolicy::APN_EMERGENCY))
    {
        return;
    }

    m_piWifiWatcher = PhoneInfoService::GetPhoneInfoService()->GetWifiWatcher();
    if (m_piWifiWatcher == IMS_NULL)
    {
        A_IMS_TRACE_I(CNXID, "can not get WifiWatcher", 0, 0, 0);
        return;
    }

    if (IsWlanEnabled())
    {
        A_IMS_TRACE_I(CNXID, "register WiFi status observer", 0, 0, 0);
        m_piWifiWatcher->RegisterObserver(this);
        IMS_EVENT_SendEventForSlotId(IMS_EVENT_WIFI_SERVICE, IMS_WIFI_ON, 0, m_nSlotId);
    }
    else
    {
        A_IMS_TRACE_I(CNXID, "remove WiFi status observer", 0, 0, 0);
        m_piWifiWatcher->RemoveObserver(this);
    }
}

PRIVATE
void AosNetTracker::ProcessInTimerExpired()
{
    StopTimer(TIMER_IN_GUARD);

    m_bIsNetAvailable = IMS_TRUE;
    Notify();
}

PRIVATE
void AosNetTracker::ProcessOutTimerExpired()
{
    StopTimer(TIMER_OUT_GUARD);
    m_bIsNetAvailable = IMS_FALSE;
    Notify();
}

PRIVATE
void AosNetTracker::ProcessRatTimerExpired()
{
    StopTimer(TIMER_RAT_GUARD);

    m_nNetRadioType = m_nChangingRat;
    Notify();
}

PRIVATE
void AosNetTracker::ProcessVoiceRatTimerExpired()
{
    StopTimer(TIMER_VOICE_RAT_GUARD);

    m_nNetVoiceRadioType = m_nNetChangingVoiceRadioType;
    Notify();
}

PRIVATE
void AosNetTracker::ClearTimers()
{
    StopTimer(TIMER_IN_GUARD);
    StopTimer(TIMER_OUT_GUARD);
    StopTimer(TIMER_RAT_GUARD);
    StopTimer(TIMER_VOICE_RAT_GUARD);
}

PRIVATE
void AosNetTracker::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_IN_GUARD:
            ppiTimer = &m_piServiceInTimer;
            break;

        case TIMER_OUT_GUARD:
            ppiTimer = &m_piServiceOutTimer;
            break;

        case TIMER_RAT_GUARD:
            ppiTimer = &m_piRatTimer;
            break;

        case TIMER_VOICE_RAT_GUARD:
            ppiTimer = &m_piVoiceRatTimer;
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

PRIVATE
void AosNetTracker::StopTimer(IN IMS_UINT32 nType)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_IN_GUARD:
            ppiTimer = &m_piServiceInTimer;
            break;

        case TIMER_OUT_GUARD:
            ppiTimer = &m_piServiceOutTimer;
            break;

        case TIMER_RAT_GUARD:
            ppiTimer = &m_piRatTimer;
            break;

        case TIMER_VOICE_RAT_GUARD:
            ppiTimer = &m_piVoiceRatTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer == IMS_NULL)
    {
        return;
    }

    m_pUtil->StopTimer(*ppiTimer, TimerToString(nType));
}

PRIVATE VIRTUAL void AosNetTracker::AosConnection_StateChanged(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_D(CNXID, "AoSConnection_StateChanged :: nState (%d)", nState, 0, 0);

    IMS_BOOL bCurrConnected = (nState == IAosConnection::STATE_ACTIVE) ? IMS_TRUE : IMS_FALSE;

    if (IsDataConnected() != bCurrConnected)
    {
        SetDataConnected(bCurrConnected);

        // check the epdg connection
        if (bCurrConnected)
        {
            SetEpdgEnabled(m_piConnection->IsEpdgEnabled());
        }

        Notify();
    }
}

PRIVATE VIRTUAL void AosNetTracker::AosConnection_IpChanged() {}

PRIVATE VIRTUAL void AosNetTracker::AosConnection_IpcanCatChanged()
{
    IMS_BOOL bEpdgEnabled = m_piConnection->IsEpdgEnabled();

    if (IsEpdgEnabled() == bEpdgEnabled)
    {
        return;
    }

    SetEpdgEnabled(bEpdgEnabled);

    Notify();
}

PRIVATE VIRTUAL void AosNetTracker::AosConnection_PcscfChanged() {}

PRIVATE VIRTUAL void AosNetTracker::AosConnection_ConnectionFailed() {}

PRIVATE VIRTUAL void AosNetTracker::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piServiceInTimer)
    {
        ProcessInTimerExpired();
        return;
    }

    if (piTimer == m_piServiceOutTimer)
    {
        ProcessOutTimerExpired();
        return;
    }

    if (piTimer == m_piRatTimer)
    {
        ProcessRatTimerExpired();
        return;
    }

    if (piTimer == m_piVoiceRatTimer)
    {
        ProcessVoiceRatTimerExpired();
        return;
    }
}

PRIVATE
AString AosNetTracker::FeaturesToString()
{
    AString strFeature = "| ";

    if (m_nFeature & FEATURE_IN_GUARD)
        strFeature += "FEATURE_IN_GUARD | ";

    if (m_nFeature & FEATURE_OUT_GUARD)
        strFeature += "FEATURE_OUT_GUARD | ";

    if (m_nFeature & FEATURE_RAT_GUARD)
        strFeature += "FEATURE_RAT_GUARD | ";

    if (m_nFeature & FEATURE_VOICE_RAT_GUARD)
        strFeature += "FEATURE_VOICE_RAT_GUARD | ";

    return strFeature;
}

PRIVATE GLOBAL const IMS_CHAR* AosNetTracker::RadioTypeToString(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        case (0x00010000 << 0):
            return "NW_REPORT_RADIO_NOSRV";

        case (0x00010000 << 1):
            return "NW_REPORT_RADIO_AMPS";

        case (0x00010000 << 2):
            return "NW_REPORT_RADIO_CDMA";

        case (0x00010000 << 3):
            return "NW_REPORT_RADIO_GSM";

        case (0x00010000 << 4):
            return "NW_REPORT_RADIO_HDR";

        case (0x00010000 << 5):
            return "NW_REPORT_RADIO_WCDMA";

        case (0x00010000 << 6):
            return "NW_REPORT_RADIO_GPS";

        case (0x00010000 << 7):
            return "NW_REPORT_RADIO_EDGE";

        case (0x00010000 << 8):
            return "NW_REPORT_RADIO_WLAN";

        case (0x00010000 << 10):
            return "NW_REPORT_RADIO_EVDODO";

        case (0x00010000 << 12):
            return "NW_REPORT_RADIO_EHRPD";

        case (0x00010000 << 13):
            return "NW_REPORT_RADIO_LTE";

        case (0x00010000 << 14):
            return "NW_REPORT_RADIO_HSPA";

        case NW_REPORT_RADIO_NR:
            return "NW_REPORT_RADIO_NR";

        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* AosNetTracker::ServiceTypeToString(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        case (0x01 << 0):
            return "NW_REPORT_SRV_NOSRV";

        case (0x01 << 1):
            return "NW_REPORT_SRV_LIMITED";

        case (0x01 << 2):
            return "NW_REPORT_SRV_SRV";

        case (0x01 << 3):
            return "NW_REPORT_SRV_LIMITEDREGION";

        case (0x01 << 4):
            return "NW_REPORT_SRV_PWRSAVE";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosNetTracker::TimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TIMER_IN_GUARD:
            return "TIMER_IN_GUARD";

        case TIMER_OUT_GUARD:
            return "TIMER_OUT_GUARD";

        case TIMER_RAT_GUARD:
            return "TIMER_RAT_GUARD";

        case TIMER_VOICE_RAT_GUARD:
            return "TIMER_VOICE_RAT_GUARD";

        default:
            return "__INVALID__";
    }
}
