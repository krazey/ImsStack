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

#ifndef WFC_BLOCK_RULE_H_
#define WFC_BLOCK_RULE_H_

#include "call/block/IMtcBlockRule.h"

class IMtcCallContext;
class IImsRadio;

/**
 * This class checks the condition where the UE is registered on the Wi-Fi but WFC is off.
 * In this case, voice calls should be blocked if voice call on cellular network is unavailable for
 * any reason: including VoPS off or voice SSAC barring.
 *
 * It covers Verizon's requirement VZ_REQ_VOWIFI_6230932. See {@code MtcRoutingRejectHandler} too.
 */
class WfcBlockRule final : public IMtcBlockRule
{
public:
    explicit WfcBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType);
    virtual ~WfcBlockRule() {}
    WfcBlockRule(IN const WfcBlockRule&) = delete;
    WfcBlockRule& operator=(IN const WfcBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMtcCallContext& m_objContext;
    IImsRadio* m_pImsRadio;

    CallType m_eCallType;

    IMS_BOOL IsVoiceCallAvailableInCellular() const;
    IMS_BOOL IsVopsAvailable() const;
    IMS_BOOL IsVoiceBlockedBySsac() const;
    IMS_BOOL IsWfcOn() const;
};

#endif
