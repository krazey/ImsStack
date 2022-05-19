#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "ect/EctController.h"
#include "ect/IEctControllerListener.h"
#include "ect/EctReference.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "IMtcContext.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/MtcUiNotifier.h"
#include "FailReason.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EctController::EctController(
        IN IMtcContext& objContext, IN CallKey nCallKey, IN IEctControllerListener& objListener) :
        m_objContext(objContext),
        m_nTransfereeKey(nCallKey),
        m_objListener(objListener),
        m_pReference(nullptr),
        m_piTimer(IMS_NULL)
{
    IMS_TRACE_D("+EctController", 0, 0, 0);
    StartTimer();
}

PUBLIC
EctController::~EctController()
{
    IMS_TRACE_D("~EctController", 0, 0, 0);
    StopTimer();
}

PUBLIC VIRTUAL void EctController::OnReferenceStarted()
{
    IMS_TRACE_D("OnReferenceStarted", 0, 0, 0);
}

PUBLIC VIRTUAL void EctController::OnReferenceStartFailed()
{
    IMS_TRACE_D("OnReferenceStartFailed", 0, 0, 0);
    OnFailed();
}

PUBLIC VIRTUAL void EctController::OnReferenceUpdated(IN SipStatusCode nSipFragCode)
{
    IMS_TRACE_D("OnReferenceUpdated", 0, 0, 0);
    if (SipStatusCode::IsFinalSuccess(nSipFragCode.ToInt()))
    {
        OnCompleted();
    }
    else if (SipStatusCode::IsFinalFailure(nSipFragCode.ToInt()))
    {
        OnFailed();
    }
}

PUBLIC VIRTUAL void EctController::Timer_TimerExpired(IN ITimer* piTimer)
{
    IMS_TRACE_D("Timer_TimerExpired", 0, 0, 0);
    if (m_piTimer != piTimer)
    {
        return;
    }

    OnFailed();
}

PROTECTED
IMtcCall* EctController::GetTransferee() const
{
    IMtcCall* piTransferee = m_objContext.GetCallManager().GetCallByCallKey(m_nTransfereeKey);
    if (m_nTransfereeKey != piTransferee->GetKey())
    {
        IMS_TRACE_E(0, "NullCall.", 0, 0, 0);
        // TODO: What to do?
    }

    return m_objContext.GetCallManager().GetCallByCallKey(m_nTransfereeKey);
}

PROTECTED VIRTUAL void EctController::OnCompleted()
{
    NotifyResult(IMS_SUCCESS, FAIL_REASON_ECT_COMPLETED);
    TerminateTransfereeCall();
    m_objListener.OnEctCompleted();
}

PROTECTED VIRTUAL void EctController::OnFailed()
{
    // TODO: Recover()?
    NotifyResult(IMS_FAILURE, FAIL_REASON_ECT_COMPLETED);
    m_objListener.OnEctCompleted();
}

PROTECTED
void EctController::NotifyResult(
        IN IMS_RESULT nResult, IN IMS_SINT32 nReason /* = FAIL_REASON_NONE*/) const
{
    IMS_TRACE_D("NotifyResult", 0, 0, 0);
    // TODO: is reason meaningful? what kind of reason to be used for ECT failure?
    MtcUiNotifier& objNotifier = GetTransferee()->GetCallContext().GetUiNotifier();
    objNotifier.SendEctCompleted(nResult, FailReason(nReason));
}

PROTECTED
void EctController::CreateReference()
{
    IMS_TRACE_I("CreateReference", 0, 0, 0);

    m_pReference = std::make_unique<EctReference>(m_objContext, m_nTransfereeKey, *this);
}

PROTECTED
void EctController::TerminateTransfereeCall()
{
    IMS_TRACE_I("TerminateTransfereeCall", 0, 0, 0);
    GetTransferee()->Terminate(FailReason(FAIL_REASON_ECT_COMPLETED));
}

PROTECTED
void EctController::StartTimer()
{
    m_piTimer = TimerService::GetTimerService()->CreateTimer();
    m_piTimer->SetTimer(TIME_WAIT_OPERATION_COMPLETE, this);
}

PROTECTED
void EctController::StopTimer()
{
    if (m_piTimer == IMS_NULL)
    {
        return;
    }

    m_piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(m_piTimer);
    m_piTimer = IMS_NULL;
}
