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

static MediaProfileFactory* g_pMediaProfileFactory = IMS_NULL;

__IMS_TRACE_TAG_MEDIA__;

PRIVATE
MediaProfileFactory::MediaProfileFactory() {}

PUBLIC VIRTUAL MediaProfileFactory::~MediaProfileFactory() {}

PUBLIC
MediaBaseProfile* MediaProfileFactory::CreateProfile(IN MediaEnvironment* pEnvironment,
        IN MediaConfiguration* pConfig, IN IMS_SINT32 nSlotId, IN MEDIA_CONTENT_TYPE eType)
{
    if (pEnvironment == IMS_NULL || pConfig == IMS_NULL || pEnvironment->pIService == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateProfile() media type [%d]", eType, 0, 0);

    MediaManager* pMediaManager = MediaManager::GetInstance(nSlotId);

    if (pMediaManager == IMS_NULL)
    {
        return IMS_NULL;
    }

    MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr == IMS_NULL)
    {
        return IMS_NULL;
    }

    MediaBaseProfile* pProfile = IMS_NULL;

    switch (eType)
    {
        case MEDIA_TYPE_AUDIO:
            pProfile = CreateAudioProfile();
            pProfile = SetAudioProfile(pProfile, pConfig);
            break;
        case MEDIA_TYPE_TEXT:
            pProfile = CreateTextProfile();
            pProfile = SetTextProfile(pProfile, pConfig);
            break;
        case MEDIA_TYPE_VIDEO:
            pProfile = CreateVideoProfile();
            pProfile = SetVideoProfile(pProfile, pConfig);
            break;
        default:
            return IMS_NULL;
    }

    // Setting each payload and bandwidth
    pProfile = CreateCodecPayloads(pProfile, pConfig);

    if (pProfile != IMS_NULL)
    {
        if (eType == MEDIA_TYPE_VIDEO)
        {
            SetMaxProfileFrameRate(static_cast<VideoProfile*>(pProfile));
        }

        // Setting IP address
        pProfile->objIpAddress = pEnvironment->pIService->GetIpAddress();
        pProfile->nDataPort = pResourceMngr->AcquireRtpPort(pConfig);
        pProfile->nControlPort = pProfile->nDataPort + 1;

        IMS_TRACE_I("CreateProfile() - IpAddress[%s], rtp port[%d], rtcp port[%d]",
                pProfile->objIpAddress.ToCharString(), pProfile->nDataPort, pProfile->nControlPort);

        // Setting direction
        pProfile->eDirection = MEDIA_DIRECTION_SEND_RECEIVE;
        pProfile->nBandwidthAs = pConfig->GetAsBandwidthKbps();
        pProfile->nBandwidthRr = pConfig->GetRrBandwidthBps();
        pProfile->nBandwidthRs = pConfig->GetRsBandwidthBps();
        pProfile->nRtcpInterval = pConfig->GetRtcpInterval();

        IMS_TRACE_I("CreateProfile() - direction[%d], rtcp Interval[%d]", pProfile->eDirection,
                pProfile->nRtcpInterval, 0);
    }

    return pProfile;
}
PRIVATE MediaBaseProfile* MediaProfileFactory::CreateCodecPayloads(
        IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig)
{
    ImsList<CodecConfig*> pCodecs;
    pCodecs = pConfig->GetCodecConfigs();

    for (IMS_UINT32 i = 0; i < pCodecs.GetSize(); i++)
    {
        CodecConfig* pCodecConfig = pCodecs.GetAt(i);

        if (pCodecConfig == IMS_NULL)
        {
            IMS_TRACE_D("pCodecConfig is NULL", 0, 0, 0);
            break;
        }

        IMS_SINT32 nCodec = pCodecConfig->GetCodec();

        if (pCodecConfig->GetPayloadType() == -1)
        {
            IMS_TRACE_D("CreateProfile() invalid payload type, skip config(%d) - %d:%s", i, nCodec,
                    ImsCodec::CodecToString(nCodec));
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
                static_cast<AudioProfile*>(pProfile)->lstPayload.Append(pTempPayload);
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
                static_cast<VideoProfile*>(pProfile)->lstPayload.Append(pTempPayload);
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
                static_cast<TextProfile*>(pProfile)->lstPayload.Append(pTempPayload);
            }
        }
        else
        {
            IMS_TRACE_E(0, "CreateProfile() - Invalid Codec(%d)", nCodec, 0, 0);
            delete pProfile;
            return IMS_NULL;
        }
    }

    return pProfile;
}

