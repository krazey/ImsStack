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

// == INCLUDES =============================================================

#include "ServiceTrace.h"
#include "config/CodecAMRConfig.h"
#include "config/CodecAMRConfigEx.h"
#include "config/CodecTelephoneEventConfig.h"
#include "config/CodecTelephoneEventConfigEx.h"
#include "config/CodecEVSConfig.h"
#include "config/CodecPCMConfig.h"
#include "audio/AudioDef.h"
#include "MediaEnvironment.h"
#include "config/MediaSessionConfigFactory.h"
#include "IService.h"
#include "audio/AudioProfileConfigurer.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "MediaManager.h"

// == DEFINES =========================================================
__IMS_TRACE_TAG_USER_DECL__("MED.PC");

const IMS_SINT32 AudioProfileConfigurer::AMR_AS[8][9] = {
    { 22, 22, 23, 24, 24, 25, 27, 29, 0 },      // nb, ipv4, be
    { 22, 22, 23, 24, 25, 25, 28, 30, 0 },      // nb, ipv4, oa
    { 30, 30, 31, 32, 32, 33, 35, 37, 0 },      // nb, ipv6, be
    { 30, 30, 31, 32, 33, 33, 36, 38, 0 },      // nb, ipv6, oa
    { 24, 26, 30, 31, 33, 35, 37, 40, 41 },     // wb, ipv4, be
    { 24, 26, 30, 32, 33, 36, 37, 40, 41 },     // wb, ipv4, oa
    { 32, 34, 38, 39, 41, 43, 45, 48, 49 },     // wb, ipv6, be
    { 32, 34, 38, 40, 41, 44, 45, 48, 49 }      // wb, ipv6, oa
};

const IMS_SINT32 AudioProfileConfigurer::EVS_AS[4][12] = {
    { 23, 24, 25, 27, 30, 34, 42, 49, 65, 81, 113, 145 },    // Primary, ipv4
    { 31, 32, 33, 35, 38, 42, 50, 57, 73, 89, 121, 153 },    // Primary, ipv6
    { 23, 26, 29, 31, 32, 35, 36, 40, 40, 0, 0, 0 },        // AMR IO, ipv4
    { 31, 34, 37, 39, 40, 43, 44, 48, 48, 0, 0, 0 }         // AMR IO, ipv6
};

#define IMSMEDIA_AUDIO_HALFRATE_LTE_RSRP_THRESHOLD (-94)

