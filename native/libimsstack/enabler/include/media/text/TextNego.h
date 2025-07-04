/*
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
#include "config/TextConfiguration.h"
#include "text/TextDef.h"
#include "text/TextSdpParser.h"
#include "text/TextProfileNegotiator.h"

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
    virtual ~TextNego() override;

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
     * @brief static cast from MediaBaseProfile to TextProfile
     */
    TextProfile* ProfileCasting(IN MediaBaseProfile* pProfile);

    /**
     * @brief static cast from MediaBaseProfile::BasePayload to TextProfile::Payload
     */
    TextProfile::Payload* PayloadCasting(IN MediaBaseProfile::BasePayload* pPayload);

    /**
     * @brief Set the SDP parser object
     */
    void SetSdpParser(std::shared_ptr<TextSdpParser> pSdpParser) { m_pSdpParser = pSdpParser; }

    /**
     * @brief Set the profile negotiator object
     */
    void SetProfileNegotiator(std::shared_ptr<TextProfileNegotiator> pNegotiator)
    {
        m_pProfileNegotiator = pNegotiator;
    }

protected:
    TextProfile* GetLocalProfile(IN const OaModel& objOaModel) override;
    TextProfile* GetPeerProfile(IN const OaModel& objOaModel) override;
    TextProfile* GetNegotiatedProfile(IN const OaModel& objOaModel) override;
    IMS_BOOL FormOffer(IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable) override;
    IMS_BOOL FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection,
            IN IMS_BOOL bDisable) override;
    IMS_BOOL FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable,
            IN IMS_BOOL bEnforceReofferMode) override;
    MEDIA_DIRECTION NegotiateOffer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor) override;
    MEDIA_DIRECTION NegotiateAnswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor) override;

private:
    std::shared_ptr<TextSdpParser> m_pSdpParser;
    std::shared_ptr<TextProfileNegotiator> m_pProfileNegotiator;
};

#endif
