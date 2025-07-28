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

#ifndef MEDIA_PROFILE_UTIL_H_
#define MEDIA_PROFILE_UTIL_H_

#include "AString.h"
#include "MediaDef.h"

class MediaBaseProfile;
class MediaConfiguration;

/**
 * Thi class is an utility class for MediaProfile
 */
class MediaProfileUtil
{
public:
    /**
     * @brief Get media type (audio/text/video) from the payload type.
     *
     * @param payloadType media codec payload type
     * @return MEDIA_CONTENT_TYPE The media type (audio/text/video)
     */
    static MEDIA_CONTENT_TYPE GetMediaType(IN const AString payloadType);

    /**
     * @brief Set RTCP RS/RR from the media configuration
     *
     * @param pProfile target profile that RS/RR to be set
     * @param pConfig media configuration
     * @param bDirHold direction flag if the media direction is not sendrecv, this flag is IMS_TRUE,
     * otherwise IMS_FALSE
     */
    static void SetRtcpRsRr(OUT MediaBaseProfile* pProfile, IN const MediaConfiguration* pConfig,
            IN IMS_BOOL bDirHold);
};

#endif
