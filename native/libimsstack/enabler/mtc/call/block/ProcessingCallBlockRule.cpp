/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/block/ProcessingCallBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ProcessingCallBlockRule::ProcessingCallBlockRule(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC VIRTUAL ProcessingCallBlockRule::~ProcessingCallBlockRule() {}

PUBLIC VIRTUAL ProcessingCallBlockRule::Result ProcessingCallBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    IMSList<IMtcCall*> lstCalls = m_objContext.GetOtherCalls();

    if (m_objContext.GetCallInfo().ePeerType == PeerType::MO)
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
    if (IsOtherIdleCallExists(lstCalls) || IsIncomingCallExists(lstCalls) ||
            IsOutgoingCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_REJECT_ONGOING_CALL_SETUP));
    }

    if (IsEmergencyCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_SERVICE_UNAVAILABLE));
    }

    return Result(Result::Status::UNBLOCKED);
}

PRIVATE
ProcessingCallBlockRule::Result ProcessingCallBlockRule::CheckForIncomingCall(
        IN const IMSList<IMtcCall*>& lstCalls)
{
    if (IsOtherIdleCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_CALL_BUSY));
    }

    if (IsIncomingCallExists(lstCalls) || IsOutgoingCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_REJECT_ONGOING_CALL_SETUP));
    }

    if (IsEmergencyCallExists(lstCalls))
    {
        return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_REJECT_ONGOING_E911_CALL));
    }

    return Result(Result::Status::UNBLOCKED);
}

PRIVATE
IMS_BOOL ProcessingCallBlockRule::IsOtherIdleCallExists(IN const IMSList<IMtcCall*>& lstCalls)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        IMtcCall::State eState = lstCalls.GetAt(nIndex)->GetState();
        if (eState == IMtcCall::State::IDLE)
        {
            IMS_TRACE_I("IsOtherIdleCallExists : Idle call exists", 0, 0, 0);
            return IMS_TRUE;
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
IMS_BOOL ProcessingCallBlockRule::IsEmergencyCallExists(IN const IMSList<IMtcCall*>& lstCalls)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        if (lstCalls.GetAt(nIndex)->GetCallContext().GetCallInfo().bEmergency)
        {
            IMS_TRACE_I("IsEmergencyCallExists : Emergency call exists", 0, 0, 0);
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}
