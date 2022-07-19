/**
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

#ifndef _MEDIA_SESSION_CONFIG_FACTORY_H_
#define _MEDIA_SESSION_CONFIG_FACTORY_H_

// == INCLUDES =========================================================

#include "ImsMap.h"
#include "ImsList.h"
#include "config/MediaSessionConfig.h"

class MediaSessionConfigFactory
{
private:
    /**
     * @brief Construct a new media session config factory
     *
     */
    MediaSessionConfigFactory();

public:
    /**
     * @brief Destroy the media session config factory
     *
     */
    virtual ~MediaSessionConfigFactory();
    /**
     * @brief Create a media session config
     *
     * @param nSlotId SIM slot id - default : 0
     * @param eServiceType service type (ex: default, emergency)
     */
    void CreateMediaSessionConfig(IN IMS_SINT32 nSlotId, IN MEDIA_SERVICE_TYPE eServiceType);
    /**
     * @brief Add a media session config
     *
     * @param nSlotId SIM sloit id - default : 0
     * @param mediaSessionConfig media session config
     */
    void AddMediaSessionConfig(IN IMS_SINT32 nSlotId, IN MediaSessionConfig* mediaSessionConfig);
    /**
     * @brief Distroy the list session config
     *
     * @param nSlotId SIM sloit id - default : 0
     */
    void DestroyListSessionConfig(IN IMS_SINT32 nSlotId);
    /**
     * @brief Get the list session config
     *
     * @param nSlotId SIM sloit id - default : 0
     * @return IMSList<MediaSessionConfig*>* mediasession config list
     */
    IMSList<MediaSessionConfig*>* GetListSessionConfig(IN IMS_SINT32 nSlotId);
    /**
     * @brief Find a media session config
     *
     * @param nSlotId SIM sloit id - default : 0
     * @param eServiceType service type (ex: default, emergency)
     * @return MediaSessionConfig* mediasession config
     */
    MediaSessionConfig* FindMediaSessionConfig(
            IN IMS_SINT32 nSlotId, IN MEDIA_SERVICE_TYPE eServiceType);
    /**
     * @brief Destroy a media session config
     *
     * @param pMediaSessionConfig set mediasession config
     */
    void DestroySessionConfig(IN MediaSessionConfig* pMediaSessionConfig);
    /**
     * @brief Get the instance of the mediasession config
     *
     * @return MediaSessionConfigFactory* Return the mediasession config instance
     */
    static MediaSessionConfigFactory* GetInstance();
    /**
     * @brief Release the instance of the mediasession config
     *
     * @param pSessionConfigFactory mediasession config factory instance
     */
    static void ReleaseInstance(MediaSessionConfigFactory* pSessionConfigFactory);

private:
    IMSMap<IMS_UINT32, IMSList<MediaSessionConfig*>*> m_mapListMediaSessionConfig;
};

#endif /* _MEDIA_SESSION_CONFIG_FACTORY_H_ */
