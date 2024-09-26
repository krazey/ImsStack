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
#include "offeranswer/SdpMediaFormatParameter.h"
#include "offeranswer/SdpRtcpFeedback.h"

#include "video/VideoSdpParser.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC VideoSdpParser::VideoSdpParser() :
        MediaSdpParser(MEDIA_TYPE_VIDEO)
{
    IMS_TRACE_I("+VideoSdpParser()", 0, 0, 0);
}

PUBLIC VIRTUAL VideoSdpParser::~VideoSdpParser()
{
    IMS_TRACE_I("~VideoSdpParser()", 0, 0, 0);
}

PUBLIC
IMS_BOOL VideoSdpParser::Parse(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("Parse()", 0, 0, 0);

    MediaSdpParser::Parse(pSessionDescriptor, pDescriptor, pProfile);
    ParseTranportType(pDescriptor, pProfile);
    SetAvpfSupport(pProfile);

    // read CapaNego profile From SDP
    if (ParseCapaNego(pDescriptor, &(pProfile->GetCapaNego())) == IMS_TRUE)
    {
        // Get Capa nego value from the incoming SDP
        if (IsAvpfSupported(pProfile) == IMS_TRUE)
        {
            pProfile->SetSupportCapaNegoForAvpf(IMS_TRUE);
        }
    }

    ParsePayloads(pDescriptor, pProfile);

    // framerate
    pProfile->SetFrameRate(pDescriptor->GetAttributeInt(SdpAttribute::FRAMERATE));

    ParseCvo(pDescriptor, pProfile);

    IMS_TRACE_I("Parse() - Ended[%d]", pProfile->GetPayloadList().GetSize(), 0, 0);
    return IMS_TRUE;
}

PRIVATE
void VideoSdpParser::ParseTranportType(
        IN const IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    SdpMedia* pSdpMedia = const_cast<SdpMedia*>(pDescriptor->GetMediaDescriptionEx());

    if (pSdpMedia != IMS_NULL)
    {
        pProfile->SetTransportType(pSdpMedia->GetTransportProtocolEx());
        IMS_TRACE_D("ParseTranportType() - transport type[%s]",
                pProfile->GetTransportType().GetStr(), 0, 0);
    }
}

PRIVATE
void VideoSdpParser::SetAvpfSupport(OUT VideoProfile* pProfile)
{
    if (pProfile == IMS_NULL)
    {
        return;
    }

    if (pProfile->GetTransportType().EqualsIgnoreCase("RTP/AVP") == IMS_TRUE)
    {
        pProfile->SetSupportAvpf(IMS_FALSE);
        pProfile->SetSupportCapaNegoForAvpf(IMS_FALSE);
    }
    else if (pProfile->GetTransportType().EqualsIgnoreCase("RTP/AVPF") == IMS_TRUE)
    {
        pProfile->SetSupportAvpf(IMS_TRUE);
        pProfile->SetSupportCapaNegoForAvpf(IMS_TRUE);
    }

    IMS_TRACE_D("SetAvpfSupport() - support AVPF[%d], support CapaNego[%d]",
            pProfile->IsAvpfSupported(), pProfile->IsCapaNegoForAvpfSupported(), 0);
}

PRIVATE
void VideoSdpParser::ParsePayloads(IN IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    ImsList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();
    ImsList<AString> objImageAttributes = pDescriptor->GetAttributes(SdpAttribute::IMAGEATTR);
    ImsList<AString> objFrameSizes = pDescriptor->GetAttributes(SdpAttribute::FRAMESIZE);

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSdpCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));
        VideoProfile::Payload* pPayload = new VideoProfile::Payload();

        if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
        {
            delete pPayload;
            continue;
        }

        IMS_TRACE_I("ParsePayloads() - At[%d]", i, 0, 0);

        ParseRtpMap(pSdpCodec, pPayload);

        VIDEO_CODEC eVideoCodec = VIDEO_CODEC_NONE;
        eVideoCodec = SetCodec(pPayload);

        if (IsValidCodec(eVideoCodec))
        {
            AString strImageAttr = AString::ConstNull();
            if (objImageAttributes.GetSize() > i)
            {
                strImageAttr = ParseImageAttr(pSdpCodec, objImageAttributes, pPayload);
            }

            AString strFrameSize = AString::ConstNull();
            if (objFrameSizes.GetSize() > i)
            {
                strFrameSize = ParseFrameSize(pSdpCodec, objFrameSizes, pPayload);
            }

            ParseFmtp(pSdpCodec, pPayload, eVideoCodec);
            ParseResolution(pPayload, strImageAttr, strFrameSize, eVideoCodec);
            ParseAvpfAttribute(pSdpCodec, pPayload, pProfile);

            pProfile->GetPayloadList().Append(pPayload);
        }
        else
        {
            delete pPayload;
            continue;
        }
    }
}

PRIVATE
void VideoSdpParser::ParseRtpMap(
        IN const SdpAvCodec* pSdpCodec, OUT VideoProfile::Payload* pPayload)
{
    if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nPayloadTypeNumber = pSdpCodec->GetPayloadType();
    AString strCodecName = pSdpCodec->GetName();
    IMS_UINT32 nSamplingRate = pSdpCodec->GetClockRate();
    pPayload->SetRtpMap(nPayloadTypeNumber, strCodecName, nSamplingRate);

    IMS_TRACE_D("ParseRtpMap() - Payload[%d], Codec[%s], Sampling rate[%d]", nPayloadTypeNumber,
            strCodecName.GetStr(), nSamplingRate);
}

