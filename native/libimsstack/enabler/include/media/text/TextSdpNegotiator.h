// Copyright 2024 Google LLC

#ifndef TEXT_SDP_NEGOTIATOR_H_
#define TEXT_SDP_NEGOTIATOR_H_

#include "SdpNegotiator.h"
#include "text/TextProfileUtil.h"

class MediaConfiguration;

class TextSdpNegotiator : public SdpNegotiator
{
public:
    TextSdpNegotiator();
    virtual ~TextSdpNegotiator();

    IMS_BOOL Negotiate(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT TextProfile* pNegotiatedProfile,
            IN MediaConfiguration* pConfig);

private:
    IMS_BOOL ResetNegotiatedProfile(IN IMS_BOOL bPeerPreferred, IN TextProfile* pLocalProfile,
            IN TextProfile* pPeerProfile, OUT TextProfile* pNegotiatedProfile);
    IMS_BOOL NegotiatePayload(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            OUT TextProfile* pNegotiatedProfile);
    void AppendT140Payload(IN TextProfile::Payload* pPayload, OUT TextProfile* pNegotiatedProfile);
    void NegotiateDirection(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            OUT TextProfile* pNegotiatedProfile);
    void NegotiateBandwidth(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            IN IMS_SINT32 nAsValueOfNegoticatedCodec, OUT TextProfile* pNegotiatedProfile);
    void NegotiateRtcpInterval(OUT TextProfile* pNegotiatedProfile, IN MediaConfiguration* pConfig);
    IMS_BOOL FindT140InProfile(IN TextProfile* pProfile, IN TextProfile::Payload* pPayload);
    MEDIA_DIRECTION UpdateDirectionToMine(
            IN MEDIA_DIRECTION ePeerDirection, IN MEDIA_DIRECTION eLocalDirection);
};

#endif
