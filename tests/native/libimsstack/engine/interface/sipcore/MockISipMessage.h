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

#ifndef MOCK_I_SIP_MESSAGE_H_
#define MOCK_I_SIP_MESSAGE_H_

#include <gmock/gmock.h>

#include "ISipMessageBodyPart.h"
#include "ISipObject.h"
#include "SipMethod.h"

#include "ISipMessage.h"

class MockISipMessage : public ISipMessage
{
public:
    inline MockISipMessage() {}
    inline virtual ~MockISipMessage() {}

    MOCK_METHOD(void, Destroy, (), (override));

    MOCK_METHOD(ISipMessage*, Clone, (), (const, override));
    MOCK_METHOD(IMS_RESULT, AddHeader,
            (IN IMS_SINT32 nType, IN const AString& strValue, IN const AString& strName),
            (override));
    MOCK_METHOD(IMS_UINT32, GetCSeqNumber, (), (const, override));
    MOCK_METHOD(AString, GetHeader,
            (IN IMS_SINT32 nType, IN IMS_SINT32 nIndex, IN const AString& strName),
            (const, override));
    MOCK_METHOD(IMS_SINT32, GetHeaderCount, (IN IMS_SINT32 nType, IN const AString& strName),
            (const, override));
    MOCK_METHOD(ImsList<AString>, GetHeaders, (IN IMS_SINT32 nType, IN const AString& strName),
            (const, override));
    MOCK_METHOD(const SipMethod&, GetMethod, (), (const, override));
    MOCK_METHOD(const AString&, GetReasonPhrase, (), (const, override));
    MOCK_METHOD(const AString&, GetRequestUri, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetStatusCode, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetType, (), (const, override));
    MOCK_METHOD(IMS_RESULT, PrependHeader,
            (IN IMS_SINT32 nType, IN const AString& strValue, IN const AString& strName),
            (override));
    MOCK_METHOD(void, RemoveHeader, (IN IMS_SINT32 nType, IN const AString& strName), (override));
    MOCK_METHOD(IMS_RESULT, SetHeader,
            (IN IMS_SINT32 nType, IN const AString& strValue, IN const AString& strName),
            (override));
    MOCK_METHOD(ISipMessageBodyPart*, CreateBodyPart, (), (override));
    MOCK_METHOD(ISipMessageBodyPart*, CreateSdpBodyPart, (), (override));
    MOCK_METHOD(ImsList<ISipMessageBodyPart*>, GetBodyParts, (), (const, override));
    MOCK_METHOD(ISipMessageBodyPart*, GetSdpBodyPart, (), (const, override));
    MOCK_METHOD(ImsList<ISipMessageBodyPart*>, GetSdpBodyParts, (), (const, override));
    MOCK_METHOD(IMS_RESULT, CopyHeadersAndBodyParts, (IN const ISipMessage* piSipMsg), (override));
    MOCK_METHOD(IMS_BOOL, IsHeaderPresent, (IN IMS_SINT32 nType, IN const AString& strName),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsMessageRpr, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsOptionRequired, (IN const AString& strOption), (const, override));
    MOCK_METHOD(IMS_BOOL, IsOptionSupported, (IN const AString& strOption), (const, override));
    MOCK_METHOD(void, RemoveBodyParts, (), (override));
    MOCK_METHOD(ByteArray, ToByteArray, (IN IMS_SINT32 nOptions), (const, override));
};

#endif  // MOCK_I_SIP_MESSAGE_H_
