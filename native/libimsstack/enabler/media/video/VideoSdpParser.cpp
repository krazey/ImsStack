/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the"License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an"AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ServiceTrace.h"
#include "offeranswer/SdpMediaFormatParameter.h"
#include "offeranswer/SdpRtcpFeedback.h"

#include "video/VideoSdpParser.h"
#include "video/VideoProfileUtil.h"

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
        IMS_TRACE_E(0, "Parse(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    MediaSdpParser::Parse(pSessionDescriptor, pDescriptor, pProfile);
    ParseTransportType(pDescriptor, pProfile);
    SetAvpfSupport(pProfile);

    // read CapaNego profile From SDP
    if (ParseCapaNego(pDescriptor, &(pProfile->GetCapaNego())))
    {
        // Get Capa nego value from the incoming SDP
        if (IsAvpfSupported(pProfile))
        {
            pProfile->SetSupportCapaNegoForAvpf(IMS_TRUE);
        }
    }

    ParsePayloads(pDescriptor, pProfile);

    // framerate
    pProfile->SetFrameRate(pDescriptor->GetAttributeInt(SdpAttribute::FRAMERATE));

    ParseCvo(pDescriptor, pProfile);

    IMS_TRACE_I("Parse(): Ended[%d]", pProfile->GetPayloadList().GetSize(), 0, 0);
    return IMS_TRUE;
}

PRIVATE
void VideoSdpParser::ParseTransportType(
        IN const IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseTransportType(): invalid arguments", 0, 0, 0);
        return;
    }

    const SdpMedia* pSdpMedia = pDescriptor->GetMediaDescriptionEx();

    if (pSdpMedia != IMS_NULL)
    {
        pProfile->SetTransportType(pSdpMedia->GetTransportProtocolEx());
        IMS_TRACE_D("ParseTransportType(): transport type[%s]",
                pProfile->GetTransportType().GetStr(), 0, 0);
    }
}

PRIVATE
void VideoSdpParser::SetAvpfSupport(OUT VideoProfile* pProfile)
{
    if (pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetAvpfSupport(): invalid arguments", 0, 0, 0);
        return;
    }

    if (pProfile->GetTransportType().EqualsIgnoreCase("RTP/AVP"))
    {
        pProfile->SetSupportAvpf(IMS_FALSE);
        pProfile->SetSupportCapaNegoForAvpf(IMS_FALSE);
    }
    else if (pProfile->GetTransportType().EqualsIgnoreCase("RTP/AVPF"))
    {
        pProfile->SetSupportAvpf(IMS_TRUE);
        pProfile->SetSupportCapaNegoForAvpf(IMS_TRUE);
    }

    IMS_TRACE_D("SetAvpfSupport(): support AVPF[%d], support CapaNego[%d]",
            pProfile->IsAvpfSupported(), pProfile->IsCapaNegoForAvpfSupported(), 0);
}

