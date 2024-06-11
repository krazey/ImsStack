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
     * @brief Check if this payload type is the audio
     *
     * @param payloadType media codec payload type
     * @return IMS_TRUE if this payload type is the audio, otherwise IMS_FALSE
     */
    static IMS_BOOL IsAudioType(IN const AString payloadType);

    /**
     * @brief Check if this payload type is the text
     *
     * @param payloadType media codec payload type
     * @return IMS_TRUE if this payload type is the text, otherwise IMS_FALSE
     */
    static IMS_BOOL IsTextType(IN const AString payloadType);

    /**
     * @brief Check if this payload type is the video
     *
     * @param payloadType media codec payload type
     * @return IMS_TRUE if this payload type is the video, otherwise IMS_FALSE
     */
    static IMS_BOOL IsVideoType(IN const AString payloadType);
};

#endif
