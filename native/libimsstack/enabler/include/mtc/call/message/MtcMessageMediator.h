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

#ifndef MTC_MESSAGE_MEDIATOR_H_
#define MTC_MESSAGE_MEDIATOR_H_

#include "ImsTypeDef.h"
#include "base/IMessageMediator.h"
#include "call/IMtcCall.h"

class IMtcCallContext;
class ISipMessage;

class MtcMessageMediator final : public IMessageMediator
{
public:
    explicit MtcMessageMediator(IN IMtcCallContext& objContext);
    ~MtcMessageMediator();
    MtcMessageMediator(IN const MtcMessageMediator&) = delete;
    MtcMessageMediator& operator=(IN const MtcMessageMediator&) = delete;

    IMS_RESULT MessageMediator_AdjustMessage(
            IN_OUT ISipMessage* piSipMessage, IN IMS_SINT32 nMessage) override;

private:
    AString GetContactHeaderWithoutFeatureTag(IN const AString& strFeatureTag);
    CallType GetCallTypeOfCurrentMessage();

    IMtcCallContext& m_objContext;
    AString m_strOriginalContactHeader;
};

#endif