PUBLIC GLOBAL
IMS_BOOL AudioProfileConfigurer::CreateAudioProfile(OUT AudioProfile* pAudioProfile,
        IN MediaEnvironment* pEnvironment, IN AudioConfiguration* pConfig, IN IMS_SINT32 nSlotId)
{
    IMS_SINT32 nAsOptimal = -1;
    IMS_SINT32 nAsMax = -1;

    if (pEnvironment == IMS_NULL || pConfig == IMS_NULL ||
        pAudioProfile == IMS_NULL || pEnvironment->pIService == IMS_NULL)
    {
        return IMS_FALSE;
    }
    IMS_TRACE_D("CreateAudioProfile() Entered.", 0, 0, 0);

    MediaManager* pMediaManager = MediaManager::GetInstance(nSlotId);

    if (pMediaManager == IMS_NULL)
    {
        return IMS_FALSE;
    }

    MediaResourceMngr* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Setting IP address
    // In IPv6 system, The MediaLogic have to write a modem IPv6 to "c line" of session level.
    // In Ipv4, this is not necessary.
    if (pEnvironment->eNetworkType == MEDIA_NETWORK_WIFI)
    {
        pAudioProfile->objIpAddr = pEnvironment->pIService->GetIPAddress();
    }
    else
    {
        IMS_SINT32 nPDNType = MediaResourceMngr::PDN_IMS;

        if (pEnvironment->eServiceType == MEDIA_SERVICE_EMERGENCY)
        {
            nPDNType = MediaResourceMngr::PDN_EMERGENCY;
        }

        pResourceMngr->UpdatePdnResource(nPDNType,
                pEnvironment->pIService->GetIPAddress().IsIPv6Address());
        // Android IP
        pAudioProfile->objIpAddr = pEnvironment->pIService->GetIPAddress();
    }

    // Setting RTP/RTCP port from resource manager
    if (pAudioProfile->nDataPort > 0)
    {
        pAudioProfile->nDataPort = pResourceMngr->AcquireRtpPort(
                pAudioProfile->nDataPort, pAudioProfile->nDataPort);
        pAudioProfile->nControlPort = pAudioProfile->nDataPort+1;
    }
    else
    {
        pAudioProfile->nDataPort = pResourceMngr->AcquireRtpPort(pConfig);
        pAudioProfile->nControlPort = pAudioProfile->nDataPort+1;
    }

    IMS_TRACE_D("CreateAudioProfile() objIpAddr[%s], port[%d]",
            pAudioProfile->objIpAddr.ToCharString(), pAudioProfile->nDataPort, 0);

    // Setting profile type
    if ((pConfig->GetSrtp() == 1) && (pConfig->GetSrtpSupportCapaNego() == 0))
    {
        pAudioProfile->strTransportType = "RTP/SAVP";
    }
    else
    {
        pAudioProfile->strTransportType = "RTP/AVP";
    }
    while (pAudioProfile->lstPayload.GetSize() > 0)
    {
        AudioProfile::Payload* pPayload = pAudioProfile->lstPayload.GetAt(0);
        if (pPayload != IMS_NULL)
        {
            delete pPayload;
        }
        pAudioProfile->lstPayload.RemoveAt(0);
    }

    IMS_BOOL bRtcpXr = pConfig->GetRtcpXr();

    // RTCP-XR
    if (bRtcpXr == IMS_TRUE)
    {
        pAudioProfile->bSupportRtcpXr = IMS_TRUE;
        if (pConfig->GetRtcpXrVoip() == IMS_TRUE)
        {
            pAudioProfile->objRtcpXrAttr.bSupportVoipMatircs = IMS_TRUE;
        }
        if (pConfig->GetRtcpXrStatistics() == IMS_TRUE)
        {
            pAudioProfile->objRtcpXrAttr.bSupportStatisticMetrics = IMS_TRUE;
        }
        if (pConfig->GetRtcpXrPlr() == IMS_TRUE)
        {
            pAudioProfile->objRtcpXrAttr.bSupportPacketLossRle = IMS_TRUE;
        }
        if (pConfig->GetRtcpXrPdr() == IMS_TRUE)
        {
            pAudioProfile->objRtcpXrAttr.bSupportPacketDuplicatedRle = IMS_TRUE;
        }
        IMS_TRACE_D("CreateAudioProfile() Add RTCP-XR. VoIP[%d], Stat[%d], PLR[%d]",
                pAudioProfile->objRtcpXrAttr.bSupportVoipMatircs,
                pAudioProfile->objRtcpXrAttr.bSupportStatisticMetrics,
                pAudioProfile->objRtcpXrAttr.bSupportPacketLossRle);
        IMS_TRACE_D("CreateAudioProfile() Add RTCP-XR. PDR[%d]",
                pAudioProfile->objRtcpXrAttr.bSupportPacketDuplicatedRle, 0, 0);
    }

    // set RS/RR zero though Initial media direction is inactive
    pAudioProfile->bRtcpDisableBeforeSetup = pConfig->IsRtcpDisableBeforeSetup();
    // Setting each payload and bandwidth
    IMSList<CodecConfig*> pCodecs;
    pCodecs = pConfig->GetCodecConfigs();
    IMS_BOOL bAudioHalfRate = IMS_FALSE;

    if (pConfig->GetAudioHalfRateMode())
    {
        IMS_SINT32 nlteRSRP =
                PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher()->GetLteRsrpStrength();
        if (nlteRSRP < IMSMEDIA_AUDIO_HALFRATE_LTE_RSRP_THRESHOLD)
        {
            bAudioHalfRate = IMS_TRUE;
            IMS_TRACE_D("CreateAudioProfile() AudioHalfRate adjust[%d]", nlteRSRP, 0, 0);
        }
    }

    for (IMS_UINT32 i = 0; i < pCodecs.GetSize(); i++)
    {
        CodecConfig* pCodecConfig = pCodecs.GetAt(i);
        if (pCodecConfig == IMS_NULL)
        {
            break;
        }
        if (pCodecConfig->GetCodecName().EqualsIgnoreCase("None") == IMS_TRUE ||
                pCodecConfig->GetCodecName().IsEmpty() == IMS_TRUE)
        {
            IMS_TRACE_D("CreateAudioProfile() invalid codec name, skip config(%d) - %s",
                    i, pCodecConfig->GetCodecName().GetStr(), 0);
            continue;
        }
        if ((pEnvironment->eNetworkType & pCodecConfig->GetAvailableNetworkType()) == 0)
        {
            IMS_TRACE_D("CreateAudioProfile() skip config(%d) - %s",
                    i, pCodecConfig->GetCodecName().GetStr(), 0);
            continue;
        }
        if (pCodecConfig->GetPayloadType() == -1)
        {
            IMS_TRACE_D("CreateAudioProfile() invalid payload type, skip config(%d) - %s",
                    i, pCodecConfig->GetCodecName().GetStr(), 0);
            continue;
        }

        if (pCodecConfig->GetCodecName().EqualsIgnoreCase("AMR") == IMS_TRUE ||
                pCodecConfig->GetCodecName().EqualsIgnoreCase("AMR-WB") == IMS_TRUE)
        {
            CodecAMRConfigEx* pAmrConfig = (CodecAMRConfigEx*)pCodecConfig;
            AUDIO_CODEC nCurrCodec;
            AString strCodecName;
            AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp();
            pAmrFmtp->nModeSetList = pAmrConfig->GetModeSets();
            pAmrFmtp->nDefaultRtpModeSet = pAmrConfig->GetDefaultRtpModeSet();

            if (pAmrConfig->GetOctetAlign() != -1)
            {
                pAmrFmtp->nOctetAlign = pAmrConfig->GetOctetAlign();
                pAmrFmtp->bShow_OctetAlign = IMS_TRUE;
            }
            if (pAmrConfig->GetModeChangeCapability() != -1)
            {
                pAmrFmtp->nModeChangeCapability = pAmrConfig->GetModeChangeCapability();
                pAmrFmtp->bShowModeChangeCapability = IMS_TRUE;
            }
            if (pAmrConfig->GetModeChangePeriod() != -1)
            {
                pAmrFmtp->nModeChangePeriod = pAmrConfig->GetModeChangePeriod();
                pAmrFmtp->bShowModeChangePeriod = IMS_TRUE;
            }
            if (pAmrConfig->GetModeChangeNeighbor() != -1)
            {
                pAmrFmtp->nModeChangeNeighbor = pAmrConfig->GetModeChangeNeighbor();
                pAmrFmtp->bShowModeChangeNeighbor = IMS_TRUE;
            }
            if (pAmrConfig->GetMaxRed() != -1)
            {
                pAmrFmtp->nMaxRed = pAmrConfig->GetMaxRed();
                pAmrFmtp->bShowMaxRed = IMS_TRUE;
            }
            if (pAmrConfig->GetRobustSorting() != -1)
            {
                pAmrFmtp->nRobustSorting = pAmrConfig->GetRobustSorting();
                pAmrFmtp->bShow_RobustSorting = IMS_TRUE;
            }
            if (pAmrConfig->GetPtime() != -1)
            {
                pAmrFmtp->nPtime = pAmrConfig->GetPtime();
                pAmrFmtp->bShowPtime = IMS_TRUE;
            }
            if (pAmrConfig->GetMaxPtime() != -1)
            {
                pAmrFmtp->nMaxPtime = pAmrConfig->GetMaxPtime();
                pAmrFmtp->bShowMaxPtime = IMS_TRUE;
            }

            pAmrFmtp->bSCREnable = pAmrConfig->GetScr();

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
            pAmrPayload->pFmtp = (void*)pAmrFmtp;
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

            IMS_TRACE_D("CreateAudioProfile() add payload(%d) - %s, %d",
                    i, pAmrConfig->GetCodecName().GetStr(), pAmrConfig->GetSamplingRate());
        }
        else if (pCodecConfig->GetCodecName().EqualsIgnoreCase("telephone-event") == IMS_TRUE)
        {
            CodecTelephoneEventConfigEx* pDtmfConfig = (CodecTelephoneEventConfigEx*)pCodecConfig;
            AudioProfile::TelephoneEventFmtp* pTeFmtp =
                    new AudioProfile::TelephoneEventFmtp(pDtmfConfig->GetEvents());
            AudioProfile::Payload* pTelephoneEvent = new AudioProfile::Payload();
            pTelephoneEvent->SetRtpMap(pDtmfConfig->GetPayloadType(), pDtmfConfig->GetCodecName(),
                    pDtmfConfig->GetSamplingRate(), 0);
            pTelephoneEvent->pFmtp = (void*)pTeFmtp;
            pAudioProfile->lstPayload.Append(pTelephoneEvent);
            IMS_TRACE_D("CreateAudioProfile() add payload(%d) - %s, %d",
                    i, pDtmfConfig->GetCodecName().GetStr(), pDtmfConfig->GetSamplingRate());
        }
        else if (pCodecConfig->GetCodecName().EqualsIgnoreCase("EVS") == IMS_TRUE)
        {
            // make EVS Profile
            CodecEVSConfig* pEvsConfig = (CodecEVSConfig*)pCodecConfig;
            AUDIO_CODEC nCurrCodec = AUDIO_CODEC_EVS;
            AString strCodecName;
            strCodecName.Sprintf("%s", "EVS");

            AudioProfile::EvsFmtp* pEvsFmtp = new AudioProfile::EvsFmtp();

            // Mode set list
            pEvsFmtp->nModeSetList = pEvsConfig->GetModeSetList();
            pEvsFmtp->nBrList = pEvsConfig->GetBrList();
            pEvsFmtp->nBwList = pEvsConfig->GetBwList();

            // get default Bandwidth & default bitrate
            pEvsFmtp->nDefaultBandwidthList = pEvsConfig->GetDefaultBandwidth();
            pEvsFmtp->nDefaultBitrateList = pEvsConfig->GetDefaultBitrate();
            pEvsFmtp->nDefaultRtpModeSet = pEvsConfig->GetDefaultRptModeSet();

            if (PhoneInfoService::GetPhoneInfoService()->
                    GetNetworkWatcher(nSlotId)->GetRoamingState() == IMS_TRUE)
            {
                // In case of roaming use up to 13.2kbps as default bitrate
                pEvsFmtp->nDefaultBitrateList = 0x1f;
            }

            if (pEvsConfig->GetPtime() != -1)
            {
                pEvsFmtp->nPtime = pEvsConfig->GetPtime();
                pEvsFmtp->bShowPtime = IMS_TRUE;
            }
            if (pEvsConfig->GetMaxPtime() != -1)
            {
                pEvsFmtp->nMaxPtime = pEvsConfig->GetMaxPtime();
                pEvsFmtp->bShowMaxPtime = IMS_TRUE;
            }
            if (pEvsConfig->GetDtx() != -1)
            {
                pEvsFmtp->nDtx = pEvsConfig->GetDtx();
                pEvsFmtp->bShowDtx = IMS_TRUE;
            }
            if (pEvsConfig->GetHfOnly() != -1)
            {
                pEvsFmtp->nHfOnly = pEvsConfig->GetHfOnly();
                pEvsFmtp->bShowHfOnly = IMS_TRUE;
            }
            if (pEvsConfig->GetEvsModeSwitch() != -1)
            {
                pEvsFmtp->nEvsModeSwitch = pEvsConfig->GetEvsModeSwitch();
                pEvsFmtp->bShowEvsModeSwitch = IMS_TRUE;
            }
            if (pEvsConfig->GetMaxRed() != -1)
            {
                pEvsFmtp->nMaxRed = pEvsConfig->GetMaxRed();
                pEvsFmtp->bShowMaxRed = IMS_TRUE;
            }
            if (pEvsConfig->GetCmr() != -1)
            {
                pEvsFmtp->nCmr = pEvsConfig->GetCmr();
                pEvsFmtp->bShowCmr = IMS_TRUE;
                if (pEvsConfig->GetCmr() == 99)
                {
                    pEvsFmtp->nCmr = -1;
                }
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

            pEvsFmtp->bSendCmr = pEvsConfig->GetSendCmr();

            // set EVS codec fmtp
            AudioProfile::Payload* pEvsPayload = new AudioProfile::Payload();
            pEvsPayload->SetRtpMap(pEvsConfig->GetPayloadType(), strCodecName, 16000,
                    pEvsConfig->GetChannel());
            pEvsPayload->pFmtp = (void*)pEvsFmtp;

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
                    nTempCodecMode, pEvsConfig->GetBr());    // primary mode
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

            IMS_TRACE_D("CreateAudioProfile() add payload(%d) - %s, %d",
                    i, pEvsConfig->GetCodecName().GetStr(),  16000);

        }
        else if (pCodecConfig->GetCodecName().EqualsIgnoreCase("PCMU") ||
                pCodecConfig->GetCodecName().EqualsIgnoreCase("PCMA"))
        {
            AudioProfile::Payload* pPCM_Payload = new AudioProfile::Payload();

            // add pcmu/pcma payload
            if (pCodecConfig->GetCodecName().EqualsIgnoreCase("PCMU") == IMS_TRUE)
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
            IMS_TRACE_D("CreateAudioProfile() add payload(%d) - %s",
                    i, pCodecConfig->GetCodecName().GetStr(), 0);
        }
    }

    // Step 5. Setting direction
    pAudioProfile->eDirection = MEDIA_DIRECTION_SEND_RECEIVE;
    // Step 6. Setting ptime & maxptime & bandwidth
    pAudioProfile->nPtime = pConfig->GetPacketTime();
    pAudioProfile->nMaxPtime = pConfig->GetMaxPacketTime();
    pAudioProfile->nCandidatePriority = pConfig->GetCandidatePriority();

    switch (pConfig->GetBandwidthMode(MediaConfiguration::BW_AS))
    {
        case MediaConfiguration::BW_MODE_HIDE:
            pAudioProfile->nBandwidthAs = -1;
            break;
        case MediaConfiguration::BW_MODE_OPTIMAL:
            pAudioProfile->nBandwidthAs = nAsOptimal;
            break;
        case MediaConfiguration::BW_MODE_MAX:
            pAudioProfile->nBandwidthAs = nAsMax;
            break;
        case MediaConfiguration::BW_MODE_MANUAL:
            pAudioProfile->nBandwidthAs = pConfig->GetBandwidth(MediaConfiguration::BW_AS,
                    pAudioProfile->objIpAddr.IsIPv6Address());
            break;
        default:
            IMS_TRACE_E(0, "CreateAudioProfile - invalide bw as mode %d",
                    pConfig->GetBandwidthMode(MediaConfiguration::BW_AS), 0, 0);
            break;
    }

    SetAudioRsRr(pAudioProfile, pConfig, MEDIA_DIRECTION_SEND_RECEIVE);

    IMS_SINT32 nTCap = 0;
    IMS_SINT32 nAcap = 0;
    if (pConfig->GetSrtp() == IMS_TRUE)
    {
        AString strTcap = "";
        AString strAcap = "";
        AString strPcfg = "";
        IMS_SINT32 nCrypto = 0;

        //IMS_SINT32 nSrtpTcapNumStart = nTCap+1;
        IMS_SINT32 nSrtpAcapNumStart = nAcap+1;

        // Set Attribute Capa to Audio Profile
        pAudioProfile->bSupportSrtp = IMS_TRUE;
        pAudioProfile->nMasterKeyLifeTime = pConfig->GetSrtpMasterKeyLifeTime();
        pAudioProfile->eSrtpCryptoType= pConfig->GetSrtpCryptoType();

        IMS_TRACE_D("CreateAudioProfile :: GetSrtp[%d] GetSrtpMasterKeyLifeTime[%d] \
                GetSrtpProfile[%d]",pConfig->GetSrtp(), pConfig->GetSrtpMasterKeyLifeTime(),
                pConfig->GetSrtpCryptoType());

        // Generate Master Key for SRTP
        IMS_UINT8 nTempBuffer[MEDIA_MAX_KEY_LEN];
        AString strMasterKey = "";

        //Support one crypto attribute
        nAcap++;
        nCrypto++;
        IMS_MEM_Memset(pAudioProfile->szKey, 0, sizeof(pAudioProfile->szKey));
        switch (pAudioProfile->eSrtpCryptoType)
        {
            case MMPF_SRTP_CRYPTO_TYPE_AES128_CM_SHA1_80:
                //Generate Master Key
                for (int i = 0; i < 16; i++)
                {
                    nTempBuffer[i] = IMS_SYS_GetRandom(255);
                }

                //Generate Master Salt Key
                for (int i = 16; i < 30; i++)
                {
                    nTempBuffer[i] = IMS_SYS_GetRandom(255);
                }

                IMS_MEM_Memcpy(pAudioProfile->szKey,nTempBuffer,30);
                strMasterKey = AString((IMS_CHAR*)pAudioProfile->szKey,30).ToBase64();

                strAcap.Sprintf("crypto:%d %s inline:%s|2^%d|1:4",nCrypto,"AES_CM_128_HMAC_SHA1_80",
                        strMasterKey.GetStr(), pAudioProfile->nMasterKeyLifeTime);
                break;
            case MMPF_SRTP_CRYPTO_TYPE_AES128_CM_SHA1_32:
                //Generate Master Key
                for (int i = 0; i < 16; i++)
                {
                    nTempBuffer[i] = IMS_SYS_GetSRandom(255);
                }

                //Generate Master Salt Key
                for (int i = 16; i < 30; i++)
                {
                    nTempBuffer[i] = IMS_SYS_GetRandom(255);
                }

                IMS_MEM_Memcpy(pAudioProfile->szKey, nTempBuffer, 30);
                strMasterKey = AString((IMS_CHAR*)pAudioProfile->szKey,30).ToBase64();

                strAcap.Sprintf("crypto:%d %s inline:%s|2^%d|1:4",nCrypto,"AES_CM_128_HMAC_SHA1_32",
                        strMasterKey.GetStr(), pAudioProfile->nMasterKeyLifeTime);
                break;
            case MMPF_SRTP_CRYPTO_TYPE_AES256_CM_SHA1_80:
                for (int i = 0; i < 32; i++)
                {
                    nTempBuffer[i] = IMS_SYS_GetSRandom(255);
                }

                IMS_MEM_Memcpy(pAudioProfile->szKey,nTempBuffer,32);
                strMasterKey = AString((IMS_CHAR*)pAudioProfile->szKey,32).ToBase64();

                strAcap.Sprintf("crypto:%d %s inline:%s|2^%d|1:4",nCrypto,"AES_CM_256_HMAC_SHA1_80",
                        strMasterKey.GetStr(), pAudioProfile->nMasterKeyLifeTime);
                break;
            case MMPF_SRTP_CRYPTO_TYPE_AES256_CM_SHA1_32:
                for (int i = 0; i < 32; i++)
                {
                    nTempBuffer[i] = IMS_SYS_GetSRandom(255);
                }

                IMS_MEM_Memcpy(pAudioProfile->szKey,nTempBuffer,32);
                strMasterKey = AString((IMS_CHAR*)pAudioProfile->szKey,32).ToBase64();

                strAcap.Sprintf("crypto:%d %s inline:%s|2^%d|1:4",nCrypto,"AES_CM_256_HMAC_SHA1_32",
                        strMasterKey.GetStr(), pAudioProfile->nMasterKeyLifeTime);
                break;
            case MMPF_SRTP_CRYPTO_TYPE_NULL_SHA1_80:
                strAcap.Sprintf("crypto:%d %s inline:%s|2^%d|1:4",nCrypto,"NULL_HMAC_SHA1_80",
                        strMasterKey.GetStr(), pAudioProfile->nMasterKeyLifeTime);
                break;
            case MMPF_SRTP_CRYPTO_TYPE_NULL_SHA1_32:
                strAcap.Sprintf("crypto:%d %s inline:%s|2^%d|1:4",nCrypto,"NULL_HMAC_SHA1_32",
                        strMasterKey.GetStr(), pAudioProfile->nMasterKeyLifeTime);
                break;
            default :
                break;
        }
        pAudioProfile->objCapaNego.mapAttributeCapa.SetValue(nAcap, strAcap);
        IMS_TRACE_I("CreateAudioProfile :: crypto acap is [%s]",strAcap.GetStr(),0,0);

        if (pConfig->GetSrtpSupportCapaNego() == IMS_TRUE)
        {
            //Set Transport Capa to Audio Profile
            AString strTmp;
            pAudioProfile->bSupportCapaNegoForSrtp = IMS_TRUE;
            strTcap = "RTP/SAVP";
            nTCap++;
            pAudioProfile->objCapaNego.mapTransportCapa.SetValue(nTCap, strTcap);
            strPcfg.Sprintf("t=%d a=",nTCap);
            strTmp="";
            for (int i=nSrtpAcapNumStart; i <= nAcap; i++)
            {
                if (strTmp.GetLength() > 0)
                {
                    strTmp.Append(",");
                }

                AString strTmp2;
                strTmp2.Sprintf("%d", i);
                strTmp.Append(strTmp2);
            }
            strPcfg.Append(strTmp);
            pAudioProfile->objCapaNego.bIsAttCapaInPcfg = IMS_TRUE;
            pAudioProfile->objCapaNego.lstPotentialConfig.Append(strPcfg);
        }
        else
        {
            pAudioProfile->bSupportCapaNegoForSrtp = IMS_FALSE;
        }
    }
    IMS_TRACE_D("CreateAudioProfile() - bSupportSrtp[%d], eSrtpProfile[%d], nMasterKeyLifeTime[%d]",
            pAudioProfile->bSupportSrtp, pAudioProfile->eSrtpCryptoType,
            pAudioProfile->nMasterKeyLifeTime);
    IMS_TRACE_D("CreateAudioProfile() - b as mode[%d], b rs mode [%d], b rr mode [%d]",
            pConfig->GetBandwidthMode(MediaConfiguration::BW_AS),
            pConfig->GetBandwidthMode(MediaConfiguration::BW_RS),
            pConfig->GetBandwidthMode(MediaConfiguration::BW_RR));
    IMS_TRACE_D("CreateAudioProfile() - RTCP Enable [%d]", pConfig->GetRtcpEnable(), 0, 0);
    IMS_TRACE_D("CreateAudioProfile() - AS[%d], RR[%d], RS[%d]",
            pAudioProfile->nBandwidthAs, pAudioProfile->nBandwidthRr, pAudioProfile->nBandwidthRs);
    IMS_TRACE_D("CreateAudioProfile() Ended. PayloadSize[%d]",
            pAudioProfile->lstPayload.GetSize(), 0, 0);

    return IMS_TRUE;
}

