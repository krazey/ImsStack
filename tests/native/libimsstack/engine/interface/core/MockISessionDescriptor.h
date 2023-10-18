/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef MOCK_I_SESSION_DESCRIPTOR_H_
#define MOCK_I_SESSION_DESCRIPTOR_H_

#include <gmock/gmock.h>
#include "ISessionDescriptor.h"

class MockISessionDescriptor : public ISessionDescriptor
{
public:
    MOCK_METHOD(IMS_RESULT, AddAttribute, (IN const AString& strAttribute), (override));
    MOCK_METHOD(ImsList<AString>, GetAttributes, (), (const, override));
    MOCK_METHOD(AString, GetProtocolVersion, (), (const, override));
    MOCK_METHOD(const AString&, GetSessionId, (), (const, override));
    MOCK_METHOD(AString, GetSessionInfo, (), (const, override));
    MOCK_METHOD(AString, GetSessionName, (), (const, override));
    MOCK_METHOD(IMS_RESULT, RemoveAttribute, (IN const AString& strAttribute), (override));
    MOCK_METHOD(IMS_RESULT, SetSessionInfo, (IN const AString& strInfo), (override));
    MOCK_METHOD(IMS_RESULT, SetSessionName, (IN const AString& strName), (override));
    MOCK_METHOD(IMS_RESULT, AddAttribute,
            (IN IMS_SINT32 nType, IN const AString& strAttrValue, IN const AString& strType),
            (override));
    MOCK_METHOD(IMS_RESULT, AddAttributeInt,
            (IN IMS_SINT32 nType, IN IMS_SINT32 nAttrValue, IN const AString& strType), (override));
    MOCK_METHOD(IMS_RESULT, AddBandwidth,
            (IN IMS_SINT32 nType, IN IMS_SINT32 nBandwidth, IN const AString& strType), (override));
    MOCK_METHOD(const AString&, GetAttribute, (IN IMS_SINT32 nType, IN const AString& strType),
            (const, override));
    MOCK_METHOD(IMS_SINT32, GetAttributeInt, (IN IMS_SINT32 nType, IN const AString& strType),
            (const, override));
    MOCK_METHOD(IMS_SINT32, GetBandwidth, (IN IMS_SINT32 nType, IN const AString& strType),
            (const, override));
    MOCK_METHOD(IMS_SINT32, GetDirection, (), (const, override));
    MOCK_METHOD(const AString&, GetSessionVersion, (), (const, override));
    MOCK_METHOD(const AString&, GetUsername, (), (const, override));
    MOCK_METHOD(IMS_RESULT, RemoveAttribute, (IN const SdpAttribute& objAttribute), (override));
    MOCK_METHOD(IMS_RESULT, RemoveAttribute,
            (IN IMS_SINT32 nType, IN const AString& strAttrValue, IN const AString& strType),
            (override));
    MOCK_METHOD(IMS_RESULT, RemoveAllBandwidths, (), (override));
    MOCK_METHOD(IMS_RESULT, SetConnectionAddress, (IN const AString& strAddress), (override));
    MOCK_METHOD(IMS_RESULT, SetDirection, (IN IMS_SINT32 nDirection), (override));
    MOCK_METHOD(IMS_RESULT, SetOriginAddress, (IN const AString& strAddress), (override));
    MOCK_METHOD(IpAddress, GetLocalAddress, (), (const, override));
    MOCK_METHOD(IpAddress, GetRemoteAddress, (), (const, override));
    MOCK_METHOD(const AString&, GetRemoteAddressAsString, (), (const, override));
};

#endif
