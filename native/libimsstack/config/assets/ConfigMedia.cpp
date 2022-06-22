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
#include "ConfigMedia.h"

#define CONFIG_CAPABILITIES "capabilities"
#define CONFIG_MEDIA_CAPABILITIES "ims.media.capabilities"

PUBLIC GLOBAL const IMS_CHAR ConfigMedia::MEDIA_NAME[] = "media";
PUBLIC GLOBAL const IMS_CHAR ConfigMedia::MEDIA_CAPABILITIES_NAME[] = CONFIG_MEDIA_CAPABILITIES;

PUBLIC GLOBAL const IMS_CHAR ConfigMedia::MEDIA_CONFIG[] = {
        "[profiles]\n"
        "ids=" CONFIG_CAPABILITIES "," CONFIG_MEDIA_MTC "\n"
        CONFIG_CAPABILITIES "=" CONFIG_MEDIA_CAPABILITIES "\n"
        CONFIG_MEDIA_MTC "=" CONFIG_MEDIA_CAPABILITIES "\n"
        "\n"
};

PUBLIC GLOBAL const IMS_CHAR ConfigMedia::MEDIA_CAPABILITIES_CONFIG[] = {
        "[" CONFIG_CAPABILITIES "]\n"
        "stream_audio_count=3\n"
        "stream_audio_0=m=audio 0 RTP/AVP 96 100\n"
        "stream_audio_1=a=rtpmap:96 AMR/8000/1\n"
        "stream_audio_2=a=rtpmap:100 AMR-WB/16000/1\n"
        "stream_video_count=5\n"
        "stream_video_0=m=video 0 RTP/AVP 102 34\n"
        "stream_video_1=a=rtpmap:102 H264/90000\n"
        "stream_video_2=a=fmtp:102 profile-level-id=42C016\n"
        "stream_video_3=a=rtpmap:34 H263/90000\n"
        "stream_video_4=a=fmtp:34 profile=0; level=10\n"
        "\n"
        "[" CONFIG_MEDIA_MTC "]\n"
        "stream_audio_count=3\n"
        "stream_audio_0=m=audio 0 RTP/AVP 96 100\n"
        "stream_audio_1=a=rtpmap:96 AMR/8000/1\n"
        "stream_audio_2=a=rtpmap:100 AMR-WB/16000/1\n"
        "stream_video_count=5\n"
        "stream_video_0=m=video 0 RTP/AVP 102 34\n"
        "stream_video_1=a=rtpmap:102 H264/90000\n"
        "stream_video_2=a=fmtp:102 profile-level-id=42800D\n"
        "stream_video_3=a=rtpmap:34 H263/90000\n"
        "stream_video_4=a=fmtp:34 profile=0; level=10\n"
        "\n"
};
