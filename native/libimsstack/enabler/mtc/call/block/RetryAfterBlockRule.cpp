/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/MtcCallManager.h"
#include "call/block/RetryAfterBlockRule.h"
#include "call/termination/StartErrorHandler.h"
#include "helper/IPassiveTimerHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
RetryAfterBlockRule::RetryAfterBlockRule(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_piMtcBlockRuleCheckListener(IMS_NULL)
{
}

PUBLIC VIRTUAL RetryAfterBlockRule::~RetryAfterBlockRule()
{
    m_objContext.GetPassiveTimerHolder().RemoveListener(
            IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, this);
}

PUBLIC VIRTUAL RetryAfterBlockRule::Result RetryAfterBlockRule::Check(
        IN IMtcBlockRuleCheckListener& objListener)
{
    if (m_objContext.GetCallInfo().IsEmergency())
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (!m_objContext.GetPassiveTimerHolder().IsActive(
                IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER))
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (!m_objContext.GetCallManager().GetCallsByState(IMtcCall::State::ESTABLISHED).IsEmpty())
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (!m_objContext.IsCsfbAvailable())
    {
        m_piMtcBlockRuleCheckListener = &objListener;
        m_objContext.GetPassiveTimerHolder().AddListener(
                IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER, this);
        return Result(Result::Status::PENDING);
    }

    IMS_TRACE_I("Check : CSFB by Retry-After", 0, 0, 0);
    return Result(Result::Status::BLOCKED,
            CallReasonInfo(CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
}

PUBLIC VIRTUAL void RetryAfterBlockRule::OnPassiveTimerExpired(
        IN IPassiveTimerHolder::Type /* eType */)
{
    m_piMtcBlockRuleCheckListener->OnBlockRuleChecked(Result(Result::Status::UNBLOCKED));
}
