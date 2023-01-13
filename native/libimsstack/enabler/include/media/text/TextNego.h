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

#include "ImsSlot.h"
#include "media/IMedia.h"
#include "ISession.h"
#include "MediaDef.h"
#include "config/TextConfiguration.h"
#include "text/TextDef.h"
#include "text/TextProfile.h"
#include "text/TextProfileUtil.h"

/**
 * @brief The class to negotiate and form the SDP attribute belong to the m=text line
 *
 */
class TextNego : public ImsSlot
{
public:
    /**
     * @brief The class to store the negotiation attribute of the local and peer
     *
     */
    class OaModel
    {
    public:
        /** The SDP profile for local device side */
        TextProfile* pLocalProfile;
        /** The SDP profile for peer device side */
        TextProfile* pPeerProfile;
        /** The SDP profile to store negotiated profiles */
        TextProfile* pNegotiatedProfile;
        /** The identification of SDP description object from the SDP engine */
        IMS_SINTP nSessionDescriptorKey;
        /** checking variable for confirmed session*/
        IMS_BOOL bConfirmedSession;

    public:
        OaModel() :
                pLocalProfile(IMS_NULL),
                pPeerProfile(IMS_NULL),
                pNegotiatedProfile(IMS_NULL),
                nSessionDescriptorKey(0),
                bConfirmedSession(IMS_FALSE){};
        ~OaModel()
        {
            if (pLocalProfile != IMS_NULL)
                delete pLocalProfile;

            if (pPeerProfile != IMS_NULL)
                delete pPeerProfile;

            if (pNegotiatedProfile != IMS_NULL)
                delete pNegotiatedProfile;
        };

    private:
        OaModel(IN const OaModel& obj);
        OaModel& operator=(IN const OaModel& obj);

    public:
        IMS_BOOL IsAllProfileExist()
        {
            if (pLocalProfile != IMS_NULL && pPeerProfile != IMS_NULL &&
                    pNegotiatedProfile != IMS_NULL)
                return IMS_TRUE;
            else
                return IMS_FALSE;
        };
    };

public:
    explicit TextNego(IMS_SINT32 nSlotId = IMS_SLOT_0);
    TextNego(IN const TextNego& objTextNego);
    TextNego& operator=(IN const TextNego& obj);
    virtual ~TextNego();

    /**
     * @brief Create a base local/peer/negotiate profile with given configuration
     *
     * @param pEnvironment The MediaEnvironment
     * @param pConfig The configuration to create the TextProfile
     */
    virtual void CreateProfiles(IN MediaEnvironment* pEnvironment, IN TextConfiguration* pConfig);

    /**
     * @brief Form the SDP with the current profile based on the state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to form the session level SDP
     * @param pDescriptor The SDP descriptor instance to form the m=text level SDP
     * @param eDir The media direction of the SDP
     * @param bDisable if it is IMS_TRUE, set the port number to zero
     * @param bEnforceReofferMode To indicate the SDP should be set using full codec capability
     * @return IMS_BOOL Returns IMS_TRUE when there is no error during forming SDP, IMS_FALSE when
     * it is failed to form
     */
    virtual IMS_BOOL FormSDP(IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable,
            IN IMS_BOOL bEnforceReofferMode);

    /**
     * @brief Negotiate the SDP and make the negotiate profile based on the nego state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to negotiate the session level SDP
     * @param pDescriptor The SDP descriptor instance to negotiate the m=text level SDP
     * @param eDir The media direction of the SDP
     */
    virtual void NegotiateSDP(IN const NEGO_STATE eNegoState,
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT IMS_SINT32& eDir);

    /**
     * @brief Remove incomplete SDP negotiation set to keep the negotiation set to certain size
     *
     * @param pSessionDescriptor The SDP descriptor instance to access session level SDP
     * @param eNegoState The current negotiation state to decide to remove the OA model item
     */
    virtual void FinalizeSDP(IN ISessionDescriptor* pSessionDescriptor, NEGO_STATE eNegoState);

    /**
     * @brief Set the local port number of the TextProfile
     *
     * @param nPort The port number
     * @return IMS_BOOL IMS_TRUE when the port number is unique and valid, IMS_FALSE when it is
     * invalid port number which is already reserved
     */
    virtual IMS_BOOL SetPort(IN IMS_UINT32 nPort);

    /**
     * @brief Get the local ip address
     *
     * @return const IPAddress& The local ip address
     */
    virtual const IPAddress& GetLocalAddress() { return m_objBaseProfile.objIpAddress; };

    /**
     * @brief Get the local port number
     *
     * @return IMS_UINT32 The local port number
     */
    virtual IMS_UINT32 GetLocalPort() { return m_objBaseProfile.nDataPort; };

    /**
     * @brief Get the negotiated remote ip address
     *
     * @return const IPAddress& The ip address
     */
    virtual const IPAddress& GetNegotiatedRemoteAddress();

    /**
     * @brief Get the negotiated remote port number
     *
     * @return IMS_UINT32 The port number
     */
    virtual IMS_UINT32 GetNegotiatedRemotePort();

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

private:
    void copy(IN const TextNego* pTextNego);
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
    IMS_BOOL GetFmtpFromString(IN AString strFmtp, OUT TextProfile::RedFmtp* pFmtp);
    IMS_BOOL FindT140InProfile(IN TextProfile* pProfile, IN TextProfile::Payload* pPayload);
    MEDIA_DIRECTION UpdateDirectionToMine(IN MEDIA_DIRECTION ePeerDirection,
            IN MEDIA_DIRECTION eLocalDirection, IN IMS_BOOL bIsMtCase);
    OaModel* GetNegotiatedOaModel(IMS_BOOL bCheckConfirmed = IMS_FALSE);

    IMSList<OaModel*> m_listOaModel;
    TextProfile m_objBaseProfile;
    MediaEnvironment* m_pEnvironment;
    TextConfiguration* m_pConfig;
};

#endif
