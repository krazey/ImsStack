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
#ifndef MOCK_I_MEDIA_DESCRIPTOR_H_
#define MOCK_I_MEDIA_DESCRIPTOR_H_

#include <gmock/gmock.h>
#include "core/media/IMediaDescriptor.h"

class MockIMediaDescriptor : public IMediaDescriptor
{
public:
    MOCK_METHOD(IMS_RESULT, AddAttribute, (IN const AString& strAttribute), (override));
    MOCK_METHOD(ImsList<AString>, GetAttributes, (), (const, override));
    MOCK_METHOD(ImsList<AString>, GetBandwidthInfo, (), (const, override));
    MOCK_METHOD(AString, GetMediaDescription, (), (const, override));
    MOCK_METHOD(AString, GetMediaTitle, (), (const, override));
    MOCK_METHOD(IMS_RESULT, RemoveAttribute, (IN const AString& strAttribute), (override));
    MOCK_METHOD(IMS_RESULT, SetBandwidthInfo, (IN const ImsList<AString>& strBandwidthInfos),
            (override));
    MOCK_METHOD(IMS_RESULT, SetMediaTitle, (IN const AString& strTitle), (override));
    MOCK_METHOD(IMS_RESULT, AddAttribute,
            (IN IMS_SINT32 nType, IN const AString& strAttrValue, IN const AString& strType),
            (override));
    MOCK_METHOD(IMS_RESULT, AddAttributeInt,
            (IN IMS_SINT32 nType, IN IMS_SINT32 nAttrValue, IN const AString& strType), (override));
    MOCK_METHOD(IMS_RESULT, AddBandwidth,
            (IN IMS_SINT32 nType, IN IMS_SINT32 nBandwidth, IN const AString& strType), (override));
    MOCK_METHOD(const AString&, GetAttribute, (IN IMS_SINT32 nType, IN const AString& strType),
            (const, override));
    MOCK_METHOD(ImsList<AString>, GetAttributes, (IN IMS_SINT32 nType, IN const AString& strType),
            (const, override));
    MOCK_METHOD(IMS_SINT32, GetAttributeInt, (IN IMS_SINT32 nType, IN const AString& strType),
            (const, override));
    MOCK_METHOD(IMS_SINT32, GetBandwidth, (IN IMS_SINT32 nType, IN const AString& strType),
            (const, override));
    MOCK_METHOD(IMS_SINT32, GetDirection, (), (const, override));
    MOCK_METHOD(const SdpMedia*, GetMediaDescriptionEx, (), (const, override));
    MOCK_METHOD(const ImsList<SdpMediaFormat*>&, GetMediaFormats, (), (const, override));
    MOCK_METHOD(IMS_RESULT, RemoveAttribute, (IN const SdpAttribute& objAttribute), (override));
    MOCK_METHOD(IMS_RESULT, RemoveAttribute,
            (IN IMS_SINT32 nType, IN const AString& strAttrValue, IN const AString& strType),
            (override));
    MOCK_METHOD(IMS_RESULT, RemoveMediaFormat, (IN IMS_SINT32 nType, IN const AString& strValue),
            (override));
    MOCK_METHOD(IMS_RESULT, SetConnectionAddress, (IN const AString& strAddress), (override));
    MOCK_METHOD(IMS_RESULT, SetDirection, (IN IMS_SINT32 nDirection), (override));
    MOCK_METHOD(IMS_RESULT, SetMediaDescription,
            (IN IMS_SINT32 nType, IN IMS_SINT32 nPort, IN IMS_SINT32 nTransportProtocol,
                    IN const AStringArray& objFormats),
            (override));
    MOCK_METHOD(IMS_RESULT, SetMediaFormat, (IN const SdpMediaFormat* pMediaFormat), (override));
    MOCK_METHOD(IMS_RESULT, SetMediaFormat,
            (IN IMS_SINT32 nType, IN const AString& strValue, IN const AString& strAnyMap,
                    IN const AString& strFmtp),
            (override));
    MOCK_METHOD(IMS_RESULT, SetPort, (IN IMS_SINT32 nPort), (override));
    MOCK_METHOD(const SdpMedia*, GetMediaDescriptionExAsLocal, (), (const, override));
    MOCK_METHOD(IpAddress, GetLocalAddress, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetLocalPort, (), (const, override));
    MOCK_METHOD(IpAddress, GetRemoteAddress, (), (const, override));
    MOCK_METHOD(const AString&, GetRemoteAddressAsString, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRemotePort, (), (const, override));
    MOCK_METHOD(const SdpPrecondition*, GetPrecondition,
            (IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType), (const, override));
    MOCK_METHOD(IMS_RESULT, RemovePrecondition, (IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType),
            (override));
    MOCK_METHOD(IMS_RESULT, SetPrecondition,
            (IN IMS_SINT32 nAttribute, IN const SdpPrecondition* pPrecondition), (override));
};

#endif
