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

#include "offeranswer/SdpMediaFormat.h"
#include "text/TextSdpParser.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextSdpParser::TextSdpParser() :
        MediaSdpParser(MEDIA_TYPE_TEXT)
{
}

PUBLIC VIRTUAL TextSdpParser::~TextSdpParser() {}

PUBLIC IMS_BOOL TextSdpParser::Parse(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "Parse(): invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    MediaSdpParser::Parse(pSessionDescriptor, pDescriptor, pProfile);
    ParsePayloads(pDescriptor, pProfile);

    if (pProfile->GetPayloadList().GetSize() == 0)
    {
        ParsePayloadTypeNumber(pDescriptor, pProfile);
    }

    return IMS_TRUE;
}

PROTECTED
void TextSdpParser::ParsePayloads(IN const IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParsePayloads(): invalid argument", 0, 0, 0);
        return;
    }

    ImsList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    IMS_TRACE_I("ParsePayloads(): payload size[%d]", lstMediaFormat.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        if (lstMediaFormat.GetAt(i) == IMS_NULL ||
                lstMediaFormat.GetAt(i)->GetType() != SdpMediaFormat::TYPE_RTP)
        {
            continue;
        }

        const SdpAvCodec* pSdpCodec = static_cast<SdpAvCodec*>(lstMediaFormat.GetAt(i));
        TextProfile::Payload* pPayload = new TextProfile::Payload();

        AString strCodecName = AString::ConstNull();
        ParseRtpMap(pSdpCodec, pPayload, strCodecName);

        IMS_BOOL bSuccess = IS_DYNAMIC_PAYLOAD_TYPE(pPayload->GetRtpMap().GetPayloadNumber());

        if (bSuccess)
        {
            if (strCodecName.EqualsIgnoreCase("red"))
            {
                bSuccess = ParseFmtp(pSdpCodec, pPayload, lstMediaFormat);
            }
            else if (strCodecName.EqualsIgnoreCase("t140"))
            {
                bSuccess = IMS_TRUE;
                ParseT140Fmtp(pSdpCodec, pPayload);
            }
            else
            {
                IMS_TRACE_E(0, "ParsePayloads(): Invalid codec[%s]", strCodecName.GetStr(), 0, 0);
                bSuccess = IMS_FALSE;
            }
        }

        if (bSuccess)
        {
            pProfile->AddPayload(pPayload);
        }
        else
        {
            delete pPayload;
        }
    }
}

PROTECTED
void TextSdpParser::ParseRtpMap(IN const SdpAvCodec* pSdpCodec, OUT TextProfile::Payload* pPayload,
        OUT AString& strCodecName)
{
    if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseRtpMap(): invalid argument", 0, 0, 0);
        return;
    }

    IMS_SINT32 nPayloadTypeNumber = pSdpCodec->GetPayloadType();
    strCodecName = pSdpCodec->GetName();
    IMS_UINT32 nSamplingRate = pSdpCodec->GetClockRate();

    pPayload->SetRtpMap(nPayloadTypeNumber, strCodecName, nSamplingRate);

    IMS_TRACE_D("ParseRtpMap(): Payload[%d], Codec[%s], Sampling rate[%d]", nPayloadTypeNumber,
            strCodecName.GetStr(), nSamplingRate);
}

