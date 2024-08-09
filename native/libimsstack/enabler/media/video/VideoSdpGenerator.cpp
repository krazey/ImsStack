// Copyright 2024 Google LLC

#include "ServiceTrace.h"
#include "offeranswer/SdpAvCodec.h"
#include "offeranswer/SdpMediaFormatParameter.h"
#include "offeranswer/SdpRtcpFeedback.h"

#include "video/VideoNegoAvc.h"
#include "video/VideoNegoHevc.h"
#include "video/VideoProfileUtil.h"
#include "video/VideoSdpGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC VideoSdpGenerator::VideoSdpGenerator() :
        SdpGenerator(MEDIA_TYPE_VIDEO)
{
    IMS_TRACE_I("+VideoSdpGenerator()", 0, 0, 0);
}

PUBLIC VIRTUAL VideoSdpGenerator::~VideoSdpGenerator()
{
    IMS_TRACE_I("~VideoSdpGenerator()", 0, 0, 0);
}

PUBLIC
IMS_BOOL VideoSdpGenerator::Generate(OUT ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pBaseProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pBaseProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    VideoProfile* pProfile = static_cast<VideoProfile*>(pBaseProfile);

    IMS_TRACE_I("Generate() - PayloadSize[%d], AS[%d]", pProfile->GetPayloadList().GetSize(),
            pProfile->GetBandwidthAs(), 0);

    pDescriptor->RemoveAttribute(SdpAttribute::ATTRIBUTE_ALL);
    ImsList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

    // make "c" line of media level if IP does not matched
    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->GetIpAddress()))
    {
        IMS_TRACE_D("Generate() - IP does not matched, SessionIP[%s], ProfileIP[%s]",
                pSessionDescriptor->GetLocalAddress().ToCharString(),
                pProfile->GetIpAddress().ToCharString(), 0);

        pDescriptor->SetConnectionAddress(pProfile->GetIpAddress().ToString());
    }

    // make "m" line
    // ------ "m=video xxxx RTP/AVP aaa bbb ccc ddd"
    AStringArray objVideoFormat;
    AString strPayloadNum;

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        VideoProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        strPayloadNum.Sprintf("%d", pPayload->GetRtpMap().GetPayloadNumber());
        objVideoFormat.AddElement(strPayloadNum);
    }

    // make SDPCapNeg attributes for initial SDP if AVPF is supported
    if (pProfile->GetTransportType().EqualsIgnoreCase("RTP/AVPF"))
    {
        pDescriptor->SetMediaDescription(SdpMedia::TYPE_VIDEO, pProfile->GetDataPort(),
                SdpMedia::TRANSPORT_RTP_AVPF, objVideoFormat);
    }
    else
    {
        pDescriptor->SetMediaDescription(SdpMedia::TYPE_VIDEO, pProfile->GetDataPort(),
                SdpMedia::TRANSPORT_RTP_AVP, objVideoFormat);
    }

    // Previously check all payload for RTCP-FB wildcard(*) attributes
    IMS_BOOL bTrrSupportedAll = IMS_TRUE;
    IMS_BOOL bNackSupportedAll = IMS_TRUE;
    IMS_BOOL bTmmbrSupportedAll = IMS_TRUE;
    IMS_BOOL bPliSupportedAll = IMS_TRUE;
    IMS_BOOL bFirSupportedAll = IMS_TRUE;

    if (pProfile->IsAvpfSupported() == IMS_TRUE)
    {
        for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
        {
            VideoProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
            if (pPayload != IMS_NULL)
            {
                if (pPayload->GetRtcpFbAttr().IsTrrSupported() == IMS_FALSE)
                {
                    bTrrSupportedAll = IMS_FALSE;
                }
                if (pPayload->GetRtcpFbAttr().IsNackSupported() == IMS_FALSE)
                {
                    bNackSupportedAll = IMS_FALSE;
                }
                if (pPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_FALSE)
                {
                    bTmmbrSupportedAll = IMS_FALSE;
                }
                if (pPayload->GetRtcpFbAttr().IsPliSupported() == IMS_FALSE)
                {
                    bPliSupportedAll = IMS_FALSE;
                }
                if (pPayload->GetRtcpFbAttr().IsFirSupported() == IMS_FALSE)
                {
                    bFirSupportedAll = IMS_FALSE;
                }
            }
        }
    }
    else
    {
        bTrrSupportedAll = IMS_FALSE;
        bNackSupportedAll = IMS_FALSE;
        bTmmbrSupportedAll = IMS_FALSE;
        bPliSupportedAll = IMS_FALSE;
        bFirSupportedAll = IMS_FALSE;
    }

    // make bandwidth
    // ------ "b=AS:xx"
    // ------ "b=AS:xx"
    // ------ "b=AS:xx"
    if (pProfile->GetBandwidthAs() > 0)
    {
        pDescriptor->AddBandwidth(SdpBandwidth::TYPE_AS, pProfile->GetBandwidthAs());

        if (pProfile->GetBandwidthRs() >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RS, pProfile->GetBandwidthRs());
        }

        if (pProfile->GetBandwidthRr() >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RR, pProfile->GetBandwidthRr());
        }
    }

    // make each payload
    // ------ "a=rtpmap:104 H264/16000/1"
    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AString strRtpmap, strFmtp;
        AString strResolutionAttr;
        VIDEO_RESOLUTION eResolution;

        VideoProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        // make "rtpmap"
        strRtpmap.Sprintf("%d %s/%d", pPayload->GetRtpMap().GetPayloadNumber(),
                pPayload->GetRtpMap().GetPayloadType().GetStr(),
                pPayload->GetRtpMap().GetSamplingRate());

        if (pPayload->GetRtpMap().GetChannel() > 0)
        {
            AString strChannel;
            strChannel.Sprintf("/%d", pPayload->GetRtpMap().GetChannel());
            strRtpmap.Append(strChannel);
        }

        IMS_TRACE_I("Generate() - Payload[%d], strRtpmap[%s]", i, strRtpmap.GetStr(), 0);

        // make "fmtp"
        // ------ "a=fmtp:104 profile-level-id=42C016; packetization-mode=1;
        // ----------  sprop-parameter-sets=Z0LAFukDwKMg,aM4G4g=="
        SdpAvCodec* pFormat = new SdpAvCodec();

        if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
        {
            VideoProfile::AvcFmtp* pAvcFmtp = (VideoProfile::AvcFmtp*)pPayload->GetFmtp();

            if (pAvcFmtp == IMS_NULL)
            {
                delete pFormat;
                continue;
            }

            strFmtp = VideoNegoAvc::SetSdpFmtpFromAvcFmtp(pAvcFmtp);

            eResolution = pAvcFmtp->GetResolution();
        }
        else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
        {
            VideoProfile::HevcFmtp* pHevcFmtp = (VideoProfile::HevcFmtp*)pPayload->GetFmtp();

            if (pHevcFmtp == IMS_NULL)
            {
                delete pFormat;
                continue;
            }

            strFmtp = VideoNegoHevc::SetSdpFmtpFromHevcFmtp(pHevcFmtp);

            eResolution = pHevcFmtp->GetResolution();
        }

        else
        {
            delete pFormat;
            continue;
        }

        if (strFmtp.GetLength() == 0)
        {
            strFmtp = AString::ConstNull();
        }

        AString strCompletedFmtp = AString::ConstNull();
        if (!strFmtp.IsNULL())
        {
            strCompletedFmtp.Sprintf("%d ", pPayload->GetRtpMap().GetPayloadNumber());
            strCompletedFmtp.Append(strFmtp);
        }
        if (pFormat == IMS_NULL)
        {
            continue;
        }
        if (pFormat->SetParameters(strRtpmap, strCompletedFmtp) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "Generate() SetParameters() Fail. strRtpmap[%s], strFmtp[%s]",
                    strRtpmap.GetStr(), strCompletedFmtp.GetStr(), 0);
        }

        // make "image attribute"
        if (pPayload->IsImageAttrIncluded() == IMS_TRUE)
        {
            if (MakeImageAttributeLine(
                        pPayload->GetRtpMap().GetPayloadNumber(), eResolution, strResolutionAttr))
            {
                pDescriptor->AddAttribute(SdpAttribute::IMAGEATTR, strResolutionAttr);
            }
        }

        // make "framesize"
        if (pPayload->IsFrameSizeIncluded() == IMS_TRUE)
        {
            if (MakeFrameSizeLine(
                        pPayload->GetRtpMap().GetPayloadNumber(), eResolution, strResolutionAttr))
            {
                pDescriptor->AddAttribute(SdpAttribute::FRAMESIZE, strResolutionAttr);
            }
        }

        // make "rtcp-fb"
        if ((pProfile->IsAvpfSupported() == IMS_TRUE) &&
                ((pProfile->IsCapaNegoForAvpfSupported() == IMS_FALSE) ||
                        (pProfile->IsCapaNegoForAvpfSupported() == IMS_TRUE &&
                                pProfile->GetCapaNego().IsAttCapaInPcfg() == IMS_FALSE)))
        {
            IMS_SINT32 nPayloadNumForRtcpFb = -1;

            // TRR-INT
            if (bTrrSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bTrrSupportedAll == IMS_FALSE &&
                    pPayload->GetRtcpFbAttr().IsTrrSupported() == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                AString strTemp = "";
                SdpRtcpFeedback* pTrr_IntAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);

                pTrr_IntAttr->SetType("trr-int");

                strTemp.Sprintf("%d", pPayload->GetRtcpFbAttr().GetTrrInt());
                pTrr_IntAttr->SetParameter(strTemp);

                pFormat->AddExtraParameter(pTrr_IntAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // NACK
            if (bNackSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bNackSupportedAll == IMS_FALSE &&
                    pPayload->GetRtcpFbAttr().IsNackSupported() == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pNackAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pNackAttr->SetType("nack");
                pFormat->AddExtraParameter(pNackAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // PLI
            if (bPliSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bPliSupportedAll == IMS_FALSE &&
                    pPayload->GetRtcpFbAttr().IsPliSupported() == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pPliAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pPliAttr->SetType("nack");
                pPliAttr->SetParameter("pli");
                pFormat->AddExtraParameter(pPliAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // FIR
            if (bFirSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bFirSupportedAll == IMS_FALSE &&
                    pPayload->GetRtcpFbAttr().IsFirSupported() == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pFIRAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pFIRAttr->SetType("ccm");
                pFIRAttr->SetParameter("fir");
                pFormat->AddExtraParameter(pFIRAttr);
            }

            nPayloadNumForRtcpFb = -1;
            // TMMBR
            if (bTmmbrSupportedAll == IMS_TRUE && i == 0)
            {
                nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
            }
            else if (bTmmbrSupportedAll == IMS_FALSE &&
                    pPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_TRUE)
            {
                nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
            }

            if (nPayloadNumForRtcpFb != -1)
            {
                SdpRtcpFeedback* pTmmbrAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
                pTmmbrAttr->SetType("ccm");
                pTmmbrAttr->SetParameter("tmmbr");
                pFormat->AddExtraParameter(pTmmbrAttr);
            }
        }

        pDescriptor->SetMediaFormat(pFormat);

        delete pFormat;
    }

    // make direction
    pDescriptor->SetDirection(pProfile->GetDirection());

    // make framerate
    pDescriptor->AddAttributeInt(SdpAttribute::FRAMERATE, pProfile->GetFrameRate());

    // make CVO
    if (pProfile->GetCvoId() > 0)
    {
        AString strCvoAttribute;
        strCvoAttribute.Sprintf("%d urn:3gpp:video-orientation", pProfile->GetCvoId());
        pDescriptor->AddAttribute(SdpAttribute::ATTRIBUTE_OTHER, strCvoAttribute, "extmap");
    }

    // make Capa Nego Attribute
    if (pProfile->IsCapaNegoForAvpfSupported() == IMS_TRUE)
    {
        // add "ACFG" if it's a initial answer
        if (pProfile->GetCapaNego().GetAcfg().GetLength() > 0)
        {
            AString strAcfg;
            IMS_TRACE_D("Generate() - Negotiated Acfg [%s]",
                    pProfile->GetCapaNego().GetAcfg().GetStr(), 0, 0);
            strAcfg.Sprintf("%s", pProfile->GetCapaNego().GetAcfg().GetStr());
            pDescriptor->AddAttribute(SdpAttribute::ACFG, strAcfg);
        }

        IMS_TRACE_D("Generate() Support Avpf[%d], Transport Type[%s]", pProfile->IsAvpfSupported(),
                pProfile->GetTransportType().GetStr(), 0);

        if (pProfile->IsAvpfSupported() == IMS_TRUE &&
                pProfile->GetTransportType().Contains("AVPF") == IMS_FALSE)
        {
            // make tcap, acap, pcfg for capa nego offer...
            IMS_UINT32 i = 0;
            // AString strTcap = "1 RTP/AVPF";             // only support avpf profile
            AString strTcap = "";
            AString strAcap = "";
            AString strPcfg = "";

            IMS_TRACE_I("Generate() - Entered, PcfgSize[%d], TcapSize[%d], AcapSize[%d]",
                    pProfile->GetCapaNego().GetListPcfg().GetSize(),
                    pProfile->GetCapaNego().GetMapTcap().GetSize(),
                    pProfile->GetCapaNego().GetMapAcap().GetSize());

            for (i = 0; i < pProfile->GetCapaNego().GetMapTcap().GetSize(); i++)
            {
                strTcap = "";
                strTcap.Sprintf("%d %s", i + 1,
                        pProfile->GetCapaNego().GetMapTcap().GetValueAt(i).GetStr());
                pDescriptor->AddAttribute(SdpAttribute::TCAP, strTcap);
            }

            if (pProfile->GetCapaNego().IsAttCapaInPcfg() == IMS_TRUE)
            {
                for (i = 0; i < pProfile->GetCapaNego().GetMapAcap().GetSize(); i++)
                {
                    strAcap = "";
                    strAcap.Sprintf("%d %s", i + 1,
                            pProfile->GetCapaNego().GetMapAcap().GetValueAt(i).GetStr());
                    pDescriptor->AddAttribute(SdpAttribute::ACAP, strAcap);
                    IMS_TRACE_I("Generate() - Add strAcap : %s", strAcap.GetStr(), 0, 0);
                }
            }

            for (i = 0; i < pProfile->GetCapaNego().GetListPcfg().GetSize(); i++)
            {
                strPcfg = "";
                strPcfg.Sprintf(
                        "%d %s", i + 1, pProfile->GetCapaNego().GetListPcfg().GetAt(i).GetStr());
                pDescriptor->AddAttribute(SdpAttribute::PCFG, strPcfg);
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoSdpGenerator::MakeImageAttributeLine(
        IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId, OUT AString& strImageAttr)
{
    IMS_UINT32 nWidth, nHeight;

    if (GetWidthHeightFromResolutionId(eResolutionId, &nWidth, &nHeight) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    strImageAttr.Sprintf(
            "%d send [x=%d,y=%d] recv [x=%d,y=%d]", nPayloadType, nWidth, nHeight, nWidth, nHeight);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoSdpGenerator::MakeFrameSizeLine(
        IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId, OUT AString& strFrameSize)
{
    IMS_UINT32 nWidth, nHeight;

    if (GetWidthHeightFromResolutionId(eResolutionId, &nWidth, &nHeight) == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    strFrameSize.Sprintf("%d %d-%d", nPayloadType, nWidth, nHeight);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoSdpGenerator::GetWidthHeightFromResolutionId(
        IN VIDEO_RESOLUTION eResolutionId, OUT IMS_UINT32* pnWidth, OUT IMS_UINT32* pnHeight)
{
    IMS_TRACE_D("GetWidthHeightFromResolutionId() resolution ID [%d]", eResolutionId, 0, 0);
    switch (eResolutionId)
    {
        case VIDEO_RESOLUTION_QCIF_LS:
            *pnWidth = 176;
            *pnHeight = 144;
            break;
        case VIDEO_RESOLUTION_QCIF_PR:
            *pnWidth = 144;
            *pnHeight = 176;
            break;
        case VIDEO_RESOLUTION_QVGA_LS:
            *pnWidth = 320;
            *pnHeight = 240;
            break;
        case VIDEO_RESOLUTION_QVGA_PR:
            *pnWidth = 240;
            *pnHeight = 320;
            break;
        case VIDEO_RESOLUTION_VGA_LS:
            *pnWidth = 640;
            *pnHeight = 480;
            break;
        case VIDEO_RESOLUTION_VGA_PR:
            *pnWidth = 480;
            *pnHeight = 640;
            break;
        case VIDEO_RESOLUTION_CIF_LS:
            *pnWidth = 352;
            *pnHeight = 288;
            break;
        case VIDEO_RESOLUTION_CIF_PR:
            *pnWidth = 288;
            *pnHeight = 352;
            break;
        case VIDEO_RESOLUTION_SIF_PR:
            *pnWidth = 240;
            *pnHeight = 352;
            break;
        case VIDEO_RESOLUTION_SIF_LS:
            *pnWidth = 352;
            *pnHeight = 240;
            break;
        case VIDEO_RESOLUTION_SQCIF_LS:
            *pnWidth = 128;
            *pnHeight = 96;
            break;
        case VIDEO_RESOLUTION_SQCIF_PR:
            *pnWidth = 96;
            *pnHeight = 128;
            break;
        case VIDEO_RESOLUTION_HD_LS:
            *pnWidth = 1280;
            *pnHeight = 720;
            break;
        case VIDEO_RESOLUTION_HD_PR:
            *pnWidth = 720;
            *pnHeight = 1280;
            break;
        case VIDEO_RESOLUTION_FHD_LS:
            *pnWidth = 1920;
            *pnHeight = 1080;
            break;
        case VIDEO_RESOLUTION_FHD_PR:
            *pnWidth = 1080;
            *pnHeight = 1920;
            break;
        default:
            *pnWidth = 176;
            *pnHeight = 144;
            IMS_TRACE_E(0, "GetWidthHeightFromResolutionId() INVALID resolution ID", 0, 0, 0);

            return IMS_FALSE;
    }

    return IMS_TRUE;
}
