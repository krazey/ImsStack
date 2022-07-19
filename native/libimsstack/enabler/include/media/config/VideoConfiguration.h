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

#ifndef _VIDEO_CONFIGURATION_H_
#define _VIDEO_CONFIGURATION_H_

#include "config/MediaConfiguration.h"

class ICarrierConfig;

/*!
 * @class   VideoConfiguration
 * @brief   Video Configuration class
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

    static const IMS_SINT32 DEFAULT_RTP_PORT = 50010;
    static const IMS_SINT32 DEFAULT_RTP_PORT_END = 50060;
    static const IMS_SINT32 DEFAULT_RTCP_PORT = 50011;
    static const IMS_SINT32 DEFAULT_RTCP_INVERVAL_LIVE = 5;
    static const IMS_SINT32 DEFAULT_RTCP_INVERVAL = 5;
    static const IMS_SINT32 DEFAULT_AS = 960;
    static const IMS_SINT32 DEFAULT_RS = 8000;
    static const IMS_SINT32 DEFAULT_RR = 6000;
    static const IMS_SINT32 DEFAULT_RTP_INACTIVITY = 20000;
    static const IMS_SINT32 DEFAULT_RTCP_INACTIVITY = 200000;

    static const IMS_SINT32 DEFAULT_VIDEO_DSCP = 40;
    static const IMS_SINT32 DEFAULT_SEND_PERIODIC_SPS_PPS = SEND_EVERY_TIME;
    static const IMS_SINT32 DEFAULT_CVO_ID = -1;  // TODO_MEDIA need to check.
    static const IMS_BOOL DEFAULT_AVPF_ENABLED = IMS_FALSE;
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
    static const IMS_SINT32 DEFAULT_CHANNEL = 1;
    static const IMS_SINT32 DEFAULT_VIDEO_SAMPLING_RATE = 90000;

public:
    /**
     * @brief Construct a new video configuration
     *
     * @param _nSessionType mediasession type
     */
    VideoConfiguration(IN MEDIA_CONTENT_TYPE _nSessionType = MEDIA_TYPE_AUDIOVIDEO);
    /**
     * @brief Destroy the video configuration
     *
     */
    virtual ~VideoConfiguration();
    /**
     * @brief Create codec using the configuration
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);
    /**
     * @brief Update codec using the configuration
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Update(IN ICarrierConfig* piCc);
    /**
     * @brief Get the video dscp value
     *
     * @return IMS_SINT32 Return video dscp value
     */
    IMS_SINT32 GetVideoDscp() const;
    /**
     * @brief Get the sps/pps sending perioid
     *
     * @return IMS_SINT32 Return the sps/pps sending perioid
     */
    IMS_SINT32 GetVideoSendPeriodicSpsPps() const;
    /**
     * @brief Get the cvo id
     *
     * @return IMS_SINT32 Return the cvo-id
     */
    IMS_SINT32 GetCvoId() const;
    /**
     * @brief Get whether AVPF feature is enabled
     *
     * @return IMS_BOOL Return true if AVPF is enabled
     * Return false if AVPF is disabled
     */
    IMS_BOOL IsVideoAvpfEnabled() const;
    /**
     * @brief Get whether AVPF trr attribute is enabled
     *
     * @return IMS_BOOL Return true if AVPF trr is enabled
     * Return false if AVPF trr is disabled
     */
    IMS_BOOL IsVideoAvpfTrrEnabled() const;
    /**
     * @brief Get whether AVPF nack attribute is enabled
     *
     * @return IMS_BOOL Return true if AVPF nack is enabled
     * Return false if AVPF nack is disabled
     */
    IMS_BOOL IsVideoAvpfNackEnabled() const;
    /**
     * @brief Get whether AVPF tmmbr attribute is enabled
     *
     * @return IMS_BOOL Return true if AVPF tmmbr is enabled
     * Return false if AVPF tmmbr is disabled
     */
    IMS_BOOL IsVideoAvpfTmmbrEnabled() const;
    /**
     * @brief Get whether AVPF pli attribute is enabled
     *
     * @return IMS_BOOL Return true if AVPF pli is enabled
     * Return false if AVPF pli is disabled
     */
    IMS_BOOL IsVideoAvpfPliEnabled() const;
    /**
     * @brief Get whether AVPF fir attribute is enabled
     *
     * @return IMS_BOOL Return true if AVPF fir is enabled
     * Return false if AVPF fir is disabled
     */
    IMS_BOOL IsVideoAvpfFirEnabled() const;
    /**
     * @brief Get whether the sdp offer cap nego for avpf
     *
     * @return IMS_BOOL Return true if SdpOfferCapNegoForAvpf is enabled
     * Return false if SdpOfferCapNegoForAvpf is disabled
     */
    IMS_SINT32 GetSdpOfferCapNegoForAvpf() const;
    /**
     * @brief Get the video iframe interval sec
     *
     * @return IMS_SINT32 Return the interval of the video iframe
     */
    IMS_SINT32 GetVideoIframeIntervalSec() const;
    /**
     * @brief Get the channel id
     *
     * @return IMS_SINT32 Return the channel-id
     */
    IMS_SINT32 GetChannel() const;
    /**
     * @brief Get the video sampling rate
     *
     * @return IMS_SINT32 Return the video sampling rate
     */
    IMS_SINT32 GetVideoSamplingRate() const;
    /**
     * @brief Get whether the bandwidth nego option
     *
     * @return IMS_BOOL Return true if BandwidthNegoOption is enabled
     * Return false if BandwidthNegoOption is disabled
     */
    IMS_BOOL GetBandwidthNegoOption() const { return 0; }

protected:
    /**
     * @brief Create a codec configs
     *
     * @param piCc configuration
     * @return true Return the create function is executed without error
     * @return false Return the create function is failed
     */
    virtual IMS_BOOL CreateCodecConfigs(IN ICarrierConfig* piCc);
    /**
     * @brief Print debugstring
     *
     */
    virtual void ToDebugString() const;

private:
    IMS_SINT32 nVideoDscp;
    IMS_SINT32 nVideoSendPeriodicSpsPps;
    IMS_SINT32 nCvoId;
    IMS_BOOL bVideoAvpfEnabled;
    IMS_BOOL bVideoAvpfTrrEnabled;
    IMS_BOOL bVideoAvpfNackEnabled;
    IMS_BOOL bVideoAvpfTmmbrEnabled;
    IMS_BOOL bVideoAvpfPliEnabled;
    IMS_BOOL bVideoAvpfFirEnabled;
    IMS_SINT32 nSdpOfferCapNegoForAvpf;
    IMS_SINT32 nVideoIframeIntervalSec;
    IMS_SINT32 nChannel;
    IMS_SINT32 nVideoSamplingRate;
};
#endif  // _VIDEO_CONFIGURATION_H_
