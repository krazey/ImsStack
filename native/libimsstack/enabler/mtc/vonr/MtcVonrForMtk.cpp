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

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServicePhoneInfo.h"

#include "call/IMtcUiNotifier.h"
#include "MtcDef.h"
#include "vonr/MtcVonrForMtk.h"
#include "vonr/MtcVonr.h"
#include "vonr/IMtcVonrListener.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UCVoNRForMtk::UCVoNRForMtk(IN IMS_UINT32 nSlotId, IN IMtcVonrListener* piListener) :
        MtcVonr(nSlotId, piListener),
        m_bEmergency(IMS_FALSE),
        m_nCurrentSysMode(0/*IVoNr::SYS_MODE_UNKNOWN*/),
        m_eTotalInitiateType(VonrInitType::NONE)
{
    // --------------------------------------------------------------------------------------------
    IMS_TRACE_I("+UCVoNRForMtk", 0, 0, 0);
}

PUBLIC VIRTUAL UCVoNRForMtk::~UCVoNRForMtk()
{
    // --------------------------------------------------------------------------------------------
    IMS_TRACE_I("~UCVoNRForMtk : [%" PFLS_x "]", this, 0, 0);
}

#if _PUBLIC_METHOD_
#endif
PUBLIC VIRTUAL void UCVoNRForMtk::CheckBarring(
        IN IMtcCall* piMtcCall, IN CallType eCallType, IN IMS_BOOL bEmergency)
{
    // --------------------------------------------------------------------------------------------
    IMS_TRACE_I("CheckBarring - eCallType[%d]", eCallType, 0, 0);

    if (UpdateSessionInfo(piMtcCall) == IMS_FAILURE)
    {
        IMS_TRACE_E(0, "CheckBarring - UpdateSessionInfo Failure", 0, 0, 0);
        return;
    }

    // TODO, MTC BUILD
    // piUCSession->SetListener(this);
    SetUacType(eCallType, bEmergency);
    m_bEmergency = (m_nUacType == 0/*IVoNr::TYPE_EMERGENCY*/);

    if (IsUacCheckRequired() == IMS_FALSE)
    {
        NotifyCallState(0/*IVoNr::STATE_START*/);
        m_eUacStatus = UacStatus::SUCCESS;
        return;
    }

    if (m_eUacStatus == UacStatus::IDLE)
    {
        m_eUacStatus = UacStatus::WAIT_RESPONSE;
    }

    StartTimer(TIME_WAIT_UAC_RESPONSE);
    NotifyCallState(0/*IVoNr::STATE_START*/);
    // TODO, MTC BUILD
    // piUCSession->SetUacBlockType(IMtcCall::VONR_BLOCK_TYPE_WAIT_UAC);
}

#if _PROTECTED_METHOD_
#endif
PROTECTED VIRTUAL void UCVoNRForMtk::OnSessionStopped(IN IMS_UINTP nParam)
{
    // --------------------------------------------------------------------------------------------
    IMS_TRACE_I("OnSessionStopped", 0, 0, 0);

    m_eUacStatus = UacStatus::IDLE;

    // TODO, MTC BUILD
    UNUSED_PARAM(nParam);
    // IUCSessionListenBaseInfoParam* pParam
    //         = reinterpret_cast<IUCSessionListenBaseInfoParam*>(nParam);
    // delete pParam;

    // IMtcCall* pIUCSession = GetCall();
    // if (pIUCSession)
    // {
    //     pIUCSession->ReleaseListener(this);
    // }

    if (m_piListener == IMS_NULL)
    {
        return;
    }

    NotifyCallState(0/*IVoNr::STATE_IDLE*/);

    m_piListener->OnTerminated(this);
}

PROTECTED VIRTUAL void UCVoNRForMtk::NotifyCallState(IN IMS_UINT32 nState)
{
    // --------------------------------------------------------------------------------------------
    IMS_TRACE_I("NotifyCallState - nState[%d], nDirection[%d]", nState, m_nDirection, 0);

    if (/*m_piVonr == IMS_NULL || */m_piListener == IMS_NULL)
    {
        return;
    }

    m_eTotalInitiateType = m_piListener->GetTotalInitiateType();
    m_nCurrentSysMode = GetSysMode();

    IMS_BOOL bSameType;
    IMS_BOOL bDiffType;
    IMS_BOOL bDiffVoiceDomain;
    GetOngoingSessionStatus(bSameType, bDiffType, bDiffVoiceDomain);

    if (nState == 0/*IVoNr::STATE_START*/)
    {
        NotifyCallStart(bSameType, bDiffType, bDiffVoiceDomain);
    }
    else if (nState == 0/*IVoNr::STATE_IDLE*/)
    {
        NotifyCallStop(bSameType, bDiffType, bDiffVoiceDomain);
    }

    if (m_eTotalInitiateType == VonrInitType::NONE)
    {
        m_piListener->SetInitiateType(GetConvertedInitType(m_nCurrentSysMode));
        m_eTotalInitiateType = m_piListener->GetTotalInitiateType();
    }
}

