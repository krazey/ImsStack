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
#include "offeranswer/SdpAvCodec.h"
#include "offeranswer/SdpMediaFormatParameter.h"
#include "offeranswer/SdpRtcpFeedback.h"

#include "MediaProfileUtil.h"
#include "config/VideoConfiguration.h"
#include "video/VideoNegoAvc.h"
#include "video/VideoNegoHevc.h"
#include "video/VideoProfileGenerator.h"

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

PUBLIC
VideoProfile* VideoProfileGenerator::SetProfile(IN MediaBaseProfile* pProfile,
        IN MediaConfiguration* pConfig, IN MediaEnvironment* pEnvironment, IN IMS_SINT32 nSlotId)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL || pEnvironment == IMS_NULL)
    {
        return IMS_NULL;
    }

    SetCommonProfile(pProfile, pConfig, pEnvironment, nSlotId);

    VideoProfile* pVideoProfile = static_cast<VideoProfile*>(pProfile);
    VideoConfiguration* pVideoConfig = static_cast<VideoConfiguration*>(pConfig);

    pVideoProfile->SetTransportType((pVideoConfig->IsVideoAvpfEnabled() &&
                                            !pVideoConfig->IsAvpfCapabilityNegotiationEnabled())
                    ? "RTP/AVPF"
                    : "RTP/AVP");

    pVideoProfile->SetCvoId(pVideoConfig->GetCvoId());

    if (pVideoConfig->IsVideoAvpfEnabled() == IMS_TRUE)
    {
        pVideoProfile->SetSupportAvpf(IMS_TRUE);

        IMS_SINT32 nTcap, nAcap = 0;

        nTcap = SetTransportCapability(pVideoProfile);
        nAcap = SetAttributeCapability(pVideoProfile, pVideoConfig);

        SetCapaNegoForAvpf(pVideoProfile, pVideoConfig->GetSdpOfferCapNegoForAvpf(), nTcap, nAcap);
    }

    SetMaxProfileFrameRate(static_cast<VideoProfile*>(pProfile));

    IMS_TRACE_D("SetProfile - SupportAvpf[%d], SupportCapaNegoForAvpf[%d]",
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

        IMS_TRACE_I("SetTransportCapability() - Tcap[%d][%s]", nTcap,
                pVideoProfile->GetCapaNego().GetMapTcap().GetValue(nTcap).GetStr(), 0);
    }

    return nTcap;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAttributeCapability(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig)
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

    IMS_TRACE_I("SetAttributeCapability() - Acap[%d]", nAcap, 0, 0);

    return nAcap;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAvpfTrr(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfTrrEnabled() == IMS_TRUE)
        {
            AString strTemp = AString::ConstEmpty();
            strTemp.Sprintf(
                    "%s %d", "rtcp-fb:* trr-int", pVideoConfig->GetRtcpIntervalOnHold() * 1000);
            pVideoProfile->GetCapaNego().GetMapAcap().SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetAvpfTrr() - Acap[%d][%s]", nAcap,
                    pVideoProfile->GetCapaNego().GetMapAcap().GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAvpfNack(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfNackEnabled() == IMS_TRUE)
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* nack";
            pVideoProfile->GetCapaNego().GetMapAcap().SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetAvpfNack() - Acap[%d][%s]", nAcap,
                    pVideoProfile->GetCapaNego().GetMapAcap().GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAvpfPli(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfPliEnabled() == IMS_TRUE)
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* nack pli";
            pVideoProfile->GetCapaNego().GetMapAcap().SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetAvpfPli() - Acap[%d][%s]", nAcap,
                    pVideoProfile->GetCapaNego().GetMapAcap().GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAvpfFir(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfFirEnabled() == IMS_TRUE)
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* ccm fir";
            pVideoProfile->GetCapaNego().GetMapAcap().SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetAvpfFir() - Acap[%d][%s]", nAcap,
                    pVideoProfile->GetCapaNego().GetMapAcap().GetValue(nAcap).GetStr(), 0);
        }
    }

    return nAcap;
}

PROTECTED IMS_SINT32 VideoProfileGenerator::SetAvpfTmmbr(
        OUT VideoProfile* pVideoProfile, IN VideoConfiguration* pVideoConfig, IN IMS_SINT32 nAcap)
{
    if (pVideoProfile != IMS_NULL && pVideoConfig != IMS_NULL)
    {
        if (pVideoConfig->IsVideoAvpfTmmbrEnabled() == IMS_TRUE)
        {
            AString strTemp = AString::ConstEmpty();
            strTemp = "rtcp-fb:* ccm tmmbr";
            pVideoProfile->GetCapaNego().GetMapAcap().SetValue(++nAcap, strTemp);

            IMS_TRACE_I("SetAvpfTmmbr() - Acap[%d][%s]", nAcap,
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
        return;
    }

    IMS_TRACE_I("SetCapaNegoForAvpf() - Acap[%d], Tcap[%d]", nTcap, nAcap, 0);

    pVideoProfile->SetSupportCapaNegoForAvpf(
            (nCapaNegoForAvpfOption > MediaConfiguration::CAPNEG_OFFER_NONE) ? IMS_TRUE
                                                                             : IMS_FALSE);

    if (pVideoProfile->IsCapaNegoForAvpfSupported() == IMS_TRUE)
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

        IMS_SINT32 nFrameRate =
                static_cast<VideoProfile::VideoFmtp*>(pPayload->GetFmtp())->GetFramerate();

        if (nFrameRate > nMaxFrameRate)
        {
            nMaxFrameRate = nFrameRate;
        }

        IMS_TRACE_I("SetMaxProfileFrameRate() - nMaxFrameRate[%d], payload framerate[%d]",
                nMaxFrameRate, nFrameRate, 0);
    }

    pVideoProfile->SetFrameRate(nMaxFrameRate);
}