PROTECTED
IMS_BOOL TextSdpParser::ParseFmtp(IN const SdpAvCodec* pSdpCodec,
        OUT TextProfile::Payload* pPayload, IN const ImsList<SdpMediaFormat*>& lstMediaFormat)
{
    if (pPayload == IMS_NULL || pSdpCodec == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strFmtp = pSdpCodec->GetFormatSpecificParameter();
    auto pRedFmtp = std::make_shared<TextProfile::RedFmtp>();

    if (ParseRedFmtp(strFmtp, pRedFmtp))
    {
        if (!ParseRedSubPtExist(pRedFmtp->GetRedPayload(), lstMediaFormat))
        {
            IMS_TRACE_I("ParseFmtp(): RED subtype not found", 0, 0, 0);
        }
    }

    pPayload->SetFmtp(pRedFmtp);
    return IMS_TRUE;
}

PROTECTED
IMS_BOOL TextSdpParser::ParseT140Fmtp(
        IN const SdpAvCodec* pSdpCodec, OUT TextProfile::Payload* pPayload)
{
    if (pPayload == IMS_NULL || pSdpCodec == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strFmtp = pSdpCodec->GetFormatSpecificParameter();
    auto pT140Fmtp = std::make_shared<TextProfile::T140Fmtp>();

    if (pT140Fmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ImsList<AString> objSplitColon = strFmtp.Split(';');
    for (IMS_UINT32 i = 0; i < objSplitColon.GetSize(); i++)
    {
        ImsList<AString> objSplitEqual = objSplitColon.GetAt(i).Split('=');
        if (objSplitEqual.GetSize() == 2 && objSplitEqual.GetAt(0).Equals("cps"))
        {
            pT140Fmtp->SetCps(objSplitEqual.GetAt(1).ToInt32());
            pT140Fmtp->SetVisibleCps(IMS_TRUE);
            IMS_TRACE_D("ParseT140Fmtp(): cps[%d]", pT140Fmtp->GetCps(), 0, 0);
        }
    }

    pPayload->SetFmtp(pT140Fmtp);
    return IMS_TRUE;
}

PROTECTED IMS_BOOL TextSdpParser::ParseRedFmtp(
        IN const AString& strFmtp, OUT std::shared_ptr<TextProfile::RedFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL || strFmtp.IsEmpty())
    {
        IMS_TRACE_E(0, "ParseRedFmtp(): invalid fmtp", 0, 0, 0);
        return IMS_FALSE;
    }

    ImsList<AString> strArrTemp = strFmtp.Split('/');
    pFmtp->SetRedLevel(strArrTemp.GetSize());

    if (pFmtp->GetRedLevel() == 0)
    {
        IMS_TRACE_E(0, "ParseRedFmtp(): RED level is 0", 0, 0, 0);
        return IMS_FALSE;
    }

    pFmtp->SetRedPayload(strArrTemp.GetAt(0).ToInt32());

    for (IMS_SINT32 i = 0; i < pFmtp->GetRedLevel() - 1; i++)
    {
        if (strArrTemp.GetAt(i).ToInt32() != pFmtp->GetRedPayload())
        {
            pFmtp->SetRedLevel(-1);
            pFmtp->SetRedPayload(-1);
            IMS_TRACE_E(0, "ParseRedFmtp(): invalid payload", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    IMS_TRACE_D("ParseRedFmtp() Red Level[%d], Red Payload[%d]", pFmtp->GetRedLevel(),
            pFmtp->GetRedPayload(), 0);

    return IMS_TRUE;
}

PROTECTED IMS_BOOL TextSdpParser::ParseRedSubPtExist(
        IN const IMS_SINT32 nRedPayload, IN const ImsList<SdpMediaFormat*>& lstMediaFormat)
{
    IMS_BOOL bRedSubPtExist = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        if (lstMediaFormat.GetAt(i) == IMS_NULL ||
                lstMediaFormat.GetAt(i)->GetType() != SdpMediaFormat::TYPE_RTP)
        {
            continue;
        }

        const SdpAvCodec* pSdpCodec = static_cast<SdpAvCodec*>(lstMediaFormat.GetAt(i));
        IMS_TRACE_D("ParseRedSubPtExist(): Check RedSubPT, PT[%d] of PL[%d] / Red Payload[%d]",
                pSdpCodec->GetPayloadType(), i, nRedPayload);

        if (pSdpCodec->GetPayloadType() == nRedPayload)
        {
            bRedSubPtExist = IMS_TRUE;
        }
    }

    IMS_TRACE_I("ParseRedSubPtExist(): RedSubPtExist[%d]", bRedSubPtExist, 0, 0);

    return bRedSubPtExist;
}
