/*
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

#ifndef MEDIA_SESSION_CONFIG_FACTORY_H_
#define MEDIA_SESSION_CONFIG_FACTORY_H_

#include "ImsMap.h"
#include "ImsList.h"
#include "config/MediaSessionConfig.h"

class MediaSessionConfigFactory
{
protected:
    MediaSessionConfigFactory();

public:
    virtual ~MediaSessionConfigFactory();

    /**
     * @brief Create a media session config and stack to the internal list.
     *
     * @param nSlotId SIM slot id - default : 0.
     * @param eServiceType The service type(ex: default, emergency).
     */
    virtual void CreateMediaSessionConfig(
            IN IMS_SINT32 nSlotId, IN MEDIA_SERVICE_TYPE eServiceType);

    /**
     * @brief Add a media session config to the internal list.
     *
     * @param nSlotId SIM Slot id - default : 0.
     * @param pMediaSessionConfig The media session config.
     */
    virtual void AddMediaSessionConfig(
            IN IMS_SINT32 nSlotId, IN MediaSessionConfig* pMediaSessionConfig);

    /**
     * @brief Destroy the session config list.
     *
     * @param nSlotId SIM Slot id - default : 0.
     */
    virtual void DestroyListSessionConfig(IN IMS_SINT32 nSlotId);

    /**
     * @brief Get the session config list.
     *
     * @param nSlotId SIM Slot id - default : 0.
     * @return ImsList<MediaSessionConfig*>* A mediasession config list.
     */
    virtual ImsList<MediaSessionConfig*>* GetListSessionConfig(IN IMS_SINT32 nSlotId);

    /**
     * @brief Find a media session config.
     *
     * @param nSlotId SIM Slot id - default : 0.
     * @param eServiceType service type (ex: default, emergency).
     * @return MediaSessionConfig* mediasession config.
     */
    virtual MediaSessionConfig* FindMediaSessionConfig(
            IN IMS_SINT32 nSlotId, IN MEDIA_SERVICE_TYPE eServiceType);

    /**
     * @brief Destroy a media session config in the internal list.
     *
     * @param pMediaSessionConfig The mediasession config to destroy.
     */
    virtual void DestroySessionConfig(IN const MediaSessionConfig* pMediaSessionConfig);

    /**
     * @brief Get the instance of the mediasession config.
     *
     * @return MediaSessionConfigFactory* Return the mediasession config instance.
     */
    static MediaSessionConfigFactory* GetInstance();

    /**
     * @brief Release the instance of the mediasession config.
     *
     * @param pSessionConfigFactory The mediasession config factory instance.
     */
    static void ReleaseInstance(MediaSessionConfigFactory* pSessionConfigFactory);

    /**
     * @brief Set the instance of the mediasession config.
     */
    static void SetInstance(MediaSessionConfigFactory* pSessionConfigFactory);

private:
    void DestroyListSessionConfigImpl(IN IMS_SINT32 nSlotId);
    ImsMap<IMS_UINT32, ImsList<MediaSessionConfig*>*> m_mapListMediaSessionConfig;
};

#endif
