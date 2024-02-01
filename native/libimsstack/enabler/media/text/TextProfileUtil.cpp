/**
 * Copyright (C) 2022 The Android Open Source Project
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
#include "text/TextDef.h"
#include "text/TextProfileUtil.h"
#include "config/CodecT140Config.h"
#include "MediaManager.h"

__IMS_TRACE_TAG_USER_DECL__("MED.TU");

PUBLIC GLOBAL TextProfile* TextProfileUtil::CreateProfile(
        IN MediaEnvironment* pEnvironment, IN TextConfiguration* pConfig, IN IMS_SINT32 nSlotId)
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

    TextProfile* pTextProfile = new TextProfile();

    // Setting IP address
    pTextProfile->objIpAddress = pEnvironment->pIService->GetIpAddress();

    if (pTextProfile->nDataPort == 0)
    {
        pTextProfile->nDataPort = pResourceMngr->AcquireRtpPort(pConfig);
        pTextProfile->nControlPort = pTextProfile->nDataPort + 1;
    }

    IMS_TRACE_I("CreateProfile() - IpAddress[%s], port[%d]",
            pTextProfile->objIpAddress.ToCharString(), pTextProfile->nDataPort, 0);

    // Setting profile type
    pTextProfile->strTransportType = "RTP/AVP";  // Text uses a default.

    while (pTextProfile->lstPayload.GetSize() > 0)
    {
        TextProfile::Payload* pPayload = pTextProfile->lstPayload.GetAt(0);

        if (pPayload != IMS_NULL)
        {
            delete pPayload;
        }

        pTextProfile->lstPayload.RemoveAt(0);
    }

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
            CodecT140Config* pT140Config = reinterpret_cast<CodecT140Config*>(pCodecConfig);
            AString strCodecName;

            TextProfile::Payload* pT140Payload = new TextProfile::Payload();

            if (pCodecConfig->GetCodec() == ImsCodec::TEXT_RED)
            {
                strCodecName.Sprintf("%s", "red");
                TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp(
                        pT140Config->GetRedLevel(), pConfig->GetT140PayloadType());
                IMS_TRACE_I("CreateProfile() add fmtp(%d) - nRedLevel(%d), nRedPayload(%d)", i,
                        pRedFmtp->nRedLevel, pRedFmtp->nRedPayload);
                pT140Payload->pFmtp = pRedFmtp;
            }
            else
            {
                strCodecName.Sprintf("%s", "t140");
            }

            pT140Payload->SetRtpMap(
                    pT140Config->GetPayloadType(), strCodecName, pT140Config->GetSamplingRate());

            pTextProfile->lstPayload.Append(pT140Payload);

            IMS_TRACE_I("CreateProfile() add payload(%d) - %s, %d", i, strCodecName.GetStr(),
                    pT140Config->GetSamplingRate());
        }
        else
        {
            IMS_TRACE_E(0, "CreateProfile() - Invalid Codec(%d)", pCodecConfig->GetCodec(), 0, 0);
            delete pTextProfile;
            return IMS_NULL;
        }
    }

    // Setting direction
    pTextProfile->eDirection = MEDIA_DIRECTION_SEND_RECEIVE;
    pTextProfile->nBandwidthAs = pConfig->GetAsBandwidthKbps();
    pTextProfile->nBandwidthRr = pConfig->GetRrBandwidthBps();
    pTextProfile->nBandwidthRs = pConfig->GetRsBandwidthBps();
    pTextProfile->nRtcpInterval = pConfig->GetRtcpInterval();
    pTextProfile->bKeepRedLevel = pConfig->IsTextCodecEmptyRedundantEnabled();

    IMS_TRACE_I("CreateProfile() - direction[%d], rtcp Interval[%d], keepRedLevel[%d]",
            pTextProfile->eDirection, pTextProfile->nRtcpInterval, pTextProfile->bKeepRedLevel);
    IMS_TRACE_I("CreateProfile() - AS[%d], RR[%d], RS[%d]", pTextProfile->nBandwidthAs,
            pTextProfile->nBandwidthRr, pTextProfile->nBandwidthRs);
    return pTextProfile;
}

void TextProfileUtil::MakeNegotiatedBandwidth(IN TextConfiguration* pConfig,
        IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        IN IMS_SINT32 nASValueOfNegoticatedCodec, OUT TextProfile* pNegotiatedProfile)
{
    (void)pConfig;

    IMS_TRACE_D("MakeNegotiatedBandwidth() - bIsOfferReceived[%d]", bIsOfferReceived, 0, 0);

    if (bIsOfferReceived == IMS_FALSE)
    {
        if (pPeerProfile->nBandwidthAs > 0)
        {
            pNegotiatedProfile->nBandwidthAs = pPeerProfile->nBandwidthAs;
        }
        else
        {
            pNegotiatedProfile->nBandwidthAs = pLocalProfile->nBandwidthAs;
        }

        if (pNegotiatedProfile->nBandwidthRs < 0 || pNegotiatedProfile->nBandwidthRr < 0)
        {
            pNegotiatedProfile->nBandwidthRs = pLocalProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pLocalProfile->nBandwidthRr;

            IMS_TRACE_D("MakeNegotiatedProfile() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
                    pLocalProfile->nBandwidthAs, pLocalProfile->nBandwidthRs,
                    pLocalProfile->nBandwidthRr);
        }

        pNegotiatedProfile->nBandwidthRs = pLocalProfile->nBandwidthRs;
        pNegotiatedProfile->nBandwidthRr = pLocalProfile->nBandwidthRr;
    }
    else
    {
        if (nASValueOfNegoticatedCodec > 0)
        {
            pNegotiatedProfile->nBandwidthAs = nASValueOfNegoticatedCodec;

            if (nASValueOfNegoticatedCodec > pPeerProfile->nBandwidthAs &&
                    pPeerProfile->nBandwidthAs > 0)
            {
                pNegotiatedProfile->nBandwidthAs = pPeerProfile->nBandwidthAs;
            }
        }
        else
        {
            pNegotiatedProfile->nBandwidthAs = pLocalProfile->nBandwidthAs;
        }

        // Exception Handling (b=RS/RR line is not included in Answer SDP)
        if (pNegotiatedProfile->nBandwidthRs < 0 || pNegotiatedProfile->nBandwidthRr < 0)
        {
            pNegotiatedProfile->nBandwidthRs = pLocalProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pLocalProfile->nBandwidthRr;

            IMS_TRACE_D("MakeNegotiatedBandwidth() - AS[%d] RS[%d] RR[%d]",
                    pLocalProfile->nBandwidthAs, pLocalProfile->nBandwidthRs,
                    pLocalProfile->nBandwidthRr);
            return;
        }

        // Dest RS & RR == Zero case, rtcp should be disable and RS & RR == Zero in IR.92
        // release 12.
        if (pPeerProfile->nBandwidthRs == 0 && pPeerProfile->nBandwidthRr == 0)
        {
            pNegotiatedProfile->nBandwidthRs = pPeerProfile->nBandwidthRs;
            pNegotiatedProfile->nBandwidthRr = pPeerProfile->nBandwidthRr;

            IMS_TRACE_D("MakeNegotiatedProfile() - AS[%d], RTCP disable & use dest RS[%d] RR[%d]",
                    pNegotiatedProfile->nBandwidthAs, pNegotiatedProfile->nBandwidthRs,
                    pNegotiatedProfile->nBandwidthRr);

            return;
        }

        pNegotiatedProfile->nBandwidthRs = pLocalProfile->nBandwidthRs;
        pNegotiatedProfile->nBandwidthRr = pLocalProfile->nBandwidthRr;
    }

    IMS_TRACE_D("MakeNegotiatedProfile() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
            pLocalProfile->nBandwidthAs, pLocalProfile->nBandwidthRs, pLocalProfile->nBandwidthRr);
}