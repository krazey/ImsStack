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

#include "media/IMedia.h"
#include "BaseNego.h"
#include "ISession.h"
#include "MediaDef.h"
#include "config/TextConfiguration.h"
#include "text/TextDef.h"
#include "text/TextProfileUtil.h"

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
     * @brief Form the SDP with the current profile based on the state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to form the session level SDP
     * @param pDescriptor The SDP descriptor instance to form the media level SDP
     * @param eDir The media direction of the SDP
     * @param bDisable if it is IMS_TRUE, set the port number to zero
     * @param bEnforceReofferMode To indicate the SDP should be set using full codec capability
     * @return IMS_BOOL Returns IMS_TRUE when there is no error during forming SDP, IMS_FALSE when
     * it is failed to form
     */
    virtual IMS_BOOL FormSdp(IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable,
            IN IMS_BOOL bEnforceReofferMode);

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
     * @brief Negotiate the SDP and make the negotiate profile based on the nego state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to negotiate the session level SDP
     * @param pDescriptor The SDP descriptor instance to negotiate the media level SDP
     * @param eDir The media direction of the SDP
     */
    virtual void NegotiateSdp(IN const NEGO_STATE eNegoState,
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT IMS_SINT32& eDir);

    /**
     * @brief Get the negotiated remote ip address
     *
     * @return const IpAddress& The ip address
     */
    virtual const IpAddress& GetNegotiatedRemoteAddress();

    /**
     * @brief Get the negotiated remote port number
     *
     * @return IMS_UINT32 The port number
     */
    virtual IMS_SINT32 GetRemotePort();

    /**
     * @brief Get the negotiated local profile object
     */
    virtual TextProfile* GetNegotiatedLocalProfile();

    /**
     * @brief Get the negotiated negotiated profile object
     */
    virtual TextProfile* GetNegotiatedNegoProfile();

    /**
     * @brief Get the negotiated peer profile object
     */
    virtual TextProfile* GetNegotiatedPeerProfile();

    /**
     * @brief Get the negotiated audio direction
     */
    virtual MEDIA_DIRECTION GetNegotiatedDirection();

    /**
     * @brief Get the negotiated audio codec
     */
    virtual TEXT_CODEC GetNegotiatedCodec();

    /**
     * @brief Get the port number from the negotiated profile
     */
    virtual IMS_SINT32 GetNegotiatedRtpPort();

    /**
     * @brief Get the negotiated audio bandwidth
     */
    virtual IMS_SINT32 GetMediaBandwidth();

protected:
    TextConfiguration* ConfigCasting(IN MediaConfiguration* pConfig);
    TextProfile* ProfileCasting(IN MediaBaseProfile* pProfile);
    TextProfile* GetLocalProfile(IN OaModel* pOaModel) override;
    TextProfile* GetPeerProfile(IN OaModel* pOaModel) override;
    TextProfile* GetNegotiatedProfile(IN OaModel* pOaModel) override;

private:
    void Copy(IN const TextNego* pTextNego);
    IMS_BOOL FormOffer(IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable);
    IMS_BOOL FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable);
    IMS_BOOL FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable,
            IN IMS_BOOL bEnforceReofferMode);
    IMS_SINT32 NegotiateOffer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    IMS_SINT32 NegotiateAnswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    IMS_BOOL MakeSDPFromProfile(OUT ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN TextProfile* pProfile);
    IMS_BOOL MakeProfileFromSDP(IN ISessionDescriptor* pSessionDescriptor,
            IN IMediaDescriptor* pDescriptor, OUT TextProfile* pProfile);
    IMS_BOOL MakeNegotiatedProfile(IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT TextProfile* pNegotiatedProfile);
    IMS_BOOL GetFmtpFromString(IN const AString& strFmtp, OUT TextProfile::RedFmtp* pFmtp);
    IMS_BOOL FindT140InProfile(IN TextProfile* pProfile, IN TextProfile::Payload* pPayload);
    MEDIA_DIRECTION UpdateDirectionToMine(IN MEDIA_DIRECTION ePeerDirection,
            IN MEDIA_DIRECTION eLocalDirection, IN IMS_BOOL bIsMtCase);
    OaModel* GetNegotiatedOaModel(IMS_BOOL bCheckConfirmed = IMS_FALSE);
};

#endif
