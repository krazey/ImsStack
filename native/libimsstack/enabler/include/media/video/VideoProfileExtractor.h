/**
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef VIDEO_PROFILE_EXTRACTOR_H_
#define VIDEO_PROFILE_EXTRACTOR_H_

#include "ProfileExtractor.h"
#include "video/VideoProfileUtil.h"

class VideoProfileExtractor : public ProfileExtractor
{
public:
    explicit VideoProfileExtractor();
    virtual ~VideoProfileExtractor();

    IMS_BOOL Extract(IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT VideoProfile* pProfile);

private:
    void ExtractTranportType(IN const IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile);
    void SetAvpfSupport(OUT VideoProfile* pProfile);
    void ExtractPayloads(IN IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile);
    void ExtractRtpMap(IN const SdpAvCodec* pSdpCodec, OUT VideoProfile::Payload* pPayload);
    VIDEO_CODEC SetCodec(IN VideoProfile::Payload* pPayload);
    IMS_BOOL IsValidCodec(IN const VIDEO_CODEC eVideoCodec);
    AString ExtractImageAttr(IN const SdpAvCodec* pSdpCodec,
            IN const ImsList<AString>& objImageAttributes, OUT VideoProfile::Payload* pPayload);
    AString ExtractFrameSize(IN const SdpAvCodec* pSdpCodec,
            IN const ImsList<AString>& objFrameSizes, OUT VideoProfile::Payload* pPayload);
    void ExtractCvo(IN IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile);
    IMS_BOOL ExtractFmtp(IN const AString& strFmtp, OUT VideoProfile::Payload* pPayload,
            IN const VIDEO_CODEC eVideoCodec);
    IMS_BOOL ExtractVideoBaseFmtp(
            IN const ImsList<AString>& objSplitEqual, OUT VideoProfile::VideoFmtp* pFmtp);
    void ExtractAvcFmtp(IN const ImsList<AString>& objSplitEqual, IN const AString& strSpropParam,
            OUT VideoProfile::AvcFmtp* pFmtp);
    void ExtractHevcFmtp(IN const ImsList<AString>& objSplitEqual, OUT AString& strVps,
            OUT AString& strSps, OUT AString& strPps, OUT VideoProfile::HevcFmtp* pFmtp);
    IMS_BOOL ExtractFmtpPacketizationMode(
            IN const ImsList<AString>& objSplitEqual, OUT VideoProfile::VideoFmtp* pFmtp);
    IMS_BOOL ExtractFmtpProfileLevelId(
            IN const ImsList<AString>& objSplitEqual, OUT VideoProfile::AvcFmtp* pFmtp);
    IMS_BOOL ExtractFmtpSpropParameterSets(IN const ImsList<AString>& objSplitEqual,
            IN const AString& strSpropParam, OUT VideoProfile::AvcFmtp* pFmtp);
    IMS_BOOL ExtractFmtpProfileId(
            IN const ImsList<AString>& objSplitEqual, OUT VideoProfile::HevcFmtp* pFmtp);
    IMS_BOOL ExtractFmtpLevelId(
            IN const ImsList<AString>& objSplitEqual, OUT VideoProfile::HevcFmtp* pFmtp);
    IMS_BOOL ExtractFmtpVps(IN const ImsList<AString>& objSplitEqual, OUT AString& strVps);
    IMS_BOOL ExtractFmtpSps(IN const ImsList<AString>& objSplitEqual, OUT AString& strSps);
    IMS_BOOL ExtractFmtpPps(IN const ImsList<AString>& objSplitEqual, OUT AString& strPps);
    void ExtractFmtpSpropParam(IN const AString& strVps, IN const AString& strSps,
            IN const AString& strPps, OUT VideoProfile::VideoFmtp* pFmtp);
    void ExtractResolution(OUT VideoProfile::Payload* pPayload, AString& strImageAttr,
            AString& strFrameSize, VIDEO_CODEC eVideoCodec);
    void ExtractAvpfAttribute(IN SdpAvCodec* pSdpCodec, IN VideoProfile::Payload* pPayload,
            OUT VideoProfile* pProfile);
    IMS_BOOL IsAvpfSupported(IN VideoProfile* pProfile);
    IMS_BOOL GetCorrectImageIndex(IN IMS_SINT32 nPayloadTypeNum, IN ImsList<AString> objAttributes,
            OUT IMS_UINT32* nIndex);
    VIDEO_RESOLUTION GetResolutionFromSdp(IN VIDEO_CODEC codecType,
            IN const AString& strImageAttrFromSdp, IN const AString& strFrameSizeFromSdp,
            IN const AString& strSpropParam, IN IMS_SINT32 nQcif = -1);
    IMS_BOOL GetAvpfFromAttributes(IN SdpMediaFormat* pMediaFormat,
            IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr);
    IMS_BOOL GetAvpfFromAttributes_EX(
            IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr);
    IMS_BOOL GetWidthHeightFromSdp_ImageAttr(IN const AString& strImageAttrFromSdp,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
    VIDEO_RESOLUTION GetResolutionFromWidthHeight(IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight);
    IMS_BOOL GetWidthHeightFromSdp_FrameSize(IN AString strFrameSizeFromSdp,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
};

#endif
