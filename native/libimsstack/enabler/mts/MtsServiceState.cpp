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

#include "CarrierConfig.h"
#include "ServiceConfig.h"
#include "ServiceImsRadio.h"
#include "ServiceTimer.h"
#include "IImsAos.h"
#include "IImsAosInfo.h"
#include "ImsAos.h"
#include "ImsAosParameter.h"
#include "MtsStringDef.h"
#include "MtsServiceState.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsServiceState::MtsServiceState(IN IMS_SINT32 nSlotId) :
        m_nMtsServiceState(STATE_INIT),
        m_bImsConnected(IMS_FALSE),
        m_bAosRegModAdmin(IMS_FALSE),
        m_bImsSuspend(IMS_FALSE),
        m_bSmsOverIpConf(IMS_FALSE),
        m_bTemporaryBlocked(IMS_FALSE),
        m_nConnectedServices(ImsAosService::NONE),
        m_nSlotId(nSlotId),
        m_piImsRadio(IMS_NULL),
        m_piEmergencyRadioGuardTimer(IMS_NULL),
        m_piRadioGuardTimer(IMS_NULL)
{
    IMS_TRACE_I("+MtsServiceState [slot_%d]", m_nSlotId, 0, 0);

    Init();
}

PUBLIC
MtsServiceState::~MtsServiceState()
{
    IMS_TRACE_I("~MtsServiceState [slot_%d]", m_nSlotId, 0, 0);

    DeInit();
}

PUBLIC void MtsServiceState::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_I("Timer_TimerExpired", 0, 0, 0);

    if (piTimer == IMS_NULL)
    {
        return;
    }
    else if (piTimer == m_piRadioGuardTimer)
    {
        StopImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS);
    }
    else if (piTimer == m_piEmergencyRadioGuardTimer)
    {
        StopImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS);
    }
    else
    {
        IMS_TRACE_I("Timer_TimerExpired : can't find the expired timer", 0, 0, 0);
        return;
    }

    StopRadioGuardTimer(piTimer);
}

PUBLIC
void MtsServiceState::StartRadioGuardTimer(IN IMS_UINT32 nTrafficType)
{
    IMS_TRACE_I("StartRadioGuardTimer : nTrafficType[%s]", PS_TrafficType(nTrafficType), 0, 0);

    if (nTrafficType == IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS)
    {
        if (m_piEmergencyRadioGuardTimer != IMS_NULL)
        {
            TimerService::GetTimerService()->DestroyTimer(m_piEmergencyRadioGuardTimer);
        }

        m_piEmergencyRadioGuardTimer = TimerService::GetTimerService()->CreateTimer();
        m_piEmergencyRadioGuardTimer->SetTimer(MTS_RADIO_GUARD_TIME, this);
    }
    else
    {
        if (m_piRadioGuardTimer != IMS_NULL)
        {
            TimerService::GetTimerService()->DestroyTimer(m_piRadioGuardTimer);
        }

        m_piRadioGuardTimer = TimerService::GetTimerService()->CreateTimer();
        m_piRadioGuardTimer->SetTimer(MTS_RADIO_GUARD_TIME, this);
    }
}

PUBLIC
IMS_BOOL MtsServiceState::IsRadioGuardTimerActive(IN IMS_UINT32 nTrafficType)
{
    IMS_BOOL bResult;

    if (nTrafficType == IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS)
    {
        bResult = (m_piEmergencyRadioGuardTimer != IMS_NULL) ? IMS_TRUE : IMS_FALSE;
    }
    else
    {
        bResult = (m_piRadioGuardTimer != IMS_NULL) ? IMS_TRUE : IMS_FALSE;
    }

    IMS_TRACE_I("IsRadioGuardTimerActive : bResult[%s]", _TRACE_B_(bResult), 0, 0);

    return bResult;
}

PUBLIC
void MtsServiceState::SetImsRegConnected(IN IMS_BOOL bConnected)
{
    IMS_UINT32 nType = IImsAosInfo::REG_MODE_UNKNOWN;

    IMS_TRACE_I("SetImsRegConnected : m_bImsConnected[%s]/bConnected[%s]",
            _TRACE_B_(m_bImsConnected), _TRACE_B_(bConnected), 0);

    if (m_bImsConnected == bConnected)
    {
        return;
    }

    m_bImsConnected = bConnected;

    if (m_bImsConnected)
    {
        IImsAos* piImsAos =
                ImsAos::GetImsAos(AString("ims.app.mts"), AString("ims.service.mts"), m_nSlotId);

        if (piImsAos == IMS_NULL)
        {
            IMS_TRACE_E(0, "Fail to get AoSApp", 0, 0, 0);
            return;
        }

        nType = piImsAos->GetAosInfo()->GetRegistrationMode();

        if (IImsAosInfo::REG_MODE_ADMIN == nType)
        {
            m_bAosRegModAdmin = IMS_TRUE;
        }
        else
        {
            m_bAosRegModAdmin = IMS_FALSE;

            if (IImsAosInfo::REG_MODE_UNKNOWN == nType)
            {
                IMS_TRACE_I("IMS Reg Mod is UNKNOWN!!", 0, 0, 0);
            }
        }
    }

    IMS_TRACE_I("SetImsRegConnected : IMS Reg State [%s], IMS Admin Reg [%s]",
            _TRACE_B_(m_bImsConnected), _TRACE_B_(m_bAosRegModAdmin), 0);

    UpdateServiceState();
}

