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

#include "ServiceTrace.h"
#include "config/ImsCodec.h"
#include "config/CodecAmrConfig.h"
#include "config/CodecTelephoneEventConfig.h"
#include "config/CodecEvsConfig.h"
#include "config/CodecPcmConfig.h"
#include "config/AudioConfiguration.h"
#include "audio/AudioDef.h"
#include "MediaEnvironment.h"
#include "IService.h"
#include "audio/AudioProfileUtil.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "MediaManager.h"

__IMS_TRACE_TAG_USER_DECL__("MED.PC");

const IMS_SINT32 AudioProfileUtil::AMR_AS[8][9] = {
        {22, 22, 23, 24, 24, 25, 27, 29, 0 }, // nb, ipv4, be
        {22, 22, 23, 24, 25, 25, 28, 30, 0 }, // nb, ipv4, oa
        {30, 30, 31, 32, 32, 33, 35, 37, 0 }, // nb, ipv6, be
        {30, 30, 31, 32, 33, 33, 36, 38, 0 }, // nb, ipv6, oa
        {24, 26, 30, 31, 33, 35, 37, 40, 41}, // wb, ipv4, be
        {24, 26, 30, 32, 33, 36, 37, 40, 41}, // wb, ipv4, oa
        {32, 34, 38, 39, 41, 43, 45, 48, 49}, // wb, ipv6, be
        {32, 34, 38, 40, 41, 44, 45, 48, 49}  // wb, ipv6, oa
};

const IMS_SINT32 AudioProfileUtil::EVS_AS[4][12] = {
        {23, 24, 25, 27, 30, 34, 42, 49, 65, 81, 113, 145}, // Primary, ipv4
        {31, 32, 33, 35, 38, 42, 50, 57, 73, 89, 121, 153}, // Primary, ipv6
        {23, 26, 29, 31, 32, 35, 36, 40, 40, 0,  0,   0  }, // AMR IO, ipv4
        {31, 34, 37, 39, 40, 43, 44, 48, 48, 0,  0,   0  }  // AMR IO, ipv6
};

const AString AudioProfileUtil::EVS_BR[EVS_BR_CNT] = {
        "5.9", "7.2", "8", "9.6", "13.2", "16.4", "24.4", "32", "48", "64", "96", "128"};
const AString AudioProfileUtil::EVS_BW[EVS_BW_CNT] = {"nb", "wb", "swb", "fb"};
const AString AudioProfileUtil::EVS_BW_LIST[EVS_BW_LIST_CNT] = {
        "nb", "wb", "swb", "fb", "nb-wb", "nb-swb", "nb-fb", "wb-swb", "wb-fb"};
const AString AudioProfileUtil::AUDIO_CODEC_BANDWIDTH_STRING[EVS_BW_CNT] = {
        "NB", "WB", "SWB", "FB"};
const AString AudioProfileUtil::AUDIO_CODEC_BITRATE_STRING[3][9] = {
  // AMR NB
        {"4.75", "5.15", "5.90",  "6.70",  "7.40",  "7.95",  "10.20", "12.20", "0"    },
 // AMR WB/EVS AMR IO
        {"6.60", "8.85", "12.65", "14.25", "15.85", "18.25", "19.85", "23.05", "23.85"},
 // EVS
        {"5.90", "7.20", "8.00",  "9.60",  "13.20", "16.40", "24.40", "0",     "0"    }
};

