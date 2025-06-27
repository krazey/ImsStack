/*
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

#include "audio/AudioProfileGenerator.h"
#include "config/AudioConfiguration.h"
#include "config/CodecAmrConfig.h"
#include "config/CodecEvsConfig.h"
#include "config/CodecPcmConfig.h"
#include "config/CodecTelephoneEventConfig.h"
#include "audio/AudioProfileUtil.h"

static const IMS_SINT32 NOT_PRESENT = -1;

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

PROTECTED
AudioProfile* AudioProfileGenerator::SetProfile(IN MediaBaseProfile* pProfile,
        IN MediaConfiguration* pConfig, MEDIA_SERVICE_TYPE eServiceType, IN IService* pIService,
        IN IMS_SINT32 nSlotId)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL || pIService == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetProfile(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    SetCommonProfile(pProfile, pConfig, pIService, nSlotId);

    AudioProfile* pAudioProfile = static_cast<AudioProfile*>(pProfile);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);

    pAudioProfile->SetTransportType("RTP/AVP");
    pAudioProfile->SetCandidateAttr(pAudioConfig->GetAudioCandidateAttribute());
    pAudioProfile->SetPtime(pAudioConfig->GetPtime());
    pAudioProfile->SetMaxPtime(pAudioConfig->GetMaxPtime());

    AudioProfileUtil::SetRtcpXr(pAudioProfile, pAudioConfig);
    AudioProfileUtil::SetAnbr(pAudioProfile, eServiceType, nSlotId);

    IMS_TRACE_D("SetProfile(): Ptime[%d], MaxPtime[%d]", pAudioProfile->GetPtime(),
            pAudioProfile->GetMaxPtime(), 0);
    IMS_TRACE_D("SetProfile(): AS[%d], RR[%d], RS[%d]", pAudioProfile->GetBandwidthAs(),
            pAudioProfile->GetBandwidthRr(), pAudioProfile->GetBandwidthRs());

    return pAudioProfile;
}

PROTECTED
void AudioProfileGenerator::CreateCodecPayloads(IN MediaBaseProfile* pProfile, IN IMS_SINT32 nCodec,
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL || pCodecConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCodecPayloads(): invalid arguments", 0, 0, 0);
        return;
    }

    if (nCodec > ImsCodec::AUDIO_NONE && nCodec < ImsCodec::AUDIO_MAX)
    {
        AudioProfile::Payload* pTempPayload = IMS_NULL;

        if (nCodec == ImsCodec::AUDIO_AMR || nCodec == ImsCodec::AUDIO_AMR_WB)
        {
            pTempPayload = CreateAmrPayload(pCodecConfig, pConfig);
        }
        else if (nCodec == ImsCodec::AUDIO_EVS)
        {
            pTempPayload = CreateEvsPayload(pCodecConfig, pConfig);
        }
        else if (nCodec == ImsCodec::AUDIO_TELEPHONE_EVENT ||
                nCodec == ImsCodec::AUDIO_TELEPHONE_EVENT_WB)
        {
            pTempPayload = CreateTelephoneEventPayload(pCodecConfig, pConfig);
        }
        else if (nCodec == ImsCodec::AUDIO_PCMA || nCodec == ImsCodec::AUDIO_PCMU)
        {
            pTempPayload = CreatePcmPayload(pCodecConfig, pConfig);
        }

        if (pTempPayload != IMS_NULL)
        {
            static_cast<AudioProfile*>(pProfile)->GetPayloadList().Append(pTempPayload);
        }
    }
}

PROTECTED void AudioProfileGenerator::SetAudioCodecFmtp(IN CodecAudioConfig* pCodecConfig,
        IN AudioConfiguration* pAudioConfig, OUT AudioProfile::AudioFmtp* pFmtp)
{
    if (pCodecConfig == IMS_NULL || pAudioConfig == IMS_NULL || pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetAudioCodecFmtp(): invalid arguments", 0, 0, 0);
        return;
    }

    if (pCodecConfig->GetModeChangeCapability() != NOT_PRESENT)
    {
        pFmtp->SetModeChangeCapability(pCodecConfig->GetModeChangeCapability());
    }
    else
    {
        pFmtp->SetModeChangeCapability(CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY);
    }

    // TODO(b/414484057) : need to add a new asset to handle to include this attribute to SDP
    pFmtp->SetVisibleModeChangeCapability(IMS_TRUE);

    if (pCodecConfig->GetModeChangePeriod() != NOT_PRESENT)
    {
        pFmtp->SetModeChangePeriod(pCodecConfig->GetModeChangePeriod());
    }
    else
    {
        pFmtp->SetModeChangePeriod(CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD);
    }

    // TODO(b/414484057) : need to add a new asset to handle to include this attribute to SDP
    pFmtp->SetVisibleModeChangePeriod(IMS_FALSE);

    if (pCodecConfig->GetModeChangeNeighbor() != NOT_PRESENT)
    {
        pFmtp->SetModeChangeNeighbor(pCodecConfig->GetModeChangeNeighbor());
    }
    else
    {
        pFmtp->SetModeChangeNeighbor(CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR);
    }

    // TODO(b/414484057) : need to add a new asset to handle to include this attribute to SDP
    pFmtp->SetVisibleModeChangeNeighbor(IMS_FALSE);

    if (pAudioConfig->GetMaxRed() != NOT_PRESENT)
    {
        pFmtp->SetMaxRed(pAudioConfig->GetMaxRed());
        pFmtp->SetVisibleMaxRed(IMS_TRUE);
    }
    else
    {
        pFmtp->SetMaxRed(AudioConfiguration::DEFAULT_MAX_RED);
        pFmtp->SetVisibleMaxRed(IMS_FALSE);
    }
}

PROTECTED AudioProfile::Payload* AudioProfileGenerator::CreateAmrPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateAmrPayload(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    CodecAmrConfig* pAmrConfig = static_cast<CodecAmrConfig*>(pCodecConfig);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);
    AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp();
    AString strCodecName;

    SetAudioCodecFmtp(pAmrConfig, pAudioConfig, pAmrFmtp);

    pAmrFmtp->SetModeSetList(pAmrConfig->GetModeSetList());
    pAmrFmtp->SetDefaultRtpModeSet(pAmrConfig->GetDefaultModeSetList());
    pAmrFmtp->SetVisibleModeSet(pAmrConfig->GetVisibleModeSet());

    pAmrFmtp->SetVisibleOctetAlign(IMS_FALSE);
    if (pAmrConfig->GetOctetAlign() != -1)
    {
        pAmrFmtp->SetOctetAlign(pAmrConfig->GetOctetAlign());
        if (pAmrFmtp->GetOctetAlign() == 1)
        {
            pAmrFmtp->SetVisibleOctetAlign(IMS_TRUE);
        }
    }
    else
    {
        pAmrFmtp->SetOctetAlign(CodecAmrConfig::DEFAULT_PAYLOAD_FORMAT);
    }

    pAmrFmtp->SetDtx(pAmrConfig->GetDtx());
    strCodecName.Sprintf("%s", (pAmrConfig->GetSamplingRate() == 8000) ? "AMR" : "AMR-WB");

    AudioProfile::Payload* pAmrPayload = new AudioProfile::Payload();
    pAmrPayload->SetRtpMap(pAmrConfig->GetPayloadType(), strCodecName,
            pAmrConfig->GetSamplingRate(), pAmrConfig->GetChannel());
    pAmrPayload->SetFmtp(pAmrFmtp);

    IMS_TRACE_D("CreateAmrPayload(): Codec[%s], Payload[%d], SamplingRate[%d]",
            ImsCodec::CodecToString(pAmrConfig->GetCodec()), pAmrConfig->GetPayloadType(),
            pAmrConfig->GetSamplingRate());
    IMS_TRACE_D("CreateAmrPayload(): amrDtx[%d]", pAmrConfig->GetDtx(), 0, 0);

    return pAmrPayload;
}

PROTECTED AudioProfile::Payload* AudioProfileGenerator::CreateEvsPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateEvsPayload(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    CodecEvsConfig* pEvsConfig = static_cast<CodecEvsConfig*>(pCodecConfig);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);
    AudioProfile::EvsFmtp* pEvsFmtp = new AudioProfile::EvsFmtp();

    AString strCodecName;
    strCodecName.Sprintf("%s", "EVS");

    SetAudioCodecFmtp(pEvsConfig, pAudioConfig, pEvsFmtp);

    // Mode set list
    pEvsFmtp->SetModeSetList(pEvsConfig->GetModeSetList());
    pEvsFmtp->SetDefaultRtpModeSet(pEvsConfig->GetDefaultModeSetList());
    pEvsFmtp->SetVisibleModeSet(pEvsConfig->GetVisibleModeSet());
    pEvsFmtp->SetBrList(pEvsConfig->GetBrList());
    pEvsFmtp->SetBwList(pEvsConfig->GetBwList());

    // Bit-rate
    if (pEvsFmtp->GetBrList() == 0)
    {
        pEvsFmtp->SetBrList(CodecEvsConfig::DEFAULT_BR_LIST);
        pEvsFmtp->SetShowBrList(IMS_FALSE);
    }
    else
    {
        pEvsFmtp->SetShowBrList(IMS_TRUE);
    }

    // Bandwidth
    if (pEvsFmtp->GetBwList() == -1)
    {
        pEvsFmtp->SetBwList(CodecEvsConfig::DEFAULT_BW_LIST);
        pEvsFmtp->SetShowBwList(IMS_FALSE);
    }
    else
    {
        pEvsFmtp->SetShowBwList(IMS_TRUE);
    }

    pEvsFmtp->SetVisibleDtx(pEvsConfig->GetVisibleDtx());
    pEvsFmtp->SetDtx(pEvsConfig->GetDtx());

    pEvsFmtp->SetHfOnly(pEvsConfig->GetHfOnly());
    if (pEvsConfig->GetVisibleHfOnly())
    {
        pEvsFmtp->SetShowHfOnly(IMS_TRUE);
    }

    pEvsFmtp->SetEvsModeSwitch(pEvsConfig->GetEvsModeSwitch());
    if (pEvsConfig->GetVisibleEvsModeSwitch())
    {
        pEvsFmtp->SetShowEvsModeSwitch(IMS_TRUE);
    }

    pEvsFmtp->SetCmr(pEvsConfig->GetCmr());
    if (pEvsConfig->GetVisibleCmr())
    {
        pEvsFmtp->SetShowCmr(IMS_TRUE);
    }

    pEvsFmtp->SetChAwRecv(pEvsConfig->GetChAwareRecv());
    if (pEvsConfig->GetVisibleChAwareRecv())
    {
        pEvsFmtp->SetShowChannelAwMode(IMS_TRUE);
    }

    // set EVS codec fmtp
    AudioProfile::Payload* pEvsPayload = new AudioProfile::Payload();
    pEvsPayload->SetRtpMap(
            pEvsConfig->GetPayloadType(), strCodecName, 16000, pEvsConfig->GetChannel());
    pEvsPayload->SetFmtp(pEvsFmtp);

    IMS_TRACE_D("CreateEvsPayload(): Payload[%d], VisibleDtx[%d], VisibleModeSet[%d]",
            pEvsConfig->GetPayloadType(), pEvsConfig->GetVisibleDtx(),
            pEvsConfig->GetVisibleModeSet());

    return pEvsPayload;
}

PROTECTED AudioProfile::Payload* AudioProfileGenerator::CreateTelephoneEventPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateTelephoneEventPayload(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    CodecTelephoneEventConfig* pDtmfConfig = static_cast<CodecTelephoneEventConfig*>(pCodecConfig);
    AString strCodecName;
    strCodecName.Sprintf("%s", "telephone-event");

    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);
    AudioProfile::TelephoneEventFmtp* pTelephoneEventFmtp =
            new AudioProfile::TelephoneEventFmtp(pDtmfConfig->GetEvents());

    AudioProfile::Payload* pTelephoneEventPayload = new AudioProfile::Payload();
    pTelephoneEventPayload->SetRtpMap(
            pDtmfConfig->GetPayloadType(), strCodecName, pDtmfConfig->GetSamplingRate(), 0);
    pTelephoneEventPayload->SetFmtp(pTelephoneEventFmtp);

    IMS_TRACE_D("CreateTelephoneEventPayload(): Codec[%s], Payload[%d], SamplingRate[%d]",
            ImsCodec::CodecToString(pDtmfConfig->GetCodec()), pDtmfConfig->GetPayloadType(),
            pDtmfConfig->GetSamplingRate());

    return pTelephoneEventPayload;
}

PROTECTED AudioProfile::Payload* AudioProfileGenerator::CreatePcmPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreatePcmPayload(): invalid arguments", 0, 0, 0);
        return IMS_NULL;
    }

    AudioProfile::Payload* pPcmPayload = new AudioProfile::Payload();
    AString strCodecName;
    IMS_UINT32 nPayloadNum = 0;

    if (pCodecConfig->GetCodec() == ImsCodec::AUDIO_PCMU)
    {
        strCodecName.Sprintf("%s", "PCMU");
        nPayloadNum = 0;
    }
    else
    {
        strCodecName.Sprintf("%s", "PCMA");
        nPayloadNum = 8;
    }

    pPcmPayload->SetRtpMap(nPayloadNum, strCodecName, 8000, 0);

    IMS_TRACE_I(
            "CreatePcmPayload(): Codec[%s], Payload[%d]", strCodecName.GetStr(), nPayloadNum, 0);
    return pPcmPayload;
}
