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

#ifndef AUDIO_PROFILE_UTIL_H_
#define AUDIO_PROFILE_UTIL_H_

#include "MediaEnvironment.h"
#include "audio/AudioProfile.h"
#include "config/AudioConfiguration.h"

class AudioProfileUtil
{
public:
    static IMS_BOOL SetRtcpXr(OUT AudioProfile* pAudioProfile, IN AudioConfiguration* pConfig);
    static const IMS_SINT32* GetAmrAsArray(
            IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet, IN IMS_BOOL bIpV6);
    static const IMS_SINT32* GetEvsAsArray(IN IMS_SINT32 nEVSFormat, IN IMS_BOOL bIpV6);
    static IMS_SINT32 ConvertToBandwidthAS(IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet,
            IN IMS_BOOL bIpV6, IN IMS_SINT32 nModeSet, IN IMS_BOOL bGetMaxValue = IMS_FALSE);
    static IMS_SINT32 ConvertToBandwidthAS(IN IMS_SINT32 eCodec, IN IMS_BOOL bIpV6,
            IN IMS_SINT32 nCodecFormat, IN IMS_SINT32 nCodecMode,
            IN IMS_BOOL bGetMaxValue = IMS_FALSE);
    static IMS_SINT32 ConvertToModeSet(
            IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet, IN IMS_BOOL bIpV6, IN IMS_SINT32 nAs);
    static IMS_BOOL UpdateAudioProfileBandwidth(
            OUT AudioProfile* pAudioProfile, IN AudioConfiguration* pConfig);
    static IMS_SINT32 GetLargestModesetInFmtp(
            IN const AString& strCodec, IN AudioProfile::Payload* pPayload);
    static IMS_SINT32 GetModesetList(
            IN const AString& strCodec, IN AudioProfile::Payload* pPayload);
    /**
     * @brief Set Anbr on/off from the media session config
     *
     * @param pProfile target profile that anbr to be set
     * @param pEnvironment The media environment for network connection parameter
     * @param nSlotId The UICC slot id
     */
    static void SetAnbr(
            OUT AudioProfile* pProfile, IN MediaEnvironment* pEnvironment, IN IMS_SINT32 nSlotId);

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
