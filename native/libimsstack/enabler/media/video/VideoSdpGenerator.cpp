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
#include "offeranswer/SdpAvCodec.h"
#include "offeranswer/SdpMediaFormatParameter.h"
#include "offeranswer/SdpRtcpFeedback.h"

#include "video/VideoSdpGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC VideoSdpGenerator::VideoSdpGenerator() :
        MediaSdpGenerator(MEDIA_TYPE_VIDEO)
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
        IMS_TRACE_E(0, "Generate(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    GenerateCommonAttributes(pSessionDescriptor, pDescriptor, pBaseProfile);

    VideoProfile* pProfile = static_cast<VideoProfile*>(pBaseProfile);

    GeneratePayload(pDescriptor, pProfile);
    GenerateDirection(pDescriptor, pProfile);
    GenerateFrameRate(pDescriptor, pProfile);
    GenerateCvo(pDescriptor, pProfile);
    GenerateCapaNegoAttribute(pDescriptor, pProfile);
    return IMS_TRUE;
}

PROTECTED
void VideoSdpGenerator::GeneratePayload(
        OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GeneratePayload(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_BOOL bTrrSupportedAll = IMS_TRUE;
    IMS_BOOL bNackSupportedAll = IMS_TRUE;
    IMS_BOOL bPliSupportedAll = IMS_TRUE;
    IMS_BOOL bFirSupportedAll = IMS_TRUE;
    IMS_BOOL bTmmbrSupportedAll = IMS_TRUE;

    CheckRtcpFbWildCard(pProfile, bTrrSupportedAll, bNackSupportedAll, bPliSupportedAll,
            bFirSupportedAll, bTmmbrSupportedAll);

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AString strRtpMap = AString::ConstNull();
        AString strPayloadNum = AString::ConstNull();
        AString strFmtp = AString::ConstNull();

        VideoProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
        SdpAvCodec objSdpFormat;
        if (pPayload != IMS_NULL &&
                GenerateRtpMap(strRtpMap, strPayloadNum, pPayload->GetRtpMap()) &&
                GenerateFmtp(strFmtp, pPayload) &&
                GenerateCompletedFmtpRtpMap(strRtpMap, strPayloadNum, strFmtp, &objSdpFormat))
        {
            GenerateImageAttribute(pDescriptor, pPayload);
            GenerateFrameSize(pDescriptor, pPayload);
            GenerateRtcpFb(pProfile, bTrrSupportedAll, bNackSupportedAll, bPliSupportedAll,
                    bFirSupportedAll, bTmmbrSupportedAll, &objSdpFormat, i);

            pDescriptor->SetMediaFormat(&objSdpFormat);
        }
    }
}

PROTECTED
void VideoSdpGenerator::CheckRtcpFbWildCard(IN VideoProfile* pProfile,
        OUT IMS_BOOL& bTrrSupportedAll, OUT IMS_BOOL& bNackSupportedAll,
        OUT IMS_BOOL& bPliSupportedAll, OUT IMS_BOOL& bFirSupportedAll,
        OUT IMS_BOOL& bTmmbrSupportedAll)
{
    if (pProfile->IsAvpfSupported())
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
                if (pPayload->GetRtcpFbAttr().IsPliSupported() == IMS_FALSE)
                {
                    bPliSupportedAll = IMS_FALSE;
                }
                if (pPayload->GetRtcpFbAttr().IsFirSupported() == IMS_FALSE)
                {
                    bFirSupportedAll = IMS_FALSE;
                }
                if (pPayload->GetRtcpFbAttr().IsTmmbrSupported() == IMS_FALSE)
                {
                    bTmmbrSupportedAll = IMS_FALSE;
                }
            }
        }
    }
    else
    {
        bTrrSupportedAll = IMS_FALSE;
        bNackSupportedAll = IMS_FALSE;
        bPliSupportedAll = IMS_FALSE;
        bFirSupportedAll = IMS_FALSE;
        bTmmbrSupportedAll = IMS_FALSE;
    }

    IMS_TRACE_D("CheckRtcpFbWildCard() Trr[%d], Nack[%d]", bTrrSupportedAll, bNackSupportedAll, 0);
    IMS_TRACE_D("CheckRtcpFbWildCard() Pli[%d], Fir[%d], Tmmbr[%d]", bPliSupportedAll,
            bFirSupportedAll, bTmmbrSupportedAll);
}

