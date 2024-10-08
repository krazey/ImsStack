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

#include "IService.h"
#include "ServiceTrace.h"

#include "MediaEnvironment.h"
#include "MediaManager.h"
#include "MediaProfileFactory.h"
#include "MediaProfileGenerator.h"
#include "MediaProfileUtil.h"
#include "MediaResourceManager.h"

#include "audio/AudioDef.h"
#include "audio/AudioProfileUtil.h"
#include "config/CodecAvcConfig.h"
#include "config/CodecAmrConfig.h"
#include "config/CodecEvsConfig.h"
#include "config/CodecHevcConfig.h"
#include "config/CodecPcmConfig.h"
#include "config/CodecT140Config.h"
#include "config/CodecTelephoneEventConfig.h"
#include "config/ImsCodec.h"
#include "config/AudioConfiguration.h"
#include "config/TextConfiguration.h"
#include "config/VideoConfiguration.h"
#include "video/VideoProfileUtil.h"

static const IMS_SINT32 NOT_PRESENT = -1;

__IMS_TRACE_TAG_MEDIA__;

PUBLIC MediaProfileGenerator::MediaProfileGenerator(IN const MEDIA_CONTENT_TYPE eType) :
        m_eType(eType)
{
}

PUBLIC VIRTUAL MediaProfileGenerator::~MediaProfileGenerator() {}

PUBLIC
MediaBaseProfile* MediaProfileGenerator::Generate(
        IN MediaEnvironment* pEnvironment, IN MediaConfiguration* pConfig, IN IMS_SINT32 nSlotId)
{
    if (pEnvironment == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("Generate() media type[%d]", m_eType, 0, 0);

    MediaBaseProfile* pProfile = IMS_NULL;

    pProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);
    pProfile = CreateCodecPayloads(pProfile, pConfig);
    pProfile = SetProfile(pProfile, pConfig, pEnvironment, nSlotId);

    return pProfile;
}

PRIVATE MediaBaseProfile* MediaProfileGenerator::CreateCodecPayloads(
        IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    ImsList<CodecConfig*> pCodecs;
    pCodecs = pConfig->GetCodecConfigs();

    for (IMS_UINT32 i = 0; i < pCodecs.GetSize(); i++)
    {
        CodecConfig* pCodecConfig = pCodecs.GetAt(i);

        if (pCodecConfig == IMS_NULL)
        {
            IMS_TRACE_D("CreateCodecPayloads() - pCodecConfig is NULL", 0, 0, 0);
            break;
        }

        IMS_SINT32 nCodec = pCodecConfig->GetCodec();

        if (pCodecConfig->GetPayloadType() == -1)
        {
            IMS_TRACE_D("CreateCodecPayloads() - invalid payload type, skip config[%d] - %d:%s", i,
                    nCodec, ImsCodec::CodecToString(nCodec));
            continue;
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
        else if (nCodec > ImsCodec::VIDEO_NONE && nCodec < ImsCodec::VIDEO_MAX)
        {
            VideoProfile::Payload* pTempPayload = IMS_NULL;

            if (nCodec == ImsCodec::VIDEO_AVC)
            {
                pTempPayload = CreateAvcPayload(pCodecConfig, pConfig);
            }
            else if (nCodec == ImsCodec::VIDEO_HEVC)
            {
                pTempPayload = CreateHevcPayload(pCodecConfig, pConfig);
            }
            if (pTempPayload != IMS_NULL)
            {
                static_cast<VideoProfile*>(pProfile)->GetPayloadList().Append(pTempPayload);
            }
        }
        else if (nCodec > ImsCodec::TEXT_NONE && nCodec < ImsCodec::TEXT_MAX)
        {
            TextProfile::Payload* pTempPayload = IMS_NULL;

            if (nCodec == ImsCodec::TEXT_T140 || nCodec == ImsCodec::TEXT_RED)
            {
                pTempPayload = CreateT140Payload(pCodecConfig, pConfig);
            }
            if (pTempPayload != IMS_NULL)
            {
                static_cast<TextProfile*>(pProfile)->GetPayloadList().Append(pTempPayload);
            }
        }
        else
        {
            IMS_TRACE_E(0, "CreateCodecPayloads() - Invalid Codec[%d]", nCodec, 0, 0);

            delete pProfile;
            return IMS_NULL;
        }
    }
    return pProfile;
}

PROTECTED
void MediaProfileGenerator::SetCommonProfile(IN MediaBaseProfile* pProfile,
        IN MediaConfiguration* pConfig, IN MediaEnvironment* pEnvironment, IN IMS_SINT32 nSlotId)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL || pEnvironment == IMS_NULL ||
            pEnvironment->pIService == IMS_NULL)
    {
        return;
    }

    MediaManager* pMediaManager = MediaManager::GetInstance(nSlotId);

    if (pMediaManager == IMS_NULL)
    {
        return;
    }

    MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr == IMS_NULL)
    {
        return;
    }

    pProfile->SetIpAddress(pEnvironment->pIService->GetIpAddress());
    pProfile->SetDataPort(pResourceMngr->AcquireRtpPort(pConfig));
    pProfile->SetControlPort(pProfile->GetDataPort() + 1);
    pProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    pProfile->SetBandwidthAs(pConfig->GetAsBandwidthKbps());
    pProfile->SetBandwidthRr(pConfig->GetRrBandwidthBps());
    pProfile->SetBandwidthRs(pConfig->GetRsBandwidthBps());
    pProfile->SetRtcpInterval(pConfig->GetRtcpIntervalOnHold());

    IMS_TRACE_I("SetCommonProfile() - IpAddress[%s], rtp port[%d], rtcp port[%d]",
            pProfile->GetIpAddress().ToCharString(), pProfile->GetDataPort(),
            pProfile->GetControlPort());
    IMS_TRACE_I("SetCommonProfile() - AS[%d], RR[%d], RS[%d]", pProfile->GetBandwidthAs(),
            pProfile->GetBandwidthRr(), pProfile->GetBandwidthRs());
    IMS_TRACE_I("SetCommonProfile() - direction[%d], rtcp Interval[%d]", pProfile->GetDirection(),
            pProfile->GetRtcpInterval(), 0);
}

