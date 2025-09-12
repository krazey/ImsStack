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
#ifndef SESSION_DESCRIPTOR_H_
#define SESSION_DESCRIPTOR_H_

#include "ISessionDescriptor.h"

class ISessionState;

class SessionDescriptor : public ISessionDescriptor
{
public:
    explicit SessionDescriptor(IN ISessionState* piSessionState);
    ~SessionDescriptor() override;

    SessionDescriptor(IN const SessionDescriptor&) = delete;
    SessionDescriptor& operator=(IN const SessionDescriptor&) = delete;

private:
    // ISessionDescriptor interface implementations
    IMS_RESULT AddAttribute(IN const AString& strAttribute) override;
    ImsList<AString> GetAttributes() const override;
    AString GetProtocolVersion() const override;
    const AString& GetSessionId() const override;
    AString GetSessionInfo() const override;
    AString GetSessionName() const override;
    IMS_RESULT RemoveAttribute(IN const AString& strAttribute) override;
    IMS_RESULT SetSessionInfo(IN const AString& strInfo) override;
    IMS_RESULT SetSessionName(IN const AString& strName) override;

    IMS_RESULT AddAttribute(IN IMS_SINT32 nType, IN const AString& strAttrValue,
            IN const AString& strType = AString::ConstNull()) override;
    IMS_RESULT AddAttributeInt(IN IMS_SINT32 nType, IN IMS_SINT32 nAttrValue,
            IN const AString& strType = AString::ConstNull()) override;
    IMS_RESULT AddBandwidth(IN IMS_SINT32 nType, IN IMS_SINT32 nBandwidth,
            IN const AString& strType = AString::ConstNull()) override;
    const AString& GetAttribute(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const override;
    IMS_SINT32 GetAttributeInt(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const override;
    IMS_SINT32 GetBandwidth(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const override;
    IMS_SINT32 GetDirection() const override;
    const AString& GetSessionVersion() const override;
    const AString& GetUsername() const override;
    IMS_RESULT RemoveAttribute(IN const SdpAttribute& objAttribute) override;
    IMS_RESULT RemoveAttribute(IN IMS_SINT32 nType,
            IN const AString& strAttrValue = AString::ConstNull(),
            IN const AString& strType = AString::ConstNull()) override;
    IMS_RESULT RemoveAllBandwidths() override;
    IMS_RESULT SetConnectionAddress(IN const AString& strAddress) override;
    IMS_RESULT SetDirection(IN IMS_SINT32 nDirection) override;
    IMS_RESULT SetOriginAddress(IN const AString& strAddress) override;
    IpAddress GetLocalAddress() const override;
    IpAddress GetRemoteAddress() const override;
    const AString& GetRemoteAddressAsString() const override;

private:
    enum
    {
        MAX_RESERVED_ATTRIBUTE = 17
    };

    static const IMS_CHAR* RESERVED_ATTRIBUTE[MAX_RESERVED_ATTRIBUTE];

    ISessionState* m_piSessionState;
};

#endif
