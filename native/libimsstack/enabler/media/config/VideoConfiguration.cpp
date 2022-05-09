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

#include "ServiceTrace.h"
#include "IMSVector.h"
#include "config/ImsCodec.h"
#include "config/VideoConfiguration.h"

__IMS_TRACE_TAG_USER_DECL__("MED.CONF");

/*!
 * @brief   Creator
 * @details
 */
PUBLIC
VideoConfiguration::VideoConfiguration(IN MEDIA_CONTENT_TYPE _nSessionType) :
        MediaConfiguration(_nSessionType),
        nVideoRtpDscp(DEFAULT_VIDEO_DSCP),
        nVideoSendPeriodicSpsPps(DEFAULT_SEND_PERIODIC_SPS_PPS),
        bVideoAvpfTrrEnabled(DEFAULT_AVPF_TRR),
        bVideoAvpfNackEnabled(DEFAULT_AVPF_NACK),
        bVideoAvpfTmmbrEnabled(DEFAULT_AVPF_TMMBR),
        bVideoAvpfPliEnabled(DEFAULT_AVPF_PLI),
        bVideoAvpfFirEnabled(DEFAULT_AVPF_FIR),
        nVideoAvpfTmmbrDownIntervalSec(DEFAULT_TMMBR_DOWN_INTERVAL),
        nVideoAvpfTmmbrUpIntervalSec(DEFAULT_TMMBR_UP_INTERVAL),
        nVideoAvpfTmmbrLossThresholdRatio(DEFAULT_TMMBR_LOSS_RATIO),
        nVideoAvpfTmmbrMinBitrateKbps(DEFAULT_TMMBR_MIN_BR),
        nVideoAvpfTmmbrBitrateLevel(DEFAULT_TMMBR_BR_LEVEL),
        nVideoAvpfTmmbrUpLevel(DEFAULT_TMMBR_UP_LEVEL),
        nVideoIframeIntervalSec(DEFAULT_I_FRAME_INTERVAL),
        bVideoDropPFrameEnabled(IMS_FALSE),
        nVideoSamplingRate(DEFAULT_VIDEO_SAMPLING_RATE)
{
    IMS_TRACE_D("+VideoConfiguration eSessionType(%d)", eSessionType, 0, 0);
    nAsBandwidthKbps = DEFAULT_AS;
    nRsBandwidthBps = DEFAULT_RR;
    nRrBandwidthBps = DEFAULT_RS;
}

/*!
 * @brief   Destructor
 * @details
 */
PUBLIC VIRTUAL VideoConfiguration::~VideoConfiguration()
{
    IMS_TRACE_I("~VideoConfiguration", 0, 0, 0);
    Clear();
}

/*!
 * @brief   Create
 * @details
 */
PUBLIC VIRTUAL IMS_BOOL VideoConfiguration::Create(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("Create", 0, 0, 0);

    if (piCc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Media Configuration attributes
    SetPorts(piCc, CarrierConfig::ImsVt::KEY_VIDEO_PORT_RTP_INT_ARRAY);
    SetRtcpIntervals(piCc, CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INTERVAL_INT_ARRAY);

    nAsBandwidthKbps = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AS_BANDWIDTH_KBPS_INT);
    nRsBandwidthBps = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RS_BANDWIDTH_BPS_INT);
    nRrBandwidthBps = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RR_BANDWIDTH_BPS_INT);

    nRtpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_INACTIVITY_TIMER_MILLIS_INT);
    nRtcpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INACTIVITY_TIMER_MILLIS_INT);

    // Video Configuration attributes
    nVideoRtpDscp = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_DSCP_INT);
    nVideoSendPeriodicSpsPps =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_SEND_PERIODIC_SPS_PPS_INT);

    bVideoAvpfTrrEnabled = piCc->GetBoolean(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_TRR_BOOL);
    bVideoAvpfNackEnabled = piCc->GetBoolean(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_NACK_BOOL);
    bVideoAvpfTmmbrEnabled = piCc->GetBoolean(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_TMMBR_BOOL);
    bVideoAvpfPliEnabled = piCc->GetBoolean(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_PLI_BOOL);
    bVideoAvpfFirEnabled = piCc->GetBoolean(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_FIR_BOOL);

    nVideoAvpfTmmbrDownIntervalSec =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_TMMBR_DOWN_INTERVAL_SEC_INT);
    nVideoAvpfTmmbrUpIntervalSec =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_TMMBR_UP_INTERVAL_SEC_INT);
    nVideoAvpfTmmbrLossThresholdRatio =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_TMMBR_LOSS_THRESHOLD_RATIO_INT);
    nVideoAvpfTmmbrMinBitrateKbps =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_TMMBR_MIN_BITRATE_KBPS_INT);
    nVideoAvpfTmmbrBitrateLevel =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_TMMBR_BITRATE_LEVEL_INT);
    nVideoAvpfTmmbrUpLevel = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_TMMBR_UP_LEVEL_INT);

    nVideoIframeIntervalSec = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_IFRAME_INTERVAL_SEC_INT);
    bVideoDropPFrameEnabled = piCc->GetBoolean(CarrierConfig::Assets::KEY_VIDEO_DROP_P_FRAME_BOOL);

    // nVideoSamplingRate = DEFAULT_VIDEO_SAMPLING_RATE; // already set by default at creator
    if (!CreateCodecConfigs(piCc))
    {
        IMS_TRACE_E(0, "Create - CreateCodecConfigs failure ", 0, 0, 0);
        return IMS_FALSE;
    }

    ToDebugString();

    return IMS_TRUE;
}

