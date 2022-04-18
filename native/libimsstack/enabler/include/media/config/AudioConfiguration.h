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

#ifndef _AUDIO_CONFIGURATION_H_
#define _AUDIO_CONFIGURATION_H_

#include "AString.h"
#include "config/MediaConfiguration.h"

class ICarrierConfig;

class AudioConfiguration
        : public MediaConfiguration
{
public:
    AudioConfiguration(MEDIA_CONTENT_TYPE eSessionType = MEDIA_TYPE_AUDIO);
    virtual ~AudioConfiguration();

public:
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);
    virtual IMS_BOOL Update(IN ICarrierConfig* piCc);

protected:
    virtual IMS_BOOL CreateCodecConfigs(IN ICarrierConfig* piCc);
    virtual void ToDebugString() const;

public:
    IMS_SINT32 GetPtime() const;
    IMS_SINT32 GetMaxPtime() const;
    IMS_SINT32 GetMaxRed() const;
    IMS_BOOL GetBandwidthNegoOption() const;
    IMS_SINT32 GetRtpDscp() const;
    IMS_SINT32 GetJitterBufferMinSize() const;
    IMS_SINT32 GetJitterBufferMaxSize() const;
    IMS_SINT32 GetJitterBufferAdjustTime() const;
    IMS_SINT32 GetJitterBufferStepSize() const;
    IMS_BOOL IsRtcpXrEnabled() const;
    IMS_BOOL IsRtcpXrStatisticsEnabled() const;
    IMS_BOOL IsRtcpXrVoipEnabled() const;
    IMS_BOOL IsRtcpXrPlrEnabled() const;
    IMS_BOOL IsRtcpXrPdrEnabled() const;
    IMS_SINT32 GetDTMFDuration() const;
    IMS_SINT32 GetModeChangeCapability() const;
    IMS_SINT32 GetModeChangePeriod() const;
    IMS_SINT32 GetModeChangeNeighbor() const;
    const IMSVector<AString>& GetAudioCandidateAttribute() const;

public:
    enum
    {
        AUDIO_HALFRATE_SETTING = 10
    };   // setting at "dm_operation_preferred_mode" Media DB table.

    enum
    {
        BW_OPTION_LOCAL_VALUE = 0,
        BW_OPTION_REMOTE_VALUE = 1
    };

    enum
    {
        DEFAULT_VOCODER_INTERFACE = 2
    }; //[VOCODER_INTERFACE] DEFAULT_VOCODER_INTERFACE is CVD.

    static const IMS_SINT32 NEED_TO_CHECK_I = 0;

    static const IMS_SINT32 DEFAULT_PTIME = 20;
    static const IMS_SINT32 DEFAULT_MAX_PTIME = 240;
    static const IMS_SINT32 DEFAULT_MAX_RED = DEFAULT_MAX_PTIME - DEFAULT_PTIME;
    static const IMS_BOOL DEFAULT_BW_NEGO_OPERION = BW_OPTION_LOCAL_VALUE;
    static const IMS_SINT32 DEFAULT_AUDIO_DSCP = 184; // TODO_MEDIA check default value
    static const IMS_SINT32 DEFAULT_JITTER_MIN = 4;
    static const IMS_SINT32 DEFAULT_JITTER_MAX = 9;
    static const IMS_SINT32 DEFAULT_JITTER_ADJUST = 4;
    static const IMS_SINT32 DEFAULT_JITTER_STEP = 4;
    static const IMS_BOOL DEFAULT_RTCPXR = IMS_FALSE;
    static const IMS_BOOL DEFAULT_RTCPXR_STATISTICS = IMS_FALSE;
    static const IMS_BOOL DEFAULT_RTCPXR_VOIP_METRICS = IMS_FALSE;
    static const IMS_BOOL DEFAULT_RTCPXR_PACKET_LOSS_RLE = IMS_FALSE;
    static const IMS_BOOL DEFAULT_RTCPXR_PACKET_DUPLICATE_RLE = IMS_FALSE;
    static const IMS_SINT32 DEFAULT_DTMF_DURATION = 200;
    static const IMS_SINT32 DEFAULT_MODECHANGE_CAPABILITY = 1;
    static const IMS_SINT32 DEFAULT_MODECHANGE_PERIOD = 1;
    static const IMS_SINT32 DEFAULT_MODECHANGE_NEIGHBOR = 0;
    #define DEFAULT_CANDIDATE_ATTRIBUTE "1, UDP, 1119400811, 10.3.210.77, 7010, typ, host"

private:
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
    IMS_SINT32 m_nModeChangeCapability;
    IMS_SINT32 m_nModeChangePeriod;
    IMS_SINT32 m_nModeChangeNeighbor;
    IMSVector<AString> m_objAudioCandidateAttribute;
};
#endif                                              // _AUDIO_CONFIGURATION_H_