PRIVATE AudioProfile::Payload* MediaProfileGenerator::CreateAmrPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    CodecAmrConfig* pAmrConfig = static_cast<CodecAmrConfig*>(pCodecConfig);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);
    AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp();

    if (pAmrFmtp == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateAmrPayload() - payload number[%d]", pAmrConfig->GetPayloadType(), 0, 0);

    SetAudioCodecFmtp(pAmrConfig, pAudioConfig, pAmrFmtp);

    pAmrFmtp->SetModeSetList(pAmrConfig->GetAmrModeSetList());
    pAmrFmtp->SetDefaultRtpModeSet(pAmrConfig->GetDefaultAmrModeSetList());
    pAmrFmtp->SetShowModeSet(pAmrConfig->GetShowAmrModeSet());

    pAmrFmtp->SetShowOctetAlign(IMS_FALSE);
    if (pAmrConfig->GetOctetAlign() != -1)
    {
        pAmrFmtp->SetOctetAlign(pAmrConfig->GetOctetAlign());
        if (pAmrFmtp->GetOctetAlign() == 1)
        {
            pAmrFmtp->SetShowOctetAlign(IMS_TRUE);
        }
    }
    else
    {
        pAmrFmtp->SetOctetAlign(CodecAmrConfig::DEFAULT_OCTET_ALIGN);
    }

    AString strCodecName;
    strCodecName.Sprintf("%s", (pAmrConfig->GetSamplingRate() == 8000) ? "AMR" : "AMR-WB");

    AudioProfile::Payload* pAmrPayload = new AudioProfile::Payload();
    pAmrPayload->SetRtpMap(pAmrConfig->GetPayloadType(), strCodecName,
            pAmrConfig->GetSamplingRate(), pAmrConfig->GetChannel());
    pAmrPayload->SetFmtp(pAmrFmtp);

    IMS_TRACE_D("CreateAmrPayload() - codec[%s], SamplingRate[%d]",
            ImsCodec::CodecToString(pAmrConfig->GetCodec()), pAmrConfig->GetSamplingRate(), 0);

    return pAmrPayload;
}

