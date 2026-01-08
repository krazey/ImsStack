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

#ifndef AUDIO_NEGO_H_
#define AUDIO_NEGO_H_

#include "BaseNego.h"
#include "audio/AudioDef.h"
#include "audio/AudioProfile.h"

class AudioProfileNegotiator;
class AudioSdpParser;

/**
 * @brief Manages the negotiation of audio media attributes using SDP.
 *
 * This class is responsible for handling the SDP offer/answer exchange for the audio
 * media line (m=audio). It uses an `AudioSdpParser` to interpret incoming SDP offers/answers,
 * an `AudioProfileNegotiator` to determine the common supported audio codecs and parameters,
 * and an `AudioSdpGenerator` to construct outgoing SDP offers/answers based on local capabilities
 * and negotiated results. It provides methods to retrieve detailed information about the
 * negotiated audio stream, such as codec, bitrate, and bandwidth.
 */
class AudioNego : public BaseNego
{
public:
    explicit AudioNego(IMS_SINT32 nSlotId = IMS_SLOT_0);
    AudioNego(IN const AudioNego& objAudioNego);
    AudioNego& operator=(IN const AudioNego& obj);
    virtual ~AudioNego() override;

    /**
     * @brief Checks if the remote SDP contains a supported audio codec configuration.
     *
     * This method parses the provided SDP and attempts to negotiate it against the local
     * device's audio capabilities.
     *
     * @param pSessionDescriptor The SDP descriptor instance to negotiate the session level SDP
     * @param pDescriptor The SDP descriptor instance to negotiate the media level SDP
     * @return IMS_TRUE if a compatible audio codec is found and the remote audio
     * port is not 0, otherwise returns IMS_FALSE
     */
    virtual IMS_BOOL IsMediaCodecFromSdpSupported(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor);

    /**
     * @brief Gets the negotiated audio codec
     * @return The negotiated audio codec type (e.g., AMR, AMR-WB, EVS).
     */
    virtual AUDIO_CODEC GetNegotiatedCodec(void);

    /**
     * @brief Gets the negotiated audio codec mode rate (bitrate).
     *
     * For AMR/AMR-WB/EVS, this corresponds to the highest negotiated mode.
     * @return The negotiated audio codec bitrate enum value.
     */
    virtual AUDIO_CODEC_BITRATE GetNegotiatedAudioCodecRate(void);

    /**
     * @brief Get the negotiated codec bitrate (Kbps)
     *
     * @return IMS_FLOAT Returns the bitrate of the negotiated codec (Kbps)
     */
    virtual IMS_FLOAT GetNegotiatedCodecBitrateKbps(void);

    /**
     * @brief Get the negotiated codec bandwidth (Khz)
     *
     * @return IMS_FLOAT Returns the bandwidth of the negotiated codec (Khz)
     */
    virtual IMS_FLOAT GetNegotiatedCodecBandwidthKhz(void);

    /**
     * @brief Gets the negotiated audio codec bitrate range (min and max).
     *
     * @param nBitrateStart The start of the bitrate range in kbps.
     * @param nBitrateEnd The end of the bitrate range in kbps.
     */
    virtual void GetNegotiatedCodecBitrateRange(
            OUT IMS_FLOAT& nBitrateStart, OUT IMS_FLOAT& nBitrateEnd);

    /**
     * @brief Gets the negotiated audio codec bandwidth range (min and max).
     *
     * @param nBandwidthStart The start of the bandwidth range in kHz.
     * @param nBandwidthEnd The end of the bandwidth range in kHz.
     */
    virtual void GetNegotiatedCodecBandwidthRange(
            OUT IMS_FLOAT& nBandwidthStart, OUT IMS_FLOAT& nBandwidthEnd);

    /**
     * @brief Checks if the DTMF (telephone-event) payload type was successfully negotiated.
     * @return IMS_TRUE if DTMF is negotiated, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL HasNegotiatedDtmf(void);

    /**
     * @brief Sets the SDP parser object to be used for parsing audio media lines.
     */
    void SetSdpParser(std::shared_ptr<AudioSdpParser> pSdpParser) { m_pSdpParser = pSdpParser; }

    /**
     * @brief Sets the profile negotiator object to be used for negotiating audio profiles.
     */
    void SetProfileNegotiator(std::shared_ptr<AudioProfileNegotiator> pNegotiator)
    {
        m_pProfileNegotiator = pNegotiator;
    }

protected:
    AudioProfile* GetLocalProfile(IN const OaModel& objOaModel) override;
    AudioProfile* GetPeerProfile(IN const OaModel& objOaModel) override;
    AudioProfile* GetNegotiatedProfile(IN const OaModel& objOaModel) override;
    IMS_BOOL FormOffer(IN ISessionDescriptor* pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
            IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable) override;
    IMS_BOOL FormAnswer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection,
            IN IMS_BOOL bDisable) override;
    IMS_BOOL FormReoffer(IN ISessionDescriptor* pSessionDescriptor,
            OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bDisable,
            IN IMS_BOOL bEnforceReofferMode) override;
    MEDIA_DIRECTION NegotiateOffer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor) override;
    MEDIA_DIRECTION NegotiateAnswer(
            IN ISessionDescriptor* pSessionDescriptor, IN IMediaDescriptor* pDescriptor) override;

private:
    std::shared_ptr<AudioSdpParser> m_pSdpParser;
    std::shared_ptr<AudioProfileNegotiator> m_pProfileNegotiator;
};

#endif
