/*
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

#ifndef VIDEO_SDP_PARSER_H_
#define VIDEO_SDP_PARSER_H_

#include "MediaSdpParser.h"
#include "VideoProfile.h"
#include "offeranswer/SdpAvCodec.h"

/**
 * This class is to generate a peer video profile by parsing SDP media attributes from the
 * MediaDescriptor and the SessionDescriptor
 */
class VideoSdpParser : public MediaSdpParser
{
public:
    explicit VideoSdpParser();
    virtual ~VideoSdpParser() override;

    /**
     * @brief It is the core function responsible for extracting video-related attributes from
     * the SDP (Session Description Protocol) message. It takes the SDP data and populates a
     * VideoProfile object with the parsed parameters
     *
     * @param pSessionDescriptor This pointer provides access to the overall SDP session description
     * @param pDescriptor This pointer contains the SDP media description specifically for the video
     * stream
     * @param pProfile This is an output parameter. The method will populate this object with the
     * parsed video parameters.
     * @return IMS_BOOL Returns IMS_FALSE when error handling to gracefully manage situations where
     * the SDP message is malformed or missing crucial video parameters.
     */
    virtual IMS_BOOL Parse(IN ISessionDescriptor* pSessionDescriptor,
            IN IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile);

private:
    void ParseTransportType(IN const IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile);
    void SetAvpfSupport(OUT VideoProfile* pProfile);
    void ParsePayloads(IN const IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile);
    void ParseRtpMap(IN const SdpAvCodec* pSdpCodec, OUT VideoProfile::Payload* pPayload);
    VIDEO_CODEC SetCodec(IN VideoProfile::Payload* pPayload);
    IMS_BOOL IsValidCodec(IN const VIDEO_CODEC eVideoCodec);
    AString ParseImageAttr(IN const SdpAvCodec* pSdpCodec,
            IN const ImsList<AString>& objImageAttributes, OUT VideoProfile::Payload* pPayload);
    AString ParseFrameSize(IN const SdpAvCodec* pSdpCodec, IN const ImsList<AString>& objFrameSizes,
            OUT VideoProfile::Payload* pPayload);
    void ParseCvo(IN const IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile);
    IMS_BOOL ParseFmtp(IN const SdpAvCodec* pSdpCodec, OUT VideoProfile::Payload* pPayload,
            IN const VIDEO_CODEC eVideoCodec);
    IMS_BOOL ParseVideoBaseFmtp(IN const ImsList<AString>& objSplitEqual,
            OUT std::shared_ptr<VideoProfile::VideoFmtp> pFmtp);
    void ParseAvcFmtp(IN const ImsList<AString>& objSplitEqual, IN const AString& strSpropParam,
            OUT std::shared_ptr<VideoProfile::AvcFmtp> pFmtp);
    void ParseHevcFmtp(IN const ImsList<AString>& objSplitEqual, OUT AString& strVps,
            OUT AString& strSps, OUT AString& strPps,
            OUT std::shared_ptr<VideoProfile::HevcFmtp> pFmtp);
    IMS_BOOL ParsePacketizationMode(IN const ImsList<AString>& objSplitEqual,
            OUT std::shared_ptr<VideoProfile::VideoFmtp> pFmtp);
    IMS_BOOL ParseProfileLevelId(IN const ImsList<AString>& objSplitEqual,
            OUT std::shared_ptr<VideoProfile::AvcFmtp> pFmtp);
    IMS_BOOL ParseSpropParameterSets(IN const ImsList<AString>& objSplitEqual,
            IN const AString& strSpropParam, OUT std::shared_ptr<VideoProfile::AvcFmtp> pFmtp);
    IMS_BOOL ParseProfileId(IN const ImsList<AString>& objSplitEqual,
            OUT std::shared_ptr<VideoProfile::HevcFmtp> pFmtp);
    IMS_BOOL ParseLevelId(IN const ImsList<AString>& objSplitEqual,
            OUT std::shared_ptr<VideoProfile::HevcFmtp> pFmtp);
    IMS_BOOL ParseVps(IN const ImsList<AString>& objSplitEqual, OUT AString& strVps);
    IMS_BOOL ParseSps(IN const ImsList<AString>& objSplitEqual, OUT AString& strSps);
    IMS_BOOL ParsePps(IN const ImsList<AString>& objSplitEqual, OUT AString& strPps);
    void ParseSpropParam(IN const AString& strVps, IN const AString& strSps,
            IN const AString& strPps, OUT std::shared_ptr<VideoProfile::VideoFmtp> pFmtp);
    void ParseResolution(OUT VideoProfile::Payload* pPayload, const AString& strImageAttr,
            const AString& strFrameSize, VIDEO_CODEC eVideoCodec);
    void ParseAvpfAttribute(IN const SdpAvCodec* pSdpCodec, IN VideoProfile::Payload* pPayload,
            OUT VideoProfile* pProfile);
    IMS_BOOL IsAvpfSupported(IN VideoProfile* pProfile);
    IMS_BOOL GetCorrectImageIndex(IN IMS_SINT32 nPayloadTypeNum, IN ImsList<AString> objAttributes,
            OUT IMS_UINT32* nIndex);
    VIDEO_RESOLUTION GetResolutionFromSdp(IN VIDEO_CODEC codecType,
            IN const AString& strImageAttrFromSdp, IN const AString& strFrameSizeFromSdp,
            IN const AString& strSpropParam, IN IMS_SINT32 nQcif = -1);
    IMS_BOOL GetAvpfFromAttributes(IN const SdpMediaFormat* pMediaFormat,
            IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr);
    IMS_BOOL GetAvpfFromAttributesEx(
            IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr);
    IMS_BOOL GetWidthHeightFromSdpImageAttr(IN const AString& strImageAttrFromSdp,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
    VIDEO_RESOLUTION GetResolutionFromWidthHeight(IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight);
    IMS_BOOL GetWidthHeightFromSdpFrameSize(IN AString strFrameSizeFromSdp,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
};

#endif
