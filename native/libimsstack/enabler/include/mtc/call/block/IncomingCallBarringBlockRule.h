/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef INCOMING_CALL_BARRING_BLOCK_RULE_H_
#define INCOMING_CALL_BARRING_BLOCK_RULE_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/block/IMtcBlockRule.h"

class IMtcCallContext;
class IMtcService;

class IncomingCallBarringBlockRule final : public IMtcBlockRule
{
public:
    IncomingCallBarringBlockRule(IN IMtcCallContext& objContext, IN CallType eCallType);
    virtual ~IncomingCallBarringBlockRule() override;
    IncomingCallBarringBlockRule(IN const IncomingCallBarringBlockRule&) = delete;
    IncomingCallBarringBlockRule& operator=(IN const IncomingCallBarringBlockRule&) = delete;

    Result Check(IN IMtcBlockRuleCheckListener& objListener) override;

private:
    Result IsBarringActivatedByCallType(IN PermanentSuppType eBarringTypeAll,
            IN PermanentSuppType eBarringTypeRoaming,
            IN PermanentSuppType eBarringTypeAnonymous) const;

    IMtcCallContext& m_objContext;
    IMtcService& m_objService;
    CallType m_eCallType;
};

#endif
