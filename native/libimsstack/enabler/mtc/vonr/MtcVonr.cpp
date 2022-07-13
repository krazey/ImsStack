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

#include "INetworkConnection.h"
#include "INetworkWatcher.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServiceNetworkPolicy.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
//#include "ServiceVoNr.h"

#include "call/IMtcCallContext.h"
#include "call/IMtcCall.h"
#include "IuMtcService.h"
#include "MtcDef.h"
#include "vonr/MtcVonr.h"
#include "vonr/IMtcVonrListener.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcVonr::MtcVonr(IN IMS_UINT32 nSlotId, IN IMtcVonrListener* piListener) :
        m_nSlotId(nSlotId),
        m_piListener(piListener),
        // m_piVonr(IMS_NULL),
        m_nCallKey(IMtcCall::CALL_KEY_INVALID),
        m_nDirection(0 /*IVoNr::DIRECTION_MO*/),
        m_nUacType(0 /*IVoNr::TYPE_VOICE*/),
        m_piNetWatcherInfo(IMS_NULL),
        m_eUacStatus(UacStatus::IDLE),
        m_nCurrentNetwork(INetworkWatcher::RADIOTECH_TYPE_INVALID),
        m_piTimer(IMS_NULL)
{
    IMS_TRACE_I("+MtcVonr", 0, 0, 0);
    Initialize();
}

PUBLIC VIRTUAL MtcVonr::~MtcVonr()
{
    IMS_TRACE_I("~MtcVonr : [%" PFLS_x "]", this, 0, 0);

/*
    if (m_piVonr)
    {
        m_piVonr->RemoveListenerForUac(this);
        m_piVonr->RemoveListenerForCallPreference(this);
        m_piVonr->RemoveListenerForHandoff(this);
    }
*/
    if (m_piNetWatcherInfo)
    {
        m_piNetWatcherInfo->RemoveObserver(this);
    }

    StopTimer();
}

#if _IVONR_LISTENER_INTERFACE_
#endif

PUBLIC VIRTUAL void MtcVonr::VoNrUac_NotifyResponse(IN IMS_UINT32 nType, IN IMS_RESULT nResult,
        IN IMS_SINT32 nReason, IN IMS_UINT32 nSysMode, IN IMS_UINT32 nBarringTime)
{
    OnNotifyUacResponse(nType, nResult, nReason, nSysMode, nBarringTime);
}

PUBLIC VIRTUAL void MtcVonr::VoNrCallPreference_NotifyCallReady(IN IMS_UINT32 nSysMode)
{
    OnNotifyCallPreferenceReady(nSysMode);
}

PUBLIC VIRTUAL void MtcVonr::VoNrHandoff_NotifyInformation(IN IMS_UINT32 nStatus,
        IN IMS_UINT32 nSourceRAT, IN IMS_UINT32 nTargetRAT, IN IMS_SINT32 nReason)
{
    (void)nStatus;
    (void)nSourceRAT;
    (void)nTargetRAT;
    (void)nReason;
    // TODO : If you need the implement for "WLAN to 5G NR Handover case" ...
}

#if _INETWATCHER_LISTENER_INTERFACE_
#endif

PUBLIC VIRTUAL void MtcVonr::NetworkWatcher_NotifyStatus(IN INetworkWatcher* piNetWatcherInfo)
{
    if (m_piNetWatcherInfo != piNetWatcherInfo)
    {
        IMS_TRACE_D("NotifyNetWatcherStatus - INetWatcherInfo is not matched", 0, 0, 0);
        return;
    }

    IMS_SINT32 nReportedNetwork = m_piNetWatcherInfo->GetNetworkType();
    /*
        IMtcCall* piMtcCall = GetCall();
        if (piMtcCall &&
                piMtcCall->GetState() == IMtcCall::RINGBACK &&
                m_nCurrentNetwork != nReportedNetwork &&
                IsAvailableNetwork(nReportedNetwork) &&
                m_eUacStatus == UacStatus::SUCCESS)
        {
            IMS_TRACE_D("NotifyNetWatcherStatus [%d]", nReportedNetwork, 0, 0);
            NotifyCallState(IVoNr::STATE_START);
        }
    */
    m_nCurrentNetwork = nReportedNetwork;
}

#if _ITIMER_LISTENER_INTERFACE_
#endif