PRIVATE AudioProfile::Payload* MediaProfileGenerator::CreateEvsPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    CodecEvsConfig* pEvsConfig = static_cast<CodecEvsConfig*>(pCodecConfig);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);
    AudioProfile::EvsFmtp* pEvsFmtp = new AudioProfile::EvsFmtp();

    if (pEvsFmtp == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateEvsPayload() - payload number[%d]", pEvsConfig->GetPayloadType(), 0, 0);

    AString strCodecName;
    strCodecName.Sprintf("%s", "EVS");

    SetAudioCodecFmtp(pEvsConfig, pAudioConfig, pEvsFmtp);

    // Mode set list
    pEvsFmtp->SetModeSetList(pEvsConfig->GetAmrModeSetList());
    pEvsFmtp->SetDefaultRtpModeSet(pEvsConfig->GetDefaultAmrModeSetList());
    pEvsFmtp->SetShowModeSet(pEvsConfig->GetShowAmrModeSet());
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

    pEvsFmtp->SetShowDtx(pEvsConfig->GetShowDtx());

    if (pEvsConfig->GetHfOnly() == -1)  // Not Present
    {
        pEvsFmtp->SetHfOnly(CodecEvsConfig::DEFAULT_HF_ONLY);
    }
    else
    {
        pEvsFmtp->SetHfOnly(pEvsConfig->GetHfOnly());
        pEvsFmtp->SetShowHfOnly(IMS_TRUE);
    }

    if (pEvsConfig->GetEvsModeSwitch() != -1)
    {
        pEvsFmtp->SetEvsModeSwitch(pEvsConfig->GetEvsModeSwitch());
        pEvsFmtp->SetShowEvsModeSwitch(IMS_TRUE);
    }
    else
    {
        pEvsFmtp->SetEvsModeSwitch(CodecEvsConfig::DEFAULT_EVS_MODESWITCH);
        pEvsFmtp->SetShowEvsModeSwitch(IMS_FALSE);
    }

    if (pEvsConfig->GetCmr() == CodecEvsConfig::CMR_NOT_PRESENT)
    {
        pEvsFmtp->SetCmr(CodecEvsConfig::DEFAULT_CMR);
    }
    else
    {
        pEvsFmtp->SetCmr(pEvsConfig->GetCmr());
        pEvsFmtp->SetShowCmr(IMS_TRUE);
    }

    if (pEvsConfig->GetChAwareRecv() != -1)
    {
        pEvsFmtp->SetChAwRecv(pEvsConfig->GetChAwareRecv());
        pEvsFmtp->SetShowChannelAwMode(IMS_TRUE);
        if (pEvsConfig->GetChAwareRecv() == 99)
        {
            pEvsFmtp->SetChAwRecv(-1);
        }
    }
    else
    {
        pEvsFmtp->SetChAwRecv(-1);
    }

    IMS_TRACE_D("CreateEvsPayload() - visible Dtx[%d], visible AmrModeSet[%d]",
            pEvsConfig->GetShowDtx(), pEvsConfig->GetShowAmrModeSet(), 0);

    // set EVS codec fmtp
    AudioProfile::Payload* pEvsPayload = new AudioProfile::Payload();
    pEvsPayload->SetRtpMap(
            pEvsConfig->GetPayloadType(), strCodecName, 16000, pEvsConfig->GetChannel());
    pEvsPayload->SetFmtp(pEvsFmtp);

    return pEvsPayload;
}

PRIVATE AudioProfile::Payload* MediaProfileGenerator::CreateTelephoneEventPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    CodecTelephoneEventConfig* pDtmfConfig = static_cast<CodecTelephoneEventConfig*>(pCodecConfig);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);
    AudioProfile::TelephoneEventFmtp* pTelephoneEventFmtp =
            new AudioProfile::TelephoneEventFmtp(pDtmfConfig->GetEvents());

    if (pTelephoneEventFmtp == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateTelephoneEventPayload() - payload number[%d]", pDtmfConfig->GetPayloadType(),
            0, 0);

    AString strCodecName;
    strCodecName.Sprintf("%s", "telephone-event");

    AudioProfile::Payload* pTelephoneEventPayload = new AudioProfile::Payload();
    pTelephoneEventPayload->SetRtpMap(
            pDtmfConfig->GetPayloadType(), strCodecName, pDtmfConfig->GetSamplingRate(), 0);
    pTelephoneEventPayload->SetFmtp(pTelephoneEventFmtp);

    IMS_TRACE_D("CreateTelephoneEventPayload() - codec[%s], SamplingRate[%d]",
            ImsCodec::CodecToString(pDtmfConfig->GetCodec()), pDtmfConfig->GetSamplingRate(), 0);

    return pTelephoneEventPayload;
}

