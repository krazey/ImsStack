#include "ServiceTrace.h"
#include "ect/BlindTransferController.h"
#include "ect/EctReference.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "IMtcContext.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "CallReasonInfo.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
BlindTransferController::BlindTransferController(
        IN IMtcContext& objContext, IN CallKey nCallKey, IN IEctControllerListener& objListener) :
        EctController(objContext, nCallKey, objListener)
{
    IMS_TRACE_D("+BlindTransferController", 0, 0, 0);
}

PUBLIC
BlindTransferController::~BlindTransferController()
{
    IMS_TRACE_D("~BlindTransferController", 0, 0, 0);
}

PUBLIC VIRTUAL void BlindTransferController::OnReferenceStarted()
{
    IMS_TRACE_D("OnReferenceStarted", 0, 0, 0);
    OnCompleted();
}

PUBLIC VIRTUAL void BlindTransferController::Transfer(IN const AString& strNumber)
{
    IMS_TRACE_D("Transfer - blind [%s]", strNumber.GetStr(), 0, 0);

    if (IsValid() == IMS_FALSE)
    {
        OnFailed();
        return;
    }

    CreateReference();
    if (m_pReference->SendInvite(strNumber) == IMS_FAILURE)
    {
        OnFailed();
        return;
    }
}

PROTECTED VIRTUAL IMS_BOOL BlindTransferController::IsValid() const
{
    if (m_objContext.GetCallManager().GetCalls().GetSize() == 1)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
