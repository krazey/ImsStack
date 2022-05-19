#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "helper/block/ProcessingCallBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ProcessingCallBlockRule::ProcessingCallBlockRule(IN IMtcCallContext& objContext) :
        m_objCallManager(objContext.GetCallManager()),
        m_ePeerType(objContext.GetCallInfo().ePeerType)
{
}

PUBLIC VIRTUAL ProcessingCallBlockRule::~ProcessingCallBlockRule() {}

PUBLIC VIRTUAL ProcessingCallBlockRule::Result ProcessingCallBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    IMSList<IMtcCall*> lstCalls = m_objCallManager.GetCalls();

    if (m_ePeerType == PeerType::MO)
    {
        return CheckForOutgoingCall(lstCalls);
    }
    else
    {
        return CheckForIncomingCall(lstCalls);
    }
}

PRIVATE
ProcessingCallBlockRule::Result ProcessingCallBlockRule::CheckForOutgoingCall(
        IN const IMSList<IMtcCall*>& lstCalls)
{
    if (IsOtherIdleCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED, FailReason(FAIL_REASON_SESSION_IN_SETUP));
    }

    if (IsIncomingCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED, FailReason(FAIL_REASON_SESSION_IN_SETUP));
    }

    if (IsOutgoingCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED, FailReason(FAIL_REASON_SESSION_IN_SETUP));
    }

    if (IsEmergencyCallExists(m_objCallManager))
    {
        return Result(Result::Status::BLOCKED, FailReason(FAIL_REASON_SERVICE_UNAVAILABLE));
    }

    return Result(Result::Status::UNBLOCKED);
}

PRIVATE
ProcessingCallBlockRule::Result ProcessingCallBlockRule::CheckForIncomingCall(
        IN const IMSList<IMtcCall*>& lstCalls)
{
    if (IsOtherIdleCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_BUSY_NORMAL));
    }

    if (IsIncomingCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_BUSY_ALERTING));
    }

    if (IsOutgoingCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED, FailReason(REJECT_REASON_BUSY_ESTABLISHING));
    }

    if (IsEmergencyCallExists(m_objCallManager))
    {
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
                IMS_TRACE_I("IsOtherIdleCallExists : Idle call exists", 0, 0, 0);
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
            IMS_TRACE_I("IsIncomingCallExists : Incoming call exists", 0, 0, 0);
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
            IMS_TRACE_I("IsOutgoingCallExists : Outgoing call exists", 0, 0, 0);
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}

PRIVATE
IMS_BOOL ProcessingCallBlockRule::IsEmergencyCallExists(IN IMtcCallManager& objCallManager)
{
    if (objCallManager.GetCallsByServiceType(ServiceType::EMERGENCY).GetSize() > 0)
    {
        IMS_TRACE_I("IsEmergencyCallExists : Emergency call exists", 0, 0, 0);
        return IMS_TRUE;
    }
    return IMS_FALSE;
}
