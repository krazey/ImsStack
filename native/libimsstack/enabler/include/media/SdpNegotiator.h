// Copyright 2024 Google LLC

#ifndef SDP_NEGOTIATOR_H_
#define SDP_NEGOTIATOR_H_

#include "ISessionDescriptor.h"
#include "media/IMediaDescriptor.h"

#include "MediaBaseProfile.h"

class SdpNegotiator
{
public:
    explicit SdpNegotiator(IN const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_NOTUSED);
    virtual ~SdpNegotiator();

protected:
    IMS_BOOL NegotiateIpPort(IN MediaBaseProfile* pLocalProfile, IN MediaBaseProfile* pPeerProfile,
            OUT MediaBaseProfile* pNegotiatedProfile);

    MEDIA_CONTENT_TYPE m_eType;
    IMS_BOOL m_bIsOfferReceived;
};

#endif
