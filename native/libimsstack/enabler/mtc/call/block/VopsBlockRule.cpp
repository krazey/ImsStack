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

#include "IMtcImsEventReceiver.h"
#include "IMtcService.h"
#include "ImsEventDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/block/VopsBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
VopsBlockRule::VopsBlockRule(IN IMtcCallContext& objContext) :
        m_objService(objContext.GetService()),
        m_objEventReceiver(objContext.GetImsEventReceiver())
{
}

PUBLIC VIRTUAL VopsBlockRule::~VopsBlockRule() {}

PUBLIC VIRTUAL VopsBlockRule::Result VopsBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (m_objService.IsWlanIpCanType())
    {
        return Result(Result::Status::UNBLOCKED);
    }

    if (m_objEventReceiver.GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE) ==
            IMS_VOICE_OVER_PS_NOT_SUPPORTED)
    {
        IMS_TRACE_I("Check : VoPS is not supported", 0, 0, 0);
        return Result(Result::Status::BLOCKED,
                CallReasonInfo(CODE_SIP_NOT_ACCEPTABLE, EXTRA_CODE_NOT_ACCEPTABLE_BY_VOPS));
    }

    return Result(Result::Status::UNBLOCKED);
}
