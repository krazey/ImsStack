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

#ifndef TEXT_PROFILE_NEGOTIATOR_H_
#define TEXT_PROFILE_NEGOTIATOR_H_

#include "MediaProfileNegotiator.h"
#include "config/MediaConfiguration.h"
#include "text/TextProfile.h"

/**
 * This class is to generate a negotiated text profile by negotiating a local text profile and a
 * peer text profile
 */
class TextProfileNegotiator : public MediaProfileNegotiator
{
public:
    TextProfileNegotiator();
    virtual ~TextProfileNegotiator() override;

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
    virtual IMS_BOOL Negotiate(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT TextProfile* pNegotiatedProfile,
            IN MediaConfiguration* pConfig);

private:
    IMS_BOOL NegotiatePayload(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            OUT TextProfile* pNegotiatedProfile);
    TextProfile::Payload* CreatePayload(IN const MediaBaseProfile::RtpMap& objRtpMap,
            IN std::shared_ptr<TextProfile::TextFmtp> pFmtp);
    TextProfile::Payload* CreateT140PayloadFromRed(
            IN std::shared_ptr<TextProfile::RedFmtp> pRedFmtp);
    void NegotiateDirection(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            OUT TextProfile* pNegotiatedProfile);
    void NegotiateBandwidth(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            OUT TextProfile* pNegotiatedProfile);
    void NegotiateBandwidthForOfferReceived(IN TextProfile* pLocalProfile,
            IN TextProfile* pPeerProfile, OUT TextProfile* pNegotiatedProfile);
    void NegotiateBandwidthForOfferSent(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            OUT TextProfile* pNegotiatedProfile);
    void NegotiateRtcpInterval(
            OUT TextProfile* pNegotiatedProfile, IN const MediaConfiguration* pConfig);
    TextProfile::Payload* FindT140InProfile(
            IN TextProfile* pProfile, IN TextProfile::Payload* pPayload);
    MEDIA_DIRECTION UpdateDirectionToMine(
            IN MEDIA_DIRECTION ePeerDirection, IN MEDIA_DIRECTION eLocalDirection);
};

#endif
