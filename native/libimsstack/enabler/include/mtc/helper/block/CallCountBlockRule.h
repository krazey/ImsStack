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

#ifndef CALL_COUNT_BLOCK_RULE_H_
#define CALL_COUNT_BLOCK_RULE_H_

#include "IMSTypeDef.h"
#include "helper/block/IMtcBlockRule.h"

class IMtcCallContext;
class IMtcCallManager;
class MtcConfigurationProxy;
struct CallInfo;

class CallCountBlockRule final : public IMtcBlockRule
{
public:
    explicit CallCountBlockRule(IN IMtcCallContext& objContext);
    virtual ~CallCountBlockRule();
    CallCountBlockRule(IN const CallCountBlockRule&) = delete;
    CallCountBlockRule& operator=(IN const CallCountBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMS_UINT32 GetActiveCallCount();

    MtcConfigurationProxy& m_objConfiguration;
    IMtcCallManager& m_objCallManager;
    const CallInfo& m_objCallInfo;
};

#endif
