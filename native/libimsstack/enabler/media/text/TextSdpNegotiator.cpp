// Copyright 2024 Google LLC

#include "ServiceTrace.h"

#include "config/TextConfiguration.h"
#include "text/TextSdpNegotiator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextSdpNegotiator::TextSdpNegotiator() :
        SdpNegotiator(MEDIA_TYPE_TEXT)
{
    IMS_TRACE_I("+TextSdpNegotiator()", 0, 0, 0);
}

PUBLIC VIRTUAL TextSdpNegotiator::~TextSdpNegotiator()
{
    IMS_TRACE_I("~TextSdpNegotiator()", 0, 0, 0);
}

PUBLIC IMS_BOOL TextSdpNegotiator::Negotiate(IN TextProfile* pLocalProfile,
        IN TextProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
        OUT TextProfile* pNegotiatedProfile, IN MediaConfiguration* pConfig)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL ||
            pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Negotiate() invalid argument", 0, 0, 0);
        return IMS_FALSE;
    }

    m_bIsOfferReceived = bIsOfferReceived;

    IMS_TRACE_I("Negotiate() - IsOfferReceived[%d]", m_bIsOfferReceived, 0, 0);

    if (NegotiateIpPort(pLocalProfile, pPeerProfile, pNegotiatedProfile) != IMS_TRUE)
    {
        ResetNegotiatedProfile(IMS_TRUE, pLocalProfile, pPeerProfile, pNegotiatedProfile);
        return IMS_TRUE;
    }

    IMS_BOOL bPayloadNegotiated = NegotiatePayload(pLocalProfile, pPeerProfile, pNegotiatedProfile);

    IMS_BOOL bRet = IMS_FALSE;

    if (bPayloadNegotiated)
    {
        NegotiateDirection(pLocalProfile, pPeerProfile, pNegotiatedProfile);
        NegotiateBandwidth(pLocalProfile, pPeerProfile, -1, pNegotiatedProfile);

        bRet = IMS_TRUE;
    }
    else
    {
        IMS_TRACE_D("Negotiate() - no negotiated payload. use the LocalProfile and make port 0", 0,
                0, 0);

        bRet = ResetNegotiatedProfile(IMS_FALSE, pLocalProfile, pPeerProfile, pNegotiatedProfile);
    }

    NegotiateRtcpInterval(pNegotiatedProfile, pConfig);

    IMS_TRACE_D("Negotiate() - negotiated payload size[%d], port[%d], direction[%d], ",
            pNegotiatedProfile->GetPayloadList().GetSize(), pNegotiatedProfile->GetDataPort(),
            pNegotiatedProfile->GetDirection());

    return bRet;
}

PRIVATE
IMS_BOOL TextSdpNegotiator::ResetNegotiatedProfile(IN IMS_BOOL bPeerPreferred,
        IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
        OUT TextProfile* pNegotiatedProfile)
{
    IMS_BOOL bRet = IMS_FALSE;

    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return bRet;
    }

    if (bPeerPreferred)
    {
        IMS_TRACE_D("ResetNegotiatedProfile() - by Peer Profile payload size[%d]",
                pPeerProfile->GetPayloadList().GetSize(), 0, 0);

        *pNegotiatedProfile =
                (pPeerProfile->GetPayloadList().GetSize() > 0) ? *pPeerProfile : *pLocalProfile;

        pNegotiatedProfile->SetIpAddress(pLocalProfile->GetIpAddress());
        pNegotiatedProfile->SetDataPort(0);
    }
    else
    {
        IMS_TRACE_D("ResetNegotiatedProfile() - by Local Profile payload size[%d]",
                pLocalProfile->GetPayloadList().GetSize(), 0, 0);

        if (pLocalProfile->GetPayloadList().GetSize() > 0)
        {
            *pNegotiatedProfile = *pLocalProfile;
            bRet = IMS_TRUE;
        }

        pNegotiatedProfile->SetDataPort(0);
        pNegotiatedProfile->SetDirection(MEDIA_DIRECTION_INVALID);
    }

    return bRet;
}

