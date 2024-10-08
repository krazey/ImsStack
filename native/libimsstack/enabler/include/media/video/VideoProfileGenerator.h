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

#ifndef VIDEO_PROFILE_GENERATOR_H_
#define VIDEO_PROFILE_GENERATOR_H_

#include "MediaProfileGenerator.h"
#include "video/VideoProfileUtil.h"

class CodecVideoConfig;
class VideoConfiguration;

/**
 * This class is to generate a video profile by parsing a video configuration
 */
class VideoProfileGenerator : public MediaProfileGenerator
{
public:
    VideoProfileGenerator();
    virtual ~VideoProfileGenerator();

protected:
    VideoProfile* SetProfile(IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig,
            IN MediaEnvironment* pEnvironment, IN IMS_SINT32 nSlotId) override;
    void CreateCodecPayloads(IN MediaBaseProfile* pProfile, IN IMS_SINT32 nCodec,
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig) override;
    VideoProfile::Payload* CreateAvcPayload(
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig);
    VideoProfile::Payload* CreateHevcPayload(
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig);
    void SetVideoCodecFmtp(IN CodecVideoConfig* pCodecConfig, IN VideoConfiguration* pVideoConfig,
            OUT VideoProfile::VideoFmtp* pFmtp);
    void SetVideoCodecPayload(IN CodecVideoConfig* pCodecConfig,
            IN VideoConfiguration* pVideoConfig, OUT VideoProfile::Payload* pPayload);
    IMS_SINT32 SetTransportCapability(OUT VideoProfile* pVideoProfile);
    IMS_SINT32 SetAttributeCapability(
            OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig);
    IMS_SINT32 SetAvpfTrr(OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig,
            IN IMS_SINT32 nAcap);
    IMS_SINT32 SetAvpfNack(OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig,
            IN IMS_SINT32 nAcap);
    IMS_SINT32 SetAvpfPli(OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig,
            IN IMS_SINT32 nAcap);
    IMS_SINT32 SetAvpfFir(OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig,
            IN IMS_SINT32 nAcap);
    IMS_SINT32 SetAvpfTmmbr(OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig,
            IN IMS_SINT32 nAcap);
    void SetCapaNegoForAvpf(OUT VideoProfile* pVideoProfile, IN IMS_SINT32 nCapaNegoForAvpfOption,
            IN IMS_SINT32 nTcap, IN IMS_SINT32 nAcap);
    void SetMaxProfileFrameRate(OUT VideoProfile* pVideoProfile);
};

#endif
