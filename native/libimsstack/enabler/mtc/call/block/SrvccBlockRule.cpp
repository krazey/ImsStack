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

#include "ServiceTrace.h"
#include "call/block/SrvccBlockRule.h"
#include "helper/ISrvccStateListener.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
SrvccBlockRule::SrvccBlockRule(IN SrvccState eState) :
        m_eSrvccState(eState)
{
}

PUBLIC VIRTUAL SrvccBlockRule::~SrvccBlockRule() {}

PUBLIC VIRTUAL SrvccBlockRule::Result SrvccBlockRule::Check(
        IN IMtcBlockRuleCheckListener& /* objListener */)
{
    if (m_eSrvccState == SrvccState::STARTED)
    {
        return Result(Result::Status::BLOCKED, CallReasonInfo(CODE_LOCAL_CALL_VCC_ON_PROGRESSING));
    }
    return Result(Result::Status::UNBLOCKED);
}
