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

    IMS_TRACE_I("Negotiate()", 0, 0, 0);

    // Setting IP of mine
    pNegotiatedProfile->SetIpAddress(pLocalProfile->GetIpAddress());

    IMS_TRACE_D("Negotiate() - local address[%s] PeerPayloadSize[%d]",
            pLocalProfile->GetIpAddress().ToCharString(), pPeerProfile->GetPayloadList().GetSize(),
            0);

    // Setting Rtp/RTCP port of mine
    pNegotiatedProfile->SetDataPort(pLocalProfile->GetDataPort());
    pNegotiatedProfile->SetControlPort(pLocalProfile->GetControlPort());

    if (pNegotiatedProfile->GetDataPort() == 0 || pPeerProfile->GetDataPort() == 0)
    {
        *pNegotiatedProfile =
                (pPeerProfile->GetPayloadList().GetSize() > 0) ? *pPeerProfile : *pLocalProfile;

        pNegotiatedProfile->SetIpAddress(pLocalProfile->GetIpAddress());
        pNegotiatedProfile->SetDataPort(0);

        IMS_TRACE_D("Negotiate() - ZERO Port. DO NOT Use the text[%d][%d],\
                But nego is successful",
                pNegotiatedProfile->GetDataPort(), pPeerProfile->GetDataPort(), 0);
        return IMS_TRUE;
    }

    if (pConfig == IMS_NULL)
    {
        IMS_TRACE_D("Negotiate() - no config, return true to reject", 0, 0, 0);
        return IMS_TRUE;
    }

    // Compare each payload based destination's profile
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
                TextProfile::Payload* pT140 = new TextProfile::Payload();
                pT140->SetRtpMap(pPayload->GetRtpMap());

                if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"))
                {
                    pT140->SetFmtp(new TextProfile::RedFmtp(
                            *static_cast<TextProfile::RedFmtp*>(pPayload->GetFmtp())));
                }

                pNegotiatedProfile->GetPayloadList().Append(pT140);
            }
        }
    }

    IMS_BOOL bRet = IMS_FALSE;

    if (pNegotiatedProfile->GetPayloadList().GetSize() > 0)
    {
        // Setting direction
        if (pNegotiatedProfile->GetDataPort() != 0 && pPeerProfile->GetDataPort() != 0)
        {
            pNegotiatedProfile->SetDirection(UpdateDirectionToMine(
                    pPeerProfile->GetDirection(), pLocalProfile->GetDirection(), bIsOfferReceived));
        }
        else
        {
            pNegotiatedProfile->SetDirection(MEDIA_DIRECTION_INVALID);
        }

        TextProfileUtil::MakeNegotiatedBandwidth(static_cast<TextConfiguration*>(pConfig),
                pLocalProfile, pPeerProfile, bIsOfferReceived, -1, pNegotiatedProfile);
        bRet = IMS_TRUE;
    }
    else
    {
        if (pLocalProfile->GetPayloadList().GetSize() > 0)
        {
            IMS_TRACE_D("Negotiate() - no negotiated payload. use the LocalProfile and "
                        "make port 0 ",
                    0, 0, 0);
            *pNegotiatedProfile = *pLocalProfile;
            bRet = IMS_TRUE;
        }
        else
        {
            IMS_TRACE_E(0, "There's no Payload in LocalProfile", 0, 0, 0);
        }

        pNegotiatedProfile->SetDataPort(0);
        pNegotiatedProfile->SetDirection(MEDIA_DIRECTION_INVALID);
    }

    IMS_TRACE_D("Negotiate() - Direction=%d, nego rs=%d, rr=%d", pNegotiatedProfile->GetDirection(),
            pNegotiatedProfile->GetBandwidthRs(), pNegotiatedProfile->GetBandwidthRr());

    if (pNegotiatedProfile->GetBandwidthRs() == 0 && pNegotiatedProfile->GetBandwidthRr() == 0)
    {
        pNegotiatedProfile->SetRtcpInterval(0);
        IMS_TRACE_D("Negotiate() - negotiated rs and rr are 0, disable rtcp", 0, 0, 0);
    }
    else
    {
        pNegotiatedProfile->SetRtcpInterval(pConfig->GetRtcpInterval());
    }

    IMS_TRACE_D("Negotiate() - negotiated payload size[%d], port[%d], direction[%d], ",
            pNegotiatedProfile->GetPayloadList().GetSize(), pNegotiatedProfile->GetDataPort(),
            pNegotiatedProfile->GetDirection());
    return bRet;
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

PRIVATE MEDIA_DIRECTION TextSdpNegotiator::UpdateDirectionToMine(IN MEDIA_DIRECTION ePeerDirection,
        IN MEDIA_DIRECTION eLocalDirection, IN IMS_BOOL bIsMtCase)
{
    if (ePeerDirection < MEDIA_DIRECTION_INACTIVE || ePeerDirection > MEDIA_DIRECTION_SEND_RECEIVE)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_TRACE_D("UpdateDirectionToMine() - Entered. ePeerDirection[%d], eLocalDirection[%d], "
                "bIsMtCase[%d]",
            ePeerDirection, eLocalDirection, bIsMtCase);

    if (bIsMtCase == IMS_FALSE)
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