IMS_BOOL
AudioProfileConfigurer::SetAudioRsRr(OUT AudioProfile* pAudioProfile,
        IN AudioConfiguration* pConfig, IN MEDIA_DIRECTION eDir)
{
    if (pAudioProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("SetAudioRsRr(), eDir[%d], RTCPEnable[%d], RS_BWMode[%d]",
            eDir, pConfig->GetRtcpEnable(), pConfig->GetBandwidthMode(MediaConfiguration::BW_RS));

    // Set RR
    switch (pConfig->GetBandwidthMode(MediaConfiguration::BW_RR))
    {
        case MediaConfiguration::BW_RS_RR_HIDE:
            // RFC 3556, 3. Default values, 3.75% of the session bandwidth
            pAudioProfile->nBandwidthRr = -1;
            break;
        case MediaConfiguration::BW_RS_RR_PERCENT:
            pAudioProfile->nBandwidthRr = pAudioProfile->nBandwidthAs *
                    pConfig->GetBandwidth(MediaConfiguration::BW_RR) * 32 / 3125;
            break;
        case MediaConfiguration::BW_RS_RR_MANUAL:
            pAudioProfile->nBandwidthRr = pConfig->GetBandwidth(MediaConfiguration::BW_RR,
                    pAudioProfile->objIpAddr.IsIPv6Address());
            break;
        default :
            pAudioProfile->nBandwidthRr = pAudioProfile->nBandwidthAs * 1024 / 40;     // 2.5% of AS
            break;
    }

    // Set RS
    switch (pConfig->GetBandwidthMode(MediaConfiguration::BW_RS))
    {
        case MediaConfiguration::BW_RS_RR_HIDE :
            // RFC 3556, 3. Default values, 1.25% of the session bandwidth
            pAudioProfile->nBandwidthRs = -1;
            break;
        case MediaConfiguration::BW_RS_RR_PERCENT :
            pAudioProfile->nBandwidthRs = pAudioProfile->nBandwidthAs *
                    pConfig->GetBandwidth(MediaConfiguration::BW_RS) * 32 / 3125;
            break;
        case MediaConfiguration::BW_RS_RR_MANUAL :
            pAudioProfile->nBandwidthRs = pConfig->GetBandwidth(MediaConfiguration::BW_RS,
                    pAudioProfile->objIpAddr.IsIPv6Address());
            break;
        default :
            pAudioProfile->nBandwidthRs = 0;
            break;
    }

    // Change Initial RTCP disable check logic for KDDI
    /*
    if (pConfig->IsRtcpDisableBeforeSetup() == IMS_TRUE
        && m_bInitialOfferCase == IMS_TRUE
        && eDir != MEDIA_DIRECTION_SEND_RECEIVE) {
        IMS_TRACE_I(" AudioProfileConfigurer::SetAudioRsRr(), Set RS, RR zero for KDDI requirement",
            0, 0, 0);
        pAudioProfile->nBandwidthRs = 0;
        pAudioProfile->nBandwidthRr = 0;
        m_bInitialOfferCase = IMS_FALSE;
    }*/

    //Check RTCP Disable and not hold
    if (pConfig->GetRtcpEnable() == IMS_FALSE)
    {
        if (eDir != MEDIA_DIRECTION_INACTIVE && eDir != MEDIA_DIRECTION_SEND &&
                eDir != MEDIA_DIRECTION_RECEIVE)
        {
            pAudioProfile->nBandwidthRs = 0;
            pAudioProfile->nBandwidthRr = 0;
        }
    }

    // Check RS
    if (pConfig->GetBandwidthMode(MediaConfiguration::BW_RS) != MediaConfiguration::BW_RS_RR_MANUAL)
    {
        if (pAudioProfile->nBandwidthRs > 3000)
        {
            pAudioProfile->nBandwidthRs = 3000;
        }
        else if (pAudioProfile->nBandwidthRs > 100)
        {
            IMS_TRACE_I(" AudioProfileConfigurer::SetAudioRsRr(), Set RS[%d] for its full value",
                    pAudioProfile->nBandwidthRs, 0, 0);
            pAudioProfile->nBandwidthRs = (pAudioProfile->nBandwidthRs / 100) * 100;
        }
    }

    // Check RR
    if (pConfig->GetBandwidthMode(MediaConfiguration::BW_RR) != MediaConfiguration::BW_RS_RR_MANUAL)
    {
        if (pAudioProfile->nBandwidthRr > 3000)
        {
            pAudioProfile->nBandwidthRr = 3000;
        }
        else if (pAudioProfile->nBandwidthRr > 100)
        {
            IMS_TRACE_I(" AudioProfileConfigurer::SetAudioRsRr(), Set RR[%d] for its full value",
                    pAudioProfile->nBandwidthRr, 0, 0);
            pAudioProfile->nBandwidthRr = (pAudioProfile->nBandwidthRr / 100) * 100;
        }
    }

    IMS_TRACE_I(" AudioProfileConfigurer::SetAudioRsRr(), Set RS[%d], RR[%d]",
            pAudioProfile->nBandwidthRs,pAudioProfile->nBandwidthRr,0);

    return IMS_TRUE;
}

IMS_BOOL
AudioProfileConfigurer::MakeNegotiatedBandwidth(IN AudioConfiguration* pConfig,
        IN AudioProfile* pSrcProfile, IN AudioProfile* pDestProfile, IN IMS_BOOL bIsOfferReceived,
        IN IMS_SINT32 nAsValueOfNegoticatedCodec, OUT AudioProfile* pNegotiatedProfile)
{
    IMS_TRACE_D("MakeNegotiatedBandwidth() - BW_NEGO_OPTION_VALUE[%d]",
            pConfig->GetBandwidthNegoOption(), 0, 0);

    if (bIsOfferReceived == IMS_FALSE)
    {
        //MO's Bandwidth Setting
        // 1. Set AS Value
        if (pDestProfile->nBandwidthAs > 0)
        {
            if (pDestProfile->nBandwidthAs > nAsValueOfNegoticatedCodec)
            {
                pNegotiatedProfile->nBandwidthAs = nAsValueOfNegoticatedCodec;
            }
            else
            {
                pNegotiatedProfile->nBandwidthAs = pDestProfile->nBandwidthAs;
            }
        }
        else
        { //Exception Handling (b= AS line is not included in Answer SDP)
            pNegotiatedProfile->nBandwidthAs = pSrcProfile->nBandwidthAs;
        }

        // 2. Set RS/RR Value
        //2.1 Exception Handling (b=RS/RR line is not included in Answer SDP)
        if (pConfig->IsRtcpDisableBeforeSetup() == IMS_TRUE)
        {
            pNegotiatedProfile->nBandwidthRs = 0;
            pNegotiatedProfile->nBandwidthRr = 0;
        }
        else if (pNegotiatedProfile->nBandwidthRs < 0 || pNegotiatedProfile->nBandwidthRr < 0)
        {
            pNegotiatedProfile->nBandwidthRs = pSrcProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pSrcProfile->nBandwidthRr;

            IMS_TRACE_D("MakeNegotiatedBandwidth() - Nego AS[%d] RS[%d] RR[%d]",
                    pSrcProfile->nBandwidthAs, pSrcProfile->nBandwidthRs,
                    pSrcProfile->nBandwidthRr);
            return IMS_TRUE;
        }

        //2.2 Normal Case
        if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE)
        {
            // if RS/RR is used for RTCP Nego value
            pNegotiatedProfile->nBandwidthRs = pDestProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pDestProfile->nBandwidthRr;
        }
        else
        {
            // default case (RS/RR is not negotiated value)
            pNegotiatedProfile->nBandwidthRs = pSrcProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pSrcProfile->nBandwidthRr;
        }
    }
    else
    {
        //MT's Bandwidth Setting
        //1. Set Negotiated AS Value
        if (nAsValueOfNegoticatedCodec > 0)
        {
            pNegotiatedProfile->nBandwidthAs = nAsValueOfNegoticatedCodec;

            // if GetBandwidthNegoOption is BW_OPTION_NEGOTIATED_VALUE, use lower AS value
            if ((pConfig->GetBandwidthNegoOption() ==
                    MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE) &&
                    (nAsValueOfNegoticatedCodec > pDestProfile->nBandwidthAs) &&
                    (pDestProfile->nBandwidthAs > 0))
            {
                pNegotiatedProfile->nBandwidthAs = pDestProfile->nBandwidthAs;
            }
        }
        else
        {
            pNegotiatedProfile->nBandwidthAs = pSrcProfile->nBandwidthAs;
        }

        //2. if AS Mode is Manual, then use source profile's AS value (for lgu hidden menu)
        if (pConfig->GetBandwidthMode(MediaConfiguration::BW_AS) ==
                MediaConfiguration::BW_MODE_MANUAL)
        {
            pNegotiatedProfile->nBandwidthAs = pSrcProfile->nBandwidthAs;
        }

        //3. Set RS/RR Value
        if (pConfig->IsRtcpDisableBeforeSetup() == IMS_TRUE)
        {
            pNegotiatedProfile->nBandwidthRs = 0;
            pNegotiatedProfile->nBandwidthRr = 0;
        }
        else if (pDestProfile->eDirection != MEDIA_DIRECTION_SEND_RECEIVE)
        { // Hold Case
            //3.1 Hold Case
            SetAudioRsRr(pNegotiatedProfile, pConfig, pDestProfile->eDirection);
        }
        else
        {
            // 3.2 Active Call Case
            //3.2.1 Exception Handling (b=RS/RR line is not included in Answer SDP)
            if (pNegotiatedProfile->nBandwidthRs < 0 || pNegotiatedProfile->nBandwidthRr < 0)
            {
                pNegotiatedProfile->nBandwidthRs = pSrcProfile->nBandwidthRs;
                pNegotiatedProfile->nBandwidthRr = pSrcProfile->nBandwidthRr;

                IMS_TRACE_D("MakeNegotiatedBandwidth() - AS[%d] RS[%d] RR[%d]",
                        pSrcProfile->nBandwidthAs, pSrcProfile->nBandwidthRs,
                        pSrcProfile->nBandwidthRr);
                return IMS_TRUE;
            }

            //3.2.2 Normal Case
            if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE)
            {
                // if RS/RR is used for RTCP Nego value
                if (pConfig->GetRtcpEnable() == IMS_TRUE)
                {
                    // only use rtcp when rtcp state is enable
                    IMS_TRACE_D("MakeNegotiatedBandwidth() - use peer RS[%d] RR[%d]",
                            pDestProfile->nBandwidthRs, pDestProfile->nBandwidthRr, 0);
                    pNegotiatedProfile->nBandwidthRs = pDestProfile->nBandwidthRs;
                    pNegotiatedProfile->nBandwidthRr = pDestProfile->nBandwidthRr;
                }
                else
                {
                    IMS_TRACE_D("MakeNegotiatedBandwidth() - use local RS[%d] RR[%d]",
                            pSrcProfile->nBandwidthRs, pSrcProfile->nBandwidthRr, 0);
                    pNegotiatedProfile->nBandwidthRs = pSrcProfile->nBandwidthRs;
                    pNegotiatedProfile->nBandwidthRr = pSrcProfile->nBandwidthRr;
                }
            }
            else
            {
                // default case (RS/RR is not negotiated value)
                pNegotiatedProfile->nBandwidthRs = pSrcProfile->nBandwidthRs;
                pNegotiatedProfile->nBandwidthRr = pSrcProfile->nBandwidthRr;
            }
        }
    }

    IMS_TRACE_D("MakeNegotiatedBandwidth() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
            pNegotiatedProfile->nBandwidthAs, pNegotiatedProfile->nBandwidthRs,
            pSrcProfile->nBandwidthRr);
    return IMS_TRUE;
}

