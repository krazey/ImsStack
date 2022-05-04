#include "call/IMtcCall.h"
#include "call/IMtcCallManager.h"
#include "helper/block/ProcessingCallBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ProcessingCallBlockRule::ProcessingCallBlockRule(IN IMtcCallManager& objCallManager) :
        m_objCallManager(objCallManager)
{
}

PUBLIC VIRTUAL ProcessingCallBlockRule::~ProcessingCallBlockRule() {}

PUBLIC VIRTUAL ProcessingCallBlockRule::Result ProcessingCallBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    IMSList<IMtcCall*> lstCalls = m_objCallManager.GetCalls();

    if (IsOtherIdleCallExists(lstCalls))
    {
        IMS_TRACE_I("Check : Idle call exists", 0, 0, 0);
        return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_BUSY_NORMAL));
    }

    if (IsIncomingCallExists(lstCalls))
    {
        IMS_TRACE_I("Check : Incoming call exists", 0, 0, 0);
        return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_BUSY_ALERTING));
    }

    if (IsOutgoingCallExists(lstCalls))
    {
        IMS_TRACE_I("Check : Outgoing call exists", 0, 0, 0);
        return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_BUSY_ESTABLISHING));
    }

    if (IsEmergencyCallExists(m_objCallManager))
    {
        IMS_TRACE_I("Check : Emergency call exists", 0, 0, 0);
        return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_BUSY_ISEMERGENCY));
    }

    return Result(Result::Status::UNBLOCKED);
}

PRIVATE
IMS_BOOL ProcessingCallBlockRule::IsOtherIdleCallExists(IN const IMSList<IMtcCall*>& lstCalls)
{
    IMS_UINT32 nCount = 0;

    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        IMtcCall::State eState = lstCalls.GetAt(nIndex)->GetState();
        if (eState == IMtcCall::State::IDLE)
        {
            nCount += 1;

            if (nCount >= 2)  // 1 for current (checking) call, 1 for existing call
            {
                return IMS_TRUE;
            }
        }
    }
    return IMS_FALSE;
}

PRIVATE
IMS_BOOL ProcessingCallBlockRule::IsIncomingCallExists(IN const IMSList<IMtcCall*>& lstCalls)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        IMtcCall::State eState = lstCalls.GetAt(nIndex)->GetState();
        if (eState == IMtcCall::State::INCOMING || eState == IMtcCall::State::ALERTING)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PRIVATE
IMS_BOOL ProcessingCallBlockRule::IsOutgoingCallExists(IN const IMSList<IMtcCall*>& lstCalls)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        IMtcCall::State eState = lstCalls.GetAt(nIndex)->GetState();
        if (eState == IMtcCall::State::OUTGOING)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PRIVATE
IMS_BOOL ProcessingCallBlockRule::IsEmergencyCallExists(IN IMtcCallManager& objCallManager)
{
    return objCallManager.GetCallsByServiceType(ServiceType::EMERGENCY).GetSize() > 0;
}
