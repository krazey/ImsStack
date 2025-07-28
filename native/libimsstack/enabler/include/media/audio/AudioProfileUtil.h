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

class AudioProfileUtil
{
public:
    /**
     * @brief Sets the RTCP-XR (Extended Reports) attribute in an audio profile.
     *
     * @param pAudioProfile [out] The audio profile to be updated.
     * @param pConfig [in] The audio configuration containing the RTCP-XR setting.
     * @return IMS_TRUE on success, IMS_FALSE on failure.
     */
    static IMS_BOOL SetRtcpXr(
            OUT AudioProfile* pAudioProfile, IN const AudioConfiguration* pConfig);

    /**
     * @brief Retrieves the array of Application Specific (AS) bandwidth values for AMR codecs.
     *
     * @param eCodec The AMR codec type (AMR-NB or AMR-WB).
     * @param nOctet The octet-align mode.
     * @param bIpV6 Flag indicating if the network is IPv6.
     * @return A constant pointer to an array of AS bandwidth values.
     */
    static const IMS_SINT32* GetAmrAsArray(
            IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet, IN IMS_BOOL bIpV6);

    /**
     * @brief Retrieves the array of Application Specific (AS) bandwidth values for the EVS codec.
     *
     * @param nEVSFormat The EVS format (e.g., AMR-WB IO, EVS Primary).
     * @param bIpV6 Flag indicating if the network is IPv6.
     * @return A constant pointer to an array of AS bandwidth values.
     */
    static const IMS_SINT32* GetEvsAsArray(IN IMS_SINT32 nEVSFormat, IN IMS_BOOL bIpV6);

    /**
     * @brief Converts an AMR codec mode-set to its corresponding AS bandwidth value.
     *
     * @param eCodec The AMR codec type.
     * @param nOctet The octet-align mode.
     * @param bIpV6 Flag indicating if the network is IPv6.
     * @param nModeSet The mode-set to convert.
     * @param bGetMaxValue If true, returns the maximum possible bandwidth value.
     * @return The calculated AS bandwidth value.
     */
    static IMS_SINT32 ConvertToBandwidthAS(IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet,
            IN IMS_BOOL bIpV6, IN IMS_SINT32 nModeSet, IN IMS_BOOL bGetMaxValue = IMS_FALSE);

    /**
     * @brief Converts an EVS codec format/mode to its corresponding AS bandwidth value.
     */
    static IMS_SINT32 ConvertToBandwidthAS(IN IMS_SINT32 eCodec, IN IMS_BOOL bIpV6,
            IN IMS_SINT32 nCodecFormat, IN IMS_SINT32 nCodecMode,
            IN IMS_BOOL bGetMaxValue = IMS_FALSE);

    /**
     * @brief Parses the 'fmtp' attribute to find the largest mode-set value.
     *
     * @param strCodec The name of the codec.
     * @param pPayload The payload containing the fmtp attribute string.
     * @return The largest mode-set value found, or a negative value on error.
     */
    static IMS_SINT32 GetLargestModesetInFmtp(
            IN const AString& strCodec, IN AudioProfile::Payload* pPayload);

    /**
     * @brief Parses the 'fmtp' attribute to get a bitmask of all declared mode-sets.
     *
     * @param strCodec The name of the codec.
     * @param pPayload The payload containing the fmtp attribute string.
     * @return A bitmask representing the list of mode-sets.
     */
    static IMS_SINT32 GetModesetList(
            IN const AString& strCodec, IN AudioProfile::Payload* pPayload);

    /**
     * @brief Set ANBR on/off from the media session config
     *
     * @param pProfile target profile that ANBR to be set
     * @param eServiceType The service type of the session
     * @param nSlotId The UICC slot id
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