PRIVATE AudioProfile::Payload* MediaProfileGenerator::CreatePcmPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    AudioProfile::Payload* pPcmPayload = new AudioProfile::Payload();

    if (pPcmPayload == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_D("CreatePcmPayload() - codec[%s]", ImsCodec::CodecToString(pCodecConfig->GetCodec()),
            0, 0);

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

    CodecPcmConfig* pPcmConfig = static_cast<CodecPcmConfig*>(pCodecConfig);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);

    return pPcmPayload;
}

PRIVATE void MediaProfileGenerator::SetAudioCodecFmtp(IN CodecAudioConfig* pCodecConfig,
        IN AudioConfiguration* pAudioConfig, OUT AudioProfile::AudioFmtp* pFmtp)
{
    if (pCodecConfig == IMS_NULL || pAudioConfig == IMS_NULL || pFmtp == IMS_NULL)
    {
        return;
    }

    if (pCodecConfig->GetModeChangeCapability() != NOT_PRESENT)
    {
        pFmtp->SetModeChangeCapability(pCodecConfig->GetModeChangeCapability());
        pFmtp->SetShowModeChangeCapability(IMS_TRUE);
    }
    else
    {
        pFmtp->SetModeChangeCapability(CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY);
        pFmtp->SetShowModeChangeCapability(IMS_FALSE);
    }

    if (pCodecConfig->GetModeChangePeriod() != NOT_PRESENT)
    {
        pFmtp->SetModeChangePeriod(pCodecConfig->GetModeChangePeriod());
        pFmtp->SetShowModeChangePeriod(IMS_TRUE);
    }
    else
    {
        pFmtp->SetModeChangePeriod(CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD);
        pFmtp->SetShowModeChangePeriod(IMS_FALSE);
    }

    if (pCodecConfig->GetModeChangeNeighbor() != NOT_PRESENT)
    {
        pFmtp->SetModeChangeNeighbor(pCodecConfig->GetModeChangeNeighbor());
        pFmtp->SetShowModeChangeNeighbor(IMS_TRUE);
    }
    else
    {
        pFmtp->SetModeChangeNeighbor(CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR);
        pFmtp->SetShowModeChangeNeighbor(IMS_FALSE);
    }

    if (pAudioConfig->GetMaxRed() != NOT_PRESENT)
    {
        pFmtp->SetMaxRed(pAudioConfig->GetMaxRed());
        pFmtp->SetShowMaxRed(IMS_TRUE);
    }
    else
    {
        pFmtp->SetMaxRed(AudioConfiguration::DEFAULT_MAX_RED);
        pFmtp->SetShowMaxRed(IMS_FALSE);
    }

    pFmtp->SetDtx(pCodecConfig->GetDtx());
}

PRIVATE TextProfile::Payload* MediaProfileGenerator::CreateT140Payload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    CodecT140Config* pT140Config = reinterpret_cast<CodecT140Config*>(pCodecConfig);
    TextProfile::Payload* pTextPayload = new TextProfile::Payload();
    AString strCodecName;

    if (pCodecConfig->GetCodec() == ImsCodec::TEXT_RED)
    {
        strCodecName.Sprintf("%s", "red");

        TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp(pT140Config->GetRedLevel(),
                static_cast<TextConfiguration*>(pConfig)->GetT140PayloadType());

        IMS_TRACE_I("CreateT140Payload() add fmtp - red level(%d), red payload(%d)",
                pRedFmtp->GetRedLevel(), pRedFmtp->GetRedPayload(), 0);

        pTextPayload->SetFmtp(pRedFmtp);
    }
    else
    {
        strCodecName.Sprintf("%s", "t140");
    }

    pTextPayload->SetRtpMap(
            pT140Config->GetPayloadType(), strCodecName, pT140Config->GetSamplingRate());

    return pTextPayload;
}

