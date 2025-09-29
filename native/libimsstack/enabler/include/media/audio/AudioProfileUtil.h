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

#ifndef AUDIO_PROFILE_UTIL_H_
#define AUDIO_PROFILE_UTIL_H_

#include "audio/AudioProfile.h"
#include "config/AudioConfiguration.h"

/**
 * @brief A utility class for handling audio profiles, particularly for AMR and EVS codecs.
 *
 * This class provides static methods to manage and convert audio codec parameters,
 * such as RTCP-XR settings, Application Specific (AS) bandwidth, and mode-sets.
 */
class AudioProfileUtil
{
public:
    /**
     * @brief Sets the RTCP-XR (Extended Reports) attributes in the audio profile.
     *
     * @param pAudioProfile The audio profile to be updated.
     * @param pConfig The audio configuration containing RTCP-XR settings.
     * @return IMS_TRUE on success, IMS_FALSE otherwise.
     */
    static IMS_BOOL SetRtcpXr(
            OUT AudioProfile* pAudioProfile, IN const AudioConfiguration* pConfig);
    /**
     * @brief Gets the AMR AS (Application Specific) bandwidth array.
     *
     * @param eCodec The AMR codec type (AMR or AMR-WB).
     * @param nOctet The octet-align mode (0 for bandwidth-efficient, 1 for octet-aligned).
     * @param bIpV6 Flag indicating if IPv6 is used.
     * @return A pointer to the corresponding AMR AS array.
     */
    static const IMS_SINT32* GetAmrAsArray(
            IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet, IN IMS_BOOL bIpV6);
    /**
     * @brief Gets the EVS AS (Application Specific) bandwidth array.
     *
     * @param nEVSFormat The EVS format (0 for Primary, 1 for AMR-IO).
     * @param bIpV6 Flag indicating if IPv6 is used.
     * @return A pointer to the corresponding EVS AS array.
     */
    static const IMS_SINT32* GetEvsAsArray(IN IMS_SINT32 nEVSFormat, IN IMS_BOOL bIpV6);
    /**
     * @brief Converts an AMR mode-set to its corresponding AS bandwidth value.
     *
     * @param eCodec The AMR codec type.
     * @param nOctet The octet-align mode.
     * @param bIpV6 Flag indicating if IPv6 is used.
     * @param nModeSet The mode-set to convert.
     * @param bGetMaxValue Flag to get the maximum value for the given configuration.
     * @return The AS bandwidth value in kbps, or -1 on error.
     */
    static IMS_SINT32 ConvertToBandwidthAS(IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet,
            IN IMS_BOOL bIpV6, IN IMS_SINT32 nModeSet, IN IMS_BOOL bGetMaxValue = IMS_FALSE);
    /**
     * @brief Converts an EVS codec mode to its corresponding AS bandwidth value.
     *
     * @param eCodec The codec type (should be EVS).
     * @param bIpV6 Flag indicating if IPv6 is used.
     * @param nCodecFormat The EVS codec format (Primary/AMR-IO).
     * @param nCodecMode The EVS codec mode/bitrate index.
     * @param bGetMaxValue Flag to get the maximum value for the given configuration.
     * @return The AS bandwidth value in kbps, or -1 on error.
     */
    static IMS_SINT32 ConvertToBandwidthAS(IN IMS_SINT32 eCodec, IN IMS_BOOL bIpV6,
            IN IMS_SINT32 nCodecFormat, IN IMS_SINT32 nCodecMode,
            IN IMS_BOOL bGetMaxValue = IMS_FALSE);

    /**
     * @brief Gets the largest mode-set or bitrate index from the FMTP attributes of a payload.
     *
     * @param strCodec The codec name (e.g., "AMR", "AMR-WB", "EVS").
     * @param pPayload The audio payload to inspect.
     * @return The largest mode-set/bitrate index, or -1 if not found or on error.
     */
    static IMS_SINT32 GetLargestModesetInFmtp(
            IN const AString& strCodec, IN AudioProfile::Payload* pPayload);
    /**
     * @brief Gets the mode-set list or bitrate list from the FMTP attributes of a payload.
     *
     * @param strCodec The codec name (e.g., "AMR", "AMR-WB", "EVS").
     * @param pPayload The audio payload to inspect.
     * @return The mode-set/bitrate list as a bitmask, or 0 if not found or on error.
     */
    static IMS_SINT32 GetModesetList(
            IN const AString& strCodec, IN AudioProfile::Payload* pPayload);

    /**
     * @brief Gets the smallest mode-set or bitrate index from the FMTP attributes of a payload.
     *
     * @param strCodec The codec name (e.g., "AMR", "AMR-WB", "EVS").
     * @param pPayload The audio payload to inspect.
     * @return The smallest mode-set/bitrate index, or -1 if not found or on error.
     */
    static IMS_SINT32 GetSmallestModesetInFmtp(
            IN const AString& strCodec, IN AudioProfile::Payload* pPayload);

    /**
     * @brief Gets the bitrate in kbps for a given AMR/AMR-WB mode.
     *
     * @param strCodec The codec name ("AMR" or "AMR-WB").
     * @param nMode The codec mode index.
     * @return The bitrate in kbps.
     */
    static IMS_FLOAT GetBitrateFromAmrMode(IN const AString& strCodec, IN IMS_SINT32 nMode);

    /**
     * @brief Gets the bitrate in kbps for a given EVS mode.
     *
     * @param nEvsModeSwitch The EVS mode switch value (0 for Primary, 1 for AMR-WB IO).
     * @param nMode The codec mode index.
     * @return The bitrate in kbps.
     */
    static IMS_FLOAT GetBitrateFromEvsMode(IN IMS_SINT32 nEvsModeSwitch, IN IMS_SINT32 nMode);

    /**
     * @brief Gets the highest EVS bandwidth in kHz from a bandwidth list bitmask.
     *
     * @param bwList The bitmask representing the EVS bandwidth list.
     * @return The highest bandwidth in kHz.
     */
    static IMS_FLOAT GetEvsBandwidthKhz(IN IMS_UINT32 bwList);

    /**
     * @brief Sets the ANBR (Adaptive Narrow-Band Rate) support status in the audio profile.
     *
     * This function checks the media session configuration for the given slot and service type
     * to determine if ANBR is supported. It then updates the profile's ANBR flag, which
     * controls the inclusion of the 'a=anbr' attribute in the SDP offer.
     *
     * @param pProfile The audio profile to be updated with the ANBR support status.
     * @param eServiceType The service type of the session (e.g., default, emergency).
     * @param nSlotId The UICC slot ID for which to look up the configuration.
     */
    static void SetAnbr(
            OUT AudioProfile* pProfile, IN MEDIA_SERVICE_TYPE eServiceType, IN IMS_SINT32 nSlotId);

    static const IMS_UINT32 EVS_BR_CNT = 12;
    static const IMS_UINT32 EVS_BW_CNT = 4;
    static const IMS_UINT32 EVS_BW_LIST_CNT = 9;
    static const IMS_SINT32 AMR_AS[8][9];
    static const IMS_SINT32 EVS_AS[4][12];
    static const AString EVS_BR[12];
    static const AString EVS_BW[4];
    static const AString EVS_BW_LIST[9];
    static const AString AUDIO_CODEC_BANDWIDTH_STRING[4];
    static const AString AUDIO_CODEC_BITRATE_STRING[3][9];
};

#endif
