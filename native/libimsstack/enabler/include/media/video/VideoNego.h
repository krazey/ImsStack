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

#ifndef _IMS_VIDEO_NEGO_H_
#define _IMS_VIDEO_NEGO_H_

#include "ImsSlot.h"
#include "media/IMedia.h"
#include "ISession.h"
#include "MediaDef.h"
#include "video/VideoDef.h"
#include "MediaEnvironment.h"
#include "config/VideoConfiguration.h"
#include "video/VideoProfile.h"
#include "video/VideoProfileUtil.h"

class VideoNego : ImsSlot
{
public:
    class OaModel
    {
    public:
        VideoProfile* pLocalProfile;
        VideoProfile* pPeerProfile;
        VideoProfile* pNegotiatedProfile;
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
    VideoNego(IN const IMS_SINT32 nSlotID = IMS_SLOT_0);
    VideoNego(IN const VideoNego& obj);
    VideoNego& operator=(IN const VideoNego& obj);
    virtual ~VideoNego();

    /**
     * @brief Create a base local/peer/negotiate profile with given configuration
     *
     * @param pEnvironment The MediaEnvironment
     * @param pConfig The configuration to create audio profile
     */
    virtual void CreateProfiles(IN MediaEnvironment* pEnvironment, IN VideoConfiguration* pConfig);
    virtual void DestroyProfiles();
    /**
     * @brief Form the SDP with the current profile based on the state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to form the session level SDP
     * @param pDescriptor The SDP descriptor instance to form the m=text level SDP
     * @param eDir The media direction of the SDP
     * @param bDisable if it is IMS_TRUE, set the port number to zero
     * @return IMS_BOOL Returns IMS_TRUE when there is no error during forming SDP, IMS_FALSE when
     * it is failed to form
     */
    virtual IMS_BOOL FormSDP(IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable);

    /**
     * @brief Negotiate the SDP and make the negotiate profile based on the nego state
     *
     * @param eNegoState The negotiation state which decide how to use the profile from the OA model
     * list
     * @param pSessionDescriptor The SDP descriptor instance to negotiate the session level SDP
     * @param pDescriptor The SDP descriptor instance to negotiate the m=text level SDP
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
     * @brief Set the local port number of the VideoProfile
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
    virtual VideoProfile* GetNegotiatedLocalProfile();

    /**
     * @brief Get the negotiated negotiated profile object
     */
    virtual VideoProfile* GetNegotiatedNegoProfile();

    /**
     * @brief Get the negotiated peer profile object
     */
    virtual VideoProfile* GetNegotiatedPeerProfile();

    /**
     * @brief Get the negotiated audio direction
     */
    virtual MEDIA_DIRECTION GetNegotiatedDirection();

    /**
     * @brief Get the Negotiated video resolution
     *
     * @return VIDEO_RESOLUTION
     */
    virtual VIDEO_RESOLUTION GetNegotiatedResolution();

    /**
     * @brief Get the Negotiated rtp port number
     *
     * @return IMS_SINT32
     */
    virtual IMS_SINT32 GetNegotiatedRtpPort();

