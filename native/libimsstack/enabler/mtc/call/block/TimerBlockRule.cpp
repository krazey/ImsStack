/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "call/block/TimerBlockRule.h"
#include "helper/IPassiveTimerHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
TimerBlockRule::TimerBlockRule(
        IN IPassiveTimerHolder& objPassiveTimerHolder, IN IMS_BOOL bEmergencyCall) :
        m_objPassiveTimerHolder(objPassiveTimerHolder),
        m_bEmergencyCall(bEmergencyCall)
{
}

PUBLIC VIRTUAL TimerBlockRule::~TimerBlockRule() {}

PUBLIC VIRTUAL TimerBlockRule::Result TimerBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (!m_bEmergencyCall &&
            m_objPassiveTimerHolder.IsActive(
                    IPassiveTimerHolder::Type::CALL_BLOCKED_BY_RETRY_AFTER))
    {
        IMS_TRACE_I("Check : CSFB by Retry-After", 0, 0, 0);
        return Result(Result::Status::BLOCKED,
                CallReasonInfo(
                        CODE_LOCAL_CALL_CS_RETRY_REQUIRED, EXTRA_CODE_CALL_RETRY_SILENT_REDIAL));
    }

    return Result(Result::Status::UNBLOCKED);
}
