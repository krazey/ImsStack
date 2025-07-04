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

#ifndef VIDEO_NEGO_H_
#define VIDEO_NEGO_H_

#include "BaseNego.h"
#include "config/VideoConfiguration.h"
#include "video/VideoSdpParser.h"
#include "video/VideoProfileNegotiator.h"

class VideoNego : public BaseNego
{
public:
    explicit VideoNego(IN const IMS_SINT32 nSlotID = IMS_SLOT_0);
    VideoNego(IN const VideoNego& obj);
    VideoNego& operator=(IN const VideoNego& obj);
    virtual ~VideoNego() override;

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
     * @brief static cast from MediaBaseProfile to VideoProfile
     */
    VideoProfile* ProfileCasting(IN MediaBaseProfile* pProfile);

    /**
     * @brief static cast from MediaBaseProfile::BasePayload to VideoProfile::Payload
     */
    VideoProfile::Payload* PayloadCasting(IN MediaBaseProfile::BasePayload* pPayload);

    /**
     * @brief Set the SDP parser object
     */
    void SetSdpParser(std::shared_ptr<VideoSdpParser> pSdpParser) { m_pSdpParser = pSdpParser; }

    /**
     * @brief Set the profile negotiator object
     */
    void SetProfileNegotiator(std::shared_ptr<VideoProfileNegotiator> pNegotiator)
    {
        m_pProfileNegotiator = pNegotiator;
    }

protected:
    VideoProfile* GetLocalProfile(IN const OaModel& objOaModel) override;
    VideoProfile* GetPeerProfile(IN const OaModel& objOaModel) override;
    VideoProfile* GetNegotiatedProfile(IN const OaModel& objOaModel) override;
    IMS_BOOL FormOffer(IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable) override;
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
    IMS_BOOL GetWidthHeightFromSdp_SpropParam(IN VIDEO_CODEC codecType, IN IMS_CHAR* szSprop,
            OUT IMS_UINT32* nImageWidth, OUT IMS_UINT32* nImageHeight);

private:
    std::shared_ptr<VideoSdpParser> m_pSdpParser;
    std::shared_ptr<VideoProfileNegotiator> m_pProfileNegotiator;
};

#endif
