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

#include "IService.h"
#include "ServiceTrace.h"

#include "MediaEnvironment.h"
#include "MediaManager.h"
#include "MediaProfileFactory.h"
#include "MediaResourceManager.h"

#include "config/CodecT140Config.h"
#include "config/ImsCodec.h"
#include "config/TextConfiguration.h"
#include "text/TextProfile.h"

static MediaProfileFactory* g_pMediaProfileFactory = IMS_NULL;

__IMS_TRACE_TAG_USER_DECL__("MED.MPF");

PRIVATE
MediaProfileFactory::MediaProfileFactory() {}

PUBLIC VIRTUAL MediaProfileFactory::~MediaProfileFactory() {}

PUBLIC
MediaBaseProfile* MediaProfileFactory::CreateProfile(IN MediaEnvironment* pEnvironment,
        IN MediaConfiguration* pConfig, IN IMS_SINT32 nSlotId, IN MEDIA_CONTENT_TYPE eType)
{
    if (pEnvironment == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("CreateProfile()", 0, 0, 0);

    MediaManager* pMediaManager = MediaManager::GetInstance(nSlotId);

    if (pMediaManager == IMS_NULL)
    {
        return IMS_NULL;
    }

    MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr == IMS_NULL)
    {
        return IMS_NULL;
    }

    MediaBaseProfile* pProfile = IMS_NULL;

    switch (eType)
    {
        case MEDIA_TYPE_TEXT:
            pProfile = CreateTextProfile();
            pProfile = SetTextProfile(pProfile, pConfig);
            break;
        default:
            return IMS_NULL;
    }

    // Setting IP address
    pProfile->objIpAddress = pEnvironment->pIService->GetIpAddress();

    if (pProfile->nDataPort == 0)
    {
        pProfile->nDataPort = pResourceMngr->AcquireRtpPort(pConfig);
        pProfile->nControlPort = pProfile->nDataPort + 1;
    }

    IMS_TRACE_I("CreateProfile() - IpAddress[%s], port[%d]", pProfile->objIpAddress.ToCharString(),
            pProfile->nDataPort, 0);

    // Setting each payload and bandwidth
    ImsList<CodecConfig*> pCodecs;
    pCodecs = pConfig->GetCodecConfigs();

    for (IMS_UINT32 i = 0; i < pCodecs.GetSize(); i++)
    {
        CodecConfig* pCodecConfig = pCodecs.GetAt(i);

        if (pCodecConfig == IMS_NULL)
        {
            IMS_TRACE_D("pCodecConfig is NULL", 0, 0, 0);
            break;
        }

        if (pCodecConfig->GetCodec() == ImsCodec::TEXT_T140 ||
                pCodecConfig->GetCodec() == ImsCodec::TEXT_RED)
        {
            static_cast<TextProfile*>(pProfile)->lstPayload.Append(
                    CreateT140Payload(pCodecConfig, pConfig));
        }
        else
        {
            IMS_TRACE_E(0, "CreateProfile() - Invalid Codec(%d)", pCodecConfig->GetCodec(), 0, 0);
            delete pProfile;
            return IMS_NULL;
        }
    }

    // Setting direction
    pProfile->eDirection = MEDIA_DIRECTION_SEND_RECEIVE;
    pProfile->nBandwidthAs = pConfig->GetAsBandwidthKbps();
    pProfile->nBandwidthRr = pConfig->GetRrBandwidthBps();
    pProfile->nBandwidthRs = pConfig->GetRsBandwidthBps();
    pProfile->nRtcpInterval = pConfig->GetRtcpInterval();

    IMS_TRACE_I("CreateProfile() - direction[%d], rtcp Interval[%d]", pProfile->eDirection,
            pProfile->nRtcpInterval, 0);
    IMS_TRACE_I("CreateProfile() - AS[%d], RR[%d], RS[%d]", pProfile->nBandwidthAs,
            pProfile->nBandwidthRr, pProfile->nBandwidthRs);
    return pProfile;
}

PUBLIC
void MediaProfileFactory::DeleteProfile(IN MediaBaseProfile* pProfile)
{
    delete pProfile;
    pProfile = IMS_NULL;
}

PUBLIC
MediaProfileFactory* MediaProfileFactory::GetInstance()
{
    if (g_pMediaProfileFactory == IMS_NULL)
    {
        g_pMediaProfileFactory = new MediaProfileFactory();
    }

    return g_pMediaProfileFactory;
}

PUBLIC
void MediaProfileFactory::ReleaseInstance(MediaProfileFactory* pMediaProfileFactory)
{
    if (pMediaProfileFactory != IMS_NULL && pMediaProfileFactory == g_pMediaProfileFactory)
    {
        delete pMediaProfileFactory;
        g_pMediaProfileFactory = IMS_NULL;
    }
}

PRIVATE TextProfile* MediaProfileFactory::CreateTextProfile()
{
    TextProfile* pTextProfile = new TextProfile();

    return pTextProfile;
}

PRIVATE TextProfile* MediaProfileFactory::SetTextProfile(
        IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    TextProfile* pTextProfile = static_cast<TextProfile*>(pProfile);
    TextConfiguration* pTextConfig = static_cast<TextConfiguration*>(pConfig);

    pTextProfile->strTransportType = "RTP/AVP";
    pTextProfile->bKeepRedLevel = pTextConfig->IsTextCodecEmptyRedundantEnabled();

    IMS_TRACE_I("SetTextProfile() - transport type[%s], keep red level[%d]",
            pTextProfile->strTransportType.GetStr(), pTextProfile->bKeepRedLevel, 0);

    return pTextProfile;
}

PRIVATE TextProfile::Payload* MediaProfileFactory::CreateT140Payload(
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
                pRedFmtp->nRedLevel, pRedFmtp->nRedPayload, 0);

        pTextPayload->pFmtp = pRedFmtp;
    }
    else
    {
        strCodecName.Sprintf("%s", "t140");
    }

    pTextPayload->SetRtpMap(
            pT140Config->GetPayloadType(), strCodecName, pT140Config->GetSamplingRate());

    return pTextPayload;
}
