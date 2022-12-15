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

#ifndef _IMS_MEDIA_CONFIG_UTIL_H_
#define _IMS_MEDIA_CONFIG_UTIL_H_

#include "MediaDef.h"
#include "MediaSessionConfigFactory.h"

class MediaConfigUtil
{
public:
    /**
     * @brief Get the AudioConfiguration object with given parameter
     *
     * @param nSlotId The UICC slot id
     * @param type MEDIA_SERVICE_TYPE defined in MediaDef.h
     * @return AudioConfiguration*
     */
    static AudioConfiguration* GetAudioConfig(IMS_SINT32 nSlotId, MEDIA_SERVICE_TYPE type)
    {
        return MediaSessionConfigFactory::GetInstance()
                ->FindMediaSessionConfig(nSlotId, type)
                ->GetAudioConfiguration();
    }

    /**
     * @brief Get the VideoConfiguration object with given parameter
     *
     * @param nSlotId The UICC slot id
     * @param type MEDIA_SERVICE_TYPE defined in MediaDef.h
     * @return VideoConfiguration*
     */
    static VideoConfiguration* GetVideoConfig(IMS_SINT32 nSlotId, MEDIA_SERVICE_TYPE type)
    {
        return MediaSessionConfigFactory::GetInstance()
                ->FindMediaSessionConfig(nSlotId, type)
                ->GetVideoConfiguration();
    }

    /**
     * @brief Get the TextConfiguration object with given parameter
     *
     * @param nSlotId The UICC slot id
     * @param type MEDIA_SERVICE_TYPE defined in MediaDef.h
     * @return TextConfiguration*
     */
    static TextConfiguration* GetTextConfig(IMS_SINT32 nSlotId, MEDIA_SERVICE_TYPE type)
    {
        return MediaSessionConfigFactory::GetInstance()
                ->FindMediaSessionConfig(nSlotId, type)
                ->GetTextConfiguration();
    }
};

#endif