PRIVATE
VIDEO_CODEC VideoSdpParser::SetCodec(IN VideoProfile::Payload* pPayload)
{
    VIDEO_CODEC eVideoCodec = VIDEO_CODEC_NONE;

    if (pPayload == IMS_NULL)
    {
        return eVideoCodec;
    }

    AString strPayload = pPayload->GetRtpMap().GetPayloadType();

    if (strPayload.EqualsIgnoreCase("H264"))
    {
        eVideoCodec = VIDEO_CODEC_AVC;
    }
    else if (strPayload.EqualsIgnoreCase("H265"))
    {
        eVideoCodec = VIDEO_CODEC_HEVC;
    }
    else
    {
        eVideoCodec = VIDEO_CODEC_NOTUSED;
        IMS_TRACE_I("SetCodec() - codec[%s] not used", strPayload.GetStr(), 0, 0);
    }

    return eVideoCodec;
}

PRIVATE
IMS_BOOL VideoSdpParser::IsValidCodec(IN const VIDEO_CODEC eVideoCodec)
{
    return (eVideoCodec == VIDEO_CODEC_AVC || eVideoCodec == VIDEO_CODEC_HEVC) ? IMS_TRUE
                                                                               : IMS_FALSE;
}

PRIVATE
AString VideoSdpParser::ParseImageAttr(IN const SdpAvCodec* pSdpCodec,
        IN const ImsList<AString>& objImageAttributes, OUT VideoProfile::Payload* pPayload)
{
    AString strImageAttr = AString::ConstNull();

    if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
    {
        return strImageAttr;
    }

    IMS_UINT32 nIndex = 0;
    if (GetCorrectImageIndex(pSdpCodec->GetPayloadType(), objImageAttributes, &nIndex))
    {
        pPayload->SetIncludeImageAttr(IMS_TRUE);
        strImageAttr = objImageAttributes.GetAt(nIndex);
    }

    IMS_TRACE_D("ParseImageAttr() - Included[%d], Image Attribute[%s]",
            pPayload->IsImageAttrIncluded(), strImageAttr.GetStr(), 0);

    return strImageAttr;
}

PRIVATE
AString VideoSdpParser::ParseFrameSize(IN const SdpAvCodec* pSdpCodec,
        IN const ImsList<AString>& objFrameSizes, OUT VideoProfile::Payload* pPayload)
{
    AString strFrameSize = AString::ConstNull();

    if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
    {
        return strFrameSize;
    }

    IMS_UINT32 nIndex = 0;
    if (GetCorrectImageIndex(pSdpCodec->GetPayloadType(), objFrameSizes, &nIndex))
    {
        strFrameSize = objFrameSizes.GetAt(nIndex);
        pPayload->SetIncludeFrameSize(IMS_TRUE);
    }

    IMS_TRACE_D("ParseFrameSize() - Included[%d], FrameSize[%s]", pPayload->IsFrameSizeIncluded(),
            strFrameSize.GetStr(), 0);

    return strFrameSize;
}