PUBLIC
void MediaProfileFactory::DeleteProfile(IN MediaBaseProfile* pProfile)
{
    delete pProfile;
    pProfile = IMS_NULL;
}

PUBLIC
MediaProfileFactory* MediaProfileFactory::GetInstance()
{
    if (g_pMediaProfileFactory == IMS_NULL)
    {
        g_pMediaProfileFactory = new MediaProfileFactory();
    }

    return g_pMediaProfileFactory;
}

PUBLIC
void MediaProfileFactory::ReleaseInstance(MediaProfileFactory* pMediaProfileFactory)
{
    if (pMediaProfileFactory != IMS_NULL && pMediaProfileFactory == g_pMediaProfileFactory)
    {
        delete pMediaProfileFactory;
        g_pMediaProfileFactory = IMS_NULL;
    }
}

PRIVATE AudioProfile* MediaProfileFactory::CreateAudioProfile()
{
    AudioProfile* pAudioProfile = new AudioProfile();

    return pAudioProfile;
}

PRIVATE TextProfile* MediaProfileFactory::CreateTextProfile()
{
    TextProfile* pTextProfile = new TextProfile();

    return pTextProfile;
}

PRIVATE VideoProfile* MediaProfileFactory::CreateVideoProfile()
{
    VideoProfile* pVideoProfile = new VideoProfile();

    return pVideoProfile;
}

PRIVATE AudioProfile* MediaProfileFactory::SetAudioProfile(
        IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    AudioProfile* pAudioProfile = static_cast<AudioProfile*>(pProfile);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);

    pAudioProfile->strTransportType = "RTP/AVP";
    pAudioProfile->objCandidateAttr = pAudioConfig->GetAudioCandidateAttribute();
    pAudioProfile->nPtime = pAudioConfig->GetPtime();
    pAudioProfile->nMaxPtime = pAudioConfig->GetMaxPtime();

    AudioProfileUtil::SetRtcpRsRr(pAudioProfile, pAudioConfig);
    AudioProfileUtil::SetRtcpXr(pAudioProfile, pAudioConfig);

    while (pAudioProfile->lstPayload.GetSize() > 0)
    {
        AudioProfile::Payload* pPayload = pAudioProfile->lstPayload.GetAt(0);
        if (pPayload != IMS_NULL)
        {
            delete pPayload;
        }
        pAudioProfile->lstPayload.RemoveAt(0);
    }

    IMS_TRACE_D("SetAudioProfile() - nPtime[%d], nMaxPtime[%d]", pAudioProfile->nPtime,
            pAudioProfile->nMaxPtime, 0);
    IMS_TRACE_D("SetAudioProfile() - AS[%d], RR[%d], RS[%d]", pAudioProfile->nBandwidthAs,
            pAudioProfile->nBandwidthRr, pAudioProfile->nBandwidthRs);

    return pAudioProfile;
}

PRIVATE AudioProfile::Payload* MediaProfileFactory::CreateAmrPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateAmrPayload", 0, 0, 0);

    CodecAmrConfig* pAmrConfig = static_cast<CodecAmrConfig*>(pCodecConfig);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);
    AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp();
    AString strCodecName;

    SetAudioCodecFmtp(pAmrConfig, pAudioConfig, pAmrFmtp);

    pAmrFmtp->nModeSetList = pAmrConfig->GetAmrModeSetList();
    pAmrFmtp->nDefaultRtpModeSet = pAmrConfig->GetDefaultAmrModeSetList();
    pAmrFmtp->bShowModeSet = pAmrConfig->GetShowAmrModeSet();

    pAmrFmtp->bShowOctetAlign = IMS_FALSE;
    if (pAmrConfig->GetOctetAlign() != -1)
    {
        pAmrFmtp->nOctetAlign = pAmrConfig->GetOctetAlign();
        if (pAmrFmtp->nOctetAlign == 1 || pAmrFmtp->nModeSetList == 0)
        {
            pAmrFmtp->bShowOctetAlign = IMS_TRUE;
        }
    }
    else
    {
        pAmrFmtp->nOctetAlign = CodecAmrConfig::DEFAULT_OCTET_ALIGN;
    }

    strCodecName.Sprintf("%s", (pAmrConfig->GetSamplingRate() == 8000) ? "AMR" : "AMR-WB");

    AudioProfile::Payload* pAmrPayload = new AudioProfile::Payload();
    pAmrPayload->SetRtpMap(pAmrConfig->GetPayloadType(), strCodecName,
            pAmrConfig->GetSamplingRate(), pAmrConfig->GetChannel());
    pAmrPayload->pFmtp = pAmrFmtp;

    IMS_TRACE_D("CreateAmrPayload() codec(%s), SamplingRate(%d)",
            ImsCodec::CodecToString(pAmrConfig->GetCodec()), pAmrConfig->GetSamplingRate(), 0);

    return pAmrPayload;
}

