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
#include "call/block/TerminalBasedCallWaitingBlockRule.h"
#include "ServiceTrace.h"

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

    return Result(
            Result::Status::BLOCKED, CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED));
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
