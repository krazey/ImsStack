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

#ifndef _AUDIO_PROFILE_CONFIGURER_INTERFACE_H_
#define _AUDIO_PROFILE_CONFIGURER_INTERFACE_H_

#include "audio/AudioProfile.h"

class MediaSession;
class MediaEnvironment;
class AudioProfile;

class AudioProfileConfigurer
{
// == PUBLIC METHOD ==============================================================
public:
    static IMS_BOOL CreateAudioProfile(OUT AudioProfile* pAudioProfile,
            IN MediaEnvironment* pEnvironment, IN AudioConfiguration* pConfig,
            IN IMS_SINT32 nSlotId);
    static IMS_BOOL SetAudioRsRr(OUT AudioProfile* pAudioProfile, IN AudioConfiguration* pConfig,
            IN MEDIA_DIRECTION eDir);
    static IMS_BOOL MakeNegotiatedBandwidth(IN AudioConfiguration* pConfig,
            IN AudioProfile* pSrcProfile, IN AudioProfile* pDestProfile,
            IN IMS_BOOL bIsOfferReceived, IN IMS_SINT32 nAsValueOfNegoticatedCodec,
            OUT AudioProfile* pNegotiatedProfile);
    static const IMS_SINT32* GetAmrAsArray(IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet,
            IN IMS_BOOL bIpV6);
    static const IMS_SINT32* GetEvsAsArray(IN IMS_SINT32 nEVSFormat, IN IMS_BOOL bIpV6);
    static IMS_SINT32 ConvertToBandwidthAS(IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet,
            IN IMS_BOOL bIpV6, IN IMS_SINT32 nModeSet, IN IMS_BOOL bGetMaxValue = IMS_FALSE);
    static IMS_SINT32 ConvertToBandwidthAS(IN IMS_SINT32 eCodec, IN IMS_BOOL bIpV6,
            IN IMS_SINT32 nCodecFormat, IN IMS_SINT32 nCodecMode,
            IN IMS_BOOL bGetMaxValue = IMS_FALSE);
    static IMS_SINT32 ConvertToModeSet(IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet,
            IN IMS_BOOL bIpV6, IN IMS_SINT32 nAs);
    static IMS_BOOL UpdateAudioProfileBandwidth(OUT AudioProfile* pAudioProfile,
            IN AudioConfiguration* pConfig);
    static IMS_SINT32 GetLargestModesetInFmtp(IN AString strCodec,
            IN AudioProfile::Payload* pPayload);
private :
    static const IMS_SINT32 AMR_AS[8][9];
    static const IMS_SINT32 EVS_AS[4][12];
};
#endif                                              /* end of _AUDIO_PROFILE_CONFIGURER_INTERFACE_H_ */