PUBLIC
AudioProfile* AudioProfileUtil::CreateProfile(
        IN MediaEnvironment* pEnvironment, IN AudioConfiguration* pConfig, IN IMS_SINT32 nSlotId)
{
    IMS_SINT32 nAsOptimal = -1;
    IMS_SINT32 nAsMax = -1;

    if (pEnvironment == IMS_NULL || pConfig == IMS_NULL || pEnvironment->pIService == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_D("CreateProfile()", 0, 0, 0);

    MediaManager* pMediaManager = MediaManager::GetInstance(nSlotId);

    if (pMediaManager == IMS_NULL)
    {
        return IMS_NULL;
    }

    MediaResourceMngr* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr == IMS_NULL)
    {
        return IMS_NULL;
    }

    AudioProfile* pAudioProfile = new AudioProfile();

    // Setting IP address
    // In IPv6 system, The MediaLogic have to write a modem IPv6 to "c line" of session level.
    // In Ipv4, this is not necessary.
    if (pEnvironment->eNetworkType == MEDIA_NETWORK_WIFI)
    {
        pAudioProfile->objIpAddr = pEnvironment->pIService->GetIpAddress();
    }
    else
    {
        IMS_SINT32 nPDNType = MediaResourceMngr::PDN_IMS;

        if (pEnvironment->eServiceType == MEDIA_SERVICE_EMERGENCY)
        {
            nPDNType = MediaResourceMngr::PDN_EMERGENCY;
        }

        pResourceMngr->UpdatePdnResource(
                nPDNType, pEnvironment->pIService->GetIpAddress().IsIPv6Address());
        // Android IP
        pAudioProfile->objIpAddr = pEnvironment->pIService->GetIpAddress();
    }

    // Setting RTP/RTCP port from resource manager
    if (pAudioProfile->nDataPort > 0)
    {
        pAudioProfile->nDataPort =
                pResourceMngr->AcquireRtpPort(pAudioProfile->nDataPort, pAudioProfile->nDataPort);
    }
    else
    {
        pAudioProfile->nDataPort = pResourceMngr->AcquireRtpPort(pConfig);
    }
    pAudioProfile->nControlPort = pAudioProfile->nDataPort + 1;

    IMS_TRACE_D("CreateProfile() objIpAddr[%s], port[%d]", pAudioProfile->objIpAddr.ToCharString(),
            pAudioProfile->nDataPort, 0);

    pAudioProfile->strTransportType = "RTP/AVP";

    while (pAudioProfile->lstPayload.GetSize() > 0)
    {
        AudioProfile::Payload* pPayload = pAudioProfile->lstPayload.GetAt(0);
        if (pPayload != IMS_NULL)
        {
            delete pPayload;
        }
        pAudioProfile->lstPayload.RemoveAt(0);
    }

    IMS_BOOL bRtcpXr = pConfig->IsRtcpXrEnabled();

    // RTCP-XR
    if (bRtcpXr == IMS_TRUE)
    {
        pAudioProfile->bSupportRtcpXr = IMS_TRUE;
        if (pConfig->IsRtcpXrVoipEnabled() == IMS_TRUE)
        {
            pAudioProfile->objRtcpXrAttr.bSupportVoipMatircs = IMS_TRUE;
        }
        if (pConfig->IsRtcpXrStatisticsEnabled() == IMS_TRUE)
        {
            pAudioProfile->objRtcpXrAttr.bSupportStatisticMetrics = IMS_TRUE;
        }
        if (pConfig->IsRtcpXrPlrEnabled() == IMS_TRUE)
        {
            pAudioProfile->objRtcpXrAttr.bSupportPacketLossRle = IMS_TRUE;
        }
        if (pConfig->IsRtcpXrPdrEnabled() == IMS_TRUE)
        {
            pAudioProfile->objRtcpXrAttr.bSupportPacketDuplicatedRle = IMS_TRUE;
        }

        IMS_TRACE_D("CreateProfile() Add RTCP-XR. VoIP[%d], Stat[%d], PLR[%d]",
                pAudioProfile->objRtcpXrAttr.bSupportVoipMatircs,
                pAudioProfile->objRtcpXrAttr.bSupportStatisticMetrics,
                pAudioProfile->objRtcpXrAttr.bSupportPacketLossRle);
        IMS_TRACE_D("CreateProfile() Add RTCP-XR. PDR[%d]",
                pAudioProfile->objRtcpXrAttr.bSupportPacketDuplicatedRle, 0, 0);
    }

    pAudioProfile->objCandidateAttr = pConfig->GetAudioCandidateAttribute();

    // Setting each payload and bandwidth
    IMSList<CodecConfig*> pCodecs;
    pCodecs = pConfig->GetCodecConfigs();

    for (IMS_UINT32 i = 0; i < pCodecs.GetSize(); i++)
    {
        CodecConfig* pCodecConfig = pCodecs.GetAt(i);

        if (pCodecConfig == IMS_NULL)
        {
            IMS_TRACE_D("pCodecConfig is NULL", 0, 0, 0);
            break;
        }

        if (pCodecConfig->GetCodec() == ImsCodec::AUDIO_NONE)
        {
            IMS_TRACE_D("CreateProfile() invalid codec, skip config(%d) - %d", i,
                    pCodecConfig->GetCodec(), 0);
            continue;
        }

        if (pCodecConfig->GetPayloadType() == -1)
        {
            IMS_TRACE_D("CreateProfile() invalid payload type, skip config(%d) - %d:%s", i,
                    pCodecConfig->GetCodec(), ImsCodec::CodecToString(pCodecConfig->GetCodec()));
            continue;
        }

        if (pCodecConfig->GetCodec() == ImsCodec::AUDIO_AMR ||
                pCodecConfig->GetCodec() == ImsCodec::AUDIO_AMR_WB)
        {
            CodecAmrConfig* pAmrConfig = reinterpret_cast<CodecAmrConfig*>(pCodecConfig);
            AString strCodecName;
            AUDIO_CODEC nCurrCodec;
            AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp();
            pAmrFmtp->nModeSetList = pAmrConfig->GetModeSetList();
            pAmrFmtp->nDefaultRtpModeSet = pAmrConfig->GetDefaultModeSetList();
            pAmrFmtp->bShowModeSet = pAmrConfig->GetShowModeSet();

            pAmrFmtp->bShow_OctetAlign = IMS_FALSE;
            if (pAmrConfig->GetOctetAlign() != -1)
            {
                pAmrFmtp->nOctetAlign = pAmrConfig->GetOctetAlign();
                if (pAmrFmtp->nOctetAlign == 1)
                {
                    pAmrFmtp->bShow_OctetAlign = IMS_TRUE;
                }
            }
            else
            {
                pAmrFmtp->nOctetAlign = CodecAmrConfig::DEFAULT_OCTET_ALIGN;
            }

            if (pConfig->GetModeChangeCapability() != -1)
            {
                pAmrFmtp->nModeChangeCapability = pConfig->GetModeChangeCapability();
                pAmrFmtp->bShowModeChangeCapability = IMS_TRUE;
            }
            else
            {
                pAmrFmtp->nModeChangeCapability = AudioConfiguration::DEFAULT_MODECHANGE_CAPABILITY;
                pAmrFmtp->bShowModeChangeCapability = IMS_FALSE;
            }

            if (pConfig->GetModeChangePeriod() != -1)
            {
                pAmrFmtp->nModeChangePeriod = pConfig->GetModeChangePeriod();
                pAmrFmtp->bShowModeChangePeriod = IMS_TRUE;
            }
            else
            {
                pAmrFmtp->nModeChangePeriod = AudioConfiguration::DEFAULT_MODECHANGE_PERIOD;
                pAmrFmtp->bShowModeChangePeriod = IMS_FALSE;
            }

            if (pConfig->GetModeChangeNeighbor() != -1)
            {
                pAmrFmtp->nModeChangeNeighbor = pConfig->GetModeChangeNeighbor();
                pAmrFmtp->bShowModeChangeNeighbor = IMS_TRUE;
            }
            else
            {
                pAmrFmtp->nModeChangeNeighbor = AudioConfiguration::DEFAULT_MODECHANGE_NEIGHBOR;
                pAmrFmtp->bShowModeChangeNeighbor = IMS_FALSE;
            }

            if (pConfig->GetMaxRed() != -1)
            {
                pAmrFmtp->nMaxRed = pConfig->GetMaxRed();
                pAmrFmtp->bShowMaxRed = IMS_TRUE;
            }
            else
            {
                pAmrFmtp->nMaxRed = AudioConfiguration::DEFAULT_MAX_RED;
                pAmrFmtp->bShowMaxRed = IMS_FALSE;
            }
            if (pConfig->GetPtime() != -1)
            {
                pAmrFmtp->nPtime = pConfig->GetPtime();
            }
            else
            {
                pAmrFmtp->nPtime = AudioConfiguration::DEFAULT_PTIME;
                pAmrFmtp->bShowPtime = IMS_FALSE;
            }
            if (pConfig->GetMaxPtime() != -1)
            {
                pAmrFmtp->nMaxPtime = pConfig->GetMaxPtime();
            }
            else
            {
                pAmrFmtp->nMaxPtime = AudioConfiguration::DEFAULT_MAX_PTIME;
                pAmrFmtp->bShowMaxPtime = IMS_FALSE;
            }
            pAmrFmtp->bSCREnable = pAmrConfig->GetDtx();

            if (pAmrConfig->GetSamplingRate() == 8000)
            {
                nCurrCodec = AUDIO_CODEC_AMR;
                strCodecName.Sprintf("%s", "AMR");
            }
            else
            {
                nCurrCodec = AUDIO_CODEC_AMRWB;
                strCodecName.Sprintf("%s", "AMR-WB");
            }

            AudioProfile::Payload* pAmrPayload = new AudioProfile::Payload();
            pAmrPayload->SetRtpMap(pAmrConfig->GetPayloadType(), strCodecName,
                    pAmrConfig->GetSamplingRate(), pAmrConfig->GetChannel());
            pAmrPayload->pFmtp = reinterpret_cast<void*>(pAmrFmtp);
            pAudioProfile->lstPayload.Append(pAmrPayload);
            IMS_SINT32 nCurrAs;
            nCurrAs = ConvertToBandwidthAS(nCurrCodec, pAmrFmtp->nOctetAlign,
                    pAudioProfile->objIpAddr.IsIPv6Address(), pAmrConfig->GetModeSet());

            if (nCurrAs > nAsOptimal)
            {
                nAsOptimal = nCurrAs;
            }

            nCurrAs = ConvertToBandwidthAS(nCurrCodec, pAmrFmtp->nOctetAlign,
                    pAudioProfile->objIpAddr.IsIPv6Address(), pAmrConfig->GetModeSet(), IMS_TRUE);

            if (nCurrAs > nAsMax)
            {
                nAsMax = nCurrAs;
            }

            IMS_TRACE_D("CreateProfile() add payload(%d), codec(%s), SamplingRate(%d)", i,
                    ImsCodec::CodecToString(pAmrConfig->GetCodec()), pAmrConfig->GetSamplingRate());
        }
        else if (pCodecConfig->GetCodec() == ImsCodec::AUDIO_TELEPHONE_EVENT ||
                pCodecConfig->GetCodec() == ImsCodec::AUDIO_TELEPHONE_EVENT_WB)
        {
            CodecTelephoneEventConfig* pDtmfConfig =
                    reinterpret_cast<CodecTelephoneEventConfig*>(pCodecConfig);
            AString strCodecName;
            strCodecName.Sprintf("%s", "telephone-event");
            AudioProfile::TelephoneEventFmtp* pTeFmtp =
                    new AudioProfile::TelephoneEventFmtp(pDtmfConfig->GetEvents());
            AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
            pTelephoneEvent->SetRtpMap(
                    pDtmfConfig->GetPayloadType(), strCodecName, pDtmfConfig->GetSamplingRate(), 0);
            pTelephoneEvent->pFmtp = reinterpret_cast<void*>(pTeFmtp);
            pAudioProfile->lstPayload.Append(pTelephoneEvent);

            IMS_TRACE_D("CreateProfile() add payload(%d),codec(%d), SamplingRate(%d)", i,
                    pDtmfConfig->GetCodec(), pDtmfConfig->GetSamplingRate());
        }
        else if (pCodecConfig->GetCodec() == ImsCodec::AUDIO_EVS)
        {
            // make EVS Profile
            CodecEvsConfig* pEvsConfig = reinterpret_cast<CodecEvsConfig*>(pCodecConfig);
            AUDIO_CODEC nCurrCodec = AUDIO_CODEC_EVS;
            AString strCodecName;
            strCodecName.Sprintf("%s", "EVS");

            AudioProfile::EvsFmtp* pEvsFmtp = new AudioProfile::EvsFmtp();

            // Mode set list
            pEvsFmtp->nModeSetList = pEvsConfig->GetAmrWbIoModeSetList();
            pEvsFmtp->nDefaultRtpModeSet = pEvsConfig->GetDefaultModeSetList();
            pEvsFmtp->bShowModeSetList = pEvsConfig->GetShowAmrwbIoModeSet();
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

            if (pConfig->GetModeChangeCapability() == -1)  // Not Present
            {
                pEvsFmtp->nModeChangeCapability = AudioConfiguration::DEFAULT_MODECHANGE_CAPABILITY;
            }
            else
            {
                pEvsFmtp->nModeChangeCapability = pConfig->GetModeChangeCapability();
                pEvsFmtp->bShowModeChangeCapability = IMS_TRUE;
            }

            if (pConfig->GetModeChangePeriod() == -1)  // Not Present
            {
                pEvsFmtp->nModeChangePeriod = AudioConfiguration::DEFAULT_MODECHANGE_PERIOD;
            }
            else
            {
                pEvsFmtp->nModeChangePeriod = pConfig->GetModeChangePeriod();
                pEvsFmtp->bShowModeChangePeriod = IMS_TRUE;
            }

            if (pConfig->GetModeChangeNeighbor() == -1)  // Not Present
            {
                pEvsFmtp->nModeChangeNeighbor = AudioConfiguration::DEFAULT_MODECHANGE_NEIGHBOR;
            }
            else
            {
                pEvsFmtp->nModeChangeNeighbor = pConfig->GetModeChangeNeighbor();
                pEvsFmtp->bShowModeChangeNeighbor = IMS_TRUE;
            }

            if (pConfig->GetPtime() != -1)
            {
                pEvsFmtp->nPtime = pConfig->GetPtime();
            }
            else
            {
                pEvsFmtp->nPtime = AudioConfiguration::DEFAULT_PTIME;
                pEvsFmtp->bShowPtime = IMS_FALSE;
            }
            if (pConfig->GetMaxPtime() != -1)
            {
                pEvsFmtp->nMaxPtime = pConfig->GetMaxPtime();
            }
            else
            {
                pEvsFmtp->nMaxPtime = AudioConfiguration::DEFAULT_MAX_PTIME;
                pEvsFmtp->bShowMaxPtime = IMS_FALSE;
            }

            if (pConfig->GetMaxRed() != -1)
            {
                pEvsFmtp->nMaxRed = pConfig->GetMaxRed();
                pEvsFmtp->bShowMaxRed = IMS_TRUE;
            }
            else
            {
                pEvsFmtp->nMaxRed = AudioConfiguration::DEFAULT_MAX_RED;
                pEvsFmtp->bShowMaxRed = IMS_FALSE;
            }

            pEvsFmtp->nDtx = pEvsConfig->GetDtx();
            pEvsFmtp->bShowDtx = pEvsConfig->GetShowDtx();

            if (pEvsConfig->GetDtxRecv() == IMS_TRUE)  // true or Not Present
            {
                pEvsFmtp->nDtx_Recv = pEvsFmtp->nDtx;
            }
            else
            {
                pEvsFmtp->nDtx_Recv = pEvsConfig->GetDtxRecv();
            }

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

            IMS_TRACE_D("EVS - GetShowDtx: %d GetShowAmrwbIoModeSet: %d", pEvsConfig->GetShowDtx(),
                    pEvsConfig->GetShowAmrwbIoModeSet(), 0);

            // set EVS codec fmtp
            AudioProfile::Payload* pEvsPayload = new AudioProfile::Payload();
            pEvsPayload->SetRtpMap(
                    pEvsConfig->GetPayloadType(), strCodecName, 16000, pEvsConfig->GetChannel());
            pEvsPayload->pFmtp = reinterpret_cast<void*>(pEvsFmtp);

            pAudioProfile->lstPayload.Append(pEvsPayload);

            // set AS value about EVS codec.
            IMS_SINT32 nCurrAs = 0;
            IMS_SINT32 nTempCodecMode = pEvsConfig->GetEvsModeSwitch();

            if (nTempCodecMode == 1)
            {
                nTempCodecMode = 1;
            }
            else
            {
                nTempCodecMode = 0;
            }

            nCurrAs = ConvertToBandwidthAS(nCurrCodec, pAudioProfile->objIpAddr.IsIPv6Address(),
                    nTempCodecMode, pEvsConfig->GetBr());  // primary mode
            if (nCurrAs > nAsOptimal)
            {
                nAsOptimal = nCurrAs;
            }

            nCurrAs = ConvertToBandwidthAS(nCurrCodec, pAudioProfile->objIpAddr.IsIPv6Address(),
                    nTempCodecMode, pEvsConfig->GetBr(), IMS_TRUE);
            if (nCurrAs > nAsMax)
            {
                nAsMax = nCurrAs;
            }

            IMS_TRACE_D("CreateProfile() add payload(%d),codec(%d), SamplingRate(%d)", i,
                    pEvsConfig->GetCodec(), 16000);
        }
        else if (pCodecConfig->GetCodec() == ImsCodec::AUDIO_PCMU ||
                pCodecConfig->GetCodec() == ImsCodec::AUDIO_PCMA)
        {
            AudioProfile::Payload* pPCM_Payload = new AudioProfile::Payload();
            // add pcmu/pcma payload
            if (pCodecConfig->GetCodec() == ImsCodec::AUDIO_PCMU)
            {
                pPCM_Payload->SetRtpMap(0, "PCMU", 8000, 0);
            }
            else
            {
                pPCM_Payload->SetRtpMap(8, "PCMA", 8000, 0);
            }

            // br setting
            IMS_SINT32 nTempBandwidth = 0;
            if (pAudioProfile->objIpAddr.IsIPv6Address() == IMS_TRUE)
            {
                nTempBandwidth = 72;
            }
            else
            {
                nTempBandwidth = 64;
            }

            if (nTempBandwidth > nAsMax)
            {
                nAsMax = nTempBandwidth;
                nAsOptimal = nTempBandwidth;
            }

            pAudioProfile->lstPayload.Append(pPCM_Payload);

            IMS_TRACE_D(
                    "CreateProfile() add payload(%d),codec(%d)", i, pCodecConfig->GetCodec(), 0);
        }
    }

    // Setting direction
    pAudioProfile->eDirection = MEDIA_DIRECTION_SEND_RECEIVE;
    // Setting ptime & maxptime & bandwidth
    pAudioProfile->nPtime = pConfig->GetPtime();
    pAudioProfile->nMaxPtime = pConfig->GetMaxPtime();
    pAudioProfile->nBandwidthAs = pConfig->GetAsBandwidthKbps();

    SetRtcpRsRr(pAudioProfile, pConfig);
    IMS_TRACE_D("CreateProfile() - nPtime[%d], nMaxPtime[%d]", pAudioProfile->nPtime,
            pAudioProfile->nMaxPtime, 0);
    IMS_TRACE_D("CreateProfile() - AS[%d], RR[%d], RS[%d]", pAudioProfile->nBandwidthAs,
            pAudioProfile->nBandwidthRr, pAudioProfile->nBandwidthRs);
    IMS_TRACE_D("CreateProfile() - payloadSize[%d]", pAudioProfile->lstPayload.GetSize(), 0, 0);

    return pAudioProfile;
}