PRIVATE AudioProfile::Payload* MediaProfileFactory::CreateEvsPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateEvsPayload", 0, 0, 0);

    CodecEvsConfig* pEvsConfig = static_cast<CodecEvsConfig*>(pCodecConfig);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);
    AudioProfile::EvsFmtp* pEvsFmtp = new AudioProfile::EvsFmtp();

    AString strCodecName;
    strCodecName.Sprintf("%s", "EVS");

    SetAudioCodecFmtp(pEvsConfig, pAudioConfig, pEvsFmtp);

    // Mode set list
    pEvsFmtp->nModeSetList = pEvsConfig->GetAmrModeSetList();
    pEvsFmtp->nDefaultRtpModeSet = pEvsConfig->GetDefaultAmrModeSetList();
    pEvsFmtp->bShowModeSet = pEvsConfig->GetShowAmrModeSet();
    pEvsFmtp->nBrList = pEvsConfig->GetBrList();
    pEvsFmtp->nBwList = pEvsConfig->GetBwList();

    // Bit-rate
    if (pEvsFmtp->nBrList == 0)
    {
        pEvsFmtp->nBrList = CodecEvsConfig::DEFAULT_BR_LIST;
        pEvsFmtp->bShowBrList = IMS_FALSE;
    }
    else
    {
        pEvsFmtp->bShowBrList = IMS_TRUE;
    }

    // Bandwidth
    if (pEvsFmtp->nBwList == -1)
    {
        pEvsFmtp->nBwList = CodecEvsConfig::DEFAULT_BW_LIST;
        pEvsFmtp->bShowBrList = IMS_FALSE;
    }
    else
    {
        pEvsFmtp->bShowBwList = IMS_TRUE;
    }

    pEvsFmtp->bShowDtx = pEvsConfig->GetShowDtx();

    if (pEvsConfig->GetHfOnly() == -1)  // Not Present
    {
        pEvsFmtp->nHfOnly = CodecEvsConfig::DEFAULT_HF_ONLY;
    }
    else
    {
        pEvsFmtp->nHfOnly = pEvsConfig->GetHfOnly();
        pEvsFmtp->bShowHfOnly = IMS_TRUE;
    }

    if (pEvsConfig->GetEvsModeSwitch() != -1)
    {
        pEvsFmtp->nEvsModeSwitch = pEvsConfig->GetEvsModeSwitch();
        pEvsFmtp->bShowEvsModeSwitch = IMS_TRUE;
    }
    else
    {
        pEvsFmtp->nEvsModeSwitch = CodecEvsConfig::DEFAULT_EVS_MODESWITCH;
        pEvsFmtp->bShowEvsModeSwitch = IMS_FALSE;
    }

    if (pEvsConfig->GetCmr() == CodecEvsConfig::CMR_NOT_PRESENT)
    {
        pEvsFmtp->nCmr = CodecEvsConfig::DEFAULT_CMR;
    }
    else
    {
        pEvsFmtp->nCmr = pEvsConfig->GetCmr();
        pEvsFmtp->bShowCmr = IMS_TRUE;
    }
    if (pEvsConfig->GetChAwareRecv() != -1)
    {
        pEvsFmtp->nChAwRecv = pEvsConfig->GetChAwareRecv();
        pEvsFmtp->bShowChannelAwMode = IMS_TRUE;

        if (pEvsConfig->GetChAwareRecv() == 99)
        {
            pEvsFmtp->nChAwRecv = -1;
        }
    }
    else
    {
        pEvsFmtp->nChAwRecv = -1;
    }

    IMS_TRACE_D("EVS - GetShowDtx: %d GetShowAmrModeSet: %d", pEvsConfig->GetShowDtx(),
            pEvsConfig->GetShowAmrModeSet(), 0);

    // set EVS codec fmtp
    AudioProfile::Payload* pEvsPayload = new AudioProfile::Payload();
    pEvsPayload->SetRtpMap(
            pEvsConfig->GetPayloadType(), strCodecName, 16000, pEvsConfig->GetChannel());
    pEvsPayload->pFmtp = pEvsFmtp;

    return pEvsPayload;
}

