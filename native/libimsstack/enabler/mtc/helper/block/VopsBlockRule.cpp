#include "IMSEventDef.h"
#include "IMtcService.h"
#include "MtcImsEventReceiver.h"
#include "helper/block/VopsBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
VopsBlockRule::VopsBlockRule(IN IMtcService& objService, IN MtcImsEventReceiver& objEventReceiver) :
        m_objService(objService),
        m_objEventReceiver(objEventReceiver)
{
}

PUBLIC VIRTUAL
VopsBlockRule::~VopsBlockRule()
{
}

PUBLIC VIRTUAL
VopsBlockRule::Result VopsBlockRule::Check(IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (m_objService.IsWlanIpCanType())
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (m_objEventReceiver.GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE) ==
            IMS_VOICE_OVER_PS_NOT_SUPPORTED)
    {
        IMS_TRACE_I("Check : VoPS is not supported", 0, 0, 0);
        return Result(
                Result::Status::BLOCKED,
                FailReason(REJECT_REASON_SESSION_NOTACCEPTABLEHERE));
    }

    return Result(Result::Status::UNBLOCKED);
}
