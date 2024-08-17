// Copyright 2024 Google LLC

#include "ServiceTrace.h"

#include "SdpNegotiator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC SdpNegotiator::SdpNegotiator(IN const MEDIA_CONTENT_TYPE eType)
{
    IMS_TRACE_I("+SdpNegotiator() media type[%d]", eType, 0, 0);
    m_eType = eType;
}

PUBLIC VIRTUAL SdpNegotiator::~SdpNegotiator()
{
    IMS_TRACE_I("~SdpNegotiator()", 0, 0, 0);
}

PROTECTED
IMS_BOOL SdpNegotiator::NegotiateIpPort(IN MediaBaseProfile* pLocalProfile,
        IN MediaBaseProfile* pPeerProfile, OUT MediaBaseProfile* pNegotiatedProfile)
{
    if (pLocalProfile == IMS_NULL || pPeerProfile == IMS_NULL || pNegotiatedProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pNegotiatedProfile->SetIpAddress(pLocalProfile->GetIpAddress());

    IMS_TRACE_D("NegotiateIpPort() media[%d] - IP Address negotiated[%s], DestPayloadSize[%d]",
            m_eType, pNegotiatedProfile->GetIpAddress().ToCharString(),
            pPeerProfile->GetPayloadList().GetSize());

    // Setting RTP/RTCP port of mine
    pNegotiatedProfile->SetDataPort(pLocalProfile->GetDataPort());
    pNegotiatedProfile->SetControlPort(pLocalProfile->GetControlPort());

    if (pNegotiatedProfile->GetDataPort() == 0 || pPeerProfile->GetDataPort() == 0)
    {
        IMS_TRACE_D("NegotiateIpPort() ZERO Port. DO NOT Use the media type[%d], negotiated "
                    "port[%d], peer port[%d]",
                m_eType, pNegotiatedProfile->GetDataPort(), pPeerProfile->GetDataPort());
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
