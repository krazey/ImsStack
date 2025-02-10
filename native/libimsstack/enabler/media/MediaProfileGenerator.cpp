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

#include "MediaEnvironment.h"
#include "MediaManager.h"
#include "MediaProfileFactory.h"
#include "MediaProfileGenerator.h"
#include "MediaResourceManager.h"
#include "config/CodecConfig.h"
#include "config/ImsCodec.h"
#include "config/MediaConfiguration.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC MediaProfileGenerator::MediaProfileGenerator(IN const MEDIA_CONTENT_TYPE eType) :
        m_eType(eType)
{
}

PUBLIC VIRTUAL MediaProfileGenerator::~MediaProfileGenerator() {}

PUBLIC
MediaBaseProfile* MediaProfileGenerator::Generate(
        IN MediaEnvironment* pEnvironment, IN MediaConfiguration* pConfig, IN IMS_SINT32 nSlotId)
{
    if (pEnvironment == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    IMS_TRACE_I("Generate() media type[%d]", m_eType, 0, 0);

    MediaBaseProfile* pProfile = IMS_NULL;

    pProfile = MediaProfileFactory::GetInstance()->CreateProfile(m_eType);
    pProfile = SetPayloads(pProfile, pConfig);
    pProfile = SetProfile(pProfile, pConfig, pEnvironment, nSlotId);

    return pProfile;
}

PRIVATE MediaBaseProfile* MediaProfileGenerator::SetPayloads(
        IN MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_NULL;
    }

    ImsList<CodecConfig*> pCodecs;
    pCodecs = pConfig->GetCodecConfigs();

    for (IMS_UINT32 i = 0; i < pCodecs.GetSize(); i++)
    {
        CodecConfig* pCodecConfig = pCodecs.GetAt(i);

        if (pCodecConfig == IMS_NULL)
        {
            IMS_TRACE_D("CreateCodecPayloads() - pCodecConfig is NULL", 0, 0, 0);
            break;
        }

        IMS_SINT32 nCodec = pCodecConfig->GetCodec();

        if (pCodecConfig->GetPayloadType() == -1)
        {
            IMS_TRACE_D("CreateCodecPayloads() - invalid payload type, skip config[%d] - %d:%s", i,
                    nCodec, ImsCodec::CodecToString(nCodec));
            continue;
        }

        CreateCodecPayloads(pProfile, nCodec, pCodecConfig, pConfig);
    }

    return pProfile;
}

PROTECTED
void MediaProfileGenerator::SetCommonProfile(IN MediaBaseProfile* pProfile,
        IN MediaConfiguration* pConfig, IN MediaEnvironment* pEnvironment, IN IMS_SINT32 nSlotId)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL || pEnvironment == IMS_NULL ||
            pEnvironment->pIService == IMS_NULL)
    {
        return;
    }

    MediaManager* pMediaManager = MediaManager::GetInstance(nSlotId);

    if (pMediaManager == IMS_NULL)
    {
        return;
    }

    MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();

    if (pResourceMngr == IMS_NULL)
    {
        return;
    }

    pProfile->SetIpAddress(pEnvironment->pIService->GetIpAddress());
    pProfile->SetDataPort(pResourceMngr->AcquireRtpPort(pConfig));
    pProfile->SetControlPort(pProfile->GetDataPort() + 1);
    pProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    pProfile->SetBandwidthAs(pConfig->GetAsBandwidthKbps());
    pProfile->SetBandwidthRr(pConfig->GetRrBandwidthBps());
    pProfile->SetBandwidthRs(pConfig->GetRsBandwidthBps());
    pProfile->SetRtcpInterval(pConfig->GetRtcpIntervalOnHold());

    IMS_TRACE_I("SetCommonProfile() - IpAddress[%s], rtp port[%d], rtcp port[%d]",
            pProfile->GetIpAddress().ToCharString(), pProfile->GetDataPort(),
            pProfile->GetControlPort());
    IMS_TRACE_I("SetCommonProfile() - AS[%d], RR[%d], RS[%d]", pProfile->GetBandwidthAs(),
            pProfile->GetBandwidthRr(), pProfile->GetBandwidthRs());
    IMS_TRACE_I("SetCommonProfile() - direction[%d], rtcp Interval[%d]", pProfile->GetDirection(),
            pProfile->GetRtcpInterval(), 0);
}
