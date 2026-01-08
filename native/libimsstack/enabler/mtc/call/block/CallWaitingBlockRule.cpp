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

#include "IMtcService.h"
#include "ImsList.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/block/CallWaitingBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CallWaitingBlockRule::CallWaitingBlockRule(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_objService(objContext.GetService())
{
    IMS_ASSERT(objContext.GetCallInfo().ePeerType == PeerType::MT);
}

PUBLIC VIRTUAL CallWaitingBlockRule::~CallWaitingBlockRule() {}

PUBLIC VIRTUAL CallWaitingBlockRule::Result CallWaitingBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (!IsActiveCallExists(m_objContext.GetOtherCalls()))
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (m_objService.IsPermanentSuppServiceEnabled(PermanentSuppType::TB_CW))
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("Check : Terminal based call waiting is not enabled", 0, 0, 0);

    return Result(
            Result::Status::BLOCKED, CallReasonInfo(CODE_REJECT_ONGOING_CALL_WAITING_DISABLED));
}

PRIVATE
IMS_BOOL CallWaitingBlockRule::IsActiveCallExists(IN const ImsList<IMtcCall*>& lstCalls)
{
    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        if (lstCalls.GetAt(nIndex)->GetState() != IMtcCall::State::TERMINATING)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
