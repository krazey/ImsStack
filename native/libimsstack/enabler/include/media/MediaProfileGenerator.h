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

#include "IService.h"
#include "MediaBaseProfile.h"

class CodecConfig;
class MediaConfiguration;

/**
 * This class is to generate a MediaBaseProfile
 */
class MediaProfileGenerator
{
public:
    explicit MediaProfileGenerator(IN const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_NOTUSED);
    virtual ~MediaProfileGenerator();

    /**
     * @brief Generate a MediaBaseProfile
     *
     * @param eServiceType The service type to access the session configuration
     * @param pIService The object to get the network parameters
     * @param pConfig The carrier configuration for media
     * @param nSlotId The UICC slot id
     * @return MediaBaseProfile* The MediaBaseProfile to be created
     */
    MediaBaseProfile* Generate(MEDIA_SERVICE_TYPE eServiceType, IN IService* pIService,
            IN MediaConfiguration* pConfig, IN IMS_SINT32 nSlotId);

protected:
    /**
     * @brief set each media(audio/text/video) profile
     *
     * @param pProfile if not null, this profile will be copied to the media profile just created
     * @param pConfig The carrier configuration for media
     * @param eServiceType The service type to access the session configuration
     * @param pIService The object to get the network parameters
     * @param nSlotId The UICC slot id
     * @return MediaBaseProfile* The media profile created
     */
    virtual MediaBaseProfile* SetProfile(IN MediaBaseProfile* pProfile,
            IN MediaConfiguration* pConfig, MEDIA_SERVICE_TYPE eServiceType, IN IService* pIService,
            IN IMS_SINT32 nSlotId) = 0;
    void SetCommonProfile(IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig,
            IN IService* pIService, IN IMS_SINT32 nSlotId);
    MediaBaseProfile* SetPayloads(IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig);
    virtual void CreateCodecPayloads(IN MediaBaseProfile* pProfile, IN IMS_SINT32 nCodec,
            IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig) = 0;

protected:
    MEDIA_CONTENT_TYPE m_eType;
};
#endif
