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

#ifndef TEXT_PROFILE_NEGOTIATOR_H_
#define TEXT_PROFILE_NEGOTIATOR_H_

#include "MediaProfileNegotiator.h"
#include "text/TextProfileUtil.h"

class MediaConfiguration;

/**
 * This class is to generate a negotiated text profile by negotiating a local text profile and a
 * peer text profile
 */
class TextProfileNegotiator : public MediaProfileNegotiator
{
public:
    TextProfileNegotiator();
    virtual ~TextProfileNegotiator();

    IMS_BOOL Negotiate(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT TextProfile* pNegotiatedProfile,
            IN MediaConfiguration* pConfig);

private:
    IMS_BOOL ResetNegotiatedProfile(IN IMS_BOOL bPeerPreferred, IN TextProfile* pLocalProfile,
            IN TextProfile* pPeerProfile, OUT TextProfile** pNegotiatedProfile);
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
