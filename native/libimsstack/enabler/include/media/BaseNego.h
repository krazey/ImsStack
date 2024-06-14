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

#include "MediaBaseProfile.h"
#include "MediaEnvironment.h"
#include "config/MediaConfiguration.h"

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
                bConfirmedSession(IMS_FALSE){};
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

    explicit BaseNego(IN const IMS_SINT32 nSlotId = IMS_SLOT_0);
    virtual ~BaseNego();

    /**
     * @brief Get the local ip address
     *
     * @return const IpAddress& The local ip address
     */
    virtual const IpAddress& GetLocalAddress()
    {
        return (m_pBaseProfile != IMS_NULL) ? m_pBaseProfile->objIpAddress : IpAddress::NONE;
    }

    /**
     * @brief Get the local port number
     *
     * @return IMS_UINT32 The local port number
     */
    virtual IMS_UINT32 GetLocalPort()
    {
        return (m_pBaseProfile != IMS_NULL) ? m_pBaseProfile->nDataPort : 0;
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
     * @param pType The media type to form, audio/video/text defined in MEDIA_CONTENT_TYPE
     * @param pConfig The configuration to create media profile
     */
    void CreateProfiles(IN MediaEnvironment* pEnvironment, IN MEDIA_CONTENT_TYPE pType,
            IN MediaConfiguration* pConfig);

    /**
     * @brief Remove incomplete SDP negotiation set to keep the negotiation set to certain size
     *
     * @param pSessionDescriptor The SDP descriptor instance to access session level SDP
     * @param eNegoState The current negotiation state to decide to remove the OA model item
     */
    void FinalizeSdp(IN ISessionDescriptor* pSessionDescriptor, NEGO_STATE eNegoState);

protected:
    virtual MediaBaseProfile* GetLocalProfile(IN OaModel* pOaModel);
    virtual MediaBaseProfile* GetPeerProfile(IN OaModel* pOaModel);
    virtual MediaBaseProfile* GetNegotiatedProfile(IN OaModel* pOaModel);

    void DestroyListOaModel();

protected:
    MediaBaseProfile* m_pBaseProfile;
    ImsList<OaModel*> m_listOaModel;
    MediaConfiguration* m_pConfig;
    MediaEnvironment* m_pEnvironment;
};

#endif