PROTECTED VIRTUAL void UCVoNRForMtk::OnNotifyUacResponse(IN IMS_UINT32 nType, IN IMS_RESULT nResult,
        IN IMS_SINT32 nReason, IN IMS_UINT32 nSysMode, IN IMS_UINT32 nBarringTime)
{
    // --------------------------------------------------------------------------------------------
    (void)nBarringTime;

    IMS_TRACE_I("OnNotifyUacResponse - Result[%d], Reason[%d], SysMode[%d]", nResult, nReason,
            nSysMode);

    if (nSysMode != 0/*IVoNr::SYS_MODE_NR5G*/)
    {
        return;
    }

    if (nType != m_nUacType)
    {
        return;
    }

    StopTimer();

    if (m_eUacStatus != UacStatus::WAIT_RESPONSE)
    {
        // FIX_ME : allow change from UacStatus::FAILURE to UacStatus::SUCCESS???
        return;
    }

    m_eUacStatus = (nResult == IMS_SUCCESS) ? UacStatus::SUCCESS : UacStatus::FAILURE;
    IMtcCall* pIUCSession = GetCall();

    IMS_BOOL bSameType;
    IMS_BOOL bDiffType;
    IMS_BOOL bDiffVoiceDomain;
    GetOngoingSessionStatus(bSameType, bDiffType, bDiffVoiceDomain);
    if (m_eUacStatus == UacStatus::SUCCESS && pIUCSession)
    {
        SetImsVoice(0/*IVoNr::STATE_START*/, bSameType, bDiffType, bDiffVoiceDomain);
        // TODO, MTC BUILD
        // pIUCSession->SetUacBlockType(IMtcCall::VONR_BLOCK_TYPE_NONE);
    }
    else if (m_eUacStatus == UacStatus::FAILURE)
    {
        SetImsVoice(0/*IVoNr::STATE_IDLE*/, bSameType, bDiffType, bDiffVoiceDomain);
        RequestCallPreference(0/*IVoNr::RAT_LTE*/);
    }
}