PRIVATE AudioProfile::Payload* MediaProfileFactory::CreateTelephoneEventPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateTelephoneEventPayload", 0, 0, 0);

    CodecTelephoneEventConfig* pDtmfConfig = static_cast<CodecTelephoneEventConfig*>(pCodecConfig);
    AString strCodecName;
    strCodecName.Sprintf("%s", "telephone-event");

    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);
    AudioProfile::TelephoneEventFmtp* pTelephoneEventFmtp =
            new AudioProfile::TelephoneEventFmtp(pDtmfConfig->GetEvents());

    AudioProfile::Payload* pTelephoneEventPayload = new AudioProfile::Payload();
    pTelephoneEventPayload->SetRtpMap(
            pDtmfConfig->GetPayloadType(), strCodecName, pDtmfConfig->GetSamplingRate(), 0);
    pTelephoneEventPayload->pFmtp = pTelephoneEventFmtp;

    IMS_TRACE_D("CreateTelephoneEventPayload() codec(%s), SamplingRate(%d)",
            ImsCodec::CodecToString(pDtmfConfig->GetCodec()), pDtmfConfig->GetSamplingRate(), 0);

    return pTelephoneEventPayload;
}

PRIVATE AudioProfile::Payload* MediaProfileFactory::CreatePcmPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("CreatePcmPayload", 0, 0, 0);

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

    CodecPcmConfig* pPcmConfig = static_cast<CodecPcmConfig*>(pCodecConfig);
    AudioConfiguration* pAudioConfig = static_cast<AudioConfiguration*>(pConfig);

    return pPcmPayload;
}

PRIVATE void MediaProfileFactory::SetAudioCodecFmtp(IN CodecAudioConfig* pCodecConfig,
        IN AudioConfiguration* pAudioConfig, OUT AudioProfile::AudioFmtp* pFmtp)
{
    if (pCodecConfig == IMS_NULL || pAudioConfig == IMS_NULL || pFmtp == IMS_NULL)
    {
        return;
    }

    if (pCodecConfig->GetModeChangeCapability() != NOT_PRESENT)
    {
        pFmtp->nModeChangeCapability = pCodecConfig->GetModeChangeCapability();
        pFmtp->bShowModeChangeCapability = IMS_TRUE;
    }
    else
    {
        pFmtp->nModeChangeCapability = CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY;
        pFmtp->bShowModeChangeCapability = IMS_FALSE;
    }

    if (pCodecConfig->GetModeChangePeriod() != NOT_PRESENT)
    {
        pFmtp->nModeChangePeriod = pCodecConfig->GetModeChangePeriod();
        pFmtp->bShowModeChangePeriod = IMS_TRUE;
    }
    else
    {
        pFmtp->nModeChangePeriod = CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD;
        pFmtp->bShowModeChangePeriod = IMS_FALSE;
    }

    if (pCodecConfig->GetModeChangeNeighbor() != NOT_PRESENT)
    {
        pFmtp->nModeChangeNeighbor = pCodecConfig->GetModeChangeNeighbor();
        pFmtp->bShowModeChangeNeighbor = IMS_TRUE;
    }
    else
    {
        pFmtp->nModeChangeNeighbor = CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR;
        pFmtp->bShowModeChangeNeighbor = IMS_FALSE;
    }

    if (pAudioConfig->GetPtime() != NOT_PRESENT)
    {
        pFmtp->nPtime = pAudioConfig->GetPtime();
        pFmtp->bShowPtime = IMS_TRUE;
    }
    else
    {
        pFmtp->nPtime = AudioConfiguration::DEFAULT_PTIME;
        pFmtp->bShowPtime = IMS_FALSE;
    }

    if (pAudioConfig->GetMaxPtime() != NOT_PRESENT)
    {
        pFmtp->nMaxPtime = pAudioConfig->GetMaxPtime();
        pFmtp->bShowMaxPtime = IMS_TRUE;
    }
    else
    {
        pFmtp->nMaxPtime = AudioConfiguration::DEFAULT_MAX_PTIME;
        pFmtp->bShowMaxPtime = IMS_FALSE;
    }

    if (pAudioConfig->GetMaxRed() != NOT_PRESENT)
    {
        pFmtp->nMaxRed = pAudioConfig->GetMaxRed();
        pFmtp->bShowMaxRed = IMS_TRUE;
    }
    else
    {
        pFmtp->nMaxRed = AudioConfiguration::DEFAULT_MAX_RED;
        pFmtp->bShowMaxRed = IMS_FALSE;
    }

    pFmtp->bDtx = pCodecConfig->GetDtx();
}

