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

#include "call/IMtcCallContext.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/block/CallTypeBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CallTypeBlockRule::CallTypeBlockRule(IN IMtcCallContext& objContext, CallType eCallTypeToCheck) :
        m_objConfiguration(objContext.GetConfigurationProxy()),
        m_eCallTypeToCheck(eCallTypeToCheck)
{
}

PUBLIC VIRTUAL CallTypeBlockRule::~CallTypeBlockRule() {}

PUBLIC VIRTUAL CallTypeBlockRule::Result CallTypeBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (!m_objConfiguration.Is(Feature::ALLOW_TEXT_WITH_VIDEO))
    {
        if (m_eCallTypeToCheck == CallType::VIDEO_RTT)
        {
            IMS_TRACE_I("Check : Video RTT is not supported", 0, 0, 0);
            return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE));
        }
    }

    return Result(Result::Status::UNBLOCKED);
}
