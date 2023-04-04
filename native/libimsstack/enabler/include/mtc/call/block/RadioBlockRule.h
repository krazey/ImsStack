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

#ifndef RADIO_BLOCK_RULE_H_
#define RADIO_BLOCK_RULE_H_

#include "call/IMtcCall.h"
#include "call/block/IMtcBlockRule.h"
#include "call/radio/IMtcRadioChecker.h"

class IMtcCallContext;

class RadioBlockRule final : public IMtcBlockRule, public IMtcRadioCheckerListener
{
public:
    explicit RadioBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType);
    virtual ~RadioBlockRule();
    RadioBlockRule(IN const RadioBlockRule&) = delete;
    RadioBlockRule& operator=(IN const RadioBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

    // IMtcRadioCheckerListener
    void OnConnectionSetupPrepared() override;
    void OnConnectionFailed(IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis) override;

private:
    IMtcCallContext& m_objContext;
    IMtcBlockRuleCheckListener* m_piMtcBlockRuleCheckListener;
    CallType m_eCallType;

    IMS_BOOL IsEpsFallbackRequired(
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis) const;
};

#endif
