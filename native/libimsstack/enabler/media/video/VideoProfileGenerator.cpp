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

#include "config/CodecAvcConfig.h"
#include "config/CodecHevcConfig.h"
#include "config/ImsCodec.h"
#include "config/VideoConfiguration.h"
#include "video/VideoProfileGenerator.h"
#include "video/VideoProfileUtil.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC VideoProfileGenerator::VideoProfileGenerator() :
        MediaProfileGenerator(MEDIA_TYPE_VIDEO)
{
    IMS_TRACE_I("+VideoProfileGenerator()", 0, 0, 0);
}

PUBLIC VIRTUAL VideoProfileGenerator::~VideoProfileGenerator()
{
    IMS_TRACE_I("~VideoProfileGenerator()", 0, 0, 0);
}

PROTECTED
VideoProfile* VideoProfileGenerator::SetProfile(IN MediaBaseProfile* pProfile,
        IN MediaConfiguration* pConfig, MEDIA_SERVICE_TYPE /*eServiceType*/, IN IService* pIService,
        IN IMS_SINT32 nSlotId)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL || pIService == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetProfile(): invalid argument", 0, 0, 0);
        return IMS_NULL;
    }

    SetCommonProfile(pProfile, pConfig, pIService, nSlotId);

    VideoProfile* pVideoProfile = static_cast<VideoProfile*>(pProfile);
    const VideoConfiguration* pVideoConfig = static_cast<VideoConfiguration*>(pConfig);

    pVideoProfile->SetTransportType((pVideoConfig->IsVideoAvpfEnabled() &&
                                            !pVideoConfig->IsAvpfCapabilityNegotiationEnabled())
                    ? "RTP/AVPF"
                    : "RTP/AVP");

    pVideoProfile->SetCvoId(pVideoConfig->GetCvoId());

    if (pVideoConfig->IsVideoAvpfEnabled())
    {
        pVideoProfile->SetSupportAvpf(IMS_TRUE);

        IMS_SINT32 nTcap, nAcap = 0;

        nTcap = SetTransportCapability(pVideoProfile);
        nAcap = SetAttributeCapability(pVideoProfile, pVideoConfig);

        SetCapaNegoForAvpf(pVideoProfile, pVideoConfig->GetSdpOfferCapNegoForAvpf(), nTcap, nAcap);
    }

    SetMaxProfileFrameRate(static_cast<VideoProfile*>(pProfile));

    IMS_TRACE_D("SetProfile(): SupportAvpf[%d], SupportCapaNegoForAvpf[%d]",
            pVideoProfile->IsAvpfSupported(), pVideoProfile->IsCapaNegoForAvpfSupported(), 0);

    return pVideoProfile;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetTransportCapability(OUT VideoProfile* pVideoProfile)
{
    IMS_SINT32 nTcap = 0;

    if (pVideoProfile != IMS_NULL)
    {
        AString strTemp("RTP/AVPF");
        pVideoProfile->GetCapaNego().GetMapTcap().SetValue(++nTcap, strTemp);

        IMS_TRACE_I("SetTransportCapability(): Tcap[%d][%s]", nTcap,
                pVideoProfile->GetCapaNego().GetMapTcap().GetValue(nTcap).GetStr(), 0);
    }

    return nTcap;
}

PROTECTED
void VideoProfileGenerator::CreateCodecPayloads(IN MediaBaseProfile* pProfile, IN IMS_SINT32 nCodec,
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL || pCodecConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCodecPayloads(): invalid argument", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("CreateCodecPayloads(): codec[%s]", ImsCodec::CodecToString(nCodec), 0, 0);

    if (nCodec > ImsCodec::VIDEO_NONE && nCodec < ImsCodec::VIDEO_MAX)
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
}

