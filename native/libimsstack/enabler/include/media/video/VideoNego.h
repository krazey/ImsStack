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

#ifndef VIDEO_NEGO_H_
#define VIDEO_NEGO_H_

#include "media/IMedia.h"
#include "BaseNego.h"
#include "ISession.h"
#include "MediaDef.h"
#include "video/VideoDef.h"
#include "config/VideoConfiguration.h"
#include "video/VideoProfileUtil.h"

class VideoNego : public BaseNego
{
public:
    explicit VideoNego(IN const IMS_SINT32 nSlotID = IMS_SLOT_0);
    VideoNego(IN const VideoNego& obj);
    VideoNego& operator=(IN const VideoNego& obj);
    virtual ~VideoNego();

    void DestroyProfiles();
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
     * @brief Check if video codec from SDP is supported
     *
     * @param pSessionDescriptor The SDP descriptor instance to negotiate the session level SDP
     * @param pDescriptor The SDP descriptor instance to negotiate the media level SDP
     * @return IMS_BOOL Returns IMS_TRUE when video codec from SDP is supported and the remote video
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

protected:
    VideoConfiguration* ConfigCasting(IN MediaConfiguration* pConfig);
    VideoProfile* ProfileCasting(IN MediaBaseProfile* pProfile);
    VideoProfile* GetLocalProfile(IN OaModel* pOaModel) override;
    VideoProfile* GetPeerProfile(IN OaModel* pOaModel) override;
    VideoProfile* GetNegotiatedProfile(IN OaModel* pOaModel) override;

private:
    void Copy(IN const VideoNego* pVideoNego);
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
    IMS_BOOL MakeSdpFromProfile(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN VideoProfile* pProfile);
    IMS_BOOL MakeProfileFromSdp(IN ISessionDescriptor* pSessionDescriptor,
            IN IMediaDescriptor* pDescriptor, OUT VideoProfile* pProfile);
    IMS_BOOL MakeNegotiatedPayload(IN VideoProfile::Payload* pLocalPayload,
            IN VideoProfile::Payload* pPeerPayload, OUT VideoProfile::Payload* pNegoPayload);
    IMS_BOOL MakeNegotiatedProfile(IN VideoProfile* pLocalProfile, IN VideoProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT VideoProfile* pNegotiatedProfile);
    IMS_BOOL GetFmtpFromString(IN const AString& strFmtp, OUT VideoProfile::AvcFmtp* pFmtp);
    IMS_BOOL GetFmtpFromString(IN const AString& strFmtp, OUT VideoProfile::HevcFmtp* pFmtp);
    VideoProfile::Payload* FindPayloadInProfile(
            IN VideoProfile* pProfile, IN VideoProfile::Payload* pPayload);
    IMS_SINT32 FindPayloadIndexFromProfile(
            IN VideoProfile* pProfile, IN const VideoProfile::Payload* pPayload);
    MEDIA_DIRECTION UpdateDirectionToMine(
            IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir, IN IMS_BOOL bIsMtCase);
    IMS_BOOL GetAvpfFromAttributes(IN SdpMediaFormat* pMediaFormat,
            IN VideoProfile::CapaNego* pCapaNego, OUT VideoProfile::RtcpFbAttributes* pRtcpFbAttr);
    IMS_BOOL GetCorrectImageIndex(IN IMS_SINT32 nPayloadTypeNum, IN ImsList<AString> objAttributes,
            OUT IMS_UINT32* nIndex);
    VIDEO_RESOLUTION GetResolutionFromSdp(IN VIDEO_CODEC codecType,
            IN const AString& strImageAttrFromSdp, IN const AString& strFrameSizeFromSdp,
            IN const AString& strSpropParam, IN IMS_SINT32 nQcif = -1);
    IMS_BOOL GetWidthHeightFromSdp_ImageAttr(IN const AString& strImageAttrFromSdp,
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
    IMS_BOOL GetAvpfFromAttributes_EX(
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

    IMS_BOOL m_bNegotiatedCvoResult;
};

#endif
