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

#ifndef VIDEO_PROFILE_NEGOTIATOR_H_
#define VIDEO_PROFILE_NEGOTIATOR_H_

#include "MediaProfileNegotiator.h"
#include "video/VideoProfile.h"
#include "config/VideoConfiguration.h"

/**
 * This class is to generate a negotiated video profile by negotiating a local video profile and a
 * peer video profile
 */
class VideoProfileNegotiator : public MediaProfileNegotiator
{
public:
    VideoProfileNegotiator();
    virtual ~VideoProfileNegotiator();

    /**
     * @brief Make the negotiated profile using the local and peer profiles
     *
     * @param pLocalProfile The local profile
     * @param pPeerProfile The peer profile
     * @param bIsOfferReceived The option to check that the case is in sdp offer received
     * @param pNegotiatedProfile The negotiated profile to update
     * @param pConfig The configuration set
     * @return IMS_BOOL Return IMS_TRUE when there is no error in negotiation vise versa when there
     * is invalid parameter and the negotiation is failed
     */
    virtual IMS_BOOL Negotiate(IN VideoProfile* pLocalProfile, IN VideoProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT VideoProfile* pNegotiatedProfile,
            IN MediaConfiguration* pConfig);

    /**
     * @brief Get the Negotiated video resolution from given payload
     *
     * @return VIDEO_RESOLUTION The enum defined in the VideoDef.h
     */
    virtual VIDEO_RESOLUTION GetNegotiatedResolution(IN MediaBaseProfile::BasePayload* pPayload);

    /**
     * @brief Determines the appropriate media direction for the local side based on the peer's
     * direction and whether it's a Mobile Terminated (MT) or Mobile Originated (MO) call.
     *
     * @param ePeerDirection The peer side direction
     * @param eLocalDirection The local side direction
     * @param bIsMtCase The flag to tell the DUT is in MT side.
     * @return MEDIA_DIRECTION Return the chosen direction
     */
    virtual MEDIA_DIRECTION UpdateDirectionToMine(IN MEDIA_DIRECTION ePeerDirection,
            IN MEDIA_DIRECTION eLocalDirection, IN IMS_BOOL bIsMtCase);

private:
    void ResetNegotiatedProfile(IN IMS_BOOL bPeerPreferred, IN VideoProfile* pLocalProfile,
            IN VideoProfile* pPeerProfile, OUT VideoProfile** pNegotiatedProfile);
    void NegotiateAvpf(IN VideoProfile* pLocalProfile, IN VideoProfile* pPeerProfile,
            OUT VideoProfile* pNegotiatedProfile);
    void NegotiateTransportType(OUT VideoProfile* pNegotiatedProfile);
    IMS_BOOL NegotiatePayload(IN VideoProfile* pLocalProfile, IN VideoProfile* pPeerProfile,
            OUT VideoProfile* pNegotiatedProfile, OUT IMS_SINT32* nNegotiatedMaxFrameRate,
            OUT IMS_SINT32* nNegotiatedMaxAs);
    IMS_BOOL NegotiateAvc(IN VideoProfile::Payload* pLocalPayload,
            IN VideoProfile::Payload* pPeerPayload, OUT VideoProfile::Payload* pNegoPayload,
            IN IMS_UINT32 nLocalIndex, IN VideoProfile* pLocalProfile,
            OUT VideoProfile* pNegotiatedProfile, OUT VideoProfile::Payload** pTempPayload,
            OUT VideoProfile::Payload** pMatchedPeerPayload);
    IMS_BOOL NegotiateHevc(IN VideoProfile::Payload* pLocalPayload,
            IN VideoProfile::Payload* pPeerPayload, OUT VideoProfile::Payload* pNegoPayload,
            IN VideoProfile* pLocalProfile, OUT VideoProfile* pNegotiatedProfile,
            OUT VideoProfile::Payload** pTempPayload,
            OUT VideoProfile::Payload** pMatchedPeerPayload);
    VideoProfile::Payload* SetClosestPayload(IN VideoProfile* pLocalProfile,
            IN VideoProfile* pNegotiatedProfile, OUT VideoProfile::Payload* pTempPayload,
            OUT VideoProfile::Payload* pMatchedPeerPayload);
    IMS_BOOL SetClosestAvc(IN VideoProfile* pLocalProfile, OUT VideoProfile::Payload* pNegoPayload);
    IMS_BOOL SetClosestHevc(
            IN VideoProfile::Payload* pMatchedPeerPayload, OUT VideoProfile::Payload* pNegoPayload);
    void NegotiateRtcpFb(OUT VideoProfile* pNegotiatedProfile,
            IN VideoProfile::Payload* pLocalPayload, IN VideoProfile::Payload* pPeerPayload,
            OUT VideoProfile::Payload* pNegoPayload);
    IMS_BOOL SetNegotiatedPayloadIndex(OUT VideoProfile* pLocalProfile,
            OUT VideoProfile* pPeerProfile, IN IMS_SINT32 nLocalIndex, IN IMS_SINT32 nPeerIndex);
    void NegotiatePayloadNumber(
            OUT VideoProfile* pLocalProfile, IN VideoProfile::Payload* pPeerPayload);
    void NegotiateCvo(IN VideoProfile* pLocalProfile, IN VideoProfile* pPeerProfile,
            OUT VideoProfile* pNegotiatedProfile);
    void SetMaxFrameRate(IN IMS_SINT32 nFrameRate, OUT IMS_SINT32* nNegotiatedMaxFrameRate);
    void SetMaxAs(IN IMS_SINT32 nAS, OUT IMS_SINT32* nNegotiatedMaxAs);
    IMS_SINT32 FindPayloadIndexFromProfile(
            IN VideoProfile* pProfile, IN const VideoProfile::Payload* pPayload);
    IMS_BOOL MakeNegotiatedCapaNegoProfile(IN VideoProfile::CapaNego* pLocalCapaNego,
            IN VideoProfile::CapaNego* pPeerCapaNego,
            OUT VideoProfile::CapaNego* pNegotiatedCapaNego);
    IMS_BOOL MakeNegotiatedPayload(IN VideoProfile::Payload* pLocalPayload,
            IN VideoProfile::Payload* pPeerPayload, OUT VideoProfile::Payload** pNegoPayload);
    VIDEO_RESOLUTION GetAvcMaxResolutionFromLevel(IN IMS_UINT32 nLevel);
    IMS_BOOL MakeNegotiatedBandwidth(IN VideoConfiguration* pConfig, IN VideoProfile* pLocalProfile,
            IN VideoProfile* pPeerProfile, IN IMS_BOOL bIsOfferReceived,
            IN IMS_SINT32 nAsValueOfNegotiatedCodec, OUT VideoProfile* pNegotiatedProfile);
};

#endif
