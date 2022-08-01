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

#ifndef _IMS_AUDIO_NEGO_H_
#define _IMS_AUDIO_NEGO_H_

#include "ImsSlot.h"
#include "media/IMedia.h"
#include "ISession.h"
#include "MediaDef.h"
#include "audio/AudioDef.h"
#include "audio/AudioProfile.h"
#include "audio/AudioProfileUtil.h"
#include "config/AudioConfiguration.h"

/**
 * @brief The class to negotiate and form the SDP attribute belong to the m=audio line
 *
 */
class AudioNego : public ImsSlot
{
public:
    /**
     * @brief The class to store the negotiation attribute of the local and peer
     *
     */
    class OaModel
    {
    public:
        AudioProfile* pLocalProfile;
        AudioProfile* pPeerProfile;
        AudioProfile* pNegotiatedProfile;
        IMS_SINTP nSessionDescriptorKey;
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
            {
                delete pLocalProfile;
            }

            if (pPeerProfile != IMS_NULL)
            {
                delete pPeerProfile;
            }

            if (pNegotiatedProfile != IMS_NULL)
            {
                delete pNegotiatedProfile;
            }
        };

    private:
        OaModel(IN const OaModel& obj);
        OaModel& operator=(IN const OaModel& obj);

    public:
        IMS_BOOL IsAllProfileExist()
        {
            if (pLocalProfile != IMS_NULL && pPeerProfile != IMS_NULL &&
                    pNegotiatedProfile != IMS_NULL)
            {
                return IMS_TRUE;
            }
            else
            {
                return IMS_FALSE;
            }
        };
    };

public:
    AudioNego(IMS_SINT32 nSlotId = IMS_SLOT_0);
    AudioNego(IN const AudioNego& objAudioNego);
    AudioNego& operator=(IN const AudioNego& obj);
    virtual ~AudioNego();

    /**
     * @brief Create a base local/peer/negotiate profile with given configuration
     *
     * @param pEnvironment The MediaEnvironment
     * @param pConfig The configuration to create audio profile
     */
    virtual void CreateProfiles(IN MediaEnvironment* pEnvironment, IN AudioConfiguration* pConfig);

    /**
     * @brief Form the SDP with the current profile based on the state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to form the session level SDP
     * @param pDescriptor The SDP descriptor instance to form the m=audio level SDP
     * @param eDir The media direction of the SDP
     * @return IMS_BOOL Returns IMS_TRUE when there is no error during forming SDP, IMS_FALSE when
     * it is failed to form
     */
    virtual IMS_BOOL FormSDP(IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir);

    /**
     * @brief Negotiate the SDP and make the negotiate profile based on the nego state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to negotiate the session level SDP
     * @param pDescriptor The SDP descriptor instance to negotiate the m=audio level SDP
     * @param eDir The media direction of the SDP
     * @return IMS_BOOL Returns IMS_TRUE when there is no error during SDP negotiation, IMS_FALSE
     * when it is failed to form
     */
    virtual IMS_BOOL NegotiateSDP(IN NEGO_STATE eNegoState,
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
            OUT MEDIA_DIRECTION* eDir);

    /**
     * @brief Remove incomplete SDP negotiation set to keep the negotiation set to certain size
     *
     * @param pSessionDescriptor The SDP descriptor instance to access session level SDP
     * @param eNegoState The current negotiation state to decide to remove the OA model item
     */
    virtual void FinalizeSDP(IN ISessionDescriptor* pSessionDescriptor, NEGO_STATE eNegoState);

    /**
     * @brief Set the local port number of the AudioProfile
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
    virtual const IPAddress& GetLocalAddress() { return m_objBaseProfile.objIpAddr; };

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
    virtual const IPAddress& GetNegotiatedRemoteAddr();

    /**
     * @brief Get the negotiated remote port number
     *
     * @return IMS_UINT32 The port number
     */
    virtual IMS_UINT32 GetNegotiatedRemotePort();

    /**
     * @brief Get the negotiated local profile object
     */
    virtual AudioProfile* GetNegotiatedLocalProfile();

    /**
     * @brief Get the negotiated negotiated profile object
     */
    virtual AudioProfile* GetNegotiatedNegoProfile();

    /**
     * @brief Get the negotiated peer profile object
     */
    virtual AudioProfile* GetNegotiatedPeerProfile();

    /**
     * @brief Get the negotiated audio direction
     */
    virtual MEDIA_DIRECTION GetNegotiatedDirection(void);

    /**
     * @brief Get the negotiated audio codec
     */
    virtual AUDIO_CODEC GetNegotiatedCodec(void);

    /**
     * @brief Get the negotiated audio codec mode rate(bitrate)
     */
    virtual AUDIO_CODEC_BITRATE GetNegotiatedAudioCodecRate(void);

    /**
     * @brief Get if the telephony-event is negotiated
     */
    virtual IMS_BOOL HasNegotiatedDtmf(void);

    /**
     * @brief Get the port number from the negotiated profile
     */
    virtual IMS_SINT32 GetNegotiatedRtpPort(void);

    /**
     * @brief Get the negotiated audio bandwidth
     */
    virtual IMS_SINT32 GetMediaBandwidth(void);

