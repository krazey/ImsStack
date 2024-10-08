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

#ifndef MEDIA_PROFILE_GENERATOR_H_
#define MEDIA_PROFILE_GENERATOR_H_

class CodecConfig;
class CodecAudioConfig;
class CodecVideoConfig;
class AudioConfiguration;
class MediaConfiguration;
class MediaEnvironment;
class MediaResourceManager;
class VideoConfiguration;

#include "audio/AudioProfile.h"
#include "text/TextProfile.h"
#include "video/VideoProfile.h"

/**
 * This class is to generate a MediaBaseProfile by using the MediaEnvironment and
 * the MediaConfiguration
 */
class MediaProfileGenerator
{
public:
    explicit MediaProfileGenerator(IN const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_NOTUSED);
    virtual ~MediaProfileGenerator();

    /**
     * @brief Generate a MediaBaseProfile by using the MediaEnvironment and the
     * MediaConfiguration
     *
     * @param pEnvironment The network connection parameter
     * @param pConfig The carrier configuration for media
     * @param nSlotId The UICC slot id
     * @return MediaBaseProfile* The MediaBaseProfile to be created
     */
    MediaBaseProfile* Generate(IN MediaEnvironment* pEnvironment, IN MediaConfiguration* pConfig,
            IN IMS_SINT32 nSlotId);

    /**
     * @brief set each media(audio/text/video) profile
     *
     * @param pProfile if not null, this profile will be copied to the media profile just created
     * @param pConfig The carrier configuration for media
     * @param pEnvironment The network connection parameter
     * @param nSlotId The UICC slot id
     * @return MediaBaseProfile* The media profile created
     */
    virtual MediaBaseProfile* SetProfile(IN MediaBaseProfile* pProfile,
            IN MediaConfiguration* pConfig, IN MediaEnvironment* pEnvironment,
            IN IMS_SINT32 nSlotId) = 0;

protected:
    void SetCommonProfile(IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig,
            IN MediaEnvironment* pEnvironment, IN IMS_SINT32 nSlotId);

private:
    MediaBaseProfile* CreateCodecPayloads(
            IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig);
    AudioProfile::Payload* CreateAmrPayload(
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig);
    AudioProfile::Payload* CreateEvsPayload(
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig);
    AudioProfile::Payload* CreateTelephoneEventPayload(
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig);
    AudioProfile::Payload* CreatePcmPayload(
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig);
    VideoProfile::Payload* CreateAvcPayload(
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig);
    VideoProfile::Payload* CreateHevcPayload(
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig);
    void SetAudioCodecFmtp(IN CodecAudioConfig* pCodecConfig, IN AudioConfiguration* pAudioConfig,
            OUT AudioProfile::AudioFmtp* pFmtp);
    void SetVideoCodecFmtp(IN CodecVideoConfig* pCodecConfig, IN VideoConfiguration* pVideoConfig,
            OUT VideoProfile::VideoFmtp* pFmtp);
    void SetVideoCodecPayload(IN CodecVideoConfig* pCodecConfig,
            IN VideoConfiguration* pVideoConfig, OUT VideoProfile::Payload* pPayload);
    TextProfile::Payload* CreateT140Payload(
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig);

protected:
    MEDIA_CONTENT_TYPE m_eType;
};
#endif
