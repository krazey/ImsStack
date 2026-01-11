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

#ifndef AUDIO_CONFIGURATION_H_
#define AUDIO_CONFIGURATION_H_

#include "AString.h"
#include "config/MediaConfiguration.h"

/**
 * @class AudioConfiguration
 * @brief Manages audio-specific media configurations.
 * @details This class holds all configuration parameters related to audio streams,
 * such as codecs, RTP/RTCP settings, bandwidth, and jitter buffer parameters.
 */
class AudioConfiguration : public MediaConfiguration
{
public:
    /**
     * @brief Constructs a new AudioConfiguration object.
     *
     * @param eSessionType The media content type, defaulting to MEDIA_TYPE_AUDIO.
     */
    explicit AudioConfiguration(MEDIA_CONTENT_TYPE eSessionType = MEDIA_TYPE_AUDIO);

    /**
     * @brief Destroys the AudioConfiguration object.
     */
    ~AudioConfiguration() override;

    /**
     * @brief Initializes the audio configuration by reading carrier-specific settings.
     *
     * @param piCc A pointer to the carrier configuration interface.
     * @return IMS_TRUE on success, IMS_FALSE on failure.
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc) override;

    /**
     * @brief Updates the audio configuration with new carrier-specific settings.
     *
     * @param piCc A pointer to the carrier configuration interface.
     * @return IMS_TRUE on success, IMS_FALSE on failure.
     */
    virtual IMS_BOOL Update(IN ICarrierConfig* piCc) override;

    /**
     * @brief Checks if the EVS (Enhanced Voice Services) codec is supported.
     *
     * @return IMS_TRUE if EVS is supported, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsEvsSupported() const;

    /**
     * @brief Gets the packet time (ptime), the recommended duration of media in a single packet.
     *
     * @return The ptime value in milliseconds.
     */
    virtual IMS_SINT32 GetPtime() const;

    /**
     * @brief Gets the maximum packet time (maxptime), the maximum duration of media that can be
     * encapsulated in a packet.
     *
     * @return The maxptime value in milliseconds.
     */
    virtual IMS_SINT32 GetMaxPtime() const;

    /**
     * @brief Gets the maximum duration in milliseconds for redundant audio data.
     *
     * @return The maximum redundancy value in milliseconds.
     */
    virtual IMS_SINT32 GetMaxRed() const;

    /**
     * @brief Gets the bandwidth negotiation option for audio.
     *
     * @return IMS_TRUE to use the remote party's bandwidth values, IMS_FALSE to use local values.
     */
    virtual IMS_BOOL GetBandwidthNegoOption() const;

    /**
     * @brief Gets the Differentiated Services Code Point (DSCP) value for audio RTP packets.
     *
     * @return The DSCP value.
     */
    virtual IMS_SINT32 GetRtpDscp() const;

    /**
     * @brief Gets the minimum size of the jitter buffer.
     *
     * @return The minimum jitter buffer size in milliseconds.
     */
    virtual IMS_SINT32 GetJitterBufferMinSize() const;

    /**
     * @brief Gets the maximum size of the jitter buffer.
     *
     * @return The maximum jitter buffer size in milliseconds.
     */
    virtual IMS_SINT32 GetJitterBufferMaxSize() const;

    /**
     * @brief Gets the adjustment time for the jitter buffer.
     *
     * @return The jitter buffer adjustment time in milliseconds.
     */
    virtual IMS_SINT32 GetJitterBufferAdjustTime() const;

    /**
     * @brief Gets the step size for jitter buffer adjustments.
     *
     * @return The jitter buffer step size in milliseconds.
     */
    virtual IMS_SINT32 GetJitterBufferStepSize() const;

    /**
     * @brief Checks if RTCP Extended Reports (RTCP-XR) are enabled.
     *
     * @return IMS_TRUE if RTCP-XR is enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsRtcpXrEnabled() const;

    /**
     * @brief Checks if RTCP-XR statistics reporting is enabled.
     *
     * @return IMS_TRUE if RTCP-XR statistics are enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsRtcpXrStatisticsEnabled() const;

    /**
     * @brief Checks if RTCP-XR VoIP metrics reporting is enabled.
     *
     * @return IMS_TRUE if RTCP-XR VoIP metrics are enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsRtcpXrVoipEnabled() const;

    /**
     * @brief Checks if RTCP-XR Packet Loss RLE (Run Length Encoding) is enabled.
     *
     * @return IMS_TRUE if Packet Loss RLE is enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsRtcpXrPlrEnabled() const;

    /**
     * @brief Checks if RTCP-XR Packet Duplicate RLE (Run Length Encoding) is enabled.
     *
     * @return IMS_TRUE if Packet Duplicate RLE is enabled, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsRtcpXrPdrEnabled() const;

    /**
     * @brief Gets the duration for playing a DTMF (Dual-Tone Multi-Frequency) tone.
     *
     * @return The DTMF duration in milliseconds.
     */
    virtual IMS_SINT32 GetDtmfDuration() const;

    /**
     * @brief Gets the list of ICE (Interactive Connectivity Establishment) candidate attributes for
     * audio.
     *
     * @return A vector of strings representing the audio candidate attributes.
     */
    virtual const ImsVector<AString>& GetAudioCandidateAttribute() const;

