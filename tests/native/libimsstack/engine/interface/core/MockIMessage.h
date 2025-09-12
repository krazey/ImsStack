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

#ifndef MOCK_I_MESSAGE_H_
#define MOCK_I_MESSAGE_H_

#include <gmock/gmock.h>

#include "SipMethod.h"
#include "IMessage.h"

class IMessageBodyPart;
class ISipMessage;

class MockIMessage : public IMessage
{
public:
    ~MockIMessage() override = default;

    MOCK_METHOD(IMS_RESULT, AddHeader, (IN const AString& strName, IN const AString& strValue),
            (override));
    MOCK_METHOD(IMessageBodyPart*, CreateBodyPart, (), (override));
    MOCK_METHOD(ImsList<IMessageBodyPart*>, GetBodyParts, (), (const, override));
    MOCK_METHOD(ImsList<AString>, GetHeaders, (IN const AString& strName), (const, override));
    MOCK_METHOD(ISipMessage*, GetMessage, (), (const, override));
    MOCK_METHOD(const SipMethod&, GetMethod, (), (const, override));
    MOCK_METHOD(const AString&, GetReasonPhrase, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetStatusCode, (), (const, override));
};

#endif  // MOCK_I_MESSAGE_H_