PROTECTED IMS_BOOL VideoSdpGenerator::GenerateFmtp(
        OUT AString& strFmtp, IN VideoProfile::Payload* pPayload)
{
    if (pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateFmtp(): invalid payload", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"))
    {
        auto pAvcFmtp = std::static_pointer_cast<VideoProfile::AvcFmtp>(pPayload->GetFmtp());

        if (pAvcFmtp == IMS_NULL)
        {
            IMS_TRACE_E(0, "GenerateFmtp(): invalid fmtp", 0, 0, 0);
            return IMS_FALSE;
        }

        strFmtp = GenerateAvcFmtp(pAvcFmtp);
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"))
    {
        auto pHevcFmtp = std::static_pointer_cast<VideoProfile::HevcFmtp>(pPayload->GetFmtp());

        if (pHevcFmtp == IMS_NULL)
        {
            IMS_TRACE_E(0, "GenerateFmtp(): invalid fmtp", 0, 0, 0);
            return IMS_FALSE;
        }

        strFmtp = GenerateHevcFmtp(pHevcFmtp);
    }
    else
    {
        IMS_TRACE_E(0, "GenerateFmtp(): invalid codec", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("GenerateFmtp(): fmtp[%s]", strFmtp.GetStr(), 0, 0);
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL VideoSdpGenerator::GenerateCompletedFmtpRtpMap(IN const AString& strRtpMap,
        IN const AString& strPayloadNum, IN const AString& strFmtp, OUT SdpAvCodec* pFormat)
{
    if (strRtpMap.IsNULL() || strPayloadNum.IsNULL() || strFmtp.IsNULL())
    {
        IMS_TRACE_E(0, "GenerateCompletedFmtpRtpMap(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strCompletedRtpMap = AString::ConstNull();
    AString strCompletedFmtp = AString::ConstNull();

    strCompletedRtpMap.Sprintf("%s %s", strPayloadNum.GetStr(), strRtpMap.GetStr());
    strCompletedFmtp.Sprintf("%s %s", strPayloadNum.GetStr(), strFmtp.GetStr());

    return pFormat->SetParameters(strCompletedRtpMap, strCompletedFmtp);
}

PROTECTED
void VideoSdpGenerator::GenerateImageAttribute(
        OUT IMediaDescriptor* pDescriptor, IN VideoProfile::Payload* pPayload)
{
    if (pDescriptor == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateImageAttribute(): invalid arguments", 0, 0, 0);
        return;
    }

    AString strImageAttr = AString::ConstNull();
    VIDEO_RESOLUTION eResolution = pPayload->GetFmtp()->GetResolution();

    if (pPayload->IsImageAttrIncluded())
    {
        if (MakeImageAttributeLine(
                    pPayload->GetRtpMap().GetPayloadNumber(), eResolution, strImageAttr))
        {
            pDescriptor->AddAttribute(SdpAttribute::IMAGEATTR, strImageAttr);
            IMS_TRACE_D("GenerateImageAttribute(): [%s], ", strImageAttr.GetStr(), 0, 0);
        }
    }
}

PROTECTED
void VideoSdpGenerator::GenerateFrameSize(
        OUT IMediaDescriptor* pDescriptor, IN VideoProfile::Payload* pPayload)
{
    if (pDescriptor == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateFrameSize(): invalid arguments", 0, 0, 0);
        return;
    }

    AString strFrameSize = AString::ConstNull();
    VIDEO_RESOLUTION eResolution = pPayload->GetFmtp()->GetResolution();

    if (pPayload->IsFrameSizeIncluded())
    {
        if (MakeFrameSizeLine(pPayload->GetRtpMap().GetPayloadNumber(), eResolution, strFrameSize))
        {
            pDescriptor->AddAttribute(SdpAttribute::FRAMESIZE, strFrameSize);
            IMS_TRACE_D("GenerateFrameSize(): [%s], ", strFrameSize.GetStr(), 0, 0);
        }
    }
}

PROTECTED
void VideoSdpGenerator::GenerateRtcpFb(IN VideoProfile* pProfile, IN IMS_BOOL bTrrSupportedAll,
        IN IMS_BOOL bNackSupportedAll, IN IMS_BOOL bPliSupportedAll, IN IMS_BOOL bFirSupportedAll,
        IN IMS_BOOL bTmmbrSupportedAll, OUT SdpAvCodec* pFormat, IN IMS_UINT32 nPayloadIndex)
{
    if (pProfile == IMS_NULL || pFormat == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateRtcpFb(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("GenerateRtcpFb(): Avpf Support[%d], CapaNego Support[%d], AttCapa in Pcfg[%d]",
            pProfile->IsAvpfSupported(), pProfile->IsCapaNegoForAvpfSupported(),
            pProfile->GetCapaNego().IsAttCapaInPcfg());

    if ((pProfile->IsAvpfSupported()) &&
            ((pProfile->IsCapaNegoForAvpfSupported() == IMS_FALSE) ||
                    (pProfile->IsCapaNegoForAvpfSupported() &&
                            pProfile->GetCapaNego().IsAttCapaInPcfg() == IMS_FALSE)))
    {
        VideoProfile::Payload* pPayload = pProfile->GetPayloadAt(nPayloadIndex);

        GenerateRtcpFbTrrInt(pFormat, pPayload, bTrrSupportedAll, nPayloadIndex);
        GenerateRtcpFbNack(pFormat, pPayload, bNackSupportedAll, nPayloadIndex);
        GenerateRtcpFbPli(pFormat, pPayload, bPliSupportedAll, nPayloadIndex);
        GenerateRtcpFbFir(pFormat, pPayload, bFirSupportedAll, nPayloadIndex);
        GenerateRtcpFbTmmbr(pFormat, pPayload, bTmmbrSupportedAll, nPayloadIndex);
    }
}

PROTECTED
void VideoSdpGenerator::GenerateRtcpFbTrrInt(OUT SdpAvCodec* pFormat,
        IN VideoProfile::Payload* pPayload, IN IMS_BOOL bSupportedInAllPayload,
        IN IMS_UINT32 nPayloadIndex)
{
    if (pFormat == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateRtcpFbTrrInt(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nPayloadNumForRtcpFb = -1;

    if (bSupportedInAllPayload && nPayloadIndex == 0)
    {
        nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
    }
    else if (bSupportedInAllPayload == IMS_FALSE && pPayload->GetRtcpFbAttr().IsTrrSupported())
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

        IMS_TRACE_D("GenerateRtcpFbTrrInt(): payload[%d], %s", nPayloadNumForRtcpFb,
                strTemp.GetStr(), 0);
    }
}

PROTECTED
void VideoSdpGenerator::GenerateRtcpFbNack(OUT SdpAvCodec* pFormat,
        IN VideoProfile::Payload* pPayload, IN IMS_BOOL bSupportedInAllPayload,
        IN IMS_UINT32 nPayloadIndex)
{
    if (pFormat == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateRtcpFbNack(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nPayloadNumForRtcpFb = -1;

    if (bSupportedInAllPayload && nPayloadIndex == 0)
    {
        nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
    }
    else if (bSupportedInAllPayload == IMS_FALSE && pPayload->GetRtcpFbAttr().IsNackSupported())
    {
        nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
    }

    if (nPayloadNumForRtcpFb != -1)
    {
        SdpRtcpFeedback* pNackAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
        pNackAttr->SetType("nack");
        pFormat->AddExtraParameter(pNackAttr);

        IMS_TRACE_D("GenerateRtcpFbNack(): payload[%d]", nPayloadNumForRtcpFb, 0, 0);
    }
}

PROTECTED
void VideoSdpGenerator::GenerateRtcpFbPli(OUT SdpAvCodec* pFormat,
        IN VideoProfile::Payload* pPayload, IN IMS_BOOL bSupportedInAllPayload,
        IN IMS_UINT32 nPayloadIndex)
{
    if (pFormat == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateRtcpFbPli(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nPayloadNumForRtcpFb = -1;

    if (bSupportedInAllPayload && nPayloadIndex == 0)
    {
        nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
    }
    else if (bSupportedInAllPayload == IMS_FALSE && pPayload->GetRtcpFbAttr().IsPliSupported())
    {
        nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
    }

    if (nPayloadNumForRtcpFb != -1)
    {
        SdpRtcpFeedback* pPliAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
        pPliAttr->SetType("nack");
        pPliAttr->SetParameter("pli");
        pFormat->AddExtraParameter(pPliAttr);

        IMS_TRACE_D("GenerateRtcpFbPli(): payload[%d]", nPayloadNumForRtcpFb, 0, 0);
    }
}

PROTECTED
void VideoSdpGenerator::GenerateRtcpFbFir(OUT SdpAvCodec* pFormat,
        IN VideoProfile::Payload* pPayload, IN IMS_BOOL bSupportedInAllPayload,
        IN IMS_UINT32 nPayloadIndex)
{
    if (pFormat == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateRtcpFbFir(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nPayloadNumForRtcpFb = -1;

    if (bSupportedInAllPayload && nPayloadIndex == 0)
    {
        nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
    }
    else if (bSupportedInAllPayload == IMS_FALSE && pPayload->GetRtcpFbAttr().IsFirSupported())
    {
        nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
    }

    if (nPayloadNumForRtcpFb != -1)
    {
        SdpRtcpFeedback* pFIRAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
        pFIRAttr->SetType("ccm");
        pFIRAttr->SetParameter("fir");
        pFormat->AddExtraParameter(pFIRAttr);

        IMS_TRACE_D("GenerateRtcpFbFir(): payload[%d]", nPayloadNumForRtcpFb, 0, 0);
    }
}

PROTECTED
void VideoSdpGenerator::GenerateRtcpFbTmmbr(OUT SdpAvCodec* pFormat,
        IN VideoProfile::Payload* pPayload, IN IMS_BOOL bSupportedInAllPayload,
        IN IMS_UINT32 nPayloadIndex)
{
    if (pFormat == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateRtcpFbTmmbr(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nPayloadNumForRtcpFb = -1;

    if (bSupportedInAllPayload && nPayloadIndex == 0)
    {
        nPayloadNumForRtcpFb = SdpMediaFormatParameter::PT_WILDCARD;
    }
    else if (bSupportedInAllPayload == IMS_FALSE && pPayload->GetRtcpFbAttr().IsTmmbrSupported())
    {
        nPayloadNumForRtcpFb = (IMS_SINT32)pPayload->GetRtpMap().GetPayloadNumber();
    }

    if (nPayloadNumForRtcpFb != -1)
    {
        SdpRtcpFeedback* pTmmbrAttr = new SdpRtcpFeedback(nPayloadNumForRtcpFb);
        pTmmbrAttr->SetType("ccm");
        pTmmbrAttr->SetParameter("tmmbr");
        pFormat->AddExtraParameter(pTmmbrAttr);

        IMS_TRACE_D("GenerateRtcpFbTmmbr(): payload[%d]", nPayloadNumForRtcpFb, 0, 0);
    }
}

PROTECTED
void VideoSdpGenerator::GenerateFrameRate(
        OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateFrameRate(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nFrameRate = pProfile->GetFrameRate();

    if (nFrameRate > 0)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::FRAMERATE, nFrameRate);
        IMS_TRACE_D("GenerateFrameRate(): frameRate[%d]", nFrameRate, 0, 0);
    }
}

PROTECTED
void VideoSdpGenerator::GenerateCvo(OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateCvo(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nCvoId = pProfile->GetCvoId();

    if (nCvoId > 0)
    {
        AString strCvoAttribute;
        strCvoAttribute.Sprintf("%d urn:3gpp:video-orientation", nCvoId);
        pDescriptor->AddAttribute(SdpAttribute::ATTRIBUTE_OTHER, strCvoAttribute, "extmap");

        IMS_TRACE_D("GenerateCvo(): cvoId[%d]", nCvoId, 0, 0);
    }
}

PROTECTED
void VideoSdpGenerator::GenerateCapaNegoAttribute(
        OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateCapaNegoAttribute(): invalid arguments", 0, 0, 0);
        return;
    }

    if (!pProfile->IsCapaNegoForAvpfSupported())
    {
        IMS_TRACE_I("GenerateCapaNegoAttribute(): capa nego is not supported", 0, 0, 0);
        return;
    }

    MediaBaseProfile::CapaNego objCapaNego = pProfile->GetCapaNego();
    IMS_BOOL bAvpfSupport = pProfile->IsAvpfSupported();
    AString strTransportType = pProfile->GetTransportType();

    IMS_TRACE_D("GenerateCapaNegoAttribute() Support Avpf[%d], Transport Type[%s]", bAvpfSupport,
            strTransportType.GetStr(), 0);

    GenerateAcfg(pDescriptor, objCapaNego);

    if (bAvpfSupport && !strTransportType.Contains("AVPF"))
    {
        GenerateTcap(pDescriptor, objCapaNego);
        GenerateAcap(pDescriptor, objCapaNego);
        GeneratePcfg(pDescriptor, objCapaNego);
    }
}

PROTECTED
void VideoSdpGenerator::GenerateAcfg(
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile::CapaNego& objCapaNego)
{
    if (pDescriptor == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateAcfg(): invalid argument", 0, 0, 0);
        return;
    }

    AString strAcfg = objCapaNego.GetAcfg();

    if (strAcfg.GetLength() > 0)
    {
        AString strTemp;
        strTemp.Sprintf("%s", strAcfg.GetStr());
        pDescriptor->AddAttribute(SdpAttribute::ACFG, strTemp);

        IMS_TRACE_D("GenerateAcfg(): Add acfg[%s]", strTemp.GetStr(), 0, 0);
    }
}

PROTECTED
void VideoSdpGenerator::GenerateTcap(
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile::CapaNego& objCapaNego)
{
    if (pDescriptor == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateTcap(): invalid argument", 0, 0, 0);
        return;
    }

    ImsMap<IMS_SINT32, AString> objTcap = objCapaNego.GetMapTcap();

    for (IMS_UINT32 i = 0; i < objTcap.GetSize(); i++)
    {
        AString strTemp = "";
        strTemp.Sprintf("%d %s", i + 1, objTcap.GetValueAt(i).GetStr());

        pDescriptor->AddAttribute(SdpAttribute::TCAP, strTemp);

        IMS_TRACE_I("GenerateTcap(): Add tcap[%s]", strTemp.GetStr(), 0, 0);
    }
}

PROTECTED
void VideoSdpGenerator::GenerateAcap(
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile::CapaNego& objCapaNego)
{
    if (pDescriptor == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateAcap(): invalid argument", 0, 0, 0);
        return;
    }

    ImsMap<IMS_SINT32, AString> objAcap = objCapaNego.GetMapAcap();

    if (objCapaNego.IsAttCapaInPcfg())
    {
        for (IMS_UINT32 i = 0; i < objAcap.GetSize(); i++)
        {
            AString strTemp = "";
            strTemp.Sprintf("%d %s", i + 1, objAcap.GetValueAt(i).GetStr());

            pDescriptor->AddAttribute(SdpAttribute::ACAP, strTemp);

            IMS_TRACE_I("GenerateAcap(): Add acap[%s]", strTemp.GetStr(), 0, 0);
        }
    }
}

PROTECTED
void VideoSdpGenerator::GeneratePcfg(
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile::CapaNego& objCapaNego)
{
    if (pDescriptor == IMS_NULL)
    {
        IMS_TRACE_E(0, "GeneratePcfg(): invalid argument", 0, 0, 0);
        return;
    }

    ImsList<AString> lstPcfg = objCapaNego.GetListPcfg();

    for (IMS_UINT32 i = 0; i < lstPcfg.GetSize(); i++)
    {
        AString strTemp = "";
        strTemp.Sprintf("%d %s", i + 1, lstPcfg.GetAt(i).GetStr());

        pDescriptor->AddAttribute(SdpAttribute::PCFG, strTemp);

        IMS_TRACE_I("GeneratePcfg(): Add pcfg[%s]", strTemp.GetStr(), 0, 0);
    }
}

PROTECTED IMS_BOOL VideoSdpGenerator::MakeImageAttributeLine(
        IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId, OUT AString& strImageAttr)
{
    IMS_UINT32 nWidth, nHeight;

    if (!GetWidthHeightFromResolutionId(eResolutionId, &nWidth, &nHeight))
    {
        IMS_TRACE_E(0, "MakeImageAttributeLine(): invalid resolution", 0, 0, 0);
        return IMS_FALSE;
    }

    strImageAttr.Sprintf(
            "%d send [x=%d,y=%d] recv [x=%d,y=%d]", nPayloadType, nWidth, nHeight, nWidth, nHeight);

    return IMS_TRUE;
}

PROTECTED IMS_BOOL VideoSdpGenerator::MakeFrameSizeLine(
        IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId, OUT AString& strFrameSize)
{
    IMS_UINT32 nWidth, nHeight;

    if (!GetWidthHeightFromResolutionId(eResolutionId, &nWidth, &nHeight))
    {
        IMS_TRACE_E(0, "MakeFrameSizeLine(): invalid resolution", 0, 0, 0);
        return IMS_FALSE;
    }

    strFrameSize.Sprintf("%d %d-%d", nPayloadType, nWidth, nHeight);

    return IMS_TRUE;
}

PROTECTED IMS_BOOL VideoSdpGenerator::GetWidthHeightFromResolutionId(
        IN VIDEO_RESOLUTION eResolutionId, OUT IMS_UINT32* pnWidth, OUT IMS_UINT32* pnHeight)
{
    IMS_TRACE_D("GetWidthHeightFromResolutionId(): resolutionId[%d]", eResolutionId, 0, 0);
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
            IMS_TRACE_E(0, "GetWidthHeightFromResolutionId(): invalid resolution id", 0, 0, 0);
            return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED AString VideoSdpGenerator::GenerateAvcFmtp(
        IN std::shared_ptr<VideoProfile::AvcFmtp> pAvcFmtp)
{
    AString strFmtp = AString::ConstNull();

    if (pAvcFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateAvcFmtp(): invalid fmtp", 0, 0, 0);
        return strFmtp;
    }

    AddProfileLevelIdToFmtp(pAvcFmtp, strFmtp);
    AddPacketizationModeToFmtp(pAvcFmtp, strFmtp);
    AddSpropParameterSetsToFmtp(pAvcFmtp, strFmtp);

    return strFmtp;
}

PROTECTED void VideoSdpGenerator::AddProfileLevelIdToFmtp(
        IN std::shared_ptr<VideoProfile::AvcFmtp> pFmtp, OUT AString& fmtp)
{
    if (pFmtp != IMS_NULL)
    {
        IMS_TRACE_I("AddProfileLevelIdToFmtp(): profile-level-id=%s, visible=%d",
                pFmtp->GetProfileLevelId().GetStr(), pFmtp->IsProfileLevelIdVisible(), 0);

        if (pFmtp->IsProfileLevelIdVisible())
        {
            AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

            AString strTemp;
            strTemp.Sprintf("profile-level-id=%s", pFmtp->GetProfileLevelId().GetStr());
            fmtp.Append(strTemp);
        }
    }
}

PROTECTED void VideoSdpGenerator::AddPacketizationModeToFmtp(
        IN std::shared_ptr<VideoProfile::VideoFmtp> pFmtp, OUT AString& fmtp)
{
    if (pFmtp != IMS_NULL)
    {
        IMS_TRACE_I("AddPacketizationModeToFmtp(): packetization-mode=%d, visible=%d",
                pFmtp->GetPacketizationMode(), pFmtp->IsPacketizationModeVisible(), 0);

        if (pFmtp->IsPacketizationModeVisible())
        {
            AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

            AString strTemp;
            strTemp.Sprintf("packetization-mode=%d", pFmtp->GetPacketizationMode());
            fmtp.Append(strTemp);
        }
    }
}

PROTECTED void VideoSdpGenerator::AddSpropParameterSetsToFmtp(
        IN std::shared_ptr<VideoProfile::VideoFmtp> pFmtp, OUT AString& fmtp)
{
    if (pFmtp != IMS_NULL)
    {
        IMS_TRACE_I("AddSpropParameterSetsToFmtp(): sprop-parameter-sets=%s, visible=%d",
                pFmtp->GetSpropParam().GetStr(), pFmtp->IsSpropParamVisible(), 0);

        if (pFmtp->IsSpropParamVisible())
        {
            AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

            AString strTemp;
            strTemp.Sprintf("sprop-parameter-sets=%s", pFmtp->GetSpropParam().GetStr());
            fmtp.Append(strTemp);
        }
    }
}

PROTECTED AString VideoSdpGenerator::GenerateHevcFmtp(
        IN std::shared_ptr<VideoProfile::HevcFmtp> pHevcFmtp)
{
    AString strFmtp = AString::ConstNull();

    if (pHevcFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateHevcFmtp(): invalid fmtp", 0, 0, 0);
        return strFmtp;
    }

    AddProfileIdToFmtp(pHevcFmtp, strFmtp);
    AddLevelIdToFmtp(pHevcFmtp, strFmtp);
    AddSpropParamsToFmtp(pHevcFmtp, strFmtp);
    return strFmtp;
}

PROTECTED void VideoSdpGenerator::AddProfileIdToFmtp(
        IN std::shared_ptr<VideoProfile::HevcFmtp> pFmtp, OUT AString& fmtp)
{
    if (pFmtp != IMS_NULL)
    {
        IMS_TRACE_I("AddProfileIdToFmtp(): profile-id=%d, visible=%d", pFmtp->GetProfile(),
                pFmtp->IsProfileVisible(), 0);

        if (pFmtp->IsProfileVisible())
        {
            AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

            AString strTemp;
            strTemp.Sprintf("profile-id=%d", pFmtp->GetProfile());
            fmtp.Append(strTemp);
        }
    }
}

PROTECTED void VideoSdpGenerator::AddLevelIdToFmtp(
        IN std::shared_ptr<VideoProfile::HevcFmtp> pFmtp, OUT AString& fmtp)
{
    if (pFmtp != IMS_NULL)
    {
        IMS_TRACE_I("AddLevelIdToFmtp(): level-id=%d, visible=%d", pFmtp->GetLevel(),
                pFmtp->IsLevelVisible(), 0);

        if (pFmtp->IsLevelVisible())
        {
            AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

            AString strTemp;
            strTemp.Sprintf("level-id=%d", pFmtp->GetLevel());
            fmtp.Append(strTemp);
        }
    }
}

PROTECTED void VideoSdpGenerator::AddSpropParamsToFmtp(
        IN std::shared_ptr<VideoProfile::HevcFmtp> pFmtp, OUT AString& fmtp)
{
    if (pFmtp != IMS_NULL)
    {
        IMS_TRACE_I("AddSpropParamsToFmtp(): sprop parameter=%s, visible=%d",
                pFmtp->GetSpropParam().GetStr(), pFmtp->IsSpropParamVisible(), 0);

        if (pFmtp->IsSpropParamVisible())
        {
            ImsList<AString> objSplitComma = pFmtp->GetSpropParam().Split(',');

            if (objSplitComma.GetSize() == 3)
            {
                AString strVps = AString::ConstNull();
                AString strSps = AString::ConstNull();
                AString strPps = AString::ConstNull();

                strVps = objSplitComma.GetAt(0);
                strSps = objSplitComma.GetAt(1);
                strPps = objSplitComma.GetAt(2);

                if (strVps.GetLength() > 0 || strSps.GetLength() > 0 || strPps.GetLength() > 0)
                {
                    AString strTemp;

                    AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

                    strTemp.Sprintf("sprop-vps=%s", strVps.GetStr());
                    fmtp.Append(strTemp);

                    AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

                    strTemp.Sprintf("sprop-sps=%s", strSps.GetStr());
                    fmtp.Append(strTemp);

                    AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

                    strTemp.Sprintf("sprop-pps=%s", strPps.GetStr());
                    fmtp.Append(strTemp);
                }
            }
        }
    }
}
