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
    IMS_BOOL Create(IN ICarrierConfig* piCc) override;
    /**
     * @brief Update the carrier configuration items
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    IMS_BOOL Update(IN ICarrierConfig* piCc) override;
    /**
     * @brief Get whether EVS codec is supported
     *
     * @return IMS_BOOL Return true if evs is supported
     * Return false if evs is not supported
     */
    IMS_BOOL IsEvsSupported() const;
    /**
     * @brief Get the ptime (recommended length of time in milliseconds represented by the media in
     * a packet)
     *
     * @return IMS_SINT32 Return ptime
     */
    IMS_SINT32 GetPtime() const;
    /**
     * @brief Get the maxptime (maximum amount of media that can be encapsulated in each packet)
     *
     * @return IMS_SINT32 Return maxptime
     */
    IMS_SINT32 GetMaxPtime() const;
    /**
     * @brief Get the maxredundancy (The maximum duration in milliseconds between the primary and
     * redundant transmission)
     *
     * @return IMS_SINT32 Return max-red
     */
    IMS_SINT32 GetMaxRed() const;
    /**
     * @brief Get the bandwidthnegooption
     *
     * @return IMS_BOOL Return true if the audio bandwidth nego option is enabled
     * Return false if the audio bandwidth nego option is disabled
     */
    IMS_BOOL GetBandwidthNegoOption() const;
    /**
     * @brief Get the dscp (Differentiated Services Code Point) for rtp
     *
     * @return IMS_SINT32 Return dscp value
     */
    IMS_SINT32 GetRtpDscp() const;
    /**
     * @brief Get the min jitter buffer size
     *
     * @return IMS_SINT32 min jitterbuffer size
     */
    IMS_SINT32 GetJitterBufferMinSize() const;
    /**
     * @brief Get the max jitter buffer size
     *
     * @return IMS_SINT32 Return max jitterbuffer size
     */
    IMS_SINT32 GetJitterBufferMaxSize() const;
    /**
     * @brief Get the adjust time for jitter buffer
     *
     * @return IMS_SINT32 Return adjust time for jitter buffer
     */
    IMS_SINT32 GetJitterBufferAdjustTime() const;
    /**
     * @brief Get the jitterbuffer size change unit
     *
     * @return IMS_SINT32 Return jitterbuffer size change unit
     */
    IMS_SINT32 GetJitterBufferStepSize() const;
    /**
     * @brief Get whether rtcp-xr feature is enabled
     *
     * @return IMS_BOOL Return true if rtcp-xr is enabled
     * Return false if rtcp-xr is disabled
     */
    IMS_BOOL IsRtcpXrEnabled() const;
    /**
     * @brief Get whether rtcp-xr statistics feature is enabled
     *
     * @return IMS_BOOL Return true if rtcp-xr statistics is enabled
     * Return false if rtcp-xr statistics is disabled
     */
    IMS_BOOL IsRtcpXrStatisticsEnabled() const;
    /**
     * @brief Get whether rtcp-xr voip feature is enabled
     *
     * @return IMS_BOOL Return true if rtcp-xr voip is enabled
     * Return false if rtcp-xr voip is disabled
     */
    IMS_BOOL IsRtcpXrVoipEnabled() const;
    /**
     * @brief Get whether rtcp-xr plr feature is enabled
     *
     * @return IMS_BOOL Return true if rtcp-xr plr is enabled
     * Return false if rtcp-xr plr is disabled
     */
    IMS_BOOL IsRtcpXrPlrEnabled() const;
    /**
     * @brief Get whether rtcp-xr pdr feature is enabled
     *
     * @return IMS_BOOL Return true if rtcp-xr pdr is enabled
     * Return false if rtcp-xr pdr is disabled
     */
    IMS_BOOL IsRtcpXrPdrEnabled() const;
    /**
     * @brief Get dtmf playing duration in milliseconds unit
     *
     * @return IMS_SINT32 Return dtmf duration value
     */
    IMS_SINT32 GetDTMFDuration() const;
    /**
     * @brief Get the audio candidate attribute
     *
     * @return const ImsVector<AString>& Return audio-candidate-attribute
     */
    const ImsVector<AString>& GetAudioCandidateAttribute() const;

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
    static const IMS_BOOL DEFAULT_BW_NEGO_OPTION = MediaConfiguration::BW_OPTION_SOURCE_VALUE;
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
};

#endif