PUBLIC
void MtsServiceState::SetSmsOverIpState(IN IMS_BOOL bState)
{
    if (m_bSmsOverIpConf == bState)
    {
        return;
    }

    m_bSmsOverIpConf = bState;

    IMS_TRACE_I("SetSmsOverIpState : Sms Over IP Network State is [%s]",
            _TRACE_B_(m_bSmsOverIpConf), 0, 0);

    UpdateServiceState();
}

PUBLIC
IMS_BOOL MtsServiceState::IsServiceConnected(IN IMS_UINT32 nService)
{
    return (m_nConnectedServices & nService) != 0;
}

PUBLIC
void MtsServiceState::SetConnectedServices(IN IMS_UINT32 nServices)
{
    m_nConnectedServices = nServices;
}

PUBLIC
void MtsServiceState::OnImsConnected()
{
    IMS_TRACE_I("OnImsConnected", 0, 0, 0);

    SetImsRegConnected(IMS_TRUE);
}

PUBLIC
void MtsServiceState::OnImsDisconnected(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("OnImsDisconnected : Reason is (%d)", nReason, 0, 0);

    SetImsSuspendState(IMS_FALSE);
    SetImsRegConnected(IMS_FALSE);
}

PUBLIC
void MtsServiceState::OnImsDisconnecting(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("OnImsDisconnecting : Reason is (%d)", nReason, 0, 0);
}

PUBLIC
void MtsServiceState::OnImsSuspended(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("OnImsSuspended : Reason is (%d)", nReason, 0, 0);
    SetImsSuspendState(IMS_TRUE);
}

PUBLIC
void MtsServiceState::OnImsResumed()
{
    IMS_TRACE_I("OnImsResumed", 0, 0, 0);

    SetImsSuspendState(IMS_FALSE);
}

PUBLIC
void MtsServiceState::NotifySpecificMessage(
        IN IMS_UINT32 nMsg, IN IMS_UINT32 nWparam, IN IMS_UINT32 nLparam)
{
    (void)nMsg;
    (void)nWparam;
    (void)nLparam;

    IMS_TRACE_I("NotifySpecificMessage", 0, 0, 0);
}

PUBLIC
IMS_SINT32 MtsServiceState::GetServiceState()
{
    IMS_SINT32 nState = STATE_NOTREADY;

    if (m_bImsConnected)
    {
        if (m_bImsSuspend || (!m_bSmsOverIpConf) || m_bAosRegModAdmin)
        {
            nState = STATE_LIMITED;
        }
        else
        {
            nState = STATE_READY;
        }
    }

    IMS_TRACE_I("GetServiceState : nState(%d)", nState, 0, 0);

    return nState;
}

PUBLIC
void MtsServiceState::UpdateServiceState()
{
    IMS_SINT32 nTempState = GetServiceState();

    IMS_TRACE_I("UpdateServiceState : nTempState(%d) m_nMtsServiceState(%d)", nTempState,
            m_nMtsServiceState, 0);

    if (nTempState != m_nMtsServiceState)
    {
        m_nMtsServiceState = nTempState;
    }
}

PUBLIC
IMS_BOOL MtsServiceState::IsMoServiceBlocked() const
{
    return (m_nMtsServiceState != STATE_READY);
}

PUBLIC
IMS_BOOL MtsServiceState::IsMtServiceBlocked() const
{
    return (m_nMtsServiceState == STATE_NOTREADY);
}

PUBLIC
IMS_BOOL MtsServiceState::IsTemporaryServiceBlocked() const
{
    return m_bTemporaryBlocked;
}

PUBLIC
IMS_BOOL MtsServiceState::IsImsTrafficAllowed(IN IMS_UINT32 nTrafficType)
{
    IMS_BOOL bResult = m_piImsRadio->IsImsTrafficAllowed(nTrafficType);

    IMS_TRACE_I("IsImsTrafficAllowed : nTrafficType[%s], bResult[%s]",
            PS_TrafficType(nTrafficType), _TRACE_B_(bResult), 0);

    return bResult;
}

