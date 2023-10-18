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

#ifndef IMS_CODEC_H_
#define IMS_CODEC_H_

#include "AString.h"

class ImsCodec
{
public:
    /**
     * @brief Convert codec enum to string
     *
     * @param nType Enum index of the codec
     * @return const IMS_CHAR* Return the string format of codec
     */
    static const IMS_CHAR* CodecToString(IN IMS_SINT32 nType);

public:
    /** Audio Codecs */
    enum
    {
        /** Enum for the audio codec default */
        AUDIO_NONE = 0,
        /** Enum for the audio codec AMR */
        AUDIO_AMR = 1,
        /** Enum for the audio codec AMR_WB */
        AUDIO_AMR_WB,
        /** Enum for the audio codec PCMA */
        AUDIO_PCMA,
        /** Enum for the audio codec PCMU */
        AUDIO_PCMU,
        /** Enum for the audio codec TELEPHONE_EVENT */
        AUDIO_TELEPHONE_EVENT,
        /** Enum for the audio codec TELEPHONE_EVENT_WB */
        AUDIO_TELEPHONE_EVENT_WB,
        /** Enum for the audio codec EVS */
        AUDIO_EVS,
        /** Enum for max */
        AUDIO_MAX = 99
    };

    /** Video Codecs */
    enum
    {
        /** Enum for the video codec default */
        VIDEO_NONE = 100,
        /** Enum for the video codec AVC (H.264) */
        VIDEO_AVC = 101,
        /** Enum for the video codec HEVC (H.265) */
        VIDEO_HEVC,
        /** Enum for max */
        VIDEO_MAX = 199
    };

    /** Text Codecs */
    enum
    {
        /** Enum for the text codec default */
        TEXT_NONE = 200,
        /** Enum for the text codec T140 */
        TEXT_T140 = 201,
        /** Enum for the text codec Redundancy */
        TEXT_RED = 202,
        /** Enum for max */
        TEXT_MAX = 299
    };
};

#endif
