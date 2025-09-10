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

#include "text/TextProfileNegotiator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextProfileNegotiator::TextProfileNegotiator() :
        MediaProfileNegotiator(MEDIA_TYPE_TEXT)
{
    IMS_TRACE_I("+TextProfileNegotiator()", 0, 0, 0);
}

PUBLIC VIRTUAL TextProfileNegotiator::~TextProfileNegotiator()
{
    IMS_TRACE_I("~TextProfileNegotiator()", 0, 0, 0);
}

PUBLIC IMS_BOOL TextProfileNegotiator::Negotiate(IN TextProfile* pLocalProfile,
        IN TextProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        OUT TextProfile* pNegotiatedProfile, IN MediaConfiguration* pConfig)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL ||
            pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Negotiate(): invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    m_bIsOfferReceived = bIsOfferReceived;

    IMS_TRACE_I("Negotiate(): IsOfferReceived[%d]", m_bIsOfferReceived, 0, 0);

    if (NegotiateIpPort(pLocalProfile, pPeerProfile, pNegotiatedProfile) != IMS_TRUE)
    {
        ResetNegotiatedProfile(IMS_TRUE, pLocalProfile, pPeerProfile, &pNegotiatedProfile);
        return IMS_TRUE;
    }

    IMS_BOOL bPayloadNegotiated = NegotiatePayload(pLocalProfile, pPeerProfile, pNegotiatedProfile);
    IMS_BOOL bRet = IMS_TRUE;

    if (bPayloadNegotiated)
    {
        NegotiateDirection(pLocalProfile, pPeerProfile, pNegotiatedProfile);
        NegotiateBandwidth(pLocalProfile, pPeerProfile, -1, pNegotiatedProfile);
    }
    else
    {
        IMS_TRACE_D("Negotiate(): no negotiated payload. use the LocalProfile and make port 0", 0,
                0, 0);

        bRet = ResetNegotiatedProfile(IMS_FALSE, pLocalProfile, pPeerProfile, &pNegotiatedProfile);
    }

    NegotiateRtcpInterval(pNegotiatedProfile, pConfig);

    IMS_TRACE_D("Negotiate(): negotiated payload size[%d], port[%d], direction[%d], ",
            pNegotiatedProfile->GetPayloadList().GetSize(), pNegotiatedProfile->GetDataPort(),
            pNegotiatedProfile->GetDirection());

    return bRet;
}

PRIVATE
IMS_BOOL TextProfileNegotiator::ResetNegotiatedProfile(IN IMS_BOOL bPeerPreferred,
        IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
        OUT TextProfile** pNegotiatedProfile)
{
    IMS_BOOL bRet = IMS_FALSE;

    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ResetNegotiatedProfile(): invalid argument", 0, 0, 0);
        return bRet;
    }

    if (bPeerPreferred)
    {
        IMS_TRACE_D("ResetNegotiatedProfile(): by Peer Profile payload size[%d]",
                pPeerProfile->GetPayloadList().GetSize(), 0, 0);

        **pNegotiatedProfile =
                (pPeerProfile->GetPayloadList().GetSize() > 0) ? *pPeerProfile : *pLocalProfile;

        (*pNegotiatedProfile)->SetIpAddress(pLocalProfile->GetIpAddress());
    }
    else
    {
        IMS_TRACE_D("ResetNegotiatedProfile(): by Local Profile payload size[%d]",
                pLocalProfile->GetPayloadList().GetSize(), 0, 0);

        if (pLocalProfile->GetPayloadList().GetSize() > 0)
        {
            **pNegotiatedProfile = *pLocalProfile;
            bRet = IMS_TRUE;
        }
    }

    (*pNegotiatedProfile)->SetDataPort(0);
    (*pNegotiatedProfile)->SetDirection(MEDIA_DIRECTION_INVALID);

    return bRet;
}

PRIVATE
IMS_BOOL TextProfileNegotiator::NegotiatePayload(IN TextProfile* pLocalProfile,
        IN TextProfile* pPeerProfile, OUT TextProfile* pNegotiatedProfile)
{
    IMS_BOOL bRet = IMS_FALSE;

    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiatePayload(): invalid argument", 0, 0, 0);
        return bRet;
    }

    for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadList().GetSize(); i++)
    {
        TextProfile::Payload* pPeerPayload = pPeerProfile->GetPayloadAt(i);

        if (pPeerPayload == IMS_NULL)
        {
            continue;
        }

        if (pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140") ||
                pPeerPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
        {
            TextProfile::Payload* pLocalPayload = FindT140InProfile(pLocalProfile, pPeerPayload);
            if (pLocalPayload != IMS_NULL)
            {
                TextProfile::Payload* pNegotiatedPayload = CreatePayload(pPeerPayload->GetRtpMap(),
                        pPeerPayload->GetFmtp() == IMS_NULL ? pLocalPayload->GetFmtp()
                                                            : pPeerPayload->GetFmtp());
                pNegotiatedProfile->AddPayload(pNegotiatedPayload);
                bRet = IMS_TRUE;
            }
        }
    }

    return bRet;
}