PRIVATE
IMS_BOOL TextSdpNegotiator::NegotiatePayload(IN TextProfile* pLocalProfile,
        IN TextProfile* pPeerProfile, OUT TextProfile* pNegotiatedProfile)
{
    IMS_BOOL bRet = IMS_FALSE;

    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return bRet;
    }

    for (IMS_UINT32 i = 0; i < pPeerProfile->GetPayloadList().GetSize(); i++)
    {
        TextProfile::Payload* pPayload = pPeerProfile->GetPayloadAt(i);

        if (pPayload == IMS_NULL)
        {
            continue;
        }

        if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140") ||
                pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
        {
            if (FindT140InProfile(pLocalProfile, pPayload) == IMS_TRUE)
            {
                AppendT140Payload(pPayload, pNegotiatedProfile);
                bRet = IMS_TRUE;
            }
        }
    }

    return bRet;
}
PRIVATE
void TextSdpNegotiator::AppendT140Payload(
        IN TextProfile::Payload* pPayload, OUT TextProfile* pNegotiatedProfile)
{
    TextProfile::Payload* pT140 = new TextProfile::Payload();

    if (pT140 == IMS_NULL)
    {
        return;
    }

    pT140->SetRtpMap(pPayload->GetRtpMap());

    if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
    {
        pT140->SetFmtp(
                new TextProfile::RedFmtp(*static_cast<TextProfile::RedFmtp*>(pPayload->GetFmtp())));
    }

    pNegotiatedProfile->GetPayloadList().Append(pT140);
}

PRIVATE
void TextSdpNegotiator::NegotiateDirection(IN TextProfile* pLocalProfile,
        IN TextProfile* pPeerProfile, OUT TextProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return;
    }

    if (pNegotiatedProfile->GetDataPort() != 0 && pPeerProfile->GetDataPort() != 0)
    {
        pNegotiatedProfile->SetDirection(
                UpdateDirectionToMine(pPeerProfile->GetDirection(), pLocalProfile->GetDirection()));
    }
    else
    {
        pNegotiatedProfile->SetDirection(MEDIA_DIRECTION_INVALID);
    }

    IMS_TRACE_D("NegotiateDirection() - direction[%d]", pNegotiatedProfile->GetDirection(), 0, 0);
}

PRIVATE
void TextSdpNegotiator::NegotiateBandwidth(IN TextProfile* pLocalProfile,
        IN TextProfile* pPeerProfile, IN IMS_SINT32 nAsValueOfNegoticatedCodec,
        OUT TextProfile* pNegotiatedProfile)
{
    IMS_TRACE_D("NegotiateBandwidth()", 0, 0, 0);

    if (m_bIsOfferReceived == IMS_FALSE)
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

            IMS_TRACE_D("NegotiateBandwidth() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
                    pLocalProfile->GetBandwidthAs(), pLocalProfile->GetBandwidthRs(),
                    pLocalProfile->GetBandwidthRr());
        }

        pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
        pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());
    }
    else
    {
        if (nAsValueOfNegoticatedCodec > 0)
        {
            pNegotiatedProfile->SetBandwidthAs(nAsValueOfNegoticatedCodec);

            if (nAsValueOfNegoticatedCodec > pPeerProfile->GetBandwidthAs() &&
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

            IMS_TRACE_D("NegotiateBandwidth() - AS[%d] RS[%d] RR[%d]",
                    pLocalProfile->GetBandwidthAs(), pLocalProfile->GetBandwidthRs(),
                    pLocalProfile->GetBandwidthRr());
            return;
        }

        // Dest RS & RR == Zero case, rtcp should be disable and RS & RR == Zero in IR.92
        // release 12.
        if (pPeerProfile->GetBandwidthRs() == 0 && pPeerProfile->GetBandwidthRr() == 0)
        {
            pNegotiatedProfile->SetBandwidthRs(pPeerProfile->GetBandwidthRs());
            pNegotiatedProfile->SetBandwidthRr(pPeerProfile->GetBandwidthRr());

            IMS_TRACE_D("NegotiateBandwidth() - AS[%d], RTCP disable & use dest RS[%d] RR[%d]",
                    pNegotiatedProfile->GetBandwidthAs(), pNegotiatedProfile->GetBandwidthRs(),
                    pNegotiatedProfile->GetBandwidthRr());

            return;
        }

        pNegotiatedProfile->SetBandwidthRs(pLocalProfile->GetBandwidthRs());
        pNegotiatedProfile->SetBandwidthRr(pLocalProfile->GetBandwidthRr());
    }

    IMS_TRACE_D("MakeNegotiatedBandwidth() - Negotiated Profile AS[%d] RS[%d] RR[%d]",
            pLocalProfile->GetBandwidthAs(), pLocalProfile->GetBandwidthRs(),
            pLocalProfile->GetBandwidthRr());
}

