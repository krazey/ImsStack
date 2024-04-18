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

// == INCLUDES =========================================================

class MediaConfiguration;
class MediaEnvironment;
class MediaResourceManager;
class CodecConfig;

#include "text/TextProfile.h"

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
     * @brief Create a MediaBaseProfile with the MediaEnvironment and MediaConfiguration
     *
     * @param pEnvironment The network connection parameter
     * @param pConfig The carrier configuration for text
     * @param nSlotId The UICC slot id
     * @param eType The media profile type to be created
     * @return MediaBaseProfile* The MediaBaseProfile created
     */
    MediaBaseProfile* CreateProfile(IN MediaEnvironment* pEnvironment,
            IN MediaConfiguration* pConfig, IN IMS_SINT32 nSlotId, IN MEDIA_CONTENT_TYPE eType);

    /**
     * @brief Delete the profile
     *
     * @param pProfile The media profile to be delete
     */
    void DeleteProfile(IN MediaBaseProfile* pProfile);

    /**
     * @brief Distroy the list session config
     *
     * @param nSlotId SIM sloit id - default : 0
     */
    static MediaProfileFactory* GetInstance();

    /**
     * @brief Release the instance of the mediasession config
     *
     * @param pMediaProfileFactory mediasession config factory instance
     */
    static void ReleaseInstance(MediaProfileFactory* pMediaProfileFactory);

private:
    TextProfile* CreateTextProfile();
    TextProfile::Payload* CreateT140Payload(
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig);
    TextProfile* SetTextProfile(IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig);
};

#endif
