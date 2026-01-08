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

#ifndef CS_CALL_BLOCK_RULE_H_
#define CS_CALL_BLOCK_RULE_H_

#include "call/block/IMtcBlockRule.h"

class IMtcCallContext;
class IMtcImsEventReceiver;

class CsCallBlockRule final : public IMtcBlockRule
{
public:
    explicit CsCallBlockRule(IN IMtcCallContext& objContext);
    virtual ~CsCallBlockRule() override;
    CsCallBlockRule(IN const CsCallBlockRule&) = delete;
    CsCallBlockRule& operator=(IN const CsCallBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMtcImsEventReceiver& m_objEventReceiver;
    IMS_BOOL m_bEmergencyCall;
};

#endif