PRIVATE
void TextSdpNegotiator::NegotiateRtcpInterval(
        OUT TextProfile* pNegotiatedProfile, IN MediaConfiguration* pConfig)
{
    if (pNegotiatedProfile == IMS_NULL || pConfig == IMS_NULL)
    {
        return;
    }

    if (pNegotiatedProfile->GetBandwidthRs() == 0 && pNegotiatedProfile->GetBandwidthRr() == 0)
    {
        pNegotiatedProfile->SetRtcpInterval(0);
        IMS_TRACE_D("NegotiateRtcpInterval() - negotiated rs and rr are 0, disable rtcp", 0, 0, 0);
    }
    else
    {
        pNegotiatedProfile->SetRtcpInterval(pConfig->GetRtcpInterval());
    }
}

PRIVATE IMS_BOOL TextSdpNegotiator::FindT140InProfile(
        IN TextProfile* pProfile, IN TextProfile::Payload* pPayload)
{
    if (pProfile == IMS_NULL || pPayload == IMS_NULL)
        return IMS_FALSE;

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        TextProfile::Payload* comparedPayload = pProfile->GetPayloadAt(i);

        if (comparedPayload == IMS_NULL)
        {
            continue;
        }

        if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140"))
        {
            if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()) &&
                    comparedPayload->GetRtpMap().GetSamplingRate() ==
                            pPayload->GetRtpMap().GetSamplingRate())
            {
                IMS_TRACE_D("FindT140InProfile() - Found T140 at [%d], Codec[%s]", i,
                        comparedPayload->GetRtpMap().GetPayloadType().GetStr(), 0);

                return IMS_TRUE;
            }
        }
        else if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
        {
            if (comparedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase(
                        pPayload->GetRtpMap().GetPayloadType()) &&
                    comparedPayload->GetRtpMap().GetSamplingRate() ==
                            pPayload->GetRtpMap().GetSamplingRate())
            {
                TextProfile::RedFmtp* pComparedFmtp =
                        (TextProfile::RedFmtp*)comparedPayload->GetFmtp();
                TextProfile::RedFmtp* pReceivedFmtp = (TextProfile::RedFmtp*)pPayload->GetFmtp();

                if (pReceivedFmtp == IMS_NULL)
                {
                    continue;
                }

                if (pReceivedFmtp->GetRedLevel() > pComparedFmtp->GetRedLevel() ||
                        pReceivedFmtp->GetRedPayload() < 0)
                {
                    continue;
                }

                IMS_TRACE_D("FindT140InProfile() - Found RED at [%d], Codec[%s]", i,
                        comparedPayload->GetRtpMap().GetPayloadType().GetStr(), 0);

                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE MEDIA_DIRECTION TextSdpNegotiator::UpdateDirectionToMine(
        IN MEDIA_DIRECTION ePeerDirection, IN MEDIA_DIRECTION eLocalDirection)
{
    if (ePeerDirection < MEDIA_DIRECTION_INACTIVE || ePeerDirection > MEDIA_DIRECTION_SEND_RECEIVE)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_D("UpdateDirectionToMine() - Entered. ePeerDirection[%d], eLocalDirection[%d]",
            ePeerDirection, eLocalDirection, 0);

    if (m_bIsOfferReceived == IMS_FALSE)
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
