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
#include "config/AudioConfiguration.h"

__IMS_TRACE_TAG_USER_DECL__("MED.CONF");

PUBLIC
AudioConfiguration::AudioConfiguration(MEDIA_CONTENT_TYPE eSessionType) :
        MediaConfiguration(eSessionType),
        m_nAudioPtime(DEFAULT_PTIME),
        m_nAudioMaxPtime(DEFAULT_MAX_PTIME),
        m_nAudioMaxRed(DEFAULT_MAX_RED),
        m_bAudioBwNegoOptionEnabled(DEFAULT_BW_NEGO_OPERION),
        m_nAudioRtpDscp(DEFAULT_AUDIO_DSCP),
        m_nJitterBufferMinSize(DEFAULT_JITTER_MIN),
        m_nJitterBufferMaxSize(DEFAULT_JITTER_MAX),
        m_nJitterBufferAdjustTime(DEFAULT_JITTER_ADJUST),
        m_nJitterBufferStepSize(DEFAULT_JITTER_STEP),
        m_bAudioRtcpxrEnabled(DEFAULT_RTCPXR),
        m_bAudioRtcpxrStatisticsEnabled(DEFAULT_RTCPXR_STATISTICS),
        m_bAudioRtcpxrVoipMetricsEnabled(DEFAULT_RTCPXR_VOIP_METRICS),
        m_bAudioRtcpxrPacketLossRleEnabled(DEFAULT_RTCPXR_PACKET_LOSS_RLE),
        m_bAudioRtcpxrPacketDuplicateRleEnabled(DEFAULT_RTCPXR_PACKET_DUPLICATE_RLE),
        m_nDtmfDuration(DEFAULT_DTMF_DURATION),
        m_nModeChangeCapability(DEFAULT_MODECHANGE_CAPABILITY),
        m_nModeChangePeriod(DEFAULT_MODECHANGE_PERIOD),
        m_nModeChangeNeighbor(DEFAULT_MODECHANGE_NEIGHBOR),
        m_objAudioCandidateAttribute(IMSVector<AString>())
{
    IMS_TRACE_D("+AudioConfiguration eSessionType(%d)", eSessionType, 0, 0);
    m_objAudioCandidateAttribute.Push(DEFAULT_CANDIDATE_ATTRIBUTE);
}

PUBLIC
AudioConfiguration::~AudioConfiguration()
{
    IMS_TRACE_I("~AudioConfiguration", 0, 0, 0);
    Clear();
}

