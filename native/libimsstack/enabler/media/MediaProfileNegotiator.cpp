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

#include "MediaProfileNegotiator.h"
#include "MediaBaseProfile.h"
#include "MediaDef.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC MediaProfileNegotiator::MediaProfileNegotiator(IN const MEDIA_CONTENT_TYPE eType) :
        m_eType(eType),
        m_bIsOfferReceived(IMS_FALSE)
{
}

PUBLIC VIRTUAL MediaProfileNegotiator::~MediaProfileNegotiator() {}

PROTECTED
IMS_BOOL MediaProfileNegotiator::NegotiateIpPort(IN const MediaBaseProfile* pLocalProfile,
        IN const MediaBaseProfile* pPeerProfile, OUT MediaBaseProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateIpPort(): invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    pNegotiatedProfile->SetIpAddress(pLocalProfile->GetIpAddress());

    IMS_TRACE_D("NegotiateIpPort(): type[%d] - IP Address negotiated[%s], DestPayloadSize[%d]",
            m_eType, pNegotiatedProfile->GetIpAddress().ToCharString(),
            pPeerProfile->GetPayloadListSize());

    // Setting RTP/RTCP port of mine
    pNegotiatedProfile->SetDataPort(pLocalProfile->GetDataPort());
    pNegotiatedProfile->SetControlPort(pLocalProfile->GetControlPort());

    if (pPeerProfile->GetDataPort() == 0)
    {
        pNegotiatedProfile->SetDataPort(0);
        pNegotiatedProfile->SetControlPort(0);
    }

    IMS_TRACE_D("NegotiateIpPort(): type[%d], negotiatedPort[%d], peerPort[%d]", m_eType,
            pNegotiatedProfile->GetDataPort(), pPeerProfile->GetDataPort());

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL MediaProfileNegotiator::ResetNegotiatedProfile(IN IMS_BOOL bPeerPreferred,
        IN const MediaBaseProfile* pLocalProfile, IN const MediaBaseProfile* pPeerProfile,
        OUT MediaBaseProfile** pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ResetNegotiatedProfile(): type[%d], invalid argument", m_eType, 0, 0);
        return IMS_FALSE;
    }

    if (bPeerPreferred)
    {
        **pNegotiatedProfile = *pPeerProfile;
        (*pNegotiatedProfile)->SetIpAddress(pLocalProfile->GetIpAddress());
    }
    else
    {
        **pNegotiatedProfile = *pLocalProfile;
    }

    (*pNegotiatedProfile)->SetDataPort(0);
    (*pNegotiatedProfile)->SetDirection(MEDIA_DIRECTION_INACTIVE);
    (*pNegotiatedProfile)->SetNegotiatedPayloadIndex(-1);

    IMS_TRACE_D(
            "ResetNegotiatedProfile(): type[%d], peerPreferred[%d]", m_eType, bPeerPreferred, 0);
    return IMS_TRUE;
}
