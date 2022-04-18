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

#include "config/ImsCodec.h"

PUBLIC GLOBAL
const IMS_CHAR* ImsCodec::CodecToString(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case AUDIO_AMR:
            return "AMR";
        case AUDIO_AMR_WB:
            return "AMR-WB";
        case AUDIO_PCMA:
            return "PCMA";
        case AUDIO_PCMU:
            return "PCMU";
        case AUDIO_TELEPHONE_EVENT:
        case AUDIO_TELEPHONE_EVENT_WB:
            return "telephone-event";
        case AUDIO_EVS:
            return "EVS";
        case VIDEO_AVC:
            return "AVC";
        case VIDEO_HEVC:
            return "HEVC";
        case TEXT_T140:
            return "t140";
        case TEXT_RED:
            return "red";
        default:
            return "";
    }
}
