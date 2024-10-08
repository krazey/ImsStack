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

#include "ServiceTrace.h"
#include "MediaProfileUtil.h"

#include "audio/AudioProfileGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC AudioProfileGenerator::AudioProfileGenerator() :
        MediaProfileGenerator(MEDIA_TYPE_AUDIO)
{
    IMS_TRACE_I("+AudioProfileGenerator()", 0, 0, 0);
}

PUBLIC VIRTUAL AudioProfileGenerator::~AudioProfileGenerator()
{
    IMS_TRACE_I("~AudioProfileGenerator()", 0, 0, 0);
}

PUBLIC
AudioProfile* AudioProfileGenerator::SetProfile(IN MediaBaseProfile* pProfile,
        IN MediaConfiguration* pConfig, IN MediaEnvironment* pEnvironment, IN IMS_SINT32 nSlotId)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL || pEnvironment == IMS_NULL)
    {
        return IMS_NULL;
    }

    SetCommonProfile(pProfile, pConfig, pEnvironment, nSlotId);

    AudioProfile* pAudioProfile = static_cast<AudioProfile*>(pProfile);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);

    pAudioProfile->SetTransportType("RTP/AVP");
    pAudioProfile->SetCandidateAttr(pAudioConfig->GetAudioCandidateAttribute());
    pAudioProfile->SetPtime(pAudioConfig->GetPtime());
    pAudioProfile->SetMaxPtime(pAudioConfig->GetMaxPtime());

    AudioProfileUtil::SetRtcpXr(pAudioProfile, pAudioConfig);
    AudioProfileUtil::SetAnbr(pAudioProfile, pEnvironment, nSlotId);

    IMS_TRACE_D("SetProfile() - Ptime[%d], MaxPtime[%d]", pAudioProfile->GetPtime(),
            pAudioProfile->GetMaxPtime(), 0);
    IMS_TRACE_D("SetProfile() - AS[%d], RR[%d], RS[%d]", pAudioProfile->GetBandwidthAs(),
            pAudioProfile->GetBandwidthRr(), pAudioProfile->GetBandwidthRs());

    return pAudioProfile;
}