VIRTUAL
IMS_BOOL AudioProfileConfigurer::CheckHoldDirection(IN AudioConfiguration* pConfig,
        IN MEDIA_DIRECTION eDir, IN AudioProfile* pSrcProfile)
{
    IMS_BOOL bRet = IMS_FALSE;

    if (pConfig == NULL)
    {
        return bRet;
    }

    // Check the direction if it's a HOLD
    IMSList<IMS_SINT32> eListHoldDir = pConfig->GetHoldDirList();
    for (IMS_UINT32 i=0; i<eListHoldDir.GetSize(); i++)
    {
        if (eDir == (MEDIA_DIRECTION)eListHoldDir.GetAt(i))
        {
            // If the direction is involved in list of HOLD direction, it'll be treated as HOLD
            IMS_TRACE_D("CheckHoldDirection() - Direction[%d] is treated as HOLD",
                    eListHoldDir.GetAt(i), 0, 0);
            bRet = IMS_TRUE;
            break;
        }
    }

    // Check unicast start stream.
    if (pSrcProfile != NULL && bRet == IMS_TRUE && pSrcProfile->bIsOfferCase == IMS_TRUE &&
            pSrcProfile->eDirection == MEDIA_DIRECTION_SEND_RECEIVE &&
            eDir != MEDIA_DIRECTION_INACTIVE)
    {
        IMS_TRACE_D("CheckHoldDirection() - Direction[%d] unicast stream media start case",
                eDir, 0, 0);
        bRet = IMS_FALSE;
    }

    return bRet;
}