PRIVATE
void VideoSdpParser::ParseCvo(IN IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseCvo() - invalid argument", 0, 0, 0);
        return;
    }

    ImsList<AString> objAttributes =
            pDescriptor->GetAttributes(SdpAttribute::ATTRIBUTE_OTHER, "extmap");

    for (IMS_UINT32 nIndex = 0; nIndex < objAttributes.GetSize(); nIndex++)
    {
        AString strExtmap = objAttributes.GetAt(nIndex);

        if (strExtmap.Contains("urn:3gpp:video-orientation") == IMS_TRUE)
        {
            AString strCvoTrim = strExtmap.Trim();
            ImsList<AString> strSplitSpace = strCvoTrim.Split(' ');

            if (strSplitSpace.GetAt(0).GetLength() > 0)
            {
                if (strSplitSpace.GetAt(0).Contains("/"))
                {
                    ImsList<AString> strSplitSlash = strSplitSpace.GetAt(0).Split('/');

                    if (strSplitSlash.GetSize() > 0 && strSplitSlash.GetAt(0).GetLength() > 0)
                    {
                        pProfile->SetCvoId(strSplitSlash.GetAt(0).ToInt32());
                    }
                }
                else
                {
                    pProfile->SetCvoId(strSplitSpace.GetAt(0).ToInt32());
                }

                IMS_TRACE_D("ParseCvo() - CVO found. ID[%d]", pProfile->GetCvoId(), 0, 0);
            }
        }
    }
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseFmtp(IN const SdpAvCodec* pSdpCodec,
        OUT VideoProfile::Payload* pPayload, IN const VIDEO_CODEC eVideoCodec)
{
    if (pPayload == IMS_NULL || pSdpCodec == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strFmtp = pSdpCodec->GetFormatSpecificParameter();
    VideoProfile::VideoFmtp* pFmtp = IMS_NULL;

    if (eVideoCodec == VIDEO_CODEC_AVC)
    {
        pFmtp = new VideoProfile::AvcFmtp();
    }
    else if (eVideoCodec == VIDEO_CODEC_HEVC)
    {
        pFmtp = new VideoProfile::HevcFmtp();
    }
    else
    {
        IMS_TRACE_E(0, "ParseFmtp() - NOT SUPPORTED video codec", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsList<AString> objSplitColon = strFmtp.Split(';');

    AString strVps = AString::ConstNull();
    AString strSps = AString::ConstNull();
    AString strPps = AString::ConstNull();

    for (IMS_UINT32 i = 0; i < objSplitColon.GetSize(); i++)
    {
        if (objSplitColon.GetAt(i).GetLength() == 0)
        {
            continue;
        }

        ImsList<AString> objSplitEqual = objSplitColon.GetAt(i).Split('=');

        if (objSplitEqual.GetSize() < 2)
        {
            const AString& strTmp = objSplitColon.GetAt(i);
            IMS_TRACE_D("ParseFmtp() - Invalid video fmtp parameter(%s) at index(%d)",
                    strTmp.GetStr(), i, 0);
            continue;
        }

        if ((objSplitEqual.GetAt(0).GetLength() == 0) || (objSplitEqual.GetAt(1).GetLength() == 0))
        {
            continue;
        }

        if (ParseVideoBaseFmtp(objSplitEqual, pFmtp) == IMS_FALSE)
        {
            switch (eVideoCodec)
            {
                case VIDEO_CODEC_AVC:
                {
                    IMS_SINT32 nIndexOf1stEqual = objSplitColon.GetAt(i).GetIndexOf("=");
                    AString strSpropParam = objSplitColon.GetAt(i).GetSubStr(nIndexOf1stEqual + 1);
                    ParseAvcFmtp(objSplitEqual, strSpropParam,
                            static_cast<VideoProfile::AvcFmtp*>(pFmtp));
                }
                break;
                case VIDEO_CODEC_HEVC:
                    ParseHevcFmtp(objSplitEqual, strVps, strSps, strPps,
                            static_cast<VideoProfile::HevcFmtp*>(pFmtp));
                    break;
                default:
                    break;
            }
        }
    }

    pPayload->SetFmtp(pFmtp);

    if (eVideoCodec == VIDEO_CODEC_HEVC)
    {
        ParseSpropParam(strVps, strSps, strPps, pFmtp);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseVideoBaseFmtp(
        IN const ImsList<AString>& objSplitEqual, OUT VideoProfile::VideoFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (ParsePacketizationMode(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void VideoSdpParser::ParseAvcFmtp(IN const ImsList<AString>& objSplitEqual,
        IN const AString& strSpropParam, OUT VideoProfile::AvcFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return;
    }

    if (ParseProfileLevelId(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ParseSpropParameterSets(objSplitEqual, strSpropParam, pFmtp))
    {
        return;
    }
}

PRIVATE
void VideoSdpParser::ParseHevcFmtp(IN const ImsList<AString>& objSplitEqual, OUT AString& strVps,
        OUT AString& strSps, OUT AString& strPps, OUT VideoProfile::HevcFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return;
    }

    if (ParseProfileId(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ParseLevelId(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ParseVps(objSplitEqual, strVps))
    {
        return;
    }
    if (ParseSps(objSplitEqual, strSps))
    {
        return;
    }
    if (ParsePps(objSplitEqual, strPps))
    {
        return;
    }
}

PRIVATE
IMS_BOOL VideoSdpParser::ParsePacketizationMode(
        IN const ImsList<AString>& objSplitEqual, OUT VideoProfile::VideoFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("packetization-mode") == IMS_TRUE)
    {
        pFmtp->SetPacketizationMode((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowPacketizationMode(IMS_TRUE);

        IMS_TRACE_I("ParsePacketizationMode() - Packetization mode[%d], Visible[%d]",
                pFmtp->GetPacketizationMode(), pFmtp->IsPacketizationModeVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseProfileLevelId(
        IN const ImsList<AString>& objSplitEqual, OUT VideoProfile::AvcFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("profile-level-id") == IMS_TRUE)
    {
        pFmtp->SetProfileLevelId(objSplitEqual.GetAt(1));
        pFmtp->SetProfile(
                VideoProfileUtil::GetAvcProfileFromProfileLevelId(pFmtp->GetProfileLevelId()));
        pFmtp->SetLevel(
                VideoProfileUtil::GetAvcLevelFromProfileLevelId(pFmtp->GetProfileLevelId()));
        pFmtp->SetShowProfileLevelId(IMS_TRUE);

        IMS_TRACE_I("ParseProfileLevelId() - profile-level-id[%s], Visible[%d]",
                pFmtp->GetProfileLevelId().GetStr(), pFmtp->IsProfileLevelIdVisible(), 0);
        IMS_TRACE_I("ParseProfileLevelId() - Profile[%d], Level[%d]", pFmtp->GetProfile(),
                pFmtp->GetLevel(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseSpropParameterSets(IN const ImsList<AString>& objSplitEqual,
        IN const AString& strSpropParam, OUT VideoProfile::AvcFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("sprop-parameter-sets") == IMS_TRUE)
    {
        ImsList<AString> objSplitComma = strSpropParam.Split(',');
        if (objSplitComma.GetSize() < 2)
        {
            IMS_TRACE_E(0, "ParseAvcFmtp() - objSplitComma's size less than 2 !!!", 0, 0, 0);
            return IMS_FALSE;
        }
        else if ((objSplitComma.GetAt(0).GetLength() % 4 != 0) &&
                (objSplitComma.GetAt(1).GetLength() % 4 != 0))
        {
            IMS_TRACE_E(0, "ParseAvcFmtp() - Sprop Length Error - SPS[%d], PPS[%d]",
                    objSplitComma.GetAt(0).GetLength(), objSplitComma.GetAt(1).GetLength(), 0);
            return IMS_FALSE;
        }

        pFmtp->SetSpropParam(strSpropParam);
        pFmtp->SetShowSpropParam(IMS_TRUE);

        IMS_TRACE_I("ParseSpropParameterSets() - Sprop Params[%s], Visible[%d]",
                pFmtp->GetSpropParam().GetStr(), pFmtp->IsSpropParamVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseProfileId(
        IN const ImsList<AString>& objSplitEqual, OUT VideoProfile::HevcFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("profile-id") == IMS_TRUE)
    {
        pFmtp->SetProfile((VIDEO_PROFILE_HEVC)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowProfile(IMS_TRUE);

        IMS_TRACE_I("ParseProfileId() - Profile Id[%d], Visible[%d]", pFmtp->GetProfile(),
                pFmtp->IsProfileVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseLevelId(
        IN const ImsList<AString>& objSplitEqual, OUT VideoProfile::HevcFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("level-id") == IMS_TRUE)
    {
        pFmtp->SetLevel((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowLevel(IMS_TRUE);

        IMS_TRACE_I("ParseLevelId() - Level Id[%d], Visible[%d]", pFmtp->GetLevel(),
                pFmtp->IsLevelVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseVps(IN const ImsList<AString>& objSplitEqual, OUT AString& strVps)
{
    if (objSplitEqual.GetAt(0).Equals("sprop-vps") == IMS_TRUE)
    {
        strVps = objSplitEqual.GetAt(1);

        IMS_TRACE_D("ParseVps() - Sprop Vps[%s]", strVps.GetStr(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseSps(IN const ImsList<AString>& objSplitEqual, OUT AString& strSps)
{
    if (objSplitEqual.GetAt(0).Equals("sprop-sps") == IMS_TRUE)
    {
        strSps = objSplitEqual.GetAt(1);

        IMS_TRACE_D("ParseSps() - Sprop Sps[%s]", strSps.GetStr(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParsePps(IN const ImsList<AString>& objSplitEqual, OUT AString& strPps)
{
    if (objSplitEqual.GetAt(0).Equals("sprop-pps") == IMS_TRUE)
    {
        strPps = objSplitEqual.GetAt(1);

        IMS_TRACE_D("ParsePps() - Sprop Pps[%s]", strPps.GetStr(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void VideoSdpParser::ParseSpropParam(IN const AString& strVps, IN const AString& strSps,
        IN const AString& strPps, OUT VideoProfile::VideoFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return;
    }

    if (!strVps.IsNULL() && !strSps.IsNULL() && !strPps.IsNULL())
    {
        AString strTemp;
        strTemp.Append(strVps);
        strTemp.Append(",");
        strTemp.Append(strSps);
        strTemp.Append(",");
        strTemp.Append(strPps);

        pFmtp->SetSpropParam(strTemp);
        pFmtp->SetShowSpropParam(IMS_TRUE);

        IMS_TRACE_I("ParseSpropParam() - Sprop Param[%s], Visible[%d]",
                pFmtp->GetSpropParam().GetStr(), pFmtp->IsSpropParamVisible(), 0);
    }
}

PRIVATE
void VideoSdpParser::ParseResolution(OUT VideoProfile::Payload* pPayload, AString& strImageAttr,
        AString& strFrameSize, VIDEO_CODEC eVideoCodec)
{
    if (pPayload == IMS_NULL)
    {
        return;
    }

    VideoProfile::VideoFmtp* pFmtp = static_cast<VideoProfile::VideoFmtp*>(pPayload->GetFmtp());

    if (pFmtp == IMS_NULL)
    {
        return;
    }

    pFmtp->SetResolution(
            GetResolutionFromSdp(eVideoCodec, strImageAttr, strFrameSize, pFmtp->GetSpropParam()));

    IMS_TRACE_I("ParseResolution() - resolution[%d]", pFmtp->GetResolution(), 0, 0);
}

PRIVATE
void VideoSdpParser::ParseAvpfAttribute(
        IN SdpAvCodec* pSdpCodec, IN VideoProfile::Payload* pPayload, OUT VideoProfile* pProfile)
{
    if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    if (pProfile->IsAvpfSupported() == IMS_TRUE)
    {
        IMS_TRACE_I("ParseAvpfAttribute()", 0, 0, 0);

        if (GetAvpfFromAttributes(
                    pSdpCodec, &pProfile->GetCapaNego(), &pPayload->GetRtcpFbAttr()) == IMS_FALSE)
        {
            GetAvpfFromAttributes_EX(&pProfile->GetCapaNego(), &pPayload->GetRtcpFbAttr());
        }
    }
}

PRIVATE IMS_BOOL VideoSdpParser::IsAvpfSupported(IN VideoProfile* pProfile)
{
    if (pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_UINT32 i = 0;
    AString strTcap = "";
    AString strAvpf = "AVPF";

    for (i = 0; i < pProfile->GetCapaNego().GetMapTcap().GetSize(); i++)
    {
        strTcap = pProfile->GetCapaNego().GetMapTcap().GetValueAt(i);

        if (strTcap.Contains(strAvpf) == IMS_TRUE)
        {
            IMS_TRACE_I("IsAvpfSupported() - found AVPF from profile ", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    IMS_TRACE_I("IsAvpfSupported() - Not Supported ", 0, 0, 0);
    return IMS_FALSE;
}

PRIVATE IMS_BOOL VideoSdpParser::GetCorrectImageIndex(
        IN IMS_SINT32 nPayloadTypeNum, IN ImsList<AString> objAttributes, OUT IMS_UINT32* nIndex)
{
    for (IMS_UINT32 i = 0; i < objAttributes.GetSize(); i++)
    {
        ImsList<AString> objTokens = objAttributes.GetAt(i).Split(TextParser::CHAR_SP);

        if (objTokens.GetSize() < 2)
        {
            return IMS_FALSE;
        }

        // add a case of wild-card
        if (objTokens.GetAt(0).Equals("*"))
        {
            IMS_TRACE_D("GetCorrectImageIndex()-wild-card - nIndex[%d], nPayloadNum[%d]", *nIndex,
                    nPayloadTypeNum, 0);
            return IMS_TRUE;
        }

        if (nPayloadTypeNum == objTokens.GetAt(0).ToInt32())
        {
            *nIndex = i;
            IMS_TRACE_D("GetCorrectImageIndex()-nIndex[%d], nPayloadNum[%d]", *nIndex,
                    nPayloadTypeNum, 0);

            return IMS_TRUE;
        }
    }

    IMS_TRACE_E(0, "GetCorrectImageIndex()-No matched", 0, 0, 0);

    return IMS_FALSE;
}

PRIVATE VIDEO_RESOLUTION VideoSdpParser::GetResolutionFromSdp(IN VIDEO_CODEC codecType,
        IN const AString& strImageAttr, IN const AString& strFrameSize,
        IN const AString& strSpropParam, IN IMS_SINT32 nQcif)
{
    IMS_UINT32 nWidth, nHeight;

    // Get nWidth, nHeight From Image Attribute
    if (strImageAttr.GetLength() != 0 &&
            (GetWidthHeightFromSdp_ImageAttr(strImageAttr, &nWidth, &nHeight) != IMS_FALSE))
    {
        return GetResolutionFromWidthHeight(nWidth, nHeight);
    }

    /** TODO_MEDIA video sprop */
    (void)codecType;
    (void)strSpropParam;
    // // - Get nWidth, nHeight From SpropParam
    // if (strSpropParam.GetLength() != 0 &&
    //        (GetWidthHeightFromSdp_SpropParam(
    //                 codecType, strSpropParam.GetStr(), &nWidth, &nHeight) != IMS_FALSE))
    // {
    //    return GetResolutionFromWidthHeight(nWidth, nHeight);
    // }

    // Get nWidth, nHeight From Framesize
    if (strFrameSize.GetLength() != 0 &&
            (GetWidthHeightFromSdp_FrameSize(strFrameSize, &nWidth, &nHeight) != IMS_FALSE))
    {
        return GetResolutionFromWidthHeight(nWidth, nHeight);
    }

    // Check if nQcif exist
    else if (nQcif != -1)
    {
        return VIDEO_RESOLUTION_QCIF_LS;
    }
    else
    {
        IMS_TRACE_E(0, "GetResolutionFromSdp() - No preferred resolution from SDP...", 0, 0, 0);
        return VIDEO_RESOLUTION_NOT_USED;
    }
}

PRIVATE IMS_BOOL VideoSdpParser::GetAvpfFromAttributes(IN SdpMediaFormat* pMediaFormat,
        IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr)
{
    if (pMediaFormat == IMS_NULL || pRtcpFbAttr == IMS_NULL || pCapaNego == IMS_NULL)
        return IMS_FALSE;

    ImsList<SdpMediaFormatParameter*> lstRTCPFeedback = pMediaFormat->GetExtraParameters();

    IMS_TRACE_I("GetAvpfFromAttributes() - Entered. number of RTCP attributes[%d]",
            lstRTCPFeedback.GetSize(), 0, 0);
    if (lstRTCPFeedback.GetSize() == 0)
        return IMS_FALSE;

    for (IMS_UINT32 i = 0; i < lstRTCPFeedback.GetSize(); i++)
    {
        SdpMediaFormatParameter* pMediaParam = lstRTCPFeedback.GetAt(i);
        if (pMediaParam == IMS_NULL)
            continue;

        if (pMediaParam->GetAttribute() == SdpAttribute::RTCP_FB)
        {
            SdpRtcpFeedback* pRtcpParam = static_cast<SdpRtcpFeedback*>(pMediaParam);
            if (pRtcpParam->GetType().Equals("trr-int"))
            {
                pRtcpFbAttr->SetTrrSupported(IMS_TRUE);
                pRtcpFbAttr->SetTrrInt(pRtcpParam->GetParamName().ToInt32());
            }
            else if (pRtcpParam->GetType().Equals("nack"))
            {
                pRtcpFbAttr->SetNackSupported(IMS_TRUE);

                if (pRtcpParam->GetParamName().Equals("pli"))
                {
                    pRtcpFbAttr->SetPliSupported(IMS_TRUE);
                }
            }
            else if (pRtcpParam->GetType().Equals("ccm"))
            {
                if (pRtcpParam->GetParamName().Equals("fir"))
                {
                    pRtcpFbAttr->SetFirSupported(IMS_TRUE);
                }

                if (pRtcpParam->GetParamName().Equals("tmmbr"))
                {
                    pRtcpFbAttr->SetTmmbrSupported(IMS_TRUE);
                }
            }

            IMS_TRACE_D("GetAvpfFromAttributes() - pRtcpParam->GetType[%s], GetParamName[%s]",
                    pRtcpParam->GetType().GetStr(), pRtcpParam->GetParamName().GetStr(), 0);
        }
    }

    if (pCapaNego->GetMapAcap().GetSize() > 0)
    {
        IMS_TRACE_D("GetAvpfFromAttributes() - AttributeCapa value exist - Size[%d]",
                pCapaNego->GetMapAcap().GetSize(), 0, 0);
        for (IMS_UINT32 i = 0; i < pCapaNego->GetMapAcap().GetSize(); i++)
        {
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("trr-int") == IMS_TRUE)
            {
                ImsList<AString> strTemp =
                        pCapaNego->GetMapAcap().GetValueAt(i).Split(TextParser::CHAR_SP);

                if (strTemp.GetSize() >= 2)
                {
                    pRtcpFbAttr->SetTrrInt(strTemp.GetAt(strTemp.GetSize() - 1).ToInt32());
                    pRtcpFbAttr->SetTrrSupported(IMS_TRUE);
                }
            }
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("nack") == IMS_TRUE)
                pRtcpFbAttr->SetNackSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("pli") == IMS_TRUE)
                pRtcpFbAttr->SetPliSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("fir") == IMS_TRUE)
                pRtcpFbAttr->SetFirSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("tmmbr") == IMS_TRUE)
                pRtcpFbAttr->SetTmmbrSupported(IMS_TRUE);
        }
    }

    IMS_TRACE_D("GetAvpfFromAttributes() - support = bNACK[%d], bPLI[%d], bTMMBR[%d]",
            pRtcpFbAttr->IsNackSupported(), pRtcpFbAttr->IsPliSupported(),
            pRtcpFbAttr->IsTmmbrSupported());
    IMS_TRACE_D("GetAvpfFromAttributes() - support = bFIR[%d], bTRR_Int[%d]",
            pRtcpFbAttr->IsFirSupported(), pRtcpFbAttr->IsTrrSupported(), 0);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoSdpParser::GetAvpfFromAttributes_EX(
        IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr)
{
    if (pRtcpFbAttr == IMS_NULL || pCapaNego == IMS_NULL)
        return IMS_FALSE;

    // check attribute..
    if (pCapaNego->GetMapAcap().GetSize() > 0)
    {
        IMS_TRACE_D("GetAvpfFromAttributes_EX() - AttributeCapa value exist - Size[%d]",
                pCapaNego->GetMapAcap().GetSize(), 0, 0);
        for (IMS_UINT32 i = 0; i < pCapaNego->GetMapAcap().GetSize(); i++)
        {
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("trr-int") == IMS_TRUE)
            {
                ImsList<AString> strTemp =
                        pCapaNego->GetMapAcap().GetValueAt(i).Split(TextParser::CHAR_SP);

                if (strTemp.GetSize() >= 2)
                {
                    pRtcpFbAttr->SetTrrInt(strTemp.GetAt(strTemp.GetSize() - 1).ToInt32());
                    pRtcpFbAttr->SetTrrSupported(IMS_TRUE);
                }
            }
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("nack") == IMS_TRUE)
                pRtcpFbAttr->SetNackSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("pli") == IMS_TRUE)
                pRtcpFbAttr->SetPliSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("fir") == IMS_TRUE)
                pRtcpFbAttr->SetFirSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("tmmbr") == IMS_TRUE)
                pRtcpFbAttr->SetTmmbrSupported(IMS_TRUE);
        }
        IMS_TRACE_D("GetAvpfFromAttributes_EX() - support = NACK[%d], PLI[%d], TMMBR[%d]",
                pRtcpFbAttr->IsNackSupported(), pRtcpFbAttr->IsPliSupported(),
                pRtcpFbAttr->IsTmmbrSupported());
        IMS_TRACE_D("GetAvpfFromAttributes_EX() - support = FIR[%d], TRR[%d], TTR-Int[%d]",
                pRtcpFbAttr->IsFirSupported(), pRtcpFbAttr->IsTrrSupported(),
                pRtcpFbAttr->GetTrrInt());
    }

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoSdpParser::GetWidthHeightFromSdp_ImageAttr(
        IN const AString& strImageAttr, OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight)
{
    IMS_UINT32 nImagePayloadNum = 0;  // Payload Number in Image Attr
    IMS_UINT32 nDirection = 1;        // Direction : send or recv
    IMS_UINT32 nImageValueIndex = 2;  // Image value : width and height
    ImsList<AString> objTokens;
    ImsList<AString> strTempValue;
    AString nRealValueString = AString::ConstNull();
    // Check SPACE is ...
    if (strImageAttr.Contains(TextParser::CHAR_SP) == IMS_FALSE)
    {
        IMS_TRACE_E(0,
                "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have CHAR_SP, ...", 0,
                0, 0);
        return IMS_FALSE;
    }
    else
    {
        objTokens = strImageAttr.Split(TextParser::CHAR_SP);

        if (objTokens.GetSize() < 3)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - the size of Peer ImageAttr is too small "
                    "to parse it ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
    }

    // 1st step : Take "Payload Number" out of Image Attribute.
    nImagePayloadNum = objTokens.GetAt(0).ToInt32();

    // 2nd step : Take "Image Size Values" out of Image Size according the Send/Recv direction.
    if (objTokens.GetAt(nDirection).EqualsIgnoreCase("send") == IMS_TRUE)
    {
        if (objTokens.GetAt(nImageValueIndex).Equals("*") == IMS_TRUE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - PeerResolution-Wildcard, Not Support", 0,
                    0, 0);
            return IMS_FALSE;
        }
        // remove spaces in Bracket, [ ]
        for (IMS_UINT32 i = nImageValueIndex; i < objTokens.GetSize(); i++)
        {
            if (objTokens.GetAt(i).EqualsIgnoreCase("recv") == IMS_TRUE)
            {
                // IMS_TRACE_D("GetWidthHeightFromSdp_ImageAttr() Removed all spaces in send
                // direction Bracket", 0, 0, 0);
                break;
            }
            nRealValueString.Append(objTokens.GetAt(i));
        }

        // Check LSBRACKET is ...
        if ((nRealValueString == AString::ConstNull()) ||
                (nRealValueString.Contains(TextParser::CHAR_LSBRACKET) == IMS_FALSE))
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have "
                    "CHAR_LSBRACKET, [ ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = nRealValueString.Split(TextParser::CHAR_LSBRACKET);
        }

        // Check RSBRACKET is ...
        if (strTempValue.GetAt(1).Contains(TextParser::CHAR_RSBRACKET) == IMS_FALSE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have "
                    "CHAR_RSBRACKET, ] ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(1).Split(TextParser::CHAR_RSBRACKET);
        }

        // Check COMMA is ...
        if (strTempValue.GetAt(0).Contains(TextParser::CHAR_COMMA) == IMS_FALSE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have a CHAR_COMMA, "
                    ", ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(0).Split(TextParser::CHAR_COMMA);
        }

        for (IMS_UINT32 i = 0; i < strTempValue.GetSize(); i++)
        {
            // Check EQUAL is ...
            if (strTempValue.GetAt(i).Contains(TextParser::CHAR_EQUAL) == IMS_FALSE)
            {
                IMS_TRACE_E(0,
                        "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have a "
                        "CHAR_EQUAL, = ...",
                        0, 0, 0);
                return IMS_FALSE;
            }
            else
            {
                ImsList<AString> strSendValue = strTempValue.GetAt(i).Split(TextParser::CHAR_EQUAL);

                if (strSendValue.GetAt(0).Equals("x") == IMS_TRUE)
                {
                    (*nImageWidth) =
                            strSendValue.GetAt(1).ToInt32();  // Image Width for Send Direction
                }
                else if (strSendValue.GetAt(0).Equals("y") == IMS_TRUE)
                {
                    (*nImageHeight) =
                            strSendValue.GetAt(1).ToInt32();  // Image Height for Send Direction
                }
                /* TODO: need to add
                else if (strSendValue.GetAt(0).Equals("sar") == IMS_TRUE)
                {
                    // dImageOptSAR = (IMS_DOUBLE)strSendValue.GetAt(1).GetStr();  // Sample Aspect
                    // Ratio for Send Direction
                }
                else if (strSendValue.GetAt(0).Equals("par") == IMS_TRUE)
                {
                    // dImageOptPAR = strSendValue.GetAt(1).ToDouble();  // Picture Aspect Ratio for
                    // Send Direction
                }
                else if (strSendValue.GetAt(0).Equals("q") == IMS_TRUE)
                {
                    // dImageOptQ = strSendValue.GetAt(1).ToDouble();    // Higher Preference for
                    // Send Direction
                }
                */
            }
        }

        IMS_TRACE_D("GetWidthHeightFromSdp_ImageAttr() nImagePayloadNum[%d] ImageWidth[%d], "
                    "ImageHeight[%d]",
                nImagePayloadNum, (*nImageWidth), (*nImageHeight));

        return IMS_TRUE;
    }
    else if (objTokens.GetAt(nDirection).Equals("recv") == IMS_TRUE)
    {
        if (objTokens.GetAt(nImageValueIndex).Equals("*") == IMS_TRUE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - PeerResolution-Wildcard, Not Support", 0,
                    0, 0);
            return IMS_FALSE;
        }
        // remove spaces in Bracket, [ ]
        for (IMS_UINT32 i = nImageValueIndex; i < objTokens.GetSize(); i++)
        {
            if (objTokens.GetAt(i).Equals("send") == IMS_TRUE)
            {
                // IMS_TRACE_D("GetWidthHeightFromSdp_ImageAttr() Removed all spaces in recv
                // direction Bracket", 0, 0, 0);
                break;
            }

            nRealValueString.Append(objTokens.GetAt(i));
        }

        // Check LSBRACKET is ...
        if ((nRealValueString == AString::ConstNull()) ||
                (nRealValueString.Contains(TextParser::CHAR_LSBRACKET) == IMS_FALSE))
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have "
                    "CHAR_LSBRACKET, [ ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = nRealValueString.Split(TextParser::CHAR_LSBRACKET);
        }

        // Check RSBRACKET is ...
        if (strTempValue.GetAt(1).Contains(TextParser::CHAR_RSBRACKET) == IMS_FALSE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have "
                    "CHAR_RSBRACKET, ] ...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(1).Split(TextParser::CHAR_RSBRACKET);
        }

        // Check COMMA is ...
        if (strTempValue.GetAt(0).Contains(TextParser::CHAR_COMMA) == IMS_FALSE)
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have CHAR_COMMA, , "
                    "...",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(0).Split(TextParser::CHAR_COMMA);
        }

        for (IMS_UINT32 i = 0; i < strTempValue.GetSize(); i++)
        {
            if (strTempValue.GetAt(i).Contains(TextParser::CHAR_EQUAL) == IMS_FALSE)
            {
                IMS_TRACE_E(0,
                        "GetWidthHeightFromSdp_ImageAttr() - Peer ImageAttr doesn't have "
                        "CHAR_EQUAL, = ...",
                        0, 0, 0);
                return IMS_FALSE;
            }
            else
            {
                ImsList<AString> strRecvValue = strTempValue.GetAt(i).Split(TextParser::CHAR_EQUAL);

                if (strRecvValue.GetAt(0).Equals("x") == IMS_TRUE)
                {
                    (*nImageWidth) =
                            strRecvValue.GetAt(1).ToInt32();  // Image Width for Recv Direction
                }
                else if (strRecvValue.GetAt(0).Equals("y") == IMS_TRUE)
                {
                    (*nImageHeight) =
                            strRecvValue.GetAt(1).ToInt32();  // Image Height for Recv Direction
                }
                /* TODO: Need to add
                else if (strRecvValue.GetAt(0).Equals("sar") == IMS_TRUE)
                {
                    // dImageOptSAR = strRecvValue.GetAt(1).ToDouble();  // Sample Aspect Ratio for
                    // Recv Direction
                }
                else if (strRecvValue.GetAt(0).Equals("par") == IMS_TRUE)
                {
                    // dImageOptPAR = strRecvValue.GetAt(1).ToDouble();  // Picture Aspect Ratio for
                    // Recv Direction
                }
                else if (strRecvValue.GetAt(0).Equals("q") == IMS_TRUE)
                {
                    // dImageOptQ = strRecvValue.GetAt(1).ToDouble();    // Higher Preference for
                    // Recv Direction
                }
                */
            }
        }

        IMS_TRACE_D("GetWidthHeightFromSdp_ImageAttr() nImagePayloadNum[%d] ImageWidth[%d], "
                    "ImageHeight[%d]",
                nImagePayloadNum, (*nImageWidth), (*nImageHeight));
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE VIDEO_RESOLUTION VideoSdpParser::GetResolutionFromWidthHeight(
        IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight)
{
    IMS_TRACE_D("GetResolutionFromWidthHeight() Widht[%d], Height[%d]", nWidth, nHeight, 0);
    if (nWidth == 480 && nHeight == 640)
        return VIDEO_RESOLUTION_VGA_PR;
    else if (nWidth == 640 && nHeight == 480)
        return VIDEO_RESOLUTION_VGA_LS;
    else if (nWidth == 240 && nHeight == 320)
        return VIDEO_RESOLUTION_QVGA_PR;
    else if (nWidth == 320 && nHeight == 240)
        return VIDEO_RESOLUTION_QVGA_LS;
    else if (nWidth == 144 && nHeight == 176)
        return VIDEO_RESOLUTION_QCIF_PR;
    else if (nWidth == 176 && nHeight == 144)
        return VIDEO_RESOLUTION_QCIF_LS;
    else if (nWidth == 352 && nHeight == 288)
        return VIDEO_RESOLUTION_CIF_LS;
    else if (nWidth == 288 && nHeight == 352)
        return VIDEO_RESOLUTION_CIF_PR;
    else if (nWidth == 240 && nHeight == 352)
        return VIDEO_RESOLUTION_SIF_PR;
    else if (nWidth == 352 && nHeight == 240)
        return VIDEO_RESOLUTION_SIF_LS;
    else if (nWidth == 128 && nHeight == 96)
        return VIDEO_RESOLUTION_SQCIF_LS;
    else if (nWidth == 96 && nHeight == 128)
        return VIDEO_RESOLUTION_SQCIF_PR;
    else if (nWidth == 720 && nHeight == 1280)
        return VIDEO_RESOLUTION_HD_PR;
    else if (nWidth == 1280 && nHeight == 720)
        return VIDEO_RESOLUTION_HD_LS;
    else if (nWidth == 1080 && nHeight == 1920)
        return VIDEO_RESOLUTION_FHD_PR;
    else if (nWidth == 1920 && nHeight == 1080)
        return VIDEO_RESOLUTION_FHD_LS;
    else
        IMS_TRACE_E(0, "GetResolutionFromWidthHeight() INVALID Widht[%d], Height[%d]", nWidth,
                nHeight, 0);
    return VIDEO_RESOLUTION_QCIF_PR;
}

PRIVATE IMS_BOOL VideoSdpParser::GetWidthHeightFromSdp_FrameSize(
        IN AString strFrameSize, OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight)
{
    IMS_UINT32 nFrameSizePayloadNum = 0;  // Payload Number in Image Attr

    ImsList<AString> objTokens = strFrameSize.Split(TextParser::CHAR_SP);

    if (objTokens.GetSize() < 2)
    {
        return IMS_FALSE;
    }

    // 1st step : Take "Payload Number" out of Image Attribute.
    nFrameSizePayloadNum = objTokens.GetAt(0).ToInt32();

    // 2nd step : Take "Frame Size Values" out of Frame Size Attribute.
    // Check CHAR_HYPHEN is ...
    if (objTokens.GetAt(1).Contains(TextParser::CHAR_HYPHEN) == IMS_FALSE)
    {
        IMS_TRACE_E(0,
                "GetWidthHeightFromSdp_FrameSize() - Peer ImageAttr doesn't have CHAR_HYPHEN, "
                "[%s], ...",
                strFrameSize.GetStr(), 0, 0);
        return IMS_FALSE;
    }
    else
    {
        ImsList<AString> strFrameSizeValue = objTokens.GetAt(1).Split(TextParser::CHAR_HYPHEN);

        *nImageWidth = strFrameSizeValue.GetAt(0).ToInt32();   // Width
        *nImageHeight = strFrameSizeValue.GetAt(1).ToInt32();  // Height
        IMS_TRACE_D("GetWidthHeightFromSdp_FrameSize() nFrameSizePayloadNum[%d] ImageWidth[%d], "
                    "ImageHeight[%d]",
                nFrameSizePayloadNum, *nImageWidth, *nImageHeight);

        return IMS_TRUE;
    }
}
