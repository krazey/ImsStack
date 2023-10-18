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

#ifndef VIDEO_DEF_H_
#define VIDEO_DEF_H_

typedef enum
{
    VIDEO_CODEC_NONE = 0,
    VIDEO_CODEC_AVC,
    VIDEO_CODEC_HEVC,
    VIDEO_CODEC_NOTUSED
} VIDEO_CODEC;

typedef enum
{
    VIDEO_RESOLUTION_INVALID = 0,
    VIDEO_RESOLUTION_QCIF_LS,  // QCIF Landscape (LGU+ group VT)
    VIDEO_RESOLUTION_QVGA_LS,  // QVGA Landscape
    VIDEO_RESOLUTION_QVGA_PR,  // QVGA Portrait
    VIDEO_RESOLUTION_VGA_LS,   // VGA Landscape
    VIDEO_RESOLUTION_VGA_PR,   // VGA Portrait
    VIDEO_RESOLUTION_CIF_LS,   // CIF Landscape
    VIDEO_RESOLUTION_CIF_PR,   // COF Portrait
    VIDEO_RESOLUTION_QCIF_PR,  // QCIF Portrait (base)
    VIDEO_RESOLUTION_SQCIF_LS,
    VIDEO_RESOLUTION_SQCIF_PR,
    VIDEO_RESOLUTION_SIF_LS,
    VIDEO_RESOLUTION_SIF_PR,
    VIDEO_RESOLUTION_HD_LS,
    VIDEO_RESOLUTION_HD_PR,
    VIDEO_RESOLUTION_FHD_LS,
    VIDEO_RESOLUTION_FHD_PR,
    VIDEO_RESOLUTION_CUST = 98,
    VIDEO_RESOLUTION_NOT_USED = 99,
} VIDEO_RESOLUTION;

typedef enum  // AVC profile
{
    AVC_PROFILE_NONE = 0,
    AVC_PROFILE_CB,  // constrained baseline profile
    AVC_PROFILE_B,   // baseline profile
    AVC_PROFILE_M,   // main profile
    AVC_PROFILE_E,   // extended profile
    AVC_PROFILE_H,   // high profile
    AVC_PROFILE_NOT_USED
} VIDEO_PROFILE_AVC;

typedef enum  // HEVC profile
{
    HEVC_PROFILE_NONE = 0,
    HEVC_PROFILE_MAIN,
    HEVC_PROFILE_MAIN10,
    HEVC_PROFILE_NOT_USED
} VIDEO_PROFILE_HEVC;

#endif
