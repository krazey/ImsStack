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
#include "ImsVector.h"
#include "config/ImsCodec.h"
#include "config/VideoConfiguration.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
VideoConfiguration::VideoConfiguration(IN MEDIA_CONTENT_TYPE eSessionType) :
        MediaConfiguration(eSessionType),
        m_nVideoDscp(DEFAULT_VIDEO_DSCP),
        m_nVideoSendPeriodicSpsPps(DEFAULT_SEND_PERIODIC_SPS_PPS),
        m_nCvoId(DEFAULT_CVO_ID),
        m_bVideoAvpfEnabled(DEFAULT_AVPF_ENABLED),
        m_bVideoAvpfTrrEnabled(DEFAULT_AVPF_TRR),
        m_bVideoAvpfNackEnabled(DEFAULT_AVPF_NACK),
        m_bVideoAvpfTmmbrEnabled(DEFAULT_AVPF_TMMBR),
        m_bVideoAvpfPliEnabled(DEFAULT_AVPF_PLI),
        m_bVideoAvpfFirEnabled(DEFAULT_AVPF_FIR),
        m_nSdpOfferCapNegoForAvpf(DEFAULT_AVPF_CAPA_NEGO),
        m_nVideoIframeIntervalSec(DEFAULT_I_FRAME_INTERVAL),
        m_nChannel(DEFAULT_CHANNEL),
        m_nVideoSamplingRate(DEFAULT_VIDEO_SAMPLING_RATE),
        m_bVideoBwNegoOptionEnabled(DEFAULT_BW_NEGO_OPTION),
        m_nVideoLowestBitrateBps(DEFAULT_VIDEO_LOWEST_BITRATE)
{
    IMS_TRACE_I("+VideoConfiguration - SessionType[%d]", eSessionType, 0, 0);
}

PUBLIC VIRTUAL VideoConfiguration::~VideoConfiguration()
{
    IMS_TRACE_I("~VideoConfiguration", 0, 0, 0);
    Clear();
}

