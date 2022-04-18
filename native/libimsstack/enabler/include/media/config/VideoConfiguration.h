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
class VideoConfiguration :
        public MediaConfiguration
{
public:
    enum
    {
        FR_MODE_HIDE    = 0,
        FR_MDOE_AUTO    = 1,
        FR_MODE_MANUAL  = 2
    };
    enum
    {
        DB_RESOLUTION_QCIF_PR     = 0,
        DB_RESOLUTION_QVGA_LS     = 1,
        DB_RESOLUTION_QVGA_PR     = 2,
        DB_RESOLUTION_VGA_LS      = 3,
        DB_RESOLUTION_VGA_PR      = 4,
        DB_RESOLUTION_QCIF_LS     = 5,
        DB_RESOLUTION_CIF_LS      = 6,
        DB_RESOLUTION_CIF_PR      = 7,
        DB_RESOLUTION_SQCIF_LS    = 8,
        DB_RESOLUTION_SQCIF_PR    = 9,
        DB_RESOLUTION_SIF_LS      = 10,
        DB_RESOLUTION_SIF_PR      = 11,
        DB_RESOLUTION_HD_LS       = 12,
        DB_RESOLUTION_HD_PR       = 13,
        DB_RESOLUTION_FHD_LS      = 14,
        DB_RESOLUTION_FHD_PR      = 15,
        DB_RESOLUTION_MAX         = 99,
    };

    enum
    {
        SEND_ONCE_AT_START = 0,
        SEND_EVERY_TIME = 1,
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
    static const IMS_BOOL DEFAULT_AVPF_TRR = IMS_FALSE;
    static const IMS_BOOL DEFAULT_AVPF_NACK = IMS_TRUE;
    static const IMS_BOOL DEFAULT_AVPF_TMMBR = IMS_TRUE;
    static const IMS_BOOL DEFAULT_AVPF_PLI = IMS_TRUE;
    static const IMS_BOOL DEFAULT_AVPF_FIR = IMS_TRUE;
    static const IMS_SINT32 DEFAULT_TMMBR_DOWN_INTERVAL = 5;
    static const IMS_SINT32 DEFAULT_TMMBR_UP_INTERVAL = 10;
    static const IMS_SINT32 DEFAULT_TMMBR_LOSS_RATIO = 5;
    static const IMS_SINT32 DEFAULT_TMMBR_MIN_BR = 192;
    static const IMS_SINT32 DEFAULT_TMMBR_BR_LEVEL = 5;
    static const IMS_SINT32 DEFAULT_TMMBR_UP_LEVEL = 1;
    static const IMS_SINT32 DEFAULT_I_FRAME_INTERVAL = 1;
    static const IMS_BOOL DEFAULT_DROP_P_FRAME = IMS_FALSE;
    static const IMS_SINT32 DEFAULT_VIDEO_SAMPLING_RATE = 9000;

public:
    VideoConfiguration(IN MEDIA_CONTENT_TYPE _nSessionType = MEDIA_TYPE_AUDIOVIDEO);
    virtual ~VideoConfiguration();

public:
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);
    virtual IMS_BOOL Update(IN ICarrierConfig* piCc);

protected:
    virtual IMS_BOOL CreateCodecConfigs(IN ICarrierConfig* piCc);
    virtual void ToDebugString() const;

public:
    IMS_SINT32 GetVideoRtpDscp() const;
    IMS_SINT32 GetVideoSendPeriodicSpsPps() const;
    IMS_BOOL IsVideoAvpfTrrEnabled() const;
    IMS_BOOL IsbVideoAvpfNackEnabled() const;
    IMS_BOOL IsVideoAvpfTmmbrEnabled() const;
    IMS_BOOL IsVideoAvpfPliEnabled() const;
    IMS_BOOL IsVideoAvpfFirEnabled() const;
    IMS_SINT32 GetVideoAvpfTmmbrDownIntervalSec() const;
    IMS_SINT32 GetVideoAvpfTmmbrUpIntervalSec() const;
    IMS_SINT32 GetVideoAvpfTmmbrLossThresholdRatio() const;
    IMS_SINT32 GetVideoAvpfTmmbrMinBitrateKbps() const;
    IMS_SINT32 GetVideoAvpfTmmbrBitrateLevel() const;
    IMS_SINT32 GetVideoAvpfTmmbrUpLevel() const;
    IMS_SINT32 GetVideoIframeIntervalSec() const;
    IMS_BOOL IsVideoDropPFrameEnabled() const;
    IMS_SINT32 GetVideoSamplingRate() const;

private:
    IMS_SINT32 nVideoRtpDscp;
    IMS_SINT32 nVideoSendPeriodicSpsPps;
    IMS_BOOL bVideoAvpfTrrEnabled;
    IMS_BOOL bVideoAvpfNackEnabled;
    IMS_BOOL bVideoAvpfTmmbrEnabled;
    IMS_BOOL bVideoAvpfPliEnabled;
    IMS_BOOL bVideoAvpfFirEnabled;
    IMS_SINT32 nVideoAvpfTmmbrDownIntervalSec;
    IMS_SINT32 nVideoAvpfTmmbrUpIntervalSec;
    IMS_SINT32 nVideoAvpfTmmbrLossThresholdRatio;
    IMS_SINT32 nVideoAvpfTmmbrMinBitrateKbps;
    IMS_SINT32 nVideoAvpfTmmbrBitrateLevel;
    IMS_SINT32 nVideoAvpfTmmbrUpLevel;
    IMS_SINT32 nVideoIframeIntervalSec;
    IMS_BOOL bVideoDropPFrameEnabled;
    IMS_SINT32 nVideoSamplingRate;
};
#endif                                              // _VIDEO_CONFIGURATION_H_
