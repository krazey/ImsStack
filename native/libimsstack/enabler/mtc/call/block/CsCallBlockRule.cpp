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
#include "ImsEventDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/block/CsCallBlockRule.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
CsCallBlockRule::CsCallBlockRule(IN IMtcCallContext& objContext) :
        m_objEventReceiver(objContext.GetImsEventReceiver()),
        m_bEmergencyCall(objContext.GetCallInfo().IsEmergency())
{
}

PUBLIC VIRTUAL CsCallBlockRule::~CsCallBlockRule() {}

PUBLIC VIRTUAL CsCallBlockRule::Result CsCallBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (m_bEmergencyCall)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_UINT32 nCsCallState = m_objEventReceiver.GetWParam(IMS_EVENT_CSCALL_STATE);
    if (nCsCallState == IMS_CSCALL_STATE_IDLE ||
            nCsCallState == IMtcImsEventReceiver::UNKNOWN_VALUE)
    {
        return Result(Result::Status::UNBLOCKED);
    }

    IMS_TRACE_I("Check : CS call exists", 0, 0, 0);

    return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_REJECT_ONGOING_CS_CALL));
}
