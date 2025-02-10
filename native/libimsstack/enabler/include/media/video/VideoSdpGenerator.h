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

#ifndef VIDEO_SDP_GENERATOR_H_
#define VIDEO_SDP_GENERATOR_H_

#include "MediaSdpGenerator.h"
#include "video/VideoProfile.h"

class SdpAvCodec;

/**
 * This class is to generate a video Sdp by adding video attributes from video profile to the
 * MediaDescriptor and the SessionDescriptor
 */
class VideoSdpGenerator : public MediaSdpGenerator
{
public:
    VideoSdpGenerator();
    virtual ~VideoSdpGenerator();

    IMS_BOOL Generate(OUT ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MediaBaseProfile* pBaseProfile) override;

protected:
    void GeneratePayload(OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile);
    void CheckRtcpFbWildCard(IN VideoProfile* pProfile, OUT IMS_BOOL& bTrrSupportedAll,
            OUT IMS_BOOL& bNackSupportedAll, OUT IMS_BOOL& bPliSupportedAll,
            OUT IMS_BOOL& bFirSupportedAll, OUT IMS_BOOL& bTmmbrSupportedAll);
    IMS_BOOL GenerateFmtp(OUT AString& strCompletedFmtp, IN VideoProfile::Payload* pPayload);
    IMS_BOOL GenerateCompletedFmtpRtpMap(IN const AString& strRtpMap,
            IN const AString& strPayloadNum, IN const AString& strFmtp, OUT SdpAvCodec* pFormat);
    void GenerateImageAttribute(
            OUT IMediaDescriptor* pDescriptor, IN VideoProfile::Payload* pPayload);
    void GenerateFrameSize(OUT IMediaDescriptor* pDescriptor, IN VideoProfile::Payload* pPayload);
    void GenerateRtcpFb(IN VideoProfile* pProfile, IN IMS_BOOL bTrrSupportedAll,
            IN IMS_BOOL bNackSupportedAll, IN IMS_BOOL bPliSupportedAll,
            IN IMS_BOOL bFirSupportedAll, IN IMS_BOOL bTmmbrSupportedAll, OUT SdpAvCodec* pFormat,
            IN IMS_UINT32 nPayloadIndex);
    void GenerateRtcpFbTrrInt(OUT SdpAvCodec* pFormat, IN VideoProfile::Payload* pPayload,
            IN IMS_BOOL bSupportedInAllPayload, IN IMS_UINT32 nPayloadIndex);
    void GenerateRtcpFbNack(OUT SdpAvCodec* pFormat, IN VideoProfile::Payload* pPayload,
            IN IMS_BOOL bSupportedInAllPayload, IN IMS_UINT32 nPayloadIndex);
    void GenerateRtcpFbPli(OUT SdpAvCodec* pFormat, IN VideoProfile::Payload* pPayload,
            IN IMS_BOOL bSupportedInAllPayload, IN IMS_UINT32 nPayloadIndex);
    void GenerateRtcpFbFir(OUT SdpAvCodec* pFormat, IN VideoProfile::Payload* pPayload,
            IN IMS_BOOL bSupportedInAllPayload, IN IMS_UINT32 nPayloadIndex);
    void GenerateRtcpFbTmmbr(OUT SdpAvCodec* pFormat, IN VideoProfile::Payload* pPayload,
            IN IMS_BOOL bSupportedInAllPayload, IN IMS_UINT32 nPayloadIndex);
    void GenerateFrameRate(OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile);
    void GenerateCvo(OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile);
    void GenerateCapaNegoAttribute(OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile);
    void GenerateAcfg(
            OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile::CapaNego& objCapaNego);
    void GenerateTcap(
            OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile::CapaNego& objCapaNego);
    void GenerateAcap(
            OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile::CapaNego& objCapaNego);
    void GeneratePcfg(
            OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile::CapaNego& objCapaNego);

    IMS_BOOL MakeImageAttributeLine(IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId,
            OUT AString& strImageAttr);
    IMS_BOOL MakeFrameSizeLine(IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId,
            OUT AString& strFrameSize);
    /**
     * @brief Get the width and height from video resolution enum id
     *
     * @param eResolutionId The enum of video resolution set
     * @param pnWidth The width of video resolution
     * @param pnHeight The height of video resolution
     * @return IMS_BOOL
     */
    IMS_BOOL GetWidthHeightFromResolutionId(
            IN VIDEO_RESOLUTION eResolutionId, OUT IMS_UINT32* pnWidth, OUT IMS_UINT32* pnHeight);

    AString GenerateAvcFmtp(IN VideoProfile::AvcFmtp* pAvcFmtp);
    void AddProfileLevelIdToFmtp(IN VideoProfile::AvcFmtp* pFmtp, OUT AString& strFmtp);
    void AddPacketizationModeToFmtp(IN VideoProfile::VideoFmtp* pFmtp, OUT AString& strFmtp);
    void AddSpropParameterSetsToFmtp(IN VideoProfile::VideoFmtp* pFmtp, OUT AString& strFmtp);

    AString GenerateHevcFmtp(IN VideoProfile::HevcFmtp* pHevcFmtp);
    void AddProfileIdToFmtp(IN VideoProfile::HevcFmtp* pFmtp, OUT AString& strFmtp);
    void AddLevelIdToFmtp(IN VideoProfile::HevcFmtp* pFmtp, OUT AString& strFmtp);
    void AddSpropParamsToFmtp(IN VideoProfile::HevcFmtp* pFmtp, OUT AString& strFmtp);
};

#endif
