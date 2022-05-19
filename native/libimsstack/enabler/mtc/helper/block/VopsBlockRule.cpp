#include "ImsEventDef.h"
#include "IMtcService.h"
#include "MtcImsEventReceiver.h"
#include "call/IMtcCallContext.h"
#include "helper/block/VopsBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
VopsBlockRule::VopsBlockRule(IN IMtcCallContext& objContext) :
        m_objService(objContext.GetService()),
        m_objEventReceiver(objContext.GetImsEventReceiver()),
        m_ePeerType(objContext.GetCallInfo().ePeerType)
{
}

PUBLIC VIRTUAL VopsBlockRule::~VopsBlockRule() {}

PUBLIC VIRTUAL VopsBlockRule::Result VopsBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (m_objService.IsWlanIpCanType())
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (m_objEventReceiver.GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE) ==
            IMS_VOICE_OVER_PS_NOT_SUPPORTED)
    {
        IMS_TRACE_I("Check : VoPS is not supported", 0, 0, 0);

        if (m_ePeerType == PeerType::MO)
        {
            return Result(
                    Result::Status::BLOCKED, FailReason(FAIL_REASON_SESSION_NOTACCEPTABLEHERE));
        }
        else
        {
            return Result(
                    Result::Status::BLOCKED, FailReason(REJECT_REASON_SESSION_NOTACCEPTABLEHERE));
        }
    }

    return Result(Result::Status::UNBLOCKED);
}