PRIVATE VideoProfile::Payload* MediaProfileGenerator::CreateAvcPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateAvcPayload", 0, 0, 0);

    CodecAvcConfig* pAvcConfig = static_cast<CodecAvcConfig*>(pCodecConfig);
    VideoConfiguration* pVideoConfig = static_cast<VideoConfiguration*>(pConfig);
    VideoProfile::AvcFmtp* pAvcFmtp = new VideoProfile::AvcFmtp();

    if (pAvcConfig->GetProfileLevelId() == AString::ConstEmpty())
    {
        IMS_TRACE_D("CreateAvcPayload - ProfileLevelId is empty, delete pAvcFmtp", 0, 0, 0);

        delete pAvcFmtp;
        return IMS_NULL;
    }

    SetVideoCodecFmtp(pAvcConfig, pVideoConfig, pAvcFmtp);

    pAvcFmtp->SetProfile(
            VideoProfileUtil::GetAvcProfileFromProfileLevelId(pAvcConfig->GetProfileLevelId()));
    pAvcFmtp->SetLevel(
            VideoProfileUtil::GetAvcLevelFromProfileLevelId(pAvcConfig->GetProfileLevelId()));

    IMS_TRACE_I("CreateAvcPayload - Profile[%d], Level[%d]", pAvcFmtp->GetProfile(),
            pAvcFmtp->GetLevel(), 0);

    const IMS_CHAR* pbAvc4SpropParameterSets;
    pbAvc4SpropParameterSets = pAvcConfig->GetSpropParameterSets().GetStr();
    /** TODO: later sprop need to find a way to get SpropPramaterSets
    pbAvc4SpropParameterSets = GetAvcSpropParameterSets(
            pAvcFmtp->GetResolution(), pAvcFmtp->GetProfile(), pAvcFmtp->GetLevel());
    */

    IMS_TRACE_I("CreateAvcPayload - pbAvc4SpropParameterSets[%s]", pbAvc4SpropParameterSets, 0, 0);

    if (pAvcConfig->GetProfileLevelId().GetLength() != 0)
    {
        pAvcFmtp->SetProfileLevelId(pAvcConfig->GetProfileLevelId());
        pAvcFmtp->SetShowProfileLevelId(IMS_TRUE);
    }

    if (pAvcConfig->GetIncludeSpropParameterSets())
    {
        pAvcFmtp->SetSpropParam(pbAvc4SpropParameterSets);
        pAvcFmtp->SetShowSpropParam(IMS_TRUE);
    }

    VideoProfile::Payload* pAvcPayload = new VideoProfile::Payload();
    pAvcPayload->SetFmtp(pAvcFmtp);
    SetVideoCodecPayload(pAvcConfig, pVideoConfig, pAvcPayload);

    return pAvcPayload;
}

PRIVATE VideoProfile::Payload* MediaProfileGenerator::CreateHevcPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateHevcPayload", 0, 0, 0);

    CodecHevcConfig* pHevcConfig = reinterpret_cast<CodecHevcConfig*>(pCodecConfig);
    VideoConfiguration* pVideoConfig = static_cast<VideoConfiguration*>(pConfig);
    VideoProfile::HevcFmtp* pHevcFmtp = new VideoProfile::HevcFmtp();

    SetVideoCodecFmtp(pHevcConfig, pVideoConfig, pHevcFmtp);

    if (pHevcConfig->GetHevcProfile() != -1)
    {
        pHevcFmtp->SetProfile(static_cast<VIDEO_PROFILE_HEVC>(pHevcConfig->GetHevcProfile()));
        pHevcFmtp->SetShowProfile(IMS_TRUE);
    }

    if (pHevcConfig->GetHevcLevel() != -1)
    {
        pHevcFmtp->SetLevel(pHevcConfig->GetHevcLevel());
        pHevcFmtp->SetShowLevel(IMS_TRUE);
    }

    if (pHevcConfig->GetSpropParameterSets().GetLength() != 0)
    {
        pHevcFmtp->SetSpropParam(pHevcConfig->GetSpropParameterSets());
        pHevcFmtp->SetShowSpropParam(IMS_TRUE);
    }

    VideoProfile::Payload* pHevcPayload = new VideoProfile::Payload();
    pHevcPayload->SetFmtp(pHevcFmtp);
    SetVideoCodecPayload(pHevcConfig, pVideoConfig, pHevcPayload);

    return pHevcPayload;
}