PUBLIC GLOBAL IMS_BOOL AudioProfileUtil::SetRtcpRsRr(
        OUT AudioProfile* pAudioProfile, IN AudioConfiguration* pConfig)
{
    if (pAudioProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pAudioProfile->nBandwidthRr = pConfig->GetRrBandwidthBps();
    pAudioProfile->nBandwidthRs = pConfig->GetRsBandwidthBps();

    IMS_TRACE_I("SetRtcpRsRr(), Set RS[%d], RR[%d]", pAudioProfile->nBandwidthRs,
            pAudioProfile->nBandwidthRr, 0);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL AudioProfileUtil::MakeNegotiatedBandwidth(IN AudioConfiguration* pConfig,
        IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        IN IMS_SINT32 nAsValueOfNegoticatedCodec, OUT AudioProfile* pNegotiatedProfile)
{
    IMS_TRACE_D("MakeNegotiatedBandwidth() - BW_NEGO_OPTION_VALUE[%d]",
            pConfig->GetBandwidthNegoOption(), 0, 0);

    if (bIsOfferReceived == IMS_FALSE)
    {
        // MO's Bandwidth Setting
        //  1. Set AS Value
        if (pPeerProfile->nBandwidthAs > 0)
        {
            if (pPeerProfile->nBandwidthAs > nAsValueOfNegoticatedCodec)
            {
                pNegotiatedProfile->nBandwidthAs = nAsValueOfNegoticatedCodec;
            }
            else
            {
                pNegotiatedProfile->nBandwidthAs = pPeerProfile->nBandwidthAs;
            }
        }
        else
        {  // Exception Handling (b= AS line is not included in Answer SDP)
            pNegotiatedProfile->nBandwidthAs = pLocalProfile->nBandwidthAs;
        }

        // 2. Set RS/RR Value
        // 2.1 Exception Handling (b=RS/RR line is not included in Answer SDP)
        if (pNegotiatedProfile->nBandwidthRs < 0 || pNegotiatedProfile->nBandwidthRr < 0)
        {
            pNegotiatedProfile->nBandwidthRs = pLocalProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pLocalProfile->nBandwidthRr;

            IMS_TRACE_D("MakeNegotiatedBandwidth() - Nego AS[%d] RS[%d] RR[%d]",
                    pLocalProfile->nBandwidthAs, pLocalProfile->nBandwidthRs,
                    pLocalProfile->nBandwidthRr);
            return IMS_TRUE;
        }

        // 2.2 Normal Case
        if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE)
        {
            // if RS/RR is used for RTCP Nego value
            pNegotiatedProfile->nBandwidthRs = pPeerProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pPeerProfile->nBandwidthRr;
        }
        else
        {
            // default case (RS/RR is not negotiated value)
            pNegotiatedProfile->nBandwidthRs = pLocalProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pLocalProfile->nBandwidthRr;
        }
    }
    else
    {
        // MT's Bandwidth Setting
        // 1. Set Negotiated AS Value
        if (nAsValueOfNegoticatedCodec > 0)
        {
            pNegotiatedProfile->nBandwidthAs = nAsValueOfNegoticatedCodec;

            // if GetBandwidthNegoOption is BW_OPTION_NEGOTIATED_VALUE, use lower AS value
            if ((pConfig->GetBandwidthNegoOption() ==
                        MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE) &&
                    (nAsValueOfNegoticatedCodec > pPeerProfile->nBandwidthAs) &&
                    (pPeerProfile->nBandwidthAs > 0))
            {
                pNegotiatedProfile->nBandwidthAs = pPeerProfile->nBandwidthAs;
            }
        }
        else
        {
            pNegotiatedProfile->nBandwidthAs = pLocalProfile->nBandwidthAs;
        }
        // 3. Set RS/RR Value
        if (pPeerProfile->eDirection != MEDIA_DIRECTION_SEND_RECEIVE)
        {  // Hold Case
            // 3.1 Hold Case
            SetRtcpRsRr(pNegotiatedProfile, pConfig);
        }
        else
        {
            // 3.2 Active Call Case
            // 3.2.1 Exception Handling (b=RS/RR line is not included in Answer SDP)
            if (pNegotiatedProfile->nBandwidthRs < 0 || pNegotiatedProfile->nBandwidthRr < 0)
            {
                pNegotiatedProfile->nBandwidthRs = pLocalProfile->nBandwidthRs;
                pNegotiatedProfile->nBandwidthRr = pLocalProfile->nBandwidthRr;

                IMS_TRACE_D("MakeNegotiatedBandwidth() - AS[%d] RS[%d] RR[%d]",
                        pLocalProfile->nBandwidthAs, pLocalProfile->nBandwidthRs,
                        pLocalProfile->nBandwidthRr);
                return IMS_TRUE;
            }

            // 3.2.2 Normal Case
            if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE)
            {
                // if RS/RR is used for RTCP Nego value
                IMS_TRACE_D("MakeNegotiatedBandwidth() - use peer RS[%d] RR[%d]",
                        pPeerProfile->nBandwidthRs, pPeerProfile->nBandwidthRr, 0);
                pNegotiatedProfile->nBandwidthRs = pPeerProfile->nBandwidthRs;
                pNegotiatedProfile->nBandwidthRr = pPeerProfile->nBandwidthRr;
            }
            else
            {
                // default case (RS/RR is not negotiated value)
                pNegotiatedProfile->nBandwidthRs = pLocalProfile->nBandwidthRs;
                pNegotiatedProfile->nBandwidthRr = pLocalProfile->nBandwidthRr;
            }
        }
    }

    IMS_TRACE_D("MakeNegotiatedBandwidth() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
            pNegotiatedProfile->nBandwidthAs, pNegotiatedProfile->nBandwidthRs,
            pLocalProfile->nBandwidthRr);
    return IMS_TRUE;
}

