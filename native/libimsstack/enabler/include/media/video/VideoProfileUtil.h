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

#ifndef VIDEO_PROFILE_UTIL_H_
#define VIDEO_PROFILE_UTIL_H_

#include "MediaEnvironment.h"
#include "video/VideoProfile.h"

#define VIDEO_MAX_CONFIG_LEN 256

class VideoConfiguration;

class VideoProfileUtil
{
public:
    static VideoProfile* CreateProfile(IN MediaEnvironment* pEnvironment,
            IN VideoConfiguration* pConfig, IN IMS_SINT32 nSlotId);
    /**
     * @brief UpdateAudioProfile for IP changes or IP setting latency
     *
     * @param pVideoProfile
     * @param pEnvironment
     * @return IMS_BOOL
     */
    static IMS_BOOL UpdateVideoProfile(
            OUT VideoProfile* pVideoProfile, IN MediaEnvironment* pEnvironment);
    static void GetWidthHeightFromResolution(
            IN VIDEO_RESOLUTION eResolIc, IN IMS_UINT32* nWidth, IN IMS_UINT32* nHeight);
    static VIDEO_RESOLUTION GetResolutionFromWidthHeight(
            IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight);
    static VIDEO_PROFILE_AVC GetAvcProfileFromProfileLevelId(IN const AString& strProfileLevelId);
    static IMS_UINT32 GetAvcLevelFromProfileLevelId(IN const AString& strProfileLevelId);
};

#endif
