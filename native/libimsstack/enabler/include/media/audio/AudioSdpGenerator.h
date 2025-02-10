/**
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef AUDIO_SDP_GENERATOR_H_
#define AUDIO_SDP_GENERATOR_H_

#include "MediaSdpGenerator.h"
#include "audio/AudioProfile.h"

/**
 * This class is to generate an audio Sdp by adding audio attributes from audio profile to the
 * MediaDescriptor and the SessionDescriptor
 */
class AudioSdpGenerator : public MediaSdpGenerator
{
public:
    AudioSdpGenerator();
    virtual ~AudioSdpGenerator();

    IMS_BOOL Generate(OUT ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MediaBaseProfile* pBaseProfile) override;

protected:
    void GeneratePayload(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    IMS_BOOL GenerateFmtp(OUT AString& strFmtp, IN AudioProfile::Payload* pPayload);
    void GeneratePtime(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateMaxPtime(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateCandidateAttribute(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateRtcpXr(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateAnbr(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    AString GenerateAmrFmtp(IN AudioProfile::AmrFmtp* pAmrFmtp);
    AString GenerateEvsFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp);

    // AudioFmtp
    void AddModeSetListToFmtp(IN AudioProfile::AudioFmtp* pFmtp, OUT AString& strFmtp);
    void AddModeChangeCapabilityToFmtp(IN AudioProfile::AudioFmtp* pFmtp, OUT AString& strFmtp);
    void AddModeChangePeriodToFmtp(IN AudioProfile::AudioFmtp* pFmtp, OUT AString& strFmtp);
    void AddModeChangeNeighborToFmtp(IN AudioProfile::AudioFmtp* pFmtp, OUT AString& strFmtp);
    void AddMaxRedToFmtp(IN AudioProfile::AudioFmtp* pFmtp, OUT AString& strFmtp);

    // AmrFmtp
    void AddOctetAlignToFmtp(IN AudioProfile::AmrFmtp* pFmtp, OUT AString& strFmtp);

    // EvsFmtp
    void AddDtxToFmtp(IN AudioProfile::EvsFmtp* pFmtp, OUT AString& strFmtp);
    void AddHfOnlyToFmtp(IN AudioProfile::EvsFmtp* pFmtp, OUT AString& strFmtp);
    void AddEvsModeSwitchToFmtp(IN AudioProfile::EvsFmtp* pFmtp, OUT AString& strFmtp);
    void AddBwToFmtp(IN AudioProfile::EvsFmtp* pFmtp, OUT AString& strFmtp);
    void AddBrToFmtp(IN AudioProfile::EvsFmtp* pFmtp, OUT AString& strFmtp);
    void AddCmrToFmtp(IN AudioProfile::EvsFmtp* pFmtp, OUT AString& strFmtp);
    void AddChannelAwModeToFmtp(IN AudioProfile::EvsFmtp* pFmtp, OUT AString& strFmtp);
    void AddBwSendToFmtp(IN AudioProfile::EvsFmtp* pFmtp, OUT AString& strFmtp);
    void AddBwRecvToFmtp(IN AudioProfile::EvsFmtp* pFmtp, OUT AString& strFmtp);
    void AddBrSendToFmtp(IN AudioProfile::EvsFmtp* pFmtp, OUT AString& strFmtp);
    void AddBrRecvToFmtp(IN AudioProfile::EvsFmtp* pFmtp, OUT AString& strFmtp);

    void ForceToAddModeSetList(IN AudioProfile::AudioFmtp* pFmtp, OUT AString& strFmtp);
    void ForceToAddOctetAlign(IN AudioProfile::AmrFmtp* pFmtp, OUT AString& strFmtp);
};

#endif