PROTECTED VideoProfile::Payload* VideoProfileGenerator::CreateAvcPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateAvcPayload(): invalid argument", 0, 0, 0);
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateAvcPayload()", 0, 0, 0);

    auto pAvcConfig = static_cast<CodecAvcConfig*>(pCodecConfig);
    const VideoConfiguration* pVideoConfig = static_cast<VideoConfiguration*>(pConfig);
    auto pAvcFmtp = std::make_shared<VideoProfile::AvcFmtp>();

    if (pAvcConfig->GetProfileLevelId() == AString::ConstEmpty())
    {
        IMS_TRACE_D("CreateAvcPayload(): ProfileLevelId is empty, delete pAvcFmtp", 0, 0, 0);
        return IMS_NULL;
    }

    SetVideoCodecFmtp(pAvcConfig, pVideoConfig, pAvcFmtp);

    pAvcFmtp->SetProfile(
            VideoProfileUtil::GetAvcProfileFromProfileLevelId(pAvcConfig->GetProfileLevelId()));
    pAvcFmtp->SetLevel(
            VideoProfileUtil::GetAvcLevelFromProfileLevelId(pAvcConfig->GetProfileLevelId()));
    IMS_TRACE_I("CreateAvcPayload(): Profile[%d], Level[%d]", pAvcFmtp->GetProfile(),
            pAvcFmtp->GetLevel(), 0);

    const IMS_CHAR* pbAvc4SpropParameterSets;
    pbAvc4SpropParameterSets = pAvcConfig->GetSpropParameterSets().GetStr();

    /** TODO: later sprop need to find a way to get SpropPramaterSets
    pbAvc4SpropParameterSets = GetAvcSpropParameterSets(
            pAvcFmtp->GetResolution(), pAvcFmtp->GetProfile(), pAvcFmtp->GetLevel());
    */

    IMS_TRACE_I("CreateAvcPayload(): SpropParameterSets[%s]", pbAvc4SpropParameterSets, 0, 0);

    if (pAvcConfig->GetProfileLevelId().GetLength() != 0)
    {
        pAvcFmtp->SetProfileLevelId(pAvcConfig->GetProfileLevelId());
        pAvcFmtp->SetVisibleProfileLevelId(IMS_TRUE);
    }

    if (pAvcConfig->GetIncludeSpropParameterSets())
    {
        pAvcFmtp->SetSpropParam(pbAvc4SpropParameterSets);
        pAvcFmtp->SetVisibleSpropParam(IMS_TRUE);
    }

    VideoProfile::Payload* pAvcPayload = new VideoProfile::Payload();
    pAvcPayload->SetFmtp(pAvcFmtp);

    SetVideoCodecPayload(pAvcConfig, pVideoConfig, pAvcPayload);

    return pAvcPayload;
}

PROTECTED VideoProfile::Payload* VideoProfileGenerator::CreateHevcPayload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateHevcPayload(): invalid argument", 0, 0, 0);
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateHevcPayload()", 0, 0, 0);

    const CodecHevcConfig* pHevcConfig = reinterpret_cast<CodecHevcConfig*>(pCodecConfig);
    const VideoConfiguration* pVideoConfig = static_cast<VideoConfiguration*>(pConfig);
    auto pHevcFmtp = std::make_shared<VideoProfile::HevcFmtp>();

    SetVideoCodecFmtp(pHevcConfig, pVideoConfig, pHevcFmtp);

    if (pHevcConfig->GetHevcProfile() != -1)
    {
        pHevcFmtp->SetProfile(static_cast<VIDEO_PROFILE_HEVC>(pHevcConfig->GetHevcProfile()));
        pHevcFmtp->SetVisibleProfile(IMS_TRUE);
    }

    if (pHevcConfig->GetHevcLevel() != -1)
    {
        pHevcFmtp->SetLevel(pHevcConfig->GetHevcLevel());
        pHevcFmtp->SetVisibleLevel(IMS_TRUE);
    }

    if (pHevcConfig->GetSpropParameterSets().GetLength() != 0)
    {
        pHevcFmtp->SetSpropParam(pHevcConfig->GetSpropParameterSets());
        pHevcFmtp->SetVisibleSpropParam(IMS_TRUE);
    }

    VideoProfile::Payload* pHevcPayload = new VideoProfile::Payload();
    pHevcPayload->SetFmtp(pHevcFmtp);

    SetVideoCodecPayload(pHevcConfig, pVideoConfig, pHevcPayload);

    return pHevcPayload;
}

