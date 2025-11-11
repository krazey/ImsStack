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

#ifndef VIDEO_CONFIGURATION_H_
#define VIDEO_CONFIGURATION_H_

#include "config/MediaConfiguration.h"

/**
 * @class VideoConfiguration
 * @brief Manages video-specific media configurations.
 * @details This class holds all configuration parameters related to video streams,
 * such as RTP/RTCP settings, bandwidth, DSCP values, and AVPF features.
 */
class VideoConfiguration : public MediaConfiguration
{
public:
    /**
     * Enum for the period to transmit SPS/PPS
     */
    enum
    {
        /** Default value */
        SEND_ONCE_AT_START = 0,
        /** Every second */
        SEND_EVERY_TIME = 1,
        /** Every 10 seconds */
        SEND_FOR_10SEC = 2
    };

    static const IMS_SINT32 DEFAULT_AS_VIDEO = 960;
    static const IMS_SINT32 DEFAULT_RS_VIDEO = 8000;
    static const IMS_SINT32 DEFAULT_RR_VIDEO = 6000;
    static const IMS_SINT32 DEFAULT_VIDEO_DSCP = 34;
    static const IMS_SINT32 DEFAULT_SEND_PERIODIC_SPS_PPS = SEND_EVERY_TIME;
    static const IMS_SINT32 DEFAULT_CVO_ID = -1;  // TODO: need to check.
    static const IMS_BOOL DEFAULT_AVPF_ENABLED = IMS_FALSE;
    static const IMS_SINT32 DEFAULT_AVPF_FEATURE = 0;
    static const IMS_BOOL DEFAULT_AVPF_TRR = IMS_FALSE;
    static const IMS_BOOL DEFAULT_AVPF_NACK = IMS_TRUE;
    static const IMS_BOOL DEFAULT_AVPF_TMMBR = IMS_TRUE;
    static const IMS_BOOL DEFAULT_AVPF_PLI = IMS_TRUE;
    static const IMS_BOOL DEFAULT_AVPF_FIR = IMS_TRUE;
    static const IMS_BOOL DEFAULT_AVPF_CAPA_NEGO = MediaConfiguration::CAPNEG_OFFER_WITH_ACAP;
    static const IMS_SINT32 DEFAULT_TMMBR_DOWN_INTERVAL = 5;
    static const IMS_SINT32 DEFAULT_TMMBR_UP_INTERVAL = 10;
    static const IMS_SINT32 DEFAULT_TMMBR_LOSS_RATIO = 5;
    static const IMS_SINT32 DEFAULT_TMMBR_MIN_BR = 192;
    static const IMS_SINT32 DEFAULT_TMMBR_BR_LEVEL = 5;
    static const IMS_SINT32 DEFAULT_TMMBR_UP_LEVEL = 1;
    static const IMS_SINT32 DEFAULT_I_FRAME_INTERVAL = 1;
    static const IMS_SINT32 DEFAULT_CHANNEL = 0;
    static const IMS_SINT32 DEFAULT_VIDEO_SAMPLING_RATE = 90000;
    static const IMS_BOOL DEFAULT_BW_NEGO_OPTION = MediaConfiguration::BW_OPTION_LOCAL_VALUE;
    static const IMS_SINT32 DEFAULT_VIDEO_LOWEST_BITRATE = 0;

public:
    /**
     * @brief Constructs a new VideoConfiguration object.
     *
     * @param eSessionType The type of media session.
     */
    explicit VideoConfiguration(IN MEDIA_CONTENT_TYPE eSessionType = MEDIA_TYPE_AUDIOVIDEO);

    /**
     * @brief Destroys the VideoConfiguration object.
     */
    ~VideoConfiguration() override;

    /**
     * @brief Initializes the video configuration by reading carrier-specific settings.
     *
     * @param piCc A pointer to the carrier configuration interface.
     * @return IMS_TRUE on success, IMS_FALSE on failure.
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc) override;

    /**
     * @brief Updates the video configuration with new carrier-specific settings.
     *
     * @param piCc A pointer to the carrier configuration interface.
     * @return IMS_TRUE on success, IMS_FALSE on failure.
     */
    virtual IMS_BOOL Update(IN ICarrierConfig* piCc) override;

    /**
     * @brief Gets the Differentiated Services Code Point (DSCP) value for video RTP packets.
     *
     * @return The DSCP value.
     */
    virtual IMS_SINT32 GetVideoDscp() const;

    /**
     * @brief Gets the period for sending SPS/PPS NAL units.
     *
     * @return The sending period setting. See {@link VideoConfiguration::SEND_ONCE_AT_START}.
     */
    virtual IMS_SINT32 GetVideoSendPeriodicSpsPps() const;