PUBLIC GLOBAL const IMS_SINT32* AudioProfileUtil::GetAmrAsArray(
        IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet, IN IMS_BOOL bIpV6)
{
    const IMS_SINT32* pArrAmr;
    IMS_SINT32 AsArrIndex = 0;
    if (eCodec == AUDIO_CODEC_AMRWB)
    {
        AsArrIndex += 4;
    }
    if (bIpV6 == IMS_TRUE)
    {
        AsArrIndex += 2;
    }
    if (nOctet == 1)
    {
        AsArrIndex += 1;
    }
    pArrAmr = AMR_AS[AsArrIndex];
    return pArrAmr;
}

PUBLIC GLOBAL const IMS_SINT32* AudioProfileUtil::GetEvsAsArray(
        IN IMS_SINT32 nEVSFormat, IN IMS_BOOL bIpV6)
{
    const IMS_SINT32* pArrEvs;
    IMS_SINT32 AsArrIndex = 0;
    if (nEVSFormat == 1)
    {
        AsArrIndex += 2;  // AMR IO mode == 1
    }
    if (bIpV6 == IMS_TRUE)
    {
        AsArrIndex += 1;
    }
    pArrEvs = EVS_AS[AsArrIndex];
    return pArrEvs;
}

PUBLIC GLOBAL IMS_SINT32 AudioProfileUtil::ConvertToBandwidthAS(IN IMS_SINT32 eCodec,
        IN IMS_SINT32 nOctet, IN IMS_BOOL bIpV6, IN IMS_SINT32 nModeSet,
        IN IMS_BOOL bGetMaxValue /*= IMS_FALSE*/)
{
    IMS_TRACE_D("ConvertToBandwidthAS() - Entered. eCodec[%d] nOctet[%d] nModeSet[%d]", eCodec,
            nOctet, nModeSet);

    IMS_SINT32 nResultAs = -1;

    const IMS_SINT32* pArrAmr;
    IMS_SINT32 nModeCount;

    pArrAmr = GetAmrAsArray(eCodec, nOctet, bIpV6);
    // Bandwidth for bandwidth-efficient mode
    // Bandwidth for octet-align mode

    if (eCodec == AUDIO_CODEC_AMRWB)
    {
        nModeCount = 9;
    }
    else
    {
        nModeCount = 8;
    }

    if (bGetMaxValue || (nModeSet >= nModeCount))
    {
        // Bandwidth for bandwidth-efficient mode
        // Bandwidth for octet-align mode
        nModeSet = nModeCount - 1;
    }

    nResultAs = pArrAmr[nModeSet];

    IMS_TRACE_D("ConvertToBandwidthAS() - Ended : IPv6[%d], nAs[%d]", bIpV6, nResultAs, 0);
    return nResultAs;
}