/*!
 * @brief   Update
 * @details Update the video configurations
 */
PUBLIC
IMS_BOOL VideoConfiguration::Update(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    Clear();
    if (!Create(piCc))
    {
        IMS_TRACE_E(0, "Re-create VideoConfiguration failure", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*!
 * @brief   CreateCodecConfigs
 * @details
 */

PROTECTED VIRTUAL IMS_BOOL VideoConfiguration::CreateCodecConfigs(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCodecConfigs : piCc is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    ICarrierConfig* piCcBundle =
            piCc->GetBundle(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

    if (piCcBundle == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMSVector<IMS_SINT32> objAvcPayloadType =
            piCcBundle->GetIntArray(CarrierConfig::ImsVt::KEY_H264_PAYLOAD_TYPE_INT_ARRAY);

    // TODO_MEDIA need to add after creating HEVC in CarrierConfig

    // IMSVector<IMS_SINT32> objHevcPayloadType = piCcBundle->GetIntArray(
    //         CarrierConfig::ImsVt::KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY);

    IMS_UINT32 nCodecCnt = 0;
    if (objAvcPayloadType.GetSize() > 0)
    {
        nCodecCnt = MakeEachCodecs(piCc, ImsCodec::VIDEO_AVC, nCodecCnt, objAvcPayloadType);
    }

    // TODO_MEDIA need to add after creating HEVC in CarrierConfig

    // if (objHevcPayloadType.GetSize() > 0)
    // {
    //     nCodecCnt = MakeEachCodecs(piCc, ImsCodec::VIDEO_HEVC, nCodecCnt, objHevcPayloadType);
    // }

    piCcBundle->ReleaseBundle();
    piCcBundle = IMS_NULL;

    return IMS_TRUE;
}

/*!
 * @brief   ToDebugString
 * @details Print ToDebugString
 */
PROTECTED VIRTUAL void VideoConfiguration::ToDebugString() const
{
    // MediaConfiguration::ToDebugString();

    IMS_TRACE_D("nVideoRtpDscp(%d), nVideoSendPeriodicSpsPps(%d)", nVideoRtpDscp,
            nVideoSendPeriodicSpsPps, 0);
    IMS_TRACE_D("bVideoAvpfTrrEnabled(%d), bVideoAvpfNackEnabled(%d)", bVideoAvpfTrrEnabled,
            bVideoAvpfNackEnabled, 0);
    IMS_TRACE_D("bVideoAvpfTmmbrEnabled(%d), bVideoAvpfPliEnabled(%d), bVideoAvpfFirEnabled(%d)",
            bVideoAvpfTmmbrEnabled, bVideoAvpfPliEnabled, bVideoAvpfFirEnabled);
    IMS_TRACE_D("nVideoAvpfTmmbrDownIntervalSec(%d), nVideoAvpfTmmbrUpIntervalSec(%d)",
            nVideoAvpfTmmbrDownIntervalSec, nVideoAvpfTmmbrUpIntervalSec, 0);
    IMS_TRACE_D("nVideoAvpfTmmbrLossThresholdRatio(%d), nVideoAvpfTmmbrMinBitrateKbps(%d)",
            nVideoAvpfTmmbrLossThresholdRatio, nVideoAvpfTmmbrMinBitrateKbps, 0);
    IMS_TRACE_D("nVideoAvpfTmmbrBitrateLevel(%d), nVideoAvpfTmmbrUpLevel(%d)",
            nVideoAvpfTmmbrBitrateLevel, nVideoAvpfTmmbrUpLevel, 0);
    IMS_TRACE_D("nVideoIframeIntervalSec(%d), bVideoDropPFrameEnabled(%d), nVideoSamplingRate(%d)",
            nVideoIframeIntervalSec, bVideoDropPFrameEnabled, nVideoSamplingRate);
    for (IMS_UINT32 i = 0; i < objCodecConfigs.GetSize(); ++i)
    {
        ToDebugStringCodecs(objCodecConfigs.GetAt(i));
    }
}

/*!
 * @brief   GetVideoRtpDscp
 * @details
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoRtpDscp() const
{
    return nVideoRtpDscp;
}

/*!
 * @brief   GetVideoSendPeriodicSpsPps
 * @details
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoSendPeriodicSpsPps() const
{
    return nVideoSendPeriodicSpsPps;
}

/*!
 * @brief   IsVideoAvpfTrrEnabled
 * @details Get Get avpf trr-int enable
 */
PUBLIC
IMS_BOOL VideoConfiguration::IsVideoAvpfTrrEnabled() const
{
    return bVideoAvpfTrrEnabled;
}

/*!
 * @brief   IsbVideoAvpfNackEnabled
 * @details Get avpf NACK attribute enabled
 */
PUBLIC
IMS_BOOL VideoConfiguration::IsbVideoAvpfNackEnabled() const
{
    return bVideoAvpfNackEnabled;
}

/*!
 * @brief   IsVideoAvpfTmmbrEnabled
 * @details Get avpf TMMBR attribute enabled
 */
PUBLIC
IMS_BOOL VideoConfiguration::IsVideoAvpfTmmbrEnabled() const
{
    return bVideoAvpfTmmbrEnabled;
}

/*!
 * @brief   IsVideoAvpfPliEnabled
 * @details Get avpf PLI attribute enabled
 */
PUBLIC
IMS_BOOL VideoConfiguration::IsVideoAvpfPliEnabled() const
{
    return bVideoAvpfPliEnabled;
}

/*!
 * @brief   IsVideoAvpfFirEnabled
 * @details Get avpf FIR attribute enabled
 */
PUBLIC
IMS_BOOL VideoConfiguration::IsVideoAvpfFirEnabled() const
{
    return bVideoAvpfFirEnabled;
}

/*!
 * @brief   GetAVPF_TMMBR_DownInterval
 * @details Get avpf TMMBR Time interval to determine downward
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoAvpfTmmbrDownIntervalSec() const
{
    return nVideoAvpfTmmbrDownIntervalSec;
}

/*!
 * @brief   GetAVPF_TMMBR_UpInterval
 * @details Get avpf TMMBR Time interval to determine upward
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoAvpfTmmbrUpIntervalSec() const
{
    return nVideoAvpfTmmbrUpIntervalSec;
}

/*!
 * @brief   GetAVPF_TMMBR_LossThreshold
 * @details Get avpf TMMBR Threshold of loss rate to cause TMMBR
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoAvpfTmmbrLossThresholdRatio() const
{
    return nVideoAvpfTmmbrLossThresholdRatio;
}

/*!
 * @brief   GetAVPF_TMMBR_MinBitrateRatio
 * @details Get avpf TMMBR Minimum threshold of bitrate
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoAvpfTmmbrMinBitrateKbps() const
{
    return nVideoAvpfTmmbrMinBitrateKbps;
}

/*!
 * @brief   GetAVPF_TMMBR_BitrateLevel
 * @details Get avpf TMMBR Level of bitrate change (-> n+1 step of bitrate in total)
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoAvpfTmmbrBitrateLevel() const
{
    return nVideoAvpfTmmbrBitrateLevel;
}

/*!
 * @brief   GetAVPF_TMMBR_MinBitrateRatio
 * @details Get avpf TMMBR Minimum threshold of bitrate
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoAvpfTmmbrUpLevel() const
{
    return nVideoAvpfTmmbrUpLevel;
}

/*!
 * @brief   GetVideoIframeIntervalSec
 * @details
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoIframeIntervalSec() const
{
    return nVideoIframeIntervalSec;
}

/*!
 * @brief   IsVideoDropPFrameEnabled
 * @details
 */
PUBLIC
IMS_BOOL VideoConfiguration::IsVideoDropPFrameEnabled() const
{
    return bVideoDropPFrameEnabled;
}

/*!
 * @brief   GetVideoIframeIntervalSec
 * @details
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoSamplingRate() const
{
    return nVideoSamplingRate;
}
