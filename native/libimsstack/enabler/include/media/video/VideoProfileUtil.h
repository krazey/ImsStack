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

#ifndef VIDEO_PROFILE_UTIL_H_
#define VIDEO_PROFILE_UTIL_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "VideoDef.h"

/**
 * @brief A utility class for handling video profiles.
 *
 * This class provides static methods to manage and convert video codec parameters,
 * such as resolution, profile, and level.
 */
class VideoProfileUtil
{
public:
    /**
     * @brief Converts a VIDEO_RESOLUTION enum to its corresponding width and height.
     *
     * @param eResolIc The input video resolution enum.
     * @param nWidth A pointer to store the output width.
     * @param nHeight A pointer to store the output height.
     */
    static void GetWidthHeightFromResolution(
            IN VIDEO_RESOLUTION eResolIc, IN IMS_UINT32* nWidth, IN IMS_UINT32* nHeight);
    /**
     * @brief Converts width and height values to the corresponding VIDEO_RESOLUTION enum.
     *
     * @param nWidth The input width.
     * @param nHeight The input height.
     * @return The corresponding VIDEO_RESOLUTION enum, or VIDEO_RESOLUTION_INVALID if no match is
     * found.
     */
    static VIDEO_RESOLUTION GetResolutionFromWidthHeight(
            IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight);
    /**
     * @brief Parses the AVC profile from a profile-level-id string (RFC 6184).
     *
     * @param strProfileLevelId The profile-level-id string from SDP.
     * @return The corresponding VIDEO_PROFILE_AVC enum.
     */
    static VIDEO_PROFILE_AVC GetAvcProfileFromProfileLevelId(IN const AString& strProfileLevelId);
    /**
     * @brief Parses the AVC level from a profile-level-id string (RFC 6184).
     *
     * @param strProfileLevelId The profile-level-id string from SDP.
     * @return The parsed AVC level value.
     */
    static IMS_UINT32 GetAvcLevelFromProfileLevelId(IN const AString& strProfileLevelId);
    /**
     * @brief Determines the maximum supported resolution for a given AVC level.
     *
     * @param nLevel The AVC level.
     * @return The maximum VIDEO_RESOLUTION enum for that level.
     */
    static VIDEO_RESOLUTION GetAvcMaxResolutionFromLevel(IN IMS_UINT32 nLevel);

    /**
     * @brief Compares two VIDEO_RESOLUTION enums to determine which is larger based on
     * pixel count.
     *
     * @param eResolution1 The first resolution to compare.
     * @param eResolution2 The second resolution to compare.
     * @return 1 if eResolution1 is larger, -1 if eResolution2 is larger, 0 if they are
     * equal.
     */
    static IMS_SINT32 CompareResolution(
            IN VIDEO_RESOLUTION eResolution1, IN VIDEO_RESOLUTION eResolution2);

    /**
     * @brief Parses the video width and height from the `sprop-parameter-sets` SDP attribute.
     *
     * This function decodes the Base64-encoded parameter string to extract the
     * Sequence Parameter Set (SPS) and then parses the SPS to determine the
     * video frame dimensions.
     *
     * @param eCodecType The video codec type (e.g., AVC).
     * @param strSpropParam The `sprop-parameter-sets` string from the SDP.
     * @param nImageWidth A pointer to store the output width.
     * @param nImageHeight A pointer to store the output height.
     * @return IMS_TRUE on successful parsing, IMS_FALSE otherwise.
     */
    static IMS_BOOL GetWidthHeightFromSpropParam(IN VIDEO_CODEC eCodecType,
            IN const AString& strSpropParam, OUT IMS_UINT32* nImageWidth,
            OUT IMS_UINT32* nImageHeight);
};

#endif