private:
    void copy(IN const AudioNego* pAudioNego);
    IMS_BOOL FormOffer(IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MEDIA_DIRECTION eDir);
    IMS_BOOL FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir);
    IMS_BOOL FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir);
    MEDIA_DIRECTION NegotiateOffer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    MEDIA_DIRECTION NegotiateAnswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    IMS_BOOL MakeSdpFromProfile(OUT ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile);
    IMS_BOOL MakeProfileFromSdp(IN ISessionDescriptor* pSessionDescriptor,
            IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile);
    IMS_BOOL MakeNegotiatedProfile(IN AudioProfile* pLocalProfile, IN AudioProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT AudioProfile* pNegotiatedProfile);
    IMS_BOOL GetFmtpFromString(IN AString strFmtp, OUT AudioProfile::EvsFmtp* pFmtp);
    IMS_BOOL FindEvsInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bIsOfferReceived, OUT IMS_UINT32* pBandwidthNegoList,
            OUT IMS_UINT32* pBitrateNegoList, OUT IMS_UINT32* pModeSetNegoList);
    IMS_BOOL GetFmtpFromString(IN AString strFmtp, OUT AudioProfile::AmrFmtp* pFmtp);
    IMS_BOOL FindAmrInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload,
            IN IMS_BOOL bIsOfferReceived, OUT IMS_UINT32* pnNegoModeSetList);
    IMS_BOOL FindPcmInProfile(IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload);
    IMS_SINT32 CompareModeSet(IN AudioProfile::AmrFmtp* pSrcFmtp,
            IN AudioProfile::AmrFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived,
            OUT IMS_UINT32* nNegoModeSet);
    IMS_BOOL CompareEvsBwBrMode(IN AudioProfile::EvsFmtp* pSrcFmtp,
            IN AudioProfile::EvsFmtp* pDestFmtp, IN IMS_BOOL bIsOfferReceived,
            OUT IMS_UINT32* nNegoBwList, OUT IMS_UINT32* nNegoBrList,
            OUT IMS_UINT32* nNegoModeList);
    IMS_BOOL CompareEvsBwBrModeLegacy(IN AudioProfile::EvsFmtp* pSrcFmtp,
            IN AudioProfile::EvsFmtp* pDestFmtp, OUT IMS_UINT32* nNegoBwList,
            OUT IMS_UINT32* nNegoBrList, OUT IMS_UINT32* nNegoModeList);
    IMS_BOOL FindTelephoneEventInProfile(
            IN AudioProfile* pProfile, IN AudioProfile::Payload* pPayload);
    IMS_SINT32 FindPayloadIndexFromProfile(IN AString strCodecName, IN AudioProfile* pProfile,
            IN AudioProfile::Payload* pPayload, IN IMS_BOOL isOfferReceivedCase);
    void RearrangeModeSetByAs(
            OUT AudioProfile::Payload* pPayload, IMS_BOOL bIpV6, IN IMS_SINT32 nAs);
    MEDIA_DIRECTION UpdateDirectionToMine(IN MEDIA_DIRECTION ePeerDirection,
            IN MEDIA_DIRECTION eLocalDirection, IN IMS_BOOL bIsMtCase);
    IMS_BOOL MakeCapaNegoProfileFromSdp(
            IN IMediaDescriptor* pDescriptor, OUT AudioProfile::CapaNego* pObjCapaNego);
    IMS_BOOL MakeNegotiatedCapaNegoProfile(IN AudioProfile::CapaNego* pSrcCapaNego,
            IN AudioProfile::CapaNego* pDestCapaNego,
            OUT AudioProfile::CapaNego* pNegotiatedCapaNego);
    OaModel* GetNegotiatedOaModel(IMS_BOOL bCheckConfirmed = IMS_FALSE);

    IMSList<OaModel*> m_lstOaModel;
    AudioProfile m_objBaseProfile;
    MediaEnvironment* m_pEnvironment;
    AudioConfiguration* m_pConfig;
};

#endif /* _IMS_AUDIO_NEGO_H_ */
