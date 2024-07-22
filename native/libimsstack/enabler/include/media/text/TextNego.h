/**
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TEXT_NEGO_H_
#define TEXT_NEGO_H_

#include "BaseNego.h"
#include "MediaDef.h"
#include "config/TextConfiguration.h"
#include "text/TextDef.h"
#include "text/TextProfileUtil.h"
#include "text/TextProfileExtractor.h"

/**
 * @brief The class to negotiate and form the SDP attribute belong to the m=text line
 *
 */
class TextNego : public BaseNego
{
public:
    explicit TextNego(IMS_SINT32 nSlotId = IMS_SLOT_0);
    TextNego(IN const TextNego& objTextNego);
    TextNego& operator=(IN const TextNego& obj);
    virtual ~TextNego();

    /**
     * @brief Check if text codec from SDP is supported
     *
     * @param pSessionDescriptor The SDP descriptor instance to negotiate the session level SDP
     * @param pDescriptor The SDP descriptor instance to negotiate the media level SDP
     * @return IMS_BOOL Returns IMS_TRUE when text codec from SDP is supported and the remote audio
     * port is not 0, otherwise returns IMS_FALSE
     */
    virtual IMS_BOOL IsMediaCodecFromSdpSupported(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);

    /**
     * @brief Get the negotiated audio codec
     */
    virtual TEXT_CODEC GetNegotiatedCodec();

    /**
     * @brief static cast from MediaConfiguration to TextConfiguration
     */
    TextConfiguration* ConfigCasting(IN MediaConfiguration* pConfig);

    /**
     * @brief static cast from MediaBaseProfile to TextProfile
     */
    TextProfile* ProfileCasting(IN MediaBaseProfile* pProfile);

    /**
     * @brief static cast from MediaBaseProfile::BasePayload to TextProfile::Payload
     */
    TextProfile::Payload* PayloadCasting(IN MediaBaseProfile::BasePayload* pPayload);

protected:
    TextProfile* GetLocalProfile(IN OaModel* pOaModel) override;
    TextProfile* GetPeerProfile(IN OaModel* pOaModel) override;
    TextProfile* GetNegotiatedProfile(IN OaModel* pOaModel) override;
    IMS_BOOL FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir,
            IN IMS_BOOL bDisable) override;
    IMS_BOOL FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable,
            IN IMS_BOOL bEnforceReofferMode) override;
    MEDIA_DIRECTION NegotiateOffer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor) override;
    MEDIA_DIRECTION NegotiateAnswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor) override;
    IMS_BOOL MakeSdpFromProfile(OUT ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pBaseProfile) override;

private:
    IMS_BOOL MakeNegotiatedProfile(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT TextProfile* pNegotiatedProfile);
    IMS_BOOL FindT140InProfile(IN TextProfile* pProfile, IN TextProfile::Payload* pPayload);
    MEDIA_DIRECTION UpdateDirectionToMine(IN MEDIA_DIRECTION ePeerDirection,
            IN MEDIA_DIRECTION eLocalDirection, IN IMS_BOOL bIsMtCase);

private:
    std::unique_ptr<TextProfileExtractor> m_pProfileExtractor;
};

#endif
