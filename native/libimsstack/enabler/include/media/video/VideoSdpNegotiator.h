// Copyright 2024 Google LLC

#ifndef VIDEO_SDP_NEGOTIATOR_H_
#define VIDEO_SDP_NEGOTIATOR_H_

#include "SdpNegotiator.h"
#include "video/VideoProfileUtil.h"

class MediaConfiguration;

class VideoSdpNegotiator : public SdpNegotiator
{
public:
    VideoSdpNegotiator();
    virtual ~VideoSdpNegotiator();

    IMS_BOOL Negotiate(IN VideoProfile* pLocalProfile, IN VideoProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT VideoProfile* pNegotiatedProfile,
            IN MediaConfiguration* pConfig);
    /**
     * @brief Get the Negotiated video resolution
     *
     * @return VIDEO_RESOLUTION
     */
    VIDEO_RESOLUTION GetNegotiatedResolution(IN MediaBaseProfile::BasePayload* pPayload);
    MEDIA_DIRECTION UpdateDirectionToMine(
            IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir, IN IMS_BOOL bIsMtCase);

private:
    void ResetNegotiatedProfile(
            IN const VideoProfile* pLocalProfile, OUT VideoProfile* pNegotiatedProfile);
    IMS_SINT32 FindPayloadIndexFromProfile(
            IN VideoProfile* pProfile, IN const VideoProfile::Payload* pPayload);
    IMS_BOOL MakeNegotiatedCapaNegoProfile(IN VideoProfile::CapaNego* pSrcCapaNego,
            IN VideoProfile::CapaNego* pDestCapaNego,
            OUT VideoProfile::CapaNego* pNegotiatedCapaNego);
    IMS_BOOL MakeNegotiatedPayload(IN VideoProfile::Payload* pLocalPayload,
            IN VideoProfile::Payload* pPeerPayload, OUT VideoProfile::Payload* pNegoPayload);
    VIDEO_RESOLUTION GetAvcMaxResolutionFromLevel(IN IMS_UINT32 nLevel);
};

#endif
