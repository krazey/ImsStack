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

/*!
 * @brief   Creator
 * @details
 */
PUBLIC
VideoConfiguration::VideoConfiguration(IN MEDIA_CONTENT_TYPE _nSessionType) :
        MediaConfiguration(_nSessionType),
        nVideoDscp(DEFAULT_VIDEO_DSCP),
        nVideoSendPeriodicSpsPps(DEFAULT_SEND_PERIODIC_SPS_PPS),
        nCvoId(DEFAULT_CVO_ID),
        bVideoAvpfTrrEnabled(DEFAULT_AVPF_TRR),
        bVideoAvpfNackEnabled(DEFAULT_AVPF_NACK),
        bVideoAvpfTmmbrEnabled(DEFAULT_AVPF_TMMBR),
        bVideoAvpfPliEnabled(DEFAULT_AVPF_PLI),
        bVideoAvpfFirEnabled(DEFAULT_AVPF_FIR),
        nSdpOfferCapNegoForAvpf(DEFAULT_AVPF_CAPA_NEGO),
        nVideoIframeIntervalSec(DEFAULT_I_FRAME_INTERVAL),
        nChannel(DEFAULT_CHANNEL),
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
    SetPorts(piCc, CarrierConfig::Assets::KEY_VIDEO_RTP_PORT_RANGE_INT_ARRAY);
    SetRtcpIntervals(piCc, CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INTERVAL_INT_ARRAY);

    nAsBandwidthKbps = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AS_BANDWIDTH_KBPS_INT);
    nRsBandwidthBps = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RS_BANDWIDTH_BPS_INT);
    nRrBandwidthBps = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RR_BANDWIDTH_BPS_INT);

    nRtpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_INACTIVITY_TIMER_MILLIS_INT);
    nRtcpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTCP_INACTIVITY_TIMER_MILLIS_INT);

    // Video Configuration attributes
    nVideoDscp = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_RTP_DSCP_INT);
    nVideoSendPeriodicSpsPps =
            piCc->GetInt(CarrierConfig::Assets::KEY_VIDEO_SEND_PERIODIC_SPS_PPS_INT);
    nCvoId = piCc->GetInt(CarrierConfig::Assets::KEY_VIDEO_CVO_VALUE_INT);

    IMS_SINT32 nVideoAvpfFeature = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_AVPF_FEATURE_INT);
    bVideoAvpfTrrEnabled = nVideoAvpfFeature & 0x01 ? IMS_TRUE : IMS_FALSE;
    bVideoAvpfNackEnabled = (nVideoAvpfFeature >> 1) & 0x01 ? IMS_TRUE : IMS_FALSE;
    bVideoAvpfTmmbrEnabled = (nVideoAvpfFeature >> 2) & 0x01 ? IMS_TRUE : IMS_FALSE;
    bVideoAvpfPliEnabled = (nVideoAvpfFeature >> 3) & 0x01 ? IMS_TRUE : IMS_FALSE;
    bVideoAvpfFirEnabled = (nVideoAvpfFeature >> 4) & 0x01 ? IMS_TRUE : IMS_FALSE;

    // TODO_MEDIA need to check if it is needed for KR
    nSdpOfferCapNegoForAvpf =
            piCc->GetInt(CarrierConfig::Assets::KEY_VIDEO_SDP_OFFER_CAP_NEGO_FOR_AVPF_INT);

    nVideoIframeIntervalSec =
            piCc->GetInt(CarrierConfig::Assets::KEY_VIDEO_IFRAME_INTERVAL_SEC_INT);

    // nChannel = DEFAULT_CHANNEL; // already set by default at creator
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

    IMS_TRACE_D("nVideoDscp(%d), nVideoSendPeriodicSpsPps(%d), nCvoId(%d)", nVideoDscp,
            nVideoSendPeriodicSpsPps, nCvoId);
    IMS_TRACE_D("bVideoAvpfTrrEnabled(%d), bVideoAvpfNackEnabled(%d)", bVideoAvpfTrrEnabled,
            bVideoAvpfNackEnabled, 0);
    IMS_TRACE_D("bVideoAvpfTmmbrEnabled(%d), bVideoAvpfPliEnabled(%d), bVideoAvpfFirEnabled(%d)",
            bVideoAvpfTmmbrEnabled, bVideoAvpfPliEnabled, bVideoAvpfFirEnabled);
    IMS_TRACE_D("nVideoIframeIntervalSec(%d), nVideoSamplingRate(%d)", nVideoIframeIntervalSec,
            nVideoSamplingRate, 0);
    for (IMS_UINT32 i = 0; i < objCodecConfigs.GetSize(); ++i)
    {
        ToDebugStringCodecs(objCodecConfigs.GetAt(i));
    }
}

/*!
 * @brief   GetVideoDscp
 * @details
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetVideoDscp() const
{
    return nVideoDscp;
}

/*!
 * @brief   GetCvoId
 * @details
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetCvoId() const
{
    return nCvoId;
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
 * @brief   IsVideoAvpfEnabled
 * @details Get  avpf  enable
 */
PUBLIC
IMS_BOOL VideoConfiguration::IsVideoAvpfEnabled() const
{
    // TODO Media - return bVideoAvpfTrrEnabled || bVideoAvpfNackEnabled || bVideoAvpfTmmbrEnabled
    // ||
    //         bVideoAvpfPliEnabled || bVideoAvpfFirEnabled;

    if (nSdpOfferCapNegoForAvpf == MediaConfiguration::CAPNEG_OFFER_WITHOUT_ACAP ||
            nSdpOfferCapNegoForAvpf == MediaConfiguration::CAPNEG_OFFER_WITH_ACAP)
    {
        return true;
    }
    else
    {
        return false;
    }
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
 * @brief   GetSdpOfferCapNegoForAvpf
 * @details Get avpf sdp offer capa nego
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetSdpOfferCapNegoForAvpf() const
{
    return nSdpOfferCapNegoForAvpf;
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
 * @brief   GetChannel
 * @details
 */
PUBLIC
IMS_SINT32 VideoConfiguration::GetChannel() const
{
    return nChannel;
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