PRIVATE TextProfile* MediaProfileFactory::SetTextProfile(
        IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    TextProfile* pTextProfile = static_cast<TextProfile*>(pProfile);
    TextConfiguration* pTextConfig = static_cast<TextConfiguration*>(pConfig);

    pTextProfile->strTransportType = "RTP/AVP";
    pTextProfile->bKeepRedLevel = pTextConfig->IsTextCodecEmptyRedundantEnabled();

    IMS_TRACE_I("SetTextProfile() - transport type[%s], keep red level[%d]",
            pTextProfile->strTransportType.GetStr(), pTextProfile->bKeepRedLevel, 0);

    return pTextProfile;
}

PRIVATE TextProfile::Payload* MediaProfileFactory::CreateT140Payload(
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
                pRedFmtp->nRedLevel, pRedFmtp->nRedPayload, 0);

        pTextPayload->pFmtp = pRedFmtp;
    }
    else
    {
        strCodecName.Sprintf("%s", "t140");
    }

    pTextPayload->SetRtpMap(
            pT140Config->GetPayloadType(), strCodecName, pT140Config->GetSamplingRate());

    return pTextPayload;
}

PRIVATE VideoProfile* MediaProfileFactory::SetVideoProfile(
        IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    VideoProfile* pVideoProfile = static_cast<VideoProfile*>(pProfile);
    VideoConfiguration* pVideoConfig = static_cast<VideoConfiguration*>(pConfig);

    pVideoProfile->strTransportType = (pVideoConfig->IsVideoAvpfEnabled() &&
                                              !pVideoConfig->IsAvpfCapabilityNegotiationEnabled())
            ? "RTP/AVPF"
            : "RTP/AVP";

    pVideoProfile->nCvoId = pVideoConfig->GetCvoId();
    VideoProfileUtil::SetVideoRsRr(pVideoProfile, pVideoConfig);

    if (pVideoConfig->IsVideoAvpfEnabled() == IMS_TRUE)
    {
        pVideoProfile->bSupportAvpf = IMS_TRUE;

        IMS_SINT32 nTcap, nAcap = 0;

        nTcap = SetTransportCapa(pVideoProfile);
        nAcap = SetAttributeCapa(pVideoProfile, pVideoConfig);

        SetCapaNegoForAvpf(pVideoProfile, pVideoConfig->GetSdpOfferCapNegoForAvpf(), nTcap, nAcap);
    }

    IMS_TRACE_D("SetVideoProfile - SupportAvpf[%d], SupportCapaNegoForAvpf[%d]",
            pVideoProfile->bSupportAvpf, pVideoProfile->bSupportCapaNegoForAvpf, 0);

    return pVideoProfile;
}

