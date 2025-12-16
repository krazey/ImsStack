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

#ifndef CALL_TYPE_BLOCK_RULE_H_
#define CALL_TYPE_BLOCK_RULE_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/block/IMtcBlockRule.h"

class IMtcCallContext;
class MtcConfigurationProxy;
template <class T>
class ImsList;

class CallTypeBlockRule final : public IMtcBlockRule
{
public:
    explicit CallTypeBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType);
    virtual ~CallTypeBlockRule() override;
    CallTypeBlockRule(IN const CallTypeBlockRule&) = delete;
    CallTypeBlockRule& operator=(IN const CallTypeBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    IMS_BOOL IsBlockedByTextVideoCall();
    IMS_BOOL IsBlockedByVideoMultipleCall();
    static CallType GetRemoteCallTypeIncludingInactiveMedia(IN IMtcCallContext& objContext);
    static IMS_BOOL HasVideoCall(IN const ImsList<IMtcCall*>& lstCalls);
    static IMS_BOOL HasRttCall(IN const ImsList<IMtcCall*>& lstCalls);
    static IMS_BOOL IsVideoCall(IN CallType eCallType);

    IMtcCallContext& m_objContext;
    MtcConfigurationProxy& m_objConfiguration;
    CallType m_eCallType;
};

#endif