PRIVATE void MediaProfileGenerator::SetVideoCodecFmtp(IN CodecVideoConfig* pCodecConfig,
        IN VideoConfiguration* pVideoConfig, OUT VideoProfile::VideoFmtp* pFmtp)
{
    if (pCodecConfig == IMS_NULL || pVideoConfig == IMS_NULL || pFmtp == IMS_NULL)
    {
        return;
    }

    pFmtp->SetFramerate(pCodecConfig->GetFramerate());
    pFmtp->SetResolution(VideoProfileUtil::GetResolutionFromWidthHeight(
            pCodecConfig->GetResolutionWidth(), pCodecConfig->GetResolutionHeight()));
    pFmtp->SetBitrate(pCodecConfig->GetBitrate());
    pFmtp->SetAs(pVideoConfig->GetAsBandwidthKbps());

    if (pCodecConfig->GetPacketizationMode() != -1)
    {
        pFmtp->SetPacketizationMode(pCodecConfig->GetPacketizationMode());
        pFmtp->SetShowPacketizationMode(IMS_TRUE);
    }

    IMS_TRACE_D("SetVideoCodecFmtp - FrameRate[%d], Resolution[%d], Bitrate[%d]",
            pFmtp->GetFramerate(), pFmtp->GetResolution(), pFmtp->GetBitrate());
    IMS_TRACE_D("SetVideoCodecFmtp - AS[%d], PacketizationMode[%d]", pFmtp->GetAs(),
            pFmtp->GetPacketizationMode(), 0);
}

PRIVATE void MediaProfileGenerator::SetVideoCodecPayload(IN CodecVideoConfig* pCodecConfig,
        IN VideoConfiguration* pVideoConfig, OUT VideoProfile::Payload* pPayload)
{
    if (pCodecConfig == IMS_NULL || pVideoConfig == IMS_NULL || pPayload == IMS_NULL)
    {
        return;
    }

    pPayload->SetRtpMap(pCodecConfig->GetPayloadType(),
            ImsCodec::CodecToString(pCodecConfig->GetCodec()), pVideoConfig->GetVideoSamplingRate(),
            pCodecConfig->GetChannel());

    if (pCodecConfig->GetImageAttr().GetLength() != 0)
    {
        pPayload->SetIncludeImageAttr(IMS_TRUE);
        pPayload->SetImageAttr(pCodecConfig->GetImageAttr());
    }
    else if (pCodecConfig->GetFrameSize().GetLength() != 0)
    {
        pPayload->SetIncludeFrameSize(IMS_TRUE);
    }

    if (pVideoConfig->IsVideoAvpfEnabled() == IMS_TRUE)
    {
        if (pVideoConfig->IsVideoAvpfTrrEnabled() == IMS_TRUE)
        {
            pPayload->GetRtcpFbAttr().SetTrrSupported(IMS_TRUE);
            pPayload->GetRtcpFbAttr().SetTrrInt(pVideoConfig->GetRtcpIntervalOnHold() * 1000);
        }

        if (pVideoConfig->IsVideoAvpfNackEnabled() == IMS_TRUE)
        {
            pPayload->GetRtcpFbAttr().SetNackSupported(IMS_TRUE);
        }

        if (pVideoConfig->IsVideoAvpfTmmbrEnabled() == IMS_TRUE)
        {
            pPayload->GetRtcpFbAttr().SetTmmbrSupported(IMS_TRUE);
            pPayload->GetRtcpFbAttr().SetTmmbrSmaxPr(40);
        }

        if (pVideoConfig->IsVideoAvpfPliEnabled() == IMS_TRUE)
        {
            pPayload->GetRtcpFbAttr().SetPliSupported(IMS_TRUE);
        }

        if (pVideoConfig->IsVideoAvpfFirEnabled() == IMS_TRUE)
        {
            pPayload->GetRtcpFbAttr().SetFirSupported(IMS_TRUE);
        }

        IMS_TRACE_I("SetVideoCodecPayload() AVPF. TRR[%d], NACK[%d], TMMBR[%d]",
                pPayload->GetRtcpFbAttr().IsTrrSupported(),
                pPayload->GetRtcpFbAttr().IsNackSupported(),
                pPayload->GetRtcpFbAttr().IsTmmbrSupported());
        IMS_TRACE_I("SetVideoCodecPayload() AVPF. PLI[%d], FIR[%d]",
                pPayload->GetRtcpFbAttr().IsPliSupported(),
                pPayload->GetRtcpFbAttr().IsFirSupported(), 0);
    }
}
