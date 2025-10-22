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

#ifndef BASE_NEGO_H_
#define BASE_NEGO_H_

#include <memory>

#include "ImsSlot.h"
#include "ISession.h"
#include "MediaBaseProfile.h"
#include "MediaDef.h"
#include "media/IMediaDescriptor.h"

class MediaEnvironment;
class MediaConfiguration;
class MediaSdpGenerator;
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
        std::shared_ptr<MediaBaseProfile> pLocalProfile;
        /** The SDP profile for peer device side */
        std::shared_ptr<MediaBaseProfile> pPeerProfile;
        /** The SDP profile to store negotiated profiles */
        std::shared_ptr<MediaBaseProfile> pNegotiatedProfile;

        OaModel() :
                pLocalProfile(IMS_NULL),
                pPeerProfile(IMS_NULL),
                pNegotiatedProfile(IMS_NULL)
        {
        }

        IMS_BOOL IsAllProfileExist()
        {
            return (pLocalProfile != IMS_NULL && pPeerProfile != IMS_NULL &&
                           pNegotiatedProfile != IMS_NULL)
                    ? IMS_TRUE
                    : IMS_FALSE;
        }

    private:
        OaModel(IN const OaModel& obj);
        OaModel& operator=(IN const OaModel& obj);
    };

    explicit BaseNego(IN const IMS_SINT32 nSlotId = IMS_SLOT_0,
            IN const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_NOTUSED);
    BaseNego(IN const BaseNego& obj);
    BaseNego& operator=(IN const BaseNego& obj);
    virtual ~BaseNego() override;

    /**
     * @brief Form the SDP with the current profile based on the state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to form the session level SDP
     * @param pDescriptor The SDP descriptor instance to form the media level SDP
     * @param eDirection The media direction of the SDP
     * @param bDisable if it is IMS_TRUE, set the port number to zero
     * @param bEnforceReofferMode To indicate the SDP should be set using full codec capability
     * @return IMS_BOOL Returns IMS_TRUE when there is no error during forming SDP, IMS_FALSE when
     * it is failed to form
     */
    virtual IMS_BOOL FormSdp(IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable,
            IN IMS_BOOL bEnforceReofferMode);

    /**
     * @brief Negotiate the SDP and make the negotiate profile based on the nego state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to negotiate the session level SDP
     * @param pDescriptor The SDP descriptor instance to negotiate the media level SDP
     * @param eDirection The media direction of the SDP
     */
    virtual void NegotiateSdp(IN const NEGO_STATE eNegoState,
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT MEDIA_DIRECTION& eDirection);

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
    virtual IMS_BOOL SetLocalPort(IN IMS_UINT32 nPort);

    /**
     * @brief Create a base local/peer/negotiate profile with given configuration
     *
     * @param pEnvironment The MediaEnvironment
     * @param pConfig The configuration to create media profile
     */
    virtual void CreateProfiles(
            IN std::shared_ptr<MediaEnvironment> pEnvironment, IN MediaConfiguration* pConfig);

    /**
     * @brief Cleans up any stale or incomplete negotiation models (OaModel).
     */
    virtual void CleanupIncompleteOaModels();

    /**
     * @brief Get the MediaBaseProfile object
     */
    MediaBaseProfile* GetBaseProfile() const { return m_pBaseProfile.get(); }

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

    /**
     * @brief Set the SDP generator object
     */
    void SetSdpGenerator(std::shared_ptr<MediaSdpGenerator> pSdpGenerator);

    /**
     * @brief Set the media profile generator object
     */
    void SetProfileGenerator(std::shared_ptr<MediaProfileGenerator> pProfileGenerator);

    /**
     * @brief Get the OaModel list
     */
    ImsList<std::shared_ptr<BaseNego::OaModel>>& GetOaModelList();

protected:
    virtual MediaBaseProfile* GetLocalProfile(IN const OaModel& objOaModel);
    virtual MediaBaseProfile* GetPeerProfile(IN const OaModel& objOaModel);
    virtual MediaBaseProfile* GetNegotiatedProfile(IN const OaModel& objOaModel);
    std::shared_ptr<OaModel> GetNegotiatedOaModel();
    void DestroyListOaModel();
    void Copy(IN const BaseNego* pNego);
    virtual std::shared_ptr<BaseNego::OaModel> CreateOaModel(
            IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable);
    virtual IMS_BOOL CheckArgument(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection);
    virtual IMS_BOOL FormOffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection,
            IN IMS_BOOL bDisable) = 0;
    virtual IMS_BOOL FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection,
            IN IMS_BOOL bDisable) = 0;
    virtual IMS_BOOL FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable,
            IN IMS_BOOL bEnforceReofferMode) = 0;
    virtual MEDIA_DIRECTION NegotiateOffer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor) = 0;
    virtual MEDIA_DIRECTION NegotiateAnswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor) = 0;

protected:
    MEDIA_CONTENT_TYPE m_eType;
    std::shared_ptr<MediaBaseProfile> m_pBaseProfile;
    ImsList<std::shared_ptr<OaModel>> m_listOaModel;
    MediaConfiguration* m_pConfig;
    std::shared_ptr<MediaEnvironment> m_pEnvironment;
    std::shared_ptr<MediaSdpGenerator> m_pSdpGenerator;
    std::shared_ptr<MediaProfileGenerator> m_pProfileGenerator;
};

#endif
