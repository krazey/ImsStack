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

__IMS_TRACE_TAG_USER_DECL__("MED.CONF");

PUBLIC
VideoConfiguration::VideoConfiguration(IN MEDIA_CONTENT_TYPE _nSessionType) :
        MediaConfiguration(_nSessionType),
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
        m_nVideoSamplingRate(DEFAULT_VIDEO_SAMPLING_RATE)
{
    IMS_TRACE_D("+VideoConfiguration eSessionType(%d)", eSessionType, 0, 0);
    nAsBandwidthKbps = DEFAULT_AS;
    nRsBandwidthBps = DEFAULT_RR;
    nRrBandwidthBps = DEFAULT_RS;
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
    SetPorts(piCc, CarrierConfig::Assets::KEY_VIDEO_RTP_PORT_RANGE_INT_ARRAY);
    SetRtcpIntervals(piCc, CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INTERVAL_INT_ARRAY);

    nAsBandwidthKbps = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AS_BANDWIDTH_KBPS_INT);
    nRsBandwidthBps = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RS_BANDWIDTH_BPS_INT);
    nRrBandwidthBps = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RR_BANDWIDTH_BPS_INT);

    nRtpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_INACTIVITY_TIMER_MILLIS_INT);
    nRtcpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INACTIVITY_TIMER_MILLIS_INT);

    /** According to RFC 2474, six bits of the DS field are used as a codepoint (DSCP),
     * a two-bit currently unused (CU) field is reserved. So two left shift operations are required.
     */
    m_nVideoDscp = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_DSCP_INT);
    m_nVideoDscp = m_nVideoDscp << 2;

    m_nVideoSendPeriodicSpsPps =
            piCc->GetInt(CarrierConfig::Assets::KEY_VIDEO_SEND_PERIODIC_SPS_PPS_INT);
    m_nCvoId = piCc->GetInt(CarrierConfig::Assets::KEY_VIDEO_CVO_VALUE_INT);
    m_bVideoAvpfEnabled = piCc->GetBoolean(CarrierConfig::Assets::KEY_VIDEO_AVPF_ENABLE_BOOL);

    IMS_SINT32 nVideoAvpfFeature = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_FEATURE_INT);
    m_bVideoAvpfTrrEnabled = nVideoAvpfFeature & 0x01 ? IMS_TRUE : IMS_FALSE;
    m_bVideoAvpfNackEnabled = (nVideoAvpfFeature >> 1) & 0x01 ? IMS_TRUE : IMS_FALSE;
    m_bVideoAvpfTmmbrEnabled = (nVideoAvpfFeature >> 2) & 0x01 ? IMS_TRUE : IMS_FALSE;
    m_bVideoAvpfPliEnabled = (nVideoAvpfFeature >> 3) & 0x01 ? IMS_TRUE : IMS_FALSE;
    m_bVideoAvpfFirEnabled = (nVideoAvpfFeature >> 4) & 0x01 ? IMS_TRUE : IMS_FALSE;

    /** TODO_MEDIA need to check if it is needed for KR */
    m_nSdpOfferCapNegoForAvpf =
            piCc->GetInt(CarrierConfig::Assets::KEY_VIDEO_SDP_OFFER_CAP_NEGO_FOR_AVPF_INT);

    m_nVideoIframeIntervalSec =
            piCc->GetInt(CarrierConfig::Assets::KEY_VIDEO_IFRAME_INTERVAL_SEC_INT);

    // m_nChannel = DEFAULT_CHANNEL; // already set by default at creator
    // m_nVideoSamplingRate = DEFAULT_VIDEO_SAMPLING_RATE; // already set by default at creator
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
        IMS_TRACE_E(0, "Re-create VideoConfiguration failure", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL VideoConfiguration::CreateCodecConfigs(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCodecConfigs : piCc is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    /** TODO: to access bundle for Video codec - later */
    /* ICarrierConfig* piCcBundle =
            piCc->GetBundle(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

    if (piCcBundle == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMSVector<IMS_SINT32> objAvcPayloadType =
            piCcBundle->GetIntArray(CarrierConfig::ImsVt::KEY_H264_PAYLOAD_TYPE_INT_ARRAY); */

    // TODO: read carrier_config xml instead of Bundle due to comment out above
    IMSVector<IMS_SINT32> objAvcPayloadType =
            piCc->GetIntArray(CarrierConfig::ImsVt::KEY_H264_PAYLOAD_TYPE_INT_ARRAY);

    /** TODO: need to add after creating HEVC in CarrierConfig */
    /* IMSVector<IMS_SINT32> objHevcPayloadType = piCcBundle->GetIntArray(
            CarrierConfig::ImsVt::KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY); */

    IMS_UINT32 nCodecCnt = 0;
    if (objAvcPayloadType.GetSize() > 0)
    {
        nCodecCnt = MakeEachCodecs(piCc, ImsCodec::VIDEO_AVC, nCodecCnt, objAvcPayloadType);
    }

    /** TODO: need to add after creating HEVC in CarrierConfig */
    /* if (objHevcPayloadType.GetSize() > 0)
    {
        nCodecCnt = MakeEachCodecs(piCc, ImsCodec::VIDEO_HEVC, nCodecCnt, objHevcPayloadType);
    }*/

    /** TODO: to access bundle for Video codec - later */
    /* piCcBundle->ReleaseBundle();
    piCcBundle = IMS_NULL; */

    return IMS_TRUE;
}

PROTECTED VIRTUAL void VideoConfiguration::ToDebugString() const
{
    // MediaConfiguration::ToDebugString();

    IMS_TRACE_D("m_nVideoDscp(%d), m_nVideoSendPeriodicSpsPps(%d), m_nCvoId(%d)", m_nVideoDscp,
            m_nVideoSendPeriodicSpsPps, m_nCvoId);
    IMS_TRACE_D("m_bVideoAvpfEnabled(%d), m_bVideoAvpfTrrEnabled(%d), m_bVideoAvpfNackEnabled(%d)",
            m_bVideoAvpfEnabled, m_bVideoAvpfTrrEnabled, m_bVideoAvpfNackEnabled);
    IMS_TRACE_D(
            "m_bVideoAvpfTmmbrEnabled(%d), m_bVideoAvpfPliEnabled(%d), m_bVideoAvpfFirEnabled(%d)",
            m_bVideoAvpfTmmbrEnabled, m_bVideoAvpfPliEnabled, m_bVideoAvpfFirEnabled);
    IMS_TRACE_D("m_nVideoIframeIntervalSec(%d), m_nVideoSamplingRate(%d)",
            m_nVideoIframeIntervalSec, m_nVideoSamplingRate, 0);
    for (IMS_UINT32 i = 0; i < objCodecConfigs.GetSize(); ++i)
    {
        ToDebugStringCodecs(objCodecConfigs.GetAt(i));
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
