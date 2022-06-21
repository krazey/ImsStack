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

#ifndef TERMINAL_BASED_CALL_WAITING_BLOCK_RULE_H_
#define TERMINAL_BASED_CALL_WAITING_BLOCK_RULE_H_

#include "IMSTypeDef.h"
#include "helper/block/IMtcBlockRule.h"

class IMtcCall;
class IMtcCallContext;
class IMtcCallManager;

class TerminalBasedCallWaitingBlockRule final : public IMtcBlockRule
{
public:
    explicit TerminalBasedCallWaitingBlockRule(IN IMtcCallContext& objContext);
    virtual ~TerminalBasedCallWaitingBlockRule();
    TerminalBasedCallWaitingBlockRule(IN const TerminalBasedCallWaitingBlockRule&) = delete;
    TerminalBasedCallWaitingBlockRule& operator=(
            IN const TerminalBasedCallWaitingBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMS_UINT32 GetActiveCallCount(IN const IMSList<IMtcCall*> lstCalls);

    IMtcService& m_objService;
    IMtcCallManager& m_objCallManager;
};

#endif
