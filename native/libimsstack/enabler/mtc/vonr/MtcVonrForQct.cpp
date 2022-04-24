/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20200924  dongo.yi@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServicePhoneInfo.h"

#include "call/MtcUiNotifier.h"
#include "MtcDef.h"
#include "vonr/MtcVonrForQct.h"
#include "vonr/IMtcVonrListener.h"

__IMS_TRACE_TAG_COM_UC__;


PUBLIC
UCVoNRForQct::UCVoNRForQct(IN IMS_UINT32 nSlotId, IN IMtcVonrListener* piListener)
    : MtcVonr(nSlotId, piListener)
{
    // --------------------------------------------------------------------------------------------
    IMS_TRACE_I("+UCVoNRForQct", 0, 0, 0);
}

PUBLIC VIRTUAL
UCVoNRForQct::~UCVoNRForQct()
{
    // --------------------------------------------------------------------------------------------
    IMS_TRACE_I("~UCVoNRForQct : [%" PFLS_x "]", this, 0, 0);
}

#if _PUBLIC_METHOD_
#endif
PUBLIC VIRTUAL
void UCVoNRForQct::CheckBarring(IN IMtcCall* piMtcCall, IN CallType eCallType,
        IN IMS_BOOL bEmergency)
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

    if (IsUacCheckRequired() == IMS_FALSE)
    {
        if (m_piListener->IsOtherSessionAlive(this, m_nUacType) == IMS_FALSE)
        {
            NotifyCallState(IVoNr::STATE_START);
        }
        m_eUacStatus = UacStatus::SUCCESS;
        return;
    }

    if (m_eUacStatus == UacStatus::IDLE)
    {
        m_eUacStatus = UacStatus::WAIT_RESPONSE;
    }

    StartTimer(TIME_WAIT_UAC_RESPONSE);
    NotifyCallState(IVoNr::STATE_START);
    // TODO, MTC BUILD
    // piUCSession->SetUacBlockType(IMtcCall::VONR_BLOCK_TYPE_WAIT_UAC);
}

#if _PROTECTED_METHOD_
#endif
PROTECTED VIRTUAL
void UCVoNRForQct::OnSessionStopped(IN IMS_UINTP nParam)
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

    if (m_piListener->IsOtherSessionAlive(this, m_nUacType) == IMS_FALSE)
    {
        NotifyCallState(IVoNr::STATE_IDLE);
    }

    m_piListener->OnTerminated(this);
}

PROTECTED VIRTUAL
void UCVoNRForQct::NotifyCallState(IN IMS_UINT32 nState)
{
    // --------------------------------------------------------------------------------------------
    IMS_TRACE_I("NotifyCallState - nState[%d], nDirection[%d]", nState, m_nDirection, 0);

    // TODO, MTC BUILD
    UNUSED_PARAM(nState);
    // IMtcCall* pITemp = GetCall();
    // if (pITemp)
    // {
    //     pITemp->HandleCallState(GetConvertedCallState(nState, m_nDirection));
    // }
}

PROTECTED VIRTUAL
void UCVoNRForQct::OnNotifyUacResponse(IN IMS_UINT32 nType, IN IMS_RESULT nResult,
        IN IMS_SINT32 nReason, IN IMS_UINT32 nSysMode, IN IMS_UINT32 nBarringTime)
{
    // --------------------------------------------------------------------------------------------
    (void)nBarringTime;

    IMS_TRACE_I("OnNotifyUacResponse - Result[%d], Reason[%d], SysMode[%d]",
            nResult, nReason, nSysMode);

    if (nSysMode != IVoNr::SYS_MODE_NR5G)
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

    if (m_eUacStatus == UacStatus::SUCCESS && pIUCSession)
    {
        // TODO, MTC BUILD
        // pIUCSession->SetUacBlockType(IMtcCall::VONR_BLOCK_TYPE_NONE);
    }
    else if (m_eUacStatus == UacStatus::FAILURE)
    {
        RequestCallPreference(IVoNr::RAT_LTE);
    }
}

PROTECTED VIRTUAL
void UCVoNRForQct::OnNotifyCallPreferenceReady(IN IMS_UINT32 nSysMode)
{
    // --------------------------------------------------------------------------------------------
    if (m_eUacStatus == UacStatus::SUCCESS
            || nSysMode != IVoNr::SYS_MODE_LTE)
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

    FailReason objFailReason(FAIL_REASON_NONE);

    // TODO, MTC BUILD
    // if (UC_BLOCK(m_nSlotId)->HandleOutSSAC(
    //         pIUCSession->GetInitiatedSessionType(), pIUCSession, &objFailReason))
    // {
    //     pIUCSession->SetUacBlockType(IMtcCall::VONR_BLOCK_TYPE_FINAL_FAILURE);
    //     return;
    // }

    m_eUacStatus = UacStatus::SUCCESS;
    NotifyCallState(IVoNr::STATE_START);

    // not to wait VoNrUac_NotifyResponse for LTE.
    // TODO, MTC BUILD
    // pIUCSession->SetUacBlockType(IMtcCall::VONR_BLOCK_TYPE_NONE);
}

PROTECTED VIRTUAL
IMS_BOOL UCVoNRForQct::IsUacCheckRequired()
{
    // --------------------------------------------------------------------------------------------
#ifdef _VONR_TEST_
    if (m_nUacType == IVoNr::TYPE_EMERGENCY
            || m_nDirection == IVoNr::DIRECTION_MT)
    {
        IMS_TRACE_I("IsUacCheckRequired : FALSE", 0, 0, 0);
        return IMS_FALSE;
    }
#else
    if (m_nUacType == IVoNr::TYPE_EMERGENCY
            || m_piVonr->IsUacCheckRequired(m_nUacType) == IMS_FALSE
            || m_nDirection == IVoNr::DIRECTION_MT)
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
IMS_UINT32 UCVoNRForQct::GetConvertedCallState(IN IMS_UINT32 nState, IN IMS_UINT32 nDirection)
{
    // --------------------------------------------------------------------------------------------
    if (nState == IVoNr::STATE_START)
    {
        if (nDirection == IVoNr::DIRECTION_MO)
        {
            return VOLTE_CALL_STATE_RINGBACK;
        }
        return VOLTE_CALL_STATE_RINGING;
    }

    // call state is defined by UCSession state.
    return VOLTE_CALL_STATE_UNDEFINED;
}