PROTECTED VIRTUAL void UCVoNRForMtk::OnNotifyCallPreferenceReady(IN IMS_UINT32 nSysMode)
{
    // --------------------------------------------------------------------------------------------
    if (m_eUacStatus == UacStatus::SUCCESS || nSysMode != 0/*IVoNr::SYS_MODE_LTE*/)
    {
        IMS_TRACE_I("OnNotifyCallPreferenceReady - invalid", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("OnNotifyCallPreferenceReady - nSysMode[%d]", nSysMode, 0, 0);

    IMtcCall* pIUCSession = GetCall();
    if (pIUCSession == IMS_NULL)
    {
        return;
    }

    // TODO, MTC BUILD
    // CallReasonInfo objReason;
    // if (UC_BLOCK(m_nSlotId)->HandleOutSSAC(
    //         pIUCSession->GetInitiatedSessionType(), pIUCSession, &objReason))
    // {
    //     pIUCSession->SetUacBlockType(IMtcCall::VONR_BLOCK_TYPE_FINAL_FAILURE);
    //     return;
    // }

    m_eUacStatus = UacStatus::SUCCESS;
    NotifyCallState(0/*IVoNr::STATE_START*/);

    // not to wait VoNrUac_NotifyResponse for LTE.
    // TODO, MTC BUILD
    // pIUCSession->SetUacBlockType(IMtcCall::VONR_BLOCK_TYPE_NONE);
}

PROTECTED VIRTUAL IMS_BOOL UCVoNRForMtk::IsUacCheckRequired()
{
    // --------------------------------------------------------------------------------------------
#ifdef _VONR_TEST_
    if (m_nUacType == IVoNr::TYPE_EMERGENCY || m_nDirection == IVoNr::DIRECTION_MT ||
            m_piListener->IsOtherSessionAlive(this, m_nUacType))
    {
        IMS_TRACE_I("IsUacCheckRequired : FALSE", 0, 0, 0);
        return IMS_FALSE;
    }
#else
    if (/*m_piVonr->IsUacCheckRequired(m_nUacType) == IMS_FALSE ||*/
            m_nUacType == 0/*IVoNr::TYPE_EMERGENCY */|| m_nDirection == 0/*IVoNr::DIRECTION_MT*/ ||
            m_piListener->IsOtherSessionAlive(this, m_nUacType))
    {
        IMS_TRACE_I("IsUacCheckRequired : FALSE", 0, 0, 0);
        return IMS_FALSE;
    }
#endif

    return IMS_TRUE;
}

#if _PRIVATE_METHOD_
#endif
PRIVATE
void UCVoNRForMtk::NotifyCallStart(
        IN IMS_BOOL bSameType, IN IMS_BOOL bDiffType, IN IMS_BOOL bDiffVoiceDomain)
{
    // --------------------------------------------------------------------------------------------
    if (m_eUacStatus == UacStatus::SUCCESS)
    {
        IMS_TRACE_I("NotifyCallStart : already started. no duplicate", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("NotifyCallStart : SameType[%s] DiffType[%s] DiffDomain[%s]", _TRACE_B_(bSameType),
            _TRACE_B_(bDiffType), _TRACE_B_(bDiffVoiceDomain));

    if (bSameType)
    {
        return;
    }

    SetVoice(0/*IVoNr::STATE_START*/, bDiffType, bDiffVoiceDomain);
    SetImsSession(0/*IVoNr::STATE_START*/);
    SetUacCheck(0/*IVoNr::STATE_START*/);
    SetImsVoice(0/*IVoNr::STATE_START*/, bSameType, bDiffType, bDiffVoiceDomain);
}

PRIVATE
void UCVoNRForMtk::NotifyCallStop(
        IN IMS_BOOL bSameType, IN IMS_BOOL bDiffType, IN IMS_BOOL bDiffVoiceDomain)
{
    // --------------------------------------------------------------------------------------------

    if (bSameType)
    {
        return;
    }

    IMS_TRACE_I("NotifyCallStop : SameType[%s] DiffType[%s] DiffDomain[%s]", _TRACE_B_(bSameType),
            _TRACE_B_(bDiffType), _TRACE_B_(bDiffVoiceDomain));

    SetImsVoice(0/*IVoNr::STATE_IDLE*/, bSameType, bDiffType, bDiffVoiceDomain);
    SetUacCheck(0/*IVoNr::STATE_IDLE*/);
    SetImsSession(0/*IVoNr::STATE_IDLE*/);
    SetVoice(0/*IVoNr::STATE_IDLE*/, bDiffType, bDiffVoiceDomain);
}

PRIVATE
void UCVoNRForMtk::SetVoice(
        IN IMS_UINT32 nState, IN IMS_BOOL bDiffType, IN IMS_BOOL bDiffVoiceDomain)
{
    // SetVoiceDomainStatus - VVS
    // MD Call Session Start/Stop
    // --------------------------------------------------------------------------------------------
    IMS_TRACE_D("SetVoice state[%d]", nState, 0, 0);

    if (nState == 0/*IVoNr::STATE_START*/)
    {
        if (m_eTotalInitiateType == VonrInitType::NONE)
        {
            // this is first call
            // no diffType call is alive.
            //m_piVonr->SetVoice(IVoNr::MTK_CALL_START, m_bEmergency);
            return;
        }

        if (bDiffVoiceDomain)
        {
            // aduio + emc OR video + emc case.
            //m_piVonr->SetVoice(IVoNr::MTK_CALL_START, m_bEmergency);
        }
        else
        {
            // audio + video case.
        }
    }
    else if (nState == 0/*IVoNr::STATE_IDLE*/)
    {
        if (bDiffType)
        {
            // audio + video case.
            return;
        }
        //m_piVonr->SetVoice(IVoNr::MTK_CALL_STOP, m_bEmergency);
    }
}

PRIVATE
void UCVoNRForMtk::SetImsVoice(IN IMS_UINT32 nState, IN IMS_BOOL bSameType, IN IMS_BOOL bDiffType,
        IN IMS_BOOL bDiffVoiceDomain)
{
    // SetImsVoiceDomainStatus - IVS
    // MD IMS call session start/stop,
    // it must be called when the first IMS call start and the last IMS Call
    // --------------------------------------------------------------------------------------------
    if (bSameType || bDiffType || bDiffVoiceDomain)
    {
        return;
    }

    IMS_TRACE_D("SetImsVoice state[%d] SysMode[%d] TotalInitiateType[%d]", nState,
            m_nCurrentSysMode, m_eTotalInitiateType);

    if (nState == 0/*IVoNr::STATE_START*/)
    {
        if (m_nDirection == 0/*IVoNr::DIRECTION_MO*/ &&
                m_nCurrentSysMode == 0/*IVoNr::SYS_MODE_NR5G*/ &&
                m_eTotalInitiateType == VonrInitType::NONE)
        {
            // first call is started in NR5G. UAC check required.
            // this is called after UAC response by MtcVonr::VoNrUac_NotifyResponse()
            // with m_eTotalInitiateType 'VonrInitType::NR5G'
            return;
        }
        //m_piVonr->SetImsVoice(IVoNr::MTK_CALL_START, m_nCurrentSysMode);
    }
    else if (nState == 0/*IVoNr::STATE_IDLE*/)
    {
        //m_piVonr->SetImsVoice(IVoNr::MTK_CALL_STOP, m_nCurrentSysMode);
    }
}

PRIVATE
void UCVoNRForMtk::SetImsSession(IN IMS_UINT32 nState)
{
    // SetImsSession - ISESS
    // Indicate MD the start/stop of an IMS Session over 5G.
    // --------------------------------------------------------------------------------------------
    if (m_nUacType == 0/*IVoNr::TYPE_EMERGENCY*/)
    {
        return;
    }

    IMS_TRACE_D("SetImsSession state[%d] SysMode[%d] TotalInitiateType[%d]", nState,
            m_nCurrentSysMode, m_eTotalInitiateType);

    if (nState == 0/*IVoNr::STATE_START*/)
    {
        if (m_nCurrentSysMode != 0/*IVoNr::SYS_MODE_NR5G*/ &&
                m_eTotalInitiateType != VonrInitType::NR)
        {
            return;
        }

        // TODO:: SetImsSession required for 1stVoWifiCall - NR HO - 2ndNRCall(diff calltype)
        if (m_nCurrentSysMode == 0/*IVoNr::SYS_MODE_WLAN*/ ||
                m_eTotalInitiateType == VonrInitType::WIFI)
        {
            return;
        }

        //m_piVonr->SetImsSession(m_nUacType, IVoNr::MTK_CALL_START);
    }
    else if (nState == 0/*IVoNr::STATE_IDLE*/)
    {
        if (m_eTotalInitiateType == VonrInitType::LTE || m_eTotalInitiateType == VonrInitType::WIFI)
        {
            // call started in LTE/WIFI case.
            return;
        }
        //m_piVonr->SetImsSession(m_nUacType, IVoNr::MTK_CALL_STOP);
    }
}

PRIVATE
void UCVoNRForMtk::SetUacCheck(IN IMS_UINT32 nState)
{
    // SetUacCheck - UAC
    // Request/Clear UAC Check over 5G from MD.
    // It should bring start/stop for the IMS as same as SetImsSession()
    // --------------------------------------------------------------------------------------------
    if (m_nDirection == 0/*IVoNr::DIRECTION_MT*/)
    {
        return;
    }

    if (m_nUacType == 0/*IVoNr::TYPE_EMERGENCY*/)
    {
        return;
    }

    IMS_TRACE_D("SetUacCheck state[%d] SysMode[%d] TotalInitiateType[%d]", nState,
            m_nCurrentSysMode, m_eTotalInitiateType);

    if (nState == 0/*IVoNr::STATE_START*/)
    {
        if (m_nCurrentSysMode == 0/*IVoNr::SYS_MODE_NR5G*/ ||
                m_eTotalInitiateType == VonrInitType::NR)
        {
            //m_piVonr->SetUacCheck(m_nUacType, IVoNr::MTK_CALL_START);
        }
    }
    else if (nState == 0/*IVoNr::STATE_IDLE*/)
    {
        if (m_eTotalInitiateType == VonrInitType::LTE || m_eTotalInitiateType == VonrInitType::WIFI)
        {
            // call started in LTE/WIFI case.
            return;
        }
        //m_piVonr->SetUacCheck(m_nUacType, IVoNr::MTK_CALL_STOP);
    }
}

PRIVATE
void UCVoNRForMtk::GetOngoingSessionStatus(
        OUT IMS_BOOL& bSameType, OUT IMS_BOOL& bDiffType, OUT IMS_BOOL& bDiffVoiceDomain)
{
    // --------------------------------------------------------------------------------------------
    IMS_BOOL bVoiceSession = m_piListener->IsOtherSessionAlive(this, 0/*IVoNr::TYPE_VOICE*/);
    IMS_BOOL bVideoSession = m_piListener->IsOtherSessionAlive(this, 0/*IVoNr::TYPE_VIDEO*/);

    if (m_bEmergency)
    {
        bSameType = IMS_FALSE;  // no 2 call is available
        bDiffType = IMS_FALSE;  // no diff type in emergency domain.
        bDiffVoiceDomain = bVoiceSession || bVideoSession;
    }
    else
    {
        bSameType = (m_nUacType == 0/*IVoNr::TYPE_VOICE*/) ? bVoiceSession : bVideoSession;
        bDiffType = (m_nUacType == 0/*IVoNr::TYPE_VOICE*/) ? bVideoSession : bVoiceSession;
        bDiffVoiceDomain = m_piListener->IsOtherSessionAlive(this, 0/*IVoNr::TYPE_EMERGENCY*/);
    }
}
