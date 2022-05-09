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

#ifndef _IMS_CODEC_H_
#define _IMS_CODEC_H_

#include "AString.h"

class ImsCodec
{
public:
    // Convert codec enum to string
    static const IMS_CHAR* CodecToString(IN IMS_SINT32 nType);

public:
    // Audio Codecs
    enum
    {
        AUDIO_NONE = 0,
        AUDIO_AMR = 1,
        AUDIO_AMR_WB,
        AUDIO_PCMA,
        AUDIO_PCMU,
        AUDIO_TELEPHONE_EVENT,
        AUDIO_TELEPHONE_EVENT_WB,
        AUDIO_EVS,
        AUDIO_MAX = 99
    };

    // Video Codecs
    enum
    {
        VIDEO_NONE = 100,
        VIDEO_AVC = 101,
        VIDEO_HEVC,
        VIDEO_MAX = 199
    };

    // Text Codecs
    enum
    {
        TEXT_NONE = 200,
        TEXT_T140 = 201,
        TEXT_RED = 202,
        TEXT_MAX = 299
    };
};
#endif  // _IMS_CODEC_H_
