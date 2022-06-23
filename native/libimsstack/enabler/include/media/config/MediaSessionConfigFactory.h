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
    MediaSessionConfigFactory();

public:
    virtual ~MediaSessionConfigFactory();

    void CreateMediaSessionConfig(IN IMS_SINT32 nSlotId, IN MEDIA_SERVICE_TYPE eServiceType);
    void AddMediaSessionConfig(IN IMS_SINT32 nSlotId, IN MediaSessionConfig* mediaSessionConfig);
    void DestroyListSessionConfig(IN IMS_SINT32 nSlotId);
    IMSList<MediaSessionConfig*>* GetListSessionConfig(IN IMS_SINT32 nSlotId);
    MediaSessionConfig* FindMediaSessionConfig(
            IN IMS_SINT32 nSlotId, IN MEDIA_SERVICE_TYPE eServiceType);
    void DestroySessionConfig(IN MediaSessionConfig* pMediaSessionConfig);

    static MediaSessionConfigFactory* GetInstance();
    static void ReleaseInstance(MediaSessionConfigFactory* pSessionConfigFactory);

private:
    IMSMap<IMS_UINT32, IMSList<MediaSessionConfig*>*> m_mapListMediaSessionConfig;
};

#endif /* _MEDIA_SESSION_CONFIG_FACTORY_H_ */