PUBLIC
void MtsServiceState::StartImsTraffic(IN IMS_UINT32 nTrafficType, IN IMS_UINT32 nAccessNetworkType,
        IN IImsRadioConnectionListener* piListener)
{
    IMS_TRACE_I("StartImsTraffic : nTrafficType[%s]", PS_TrafficType(nTrafficType), 0, 0);

    m_piImsRadio->StartImsTraffic(nTrafficType, nAccessNetworkType, piListener);
}

PUBLIC
void MtsServiceState::TriggerEpsFallback(IN IMS_UINT32 nEpsfbReason)
{
    IMS_TRACE_I("TriggerEpsFallback", 0, 0, 0);

    m_piImsRadio->TriggerEpsFallback(nEpsfbReason);
}

PUBLIC
void MtsServiceState::AddListenerForTrafficPriority(
        IN IImsRadioTrafficPriorityListener* piListener)
{
    IMS_TRACE_I("AddListenerForTrafficPriority", 0, 0, 0);

    m_piImsRadio->AddListenerForTrafficPriority(piListener);
}

PUBLIC
void MtsServiceState::RemoveListenerForTrafficPriority(
        IN IImsRadioTrafficPriorityListener* piListener)
{
    IMS_TRACE_I("RemoveListenerForTrafficPriority", 0, 0, 0);

    m_piImsRadio->RemoveListenerForTrafficPriority(piListener);
}

PRIVATE
void MtsServiceState::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);

    m_piImsRadio = ImsRadioService::GetImsRadioService()->GetImsRadio(m_nSlotId);

    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    IMS_BOOL bSmsOverIpNetwork =
            piCc->GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL);

    // TODO(Mts): Check whether Mts should consider KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY
    IMSVector<IMS_SINT32> objSupportedRats =
            piCc->GetIntArray(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY);

    IMS_TRACE_I("GetSmOverIpConfigInfo : bSmsOverIpNetwork[%d]", bSmsOverIpNetwork, 0, 0);

    for (IMS_UINT32 i = 0; i < objSupportedRats.GetSize(); ++i)
    {
        IMS_SINT32 nValue = objSupportedRats.GetAt(i);
        IMS_TRACE_I("GetSmOverIpConfigInfo : objSupportedRats[%d][%d]", i, nValue, 0);
    }

    if (bSmsOverIpNetwork)
    {
        SetSmsOverIpState(IMS_TRUE);
    }
    else
    {
        SetSmsOverIpState(IMS_FALSE);
    }
}

PRIVATE
void MtsServiceState::DeInit()
{
    IMS_TRACE_I("DeInit", 0, 0, 0);

    if (m_piRadioGuardTimer != IMS_NULL)
    {
        TimerService::GetTimerService()->DestroyTimer(m_piRadioGuardTimer);
    }

    if (m_piEmergencyRadioGuardTimer != IMS_NULL)
    {
        TimerService::GetTimerService()->DestroyTimer(m_piRadioGuardTimer);
    }
}

PRIVATE
void MtsServiceState::SetImsSuspendState(IN IMS_BOOL bState)
{
    if (m_bImsSuspend == bState)
    {
        return;
    }

    m_bImsSuspend = bState;  // if IMSAoSApp_OnImsSuspended. block mo service

    IMS_TRACE_I("SetImsSuspendState : IMS Suspend State is [%s]", _TRACE_B_(m_bImsSuspend), 0, 0);

    UpdateServiceState();
}

PRIVATE
void MtsServiceState::SetMtsServiceState(IN IMS_SINT32 nServiceState)
{
    m_nMtsServiceState = nServiceState;
}

PRIVATE
void MtsServiceState::SetTemporaryServiceBlocked(IN IMS_BOOL bBlocked)
{
    m_bTemporaryBlocked = bBlocked;

    IMS_TRACE_I("SetTemporaryServiceBlocked : Service Blocked State is [%s]",
            _TRACE_B_(m_bTemporaryBlocked), 0, 0);
}

PRIVATE
void MtsServiceState::StopImsTraffic(IN IMS_UINT32 nTrafficType)
{
    IMS_TRACE_I("StopImsTraffic : nTrafficType[%s]", PS_TrafficType(nTrafficType), 0, 0);

    m_piImsRadio->StopImsTraffic(nTrafficType);
}

PRIVATE
void MtsServiceState::StopRadioGuardTimer(IN ITimer* piTimer)
{
    if (m_piEmergencyRadioGuardTimer == piTimer)
    {
        IMS_TRACE_I("StopRadioGuardTimer : m_piEmergencyRadioGuardTimer", 0, 0, 0);
        m_piEmergencyRadioGuardTimer = IMS_NULL;
    }
    else if (m_piRadioGuardTimer == piTimer)
    {
        IMS_TRACE_I("StopRadioGuardTimer : m_piRadioGuardTimer", 0, 0, 0);
        m_piRadioGuardTimer = IMS_NULL;
    }

    piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(piTimer);
}