PUBLIC VIRTUAL void MtcVonr::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_D("Timer_TimerExpired status[%d]", m_eUacStatus, 0, 0);

    if (m_piTimer != piTimer)
    {
        return;
    }

    StopTimer();

    if (m_eUacStatus == UacStatus::FAILURE)
    {
        IMtcCall* piMtcCall = GetCall();
        if (piMtcCall == IMS_NULL)
        {
            return;
        }
        /*
                IMS_UINT32 nBlockType = IMtcCallContext::VONR_BLOCK_TYPE_FINAL_FAILURE;
                if (GetSysMode() == IVoNr::SYS_MODE_NR5G)
                {
                    // call drop
                    nBlockType = IMtcCallContext::VONR_BLOCK_TYPE_FINAL_FAILURE_ABNORMAL;
                }

                piMtcCall->SetUacBlockType(nBlockType);
        */
    }
    else
    {
        // temp policy : treat as success. in success case, other information(params) don't matter.
        VoNrUac_NotifyResponse(m_nUacType, IMS_SUCCESS, 0, 0/*IVoNr::SYS_MODE_NR5G*/, 0);
    }
}

#if _PUBLIC_METHOD_
#endif

PUBLIC VIRTUAL void MtcVonr::CheckBarring(
        IN IMtcCall* /*piMtcCall*/, IN CallType /*eCallType*/, IN IMS_BOOL /*bEmergency*/)
{
    // must override.
}

PUBLIC
IMS_UINT32 MtcVonr::GetUacType()
{
    return m_nUacType;
}

#if _PROTECTED_METHOD_
#endif

PROTECTED VIRTUAL void MtcVonr::OnSessionStopped(IN IMS_UINTP /*nParam*/)
{
    // must override.
}

PROTECTED VIRTUAL void MtcVonr::NotifyCallState(IN IMS_UINT32 /*nState*/)
{
    // must override.
}

PROTECTED VIRTUAL void MtcVonr::OnNotifyUacResponse(IN IMS_UINT32 /*nType*/,
        IN IMS_RESULT /*nResult*/, IN IMS_SINT32 /*nReason*/, IN IMS_UINT32 /*nSysMode*/,
        IN IMS_UINT32 /*nBarringTime*/)
{
    // must override.
}

PROTECTED VIRTUAL void MtcVonr::OnNotifyCallPreferenceReady(IN IMS_UINT32 /*nSysMode*/)
{
    // must override.
}

PROTECTED VIRTUAL IMS_BOOL MtcVonr::IsUacCheckRequired()
{
    // must override.
    return IMS_TRUE;
}

PROTECTED
void MtcVonr::Initialize()
{
    IMS_TRACE_D("Initialize", 0, 0, 0);
/*
    m_piVonr = VoNrService::GetVoNrService()->GetVoNr(m_nSlotId);
    m_piVonr->AddListenerForUac(this);
    m_piVonr->AddListenerForCallPreference(this);
    m_piVonr->AddListenerForHandoff(this);
*/
    m_piNetWatcherInfo = PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_nSlotId);
    m_piNetWatcherInfo->RegisterObserver(this);
}

PROTECTED
void MtcVonr::RequestCallPreference(IN IMS_UINT32 nRat)
{
    IMS_TRACE_I("RequestCallPreference - nRat[%d]", nRat, 0, 0);

    if (IMS_TRUE/*m_piVonr == IMS_NULL*/)
    {
        return;
    }

    StartTimer(TIME_WAIT_CALL_READY);
    //m_piVonr->RequestCallPreference(nRat, m_nUacType);
}

PROTECTED
IMS_RESULT MtcVonr::UpdateSessionInfo(IN IMtcCall* /*piMtcCall*/)
{
    IMS_TRACE_D("UpdateSessionInfo", 0, 0, 0);
    /*
        if (piMtcCallContext == IMS_NULL)
        {
            return IMS_FAILURE;
        }

        m_nCallKey = piMtcCallContext->GetKey();
        m_piCallList = piMtcCallContext->GetSessList();
        m_nDirection = GetConvertedDirection(piMtcCall->GetPeerType());
    */
    return IMS_SUCCESS;
}

#if _UTILITIES_
#endif

PROTECTED
void MtcVonr::SetUacType(IN CallType eCallType, IN IMS_BOOL bEmergency)
{
    IMS_TRACE_D("SetUacType - eCallType[%d]", eCallType, 0, 0);

    if (bEmergency)
    {
        m_nUacType = 0;//IVoNr::TYPE_EMERGENCY;
    }
    else if (eCallType == CallType::VOIP || eCallType == CallType::RTT)
    {
        m_nUacType = 0;//IVoNr::TYPE_VOICE;
    }
    else if (eCallType == CallType::VT || eCallType == CallType::VIDEO_RTT)
    {
        m_nUacType = 0;//IVoNr::TYPE_VIDEO;
    }
}

