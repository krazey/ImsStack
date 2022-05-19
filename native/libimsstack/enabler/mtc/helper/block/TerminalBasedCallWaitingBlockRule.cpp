#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "helper/block/TerminalBasedCallWaitingBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
TerminalBasedCallWaitingBlockRule::TerminalBasedCallWaitingBlockRule(
        IN IMtcCallContext& objContext) :
        m_objService(objContext.GetService()),
        m_objCallManager(objContext.GetCallManager())
{
    IMS_ASSERT(objContext.GetCallInfo().ePeerType == PeerType::MT);
}

PUBLIC VIRTUAL TerminalBasedCallWaitingBlockRule::~TerminalBasedCallWaitingBlockRule() {}

PUBLIC VIRTUAL TerminalBasedCallWaitingBlockRule::Result TerminalBasedCallWaitingBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (GetActiveCallCount(m_objCallManager.GetCalls()) <= 0)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (m_objService.IsTerminalBasedCallWaitingEnabled())
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("Check : Terminal based call waiting is not enabled", 0, 0, 0);

    return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_DECLINE_CW));
}

PRIVATE
IMS_UINT32 TerminalBasedCallWaitingBlockRule::GetActiveCallCount(
        IN const IMSList<IMtcCall*> lstCalls)
{
    IMS_UINT32 nCount = 0;

    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        IMtcCall::State eState = lstCalls.GetAt(nIndex)->GetState();
        if (eState == IMtcCall::State::ESTABLISHED || eState == IMtcCall::State::UPDATING)
        {
            nCount += 1;
        }
    }

    return nCount;
}
