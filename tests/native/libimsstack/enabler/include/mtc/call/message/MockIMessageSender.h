/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_I_MESSAGE_SENDER_H_
#define MOCK_I_MESSAGE_SENDER_H_

#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/message/IMessageSender.h"
#include <gmock/gmock.h>

struct CallReasonInfo;

class MockIMessageSender : public IMessageSender
{
public:
    MOCK_METHOD(IMS_RESULT, Start, (IN CallType eCallType), (override));
    MOCK_METHOD(IMS_RESULT, SendProvisionalResponse,
            (IN IMS_SINT32 eStatusCode, IN IMS_BOOL bReliable, IN IMS_BOOL bIncludeSdp,
                    IN IMS_BOOL bIncludeAlertInfo),
            (override));
    MOCK_METHOD(IMS_RESULT, SendPrack, (), (override));
    MOCK_METHOD(IMS_RESULT, RespondToPrack, (IN IMS_SINT32 eStatusCode), (override));
    MOCK_METHOD(IMS_RESULT, SendEarlyUpdate, (IN UpdateType eUpdateType), (override));
    MOCK_METHOD(IMS_RESULT, RespondToEarlyUpdate, (IN IMS_SINT32 eStatusCode), (override));
    MOCK_METHOD(IMS_RESULT, Accept, (), (override));
    MOCK_METHOD(IMS_RESULT, Reject, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(IMS_RESULT, SendAck, (), (override));
    MOCK_METHOD(IMS_RESULT, Update,
            (IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo, IN IMS_SINT32 eMethod,
                    IN IMS_BOOL bSessionRefresh),
            (override));
    MOCK_METHOD(IMS_RESULT, AcceptUpdate, (), (override));
    MOCK_METHOD(IMS_RESULT, CancelUpdate, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(IMS_RESULT, Terminate, (IN IMS_BOOL bUseBye, IN const CallReasonInfo& objReason),
            (override));
};

#endif
