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

#include "config/CodecT140Config.h"
#include "config/ImsCodec.h"
#include "config/TextConfiguration.h"
#include "text/TextProfileGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextProfileGenerator::TextProfileGenerator() :
        MediaProfileGenerator(MEDIA_TYPE_TEXT)
{
    IMS_TRACE_I("+TextProfileGenerator()", 0, 0, 0);
}

PUBLIC VIRTUAL TextProfileGenerator::~TextProfileGenerator()
{
    IMS_TRACE_I("~TextProfileGenerator()", 0, 0, 0);
}

PROTECTED
TextProfile* TextProfileGenerator::SetProfile(IN MediaBaseProfile* pProfile,
        IN MediaConfiguration* pConfig, MEDIA_SERVICE_TYPE /*eServiceType*/, IN IService* pIService,
        IN IMS_SINT32 nSlotId)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL || pIService == IMS_NULL)
    {
        return IMS_NULL;
    }

    SetCommonProfile(pProfile, pConfig, pIService, nSlotId);

    TextProfile* pTextProfile = static_cast<TextProfile*>(pProfile);
    TextConfiguration* pTextConfig = static_cast<TextConfiguration*>(pConfig);

    pTextProfile->SetTransportType("RTP/AVP");
    pTextProfile->SetKeepRedundantLevel(pTextConfig->IsTextCodecEmptyRedundantEnabled());

    IMS_TRACE_I("SetProfile() - transport type[%s], keep red level[%d]",
            pTextProfile->GetTransportType().GetStr(), pTextProfile->GetKeepRedundantLevel(), 0);

    return pTextProfile;
}

PROTECTED
void TextProfileGenerator::CreateCodecPayloads(IN MediaBaseProfile* pProfile, IN IMS_SINT32 nCodec,
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL || pCodecConfig == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("CreateCodecPayloads() - codec[%s]", ImsCodec::CodecToString(nCodec), 0, 0);

    if (nCodec > ImsCodec::TEXT_NONE && nCodec < ImsCodec::TEXT_MAX)
    {
        TextProfile::Payload* pTempPayload = IMS_NULL;

        if (nCodec == ImsCodec::TEXT_T140 || nCodec == ImsCodec::TEXT_RED)
        {
            pTempPayload = CreateT140Payload(pCodecConfig, pConfig);
        }

        if (pTempPayload != IMS_NULL)
        {
            static_cast<TextProfile*>(pProfile)->GetPayloadList().Append(pTempPayload);
        }
    }
}

PROTECTED TextProfile::Payload* TextProfileGenerator::CreateT140Payload(
        IN CodecConfig* pCodecConfig, IN MediaConfiguration* pConfig)
{
    if (pCodecConfig == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    CodecT140Config* pT140Config = reinterpret_cast<CodecT140Config*>(pCodecConfig);
    TextProfile::Payload* pTextPayload = new TextProfile::Payload();
    AString strCodecName;

    if (pCodecConfig->GetCodec() == ImsCodec::TEXT_RED)
    {
        strCodecName.Sprintf("%s", "red");

        TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp(pT140Config->GetRedLevel(),
                static_cast<TextConfiguration*>(pConfig)->GetT140PayloadType());

        IMS_TRACE_I("CreateT140Payload() add fmtp - red level(%d), red payload(%d)",
                pRedFmtp->GetRedLevel(), pRedFmtp->GetRedPayload(), 0);

        pTextPayload->SetFmtp(pRedFmtp);
    }
    else
    {
        strCodecName.Sprintf("%s", "t140");
    }

    pTextPayload->SetRtpMap(
            pT140Config->GetPayloadType(), strCodecName, pT140Config->GetSamplingRate());

    return pTextPayload;
}