PROTECTED void VideoProfileGenerator::SetVideoCodecFmtp(IN const CodecVideoConfig* pCodecConfig,
        IN const VideoConfiguration* pVideoConfig,
        OUT std::shared_ptr<VideoProfile::VideoFmtp> pFmtp)
{
    if (pCodecConfig == IMS_NULL || pVideoConfig == IMS_NULL || pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetVideoCodecFmtp(): invalid argument", 0, 0, 0);
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
        pFmtp->SetVisiblePacketizationMode(IMS_TRUE);
    }

    IMS_TRACE_D("SetVideoCodecFmtp(): FrameRate[%d], Resolution[%d], Bitrate[%d]",
            pFmtp->GetFramerate(), pFmtp->GetResolution(), pFmtp->GetBitrate());
    IMS_TRACE_D("SetVideoCodecFmtp(): AS[%d], PacketizationMode[%d]", pFmtp->GetAs(),
            pFmtp->GetPacketizationMode(), 0);
}

PROTECTED void VideoProfileGenerator::SetVideoCodecPayload(IN const CodecVideoConfig* pCodecConfig,
        IN const VideoConfiguration* pVideoConfig, OUT VideoProfile::Payload* pPayload)
{
    if (pCodecConfig == IMS_NULL || pVideoConfig == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetVideoCodecPayload(): invalid argument", 0, 0, 0);
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

    if (pVideoConfig->IsVideoAvpfEnabled())
    {
        if (pVideoConfig->IsVideoAvpfTrrEnabled())
        {
            pPayload->GetRtcpFbAttr().SetTrrSupported(IMS_TRUE);
            pPayload->GetRtcpFbAttr().SetTrrInt(pVideoConfig->GetRtcpIntervalOnHold() * 1000);
        }

        if (pVideoConfig->IsVideoAvpfNackEnabled())
        {
            pPayload->GetRtcpFbAttr().SetNackSupported(IMS_TRUE);
        }

        if (pVideoConfig->IsVideoAvpfTmmbrEnabled())
        {
            pPayload->GetRtcpFbAttr().SetTmmbrSupported(IMS_TRUE);
            pPayload->GetRtcpFbAttr().SetTmmbrSmaxPr(40);
        }

        if (pVideoConfig->IsVideoAvpfPliEnabled())
        {
            pPayload->GetRtcpFbAttr().SetPliSupported(IMS_TRUE);
        }

        if (pVideoConfig->IsVideoAvpfFirEnabled())
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

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAttributeCapability(
        OUT VideoProfile* pVideoProfile, IN const VideoConfiguration* pVideoConfig)
{
    IMS_SINT32 nAcap = 0;

    if (pVideoProfile != IMS_NULL)
    {
        nAcap = SetAvpfTrr(pVideoProfile, pVideoConfig, nAcap);
        nAcap = SetAvpfNack(pVideoProfile, pVideoConfig, nAcap);
        nAcap = SetAvpfPli(pVideoProfile, pVideoConfig, nAcap);
        nAcap = SetAvpfFir(pVideoProfile, pVideoConfig, nAcap);
        nAcap = SetAvpfTmmbr(pVideoProfile, pVideoConfig, nAcap);
    }

    IMS_TRACE_I("SetAttributeCapability(): Acap[%d]", nAcap, 0, 0);

    return nAcap;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAvpfTrr(OUT VideoProfile* pVideoProfile,
        IN const VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfTrrEnabled())
        {
            AString strTemp = AString::ConstEmpty();
            strTemp.Sprintf(
                    "%s %d", "rtcp-fb:* trr-int", pVideoConfig->GetRtcpIntervalOnHold() * 1000);
            pVideoProfile->GetCapaNego().GetMapAcap().SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetAvpfTrr(): Acap[%d][%s]", nAcap,
                    pVideoProfile->GetCapaNego().GetMapAcap().GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAvpfNack(OUT VideoProfile* pVideoProfile,
        IN const VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfNackEnabled())
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* nack";
            pVideoProfile->GetCapaNego().GetMapAcap().SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetAvpfNack(): Acap[%d][%s]", nAcap,
                    pVideoProfile->GetCapaNego().GetMapAcap().GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAvpfPli(OUT VideoProfile* pVideoProfile,
        IN const VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfPliEnabled())
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* nack pli";
            pVideoProfile->GetCapaNego().GetMapAcap().SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetAvpfPli(): Acap[%d][%s]", nAcap,
                    pVideoProfile->GetCapaNego().GetMapAcap().GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAvpfFir(OUT VideoProfile* pVideoProfile,
        IN const VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfFirEnabled())
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* ccm fir";
            pVideoProfile->GetCapaNego().GetMapAcap().SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetAvpfFir(): Acap[%d][%s]", nAcap,
                    pVideoProfile->GetCapaNego().GetMapAcap().GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAvpfTmmbr(OUT VideoProfile* pVideoProfile,
        IN const VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfTmmbrEnabled())
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* ccm tmmbr";
            pVideoProfile->GetCapaNego().GetMapAcap().SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetAvpfTmmbr(): Acap[%d][%s]", nAcap,
                    pVideoProfile->GetCapaNego().GetMapAcap().GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PROTECTED void VideoProfileGenerator::SetCapaNegoForAvpf(OUT VideoProfile* pVideoProfile,
        IN IMS_SINT32 nCapaNegoForAvpfOption, IN IMS_SINT32 nTcap, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetCapaNegoForAvpf(): invalid argument", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("SetCapaNegoForAvpf(): Acap[%d], Tcap[%d]", nTcap, nAcap, 0);

    pVideoProfile->SetSupportCapaNegoForAvpf(
            (nCapaNegoForAvpfOption > MediaConfiguration::CAPNEG_OFFER_NONE) ? IMS_TRUE
                                                                             : IMS_FALSE);

    if (pVideoProfile->IsCapaNegoForAvpfSupported())
    {
        AString strPcfg = AString::ConstNull();
        strPcfg.Sprintf("t=%d", nTcap);

        if (nCapaNegoForAvpfOption == MediaConfiguration::CAPNEG_OFFER_WITHOUT_ACAP)
        {
            pVideoProfile->GetCapaNego().SetAttCapaInPcfg(IMS_FALSE);
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
            pVideoProfile->GetCapaNego().SetAttCapaInPcfg(IMS_TRUE);
        }
        pVideoProfile->GetCapaNego().GetListPcfg().Append(strPcfg);
    }
}

PROTECTED void VideoProfileGenerator::SetMaxProfileFrameRate(OUT VideoProfile* pVideoProfile)
{
    if (pVideoProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetMaxProfileFrameRate(): invalid argument", 0, 0, 0);
        return;
    }

    IMS_SINT32 nMaxFrameRate = -1;

    for (IMS_UINT32 i = 0; i < pVideoProfile->GetPayloadList().GetSize(); i++)
    {
        VideoProfile::Payload* pPayload = pVideoProfile->GetPayloadAt(i);

        if (pPayload == IMS_NULL || pPayload->GetFmtp() == IMS_NULL)
        {
            continue;
        }

        IMS_SINT32 nFrameRate = pPayload->GetFmtp()->GetFramerate();

        if (nFrameRate > nMaxFrameRate)
        {
            nMaxFrameRate = nFrameRate;
        }

        IMS_TRACE_I("SetMaxProfileFrameRate(): nMaxFrameRate[%d], payload framerate[%d]",
                nMaxFrameRate, nFrameRate, 0);
    }

    pVideoProfile->SetFrameRate(nMaxFrameRate);
}
