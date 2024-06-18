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

#include "config/TextConfiguration.h"
#include "text/TextProfileUtil.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC GLOBAL void TextProfileUtil::MakeNegotiatedBandwidth(IN TextConfiguration* pConfig,
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