PUBLIC VIRTUAL
IMS_BOOL AudioConfiguration::Create(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("Create", 0, 0, 0);

    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "piCc is null", 0, 0, 0);
        return IMS_FALSE;
    }

    // Media Configuration attributes
    SetPorts(piCc, CarrierConfig::ImsVoice::KEY_AUDIO_PORT_RTP_INT_ARRAY);
    SetRtcpIntervals(piCc, CarrierConfig::ImsVoice::KEY_AUDIO_RTCP_INTERVAL_INT_ARRAY);

    nAsBandwidthKbps = piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_AS_BANDWIDTH_KBPS_INT);
    nRsBandwidthBps = piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RS_BANDWIDTH_BPS_INT);
    nRrBandwidthBps = piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RR_BANDWIDTH_BPS_INT);

    nRtpInactivityTimerMillis = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_AUDIO_RTP_INACTIVITY_TIMER_MILLIS_INT);
    nRtcpInactivityTimerMillis = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_AUDIO_RTCP_INACTIVITY_TIMER_MILLIS_INT);

    // Audio Configuration attributes

    // m_nAudioPtime = DEFAULT_PTIME;    // already set by default at creator
    m_nAudioMaxPtime = piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_MAXPTIME_INT);

    m_nAudioMaxRed = m_nAudioMaxPtime - m_nAudioPtime;
    m_bAudioBwNegoOptionEnabled = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_AUDIO_BW_NEGO_OPTION_BOOL);
    // m_nAudioRtpDscp = DEFAULT_AUDIO_DSCP;      // already set by default at creator
    IMSVector<IMS_SINT32> objAudioJitterBufferSize =
            piCc->GetIntArray(CarrierConfig::ImsVoice::KEY_AUDIO_JITTER_BUFFER_SIZE_INT_ARRAY);
    if (!objAudioJitterBufferSize.IsEmpty())
    {
        m_nJitterBufferMinSize = objAudioJitterBufferSize.GetAt(0);
        if (objAudioJitterBufferSize.GetSize() > 1)
        {
            m_nJitterBufferMaxSize = objAudioJitterBufferSize.GetAt(1);
            if (objAudioJitterBufferSize.GetSize() > 2)
            {
                m_nJitterBufferAdjustTime = objAudioJitterBufferSize.GetAt(2);
                if (objAudioJitterBufferSize.GetSize() > 3)
                {
                    m_nJitterBufferStepSize = objAudioJitterBufferSize.GetAt(3);
                }
            }
        }
    }

    //rtcp-xr
    m_bAudioRtcpxrEnabled = piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_ENABLE_BOOL);
    m_bAudioRtcpxrStatisticsEnabled = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_STATISTICS_BOOL);
    m_bAudioRtcpxrVoipMetricsEnabled = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_VOIP_METRICS_BOOL);
    m_bAudioRtcpxrPacketLossRleEnabled = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_PACKET_LOSS_RLE_BOOL);
    m_bAudioRtcpxrPacketDuplicateRleEnabled = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_PACKET_DUPLICATE_RLE_BOOL);

    // DTMF Duration Parameter
    IMSVector<IMS_SINT32> objAudioTelephonyEventDurations =
            piCc->GetIntArray(CarrierConfig::Assets::KEY_AUDIO_TELEPHONE_EVENT_DURATION_INT_ARRAY);
    if (!objAudioTelephonyEventDurations.IsEmpty())
    {
        m_nDtmfDuration= objAudioTelephonyEventDurations.GetAt(0);
    }

    m_nModeChangeCapability = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT);
    m_nModeChangePeriod = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT);
    m_nModeChangeNeighbor = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT);

    // Creates a codec configuration
    if (!CreateCodecConfigs(piCc))
    {
        IMS_TRACE_E(0, "Create - CreateCodecConfigs failure", 0, 0, 0);
        return IMS_FALSE;
    }

    ToDebugString();
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL AudioConfiguration::Update(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    Clear();
    if (!Create(piCc))
    {
        IMS_TRACE_E(0, "Re-create AudioConfiguration failure", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL AudioConfiguration::CreateCodecConfigs(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("CreateCodecConfigs", 0, 0, 0);

    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "piCc is NULL", 0,0,0);
        return IMS_FALSE;
    }

    ICarrierConfig* piCcBundle = piCc->GetBundle(
            CarrierConfig::ImsVoice::KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

    if (piCcBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "piCcBundle is NULL", 0,0,0);
        return IMS_FALSE;
    }

    IMSVector<IMS_SINT32> objEvsPayloadType = piCcBundle->GetIntArray(
            CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_TYPE_INT_ARRAY);
    IMSVector<IMS_SINT32> objAmrwbPayloadType = piCcBundle->GetIntArray(
            CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY);
    IMSVector<IMS_SINT32> objAmrnbPayloadType = piCcBundle->GetIntArray(
            CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY);
    IMSVector<IMS_SINT32> objDtmfwbPayloadType = piCcBundle->GetIntArray(
            CarrierConfig::ImsVoice::KEY_DTMFWB_PAYLOAD_TYPE_INT_ARRAY);
    IMSVector<IMS_SINT32> objDtmfnbPayloadType = piCcBundle->GetIntArray(
            CarrierConfig::ImsVoice::KEY_DTMFNB_PAYLOAD_TYPE_INT_ARRAY);

    piCcBundle->ReleaseBundle();
    piCcBundle = IMS_NULL;

    IMS_UINT32 nCodecCnt = 0;
    AString strTemp;

    if (objEvsPayloadType.GetSize() > 0)
    {
        // nCodecCnt = MakeEachCodecs(piCc, ImsCodec::AUDIO_EVS, nCodecCnt, objEvsPayloadType);
        // TODO_MEDIA evs is not supported for now
    }
    if (objAmrwbPayloadType.GetSize() > 0)
    {
        nCodecCnt = MakeEachCodecs(piCc, ImsCodec::AUDIO_AMR_WB, nCodecCnt, objAmrwbPayloadType);
    }
    if (objAmrnbPayloadType.GetSize() > 0)
    {
        nCodecCnt = MakeEachCodecs(piCc, ImsCodec::AUDIO_AMR, nCodecCnt, objAmrnbPayloadType);
    }
    if (objDtmfwbPayloadType.GetSize() > 0)
    {
        nCodecCnt = MakeEachCodecs(piCc, ImsCodec::AUDIO_TELEPHONE_EVENT_WB, nCodecCnt,
                objDtmfwbPayloadType);
    }
    if (objDtmfnbPayloadType.GetSize() > 0)
    {
        nCodecCnt = MakeEachCodecs(piCc, ImsCodec::AUDIO_TELEPHONE_EVENT, nCodecCnt,
                objDtmfnbPayloadType);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
void AudioConfiguration::ToDebugString() const
{
    MediaConfiguration::ToDebugString();

    IMS_TRACE_D("m_nAudioPtime[%d], m_nAudioMaxPtime[%d], m_nAudioMaxRed[%d]",
            m_nAudioPtime, m_nAudioMaxPtime, m_nAudioMaxRed);
    IMS_TRACE_D("m_bAudioBwNegoOptionEnabled[%d], m_nAudioRtpDscp[%d], DTMFDuration[%d]",
            m_bAudioBwNegoOptionEnabled, m_nAudioRtpDscp, m_nDtmfDuration);
    IMS_TRACE_D("jitter_buffer_min[%d], jitter_buffer_max[%d]",
            m_nJitterBufferMinSize, m_nJitterBufferMaxSize, 0);
    IMS_TRACE_D("jitter_buffer_adjust_time[%d], jitter_buffer_step_size[%d]",
            m_nJitterBufferAdjustTime, m_nJitterBufferStepSize, 0);
    IMS_TRACE_D("m_bAudioRtcpxrEnabled[%d], m_bAudioRtcpxrStatisticsEnabled[%d]",
            m_bAudioRtcpxrEnabled, m_bAudioRtcpxrStatisticsEnabled, 0);
    IMS_TRACE_D("m_bAudioRtcpxrVoipMetricsEnabled[%d], m_bAudioRtcpxrPacketLossRleEnabled[%d], \
            m_bAudioRtcpxrPacketDuplicateRleEnabled[%d]",
            m_bAudioRtcpxrVoipMetricsEnabled, m_bAudioRtcpxrPacketLossRleEnabled,
            m_bAudioRtcpxrPacketDuplicateRleEnabled);
    IMS_TRACE_D("m_nModeChangeCapability(%d), m_nModeChangePeriod(%d), m_nModeChangeNeighbor(%d)",
            m_nModeChangeCapability, m_nModeChangePeriod, m_nModeChangeNeighbor);

    for (IMS_UINT32 i = 0; i < m_objAudioCandidateAttribute.GetSize(); ++i)
    {
        IMS_TRACE_D("m_objAudioCandidateAttribute[%d] : [%s]",
            i, m_objAudioCandidateAttribute.GetAt(i).GetStr(), 0);
    }
    for (IMS_UINT32 i = 0; i < objCodecConfigs.GetSize(); ++i)
    {
        ToDebugStringCodecs(objCodecConfigs.GetAt(i));
    }
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetPtime() const
{
    return m_nAudioPtime;
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetMaxPtime() const
{
    return m_nAudioMaxPtime;
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetMaxRed() const
{
    return m_nAudioMaxRed;
}

PUBLIC
IMS_BOOL AudioConfiguration::GetBandwidthNegoOption() const
{
    return m_bAudioBwNegoOptionEnabled;
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetRtpDscp() const
{
    return ((m_nAudioRtpDscp >= 0) ? m_nAudioRtpDscp : 0);
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetJitterBufferMinSize() const
{
    return m_nJitterBufferMinSize;
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetJitterBufferMaxSize() const
{
    return m_nJitterBufferMaxSize;
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetJitterBufferAdjustTime() const
{
    return m_nJitterBufferAdjustTime;
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetJitterBufferStepSize() const
{
    return m_nJitterBufferStepSize;
}

/*!
 * @brief   GetRtcpXr
 * @details Get RTCP-XR enable
 */
PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrEnabled() const
{
    return m_bAudioRtcpxrEnabled;
}

/*!
 * @brief   GetRtcpXrStatistics
 * @details Get RTCP-XR statistic metrics enable
 */
PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrStatisticsEnabled() const
{
    return m_bAudioRtcpxrStatisticsEnabled;
}

/*!
 * @brief   GetRtcpXrVoip
 * @details Get RTCP-XR Voip metrics enable
 */
PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrVoipEnabled() const
{
    return m_bAudioRtcpxrVoipMetricsEnabled;
}

/*!
 * @brief   GetRtcpXrPlr
 * @details Get RTCP-XR packet loss rle enable
 */
PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrPlrEnabled() const
{
    return m_bAudioRtcpxrPacketLossRleEnabled;
}

/*!
 * @brief   GetRtcpXrPdr
 * @details Get RTCP-XR packet duplicate rle enable
 */
PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrPdrEnabled() const
{
    return m_bAudioRtcpxrPacketDuplicateRleEnabled;
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetDTMFDuration() const
{
    return m_nDtmfDuration;
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetModeChangeCapability() const
{
    return m_nModeChangeCapability;
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetModeChangePeriod() const
{
    return m_nModeChangePeriod;
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetModeChangeNeighbor() const
{
    return m_nModeChangeNeighbor;
}

PUBLIC
const IMSVector<AString>& AudioConfiguration::GetAudioCandidateAttribute() const
{
    return m_objAudioCandidateAttribute;
}
