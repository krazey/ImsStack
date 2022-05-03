#include "call/IMtcCall.h"
#include "call/IMtcCallManager.h"
#include "helper/block/CallCountBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CallCountBlockRule::CallCountBlockRule(
        IN IMS_UINT32 nMaxCount, IN IMtcCallManager& objCallManager) :
        m_nMaxCount(nMaxCount),
        m_objCallManager(objCallManager)
{
}

PUBLIC VIRTUAL CallCountBlockRule::~CallCountBlockRule() {}

PUBLIC VIRTUAL CallCountBlockRule::Result CallCountBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (GetActiveCallCount(m_objCallManager.GetCalls()) > m_nMaxCount)
    {
        IMS_TRACE_I("Check : Max call count[%d] reached", m_nMaxCount, 0, 0);
        return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_BUSY_MAXCALL));
    }

    return Result(Result::Status::UNBLOCKED);
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