    /**
     * @brief Gets the Coordination of Video Orientation (CVO) identifier.
     *
     * @return The CVO identifier.
     */
    virtual IMS_SINT32 GetCvoId() const;

    /**
     * @brief Checks if the Audio-Visual Profile with Feedback (AVPF) is enabled for video.
     *
     * @return IMS_TRUE if AVPF is enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsVideoAvpfEnabled() const;

    /**
     * @brief Checks if AVPF Temporal-Spatial Trade-off Request (TRR) is enabled.
     *
     * @return IMS_TRUE if TRR is enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsVideoAvpfTrrEnabled() const;

    /**
     * @brief Checks if AVPF Generic NACK is enabled.
     *
     * @return IMS_TRUE if NACK is enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsVideoAvpfNackEnabled() const;

    /**
     * @brief Checks if AVPF Temporary Maximum Media Stream Bit Rate Request (TMMBR) is enabled.
     *
     * @return IMS_TRUE if TMMBR is enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsVideoAvpfTmmbrEnabled() const;

    /**
     * @brief Checks if AVPF Picture Loss Indication (PLI) is enabled.
     *
     * @return IMS_TRUE if PLI is enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsVideoAvpfPliEnabled() const;

    /**
     * @brief Checks if AVPF Full Intra Request (FIR) is enabled.
     *
     * @return IMS_TRUE if FIR is enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsVideoAvpfFirEnabled() const;

    /**
     * @brief Checks if capability negotiation for AVPF is enabled.
     *
     * @return IMS_TRUE if AVPF capability negotiation is enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsAvpfCapabilityNegotiationEnabled() const;

    /**
     * @brief Gets the SDP offer capability negotiation option for AVPF.
     *
     * @return The capability negotiation option. See {@link MediaConfiguration::CAPNEG_OFFER_NONE}.
     */
    virtual IMS_SINT32 GetSdpOfferCapNegoForAvpf() const;

    /**
     * @brief Gets the interval for sending I-frames (intra-frames).
     *
     * @return The I-frame interval in seconds.
     */
    virtual IMS_SINT32 GetVideoIframeIntervalSec() const;

    /**
     * @brief Gets the number of channels for video.
     *
     * @return The number of channels.
     */
    virtual IMS_SINT32 GetChannel() const;

    /**
     * @brief Gets the sampling rate for the video clock.
     *
     * @return The sampling rate in Hz (e.g., 90000).
     */
    virtual IMS_SINT32 GetVideoSamplingRate() const;

    /**
     * @brief Gets the bandwidth negotiation option for video.
     *
     * @return IMS_TRUE to use the remote party's bandwidth values, IMS_FALSE to use local values.
     */
    virtual IMS_BOOL GetBandwidthNegoOption() const;

    /**
     * @brief Gets the lowest allowed bitrate for video.
     *
     * @return The lowest bitrate in bits per second (bps).
     */
    virtual IMS_SINT32 GetVideoLowestBitrateBps() const;

    /**
     * @brief Checks if the media direction should be set to 'inactive' when the video is on hold.
     *
     * @return IMS_TRUE to use 'inactive' on hold, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL isVideoDirectionHoldUsingInactive() const;

protected:
    /**
     * @brief Create a codec configs
     *
     * @param piCc configuration
     * @return true Return the create function is executed without error
     * @return false Return the create function is failed
     */
    IMS_BOOL CreateCodecConfigs(IN ICarrierConfig* piCc) override;
    /**
     * @brief Print debugstring
     *
     */
    void ToDebugString() const override;

private:
    IMS_SINT32 m_nVideoDscp;
    IMS_SINT32 m_nVideoSendPeriodicSpsPps;
    IMS_SINT32 m_nCvoId;
    IMS_BOOL m_bVideoAvpfEnabled;
    IMS_BOOL m_bVideoAvpfTrrEnabled;
    IMS_BOOL m_bVideoAvpfNackEnabled;
    IMS_BOOL m_bVideoAvpfTmmbrEnabled;
    IMS_BOOL m_bVideoAvpfPliEnabled;
    IMS_BOOL m_bVideoAvpfFirEnabled;
    IMS_SINT32 m_nSdpOfferCapNegoForAvpf;
    IMS_SINT32 m_nVideoIframeIntervalSec;
    IMS_SINT32 m_nChannel;
    IMS_SINT32 m_nVideoSamplingRate;
    IMS_BOOL m_bVideoBwNegoOptionEnabled;
    IMS_SINT32 m_nVideoLowestBitrateBps;
    IMS_BOOL m_bVideoHoldDirectionInactive;
};

#endif