PRIVATE VideoProfile::Payload* MediaProfileFactory::CreateAvcPayload(
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

    pAvcFmtp->nProfile =
            VideoProfileUtil::GetAvcProfileFromProfileLevelId(pAvcConfig->GetProfileLevelId());
    pAvcFmtp->nLevel =
            VideoProfileUtil::GetAvcLevelFromProfileLevelId(pAvcConfig->GetProfileLevelId());
    IMS_TRACE_I(
            "CreateAvcPayload - nProfile[%d], nLevel[%d]", pAvcFmtp->nProfile, pAvcFmtp->nLevel, 0);

    const IMS_CHAR* pbAvc4SpropParameterSets;
    pbAvc4SpropParameterSets = pAvcConfig->GetSpropParameterSets().GetStr();

    /** TODO: later sprop need to find a way to get SpropPramaterSets
    pbAvc4SpropParameterSets = GetAvcSpropParameterSets(
            pAvcFmtp->eResolution, pAvcFmtp->nProfile, pAvcFmtp->nLevel);
    */

    IMS_TRACE_I("CreateAvcPayload - pbAvc4SpropParameterSets[%s]", pbAvc4SpropParameterSets, 0, 0);

    if (pAvcConfig->GetProfileLevelId().GetLength() != 0)
    {
        pAvcFmtp->strProfileLevelId = pAvcConfig->GetProfileLevelId();
        pAvcFmtp->bShow_ProfileLevelId = IMS_TRUE;
    }

    if (pAvcConfig->GetIncludeSpropParameterSets())
    {
        pAvcFmtp->strSpropParam = pbAvc4SpropParameterSets;
        pAvcFmtp->bShow_SpropParam = IMS_TRUE;
    }

    VideoProfile::Payload* pAvcPayload = new VideoProfile::Payload();
    pAvcPayload->pFmtp = pAvcFmtp;

    SetVideoCodecPayload(pAvcConfig, pVideoConfig, pAvcPayload);

    return pAvcPayload;
}

PRIVATE VideoProfile::Payload* MediaProfileFactory::CreateHevcPayload(
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
        pHevcFmtp->nProfile = static_cast<VIDEO_PROFILE_HEVC>(pHevcConfig->GetHevcProfile());
        pHevcFmtp->bShow_Profile = IMS_TRUE;
    }

    if (pHevcConfig->GetHevcLevel() != -1)
    {
        pHevcFmtp->nLevel = pHevcConfig->GetHevcLevel();
        pHevcFmtp->bShow_Level = IMS_TRUE;
    }

    if (pHevcConfig->GetSpropParameterSets().GetLength() != 0)
    {
        pHevcFmtp->strSpropParam = pHevcConfig->GetSpropParameterSets();
        pHevcFmtp->bShow_SpropParam = IMS_TRUE;
    }

    VideoProfile::Payload* pHevcPayload = new VideoProfile::Payload();
    pHevcPayload->pFmtp = pHevcFmtp;

    SetVideoCodecPayload(pHevcConfig, pVideoConfig, pHevcPayload);

    return pHevcPayload;
}

PRIVATE void MediaProfileFactory::SetVideoCodecFmtp(IN CodecVideoConfig* pCodecConfig,
        IN VideoConfiguration* pVideoConfig, OUT VideoProfile::VideoFmtp* pFmtp)
{
    if (pCodecConfig == IMS_NULL || pVideoConfig == IMS_NULL || pFmtp == IMS_NULL)
    {
        return;
    }

    pFmtp->nFrameRate = pCodecConfig->GetFramerate();
    pFmtp->eResolution = VideoProfileUtil::GetResolutionFromWidthHeight(
            pCodecConfig->GetResolutionWidth(), pCodecConfig->GetResolutionHeight());
    pFmtp->nBitrate = pCodecConfig->GetBitrate();
    pFmtp->nAs = pVideoConfig->GetAsBandwidthKbps();

    if (pCodecConfig->GetPacketizationMode() != -1)
    {
        pFmtp->nPacketizationMode = pCodecConfig->GetPacketizationMode();
        pFmtp->bShow_PacketizationMode = IMS_TRUE;
    }

    IMS_TRACE_D("SetVideoCodecFmtp - FrameRate[%d], Resolution[%d], Bitrate[%d]", pFmtp->nFrameRate,
            pFmtp->eResolution, pFmtp->nBitrate);
    IMS_TRACE_D("SetVideoCodecFmtp - AS[%d], PacketizationMode[%d]", pFmtp->nAs,
            pFmtp->nPacketizationMode, 0);
}

