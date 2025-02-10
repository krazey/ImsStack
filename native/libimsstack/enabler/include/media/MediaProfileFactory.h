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

#ifndef MEDIA_PROFILE_FACTORY_H_
#define MEDIA_PROFILE_FACTORY_H_

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

class MediaProfileFactory
{
private:
    /**
     * @brief Construct a new media profile factory
     *
     */
    MediaProfileFactory();

public:
    /**
     * @brief Destroy the media profile factory
     *
     */
    virtual ~MediaProfileFactory();

    /**
     * @brief Create each media(audio/text/video) profile
     *
     * @param eType The media profile type to be created
     * @param pProfile if not null, this profile will be copied to the media profile just created
     * @return MediaBaseProfile* The media profile created
     */
    MediaBaseProfile* CreateProfile(
            IN MEDIA_CONTENT_TYPE eType, IN MediaBaseProfile* pProfile = IMS_NULL);

    /**
     * @brief Delete the profile
     *
     * @param pProfile The media profile to be deleted
     */
    void DeleteProfile(IN MediaBaseProfile* pProfile);

    /**
     * @brief Create a media(audio/text/video) codec payload
     *
     * @param eType The media codec payload type to be created
     * @return MediaBaseProfile::BasePayload* The media(audio/text/video) codec payload created will
     * be returned as its parent class type
     */
    MediaBaseProfile::BasePayload* CreatePayload(IN MEDIA_CONTENT_TYPE eType);

    /**
     * @brief Create a media(audio/text/video) codec payload
     *
     * @param payload The media codec payload to be created and copied
     * @return MediaBaseProfile::BasePayload* The media(audio/text/video) codec payload created will
     * be returned as its parent class type
     */
    MediaBaseProfile::BasePayload* CreatePayload(IN MediaBaseProfile::BasePayload* payload);

    /**
     * @brief Delete the payload
     *
     * @param pPayload The media codec payload to be deleted
     */
    void DeletePayload(IN MediaBaseProfile::BasePayload* pPayload);

    /**
     * @brief Get the instance of MediaProfileFactory
     *
     * @return MediaProfileFactory* Return the MediaProfileFactory instance
     */
    static MediaProfileFactory* GetInstance();

    /**
     * @brief Release the instance of the MediaProfileFactory
     *
     * @param pMediaProfileFactory the MediaProfileFactory instance
     */
    static void ReleaseInstance(MediaProfileFactory* pMediaProfileFactory);

private:
    AudioProfile* CreateAudioProfile();
    VideoProfile* CreateVideoProfile();
    TextProfile* CreateTextProfile();

    AudioProfile::Payload* CreateAudioPayload(IN AudioProfile::Payload* payload = IMS_NULL);
    TextProfile::Payload* CreateTextPayload(IN TextProfile::Payload* payload = IMS_NULL);
    VideoProfile::Payload* CreateVideoPayload(IN VideoProfile::Payload* payload = IMS_NULL);
};

#endif