PRIVATE
void VideoSdpParser::ParsePayloads(
        IN const IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParsePayloads(): invalid arguments", 0, 0, 0);
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

        IMS_TRACE_I("ParsePayloads(): At[%d]", i, 0, 0);

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
        IMS_TRACE_E(0, "ParseRtpMap(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nPayloadTypeNumber = pSdpCodec->GetPayloadType();
    AString strCodecName = pSdpCodec->GetName();
    IMS_UINT32 nSamplingRate = pSdpCodec->GetClockRate();
    pPayload->SetRtpMap(nPayloadTypeNumber, strCodecName, nSamplingRate);

    IMS_TRACE_D("ParseRtpMap(): Payload[%d], Codec[%s], Sampling rate[%d]", nPayloadTypeNumber,
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
        IMS_TRACE_I("SetCodec(): codec[%s] not used", strPayload.GetStr(), 0, 0);
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
        IMS_TRACE_E(0, "ParseImageAttr(): invalid arguments", 0, 0, 0);
        return strImageAttr;
    }

    IMS_UINT32 nIndex = 0;
    if (GetCorrectImageIndex(pSdpCodec->GetPayloadType(), objImageAttributes, &nIndex))
    {
        pPayload->SetIncludeImageAttr(IMS_TRUE);
        strImageAttr = objImageAttributes.GetAt(nIndex);
    }

    IMS_TRACE_D("ParseImageAttr(): Included[%d], Image Attribute[%s]",
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
        IMS_TRACE_E(0, "ParseFrameSize(): invalid arguments", 0, 0, 0);
        return strFrameSize;
    }

    IMS_UINT32 nIndex = 0;
    if (GetCorrectImageIndex(pSdpCodec->GetPayloadType(), objFrameSizes, &nIndex))
    {
        strFrameSize = objFrameSizes.GetAt(nIndex);
        pPayload->SetIncludeFrameSize(IMS_TRUE);
    }

    IMS_TRACE_D("ParseFrameSize(): Included[%d], FrameSize[%s]", pPayload->IsFrameSizeIncluded(),
            strFrameSize.GetStr(), 0);

    return strFrameSize;
}

PRIVATE
void VideoSdpParser::ParseCvo(IN const IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseCvo(): invalid argument", 0, 0, 0);
        return;
    }

    ImsList<AString> objAttributes =
            pDescriptor->GetAttributes(SdpAttribute::ATTRIBUTE_OTHER, "extmap");

    for (IMS_UINT32 nIndex = 0; nIndex < objAttributes.GetSize(); nIndex++)
    {
        AString strExtmap = objAttributes.GetAt(nIndex);

        if (strExtmap.Contains("urn:3gpp:video-orientation"))
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

                IMS_TRACE_D("ParseCvo(): CVO found. ID[%d]", pProfile->GetCvoId(), 0, 0);
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
        IMS_TRACE_E(0, "ParseFmtp(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strFmtp = pSdpCodec->GetFormatSpecificParameter();
    std::shared_ptr<VideoProfile::VideoFmtp> pFmtp = IMS_NULL;

    if (eVideoCodec == VIDEO_CODEC_AVC)
    {
        pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    }
    else if (eVideoCodec == VIDEO_CODEC_HEVC)
    {
        pFmtp = std::make_shared<VideoProfile::HevcFmtp>();
    }
    else
    {
        IMS_TRACE_E(0, "ParseFmtp(): NOT SUPPORTED video codec", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseFmtp(): invalid fmtp", 0, 0, 0);
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
            IMS_TRACE_D("ParseFmtp(): Invalid video fmtp parameter(%s) at index(%d)",
                    strTmp.GetStr(), i, 0);
            continue;
        }

        if ((objSplitEqual.GetAt(0).GetLength() == 0) || (objSplitEqual.GetAt(1).GetLength() == 0))
        {
            continue;
        }

        if (!ParseVideoBaseFmtp(objSplitEqual, pFmtp))
        {
            switch (eVideoCodec)
            {
                case VIDEO_CODEC_AVC:
                {
                    IMS_SINT32 nIndexOf1stEqual = objSplitColon.GetAt(i).GetIndexOf("=");
                    AString strSpropParam = objSplitColon.GetAt(i).GetSubStr(nIndexOf1stEqual + 1);
                    ParseAvcFmtp(objSplitEqual, strSpropParam,
                            std::static_pointer_cast<VideoProfile::AvcFmtp>(pFmtp));
                }
                break;
                case VIDEO_CODEC_HEVC:
                    ParseHevcFmtp(objSplitEqual, strVps, strSps, strPps,
                            std::static_pointer_cast<VideoProfile::HevcFmtp>(pFmtp));
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
IMS_BOOL VideoSdpParser::ParseVideoBaseFmtp(IN const ImsList<AString>& objSplitEqual,
        OUT std::shared_ptr<VideoProfile::VideoFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseVideoBaseFmtp(): invalid arguments", 0, 0, 0);
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
        IN const AString& strSpropParam, OUT std::shared_ptr<VideoProfile::AvcFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseAvcFmtp(): invalid arguments", 0, 0, 0);
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
        OUT AString& strSps, OUT AString& strPps, OUT std::shared_ptr<VideoProfile::HevcFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseHevcFmtp(): invalid arguments", 0, 0, 0);
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
IMS_BOOL VideoSdpParser::ParsePacketizationMode(IN const ImsList<AString>& objSplitEqual,
        OUT std::shared_ptr<VideoProfile::VideoFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParsePacketizationMode(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("packetization-mode"))
    {
        pFmtp->SetPacketizationMode((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetVisiblePacketizationMode(IMS_TRUE);

        IMS_TRACE_I("ParsePacketizationMode(): Packetization mode[%d], Visible[%d]",
                pFmtp->GetPacketizationMode(), pFmtp->IsPacketizationModeVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseProfileLevelId(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<VideoProfile::AvcFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseProfileLevelId(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("profile-level-id"))
    {
        pFmtp->SetProfileLevelId(objSplitEqual.GetAt(1));
        pFmtp->SetProfile(
                VideoProfileUtil::GetAvcProfileFromProfileLevelId(pFmtp->GetProfileLevelId()));
        pFmtp->SetLevel(
                VideoProfileUtil::GetAvcLevelFromProfileLevelId(pFmtp->GetProfileLevelId()));
        pFmtp->SetVisibleProfileLevelId(IMS_TRUE);

        IMS_TRACE_I("ParseProfileLevelId(): profile-level-id[%s], Visible[%d]",
                pFmtp->GetProfileLevelId().GetStr(), pFmtp->IsProfileLevelIdVisible(), 0);
        IMS_TRACE_I("ParseProfileLevelId(): Profile[%d], Level[%d]", pFmtp->GetProfile(),
                pFmtp->GetLevel(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseSpropParameterSets(IN const ImsList<AString>& objSplitEqual,
        IN const AString& strSpropParam, OUT std::shared_ptr<VideoProfile::AvcFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseSpropParameterSets(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("sprop-parameter-sets"))
    {
        ImsList<AString> objSplitComma = strSpropParam.Split(',');
        if (objSplitComma.GetSize() < 2)
        {
            IMS_TRACE_E(0, "ParseAvcFmtp(): objSplitComma's size less than 2 !!!", 0, 0, 0);
            return IMS_FALSE;
        }
        else if ((objSplitComma.GetAt(0).GetLength() % 4 != 0) &&
                (objSplitComma.GetAt(1).GetLength() % 4 != 0))
        {
            IMS_TRACE_E(0, "ParseAvcFmtp(): Sprop Length Error - SPS[%d], PPS[%d]",
                    objSplitComma.GetAt(0).GetLength(), objSplitComma.GetAt(1).GetLength(), 0);
            return IMS_FALSE;
        }

        pFmtp->SetSpropParam(strSpropParam);
        pFmtp->SetVisibleSpropParam(IMS_TRUE);

        IMS_TRACE_I("ParseSpropParameterSets(): Sprop Params[%s], Visible[%d]",
                pFmtp->GetSpropParam().GetStr(), pFmtp->IsSpropParamVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseProfileId(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<VideoProfile::HevcFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseProfileId(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("profile-id"))
    {
        pFmtp->SetProfile((VIDEO_PROFILE_HEVC)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetVisibleProfile(IMS_TRUE);

        IMS_TRACE_I("ParseProfileId(): Profile Id[%d], Visible[%d]", pFmtp->GetProfile(),
                pFmtp->IsProfileVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseLevelId(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<VideoProfile::HevcFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseLevelId(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("level-id"))
    {
        pFmtp->SetLevel((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetVisibleLevel(IMS_TRUE);

        IMS_TRACE_I("ParseLevelId(): Level Id[%d], Visible[%d]", pFmtp->GetLevel(),
                pFmtp->IsLevelVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseVps(IN const ImsList<AString>& objSplitEqual, OUT AString& strVps)
{
    if (objSplitEqual.GetAt(0).Equals("sprop-vps"))
    {
        strVps = objSplitEqual.GetAt(1);

        IMS_TRACE_D("ParseVps(): Sprop Vps[%s]", strVps.GetStr(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParseSps(IN const ImsList<AString>& objSplitEqual, OUT AString& strSps)
{
    if (objSplitEqual.GetAt(0).Equals("sprop-sps"))
    {
        strSps = objSplitEqual.GetAt(1);

        IMS_TRACE_D("ParseSps(): Sprop Sps[%s]", strSps.GetStr(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL VideoSdpParser::ParsePps(IN const ImsList<AString>& objSplitEqual, OUT AString& strPps)
{
    if (objSplitEqual.GetAt(0).Equals("sprop-pps"))
    {
        strPps = objSplitEqual.GetAt(1);

        IMS_TRACE_D("ParsePps(): Sprop Pps[%s]", strPps.GetStr(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void VideoSdpParser::ParseSpropParam(IN const AString& strVps, IN const AString& strSps,
        IN const AString& strPps, OUT std::shared_ptr<VideoProfile::VideoFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseSpropParam(): invalid arguments", 0, 0, 0);
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
        pFmtp->SetVisibleSpropParam(IMS_TRUE);

        IMS_TRACE_I("ParseSpropParam(): Sprop Param[%s], Visible[%d]",
                pFmtp->GetSpropParam().GetStr(), pFmtp->IsSpropParamVisible(), 0);
    }
}

PRIVATE
void VideoSdpParser::ParseResolution(OUT VideoProfile::Payload* pPayload,
        const AString& strImageAttr, const AString& strFrameSize, VIDEO_CODEC eVideoCodec)
{
    if (pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseResolution(): invalid arguments", 0, 0, 0);
        return;
    }

    std::shared_ptr<VideoProfile::VideoFmtp> pFmtp = pPayload->GetFmtp();

    if (pFmtp == IMS_NULL)
    {
        return;
    }

    pFmtp->SetResolution(
            GetResolutionFromSdp(eVideoCodec, strImageAttr, strFrameSize, pFmtp->GetSpropParam()));

    IMS_TRACE_I("ParseResolution(): resolution[%d]", pFmtp->GetResolution(), 0, 0);
}

PRIVATE
void VideoSdpParser::ParseAvpfAttribute(IN const SdpAvCodec* pSdpCodec,
        IN VideoProfile::Payload* pPayload, OUT VideoProfile* pProfile)
{
    if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseAvpfAttribute(): invalid arguments", 0, 0, 0);
        return;
    }

    if (pProfile->IsAvpfSupported())
    {
        IMS_TRACE_I("ParseAvpfAttribute()", 0, 0, 0);

        if (!GetAvpfFromAttributes(pSdpCodec, &pProfile->GetCapaNego(), &pPayload->GetRtcpFbAttr()))
        {
            GetAvpfFromAttributesEx(&pProfile->GetCapaNego(), &pPayload->GetRtcpFbAttr());
        }
    }
}

PRIVATE IMS_BOOL VideoSdpParser::IsAvpfSupported(IN VideoProfile* pProfile)
{
    if (pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "IsAvpfSupported(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_UINT32 i = 0;
    AString strTcap = "";
    AString strAvpf = "AVPF";

    for (i = 0; i < pProfile->GetCapaNego().GetMapTcap().GetSize(); i++)
    {
        strTcap = pProfile->GetCapaNego().GetMapTcap().GetValueAt(i);

        if (strTcap.Contains(strAvpf))
        {
            return IMS_TRUE;
        }
    }

    IMS_TRACE_I("IsAvpfSupported(): not supported", 0, 0, 0);
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

        if (objTokens.GetAt(0).Equals("*"))
        {
            IMS_TRACE_D("GetCorrectImageIndex(): wild-card, index[%d], payload[%d]", *nIndex,
                    nPayloadTypeNum, 0);
            return IMS_TRUE;
        }

        if (nPayloadTypeNum == objTokens.GetAt(0).ToInt32())
        {
            *nIndex = i;
            IMS_TRACE_D(
                    "GetCorrectImageIndex(): index[%d], payload[%d]", *nIndex, nPayloadTypeNum, 0);

            return IMS_TRUE;
        }
    }

    IMS_TRACE_I("GetCorrectImageIndex(): not matched", 0, 0, 0);
    return IMS_FALSE;
}

PRIVATE VIDEO_RESOLUTION VideoSdpParser::GetResolutionFromSdp(IN VIDEO_CODEC /*codecType*/,
        IN const AString& strImageAttr, IN const AString& strFrameSize,
        IN const AString& /*strSpropParam*/, IN IMS_SINT32 nQcif)
{
    IMS_UINT32 nWidth, nHeight;

    // Get nWidth, nHeight From Image Attribute
    if (strImageAttr.GetLength() != 0 &&
            (GetWidthHeightFromSdpImageAttr(strImageAttr, &nWidth, &nHeight) != IMS_FALSE))
    {
        return GetResolutionFromWidthHeight(nWidth, nHeight);
    }

    /** TODO: parse the video sprop */
    // // - Get nWidth, nHeight From SpropParam
    // if (strSpropParam.GetLength() != 0 &&
    //        (GetWidthHeightFromSdp_SpropParam(
    //                 codecType, strSpropParam.GetStr(), &nWidth, &nHeight) != IMS_FALSE))
    // {
    //    return GetResolutionFromWidthHeight(nWidth, nHeight);
    // }

    if (strFrameSize.GetLength() != 0 &&
            (GetWidthHeightFromSdpFrameSize(strFrameSize, &nWidth, &nHeight) != IMS_FALSE))
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
        IMS_TRACE_D("GetResolutionFromSdp(): no preferred resolution from SDP", 0, 0, 0);
        return VIDEO_RESOLUTION_NOT_USED;
    }
}

PRIVATE IMS_BOOL VideoSdpParser::GetAvpfFromAttributes(IN const SdpMediaFormat* pMediaFormat,
        IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr)
{
    if (pMediaFormat == IMS_NULL || pRtcpFbAttr == IMS_NULL || pCapaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetAvpfFromAttributes(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    ImsList<SdpMediaFormatParameter*> lstRTCPFeedback = pMediaFormat->GetExtraParameters();

    if (lstRTCPFeedback.GetSize() == 0)
    {
        IMS_TRACE_E(0, "GetAvpfFromAttributes(): empty RTCP-FB attribute", 0, 0, 0);
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < lstRTCPFeedback.GetSize(); i++)
    {
        SdpMediaFormatParameter* pMediaParam = lstRTCPFeedback.GetAt(i);
        if (pMediaParam == IMS_NULL)
        {
            continue;
        }

        if (pMediaParam->GetAttribute() == SdpAttribute::RTCP_FB)
        {
            const SdpRtcpFeedback* pRtcpParam = static_cast<SdpRtcpFeedback*>(pMediaParam);
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

            IMS_TRACE_D("GetAvpfFromAttributes(): type[%s], name[%s]",
                    pRtcpParam->GetType().GetStr(), pRtcpParam->GetParamName().GetStr(), 0);
        }
    }

    if (pCapaNego->GetMapAcap().GetSize() > 0)
    {
        for (IMS_UINT32 i = 0; i < pCapaNego->GetMapAcap().GetSize(); i++)
        {
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("trr-int"))
            {
                ImsList<AString> strTemp =
                        pCapaNego->GetMapAcap().GetValueAt(i).Split(TextParser::CHAR_SP);

                if (strTemp.GetSize() >= 2)
                {
                    pRtcpFbAttr->SetTrrInt(strTemp.GetAt(strTemp.GetSize() - 1).ToInt32());
                    pRtcpFbAttr->SetTrrSupported(IMS_TRUE);
                }
            }
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("nack"))
                pRtcpFbAttr->SetNackSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("pli"))
                pRtcpFbAttr->SetPliSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("fir"))
                pRtcpFbAttr->SetFirSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("tmmbr"))
                pRtcpFbAttr->SetTmmbrSupported(IMS_TRUE);
        }
    }

    IMS_TRACE_D("GetAvpfFromAttributes(): support NACK[%d], PLI[%d], TMMBR[%d]",
            pRtcpFbAttr->IsNackSupported(), pRtcpFbAttr->IsPliSupported(),
            pRtcpFbAttr->IsTmmbrSupported());
    IMS_TRACE_D("GetAvpfFromAttributes(): support FIR[%d], TRR_Int[%d]",
            pRtcpFbAttr->IsFirSupported(), pRtcpFbAttr->IsTrrSupported(), 0);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoSdpParser::GetAvpfFromAttributesEx(
        IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr)
{
    if (pRtcpFbAttr == IMS_NULL || pCapaNego == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetAvpfFromAttributesEx(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pCapaNego->GetMapAcap().GetSize() > 0)
    {
        for (IMS_UINT32 i = 0; i < pCapaNego->GetMapAcap().GetSize(); i++)
        {
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("trr-int"))
            {
                ImsList<AString> strTemp =
                        pCapaNego->GetMapAcap().GetValueAt(i).Split(TextParser::CHAR_SP);

                if (strTemp.GetSize() >= 2)
                {
                    pRtcpFbAttr->SetTrrInt(strTemp.GetAt(strTemp.GetSize() - 1).ToInt32());
                    pRtcpFbAttr->SetTrrSupported(IMS_TRUE);
                }
            }
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("nack"))
                pRtcpFbAttr->SetNackSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("pli"))
                pRtcpFbAttr->SetPliSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("fir"))
                pRtcpFbAttr->SetFirSupported(IMS_TRUE);
            if (pCapaNego->GetMapAcap().GetValueAt(i).Contains("tmmbr"))
                pRtcpFbAttr->SetTmmbrSupported(IMS_TRUE);
        }
        IMS_TRACE_D("GetAvpfFromAttributesEx(): support NACK[%d], PLI[%d], TMMBR[%d]",
                pRtcpFbAttr->IsNackSupported(), pRtcpFbAttr->IsPliSupported(),
                pRtcpFbAttr->IsTmmbrSupported());
        IMS_TRACE_D("GetAvpfFromAttributesEx(): support FIR[%d], TRR[%d], TTR-Int[%d]",
                pRtcpFbAttr->IsFirSupported(), pRtcpFbAttr->IsTrrSupported(),
                pRtcpFbAttr->GetTrrInt());
    }

    return IMS_TRUE;
}

PRIVATE IMS_BOOL VideoSdpParser::GetWidthHeightFromSdpImageAttr(
        IN const AString& strImageAttr, OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight)
{
    IMS_UINT32 nImagePayloadNum = 0;  // Payload Number in Image Attr
    IMS_UINT32 nDirection = 1;        // Direction : send or recv
    IMS_UINT32 nImageValueIndex = 2;  // Image value : width and height
    ImsList<AString> objTokens;
    ImsList<AString> strTempValue;
    AString nRealValueString = AString::ConstNull();

    if (!strImageAttr.Contains(TextParser::CHAR_SP))
    {
        IMS_TRACE_E(0, "GetWidthHeightFromSdpImageAttr(): no CHAR_SP", 0, 0, 0);
        return IMS_FALSE;
    }
    else
    {
        objTokens = strImageAttr.Split(TextParser::CHAR_SP);

        if (objTokens.GetSize() < 3)
        {
            IMS_TRACE_E(0, "GetWidthHeightFromSdpImageAttr(): too small size to parse", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    nImagePayloadNum = objTokens.GetAt(0).ToInt32();

    if (objTokens.GetAt(nDirection).EqualsIgnoreCase("send"))
    {
        if (objTokens.GetAt(nImageValueIndex).Equals("*"))
        {
            IMS_TRACE_E(0, "GetWidthHeightFromSdpImageAttr(): wildcard, not support", 0, 0, 0);
            return IMS_FALSE;
        }
        for (IMS_UINT32 i = nImageValueIndex; i < objTokens.GetSize(); i++)
        {
            if (objTokens.GetAt(i).EqualsIgnoreCase("recv"))
            {
                break;
            }
            nRealValueString.Append(objTokens.GetAt(i));
        }

        if ((nRealValueString == AString::ConstNull()) ||
                (!nRealValueString.Contains(TextParser::CHAR_LSBRACKET)))
        {
            IMS_TRACE_E(0, "GetWidthHeightFromSdpImageAttr(): no CHAR_LSBRACKET", 0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = nRealValueString.Split(TextParser::CHAR_LSBRACKET);
        }

        if (!strTempValue.GetAt(1).Contains(TextParser::CHAR_RSBRACKET))
        {
            IMS_TRACE_E(0, "GetWidthHeightFromSdpImageAttr(): no CHAR_RSBRACKET", 0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(1).Split(TextParser::CHAR_RSBRACKET);
        }

        if (!strTempValue.GetAt(0).Contains(TextParser::CHAR_COMMA))
        {
            IMS_TRACE_E(0, "GetWidthHeightFromSdpImageAttr(): no CHAR_COMMA", 0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(0).Split(TextParser::CHAR_COMMA);
        }

        for (IMS_UINT32 i = 0; i < strTempValue.GetSize(); i++)
        {
            if (!strTempValue.GetAt(i).Contains(TextParser::CHAR_EQUAL))
            {
                IMS_TRACE_E(0, "GetWidthHeightFromSdpImageAttr(): no CHAR_EQUAL", 0, 0, 0);
                return IMS_FALSE;
            }
            else
            {
                ImsList<AString> strSendValue = strTempValue.GetAt(i).Split(TextParser::CHAR_EQUAL);

                if (strSendValue.GetAt(0).Equals("x"))
                {
                    (*nImageWidth) =
                            strSendValue.GetAt(1).ToInt32();  // Image Width for Send Direction
                }
                else if (strSendValue.GetAt(0).Equals("y"))
                {
                    (*nImageHeight) =
                            strSendValue.GetAt(1).ToInt32();  // Image Height for Send Direction
                }
            }
        }

        IMS_TRACE_D("GetWidthHeightFromSdpImageAttr(): payload[%d] width[%d], height[%d]",
                nImagePayloadNum, (*nImageWidth), (*nImageHeight));

        return IMS_TRUE;
    }
    else if (objTokens.GetAt(nDirection).Equals("recv"))
    {
        if (objTokens.GetAt(nImageValueIndex).Equals("*"))
        {
            IMS_TRACE_E(0, "GetWidthHeightFromSdpImageAttr(): wildcard, not Support", 0, 0, 0);
            return IMS_FALSE;
        }

        for (IMS_UINT32 i = nImageValueIndex; i < objTokens.GetSize(); i++)
        {
            if (objTokens.GetAt(i).Equals("send"))
            {
                break;
            }

            nRealValueString.Append(objTokens.GetAt(i));
        }

        if ((nRealValueString == AString::ConstNull()) ||
                (!nRealValueString.Contains(TextParser::CHAR_LSBRACKET)))
        {
            IMS_TRACE_E(0, "GetWidthHeightFromSdpImageAttr(): no CHAR_LSBRACKET", 0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = nRealValueString.Split(TextParser::CHAR_LSBRACKET);
        }

        if (!strTempValue.GetAt(1).Contains(TextParser::CHAR_RSBRACKET))
        {
            IMS_TRACE_E(0, "GetWidthHeightFromSdpImageAttr(): no CHAR_RSBRACKET", 0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(1).Split(TextParser::CHAR_RSBRACKET);
        }

        if (!strTempValue.GetAt(0).Contains(TextParser::CHAR_COMMA))
        {
            IMS_TRACE_E(0,
                    "GetWidthHeightFromSdpImageAttr(): no CHAR_COMMA, ,"
                    "",
                    0, 0, 0);
            return IMS_FALSE;
        }
        else
        {
            strTempValue = strTempValue.GetAt(0).Split(TextParser::CHAR_COMMA);
        }

        for (IMS_UINT32 i = 0; i < strTempValue.GetSize(); i++)
        {
            if (!strTempValue.GetAt(i).Contains(TextParser::CHAR_EQUAL))
            {
                IMS_TRACE_E(0, "GetWidthHeightFromSdpImageAttr(): no CHAR_EQUAL, =", 0, 0, 0);
                return IMS_FALSE;
            }
            else
            {
                ImsList<AString> strRecvValue = strTempValue.GetAt(i).Split(TextParser::CHAR_EQUAL);

                if (strRecvValue.GetAt(0).Equals("x"))
                {
                    (*nImageWidth) =
                            strRecvValue.GetAt(1).ToInt32();  // Image Width for Recv Direction
                }
                else if (strRecvValue.GetAt(0).Equals("y"))
                {
                    (*nImageHeight) =
                            strRecvValue.GetAt(1).ToInt32();  // Image Height for Recv Direction
                }
            }
        }

        IMS_TRACE_D("GetWidthHeightFromSdpImageAttr() payload[%d] width[%d], height[%d]",
                nImagePayloadNum, (*nImageWidth), (*nImageHeight));
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE VIDEO_RESOLUTION VideoSdpParser::GetResolutionFromWidthHeight(
        IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight)
{
    IMS_TRACE_D("GetResolutionFromWidthHeight(): width[%d], height[%d]", nWidth, nHeight, 0);
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
        IMS_TRACE_E(0, "GetResolutionFromWidthHeight(): not supported width[%d], height[%d]",
                nWidth, nHeight, 0);
    return VIDEO_RESOLUTION_QCIF_PR;
}

PRIVATE IMS_BOOL VideoSdpParser::GetWidthHeightFromSdpFrameSize(
        IN AString strFrameSize, OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight)
{
    IMS_UINT32 nFrameSizePayloadNum = 0;
    ImsList<AString> objTokens = strFrameSize.Split(TextParser::CHAR_SP);

    if (objTokens.GetSize() < 2)
    {
        return IMS_FALSE;
    }

    nFrameSizePayloadNum = objTokens.GetAt(0).ToInt32();

    if (!objTokens.GetAt(1).Contains(TextParser::CHAR_HYPHEN))
    {
        IMS_TRACE_E(0, "GetWidthHeightFromSdpFrameSize(): no CHAR_HYPHEN. [%s]",
                strFrameSize.GetStr(), 0, 0);
        return IMS_FALSE;
    }
    else
    {
        ImsList<AString> strFrameSizeValue = objTokens.GetAt(1).Split(TextParser::CHAR_HYPHEN);

        *nImageWidth = strFrameSizeValue.GetAt(0).ToInt32();
        *nImageHeight = strFrameSizeValue.GetAt(1).ToInt32();
        IMS_TRACE_D("GetWidthHeightFromSdpFrameSize(): payload[%d] width[%d], height[%d]",
                nFrameSizePayloadNum, *nImageWidth, *nImageHeight);

        return IMS_TRUE;
    }
}
