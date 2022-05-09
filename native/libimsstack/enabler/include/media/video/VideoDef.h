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

#ifndef _IMS_VIDEO_DEF_H_
#define _IMS_VIDEO_DEF_H_

typedef enum
{
    VIDEO_CODEC_NONE = 0,
    VIDEO_CODEC_H264,
    VIDEO_CODEC_H263,
    VIDEO_CODEC_H265,
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

typedef enum  // H264 profile
{
    H264_PROFILE_NONE = 0,
    H264_PROFILE_CB,  // constrained baseline profile
    H264_PROFILE_B,   // baseline profile
    H264_PROFILE_M,   // main profile
    H264_PROFILE_E,   // extended profile
    H264_PROFILE_H,   // high profile
    H264_PROFILE_NOT_USED
} VIDEO_H264PROFILE;
typedef enum  // H265 profile
{
    H265_PROFILE_NONE = 0,
    H265_PROFILE_MAIN,
    H265_PROFILE_MAIN10,
    H265_PROFILE_NOT_USED
} VIDEO_H265PROFILE;

typedef enum
{
    VIDEO_VIEWTYPE_INVALID = -1,
    VIDEO_VIEWTYPE_NEAR,
    VIDEO_VIEWTYPE_FAR,
    VIDEO_VIEWTYPE_BOTH,
    VIDEO_VIEWTYPE_NOTUSED
} VIDEO_VIEWTYPE;

typedef enum
{
    VIDEO_CAMERATYPE_INVALID = -1,
    VIDEO_CAMERATYPE_REAR,
    VIDEO_CAMERATYPE_FRONT,
    // VIDEO_CAMERATYPE_UNDEFINED,
    VIDEO_CAMERATYPE_NOTUSED = 99,
} VIDEO_CAMERATYPE;

typedef enum
{
    VIDEO_SOURCETYPE_INVALID = -1,
    VIDEO_SOURCETYPE_CAMERA,
    VIDEO_SOURCETYPE_IMAGE,
    VIDEO_SOURCETYPE_PAUSE_IMAGE,
    VIDEO_SOURCETYPE_VIDEOCLIP,  // video clip type for testmode
    VIDEO_SOURCETYPE_NOTUSED
} VIDEO_SOURCETYPE;

typedef enum
{
    VIDEO_ORIENTATION_INVALID = -1,
    VIDEO_ORIENTATION_DEGREES_0,
    VIDEO_ORIENTATION_DEGREES_CCW_90,  // Counter Clockwise
    VIDEO_ORIENTATION_DEGREES_CCW_270,
    VIDEO_ORIENTATION_DEGREES_CCW_180,
    VIDEO_ORIENTATION_NOT_USED,
    VIDEO_ORIENTATION_UI_NOCHANGE_LANDSCAPE,      // right = 90CW/270CCW
    VIDEO_ORIENTATION_UI_NOCHANGE_LANDSCAPE_180,  // left = 270CW/90CCW
} VIDEO_ORIENTATION;

typedef enum
{
    VIDEO_HOLD_INVALID = -1,
    VIDEO_HOLD_PAUSE,            // Normal scenario
    VIDEO_HOLD_BGRD_IMG,         // ATT/TMO/CANADA/GLOBAL
    VIDEO_HOLD_PREVIEW,          // VZW
    VIDEO_HOLD_IGNOREDIRECTION,  // DCM (didn't care of media direction)
    VIDEO_HOLD_NOTUSED
} VIDEO_HOLD_TYPE;

typedef enum
{
    VIDEO_MULTI_INVALID = -1,
    VIDEO_MULTI_PRE_ENCODED,       // Domestic scenario
    VIDEO_MULTI_GRAYED_LASTFRAME,  // VZW old
    VIDEO_MULTI_SEND_LIVE_FRAME,   // ATT
    VIDEO_MULTI_PAUSE,             // VZW
    VIDEO_MULTI_IGNOREDIRECTION,   // DCM (didn't care of media direction)
    VIDEO_MULTI_NOTUSED
} VIDEO_MULTI_TYPE;  // Video Multitasking type

#endif /* _IMS_VIDEO_DEF_H_ */
