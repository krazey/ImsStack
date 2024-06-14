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

#ifndef RETRY_AFTER_BLOCK_RULE_H_
#define RETRY_AFTER_BLOCK_RULE_H_

#include "ImsTypeDef.h"
#include "call/block/IMtcBlockRule.h"
#include "helper/IPassiveTimerListener.h"

class IMtcCallContext;

class RetryAfterBlockRule final : public IMtcBlockRule, public IPassiveTimerListener
{
public:
    explicit RetryAfterBlockRule(IN IMtcCallContext& objContext);
    virtual ~RetryAfterBlockRule();
    RetryAfterBlockRule(IN const RetryAfterBlockRule&) = delete;
    RetryAfterBlockRule& operator=(IN const RetryAfterBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

    void OnPassiveTimerExpired(IN IPassiveTimerHolder::Type eType) override;

private:
    IMS_BOOL IsEpsOnlyAttach() const;

    IMtcCallContext& m_objContext;
    IMtcBlockRuleCheckListener* m_piMtcBlockRuleCheckListener;
};

#endif