PRIVATE void MediaProfileFactory::SetVideoCodecPayload(IN CodecVideoConfig* pCodecConfig,
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
        pPayload->bIncludeImageAttr = IMS_TRUE;
        pPayload->strImageAttr = pCodecConfig->GetImageAttr();
    }
    else if (pCodecConfig->GetFrameSize().GetLength() != 0)
    {
        pPayload->bIncludeFrameSize = IMS_TRUE;
    }

    if (pVideoConfig->IsVideoAvpfEnabled() == IMS_TRUE)
    {
        if (pVideoConfig->IsVideoAvpfTrrEnabled() == IMS_TRUE)
        {
            pPayload->objRtcpFbAttr.bTrrSupported = IMS_TRUE;
            pPayload->objRtcpFbAttr.nTrrInt = pVideoConfig->GetRtcpInterval() * 1000;
        }

        if (pVideoConfig->IsVideoAvpfNackEnabled() == IMS_TRUE)
        {
            pPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
        }

        if (pVideoConfig->IsVideoAvpfTmmbrEnabled() == IMS_TRUE)
        {
            pPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
            pPayload->objRtcpFbAttr.nTmmbrSmaxPr = 40;
        }

        if (pVideoConfig->IsVideoAvpfPliEnabled() == IMS_TRUE)
        {
            pPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
        }

        if (pVideoConfig->IsVideoAvpfFirEnabled() == IMS_TRUE)
        {
            pPayload->objRtcpFbAttr.bFirSupported = IMS_TRUE;
        }

        IMS_TRACE_I("SetVideoCodecPayload() AVPF. TRR[%d], NACK[%d], TMMBR[%d]",
                pPayload->objRtcpFbAttr.bTrrSupported, pPayload->objRtcpFbAttr.bNackSupported,
                pPayload->objRtcpFbAttr.bTmmbrSupported);
        IMS_TRACE_I("SetVideoCodecPayload() AVPF. PLI[%d], FIR[%d]",
                pPayload->objRtcpFbAttr.bPliSupported, pPayload->objRtcpFbAttr.bFirSupported, 0);
    }
}

PRIVATE IMS_SINT32 MediaProfileFactory::SetTransportCapa(OUT VideoProfile* pVideoProfile)
{
    IMS_SINT32 nTcap = 0;

    if (pVideoProfile != IMS_NULL)
    {
        AString strTemp("RTP/AVPF");
        pVideoProfile->objCapaNego.mapTransportCapa.SetValue(++nTcap, strTemp);

        IMS_TRACE_I("SetTransportCapa() - Tcap[%d][%s]", nTcap,
                pVideoProfile->objCapaNego.mapTransportCapa.GetValue(nTcap).GetStr(), 0);
    }

    return nTcap;
}

PRIVATE IMS_SINT32 MediaProfileFactory::SetAttributeCapa(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig)
{
    IMS_SINT32 nAcap = 0;

    if (pVideoProfile != IMS_NULL)
    {
        nAcap = SetVideoAvpfTrr(pVideoProfile, pVideoConfig, nAcap);
        nAcap = SetVideoAvpfNack(pVideoProfile, pVideoConfig, nAcap);
        nAcap = SetVideoAvpfPli(pVideoProfile, pVideoConfig, nAcap);
        nAcap = SetVideoAvpfFir(pVideoProfile, pVideoConfig, nAcap);
        nAcap = SetVideoAvpfTmmbr(pVideoProfile, pVideoConfig, nAcap);
    }

    IMS_TRACE_I("SetAttributeCapa() - Acap[%d]", nAcap, 0, 0);

    return nAcap;
}

