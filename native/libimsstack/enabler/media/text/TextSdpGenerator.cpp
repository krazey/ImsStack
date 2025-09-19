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

#include "text/TextSdpGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextSdpGenerator::TextSdpGenerator() :
        MediaSdpGenerator(MEDIA_TYPE_TEXT)
{
    IMS_TRACE_I("+TextSdpGenerator()", 0, 0, 0);
}

PUBLIC VIRTUAL TextSdpGenerator::~TextSdpGenerator()
{
    IMS_TRACE_I("~TextSdpGenerator()", 0, 0, 0);
}

PUBLIC
IMS_BOOL TextSdpGenerator::Generate(OUT ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pBaseProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pBaseProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "Generate(): invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("Generate(): PayloadSize[%d]", pBaseProfile->GetPayloadList().GetSize(), 0, 0);

    GenerateCommonAttributes(pSessionDescriptor, pDescriptor, pBaseProfile);

    TextProfile* pProfile = static_cast<TextProfile*>(pBaseProfile);

    CheckRedPayloadSubTypeValidity(pProfile);
    GeneratePayload(pDescriptor, pProfile);
    GenerateDirection(pDescriptor, pProfile);

    return IMS_TRUE;
}

PRIVATE
void TextSdpGenerator::CheckRedPayloadSubTypeValidity(OUT TextProfile* pProfile)
{
    if (pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "CheckRedPayloadSubTypeValidity(): invalid argument", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        TextProfile::Payload* pPayload = pProfile->GetPayloadAt(i);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
        {
            auto pRedFmtp = std::static_pointer_cast<TextProfile::RedFmtp>(pPayload->GetFmtp());
            if (pRedFmtp == IMS_NULL)
            {
                continue;
            }

            IMS_TRACE_I("CheckRedPayloadSubTypeValidity(): level[%d], Red Payload[%d]",
                    pRedFmtp->GetRedLevel(), pRedFmtp->GetRedPayload(), 0);

            IMS_BOOL bRedSubPTExist = IMS_FALSE;

            for (IMS_UINT32 j = 0; j < pProfile->GetPayloadList().GetSize(); j++)
            {
                TextProfile::Payload* pTempPayload = pProfile->GetPayloadAt(j);

                if (pTempPayload == IMS_NULL)
                {
                    continue;
                }

                IMS_TRACE_I("CheckRedPayloadSubTypeValidity(): payload[%d], Red Payload[%d]",
                        pTempPayload->GetRtpMap().GetPayloadNumber(), pRedFmtp->GetRedPayload(), 0);

                if (pTempPayload->GetRtpMap().GetPayloadNumber() ==
                        (IMS_UINT32)pRedFmtp->GetRedPayload())
                {
                    bRedSubPTExist = IMS_TRUE;
                    continue;
                }
            }

            if (!bRedSubPTExist)
            {
                IMS_TRACE_E(0,
                        "CheckRedPayloadSubTypeValidity(): SubPayloadtype isn't "
                        "exist. skip Payload, Payload[%s], PT[%d]",
                        pPayload->GetRtpMap().GetPayloadType().GetStr(),
                        pPayload->GetRtpMap().GetPayloadNumber(), 0);
                pProfile->GetPayloadList().RemoveAt(i);
            }
        }
    }
}

PRIVATE
void TextSdpGenerator::GeneratePayload(OUT IMediaDescriptor* pDescriptor, IN TextProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GeneratePayload(): invalid argument", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AString strRtpMap = AString::ConstNull();
        AString strPayloadNum = AString::ConstNull();
        AString strFmtp = AString::ConstNull();

        TextProfile::Payload* pPayload = pProfile->GetPayloadAt(i);

        if (pPayload != IMS_NULL &&
                GenerateRtpMap(strRtpMap, strPayloadNum, pPayload->GetRtpMap()) &&
                GenerateFmtp(strFmtp, pPayload))
        {
            pDescriptor->SetMediaFormat(
                    SdpMediaFormat::TYPE_RTP, strPayloadNum, strRtpMap, strFmtp);
        }
    }
}

PRIVATE
IMS_BOOL TextSdpGenerator::GenerateFmtp(OUT AString& strFmtp, IN TextProfile::Payload* pPayload)
{
    if (pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateFmtp(): invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
    {
        auto pRedFmtp = std::static_pointer_cast<TextProfile::RedFmtp>(pPayload->GetFmtp());

        if (pRedFmtp == IMS_NULL)
        {
            IMS_TRACE_E(0, "GenerateFmtp(): invalid fmtp", 0, 0, 0);
            return IMS_FALSE;
        }

        IMS_SINT32 nCount = pRedFmtp->GetRedLevel();
        AString TempSubPT;
        TempSubPT.Sprintf("%d", pRedFmtp->GetRedPayload());

        while (nCount > 0)
        {
            if (strFmtp.GetLength() > 0)
            {
                strFmtp.Append("/");
            }

            strFmtp.Append(TempSubPT);
            nCount--;
        }

        IMS_TRACE_I("GenerateFmtp(): Redundancy[%d], RED Payload[%d], Fmtp[%s]",
                pRedFmtp->GetRedLevel(), pRedFmtp->GetRedPayload(), strFmtp.GetStr());
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140"))
    {
        auto pT140Fmtp = std::static_pointer_cast<TextProfile::T140Fmtp>(pPayload->GetFmtp());

        if (pT140Fmtp != IMS_NULL && pT140Fmtp->IsCpsVisible())
        {
            strFmtp.Sprintf("cps=%d", pT140Fmtp->GetCps());
            IMS_TRACE_I("GenerateFmtp(): T140 CPS[%d], Fmtp[%s]", pT140Fmtp->GetCps(),
                    strFmtp.GetStr(), 0);
        }
    }

    return IMS_TRUE;
}
