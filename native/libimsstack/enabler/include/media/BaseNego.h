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

#ifndef BASE_NEGO_H_
#define BASE_NEGO_H_

#include "ImsSlot.h"
#include "ISession.h"
#include "media/IMedia.h"

#include "MediaBaseProfile.h"
#include "MediaEnvironment.h"
#include "config/MediaConfiguration.h"

class MediaSdpGenerator;
class MediaProfileNegotiator;
class MediaProfileGenerator;

class BaseNego : public ImsSlot
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
        MediaBaseProfile* pLocalProfile;
        /** The SDP profile for peer device side */
        MediaBaseProfile* pPeerProfile;
        /** The SDP profile to store negotiated profiles */
        MediaBaseProfile* pNegotiatedProfile;
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
                bConfirmedSession(IMS_FALSE) {};
        ~OaModel()
        {
            delete pLocalProfile;
            delete pPeerProfile;
            delete pNegotiatedProfile;
        };

    private:
        OaModel(IN const OaModel& obj);
        OaModel& operator=(IN const OaModel& obj);

    public:
        IMS_BOOL IsAllProfileExist()
        {
            return (pLocalProfile != IMS_NULL && pPeerProfile != IMS_NULL &&
                           pNegotiatedProfile != IMS_NULL)
                    ? IMS_TRUE
                    : IMS_FALSE;
        };
    };

    explicit BaseNego(IN const IMS_SINT32 nSlotId = IMS_SLOT_0,
            IN const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_NOTUSED);
    virtual ~BaseNego();

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
     * @brief Get the local ip address
     *
     * @return const IpAddress& The local ip address
     */
    virtual const IpAddress& GetLocalAddress()
    {
        return (m_pBaseProfile != IMS_NULL) ? m_pBaseProfile->GetIpAddress() : IpAddress::NONE;
    }

    /**
     * @brief Get the local port number
     *
     * @return IMS_UINT32 The local port number
     */
    virtual IMS_UINT32 GetLocalPort()
    {
        return (m_pBaseProfile != IMS_NULL) ? m_pBaseProfile->GetDataPort() : 0;
    };

    /**
     * @brief Set the local port number of the media profile
     *
     * @param nPort The port number
     * @return IMS_BOOL IMS_TRUE when the port number is unique and valid, IMS_FALSE when it is
     * invalid port number which is already reserved
     */
    IMS_BOOL SetPort(IN IMS_UINT32 nPort);

    /**
     * @brief Create a base local/peer/negotiate profile with given configuration
     *
     * @param pEnvironment The MediaEnvironment
     * @param pConfig The configuration to create media profile
     */
    virtual void CreateProfiles(IN MediaEnvironment* pEnvironment, IN MediaConfiguration* pConfig);

    /**
     * @brief Remove incomplete SDP negotiation set to keep the negotiation set to certain size
     *
     * @param pSessionDescriptor The SDP descriptor instance to access session level SDP
     * @param eNegoState The current negotiation state to decide to remove the OA model item
     */
    virtual void FinalizeSdp(IN ISessionDescriptor* pSessionDescriptor, NEGO_STATE eNegoState);

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
    virtual MediaBaseProfile* GetNegotiatedLocalProfile();

    /**
     * @brief Get the negotiated negotiated profile object
     */
    virtual MediaBaseProfile* GetNegotiatedNegoProfile();

    /**
     * @brief Get the negotiated peer profile object
     */
    virtual MediaBaseProfile* GetNegotiatedPeerProfile();

    /**
     * @brief Get the negotiated media direction
     */
    virtual MEDIA_DIRECTION GetNegotiatedDirection();

    /**
     * @brief Get the negotiated rtp port number
     */
    virtual IMS_SINT32 GetNegotiatedRtpPort();

    /**
     * @brief Get the negotiated bandwidth
     */
    virtual IMS_SINT32 GetNegotiatedBandwidth();

    /**
     * @brief Get the negotiated payload
     */
    virtual MediaBaseProfile::BasePayload* GetNegotiatedPayload();

protected:
    virtual MediaBaseProfile* GetLocalProfile(IN OaModel* pOaModel);
    virtual MediaBaseProfile* GetPeerProfile(IN OaModel* pOaModel);
    virtual MediaBaseProfile* GetNegotiatedProfile(IN OaModel* pOaModel);
    OaModel* GetNegotiatedOaModel(IMS_BOOL bCheckConfirmed = IMS_FALSE);
    void DestroyListOaModel();
    void Copy(IN const BaseNego* pNego);
    virtual IMS_BOOL FormOffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable);
    virtual IMS_BOOL FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable) = 0;
    virtual IMS_BOOL FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable,
            IN IMS_BOOL bEnforceReofferMode) = 0;
    virtual MEDIA_DIRECTION NegotiateOffer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor) = 0;
    virtual MEDIA_DIRECTION NegotiateAnswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor) = 0;

protected:
    MEDIA_CONTENT_TYPE m_eType;
    MediaBaseProfile* m_pBaseProfile;
    ImsList<OaModel*> m_listOaModel;
    MediaConfiguration* m_pConfig;
    MediaEnvironment* m_pEnvironment;
    std::shared_ptr<MediaSdpGenerator> m_pSdpGenerator;
    std::shared_ptr<MediaProfileNegotiator> m_pProfileNegotiator;
    std::shared_ptr<MediaProfileGenerator> m_pProfileGenerator;
};

#endif