PROTECTED
IMS_UINT32 MtcVonr::GetSysMode()
{
    IMS_UINT32 nSysMode = 0;//IVoNr::SYS_MODE_UNKNOWN;
    IMS_UINT32 nRadioTechType = m_piNetWatcherInfo->GetNetRadioTechType();

    const INetworkConnection* piNetConnection =
            NetworkService::GetNetworkService()->FindConnection(NetworkPolicy::APN_IMS, m_nSlotId);
    IMS_BOOL bWLAN = (piNetConnection != IMS_NULL) ? piNetConnection->IsePDGEnabled() : IMS_FALSE;

    if (bWLAN)
    {
        // only working for MTK
        nSysMode = 0;//IVoNr::SYS_MODE_WLAN;
    }
    else
    {
        if (nRadioTechType == NW_REPORT_RADIO_NR)
        {
            nSysMode = 0;//IVoNr::SYS_MODE_NR5G;
        }
        else if (nRadioTechType == NW_REPORT_RADIO_LTE)
        {
            nSysMode = 0;//IVoNr::SYS_MODE_LTE;
        }
    }

    IMS_TRACE_I("GetSysMode - RadioTech[%d] SysMode[%d]", nRadioTechType, nSysMode, 0);
    return nSysMode;
}

PROTECTED
IMS_UINT32 MtcVonr::GetConvertedDirection(IN PeerType ePeerType)
{
    IMS_TRACE_D("GetConvertedDirection - ePeerType[%d]", ePeerType, 0, 0);

    if (ePeerType == PeerType::MT)
    {
        return 0;//IVoNr::DIRECTION_MT;
    }

    return 0;//IVoNr::DIRECTION_MO;
}

PROTECTED
MtcVonr::VonrInitType MtcVonr::GetConvertedInitType(IN IMS_UINT32 nSysMode)
{
    VonrInitType nInitType = VonrInitType::NONE;

    if (m_nUacType == 0/*IVoNr::IVoNr::TYPE_EMERGENCY*/)
    {
        nInitType = VonrInitType::EMERGENCY;
    }
    else
    {
        if (nSysMode == 0/*IVoNr::SYS_MODE_LTE*/)
        {
            nInitType = VonrInitType::LTE;
        }
        else if (nSysMode == 0/*IVoNr::SYS_MODE_NR5G*/)
        {
            nInitType = VonrInitType::NR;
        }
        else if (nSysMode == 0/*IVoNr::SYS_MODE_WLAN*/)
        {
            nInitType = VonrInitType::WIFI;
        }
    }

    IMS_TRACE_D("GetConvertedInitType [%d]", nInitType, 0, 0);
    return nInitType;
}

PROTECTED
IMtcCall* MtcVonr::GetCall()
{
    /*
    if (m_piCallList == IMS_NULL || m_nCallKey == -1)
    {
        return IMS_NULL;
    }

    return m_piCallList->GetWithSessionKey(m_nCallKey);
    */
    return IMS_NULL;
}

PROTECTED
IMS_BOOL MtcVonr::IsAvailableNetwork(IN IMS_SINT32 nRadiotechType)
{
    IMS_TRACE_D("IsAvailableNetwork - nReportedNetwork[%d]", nRadiotechType, 0, 0);

    IMS_BOOL bReturn = IMS_TRUE;

    if (nRadiotechType != INetworkWatcher::RADIOTECH_TYPE_LTE &&
            nRadiotechType != INetworkWatcher::RADIOTECH_TYPE_LTE_CA &&
            nRadiotechType != INetworkWatcher::RADIOTECH_TYPE_NR)
    {
        bReturn = IMS_FALSE;
    }

    return bReturn;
}

PROTECTED
void MtcVonr::StartTimer(IN IMS_UINT32 nTime)
{
    //---------------------------------------------------------------------------------------------
    StopTimer();

    m_piTimer = TimerService::GetTimerService()->CreateTimer();
    if (m_piTimer != IMS_NULL)
    {
        m_piTimer->SetTimer(nTime, this);
    }
}

PROTECTED
void MtcVonr::StopTimer()
{
    //---------------------------------------------------------------------------------------------
    if (m_piTimer == IMS_NULL)
    {
        return;
    }

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;
}
