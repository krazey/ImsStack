#include "call/IMtcCall.h"
#include "call/IMtcCallManager.h"
#include "helper/block/TerminalBasedCallWaitingBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
TerminalBasedCallWaitingBlockRule::TerminalBasedCallWaitingBlockRule(
        IN IMtcService& objService, IN IMtcCallManager& objCallManager) :
        m_objService(objService),
        m_objCallManager(objCallManager)
{
}

PUBLIC VIRTUAL TerminalBasedCallWaitingBlockRule::~TerminalBasedCallWaitingBlockRule() {}

PUBLIC VIRTUAL TerminalBasedCallWaitingBlockRule::Result TerminalBasedCallWaitingBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (GetActiveCallCount(m_objCallManager.GetCalls()) > 0)
    {
        if (m_objService.IsTerminalBasedCallWaitingEnabled() == IMS_FALSE)
        {
            IMS_TRACE_I("Check : Terminal based call waiting is not enabled", 0, 0, 0);
            return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_DECLINE_CW));
        }
    }

    return Result(Result::Status::UNBLOCKED);
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
