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
#ifndef MEDIA_DESCRIPTION_READER_H_
#define MEDIA_DESCRIPTION_READER_H_

#include "SdpMediaDescription.h"

#include "media/IMediaDescriptor.h"

class SdpMediaFormat;
class SdpPrecondition;

class MediaDescriptionReader : public IMediaDescriptor
{
public:
    MediaDescriptionReader(
            IN const SdpMediaDescription& objSmd, IN const AString& strSessionLevelRemoteAddress);
    ~MediaDescriptionReader() override = default;

    MediaDescriptionReader(IN const MediaDescriptionReader&) = delete;
    MediaDescriptionReader& operator=(IN const MediaDescriptionReader&) = delete;

public:
    // IMediaDescriptor interface
    IMS_RESULT AddAttribute(IN const AString& strAttribute) override;
    ImsList<AString> GetAttributes() const override;
    ImsList<AString> GetBandwidthInfo() const override;
    AString GetMediaDescription() const override;
    AString GetMediaTitle() const override;
    IMS_RESULT RemoveAttribute(IN const AString& strAttribute) override;
    IMS_RESULT SetBandwidthInfo(IN const ImsList<AString>& strBandwidthInfos) override;
    IMS_RESULT SetMediaTitle(IN const AString& strTitle) override;
    IMS_RESULT AddAttribute(IN IMS_SINT32 nType, IN const AString& strAttrValue,
            IN const AString& strType = AString::ConstNull()) override;
    IMS_RESULT AddAttributeInt(IN IMS_SINT32 nType, IN IMS_SINT32 nAttrValue,
            IN const AString& strType = AString::ConstNull()) override;
    IMS_RESULT AddBandwidth(IN IMS_SINT32 nType, IN IMS_SINT32 nBandwidth,
            IN const AString& strType = AString::ConstNull()) override;
    const AString& GetAttribute(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const override;
    ImsList<AString> GetAttributes(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const override;
    IMS_SINT32 GetAttributeInt(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const override;
    IMS_SINT32 GetBandwidth(
            IN IMS_SINT32 nType, IN const AString& strType = AString::ConstNull()) const override;
    IMS_SINT32 GetDirection() const override;
    const SdpMedia* GetMediaDescriptionEx() const override;
    const ImsList<SdpMediaFormat*>& GetMediaFormats() const override;
    IMS_RESULT RemoveAttribute(IN const SdpAttribute& objAttribute) override;
    IMS_RESULT RemoveAttribute(IN IMS_SINT32 nType,
            IN const AString& strAttrValue = AString::ConstNull(),
            IN const AString& strType = AString::ConstNull()) override;
    IMS_RESULT RemoveMediaFormat(IN IMS_SINT32 nType, IN const AString& strValue) override;
    IMS_RESULT SetConnectionAddress(IN const AString& strAddress) override;
    IMS_RESULT SetDirection(IN IMS_SINT32 nDirection) override;
    IMS_RESULT SetMediaDescription(IN IMS_SINT32 nType, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nTransportProtocol, IN const AStringArray& objFormats) override;
    IMS_RESULT SetMediaFormat(IN const SdpMediaFormat* pMediaFormat) override;
    IMS_RESULT SetMediaFormat(IN IMS_SINT32 nType, IN const AString& strValue,
            IN const AString& strAnyMap, IN const AString& strFmtp) override;
    IMS_RESULT SetPort(IN IMS_SINT32 nPort) override;

    const SdpMedia* GetMediaDescriptionExAsLocal() const override;
    IpAddress GetLocalAddress() const override;
    IMS_SINT32 GetLocalPort() const override;
    IpAddress GetRemoteAddress() const override;
    const AString& GetRemoteAddressAsString() const override;
    IMS_SINT32 GetRemotePort() const override;

    // IMS_SDP_PRECONDITION
    const SdpPrecondition* GetPrecondition(IN IMS_SINT32 nAttribute,
            IN IMS_SINT32 nType = SdpPrecondition::TYPE_QOS) const override;
    IMS_RESULT RemovePrecondition(
            IN IMS_SINT32 nAttribute, IN IMS_SINT32 nType = SdpPrecondition::TYPE_QOS) override;
    IMS_RESULT SetPrecondition(
            IN IMS_SINT32 nAttribute, IN const SdpPrecondition* pPrecondition) override;

private:
    void CreateMediaFormats();
    SdpPrecondition* CreatePrecondition(IN IMS_SINT32 nAttribute);

private:
    SdpMediaDescription m_objSmd;
    ImsList<SdpMediaFormat*> m_objMediaFormats;
    SdpPrecondition* m_pCurrentStatus;
    SdpPrecondition* m_pDesiredStatus;
    SdpPrecondition* m_pConfirmedStatus;
    AString m_strSessionLevelRemoteAddress;
};

#endif
