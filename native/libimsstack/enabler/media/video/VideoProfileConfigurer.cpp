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
#include "ServiceSystemTime.h"

#include "config/CodecAvcConfig.h"
#include "config/CodecHevcConfig.h"
#include "config/VideoConfiguration.h"
#include "video/VideoDef.h"
#include "video/VideoProfileConfigurer.h"
#include "MediaManager.h"

// == DEFINES =========================================================
__IMS_TRACE_TAG_USER_DECL__("MED.PC");

PUBLIC GLOBAL IMS_BOOL VideoProfileConfigurer::CreateVideoProfile(OUT VideoProfile* pVideoProfile,
        IN MediaEnvironment* pEnvironment, IN VideoConfiguration* pConfig, IN IMS_SINT32 nSlotId)
{
    IMS_SINT32 nMaxAS = -1;
    IMS_SINT32 nMaxFramerate = -1;

    if (pEnvironment == IMS_NULL || pConfig == IMS_NULL || pVideoProfile == IMS_NULL ||
            pEnvironment->pIService == IMS_NULL)
    {
        return IMS_FALSE;
    }
    IMS_TRACE_D("CreateVideoProfile() Entered.", 0, 0, 0);

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
    //
    // Step 1. Setting IP from IService
    pVideoProfile->objIpAddr = pEnvironment->pIService->GetIPAddress();

    //
    // Step 2. Setting RTP/RTCP port from resource manager
    //

    // To reuse port in case of re-creation
    if (pVideoProfile->nDataPort > 0)
    {
        pVideoProfile->nDataPort =
                pResourceMngr->AcquireRtpPort(pVideoProfile->nDataPort, pVideoProfile->nDataPort);
    }
    else
    {
        pVideoProfile->nDataPort = pResourceMngr->AcquireRtpPort(pConfig);
    }
    pVideoProfile->nControlPort = pVideoProfile->nDataPort + 1;

    IMS_TRACE_D("CreateVideoProfile() objIpAddr[%s], port[%d]",
            pVideoProfile->objIpAddr.ToCharString(), pVideoProfile->nDataPort, 0);

    //
    // Step 3. Setting profile type
    //
    if (pConfig->IsVideoAvpfEnabled() && pConfig->GetSdpOfferCapNegoForAvpf())
    {
        pVideoProfile->strTransportType = "RTP/AVPF";
    }
    else
    {
        pVideoProfile->strTransportType = "RTP/AVP";
    }

    while (pVideoProfile->lstPayload.GetSize() > 0)
    {
        VideoProfile::Payload* pPayload = pVideoProfile->lstPayload.GetAt(0);
        if (pPayload != IMS_NULL)
        {
            delete pPayload;
        }
        pVideoProfile->lstPayload.RemoveAt(0);
    }

    //
    // Step 4. Setting each payload and bandwidth
    //
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

        if (pCodecConfig->GetCodec() == ImsCodec::VIDEO_NONE)
        {
            IMS_TRACE_D("CreateVideoProfile() invalid codec, skip config(%d) - %d", i,
                    pCodecConfig->GetCodec(), 0);
            continue;
        }

        if (pCodecConfig->GetPayloadType() == -1)
        {
            IMS_TRACE_D("CreateVideoProfile() invalid payload type, skip config(%d) - %d:%s", i,
                    pCodecConfig->GetCodec(), ImsCodec::CodecToString(pCodecConfig->GetCodec()));
            continue;
        }

        if (pCodecConfig->GetCodec() == ImsCodec::VIDEO_AVC)
        {
            CodecAvcConfig* pAvcConfig = reinterpret_cast<CodecAvcConfig*>(pCodecConfig);
            VideoProfile::AvcFmtp* pAvcFmtp = new VideoProfile::AvcFmtp();
            const IMS_CHAR* pbAvc4SpropParameterSets;

            if (pAvcConfig->GetProfileLevelId() == AString::ConstEmpty())
            {
                IMS_TRACE_D("create - delete pAvcFmtp", 0, 0, 0);
                delete pAvcFmtp;
                continue;
            }
            pVideoProfile->nFrameRate = pAvcConfig->GetFramerate();
            pAvcFmtp->nFrameRate = pAvcConfig->GetFramerate();
            pAvcFmtp->eResolution = GetResolutionFromWidthHeight(
                    pAvcConfig->GetResolutionWidth(), pAvcConfig->GetResolutionHeight());
            pAvcFmtp->nBitrate = pAvcConfig->GetBitrate();
            pAvcFmtp->nAs = pConfig->GetAsBandwidthKbps();

            pAvcFmtp->nProfile = GetAvcProfileFromProfileLevelId(pAvcConfig->GetProfileLevelId());
            pAvcFmtp->nLevel = GetAvcLevelFromProfileLevelId(pAvcConfig->GetProfileLevelId());

            pbAvc4SpropParameterSets = pAvcConfig->GetSpropParameterSets().GetStr();
            /* TODO_MEDIA later sprop need to find a way to get SpropPramaterSets
            pbAvc4SpropParameterSets = GetAvcSpropParameterSets(
                    pAvcFmtp->eResolution, pAvcFmtp->nProfile, pAvcFmtp->nLevel);
            */

            IMS_TRACE_D("create - eResolution[%d], nBitrate[%d], nFrameRate[%d]",
                    pAvcFmtp->eResolution, pAvcFmtp->nBitrate, pAvcFmtp->nFrameRate);
            IMS_TRACE_D("create - nAS[%d], nProfile[%d], nLevel[%d]", pAvcFmtp->nAs,
                    pAvcFmtp->nProfile, pAvcFmtp->nLevel);

            IMS_TRACE_D("create - pbAvc4SpropParameterSets[%s]", pbAvc4SpropParameterSets, 0, 0);

            if (pbAvc4SpropParameterSets == IMS_NULL || (pbAvc4SpropParameterSets[0] == '\0'))
            {
                continue;
            }

            if (!pAvcConfig->GetProfileLevelId().IsEmpty())
            {
                pAvcFmtp->strProfileLevelId = pAvcConfig->GetProfileLevelId();
                pAvcFmtp->bShow_ProfileLevelId = IMS_TRUE;
            }

            if (pAvcConfig->GetPacketizationMode() != -1)
            {
                pAvcFmtp->nPacketizationMode = pAvcConfig->GetPacketizationMode();
                pAvcFmtp->bShow_PacketizationMode = IMS_TRUE;
            }

            if (pAvcConfig->GetIncludeSpropParameterSets())
            {
                pAvcFmtp->strSpropParam = pbAvc4SpropParameterSets;
                pAvcFmtp->bShow_SpropParam = IMS_TRUE;
            }

            VideoProfile::Payload* pAvcPayload = new VideoProfile::Payload();

            pAvcPayload->SetRtpMap(pAvcConfig->GetPayloadType(),
                    ImsCodec::CodecToString(pAvcConfig->GetCodec()),
                    pConfig->GetVideoSamplingRate(), pAvcConfig->GetChannel());
            pAvcPayload->pFmtp = reinterpret_cast<void*>(pAvcFmtp);

            if (!pAvcConfig->GetImageAttr().IsEmpty() && !pAvcConfig->GetImageAttr().IsNULL())
            {
                pAvcPayload->bIncludeImageAttr = IMS_TRUE;
                pAvcPayload->strImageAttr = pAvcConfig->GetImageAttr();
            }
            else if (!pAvcConfig->GetFrameSize().IsEmpty() && !pAvcConfig->GetFrameSize().IsNULL())
            {
                pAvcPayload->bIncludeFrameSize = IMS_TRUE;
            }

            if (pConfig->IsVideoAvpfEnabled() == IMS_TRUE)
            {
                if (pConfig->IsVideoAvpfTrrEnabled() == IMS_TRUE)
                {
                    pAvcPayload->objRtcpFbAttr.bTrrSupported = IMS_TRUE;
                    pAvcPayload->objRtcpFbAttr.nTrrInt = pConfig->GetRtcpInterval() * 1000;
                }

                if (pConfig->IsbVideoAvpfNackEnabled() == IMS_TRUE)
                {
                    pAvcPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                }

                if (pConfig->IsVideoAvpfTmmbrEnabled() == IMS_TRUE)
                {
                    pAvcPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                    pAvcPayload->objRtcpFbAttr.nTmmbrSmaxPr = 40;

                    pAvcPayload->objRtcpFbAttr.nTmmbrDownInterval =
                            pConfig->GetVideoAvpfTmmbrDownIntervalSec();
                    pAvcPayload->objRtcpFbAttr.nTmmbrUpInterval =
                            pConfig->GetVideoAvpfTmmbrUpIntervalSec();
                    pAvcPayload->objRtcpFbAttr.nTmmbrLossThreshold =
                            pConfig->GetVideoAvpfTmmbrLossThresholdRatio();
                    pAvcPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio =
                            pConfig->GetVideoAvpfTmmbrMinBitrateKbps();
                    pAvcPayload->objRtcpFbAttr.nTmmbrBitrateLevel =
                            pConfig->GetVideoAvpfTmmbrBitrateLevel();
                    pAvcPayload->objRtcpFbAttr.nTmmbrUpLevel = pConfig->GetVideoAvpfTmmbrUpLevel();
                }

                if (pConfig->IsVideoAvpfPliEnabled() == IMS_TRUE)
                {
                    pAvcPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                }

                if (pConfig->IsVideoAvpfFirEnabled() == IMS_TRUE)
                {
                    pAvcPayload->objRtcpFbAttr.bFirSupported = IMS_TRUE;
                }

                IMS_TRACE_D("CreateVideoProfile() AVPF. bNACK[%d], bTMMBR[%d], bPLI[%d]",
                        pAvcPayload->objRtcpFbAttr.bNackSupported,
                        pAvcPayload->objRtcpFbAttr.bTmmbrSupported,
                        pAvcPayload->objRtcpFbAttr.bPliSupported);
                IMS_TRACE_D("CreateVideoProfile() AVPF. bFIR[%d], bTRR_Int[%d], nTrr-int[%d]",
                        pAvcPayload->objRtcpFbAttr.bFirSupported,
                        pAvcPayload->objRtcpFbAttr.bTrrSupported,
                        pAvcPayload->objRtcpFbAttr.nTrrInt);

                if (pAvcPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                {
                    IMS_TRACE_D("CreateVideoProfile() TMMBR - T_Down[%d], T_Up[%d],\
                            LossRate[%d]",
                            pAvcPayload->objRtcpFbAttr.nTmmbrDownInterval,
                            pAvcPayload->objRtcpFbAttr.nTmmbrUpInterval,
                            pAvcPayload->objRtcpFbAttr.nTmmbrLossThreshold);
                    IMS_TRACE_D("CreateVideoProfile() TMMBR - Level[%d], MinBitrate[%d],\
                            nTmmbrUpLevel[%d]",
                            pAvcPayload->objRtcpFbAttr.nTmmbrBitrateLevel,
                            pAvcPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio,
                            pAvcPayload->objRtcpFbAttr.nTmmbrUpLevel);
                }
            }

            pVideoProfile->lstPayload.Append(pAvcPayload);

            if (pConfig->GetAsBandwidthKbps() > nMaxAS)
            {
                nMaxAS = pConfig->GetAsBandwidthKbps();
            }
            if (pAvcConfig->GetFramerate() > nMaxFramerate)
            {
                nMaxFramerate = pAvcConfig->GetFramerate();
            }

            IMS_TRACE_D("CreateVideoProfile() add payload(%d) - %s", i,
                    ImsCodec::CodecToString(pAvcConfig->GetCodec()), 0);
        }
        else if (pCodecConfig->GetCodec() == ImsCodec::VIDEO_HEVC)
        {
            /* TODO_MEDIA later
            CodecHevcConfig* pHevcConfig = reinterpret_cast<CodecHevcConfig*>(pCodecConfig);

            VideoProfile::HevcFmtp* pHevcFmtp = new VideoProfile::HevcFmtp();

            pVideoProfile->nFrameRate = pHevcConfig->GetFramerate();
            pHevcFmtp->nFrameRate = pHevcConfig->GetFramerate();
            pHevcFmtp->eResolution = GetResolutionFromWidthHeight(
                    pHevcConfig->GetResolutionWidth(), pHevcConfig->GetResolutionHeight());
            pHevcFmtp->nBitrate = pHevcConfig->GetBitrate();

            pHevcFmtp->nAs = pConfig->GetAsBandwidthKbps();
            // TODO_MEDIA video sprop
            // const IMS_CHAR* pHevcSpropParameterSets =
            //         GetHevcSpropParameterSets(pHevcFmtp->eResolution, pHevcFmtp->nProfile);

            if (pHevcConfig->GetHevcProfile() != -1)
            {
                pHevcFmtp->nProfile =
                        reinterpret_cast<VIDEO_PROFILE_HEVC*>(pHevcConfig->GetHevcProfile());
                pHevcFmtp->bShow_Profile = IMS_TRUE;
            }

            if (pHevcConfig->GetHevcLevel() != -1)
            {
                pHevcFmtp->nLevel = pHevcConfig->GetHevcLevel();
                pHevcFmtp->bShow_Level = IMS_TRUE;
            }

            if (pHevcConfig->GetPacketizationMode() != -1)
            {
                pHevcFmtp->nPacketizationMode = pHevcConfig->GetPacketizationMode();
                pHevcFmtp->bShow_PacketizationMode = IMS_TRUE;
            }

            if (pHevcConfig->GetIncludeSpropParameterSets())
            {
                pHevcFmtp->strSpropParam = pHevcSpropParameterSets;
                pHevcFmtp->bShow_SpropParam = IMS_TRUE;
            }

            VideoProfile::Payload* pHevcPayload = new VideoProfile::Payload();

            pHevcPayload->SetRtpMap(pHevcPayload->GetPayloadType(),
                    ImsCodec::CodecToString(pHevcPayload->GetCodec()),
                    pConfig->GetVideoSamplingRate(), pHevcConfig->GetChannel());
            pHevcPayload->pFmtp = reinterpret_cast<void*>(pHevcPayload);

            if (!pHevcConfig->GetImageAttr().IsEmpty() && !pHevcConfig->GetImageAttr().IsNULL())
            {
                pHevcPayload->bIncludeImageAttr = IMS_TRUE;
            }

            else if (!pHevcConfig->GetIncludeFrameSize().IsEmpty() &&
            !pHevcConfig->GetIncludeFrameSize().IsNULL())
            {
                pHevcPayload->bIncludeFrameSize = IMS_TRUE;
            }

            if (pConfig->IsVideoAvpfEnabled() == IMS_TRUE)
            {
                if (pConfig->IsbVideoAvpfNackEnabled() == IMS_TRUE)
                {
                    pHevcPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                }

                if (pConfig->IsVideoAvpfTmmbrEnabled() == IMS_TRUE)
                {
                    pHevcPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                    pHevcPayload->objRtcpFbAttr.nTmmbrSmaxPr = 40;

                    pHevcPayload->objRtcpFbAttr.nTmmbrDownInterval =
                            pConfig->GetVideoAvpfTmmbrDownIntervalSec();
                    pHevcPayload->objRtcpFbAttr.nTmmbrUpInterval =
                            pConfig->GetVideoAvpfTmmbrUpIntervalSec();
                    pHevcPayload->objRtcpFbAttr.nTmmbrLossThreshold =
                            pConfig->GetVideoAvpfTmmbrLossThresholdRatio();
                    pHevcPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio =
                            pConfig->GetVideoAvpfTmmbrMinBitrateKbps();
                    pHevcPayload->objRtcpFbAttr.nTmmbrBitrateLevel =
                            pConfig->GetVideoAvpfTmmbrBitrateLevel();
                    pHevcPayload->objRtcpFbAttr.nTmmbrUpLevel = pConfig->GetVideoAvpfTmmbrUpLevel();
                }

                if (pConfig->IsVideoAvpfPliEnabled() == IMS_TRUE)
                {
                    pHevcPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                }

                if (pConfig->IsVideoAvpfFirEnabled() == IMS_TRUE)
                {
                    pHevcPayload->objRtcpFbAttr.bFirSupported = IMS_TRUE;
                }

                IMS_TRACE_D("CreateVideoProfile() AVPF. bNACK[%d], bTMMBR[%d], bPLI[%d]",
                        pHevcPayload->objRtcpFbAttr.bNackSupported,
                        pHevcPayload->objRtcpFbAttr.bTmmbrSupported,
                        pHevcPayload->objRtcpFbAttr.bPliSupported);
                IMS_TRACE_D("CreateVideoProfile() AVPF. bFIR[%d]",
                        pHevcPayload->objRtcpFbAttr.bFirSupported, 0, 0);

                if (pHevcPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                {
                    IMS_TRACE_D("CreateVideoProfile() TMMBR - T_Down[%d], T_Up[%d],\
                            LossRate[%d]",
                            pHevcPayload->objRtcpFbAttr.nTmmbrDownInterval,
                            pHevcPayload->objRtcpFbAttr.nTmmbrUpInterval,
                            pHevcPayload->objRtcpFbAttr.nTmmbrLossThreshold);
                    IMS_TRACE_D("CreateVideoProfile() TMMBR - Level[%d], MinBitrate[%d],\
                            nTmmbrUpLevel[%d]",
                            pHevcPayload->objRtcpFbAttr.nTmmbrBitrateLevel,
                            pHevcPayload->objRtcpFbAttr.nTmmbrMinBitrateRatio,
                            pHevcPayload->objRtcpFbAttr.nTmmbrUpLevel);
                }
            }

            pVideoProfile->lstPayload.Append(pHevcPayload);

            if (pHevcConfig->GetAsBandwidthKbps() > nMaxAS)
            {
                nMaxAS = pHevcConfig->GetAsBandwidthKbps();
            }
            if (pHevcConfig->GetFramerate() > nMaxFramerate)
            {
                nMaxFramerate = pHevcConfig->GetFramerate();
            }

            IMS_TRACE_D("CreateVideoProfile() add payload(%d) - %s", i,
                    ImsCodec::CodecToString(pHevcConfig->GetCodec()), 0);
            */
        }
    }

    // Step 5. Setting direction, candidate, framerate, as/rr/rs
    pVideoProfile->eDirection = MEDIA_DIRECTION_SEND_RECEIVE;

    // Step 6. Setting CVO
    pVideoProfile->nCvoId = pConfig->GetCvoId();

    // framerate is in CodecAvc/HevcConfig
    pVideoProfile->nBandwidthAs = pConfig->GetAsBandwidthKbps();
    SetVideoRsRr(pVideoProfile, pConfig);

    IMS_TRACE_D("CreateVideoProfile - AS[%d], RR[%d], RS[%d]", pVideoProfile->nBandwidthAs,
            pVideoProfile->nBandwidthRr, pVideoProfile->nBandwidthRs);

    IMS_SINT32 nTCap = 0;
    IMS_SINT32 nAcap = 0;
    if (pConfig->IsVideoAvpfEnabled() == IMS_TRUE)
    {
        AString strTemp = "";
        pVideoProfile->bSupportAvpf = IMS_TRUE;

        // set default capa nego value to profile
        {
            // set transport capa - only support AVPF profile..
            nTCap++;
            strTemp = "RTP/AVPF";
            pVideoProfile->objCapaNego.mapTransportCapa.SetValue(nTCap, strTemp);

            // set attribute capa to CapaNeg
            if (pConfig->IsVideoAvpfTrrEnabled() == IMS_TRUE)
            {
                nAcap++;
                strTemp.Sprintf("%s %d", "rtcp-fb:* trr-int", pConfig->GetRtcpInterval() * 1000);
                pVideoProfile->objCapaNego.mapAttributeCapa.SetValue(nAcap, strTemp);
            }

            if (pConfig->IsbVideoAvpfNackEnabled() == IMS_TRUE)
            {
                nAcap++;
                strTemp = "rtcp-fb:* nack";
                pVideoProfile->objCapaNego.mapAttributeCapa.SetValue(nAcap, strTemp);
            }

            if (pConfig->IsVideoAvpfPliEnabled() == IMS_TRUE)
            {
                nAcap++;
                strTemp = "rtcp-fb:* nack pli";
                pVideoProfile->objCapaNego.mapAttributeCapa.SetValue(nAcap, strTemp);
            }

            if (pConfig->IsVideoAvpfFirEnabled() == IMS_TRUE)
            {
                nAcap++;
                strTemp = "rtcp-fb:* ccm fir";
                pVideoProfile->objCapaNego.mapAttributeCapa.SetValue(nAcap, strTemp);
            }

            if (pConfig->IsVideoAvpfTmmbrEnabled() == IMS_TRUE)
            {
                nAcap++;
                strTemp = "rtcp-fb:* ccm tmmbr";
                pVideoProfile->objCapaNego.mapAttributeCapa.SetValue(nAcap, strTemp);
            }
        }

        IMS_SINT32 nCapaNegoForAvpfOption = pConfig->GetSdpOfferCapNegoForAvpf();
        // 1 - CAPNEG_OFFER_WITHOUT_ACAP
        // 2 - CAPNEG_OFFER_WITH_ACAP

        if (nCapaNegoForAvpfOption > MediaConfiguration::CAPNEG_OFFER_NONE)
        {
            pVideoProfile->bSupportCapaNegoForAvpf = IMS_TRUE;
            IMS_SINT32 i = 0;
            AString strPcfg = AString::ConstNull();
            AString strTmp = AString::ConstNull();
            if (nCapaNegoForAvpfOption == MediaConfiguration::CAPNEG_OFFER_WITHOUT_ACAP)
            {
                strPcfg.Sprintf("t=%d", nTCap);  // does not add acap..
                pVideoProfile->objCapaNego.bIsAttCapaInPcfg = IMS_FALSE;
            }
            else if (nCapaNegoForAvpfOption == MediaConfiguration::CAPNEG_OFFER_WITH_ACAP)
            {
                strPcfg.Sprintf("t=%d a=", nTCap);
                for (i = 1; i <= nAcap; i++)
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
        else
        {
            pVideoProfile->bSupportCapaNegoForAvpf = IMS_FALSE;
        }
    }

    IMS_TRACE_D("CreateVideoProfile - bSupportAvpf[%d], bSupportCapaNegoForAvpf[%d]",
            pVideoProfile->bSupportAvpf, pVideoProfile->bSupportCapaNegoForAvpf, 0);
    IMS_TRACE_D("CreateVideoProfile - AS[%d], RR[%d], RS[%d]", pVideoProfile->nBandwidthAs,
            pVideoProfile->nBandwidthRr, pVideoProfile->nBandwidthRs);
    IMS_TRACE_D("CreateVideoProfile - PayloadSize[%d]", pVideoProfile->lstPayload.GetSize(), 0, 0);

    return IMS_TRUE;
}

/*!
 *  @brief      UpdateAudioProfile
 *  @details    UpdateAudioProfile for IP changes or IP setting latency
 */
PUBLIC GLOBAL IMS_BOOL VideoProfileConfigurer::UpdateVideoProfile(
        OUT VideoProfile* pVideoProfile, IN MediaEnvironment* pEnvironment)
{
    if (pEnvironment == IMS_NULL || pVideoProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }
    IMS_TRACE_D("UpdateVideoProfile() Entered.", 0, 0, 0);

    pVideoProfile->objIpAddr = pEnvironment->pIService->GetIPAddress();

    return IMS_TRUE;
}

PUBLIC GLOBAL void VideoProfileConfigurer::GetWidthHeightFromResolution(
        VIDEO_RESOLUTION eResolution, IMS_UINT32* nWidth, IMS_UINT32* nHeight)
{
    switch (eResolution)
    {
        case VIDEO_RESOLUTION_QCIF_PR:
            *nWidth = 144;
            *nHeight = 176;
            break;
        case VIDEO_RESOLUTION_QVGA_LS:
            *nWidth = 320;
            *nHeight = 240;
            break;
        case VIDEO_RESOLUTION_QVGA_PR:
            *nWidth = 240;
            *nHeight = 320;
            break;
        case VIDEO_RESOLUTION_VGA_LS:
            *nWidth = 640;
            *nHeight = 480;
            break;
        case VIDEO_RESOLUTION_VGA_PR:
            *nWidth = 480;
            *nHeight = 640;
            break;
        case VIDEO_RESOLUTION_QCIF_LS:
            *nWidth = 176;
            *nHeight = 144;
            break;
        case VIDEO_RESOLUTION_CIF_LS:
            *nWidth = 352;
            *nHeight = 288;
            break;
        case VIDEO_RESOLUTION_CIF_PR:
            *nWidth = 288;
            *nHeight = 352;
            break;
        case VIDEO_RESOLUTION_SQCIF_LS:
            *nWidth = 128;
            *nHeight = 96;
            break;
        case VIDEO_RESOLUTION_SQCIF_PR:
            *nWidth = 96;
            *nHeight = 128;
            break;
        case VIDEO_RESOLUTION_SIF_LS:
            *nWidth = 352;
            *nHeight = 240;
            break;
        case VIDEO_RESOLUTION_SIF_PR:
            *nWidth = 240;
            *nHeight = 352;
            break;
        case VIDEO_RESOLUTION_HD_LS:
            *nWidth = 1280;
            *nHeight = 720;
            break;
        case VIDEO_RESOLUTION_HD_PR:
            *nWidth = 720;
            *nHeight = 1280;
            break;
        case VIDEO_RESOLUTION_FHD_LS:
            *nWidth = 1920;
            *nHeight = 1080;
            break;
        case VIDEO_RESOLUTION_FHD_PR:
            *nWidth = 1080;
            *nHeight = 1920;
            break;
        default:
            *nWidth = 0;
            *nHeight = 0;
            break;
    }

    IMS_TRACE_D("GetWidthHeightFromResolution - resolutionID[%d], nWidth[%d], nHeight[%d]",
            eResolution, *nWidth, *nHeight);
}

PUBLIC GLOBAL VIDEO_RESOLUTION VideoProfileConfigurer::GetResolutionFromWidthHeight(
        IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight)
{
    if (nWidth == 176 && nHeight == 144)
    {
        return VIDEO_RESOLUTION_QCIF_LS;
    }
    else if (nWidth == 144 && nHeight == 176)
    {
        return VIDEO_RESOLUTION_QCIF_PR;
    }
    else if (nWidth == 320 && nHeight == 240)
    {
        return VIDEO_RESOLUTION_QVGA_LS;
    }
    else if (nWidth == 240 && nHeight == 320)
    {
        return VIDEO_RESOLUTION_QVGA_PR;
    }
    else if (nWidth == 640 && nHeight == 480)
    {
        return VIDEO_RESOLUTION_VGA_LS;
    }
    else if (nWidth == 480 && nHeight == 640)
    {
        return VIDEO_RESOLUTION_VGA_PR;
    }
    else if (nWidth == 352 && nHeight == 288)
    {
        return VIDEO_RESOLUTION_CIF_LS;
    }
    else if (nWidth == 288 && nHeight == 352)
    {
        return VIDEO_RESOLUTION_CIF_PR;
    }
    else if (nWidth == 128 && nHeight == 96)
    {
        return VIDEO_RESOLUTION_SQCIF_LS;
    }
    else if (nWidth == 96 && nHeight == 128)
    {
        return VIDEO_RESOLUTION_SQCIF_PR;
    }
    else if (nWidth == 352 && nHeight == 240)
    {
        return VIDEO_RESOLUTION_SIF_LS;
    }
    else if (nWidth == 240 && nHeight == 352)
    {
        return VIDEO_RESOLUTION_SIF_PR;
    }
    else if (nWidth == 1280 && nHeight == 720)
    {
        return VIDEO_RESOLUTION_HD_LS;
    }
    else if (nWidth == 720 && nHeight == 1280)
    {
        return VIDEO_RESOLUTION_HD_PR;
    }
    else if (nWidth == 1920 && nHeight == 1080)
    {
        return VIDEO_RESOLUTION_FHD_LS;
    }
    else if (nWidth == 1080 && nHeight == 1920)
    {
        return VIDEO_RESOLUTION_FHD_PR;
    }
    else
    {
        return VIDEO_RESOLUTION_INVALID;
    }
}

/* TODO_MEDIA later sprop
PRIVATE
const IMS_CHAR* VideoProfileConfigurer::GetAvcSpropParameterSets(
        IN VIDEO_RESOLUTION eResolution, IN VIDEO_PROFILE_AVC eProfileId, IN IMS_SINT32 nLevel)
{
    IMS_UINT32 nProfile;
    IMS_UINT32 nWidth = 0;
    IMS_UINT32 nHeight = 0;

    IMS_MEM_Memset(m_strAvcSpropParameterSet, 0, sizeof(m_strAvcSpropParameterSet));

    GetWidthHeightFromResolution(eResolution, &nWidth, &nHeight);

    if (nWidth == 0 || nHeight == 0)
    {
        IMS_TRACE_E(0, "GetAvcSpropParameterSets INVALID Resolution[%d]", eResolution, 0, 0);
        return m_strAvcSpropParameterSet;
    }

    switch (eProfileId)
    {
        case AVC_PROFILE_CB:
            nProfile = (IMS_UINT32)MMPF_AVC_PROFILE_CONSTRAINED_BASELINE;
            break;
        case AVC_PROFILE_B:
            nProfile = (IMS_UINT32)MMPF_AVC_PROFILE_BASELINE;
            break;
        case AVC_PROFILE_M:
        case AVC_PROFILE_E:
            nProfile = (IMS_UINT32)MMPF_AVC_PROFILE_MAIN;
            break;
        case AVC_PROFILE_H:
            nProfile = (IMS_UINT32)MMPF_AVC_PROFILE_HIGH;
            break;
        default:
            nProfile = (IMS_UINT32)MMPF_AVC_PROFILE_BASELINE;
            break;
    }

    MMPFBoardConfigInfo::GetSpropParameterSet(
            nWidth, nHeight, nProfile, (IMS_UINT32)nLevel, m_strAvcSpropParameterSet);

    return m_strAvcSpropParameterSet;
}
*/

PUBLIC GLOBAL VIDEO_PROFILE_AVC VideoProfileConfigurer::GetAvcProfileFromProfileLevelId(
        const AString& strProfileLevelId)
{
    // Table 5.  Combinations of profile_idc and profile-iop (RFC 6184)
    //      Profile   profile_idc         profile-iop
    //                (hexadecimal)       (binary)
    //
    //        CB              42 (B)       x1xx0000
    //               same as: 4D (M)       1xxx0000
    //               same as: 58 (E)       11xx0000
    //        B               42 (B)       x0xx0000
    //               same as: 58 (E)       10xx0000
    //        M               4D (M)       0x0x0000
    //        E               58           00xx0000
    //        H               64           00000000

    if (strProfileLevelId.GetLength() < 3)
    {
        return AVC_PROFILE_NOT_USED;
    }

    IMS_UINT32 nProfileIop = 0;
    VIDEO_PROFILE_AVC nReProfile = AVC_PROFILE_NOT_USED;

    if ((IMS_UINT32)strProfileLevelId[2] >= (IMS_UINT32)'0' &&
            (IMS_UINT32)strProfileLevelId[2] <= (IMS_UINT32)'9')
    {
        nProfileIop = (IMS_UINT32)(strProfileLevelId[2] - '0');
    }
    else if ((IMS_UINT32)strProfileLevelId[2] >= (IMS_UINT32)'A' &&
            (IMS_UINT32)strProfileLevelId[2] <= (IMS_UINT32)'F')
    {
        nProfileIop = (IMS_UINT32)(strProfileLevelId[2] - 'A' + 10);
    }
    else if ((IMS_UINT32)strProfileLevelId[2] >= (IMS_UINT32)'a' &&
            (IMS_UINT32)strProfileLevelId[2] <= (IMS_UINT32)'f')
    {
        nProfileIop = (IMS_UINT32)(strProfileLevelId[2] - 'a' + 10);
    }
    else
    {
        IMS_TRACE_E(0, "GetAvcProfileFromProfileLevelId - INVALID Profil-iop[%c]",
                (IMS_CHAR)strProfileLevelId[2], 0, 0);
    }

    IMS_TRACE_D("GetAvcProfileFromProfileLevelId - nProfileIop[%d]", nProfileIop, 0, 0);

    // baseline profile
    if ((IMS_UINT32)strProfileLevelId[0] == (IMS_UINT32)'4' &&
            (IMS_UINT32)strProfileLevelId[1] == (IMS_UINT32)'2')
    {
        nReProfile = ((nProfileIop & 0x04) != 0) ? AVC_PROFILE_CB : AVC_PROFILE_B;
    }
    // main profile
    else if ((IMS_UINT32)strProfileLevelId[0] == (IMS_UINT32)'4' &&
            (IMS_UINT32)strProfileLevelId[1] == (IMS_UINT32)'D')
    {
        nReProfile = (nProfileIop & 0x08) ? AVC_PROFILE_CB : AVC_PROFILE_M;
    }
    // extended profile
    else if ((IMS_UINT32)strProfileLevelId[0] == (IMS_UINT32)'5' &&
            (IMS_UINT32)strProfileLevelId[1] == (IMS_UINT32)'8')
    {
        if (nProfileIop & 0x0C)
        {
            nReProfile = AVC_PROFILE_CB;
        }
        else if ((nProfileIop & 0x08) != 0 && (nProfileIop & 0x04) == 0)
        {
            nReProfile = AVC_PROFILE_M;
        }
        else
        {
            nReProfile = AVC_PROFILE_E;
        }
    }
    // high profile
    else if ((IMS_UINT32)strProfileLevelId[0] == (IMS_UINT32)'6' &&
            (IMS_UINT32)strProfileLevelId[1] == (IMS_UINT32)'4')
    {
        nReProfile = AVC_PROFILE_H;
    }

    IMS_TRACE_D("GetAvcProfileFromProfileLevelId - nProfile[%d]", nReProfile, 0, 0);

    return nReProfile;
}

PUBLIC GLOBAL IMS_UINT32 VideoProfileConfigurer::GetAvcLevelFromProfileLevelId(
        const AString& strProfileLevelId)
{
    IMS_UINT32 nLevel = 12;

    if (strProfileLevelId.GetLength() < 6)
    {
        return nLevel;
    }

    // IMS_TRACE_D("GetAvcLevelFromProfileLevelId - strProfileLevelId[%s]",
    // strProfileLevelId.GetStr(),0,0);
    if ((IMS_UINT32)strProfileLevelId[4] >= (IMS_UINT32)'A' &&
            (IMS_UINT32)strProfileLevelId[4] <= (IMS_UINT32)'F')
    {
        nLevel = ((IMS_UINT32)(strProfileLevelId[4] - 'A') + 10) * 16;
    }
    else if ((IMS_UINT32)strProfileLevelId[4] >= (IMS_UINT32)'a' &&
            (IMS_UINT32)strProfileLevelId[4] <= (IMS_UINT32)'f')
    {
        nLevel = ((IMS_UINT32)(strProfileLevelId[4] - 'a') + 10) * 16;
    }
    else
    {
        nLevel = (IMS_UINT32)(strProfileLevelId[4] - '0') * 16;
    }

    if ((IMS_UINT32)strProfileLevelId[5] >= (IMS_UINT32)'A' &&
            (IMS_UINT32)strProfileLevelId[5] <= (IMS_UINT32)'F')
    {
        nLevel += ((IMS_UINT32)(strProfileLevelId[5] - 'A') + 10);
    }
    else if ((IMS_UINT32)strProfileLevelId[5] >= (IMS_UINT32)'a' &&
            (IMS_UINT32)strProfileLevelId[5] <= (IMS_UINT32)'f')
    {
        nLevel += ((IMS_UINT32)(strProfileLevelId[5] - 'a') + 10);
    }
    else
    {
        nLevel += (IMS_UINT32)(strProfileLevelId[5] - '0');
    }

    IMS_TRACE_D("GetAvcLevelFromProfileLevelId - nLevel[%d]", nLevel, 0, 0);

    return nLevel;
}
/* // TODO_MEDIA video sprop
PUBLIC GLOBAL const IMS_CHAR* VideoProfileConfigurer::GetHevcSpropParameterSets(
        IN VIDEO_RESOLUTION eResolution, IN VIDEO_PROFILE_HEVC eProfileId, IN IMS_SINT32 nLevel)
{
    IMS_UINT32 nProfile;
    IMS_UINT32 nWidth = 0;
    IMS_UINT32 nHeight = 0;

    IMS_MEM_Memset(m_strHevcSpropParameterSet, 0, sizeof(m_strHevcSpropParameterSet));

    GetWidthHeightFromResolution(eResolution, &nWidth, &nHeight);

    if (nWidth == 0 || nHeight == 0)
    {
        IMS_TRACE_E(0, "GetHevcSpropParameterSets INVALID resolution(%d)", eResolution, 0, 0);

        return m_strHevcSpropParameterSet;
    }

    switch (eProfileId)
    {
        case HEVC_PROFILE_MAIN:
            nProfile = (IMS_UINT32)MMPF_HEVC_MAIN_LEVEL_4;
            break;
        case HEVC_PROFILE_MAIN10:
            nProfile = (IMS_UINT32)MMPF_HEVC_MAIN_LEVEL_4;
            break;
        default:
            nProfile = (IMS_UINT32)MMPF_HEVC_MAIN_LEVEL_4;
            break;
    }

    // MMPFBoardConfigInfo::GetHevcConfigFrameSet(
    //         nWidth, nHeight, nProfile, (IMS_UINT32)nLevel, m_strHevcSpropParameterSet);

    return m_strHevcSpropParameterSet;
}
*/
PUBLIC GLOBAL IMS_BOOL VideoProfileConfigurer::SetVideoRsRr(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pConfig)
{
    if (pVideoProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pVideoProfile->nBandwidthRr = pConfig->GetRrBandwidthBps();
    pVideoProfile->nBandwidthRs = pConfig->GetRsBandwidthBps();

    IMS_TRACE_I(" SetVideoRsRr(), Set RS[%d], RR[%d]", pVideoProfile->nBandwidthRs,
            pVideoProfile->nBandwidthRr, 0);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL VideoProfileConfigurer::MakeNegotiatedBandwidth(
        IN VideoConfiguration* pConfig, IN VideoProfile* pSrcProfile, IN VideoProfile* pDestProfile,
        IN IMS_BOOL bIsOfferReceived, IN IMS_SINT32 nAsValueOfNegoticatedCodec,
        OUT VideoProfile* pNegotiatedProfile)
{
    if (bIsOfferReceived == IMS_FALSE)
    {
        // MO's Bandwidth Setting
        // 1. Set AS Value
        // Exception Handling (b= AS line is not included in Answer SDP)

        pNegotiatedProfile->nBandwidthAs = (pDestProfile->nBandwidthAs > 0)
                ? pDestProfile->nBandwidthAs
                : pSrcProfile->nBandwidthAs;

        // 2. Set RS/RR Value
        // 2.1 Exception Handling (b=RS/RR line is not included in Answer SDP)
        if (pNegotiatedProfile->nBandwidthRs < 0 || pNegotiatedProfile->nBandwidthRr < 0)
        {
            pNegotiatedProfile->nBandwidthRs = pSrcProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pSrcProfile->nBandwidthRr;

            IMS_TRACE_D("MakeNegotiatedBandwidth() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
                    pNegotiatedProfile->nBandwidthAs, pNegotiatedProfile->nBandwidthRs,
                    pNegotiatedProfile->nBandwidthRr);

            return IMS_TRUE;
        }

        // 2.2 Normal Case
        // if RS/RR is used for RTCP Nego value
        if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE)
        {
            pNegotiatedProfile->nBandwidthRs = pDestProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pDestProfile->nBandwidthRr;
        }
        else
        {
            pNegotiatedProfile->nBandwidthRs = pSrcProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pSrcProfile->nBandwidthRr;
        }
    }
    else
    {
        // MT's Bandwidth Setting
        pNegotiatedProfile->nBandwidthAs = pSrcProfile->nBandwidthAs;
        // 1. Set Negotiated AS Value
        if (nAsValueOfNegoticatedCodec > 0)
        {
            pNegotiatedProfile->nBandwidthAs = nAsValueOfNegoticatedCodec;

            // if GetBandwidthNegoOption is BW_OPTION_NEGOTIATED_VALUE, use lower AS value
            if (pConfig->GetBandwidthNegoOption() ==
                            MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE &&
                    nAsValueOfNegoticatedCodec > pDestProfile->nBandwidthAs &&
                    pDestProfile->nBandwidthAs > 0)
            {
                pNegotiatedProfile->nBandwidthAs = pDestProfile->nBandwidthAs;
            }
        }

        if (pDestProfile->nBandwidthAs > pSrcProfile->nBandwidthAs)
        {
            pNegotiatedProfile->nBandwidthAs = pSrcProfile->nBandwidthAs;
        }
        else
        {
            pNegotiatedProfile->nBandwidthAs = pDestProfile->nBandwidthAs;
        }

        // 3. Set RS/RR Value
        if (pDestProfile->eDirection != MEDIA_DIRECTION_SEND_RECEIVE &&
                pDestProfile->eDirection != MEDIA_DIRECTION_RECEIVE &&
                pDestProfile->eDirection != MEDIA_DIRECTION_SEND)
        {
            // 3.1 Hold Case
            SetVideoRsRr(pNegotiatedProfile, pConfig);
        }
        else
        {
            // 3.2 Active Call Case
            // 3.2.1 Exception Handling (b=RS/RR line is not included in Answer SDP)
            if (pNegotiatedProfile->nBandwidthRs < 0 || pNegotiatedProfile->nBandwidthRr < 0)
            {
                pNegotiatedProfile->nBandwidthRs = pSrcProfile->nBandwidthRs;
                pNegotiatedProfile->nBandwidthRr = pSrcProfile->nBandwidthRr;

                IMS_TRACE_D("MakeNegotiatedBandwidth() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
                        pNegotiatedProfile->nBandwidthAs, pNegotiatedProfile->nBandwidthRs,
                        pNegotiatedProfile->nBandwidthRr);

                return IMS_TRUE;
            }

            // 3.2.2 Normal Case
            if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE)
            {
                // only use rtcp when rtcp state is enable
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
    }

    IMS_TRACE_D("MakeNegotiatedBandwidth() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
            pNegotiatedProfile->nBandwidthAs, pNegotiatedProfile->nBandwidthRs,
            pNegotiatedProfile->nBandwidthRr);

    return IMS_TRUE;
}