PRIVATE IMS_SINT32 MediaProfileFactory::SetVideoAvpfTrr(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfTrrEnabled() == IMS_TRUE)
        {
            AString strTemp = AString::ConstEmpty();
            strTemp.Sprintf("%s %d", "rtcp-fb:* trr-int", pVideoConfig->GetRtcpInterval() * 1000);
            pVideoProfile->objCapaNego.mapAttributeCapa.SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetVideoAvpfTrr() - Acap[%d][%s]", nAcap,
                    pVideoProfile->objCapaNego.mapAttributeCapa.GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PRIVATE IMS_SINT32 MediaProfileFactory::SetVideoAvpfNack(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfNackEnabled() == IMS_TRUE)
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* nack";
            pVideoProfile->objCapaNego.mapAttributeCapa.SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetVideoAvpfNack() - Acap[%d][%s]", nAcap,
                    pVideoProfile->objCapaNego.mapAttributeCapa.GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PRIVATE IMS_SINT32 MediaProfileFactory::SetVideoAvpfPli(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfPliEnabled() == IMS_TRUE)
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* nack pli";
            pVideoProfile->objCapaNego.mapAttributeCapa.SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetVideoAvpfPli() - Acap[%d][%s]", nAcap,
                    pVideoProfile->objCapaNego.mapAttributeCapa.GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PRIVATE IMS_SINT32 MediaProfileFactory::SetVideoAvpfFir(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfFirEnabled() == IMS_TRUE)
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* ccm fir";
            pVideoProfile->objCapaNego.mapAttributeCapa.SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetVideoAvpfFir() - Acap[%d][%s]", nAcap,
                    pVideoProfile->objCapaNego.mapAttributeCapa.GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PRIVATE IMS_SINT32 MediaProfileFactory::SetVideoAvpfTmmbr(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfTmmbrEnabled() == IMS_TRUE)
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* ccm tmmbr";
            pVideoProfile->objCapaNego.mapAttributeCapa.SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetVideoAvpfTmmbr() - Acap[%d][%s]", nAcap,
                    pVideoProfile->objCapaNego.mapAttributeCapa.GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PRIVATE void MediaProfileFactory::SetCapaNegoForAvpf(OUT VideoProfile* pVideoProfile,
        IN IMS_SINT32 nCapaNegoForAvpfOption, IN IMS_SINT32 nTcap, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("SetCapaNegoForAvpf() - Acap[%d], Tcap[%d]", nTcap, nAcap, 0);

    pVideoProfile->bSupportCapaNegoForAvpf =
            (nCapaNegoForAvpfOption > MediaConfiguration::CAPNEG_OFFER_NONE) ? IMS_TRUE : IMS_FALSE;

    if (pVideoProfile->bSupportCapaNegoForAvpf == IMS_TRUE)
    {
        AString strPcfg = AString::ConstNull();
        strPcfg.Sprintf("t=%d", nTcap);

        if (nCapaNegoForAvpfOption == MediaConfiguration::CAPNEG_OFFER_WITHOUT_ACAP)
        {
            pVideoProfile->objCapaNego.bIsAttCapaInPcfg = IMS_FALSE;
        }
        else if (nCapaNegoForAvpfOption == MediaConfiguration::CAPNEG_OFFER_WITH_ACAP)
        {
            strPcfg.Append(" a=");
            AString strTmp = AString::ConstNull();

            for (IMS_SINT32 i = 1; i <= nAcap; i++)
            {
                if (strTmp.GetLength() > 0)
                    strTmp.Append(",");

                AString strTmp2;
                strTmp2.Sprintf("%d", i);
                strTmp.Append(strTmp2);
            }
            strPcfg.Append(strTmp);
            pVideoProfile->objCapaNego.bIsAttCapaInPcfg = IMS_TRUE;
        }
        pVideoProfile->objCapaNego.lstPotentialConfig.Append(strPcfg);
    }
}

PRIVATE void MediaProfileFactory::SetMaxProfileFrameRate(OUT VideoProfile* pVideoProfile)
{
    if (pVideoProfile == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nMaxFrameRate = -1;

    for (IMS_UINT32 i = 0; i < pVideoProfile->lstPayload.GetSize(); i++)
    {
        VideoProfile::Payload* pPayload = pVideoProfile->lstPayload.GetAt(i);

        if (pPayload == IMS_NULL || pPayload->pFmtp == IMS_NULL)
        {
            continue;
        }

        IMS_SINT32 nFrameRate = static_cast<VideoProfile::VideoFmtp*>(pPayload->pFmtp)->nFrameRate;

        if (nFrameRate > nMaxFrameRate)
        {
            nMaxFrameRate = nFrameRate;
        }

        IMS_TRACE_I("SetMaxProfileFrameRate() - nMaxFrameRate[%d], payload framerate[%d]",
                nMaxFrameRate, nFrameRate, 0);
    }

    pVideoProfile->nFrameRate = nMaxFrameRate;
}
