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

#include "MediaManager.h"
#include "MediaResourceManager.h"
#include "config/CodecAvcConfig.h"
#include "config/CodecHevcConfig.h"
#include "config/VideoConfiguration.h"
#include "video/VideoProfileUtil.h"

__IMS_TRACE_TAG_USER_DECL__("MED.PC");

PUBLIC GLOBAL VideoProfile* VideoProfileUtil::CreateProfile(
        IN MediaEnvironment* pEnvironment, IN VideoConfiguration* pConfig, IN IMS_SINT32 nSlotId)
{
    IMS_SINT32 nMaxAS = -1;
    IMS_SINT32 nMaxFramerate = -1;

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

    MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr == IMS_NULL)
    {
        return IMS_NULL;
    }

    VideoProfile* pVideoProfile = new VideoProfile();
    pVideoProfile->objIpAddress = pEnvironment->pIService->GetIpAddress();

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

    IMS_TRACE_D("CreateProfile() objIpAddress[%s], port[%d]",
            pVideoProfile->objIpAddress.ToCharString(), pVideoProfile->nDataPort, 0);

    if (pConfig->IsVideoAvpfEnabled() && !pConfig->IsAvpfCapabilityNegotiationEnabled())
    {
        pVideoProfile->strTransportType = "RTP/AVPF";
    }
    else
    {
        pVideoProfile->strTransportType = "RTP/AVP";
    }

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

        if (pCodecConfig->GetCodec() == ImsCodec::VIDEO_NONE)
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

            pAvcFmtp->nFrameRate = pAvcConfig->GetFramerate();
            pAvcFmtp->eResolution = GetResolutionFromWidthHeight(
                    pAvcConfig->GetResolutionWidth(), pAvcConfig->GetResolutionHeight());
            pAvcFmtp->nBitrate = pAvcConfig->GetBitrate();
            pAvcFmtp->nAs = pConfig->GetAsBandwidthKbps();

            pAvcFmtp->nProfile = GetAvcProfileFromProfileLevelId(pAvcConfig->GetProfileLevelId());
            pAvcFmtp->nLevel = GetAvcLevelFromProfileLevelId(pAvcConfig->GetProfileLevelId());

            pbAvc4SpropParameterSets = pAvcConfig->GetSpropParameterSets().GetStr();

            /** TODO: later sprop need to find a way to get SpropPramaterSets
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

            if (pAvcConfig->GetProfileLevelId().GetLength() != 0)
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

            if (pAvcConfig->GetImageAttr().GetLength() != 0)
            {
                pAvcPayload->bIncludeImageAttr = IMS_TRUE;
                pAvcPayload->strImageAttr = pAvcConfig->GetImageAttr();
            }
            else if (pAvcConfig->GetFrameSize().GetLength() != 0)
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

                if (pConfig->IsVideoAvpfNackEnabled() == IMS_TRUE)
                {
                    pAvcPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                }

                if (pConfig->IsVideoAvpfTmmbrEnabled() == IMS_TRUE)
                {
                    pAvcPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                    pAvcPayload->objRtcpFbAttr.nTmmbrSmaxPr = 40;
                }

                if (pConfig->IsVideoAvpfPliEnabled() == IMS_TRUE)
                {
                    pAvcPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                }

                if (pConfig->IsVideoAvpfFirEnabled() == IMS_TRUE)
                {
                    pAvcPayload->objRtcpFbAttr.bFirSupported = IMS_TRUE;
                }

                IMS_TRACE_D("CreateProfile() AVPF. bNACK[%d], bTMMBR[%d], bPLI[%d]",
                        pAvcPayload->objRtcpFbAttr.bNackSupported,
                        pAvcPayload->objRtcpFbAttr.bTmmbrSupported,
                        pAvcPayload->objRtcpFbAttr.bPliSupported);
                IMS_TRACE_D("CreateProfile() AVPF. bFIR[%d], bTRR_Int[%d], nTrr-int[%d]",
                        pAvcPayload->objRtcpFbAttr.bFirSupported,
                        pAvcPayload->objRtcpFbAttr.bTrrSupported,
                        pAvcPayload->objRtcpFbAttr.nTrrInt);

                if (pAvcPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                {
                    IMS_TRACE_D("CreateProfile() TMMBR Supported", 0, 0, 0);
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

            IMS_TRACE_D("CreateProfile() add payload(%d) - %s", i,
                    ImsCodec::CodecToString(pAvcConfig->GetCodec()), 0);
        }
        else if (pCodecConfig->GetCodec() == ImsCodec::VIDEO_HEVC)
        {
            CodecHevcConfig* pHevcConfig = reinterpret_cast<CodecHevcConfig*>(pCodecConfig);

            VideoProfile::HevcFmtp* pHevcFmtp = new VideoProfile::HevcFmtp();

            pHevcFmtp->nFrameRate = pHevcConfig->GetFramerate();
            pHevcFmtp->eResolution = GetResolutionFromWidthHeight(
                    pHevcConfig->GetResolutionWidth(), pHevcConfig->GetResolutionHeight());
            pHevcFmtp->nBitrate = pHevcConfig->GetBitrate();

            pHevcFmtp->nAs = pConfig->GetAsBandwidthKbps();

            if (pHevcConfig->GetHevcProfile() != -1)
            {
                pHevcFmtp->nProfile =
                        static_cast<VIDEO_PROFILE_HEVC>(pHevcConfig->GetHevcProfile());
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

            if (pHevcConfig->GetSpropParameterSets().GetLength() != 0)
            {
                pHevcFmtp->strSpropParam = pHevcConfig->GetSpropParameterSets();
                pHevcFmtp->bShow_SpropParam = IMS_TRUE;
            }

            VideoProfile::Payload* pHevcPayload = new VideoProfile::Payload();

            pHevcPayload->SetRtpMap(pHevcConfig->GetPayloadType(),
                    ImsCodec::CodecToString(pHevcConfig->GetCodec()),
                    pConfig->GetVideoSamplingRate(), pHevcConfig->GetChannel());
            pHevcPayload->pFmtp = reinterpret_cast<void*>(pHevcFmtp);

            if (pHevcConfig->GetImageAttr().GetLength() != 0)
            {
                pHevcPayload->bIncludeImageAttr = IMS_TRUE;
            }
            else if (pHevcConfig->GetFrameSize().GetLength() != 0)
            {
                pHevcPayload->bIncludeFrameSize = IMS_TRUE;
            }

            if (pConfig->IsVideoAvpfEnabled() == IMS_TRUE)
            {
                if (pConfig->IsVideoAvpfNackEnabled() == IMS_TRUE)
                {
                    pHevcPayload->objRtcpFbAttr.bNackSupported = IMS_TRUE;
                }

                if (pConfig->IsVideoAvpfTmmbrEnabled() == IMS_TRUE)
                {
                    pHevcPayload->objRtcpFbAttr.bTmmbrSupported = IMS_TRUE;
                    pHevcPayload->objRtcpFbAttr.nTmmbrSmaxPr = 40;
                }

                if (pConfig->IsVideoAvpfPliEnabled() == IMS_TRUE)
                {
                    pHevcPayload->objRtcpFbAttr.bPliSupported = IMS_TRUE;
                }

                if (pConfig->IsVideoAvpfFirEnabled() == IMS_TRUE)
                {
                    pHevcPayload->objRtcpFbAttr.bFirSupported = IMS_TRUE;
                }

                IMS_TRACE_D("CreateProfile() AVPF. bNACK[%d], bTMMBR[%d], bPLI[%d]",
                        pHevcPayload->objRtcpFbAttr.bNackSupported,
                        pHevcPayload->objRtcpFbAttr.bTmmbrSupported,
                        pHevcPayload->objRtcpFbAttr.bPliSupported);
                IMS_TRACE_D("CreateProfile() AVPF. bFIR[%d]",
                        pHevcPayload->objRtcpFbAttr.bFirSupported, 0, 0);

                if (pHevcPayload->objRtcpFbAttr.bTmmbrSupported == IMS_TRUE)
                {
                    IMS_TRACE_D("CreateProfile() TMMBR Supported", 0, 0, 0);
                }
            }

            pVideoProfile->lstPayload.Append(pHevcPayload);

            if (pConfig->GetAsBandwidthKbps() > nMaxAS)
            {
                nMaxAS = pConfig->GetAsBandwidthKbps();
            }
            if (pHevcConfig->GetFramerate() > nMaxFramerate)
            {
                nMaxFramerate = pHevcConfig->GetFramerate();
            }

            IMS_TRACE_D("CreateProfile() add payload(%d) - %s", i,
                    ImsCodec::CodecToString(pHevcConfig->GetCodec()), 0);
        }
    }

    // 5. Setting direction, candidate, framerate, as/rr/rs
    pVideoProfile->eDirection = MEDIA_DIRECTION_SEND_RECEIVE;
    pVideoProfile->nFrameRate = nMaxFramerate;

    // 6. Setting CVO
    pVideoProfile->nCvoId = pConfig->GetCvoId();

    pVideoProfile->nBandwidthAs = pConfig->GetAsBandwidthKbps();
    SetVideoRsRr(pVideoProfile, pConfig);

    IMS_TRACE_D("CreateProfile - AS[%d], RR[%d], RS[%d]", pVideoProfile->nBandwidthAs,
            pVideoProfile->nBandwidthRr, pVideoProfile->nBandwidthRs);

    if (pConfig->IsVideoAvpfEnabled() == IMS_TRUE)
    {
        IMS_SINT32 nTCap = 0;
        IMS_SINT32 nAcap = 0;
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

            if (pConfig->IsVideoAvpfNackEnabled() == IMS_TRUE)
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
        else
        {
            pVideoProfile->bSupportCapaNegoForAvpf = IMS_FALSE;
        }
    }

    IMS_TRACE_D("CreateProfile - bSupportAvpf[%d], bSupportCapaNegoForAvpf[%d]",
            pVideoProfile->bSupportAvpf, pVideoProfile->bSupportCapaNegoForAvpf, 0);
    IMS_TRACE_D("CreateProfile - AS[%d], RR[%d], RS[%d]", pVideoProfile->nBandwidthAs,
            pVideoProfile->nBandwidthRr, pVideoProfile->nBandwidthRs);
    IMS_TRACE_D("CreateProfile - PayloadSize[%d]", pVideoProfile->lstPayload.GetSize(), 0, 0);

    return pVideoProfile;
}

PUBLIC GLOBAL IMS_BOOL VideoProfileUtil::UpdateVideoProfile(
        OUT VideoProfile* pVideoProfile, IN MediaEnvironment* pEnvironment)
{
    if (pEnvironment == IMS_NULL || pVideoProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_D("UpdateVideoProfile()", 0, 0, 0);

    pVideoProfile->objIpAddress = pEnvironment->pIService->GetIpAddress();

    return IMS_TRUE;
}

PUBLIC GLOBAL void VideoProfileUtil::GetWidthHeightFromResolution(
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

PUBLIC GLOBAL VIDEO_RESOLUTION VideoProfileUtil::GetResolutionFromWidthHeight(
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

PUBLIC GLOBAL VIDEO_PROFILE_AVC VideoProfileUtil::GetAvcProfileFromProfileLevelId(
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

PUBLIC GLOBAL IMS_UINT32 VideoProfileUtil::GetAvcLevelFromProfileLevelId(
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

PUBLIC GLOBAL IMS_BOOL VideoProfileUtil::SetVideoRsRr(
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

PUBLIC GLOBAL IMS_BOOL VideoProfileUtil::MakeNegotiatedBandwidth(IN VideoConfiguration* pConfig,
        IN VideoProfile* pLocalProfile, IN VideoProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        IN IMS_SINT32 nAsValueOfNegoticatedCodec, OUT VideoProfile* pNegotiatedProfile)
{
    if (bIsOfferReceived == IMS_FALSE)
    {
        // MO's Bandwidth Setting
        // 1. Set AS Value
        // Exception Handling (b= AS line is not included in Answer SDP)

        pNegotiatedProfile->nBandwidthAs = (pPeerProfile->nBandwidthAs > 0)
                ? pPeerProfile->nBandwidthAs
                : pLocalProfile->nBandwidthAs;

        // 2. Set RS/RR Value
        // 2.1 Exception Handling (b=RS/RR line is not included in Answer SDP)
        if (pNegotiatedProfile->nBandwidthRs < 0 || pNegotiatedProfile->nBandwidthRr < 0)
        {
            pNegotiatedProfile->nBandwidthRs = pLocalProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pLocalProfile->nBandwidthRr;

            IMS_TRACE_D("MakeNegotiatedBandwidth() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
                    pNegotiatedProfile->nBandwidthAs, pNegotiatedProfile->nBandwidthRs,
                    pNegotiatedProfile->nBandwidthRr);

            return IMS_TRUE;
        }

        // 2.2 Normal Case
        // if RS/RR is used for RTCP Nego value
        if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE)
        {
            pNegotiatedProfile->nBandwidthRs = pPeerProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pPeerProfile->nBandwidthRr;
        }
        else
        {
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
            // if GetBandwidthNegoOption is BW_OPTION_NEGOTIATED_VALUE, use lower AS value
            if (pConfig->GetBandwidthNegoOption() ==
                            MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE &&
                    nAsValueOfNegoticatedCodec > pPeerProfile->nBandwidthAs &&
                    pPeerProfile->nBandwidthAs > 0)
            {
                pNegotiatedProfile->nBandwidthAs = pPeerProfile->nBandwidthAs;
            }
            else
            {
                pNegotiatedProfile->nBandwidthAs = nAsValueOfNegoticatedCodec;
            }
        }
        else
        {
            pNegotiatedProfile->nBandwidthAs =
                    (pPeerProfile->nBandwidthAs > pLocalProfile->nBandwidthAs)
                    ? pLocalProfile->nBandwidthAs
                    : pPeerProfile->nBandwidthAs;
        }

        // 3. Set RS/RR Value
        if (pPeerProfile->eDirection != MEDIA_DIRECTION_SEND_RECEIVE &&
                pPeerProfile->eDirection != MEDIA_DIRECTION_RECEIVE &&
                pPeerProfile->eDirection != MEDIA_DIRECTION_SEND)
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
                pNegotiatedProfile->nBandwidthRs = pLocalProfile->nBandwidthRs;
                pNegotiatedProfile->nBandwidthRr = pLocalProfile->nBandwidthRr;

                IMS_TRACE_D("MakeNegotiatedBandwidth() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
                        pNegotiatedProfile->nBandwidthAs, pNegotiatedProfile->nBandwidthRs,
                        pNegotiatedProfile->nBandwidthRr);

                return IMS_TRUE;
            }

            // 3.2.2 Normal Case
            if (pConfig->GetBandwidthNegoOption() == MediaConfiguration::BW_OPTION_NEGOTIATED_VALUE)
            {
                // only use rtcp when rtcp state is enable
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
            pNegotiatedProfile->nBandwidthRr);

    return IMS_TRUE;
}
