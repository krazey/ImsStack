#include "IMSList.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/block/CallTypeBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CallTypeBlockRule::CallTypeBlockRule(IN IMtcCallContext& objContext) :
        m_objConfiguration(objContext.GetConfigurationProxy()),
        m_objCallManager(objContext.GetCallManager()),
        m_objCallInfo(objContext.GetCallInfo())
{
}

PUBLIC VIRTUAL CallTypeBlockRule::~CallTypeBlockRule() {}

PUBLIC VIRTUAL CallTypeBlockRule::Result CallTypeBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (!m_objConfiguration.Is(Feature::ALLOW_TEXT_WITH_VIDEO))
    {
        if (m_objCallInfo.eInitialCallType == CallType::VIDEO_RTT)
        {
            IMS_TRACE_I("Check : Video RTT is not supported", 0, 0, 0);
            return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE));
        }
    }

    return Result(Result::Status::UNBLOCKED);
}
