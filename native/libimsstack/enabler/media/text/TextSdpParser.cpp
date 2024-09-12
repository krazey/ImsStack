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

#include "text/TextSdpParser.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextSdpParser::TextSdpParser() :
        MediaSdpParser(MEDIA_TYPE_TEXT)
{
    IMS_TRACE_I("+TextSdpParser()", 0, 0, 0);
}

PUBLIC VIRTUAL TextSdpParser::~TextSdpParser()
{
    IMS_TRACE_I("~TextSdpParser()", 0, 0, 0);
}

PUBLIC IMS_BOOL TextSdpParser::Parse(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "Parse() - invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("Parse()", 0, 0, 0);

    MediaSdpParser::Parse(pSessionDescriptor, pDescriptor, pProfile);
    ParsePayloads(pDescriptor, pProfile);

    return IMS_TRUE;
}

PRIVATE
void TextSdpParser::ParsePayloads(IN IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParsePayloads() - invalid argument", 0, 0, 0);
        return;
    }

    ImsList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSdpCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));
        TextProfile::Payload* pPayload = new TextProfile::Payload();

        if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
        {
            delete pPayload;
            continue;
        }

        IMS_TRACE_I("ParsePayloads() - At[%d]", i, 0, 0);

        AString strCodecName = AString::ConstNull();
        ParseRtpMap(pSdpCodec, pPayload, strCodecName);

        // check fmtp of t140 redundancy
        if (strCodecName.EqualsIgnoreCase("red"))
        {
            ParseFmtp(pSdpCodec, pPayload, lstMediaFormat);
        }
        else if (!strCodecName.EqualsIgnoreCase("t140"))
        {
            IMS_TRACE_E(0, "ParsePayloads() - Invalid codec[%s]", strCodecName.GetStr(), 0, 0);
            delete pPayload;
            continue;
        }

        pProfile->GetPayloadList().Append(pPayload);
    }
}

PRIVATE
void TextSdpParser::ParseRtpMap(IN const SdpAvCodec* pSdpCodec, OUT TextProfile::Payload* pPayload,
        OUT AString& strCodecName)
{
    if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nPayloadTypeNumber = pSdpCodec->GetPayloadType();
    strCodecName = pSdpCodec->GetName();
    IMS_UINT32 nSamplingRate = pSdpCodec->GetClockRate();

    pPayload->SetRtpMap(nPayloadTypeNumber, strCodecName, nSamplingRate);

    IMS_TRACE_D("ParseRtpMap() - Payload[%d], Codec[%s], Sampling rate[%d]", nPayloadTypeNumber,
            strCodecName.GetStr(), nSamplingRate);
}

PRIVATE
IMS_BOOL TextSdpParser::ParseFmtp(IN const SdpAvCodec* pSdpCodec,
        OUT TextProfile::Payload* pPayload, IN const ImsList<SdpMediaFormat*>& lstMediaFormat)
{
    if (pPayload == IMS_NULL || pSdpCodec == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strFmtp = pSdpCodec->GetFormatSpecificParameter();
    TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp();

    if (pRedFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("ParseFmtp()", 0, 0, 0);

    if (ParseRedFmtp(strFmtp, pRedFmtp) == IMS_FALSE ||
            ParseRedSubPtExist(pRedFmtp->GetRedPayload(), lstMediaFormat) == IMS_FALSE)
    {
        IMS_TRACE_E(0, "ParseFmtp() - cannot make red fmtp or No matched subtype", 0, 0, 0);

        delete pRedFmtp;
        return IMS_FALSE;
    }

    pPayload->SetFmtp(pRedFmtp);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL TextSdpParser::ParseRedFmtp(
        IN const AString& strFmtp, OUT TextProfile::RedFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL || strFmtp.IsEmpty() == IMS_TRUE)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("ParseRedFmtp()", 0, 0, 0);

    ImsList<AString> strArrTemp = strFmtp.Split('/');
    pFmtp->SetRedLevel(strArrTemp.GetSize());

    if (pFmtp->GetRedLevel() == 0)
    {
        return IMS_FALSE;
    }

    pFmtp->SetRedPayload(strArrTemp.GetAt(0).ToInt32());

    for (IMS_SINT32 i = 0; i < pFmtp->GetRedLevel() - 1; i++)
    {
        if (strArrTemp.GetAt(i).ToInt32() != pFmtp->GetRedPayload())
        {
            pFmtp->SetRedLevel(-1);
            pFmtp->SetRedPayload(-1);

            return IMS_FALSE;
        }
    }

    IMS_TRACE_D("ParseRedFmtp() Red Level[%d], Red Payload[%d]", pFmtp->GetRedLevel(),
            pFmtp->GetRedPayload(), 0);

    return IMS_TRUE;
}

PRIVATE IMS_BOOL TextSdpParser::ParseRedSubPtExist(
        IN const IMS_SINT32 nRedPayload, IN const ImsList<SdpMediaFormat*>& lstMediaFormat)
{
    IMS_BOOL bRedSubPtExist = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSdpCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));

        if (pSdpCodec == IMS_NULL)
        {
            continue;
        }

        IMS_TRACE_D("ParseRedSubPtExist() - Check RedSubPT, PT[%d] of PL[%d] / Red Payload[%d]",
                pSdpCodec->GetPayloadType(), i, nRedPayload);

        if (pSdpCodec->GetPayloadType() == nRedPayload)
        {
            bRedSubPtExist = IMS_TRUE;
        }
    }

    IMS_TRACE_I("ParseRedSubPtExist() - RedSubPtExist[%d]", bRedSubPtExist, 0, 0);

    return bRedSubPtExist;
}