PUBLIC GLOBAL IMS_SINT32 AudioProfileUtil::ConvertToBandwidthAS(IN IMS_SINT32 eCodec,
        IN IMS_BOOL bIpV6, IN IMS_SINT32 nCodecFormat, IN IMS_SINT32 nCodecMode,
        IN IMS_BOOL bGetMaxValue)
{
    IMS_TRACE_D("ConvertToBandwidthAS() - Entered. eCodec[%d] bIpV6[%d] nCodecFormat[%d]", eCodec,
            bIpV6, nCodecFormat);

    IMS_SINT32 nResultAs = -1;

    const IMS_SINT32* pArrEvs;
    pArrEvs = GetEvsAsArray(nCodecFormat, bIpV6);

    IMS_SINT32 nModeCount = 0;

    if (nCodecFormat == 1)
    {
        nModeCount = 9;  // AMR IO Mode
    }
    else
    {
        nModeCount = 12;
    }

    if (bGetMaxValue || (nCodecMode >= nModeCount))
    {
        nCodecMode = nModeCount - 1;
    }

    nResultAs = pArrEvs[nCodecMode];
    IMS_TRACE_D("ConvertToBandwidthAS() - Ended : nAs[%d]", nResultAs, 0, 0);

    return nResultAs;
}

PUBLIC GLOBAL IMS_SINT32 AudioProfileUtil::ConvertToModeSet(
        IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet, IN IMS_BOOL bIpV6, IN IMS_SINT32 nAs)
{
    IMS_TRACE_D("ConvertToModeSet() eCodec[%d], nOctet[%d], nAs[%d]", eCodec, nOctet, nAs);

    IMS_SINT32 nModeSet = -1;
    const IMS_SINT32* pArrAmr;
    IMS_SINT32 nModeCount;
    pArrAmr = GetAmrAsArray(eCodec, nOctet, bIpV6);

    if (pArrAmr == IMS_NULL)
    {
        return -1;
    }

    if (eCodec == AUDIO_CODEC_AMRWB)
    {
        nModeCount = 9;
    }
    else
    {
        nModeCount = 8;
    }

    for (IMS_SINT32 i = 0; i < nModeCount; ++i)
    {
        if (nAs >= pArrAmr[i])
        {
            nModeSet = i;
        }
        else
        {
            break;
        }
    }

    IMS_TRACE_D("ConvertToModeSet() - Result : bIpV6[%d]m nModeSet[%d]", bIpV6, nModeSet, 0);
    return nModeSet;
}

