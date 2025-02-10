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

#ifndef SSAC_BLOCK_RULE_H_
#define SSAC_BLOCK_RULE_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/block/IMtcBlockRule.h"

class IMtcCallContext;
class IImsRadio;

class SsacBlockRule final : public IMtcBlockRule
{
public:
    explicit SsacBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType);
    virtual ~SsacBlockRule();
    SsacBlockRule(IN const SsacBlockRule&) = delete;
    SsacBlockRule& operator=(IN const SsacBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMtcCallContext& m_objContext;
    CallType m_eCallType;
    IImsRadio* m_pImsRadio;

    IMS_BOOL IsSsacTimerRunning(IN CallType eCallType) const;
    IMS_BOOL IsNeedToBar(IN CallType eCallType) const;
    void StartBarringTimer(IN CallType eCallType) const;
};

#endif
