#include "ImsEventDef.h"
#include "MtcImsEventReceiver.h"
#include "call/IMtcCallContext.h"
#include "helper/block/CsCallBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CsCallBlockRule::CsCallBlockRule(IN IMtcCallContext& objContext) :
        m_objService(objContext.GetService()),
        m_objEventReceiver(objContext.GetImsEventReceiver()),
        m_ePeerType(objContext.GetCallInfo().ePeerType)
{
}

PUBLIC VIRTUAL CsCallBlockRule::~CsCallBlockRule() {}

PUBLIC VIRTUAL CsCallBlockRule::Result CsCallBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (m_objService.GetServiceType() == ServiceType::EMERGENCY)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_UINT32 nCsCallState = m_objEventReceiver.GetWParam(IMS_EVENT_CSCALL_STATE);
    if (nCsCallState == IMS_CSCALL_STATE_IDLE || nCsCallState == MtcImsEventReceiver::UNKNOWN_VALUE)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("Check : CS call exists", 0, 0, 0);

    if (m_ePeerType == PeerType::MO)
    {
        return Result(Result::Status::BLOCKED, FailReason(FAIL_REASON_SERVICE_INCSCALL));
    }
    else
    {
        return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_BUSY_ISCSCALL));
    }
}