/*!
 *  @brief      UpdateAudioProfileBandwidth
 *  @details    UpdateAudioProfileBandwidth for update bandwidth(AS) at audio profile
 */
PUBLIC GLOBAL IMS_BOOL AudioProfileUtil::UpdateAudioProfileBandwidth(
        OUT AudioProfile* pAudioProfile, IN AudioConfiguration* pConfig)
{
    if ((pAudioProfile == IMS_NULL) || (pConfig == IMS_NULL))
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nAsOptimal = -1;
    IMS_SINT32 nAsMax = -1;
    IMS_SINT32 nCurrAs = 0;
    AUDIO_CODEC nCurrCodec = AUDIO_CODEC_NONE;

    for (IMS_UINT32 i = 0; i < pAudioProfile->lstPayload.GetSize(); i++)
    {
        AudioProfile::Payload* pAudioPayload = pAudioProfile->lstPayload.GetAt(i);
        if (pAudioPayload == IMS_NULL)
        {
            continue;
        }

        nCurrAs = 0;
        if ((pAudioPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB") == IMS_TRUE) ||
                (pAudioPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("AMR") == IMS_TRUE))
        {
            AudioProfile::AmrFmtp* pAmrFmtp =
                    reinterpret_cast<AudioProfile::AmrFmtp*>(pAudioPayload->pFmtp);
            if (pAmrFmtp == IMS_NULL)
            {
                continue;
            }

            if (pAudioPayload->objRtpMap.nSamplingRate == 16000)
            {
                nCurrCodec = AUDIO_CODEC_AMRWB;
            }
            else
            {
                nCurrCodec = AUDIO_CODEC_AMR;
            }

            // find max modeset
            IMS_SINT32 nMaxModeset = 0;
            if (pAmrFmtp->nModeSetList == 0)
            {
                if (nCurrCodec == AUDIO_CODEC_AMRWB)
                {
                    nMaxModeset = 8;  // amr wb case
                }
                else
                {
                    nMaxModeset = 7;  // amr nb case
                }
            }
            else
            {
                for (nMaxModeset = 8; nMaxModeset >= 0; nMaxModeset--)
                {
                    if (pAmrFmtp->nModeSetList & (1 << nMaxModeset))
                    {
                        break;
                    }
                }
            }

            nCurrAs = ConvertToBandwidthAS(nCurrCodec, pAmrFmtp->nOctetAlign,
                    pAudioProfile->objIpAddr.IsIPv6Address(), nMaxModeset);
            if (nCurrAs > nAsOptimal)
            {
                nAsOptimal = nCurrAs;
            }

            nCurrAs = ConvertToBandwidthAS(nCurrCodec, pAmrFmtp->nOctetAlign,
                    pAudioProfile->objIpAddr.IsIPv6Address(), nMaxModeset, IMS_TRUE);
            if (nCurrAs > nAsMax)
            {
                nAsMax = nCurrAs;
            }
        }
        else if (pAudioPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS") == IMS_TRUE)
        {
            nCurrCodec = AUDIO_CODEC_EVS;
            AudioProfile::EvsFmtp* pEvsFmtp =
                    reinterpret_cast<AudioProfile::EvsFmtp*>(pAudioPayload->pFmtp);
            if (pEvsFmtp == IMS_NULL)
            {
                continue;
            }

            // find max br
            IMS_SINT32 nMaxBr = 0;
            {
                if (pEvsFmtp->nBrList == 0)
                {
                    nMaxBr = 6;
                }
                else
                {
                    for (nMaxBr = 6; nMaxBr >= 0; nMaxBr--)
                    {
                        if (pEvsFmtp->nBrList & (1 << nMaxBr))
                        {
                            break;
                        }
                    }
                }
            }

            nCurrAs = ConvertToBandwidthAS(nCurrCodec, pAudioProfile->objIpAddr.IsIPv6Address(), 0,
                    nMaxBr);  // primary mode
            if (nCurrAs > nAsOptimal)
            {
                nAsOptimal = nCurrAs;
            }

            nCurrAs = ConvertToBandwidthAS(
                    nCurrCodec, pAudioProfile->objIpAddr.IsIPv6Address(), 0, nMaxBr, IMS_TRUE);
            if (nCurrAs > nAsMax)
            {
                nAsMax = nCurrAs;
            }
        }
        else if ((pAudioPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMU") == IMS_TRUE) ||
                (pAudioPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("PCMA") == IMS_TRUE))
        {
            if (72 > nAsMax)  // 72 is PCMU/PCMA AS value at IPv6
            {
                nAsMax = 72;
                nAsOptimal = 72;
            }
        }
    }

    pAudioProfile->nBandwidthAs = pConfig->GetAsBandwidthKbps();

    SetRtcpRsRr(pAudioProfile, pConfig);
    IMS_TRACE_D("UpdateAudioProfileBandwidth() update AS[%d]", nAsMax, 0, 0);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_SINT32 AudioProfileUtil::GetLargestModesetInFmtp(
        IN AString strCodec, IN AudioProfile::Payload* pPayload)
{
    const IMS_SINT32 NO_MODESET = -1;
    const IMS_SINT32 AMR_MAX_MODESET = 7;
    const IMS_SINT32 AMRWB_MAX_MODESET = 8;
    const IMS_SINT32 EVS_PRIMARY_MODE_MAX_MODESET = 11;
    const IMS_SINT32 EVS_IO_MODE_MAX_MODESET = 8;

    if (pPayload == IMS_NULL)
    {
        return NO_MODESET;
    }
    if (!strCodec.Equals("AMR") && !strCodec.Equals("AMR-WB") && !strCodec.Equals("EVS"))
    {
        return NO_MODESET;
    }

    if (strCodec.Equals("AMR") || strCodec.Equals("AMR-WB"))
    {
        AudioProfile::AmrFmtp* pAmrFmtp = reinterpret_cast<AudioProfile::AmrFmtp*>(pPayload->pFmtp);
        if (pAmrFmtp == NULL)
        {
            return NO_MODESET;
        }

        if (pAmrFmtp->nModeSetList == 0)
        {
            if (strCodec.Equals("AMR"))
            {
                return AMR_MAX_MODESET;
            }
            else
            {
                return AMRWB_MAX_MODESET;
            }
        }
        else
        {
            IMS_SINT32 nModeSet;
            for (nModeSet = 8; nModeSet >= 0; nModeSet--)
            {
                IMS_UINT32 nMatch = pAmrFmtp->nModeSetList & (1 << nModeSet);
                if (nMatch)
                {
                    return nModeSet;
                }
            }

            return nModeSet;
        }
    }
    else if (strCodec.Equals("EVS"))
    {
        AudioProfile::EvsFmtp* pEvsFmtp = reinterpret_cast<AudioProfile::EvsFmtp*>(pPayload->pFmtp);
        if (pEvsFmtp == NULL)
        {
            return NO_MODESET;
        }

        // Primary mode
        if (pEvsFmtp->nEvsModeSwitch != 1)
        {
            // check bitrate...
            if (pEvsFmtp->nBrList == 0)
            {
                return EVS_PRIMARY_MODE_MAX_MODESET;
            }
            else
            {
                IMS_SINT32 nBitrate = 0;
                for (nBitrate = 11; nBitrate >= 0; nBitrate--)
                {
                    IMS_UINT32 nMatch = pEvsFmtp->nBrList & (1 << nBitrate);
                    if (nMatch)
                    {
                        return nBitrate;
                    }
                }
            }
        }
        else  // AMR IO mode
        {
            if (pEvsFmtp->nModeSetList == 0)
            {
                return EVS_IO_MODE_MAX_MODESET;
            }
            else
            {
                IMS_SINT32 nModeSet = 0;
                for (nModeSet = 8; nModeSet >= 0; nModeSet--)
                {
                    IMS_UINT32 nMatch = pEvsFmtp->nModeSetList & (1 << nModeSet);
                    if (nMatch)
                    {
                        return nModeSet;
                    }
                }
            }
        }
    }

    return NO_MODESET;
}

PUBLIC GLOBAL IMS_SINT32 AudioProfileUtil::GetModesetList(
        IN AString strCodec, IN AudioProfile::Payload* pPayload)
{
    const IMS_SINT32 NO_MODESET = 0;
    const IMS_SINT32 EVS_PRIMARY_MODE_MAX_MODESET = 11;

    if (pPayload == IMS_NULL)
    {
        return NO_MODESET;
    }
    if (!strCodec.Equals("AMR") && !strCodec.Equals("AMR-WB") && !strCodec.Equals("EVS"))
    {
        return NO_MODESET;
    }

    if (strCodec.Equals("AMR") || strCodec.Equals("AMR-WB"))
    {
        AudioProfile::AmrFmtp* pAmrFmtp = reinterpret_cast<AudioProfile::AmrFmtp*>(pPayload->pFmtp);
        if (pAmrFmtp == NULL)
        {
            return NO_MODESET;
        }

        if (pAmrFmtp->nModeSetList > 0)
        {
            IMS_TRACE_D("GetModesetList is %d", pAmrFmtp->nModeSetList, 0, 0);
            return pAmrFmtp->nModeSetList;
        }
        else
        {
            IMS_TRACE_D("GetModesetList is %d", pAmrFmtp->nDefaultRtpModeSet, 0, 0);
            return pAmrFmtp->nDefaultRtpModeSet;
        }
    }
    else if (strCodec.Equals("EVS"))
    {
        AudioProfile::EvsFmtp* pEvsFmtp = reinterpret_cast<AudioProfile::EvsFmtp*>(pPayload->pFmtp);
        if (pEvsFmtp == NULL)
        {
            return NO_MODESET;
        }

        // Primary mode
        if (pEvsFmtp->nEvsModeSwitch != 1)
        {
            if (pEvsFmtp->nBrList == 0)
            {
                IMS_TRACE_D(
                        "nBrList is 0 -> GetModesetList is %d", EVS_PRIMARY_MODE_MAX_MODESET, 0, 0);
                return EVS_PRIMARY_MODE_MAX_MODESET;
            }
            else
            {
                IMS_TRACE_D("GetModesetList is %d", pEvsFmtp->nBrList, 0, 0);
                return pEvsFmtp->nBrList;
            }
        }
        else  // AMR IO mode
        {
            if (pEvsFmtp->nModeSetList > 0)
            {
                IMS_TRACE_D("GetModesetList is %d", pEvsFmtp->nModeSetList, 0, 0);
                return pEvsFmtp->nModeSetList;
            }
            else
            {
                IMS_TRACE_D("GetModesetList is %d", pEvsFmtp->nDefaultRtpModeSet, 0, 0);
                return pEvsFmtp->nDefaultRtpModeSet;
            }
        }
    }

    return NO_MODESET;
}