    /**
     * @brief Get the negotiated bandwidth
     *
     * @return IMS_SINT32
     */
    virtual IMS_SINT32 GetMediaBandwidth();

private:
    void Copy(IN const VideoNego* pVideoNego);
    IMS_BOOL FormOffer(IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable);
    IMS_BOOL FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable);
    IMS_BOOL FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir, IN IMS_BOOL bDisable);
    MEDIA_DIRECTION NegotiateOffer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    MEDIA_DIRECTION NegotiateAnswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    MEDIA_DIRECTION NegotiateReanswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);
    IMS_BOOL MakeSdpFromProfile(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile);
    IMS_BOOL MakeProfileFromSdp(IN ISessionDescriptor* pSessionDescriptor,
            IN IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile);
    IMS_BOOL MakeNegotiatedPayload(IN VideoProfile::Payload* pLocalPayload,
            IN VideoProfile::Payload* pPeerPayload, OUT VideoProfile::Payload* pNegoPayload);
    IMS_BOOL MakeNegotiatedProfile(IN VideoProfile* pLocalProfile, IN VideoProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT VideoProfile* pNegotiatedProfile);
    IMS_BOOL GetFmtpFromString(IN AString strFmtp, OUT VideoProfile::AvcFmtp* pFmtp);
    IMS_BOOL GetFmtpFromString(IN AString strFmtp, OUT VideoProfile::HevcFmtp* pFmtp);
    VideoProfile::Payload* FindPayloadInProfile(
            IN VideoProfile* pProfile, IN VideoProfile::Payload* pPayload);
    IMS_SINT32 FindPayloadIndexFromProfile(
            IN VideoProfile* pProfile, IN VideoProfile::Payload* pPayload);
    MEDIA_DIRECTION UpdateDirectionToMine(
            IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir, IN IMS_BOOL bIsMtCase);
    IMS_BOOL GetAvpfFromAttributes(IN SdpMediaFormat* pMediaFormat,
            IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr);
    IMS_BOOL GetCorrectImageIndex(IN IMS_SINT32 nPayloadTypeNum, IN IMSList<AString> objAttributes,
            OUT IMS_UINT32* nIndex);
    VIDEO_RESOLUTION GetResolutionFromSdp(IN VIDEO_CODEC codecType, IN AString strImageAttrFromSdp,
            IN AString strFrameSizeFromSdp, IN AString strSpropParam, IN IMS_SINT32 nQcif = -1);
    IMS_BOOL GetWidthHeightFromSdp_ImageAttr(IN AString strImageAttrFromSdp,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
    IMS_BOOL GetWidthHeightFromSdp_SpropParam(IN VIDEO_CODEC codecType, IN IMS_CHAR* szSprop,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
    IMS_BOOL GetWidthHeightFromSdp_FrameSize(IN AString strFrameSizeFromSdp,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
    VIDEO_RESOLUTION GetResolutionFromWidthHeight(IN IMS_UINT32 nWidth, IN IMS_UINT32 nHeight);
    IMS_BOOL MakeImageAttributeLine(IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId,
            OUT AString& strImageAttr);
    IMS_BOOL MakeFrameSizeLine(IN IMS_UINT32 nPayloadType, IN VIDEO_RESOLUTION eResolutionId,
            OUT AString& strFrameSize);
    IMS_BOOL MakeCapaNegoProfileFromSdp(
            IN IMediaDescriptor* pDescriptor, OUT VideoProfile::CapaNego* pObjCapaNego);
    IMS_BOOL MakeNegotiatedCapaNegoProfile(IN VideoProfile::CapaNego* pSrcCapaNego,
            IN VideoProfile::CapaNego* pDestCapaNego,
            OUT VideoProfile::CapaNego* pNegotiatedCapaNego);
    IMS_BOOL CheckAvpfFromProfile(IN VideoProfile* pProfile);
    IMS_BOOL GetAvpfFromAttributes_EX(IN IMediaDescriptor* pMediaDescriptor,
            IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr);
    OaModel* GetNegotiatedOaModel();

    /**
     * @brief Get the width and height from video resolution enum id
     *
     * @param eResolutionId The enum of video resolution set
     * @param pnWidth The width of video resolution
     * @param pnHeight The height of video resolution
     * @return IMS_BOOL
     */
    IMS_BOOL GetWidthHeightFromResolutionId(
            IN VIDEO_RESOLUTION eResolutionId, OUT IMS_UINT32* pnWidth, OUT IMS_UINT32* pnHeight);
    VIDEO_RESOLUTION GetAvcMaxResolutionFromLevel(IN IMS_UINT32 nLevel);

    IMSList<OaModel*> m_listOaModel;
    VideoProfile m_objBaseProfile;
    MediaEnvironment* m_pEnvironment;
    VideoConfiguration* m_pConfig;
    MEDIA_CONTENT_TYPE m_eSessionType;
    IMS_BOOL m_bNegotiatedCvoResult;
};
#endif /* _IMS_VIDEO_NEGO_H_ */