PRIVATE
TextProfile::Payload* TextProfileNegotiator::CreatePayload(
        IN const MediaBaseProfile::RtpMap& objRtpMap,
        IN std::shared_ptr<TextProfile::TextFmtp> pFmtp)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    pPayload->SetRtpMap(objRtpMap);

    if (pFmtp != IMS_NULL)
    {
        if (objRtpMap.GetPayloadType().EqualsIgnoreCase("red"))
        {
            pPayload->SetFmtp(std::make_shared<TextProfile::RedFmtp>(
                    *std::static_pointer_cast<TextProfile::RedFmtp>(pFmtp)));
        }
        else if (objRtpMap.GetPayloadType().EqualsIgnoreCase("t140"))
        {
            pPayload->SetFmtp(std::make_shared<TextProfile::T140Fmtp>(
                    *std::static_pointer_cast<TextProfile::T140Fmtp>(pFmtp)));
        }
    }

    return pPayload;
}

PRIVATE
void TextProfileNegotiator::NegotiateDirection(IN TextProfile* pLocalProfile,
        IN TextProfile* pPeerProfile, OUT TextProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateDirection(): invalid argument", 0, 0, 0);
        return;
    }

    pNegotiatedProfile->SetDirection(
            UpdateDirectionToMine(pPeerProfile->GetDirection(), pLocalProfile->GetDirection()));
    IMS_TRACE_D("NegotiateDirection(): direction[%d]", pNegotiatedProfile->GetDirection(), 0, 0);
}

PRIVATE
void TextProfileNegotiator::NegotiateBandwidth(IN TextProfile* pLocalProfile,
        IN TextProfile* pPeerProfile, IN IMS_SINT32 nAsValueOfNegotiatedCodec,
        OUT TextProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateBandwidth(): invalid argument", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("NegotiateBandwidth(): Received RS[%d] and RR[%d]", pPeerProfile->GetBandwidthRs(),
            pPeerProfile->GetBandwidthRr(), 0);
    pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
    pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());

    m_bIsOfferReceived
            ? NegotiateBandwidthForOfferReceived(
                      pLocalProfile, pPeerProfile, nAsValueOfNegotiatedCodec, pNegotiatedProfile)
            : NegotiateBandwidthForOfferSent(pLocalProfile, pPeerProfile, pNegotiatedProfile);
}

PRIVATE
void TextProfileNegotiator::NegotiateBandwidthForOfferReceived(IN TextProfile* pLocalProfile,
        IN TextProfile* pPeerProfile, IN IMS_SINT32 nAsValueOfNegotiatedCodec,
        OUT TextProfile* pNegotiatedProfile)
{
    if (nAsValueOfNegotiatedCodec > 0)
    {
        pNegotiatedProfile->SetBandwidthAs(nAsValueOfNegotiatedCodec);

        if (nAsValueOfNegotiatedCodec > pPeerProfile->GetBandwidthAs() &&
                pPeerProfile->GetBandwidthAs() > 0)
        {
            pNegotiatedProfile->SetBandwidthAs(pPeerProfile->GetBandwidthAs());
        }
    }
    else
    {
        pNegotiatedProfile->SetBandwidthAs(pLocalProfile->GetBandwidthAs());
    }

    // Exception Handling (b=RS/RR line is not included in Answer SDP)
    if (pNegotiatedProfile->GetBandwidthRs() < 0 || pNegotiatedProfile->GetBandwidthRr() < 0)
    {
        pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
        pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());

        IMS_TRACE_D("NegotiateBandwidthForOfferReceived(): AS[%d], RS[%d], RR[%d]",
                pNegotiatedProfile->GetBandwidthAs(), pNegotiatedProfile->GetBandwidthRs(),
                pNegotiatedProfile->GetBandwidthRr());
        return;
    }

    // Dest RS & RR == Zero case, rtcp should be disable and RS & RR == Zero in IR.92
    // release 12.
    if (pPeerProfile->GetBandwidthRs() == 0 && pPeerProfile->GetBandwidthRr() == 0)
    {
        pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
        pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());

        IMS_TRACE_D("NegotiateBandwidthForOfferReceived(): AS[%d], RS[%d], RR[%d]",
                pNegotiatedProfile->GetBandwidthAs(), pNegotiatedProfile->GetBandwidthRs(),
                pNegotiatedProfile->GetBandwidthRr());
    }
}

