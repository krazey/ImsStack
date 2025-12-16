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

#ifndef PROCESSING_CALL_BLOCK_RULE_H_
#define PROCESSING_CALL_BLOCK_RULE_H_

#include "ImsTypeDef.h"
#include "call/block/IMtcBlockRule.h"

class IMtcCall;
class IMtcCallContext;
template <class T>
class ImsList;

class ProcessingCallBlockRule final : public IMtcBlockRule
{
public:
    explicit ProcessingCallBlockRule(IN IMtcCallContext& objContext);
    virtual ~ProcessingCallBlockRule() override;
    ProcessingCallBlockRule(IN const ProcessingCallBlockRule&) = delete;
    ProcessingCallBlockRule& operator=(IN const ProcessingCallBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    static IMS_BOOL IsCallSetupProcessing(IN const ImsList<IMtcCall*>& lstCalls);
    static IMS_BOOL IsCallConverting(IN const ImsList<IMtcCall*>& lstCalls);
    static IMS_BOOL IsEmergencyCallExists(IN const ImsList<IMtcCall*>& lstCalls);

    IMtcCallContext& m_objContext;
};

#endif
