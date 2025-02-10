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

#ifndef AUDIO_NEGO_H_
#define AUDIO_NEGO_H_

#include "BaseNego.h"
#include "MediaDef.h"
#include "audio/AudioDef.h"
#include "audio/AudioSdpParser.h"
#include "audio/AudioProfileUtil.h"
#include "audio/AudioSdpGenerator.h"
#include "audio/AudioProfileNegotiator.h"
#include "config/AudioConfiguration.h"

/**
 * @brief The class to negotiate and form the SDP attribute belong to the m=audio line
 *
 */
class AudioNego : public BaseNego
{
public:
    explicit AudioNego(IMS_SINT32 nSlotId = IMS_SLOT_0);
    AudioNego(IN const AudioNego& objAudioNego);
    AudioNego& operator=(IN const AudioNego& obj);
    virtual ~AudioNego();

    /**
     * @brief Check if audio codec from SDP is supported
     *
     * @param pSessionDescriptor The SDP descriptor instance to negotiate the session level SDP
     * @param pDescriptor The SDP descriptor instance to negotiate the media level SDP
     * @return IMS_BOOL Returns IMS_TRUE when audio codec from SDP is supported and the remote audio
     * port is not 0, otherwise returns IMS_FALSE
     */
    virtual IMS_BOOL IsMediaCodecFromSdpSupported(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);

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
     * @brief static cast from MediaConfiguration to AudioConfiguration
     */
    AudioConfiguration* ConfigCasting(IN MediaConfiguration* pConfig);

    /**
     * @brief static cast from MediaBaseProfile to AudioProfile
     */
    AudioProfile* ProfileCasting(IN MediaBaseProfile* pProfile);

    /**
     * @brief static cast from MediaBaseProfile::BasePayload to AudioProfile::Payload
     */
    AudioProfile::Payload* PayloadCasting(IN MediaBaseProfile::BasePayload* pPayload);

protected:
    AudioProfile* GetLocalProfile(IN OaModel* pOaModel) override;
    AudioProfile* GetPeerProfile(IN OaModel* pOaModel) override;
    AudioProfile* GetNegotiatedProfile(IN OaModel* pOaModel) override;
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
    std::unique_ptr<AudioSdpParser> m_pSdpParser;
};

#endif
