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

class AudioConfiguration : public MediaConfiguration
{
public:
    /**
     * @brief Construct a new audio configuration
     *
     * @param eSessionType the Media type
     */
    explicit AudioConfiguration(MEDIA_CONTENT_TYPE eSessionType = MEDIA_TYPE_AUDIO);

    /**
     * @brief Destroy the Audio Configuration
     *
     */
    virtual ~AudioConfiguration();

    /**
     * @brief Read the carrier configuration items
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc) override;

    /**
     * @brief Update the carrier configuration items
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Update(IN ICarrierConfig* piCc) override;

    /**
     * @brief Get whether EVS codec is supported
     *
     * @return IMS_BOOL Return true if evs is supported
     * Return false if evs is not supported
     */
    virtual IMS_BOOL IsEvsSupported() const;

    /**
     * @brief Get the ptime (recommended length of time in milliseconds represented by the media in
     * a packet)
     *
     * @return IMS_SINT32 Return ptime
     */
    virtual IMS_SINT32 GetPtime() const;

    /**
     * @brief Get the maxptime (maximum amount of media that can be encapsulated in each packet)
     *
     * @return IMS_SINT32 Return maxptime
     */
    virtual IMS_SINT32 GetMaxPtime() const;

    /**
     * @brief Get the maximum redundancy (The maximum duration in milliseconds between the primary
     * and redundant transmission)
     *
     * @return IMS_SINT32 Return max-red
     */
    virtual IMS_SINT32 GetMaxRed() const;

    /**
     * @brief Get the bandwidth negotiation option
     *
     * @return IMS_BOOL Return true if the audio bandwidth negotiation option is enabled
     * Return false if the audio bandwidth negotiation option is disabled
     */
    virtual IMS_BOOL GetBandwidthNegoOption() const;

    /**
     * @brief Get the dscp (Differentiated Services Code Point) for rtp
     *
     * @return IMS_SINT32 Return dscp value
     */
    virtual IMS_SINT32 GetRtpDscp() const;

    /**
     * @brief Get the min jitter buffer size
     *
     * @return IMS_SINT32 min jitter buffer size
     */
    virtual IMS_SINT32 GetJitterBufferMinSize() const;

    /**
     * @brief Get the max jitter buffer size
     *
     * @return IMS_SINT32 Return max jitter buffer size
     */
    virtual IMS_SINT32 GetJitterBufferMaxSize() const;

    /**
     * @brief Get the adjust time for jitter buffer
     *
     * @return IMS_SINT32 Return adjust time for jitter buffer
     */
    virtual IMS_SINT32 GetJitterBufferAdjustTime() const;

    /**
     * @brief Get the jitter buffer size change unit
     *
     * @return IMS_SINT32 Return jitter buffer size change unit
     */
    virtual IMS_SINT32 GetJitterBufferStepSize() const;

    /**
     * @brief Get whether the RTCP-XR feature is enabled
     *
     * @return IMS_BOOL Return true if the RTCP-XR is enabled
     * Return false if the RTCP-XR is disabled
     */
    virtual IMS_BOOL IsRtcpXrEnabled() const;

    /**
     * @brief Get whether the RTCP-XR statistics feature is enabled
     *
     * @return IMS_BOOL Return true if the RTCP-XR statistics is enabled
     * Return false if the RTCP-XR statistics is disabled
     */
    virtual IMS_BOOL IsRtcpXrStatisticsEnabled() const;

    /**
     * @brief Get whether the RTCP-XR voip feature is enabled
     *
     * @return IMS_BOOL Return true if the RTCP-XR voip is enabled
     * Return false if the RTCP-XR voip is disabled
     */
    virtual IMS_BOOL IsRtcpXrVoipEnabled() const;

    /**
     * @brief Get whether the RTCP-XR plr feature is enabled
     *
     * @return IMS_BOOL Return true if the RTCP-XR plr is enabled
     * Return false if the RTCP-XR plr is disabled
     */
    virtual IMS_BOOL IsRtcpXrPlrEnabled() const;

    /**
     * @brief Get whether the RTCP-XR pdr feature is enabled
     *
     * @return IMS_BOOL Return true if the RTCP-XR pdr is enabled
     * Return false if the RTCP-XR pdr is disabled
     */
    virtual IMS_BOOL IsRtcpXrPdrEnabled() const;

    /**
     * @brief Get dtmf playing duration in milliseconds unit
     *
     * @return IMS_SINT32 Return dtmf duration value
     */
    virtual IMS_SINT32 GetDtmfDuration() const;

    /**
     * @brief Get the audio candidate attribute
     *
     * @return const ImsVector<AString>& Return audio-candidate-attribute
     */
    virtual const ImsVector<AString>& GetAudioCandidateAttribute() const;

    /**
     * @brief Get the call end reasons after the expiry of the inactivity timer
     *
     * RTCP_INACTIVITY_ON_HOLD = 0,
     * RTCP_INACTIVITY_ON_CONNECTED = 1,
     * RTP_INACTIVITY_ON_CONNECTED = 2,
     * E911_RTCP_INACTIVITY_ON_CONNECTED = 3,
     * E911_RTP_INACTIVITY_ON_CONNECTED = 4
     * @return IMS_BOOL Return the result whether input value is included in array
     */
    virtual IMS_BOOL IsAudioInactivityCallEndReason(IN IMS_SINT32 nReason) const;

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
    IMS_BOOL m_bEvsSupported;
    IMS_SINT32 m_nAudioPtime;
    IMS_SINT32 m_nAudioMaxPtime;
    IMS_SINT32 m_nAudioMaxRed;
    IMS_BOOL m_bAudioBwNegoOptionEnabled;
    IMS_SINT32 m_nAudioRtpDscp;
    IMS_SINT32 m_nJitterBufferMinSize;
    IMS_SINT32 m_nJitterBufferMaxSize;
    IMS_SINT32 m_nJitterBufferAdjustTime;
    IMS_SINT32 m_nJitterBufferStepSize;
    IMS_BOOL m_bAudioRtcpxrEnabled;
    IMS_BOOL m_bAudioRtcpxrStatisticsEnabled;
    IMS_BOOL m_bAudioRtcpxrVoipMetricsEnabled;
    IMS_BOOL m_bAudioRtcpxrPacketLossRleEnabled;
    IMS_BOOL m_bAudioRtcpxrPacketDuplicateRleEnabled;
    IMS_SINT32 m_nDtmfDuration;
    ImsVector<AString> m_objAudioCandidateAttribute;
    ImsVector<IMS_SINT32> m_objAudioInactivityCallEndReasons;
};

#endif
