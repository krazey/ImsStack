#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/block/CallCountBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CallCountBlockRule::CallCountBlockRule(IN IMtcCallContext& objContext) :
        m_objCallManager(objContext.GetCallManager()),
        m_objCallInfo(objContext.GetCallInfo()),
        m_nMaxCallCount(objContext.GetConfigurationProxy().GetInt(Feature::CALL_MAX_COUNT))
{
}

PUBLIC VIRTUAL CallCountBlockRule::~CallCountBlockRule() {}

PUBLIC VIRTUAL CallCountBlockRule::Result CallCountBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (m_objCallInfo.bConference && m_objCallInfo.ePeerType == PeerType::MO)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (GetActiveCallCount(m_objCallManager.GetCalls()) <= m_nMaxCallCount)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("Check : Max call count[%d] reached", m_nMaxCallCount, 0, 0);

    if (m_objCallInfo.ePeerType == PeerType::MO)
    {
        return Result(Result::Status::BLOCKED, FailReason(FAIL_REASON_SERVICE_MAXCALL));
    }
    else
    {
        return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_BUSY_MAXCALL));
    }
}

PRIVATE
IMS_UINT32 CallCountBlockRule::GetActiveCallCount(IN const IMSList<IMtcCall*> lstCalls)
{
    IMS_UINT32 nCount = 0;

    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        IMtcCall::State eState = lstCalls.GetAt(nIndex)->GetState();
        if (eState != IMtcCall::State::TERMINATING)
        {
            nCount += 1;
        }
    }

    return nCount;
}
