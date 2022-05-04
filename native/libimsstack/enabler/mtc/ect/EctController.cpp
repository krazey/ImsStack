#include "ServiceTrace.h"
#include "ect/EctController.h"
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
EctController::EctController(IN IMtcContext& objContext, IN CallKey nCallKey) :
        m_objContext(objContext),
        m_nTransferorKey(nCallKey)
{
    IMS_TRACE_D("+EctController", 0, 0, 0);
}

PUBLIC
EctController::~EctController()
{
    IMS_TRACE_D("~EctController", 0, 0, 0);
}

PUBLIC VIRTUAL void EctController::Transfer(IN const AString& strNumber)
{
    UNUSED_PARAM(strNumber);
    IMS_TRACE_D("Transfer - blind [%s]", strNumber.GetStr(), 0, 0);

    NotifyResult(IMS_FAILURE, FAIL_REASON_ECT_COMPLETED);
}

PUBLIC VIRTUAL void EctController::Transfer()
{
    IMS_TRACE_D("Transfer - consultative", 0, 0, 0);
    if (IsValid() == IMS_FALSE)
    {
        NotifyResult(IMS_FAILURE, FAIL_REASON_ECT_COMPLETED);
        return;
    }

    NotifyResult(IMS_SUCCESS, FAIL_REASON_ECT_COMPLETED);
}

PROTECTED
IMtcCall* EctController::GetTransferor() const
{
    IMtcCall* piTransferor = m_objContext.GetCallManager().GetCallByCallKey(m_nTransferorKey);
    if (m_nTransferorKey != piTransferor->GetKey())
    {
        IMS_TRACE_E(0, "NullCall.", 0, 0, 0);
        // What to do?
    }

    return m_objContext.GetCallManager().GetCallByCallKey(m_nTransferorKey);
}

PROTECTED VIRTUAL IMS_BOOL EctController::IsValid() const
{
    // TODO: base shouldn't do anything.
    // TODO: need to consider the 3rd incoming call case?
    if (m_objContext.GetCallManager().GetCalls().GetSize() == 2)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
void EctController::NotifyResult(
        IN IMS_RESULT nResult, IN IMS_SINT32 nReason /* = FAIL_REASON_NONE*/) const
{
    // TODO: is reason meaningful? what kind of reason to be used for ECT failure?
    MtcUiNotifier& objNotifier = GetTransferor()->GetCallContext().GetUiNotifier();
    objNotifier.SendEctCompleted(nResult, FailReason(nReason));
}