    /**
     * @brief Checks if a given reason is configured to trigger call termination upon media
     * inactivity.
     *
     * @param nReason The inactivity reason to check. See {@link MEDIA_INACTIVITY_CALL_END_REASON}.
     * @return IMS_TRUE if the reason should end the call, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsAudioInactivityCallEndReason(IN IMS_SINT32 nReason) const;

    /**
     * @brief Determines whether AMR codec payload formats BE/OA should be treated as relaxed
     matching comparisons during codec negotiation.
     *
     * @return IMS_TRUE if the internal asset is set to TRUE to ignore the differences in AMR codec
     * payload formats (BE/OA) and perform codec negotiation, otherwise IMS_FALSE
     */
    virtual IMS_BOOL IsAmrPayloadFormatRelaxedMatching() const;

    /**
     * @brief Determines whether to enable codec-based dynamic AS value calculation.
     *
     * @return IMS_TRUE to enable dynamic AS calculation, IMS_FALSE to use the static value from
     * config.
     */
    virtual IMS_BOOL IsCodecBasedDynamicAsEnabled() const;

public:
    enum
    {
        /** setting at "dm_operation_preferred_mode" Media DB table */
        AUDIO_HALFRATE_SETTING = 10
    };

    enum
    {
        /** [VOCODER_INTERFACE] DEFAULT_VOCODER_INTERFACE is CVD */
        DEFAULT_VOCODER_INTERFACE = 2
    };

    static const IMS_SINT32 NEED_TO_CHECK_I = 0;
    static const IMS_SINT32 DEFAULT_SUPPORT_EVS = IMS_FALSE;
    static const IMS_SINT32 DEFAULT_PTIME = 20;
    static const IMS_SINT32 DEFAULT_MAX_PTIME = 240;
    static const IMS_SINT32 DEFAULT_MAX_RED = DEFAULT_MAX_PTIME - DEFAULT_PTIME;
    static const IMS_BOOL DEFAULT_BW_NEGO_OPTION = MediaConfiguration::BW_OPTION_LOCAL_VALUE;
    static const IMS_SINT32 DEFAULT_AUDIO_DSCP = 46;
    static const IMS_SINT32 DEFAULT_JITTER_MIN = 0;
    static const IMS_SINT32 DEFAULT_JITTER_MAX = 0;
    static const IMS_SINT32 DEFAULT_JITTER_ADJUST = 0;
    static const IMS_SINT32 DEFAULT_JITTER_STEP = 0;
    static const IMS_BOOL DEFAULT_RTCPXR = IMS_FALSE;
    static const IMS_BOOL DEFAULT_RTCPXR_STATISTICS = IMS_FALSE;
    static const IMS_BOOL DEFAULT_RTCPXR_VOIP_METRICS = IMS_FALSE;
    static const IMS_BOOL DEFAULT_RTCPXR_PACKET_LOSS_RLE = IMS_FALSE;
    static const IMS_BOOL DEFAULT_RTCPXR_PACKET_DUPLICATE_RLE = IMS_FALSE;
    static const IMS_SINT32 DEFAULT_DTMF_DURATION = 200;

protected:
    IMS_BOOL CreateCodecConfigs(IN ICarrierConfig* piCc) override;
    void ToDebugString() const override;

private:
    /** Indicates whether the EVS codec is supported. */
    IMS_BOOL m_bEvsSupported;
    /** The preferred packetization time (ptime) for audio packets in milliseconds. */
    IMS_SINT32 m_nAudioPtime;
    /** The maximum allowed packetization time (maxptime) for audio packets in milliseconds. */
    IMS_SINT32 m_nAudioMaxPtime;
    /** The maximum duration for redundant audio data in milliseconds. */
    IMS_SINT32 m_nAudioMaxRed;
    /** The bandwidth negotiation option for audio. */
    IMS_BOOL m_bAudioBwNegoOptionEnabled;
    /** The DSCP value for audio RTP packets. */
    IMS_SINT32 m_nAudioRtpDscp;
    /** The minimum size of the jitter buffer in milliseconds. */
    IMS_SINT32 m_nJitterBufferMinSize;
    /** The maximum size of the jitter buffer in milliseconds. */
    IMS_SINT32 m_nJitterBufferMaxSize;
    /** The adjustment time for the jitter buffer in milliseconds. */
    IMS_SINT32 m_nJitterBufferAdjustTime;
    /** The step size for jitter buffer adjustments in milliseconds. */
    IMS_SINT32 m_nJitterBufferStepSize;
    /** Indicates whether RTCP-XR is enabled for audio. */
    IMS_BOOL m_bAudioRtcpXrEnabled;
    /** Indicates whether RTCP-XR statistics reporting is enabled. */
    IMS_BOOL m_bAudioRtcpXrStatisticsEnabled;
    /** Indicates whether RTCP-XR VoIP metrics reporting is enabled. */
    IMS_BOOL m_bAudioRtcpXrVoipMetricsEnabled;
    /** Indicates whether RTCP-XR Packet Loss RLE reporting is enabled. */
    IMS_BOOL m_bAudioRtcpXrPacketLossRleEnabled;
    /** Indicates whether RTCP-XR Packet Duplicate RLE reporting is enabled. */
    IMS_BOOL m_bAudioRtcpXrPacketDuplicateRleEnabled;
    /** Indicates whether to treat amr codec payload format BE/OA as preferred */
    IMS_BOOL m_bAmrPayloadFormatRelaxedMatching;
    /** The duration for playing a DTMF tone in milliseconds. */
    IMS_SINT32 m_nDtmfDuration;
    /** Indicates whether to enable codec-based dynamic AS value calculation. */
    IMS_BOOL m_bCodecBasedDynamicAsEnabled;
    /** A list of ICE candidate attributes for audio. */
    ImsVector<AString> m_objAudioCandidateAttribute;
    /** A list of reasons that trigger call termination upon media inactivity. */
    ImsVector<IMS_SINT32> m_objAudioInactivityCallEndReasons;
};

#endif