PRIVATE
void TextProfileNegotiator::NegotiateBandwidthForOfferSent(IN TextProfile* pLocalProfile,
        IN TextProfile* pPeerProfile, OUT TextProfile* pNegotiatedProfile)
{
    if (pPeerProfile->GetBandwidthAs() > 0)
    {
        pNegotiatedProfile->SetBandwidthAs(pPeerProfile->GetBandwidthAs());
    }
    else
    {
        pNegotiatedProfile->SetBandwidthAs(pLocalProfile->GetBandwidthAs());
    }

    if (pNegotiatedProfile->GetBandwidthRs() < 0 || pNegotiatedProfile->GetBandwidthRr() < 0)
    {
        pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
        pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());

        IMS_TRACE_D("NegotiateBandwidthForOfferSent(): AS[%d], RS[%d], RR[%d]",
                pNegotiatedProfile->GetBandwidthAs(), pNegotiatedProfile->GetBandwidthRs(),
                pNegotiatedProfile->GetBandwidthRr());
    }
}

PRIVATE
void TextProfileNegotiator::NegotiateRtcpInterval(
        OUT TextProfile* pNegotiatedProfile, IN const MediaConfiguration* pConfig)
{
    if (pNegotiatedProfile == IMS_NULL || pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "NegotiateRtcpInterval(): invalid argument", 0, 0, 0);
        return;
    }

    if (pNegotiatedProfile->GetBandwidthRs() == 0 && pNegotiatedProfile->GetBandwidthRr() == 0)
    {
        pNegotiatedProfile->SetRtcpInterval(0);
    }
    else
    {
        pNegotiatedProfile->SetRtcpInterval(pConfig->GetRtcpIntervalOnHold());

        // TODO : need to check RtcpIntervalOnActive is needed for TextSession
        if (pNegotiatedProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE &&
                pConfig->GetRtcpIntervalOnActive() > 0)
        {
            pNegotiatedProfile->SetRtcpInterval(pConfig->GetRtcpIntervalOnActive());
        }
    }
}

PRIVATE TextProfile::Payload* TextProfileNegotiator::FindT140InProfile(
        IN TextProfile* pProfile, IN TextProfile::Payload* pPayload)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "FindT140InProfile(): invalid argument", 0, 0, 0);
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        TextProfile::Payload* pComparedPayload = pProfile->GetPayloadAt(i);

        if (pComparedPayload == IMS_NULL)
        {
            continue;
        }

        if (pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140"))
        {
            if (pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()) &&
                    pComparedPayload->GetRtpMap().GetSamplingRate() ==
                            pPayload->GetRtpMap().GetSamplingRate())
            {
                IMS_TRACE_D("FindT140InProfile(): Found T140 at [%d], Codec[%s]", i,
                        pComparedPayload->GetRtpMap().GetPayloadType().GetStr(), 0);
                return pComparedPayload;
            }
        }
        else if (pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
        {
            if (pComparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()) &&
                    pComparedPayload->GetRtpMap().GetSamplingRate() ==
                            pPayload->GetRtpMap().GetSamplingRate())
            {
                auto pComparedFmtp =
                        std::static_pointer_cast<TextProfile::RedFmtp>(pComparedPayload->GetFmtp());
                auto pReceivedFmtp =
                        std::static_pointer_cast<TextProfile::RedFmtp>(pPayload->GetFmtp());

                if (pComparedFmtp == IMS_NULL || pReceivedFmtp == IMS_NULL)
                {
                    IMS_TRACE_I("FindT140InProfile(): No fmtp to compare", 0, 0, 0);
                }

                IMS_TRACE_D("FindT140InProfile(): Found RED at [%d], Codec[%s]", i,
                        pComparedPayload->GetRtpMap().GetPayloadType().GetStr(), 0);
                return pComparedPayload;
            }
        }
    }

    return IMS_NULL;
}

PRIVATE MEDIA_DIRECTION TextProfileNegotiator::UpdateDirectionToMine(
        IN MEDIA_DIRECTION ePeerDirection, IN MEDIA_DIRECTION eLocalDirection)
{
    if (ePeerDirection < MEDIA_DIRECTION_INACTIVE || ePeerDirection > MEDIA_DIRECTION_SEND_RECEIVE)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_D("UpdateDirectionToMine(): Entered. ePeerDirection[%d], eLocalDirection[%d]",
            ePeerDirection, eLocalDirection, 0);

    if (!m_bIsOfferReceived)
    {
        return eLocalDirection;
    }

    switch (ePeerDirection)
    {
        case MEDIA_DIRECTION_INACTIVE:  // FALL_THROUGH
        case MEDIA_DIRECTION_SEND_RECEIVE:
            return ePeerDirection;
        case MEDIA_DIRECTION_RECEIVE:
            return MEDIA_DIRECTION_SEND;
        case MEDIA_DIRECTION_SEND:
            return MEDIA_DIRECTION_RECEIVE;
        default:
            return MEDIA_DIRECTION_INVALID;
    }
}
