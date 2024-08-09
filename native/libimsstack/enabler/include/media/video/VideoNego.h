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

#include "BaseNego.h"
#include "MediaDef.h"
#include "video/VideoDef.h"
#include "config/VideoConfiguration.h"
#include "video/VideoProfileExtractor.h"
#include "video/VideoSdpGenerator.h"
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
     * @brief Get the Negotiated video resolution
     *
     * @return VIDEO_RESOLUTION
     */
    virtual VIDEO_RESOLUTION GetNegotiatedResolution();

    /**
     * @brief static cast from MediaConfiguration to VideoConfiguration
     */
    VideoConfiguration* ConfigCasting(IN MediaConfiguration* pConfig);

    /**
     * @brief static cast from MediaBaseProfile to VideoProfile
     */
    VideoProfile* ProfileCasting(IN MediaBaseProfile* pProfile);

    /**
     * @brief static cast from MediaBaseProfile::BasePayload to VideoProfile::Payload
     */
    VideoProfile::Payload* PayloadCasting(IN MediaBaseProfile::BasePayload* pPayload);

protected:
    VideoProfile* GetLocalProfile(IN OaModel* pOaModel) override;
    VideoProfile* GetPeerProfile(IN OaModel* pOaModel) override;
    VideoProfile* GetNegotiatedProfile(IN OaModel* pOaModel) override;
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

private:
    IMS_BOOL MakeNegotiatedPayload(IN VideoProfile::Payload* pLocalPayload,
            IN VideoProfile::Payload* pPeerPayload, OUT VideoProfile::Payload* pNegoPayload);
    IMS_BOOL MakeNegotiatedProfile(IN VideoProfile* pLocalProfile, IN VideoProfile* pPeerProfile,
            IN IMS_BOOL bIsOfferReceived, OUT VideoProfile* pNegotiatedProfile);
    VideoProfile::Payload* FindPayloadInProfile(
            IN VideoProfile* pProfile, IN VideoProfile::Payload* pPayload);
    IMS_SINT32 FindPayloadIndexFromProfile(
            IN VideoProfile* pProfile, IN const VideoProfile::Payload* pPayload);
    MEDIA_DIRECTION UpdateDirectionToMine(
            IN MEDIA_DIRECTION ePeerDir, IN MEDIA_DIRECTION eSrcDir, IN IMS_BOOL bIsMtCase);
    IMS_BOOL GetWidthHeightFromSdp_SpropParam(IN VIDEO_CODEC codecType, IN IMS_CHAR* szSprop,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);
    IMS_BOOL MakeNegotiatedCapaNegoProfile(IN VideoProfile::CapaNego* pSrcCapaNego,
            IN VideoProfile::CapaNego* pDestCapaNego,
            OUT VideoProfile::CapaNego* pNegotiatedCapaNego);

    VIDEO_RESOLUTION GetAvcMaxResolutionFromLevel(IN IMS_UINT32 nLevel);

private:
    std::unique_ptr<VideoProfileExtractor> m_pProfileExtractor;
};

#endif
