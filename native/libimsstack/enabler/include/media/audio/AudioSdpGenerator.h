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

#include "SdpGenerator.h"

class AudioProfile;

class AudioSdpGenerator : public SdpGenerator
{
public:
    AudioSdpGenerator();
    virtual ~AudioSdpGenerator();

    IMS_BOOL Generate(OUT ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MediaBaseProfile* pBaseProfile) override;

private:
    void GeneratePayload(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    IMS_BOOL GenerateFmtp(OUT AString& strFmtp, IN AudioProfile::Payload* pPayload);
    void GeneratePtime(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateMaxPtime(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateCandidateAttribute(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateRtcpXr(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    void GenerateAnbr(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
};

#endif
