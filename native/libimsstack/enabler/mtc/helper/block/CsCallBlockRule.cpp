#include "ImsEventDef.h"
#include "MtcImsEventReceiver.h"
#include "helper/block/CsCallBlockRule.h"
#include "MtcDef.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CsCallBlockRule::CsCallBlockRule(IN MtcImsEventReceiver& objEventReceiver) :
        m_objEventReceiver(objEventReceiver)
{
}

PUBLIC VIRTUAL
CsCallBlockRule::~CsCallBlockRule()
{
}

PUBLIC VIRTUAL
CsCallBlockRule::Result CsCallBlockRule::Check(IN IMtcBlockRuleCheckListener& /* objListener */)
{
    IMS_UINT32 nCsCallState = m_objEventReceiver.GetWParam(IMS_EVENT_CSCALL_STATE);
    if (nCsCallState == IMS_CSCALL_STATE_IDLE ||
            nCsCallState == MtcImsEventReceiver::UNKNOWN_VALUE)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("Check : CS call exists", 0, 0, 0);
    return Result(
            Result::Status::BLOCKED,
            FailReason(FAIL_REASON_SERVICE_INCSCALL));
}
