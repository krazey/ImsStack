/*
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

#include "MediaDef.h"

class AString;
class MediaBaseProfile;
class MediaConfiguration;

/**
 * @brief Provides static utility functions for handling `MediaBaseProfile` objects.
 *
 * This class offers helper methods for common operations related to media profiles,
 * such as determining media types from codecs and configuring RTCP parameters.
 */
class MediaProfileUtil
{
public:
    /**
     * @brief Gets the media type (e.g., audio, video, text) from a codec payload type string.
     *
     * @param payloadType The name of the codec payload (e.g., "AMR-WB", "H264").
     * @return MEDIA_CONTENT_TYPE The corresponding media type, or `MEDIA_TYPE_INVALID` if not
     * found.
     */
    static MEDIA_CONTENT_TYPE GetMediaType(IN const AString payloadType);

    /**
     * @brief Sets the RTCP sender (RS) and receiver (RR) bandwidth modifiers on a media profile.
     *
     * This method adjusts the RS/RR values based on the media configuration and whether the
     * session is on hold. If on hold, it may apply default values if specific hold-related
     * configurations are not set.
     *
     * @param pProfile The `MediaBaseProfile` to modify.
     * @param pConfig The `MediaConfiguration` containing the bandwidth settings.
     * @param bDirHold `IMS_TRUE` if the media direction is something other than send-receive (e.g.,
     * on hold), `IMS_FALSE` otherwise.
     */
    static void SetRtcpRsRr(OUT MediaBaseProfile* pProfile, IN const MediaConfiguration* pConfig,
            IN IMS_BOOL bDirHold);
};

#endif