PUBLIC VIRTUAL IMS_BOOL VideoConfiguration::Create(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("Create", 0, 0, 0);

    if (piCc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Media Configuration attributes
    SetPorts(piCc, CarrierConfig::ImsVt::KEY_VIDEO_RTP_PORT_RANGE_INT_ARRAY);
    SetRtcpIntervals(piCc, CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INTERVAL_INT_ARRAY);

    m_nAsBandwidthKbps =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AS_BANDWIDTH_KBPS_INT, DEFAULT_AS_VIDEO);
    m_nRsBandwidthBps =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RS_BANDWIDTH_BPS_INT, DEFAULT_RS_VIDEO);
    m_nRrBandwidthBps =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RR_BANDWIDTH_BPS_INT, DEFAULT_RR_VIDEO);

    m_nRtpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTP_INACTIVITY);
    m_nRtcpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTCP_INACTIVITY);

    /** According to RFC 2474, six bits of the DS field are used as a codepoint (DSCP),
     * a two-bit currently unused (CU) field is reserved. So two left shift operations are required.
     */
    m_nVideoDscp = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_DSCP_INT, DEFAULT_VIDEO_DSCP);

    m_nVideoSendPeriodicSpsPps =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_SEND_PERIODIC_SPS_PPS_INT,
                    DEFAULT_SEND_PERIODIC_SPS_PPS);
    m_nCvoId = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_CVO_VALUE_INT, DEFAULT_CVO_ID);
    m_bVideoAvpfEnabled = piCc->GetBoolean(
            CarrierConfig::ImsVt::KEY_VIDEO_AVPF_ENABLE_BOOL, DEFAULT_AVPF_ENABLED);

    IMS_SINT32 nVideoAvpfFeature =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_FEATURE_INT, DEFAULT_AVPF_FEATURE);
    m_bVideoAvpfTrrEnabled = nVideoAvpfFeature & 0x01 ? IMS_TRUE : IMS_FALSE;
    m_bVideoAvpfNackEnabled = (nVideoAvpfFeature >> 1) & 0x01 ? IMS_TRUE : IMS_FALSE;
    m_bVideoAvpfTmmbrEnabled = (nVideoAvpfFeature >> 2) & 0x01 ? IMS_TRUE : IMS_FALSE;
    m_bVideoAvpfPliEnabled = (nVideoAvpfFeature >> 3) & 0x01 ? IMS_TRUE : IMS_FALSE;
    m_bVideoAvpfFirEnabled = (nVideoAvpfFeature >> 4) & 0x01 ? IMS_TRUE : IMS_FALSE;

    /** TODO_MEDIA need to check if it is needed for KR */
    m_nSdpOfferCapNegoForAvpf =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_SDP_OFFER_CAP_NEGO_FOR_AVPF_INT,
                    DEFAULT_AVPF_CAPA_NEGO);

    m_nVideoIframeIntervalSec = piCc->GetInt(
            CarrierConfig::ImsVt::KEY_VIDEO_IFRAME_INTERVAL_SEC_INT, DEFAULT_I_FRAME_INTERVAL);
    m_bVideoBwNegoOptionEnabled = piCc->GetBoolean(
            CarrierConfig::ImsVt::KEY_VIDEO_BW_NEGO_OPTION_BOOL, DEFAULT_BW_NEGO_OPTION);
    // m_nChannel = DEFAULT_CHANNEL; // already set by default at creator
    // m_nVideoSamplingRate = DEFAULT_VIDEO_SAMPLING_RATE; // already set by default at creator
    m_nVideoLowestBitrateBps = piCc->GetInt(
            CarrierConfig::ImsVt::KEY_VIDEO_LOWEST_BITRATE_BPS_INT, DEFAULT_VIDEO_LOWEST_BITRATE);
    if (!CreateCodecConfigs(piCc))
    {
        IMS_TRACE_E(0, "Create - CreateCodecConfigs failure ", 0, 0, 0);
        return IMS_FALSE;
    }

    ToDebugString();

    return IMS_TRUE;
}

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
        IMS_TRACE_E(0, "Update - Re-create VideoConfiguration failure", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL VideoConfiguration::CreateCodecConfigs(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCodecConfigs - piCc is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    ICarrierConfig* piCcBundle =
            piCc->GetBundle(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

    if (piCcBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCodecConfigs - piCcBundle is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    ImsVector<IMS_SINT32> objAvcPayloadType =
            piCcBundle->GetIntArray(CarrierConfig::ImsVt::KEY_H264_PAYLOAD_TYPE_INT_ARRAY);

    ImsVector<IMS_SINT32> objHevcPayloadType =
            piCcBundle->GetIntArray(CarrierConfig::ImsVt::KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY);

    IMS_BOOL bHevcPriorityOrderEnabled =
            piCc->GetBoolean(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_HEVC_PRIORITY_ORDER_BOOL, false);

    piCcBundle->ReleaseBundle();

    IMS_UINT32 nCodecCnt = 0;

    if (bHevcPriorityOrderEnabled)
    {
        if (objHevcPayloadType.GetSize() > 0)
        {
            nCodecCnt = MakeEachCodecs(piCc, ImsCodec::VIDEO_HEVC, nCodecCnt, objHevcPayloadType);
        }
        if (objAvcPayloadType.GetSize() > 0)
        {
            nCodecCnt = MakeEachCodecs(piCc, ImsCodec::VIDEO_AVC, nCodecCnt, objAvcPayloadType);
        }
    }
    else
    {
        if (objAvcPayloadType.GetSize() > 0)
        {
            nCodecCnt = MakeEachCodecs(piCc, ImsCodec::VIDEO_AVC, nCodecCnt, objAvcPayloadType);
        }
        if (objHevcPayloadType.GetSize() > 0)
        {
            nCodecCnt = MakeEachCodecs(piCc, ImsCodec::VIDEO_HEVC, nCodecCnt, objHevcPayloadType);
        }
    }

    // to avoid static analysis issue (not used variable and variable scope)
    IMS_TRACE_D("CreateCodecConfigs - NumOfCodec[%d]", nCodecCnt, 0, 0);

    return IMS_TRUE;
}

PROTECTED VIRTUAL void VideoConfiguration::ToDebugString() const
{
    IMS_TRACE_D("Dscp[%d], SendPeriodicSpsPps[%d], CvoId[%d]", m_nVideoDscp,
            m_nVideoSendPeriodicSpsPps, m_nCvoId);
    IMS_TRACE_D("AvpfEnabled[%d], AvpfTrrEnabled[%d], AvpfNackEnabled[%d]", m_bVideoAvpfEnabled,
            m_bVideoAvpfTrrEnabled, m_bVideoAvpfNackEnabled);
    IMS_TRACE_D("AvpfTmmbrEnabled[%d], AvpfPliEnabled[%d], AvpfFirEnabled[%d]",
            m_bVideoAvpfTmmbrEnabled, m_bVideoAvpfPliEnabled, m_bVideoAvpfFirEnabled);
    IMS_TRACE_D("IframeIntervalSec[%d], SamplingRate[%d], "
                "BandwidthNegoOptionEnabled[%d]",
            m_nVideoIframeIntervalSec, m_nVideoSamplingRate, m_bVideoBwNegoOptionEnabled);
    IMS_TRACE_D("LowestBitrate[%d]", m_nVideoLowestBitrateBps, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objCodecConfigs.GetSize(); ++i)
    {
        ToDebugStringCodecs(m_objCodecConfigs.GetAt(i));
    }
}

PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoDscp() const
{
    return m_nVideoDscp;
}

PUBLIC
IMS_SINT32 VideoConfiguration::GetCvoId() const
{
    return m_nCvoId;
}

PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoSendPeriodicSpsPps() const
{
    return m_nVideoSendPeriodicSpsPps;
}

PUBLIC
IMS_BOOL VideoConfiguration::IsVideoAvpfEnabled() const
{
    return m_bVideoAvpfEnabled;
}

PUBLIC
IMS_BOOL VideoConfiguration::IsVideoAvpfTrrEnabled() const
{
    return m_bVideoAvpfTrrEnabled;
}

PUBLIC
IMS_BOOL VideoConfiguration::IsVideoAvpfNackEnabled() const
{
    return m_bVideoAvpfNackEnabled;
}

PUBLIC
IMS_BOOL VideoConfiguration::IsVideoAvpfTmmbrEnabled() const
{
    return m_bVideoAvpfTmmbrEnabled;
}

PUBLIC
IMS_BOOL VideoConfiguration::IsVideoAvpfPliEnabled() const
{
    return m_bVideoAvpfPliEnabled;
}

PUBLIC
IMS_BOOL VideoConfiguration::IsVideoAvpfFirEnabled() const
{
    return m_bVideoAvpfFirEnabled;
}

PUBLIC
IMS_BOOL VideoConfiguration::IsAvpfCapabilityNegotiationEnabled() const
{
    return (m_nSdpOfferCapNegoForAvpf > 0);
}

PUBLIC
IMS_SINT32 VideoConfiguration::GetSdpOfferCapNegoForAvpf() const
{
    return m_nSdpOfferCapNegoForAvpf;
}

PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoIframeIntervalSec() const
{
    return m_nVideoIframeIntervalSec;
}

PUBLIC
IMS_SINT32 VideoConfiguration::GetChannel() const
{
    return m_nChannel;
}

PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoSamplingRate() const
{
    return m_nVideoSamplingRate;
}

PUBLIC
IMS_BOOL VideoConfiguration::GetBandwidthNegoOption() const
{
    return m_bVideoBwNegoOptionEnabled;
}

PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoLowestBitrateBps() const
{
    return m_nVideoLowestBitrateBps;
}
