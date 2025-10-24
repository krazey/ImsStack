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

#ifndef VOPS_BLOCK_RULE_H_
#define VOPS_BLOCK_RULE_H_

#include "call/block/IMtcBlockRule.h"

class IMtcCallContext;
class IMtcService;
class IMtcImsEventReceiver;

class VopsBlockRule final : public IMtcBlockRule
{
public:
    explicit VopsBlockRule(IN IMtcCallContext& objContext);
    virtual ~VopsBlockRule() override;
    VopsBlockRule(IN const VopsBlockRule&) = delete;
    VopsBlockRule& operator=(IN const VopsBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMtcService& m_objService;
    IMtcImsEventReceiver& m_objEventReceiver;
};

#endif