VIRTUAL
const IMS_SINT32* AudioProfileConfigurer::GetAmrAsArray(
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

VIRTUAL
const IMS_SINT32* AudioProfileConfigurer::GetEVSASArray(
        IN IMS_SINT32 nEVSFormat, IN IMS_BOOL bIpV6)
{
    const IMS_SINT32* pArrEvs;
    IMS_SINT32 AsArrIndex = 0;
    if (nEVSFormat == 1)
    {
        AsArrIndex += 2;        // AMR IO mode == 1
    }
    if (bIpV6 == IMS_TRUE)
    {
        AsArrIndex += 1;
    }
    pArrEvs = EVS_AS[AsArrIndex];
    return pArrEvs;
}

VIRTUAL
IMS_SINT32 AudioProfileConfigurer::ConvertToBandwidthAS(IN IMS_SINT32 eCodec,IN IMS_SINT32 nOctet,
        IN IMS_BOOL bIpV6, IN IMS_SINT32 nModeSet, IN IMS_BOOL bGetMaxValue /*= IMS_FALSE*/)
{
    IMS_TRACE_D("ConvertToBandwidthAS() - Entered. eCodec[%d] nOctet[%d] nModeSet[%d]",
            eCodec, nOctet, nModeSet);

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

PUBLIC GLOBAL
IMS_SINT32 AudioProfileConfigurer::ConvertToBandwidthAS(IN IMS_SINT32 eCodec, IN IMS_BOOL bIpV6,
        IN IMS_SINT32 nCodecFormat, IN IMS_SINT32 nCodecMode, IN IMS_BOOL bGetMaxValue)
{
    IMS_TRACE_D("ConvertToBandwidthAS() - Entered. eCodec[%d] bIpV6[%d] nCodecFormat[%d]",
            eCodec, bIpV6, nCodecFormat);

    IMS_SINT32 nResultAs = -1;

    const IMS_SINT32* pArrEvs;
    pArrEvs = GetEVSASArray(nCodecFormat, bIpV6);

    IMS_SINT32 nModeCount = 0;

    if (nCodecFormat == 1)
    {
        nModeCount = 9;      // AMR IO Mode
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

VIRTUAL
IMS_SINT32 AudioProfileConfigurer::ConvertToModeSet(IN IMS_SINT32 eCodec,
        IN IMS_SINT32 nOctet, IN IMS_BOOL bIpV6, IN IMS_SINT32 nAs)
{
    IMS_TRACE_D("ConvertToModeSet() eCodec[%d], nOctet[%d], nAs[%d]",
            eCodec, nOctet, nAs);

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

    IMS_TRACE_D("ConvertToModeSet() - Result : bIpV6[%d]m nModeSet[%d]",
            bIpV6, nModeSet, 0);
    return nModeSet;
}

/*!
 *  @brief      UpdateAudioProfileBandwidth
 *  @details    UpdateAudioProfileBandwidth for update bandwidth(AS) at audio profile
 */
PUBLIC GLOBAL
IMS_BOOL AudioProfileConfigurer::UpdateAudioProfileBandwidth(
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
            AudioProfile::AmrFmtp* pAmrFmtp = (AudioProfile::AmrFmtp*)pAudioPayload->pFmtp;
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
                    nMaxModeset = 8;   // amr wb case
                }
                else
                {
                    nMaxModeset = 7;   // amr nb case
                }
            }
            else
            {
                for (nMaxModeset = 8; nMaxModeset >= 0;  nMaxModeset--)
                {
                    IMS_UINT32 nMatch = pAmrFmtp->nModeSetList & (1 << nMaxModeset);
                    if (nMatch)
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
            AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pAudioPayload->pFmtp;
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
                        IMS_UINT32 nMatch = pEvsFmtp->nBrList & (1 << nMaxBr);
                        if (nMatch)
                        {
                            break;
                        }
                    }
                }
            }

            nCurrAs = ConvertToBandwidthAS(
                    nCurrCodec, pAudioProfile->objIpAddr.IsIPv6Address(),0, nMaxBr); // primary mode
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
            if (72 > nAsMax) // 72 is PCMU/PCMA AS value at IPv6
            {
                nAsMax = 72;
                nAsOptimal = 72;
            }
        }
    }

    // setting AS/RS/RR value.
    switch (pConfig->GetBandwidthMode(MediaConfiguration::BW_AS))
    {
        case MediaConfiguration::BW_MODE_HIDE:
        {
            pAudioProfile->nBandwidthAs = -1;
            break;
        }
        case MediaConfiguration::BW_MODE_OPTIMAL:
        {
            pAudioProfile->nBandwidthAs = nAsOptimal;
            break;
        }
        case MediaConfiguration::BW_MODE_MAX:
        {
            pAudioProfile->nBandwidthAs =  nAsMax;
            break;
        }
        case MediaConfiguration::BW_MODE_MANUAL:
        {
            pAudioProfile->nBandwidthAs = pConfig->GetBandwidth(
                    MediaConfiguration::BW_AS, pAudioProfile->objIpAddr.IsIPv6Address());
            break;
        }
        default:
        {
            IMS_TRACE_E(0, "UpdateAudioProfileBandwidth - invalide bw as mode %d",
                    pConfig->GetBandwidthMode(MediaConfiguration::BW_AS), 0, 0);
            break;
        }
    }

    SetAudioRsRr(pAudioProfile, pConfig, MEDIA_DIRECTION_SEND_RECEIVE);
    IMS_TRACE_D("UpdateAudioProfileBandwidth() update AS[%d]", nAsMax, 0, 0);

    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_SINT32 AudioProfileConfigurer::GetLargestModesetInFmtp(
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
        AudioProfile::AmrFmtp* pAmrFmtp =(AudioProfile::AmrFmtp*) pPayload->pFmtp;
        if (pAmrFmtp == NULL)
        {
            return  NO_MODESET;
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
            for (nModeSet = 8; nModeSet >=0; nModeSet--)
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
        AudioProfile::EvsFmtp* pEvsFmtp =(AudioProfile::EvsFmtp*) pPayload->pFmtp;
        if (pEvsFmtp == NULL)
        {
            return  NO_MODESET;
        }

        //Primary mode
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
        else // AMR IO mode
        {
            if (pEvsFmtp->nModeSetList == 0)
            {
                return EVS_IO_MODE_MAX_MODESET;
            }
            else
            {
                IMS_SINT32 nModeSet = 0;
                for (nModeSet = 8; nModeSet >=0; nModeSet--)
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