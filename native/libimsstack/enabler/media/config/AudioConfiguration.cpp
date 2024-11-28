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
#include "config/AudioConfiguration.h"
#include "config/MediaSessionConfig.h"

__IMS_TRACE_TAG_MEDIA__;

#define DEFAULT_CANDIDATE_ATTRIBUTE "1, UDP, 1119400811, 10.3.210.77, 7010, typ, host"

PUBLIC
AudioConfiguration::AudioConfiguration(MEDIA_CONTENT_TYPE eSessionType) :
        MediaConfiguration(eSessionType),
        m_bEvsSupported(DEFAULT_SUPPORT_EVS),
        m_nAudioPtime(DEFAULT_PTIME),
        m_nAudioMaxPtime(DEFAULT_MAX_PTIME),
        m_nAudioMaxRed(DEFAULT_MAX_RED),
        m_bAudioBwNegoOptionEnabled(DEFAULT_BW_NEGO_OPTION),
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
        m_objAudioCandidateAttribute(ImsVector<AString>()),
        m_objAudioInactivityCallEndReasons(ImsVector<IMS_SINT32>())
{
    IMS_TRACE_I("+AudioConfiguration - SessionType[%d]", eSessionType, 0, 0);
    m_objAudioCandidateAttribute.Push(DEFAULT_CANDIDATE_ATTRIBUTE);
}

PUBLIC
AudioConfiguration::~AudioConfiguration()
{
    IMS_TRACE_I("~AudioConfiguration", 0, 0, 0);
    Clear();
}

PUBLIC VIRTUAL IMS_BOOL AudioConfiguration::Create(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("Create", 0, 0, 0);

    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "piCc is null", 0, 0, 0);
        return IMS_FALSE;
    }

    // Media Configuration attributes
    SetPorts(piCc, CarrierConfig::ImsVoice::KEY_AUDIO_RTP_PORT_RANGE_INT_ARRAY);
    SetRtcpIntervals(piCc, CarrierConfig::ImsVoice::KEY_AUDIO_RTCP_INTERVAL_INT_ARRAY);

    m_nAsBandwidthKbps =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_AS_BANDWIDTH_KBPS_INT, DEFAULT_AS);
    m_nRsBandwidthBps =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RS_BANDWIDTH_BPS_INT, DEFAULT_RS);
    m_nRrBandwidthBps =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RR_BANDWIDTH_BPS_INT, DEFAULT_RR);

    m_objAudioInactivityCallEndReasons = piCc->GetIntArray(
            CarrierConfig::ImsVoice::KEY_AUDIO_INACTIVITY_CALL_END_REASONS_INT_ARRAY);

    m_nRtpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RTP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTP_INACTIVITY);
    m_nRtcpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RTCP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTCP_INACTIVITY);

    m_bAudioBwNegoOptionEnabled = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_AUDIO_BW_NEGO_OPTION_BOOL, DEFAULT_BW_NEGO_OPTION);

    // Audio Configuration attributes
    m_bEvsSupported = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_AUDIO_EVS_SUPPORT_BOOL, DEFAULT_SUPPORT_EVS);
    m_nAudioPtime =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_PTIME_MILLIS_INT, DEFAULT_PTIME);
    m_nAudioMaxPtime =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_MAXPTIME_MILLIS_INT, DEFAULT_MAX_PTIME);
    m_nAudioMaxRed = piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_MAXRED_INT, DEFAULT_MAX_RED);

    /** According to RFC 2474, six bits of the DS field are used as a codepoint (DSCP),
     * a two-bit currently unused (CU) field is reserved. So two left shift operations are required.
     */
    m_nAudioRtpDscp =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_RTP_DSCP_INT, DEFAULT_AUDIO_DSCP);

    ImsVector<IMS_SINT32> objAudioJitterBufferSize =
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

    m_objAudioCandidateAttribute = piCc->GetStringArray(
            CarrierConfig::ImsVoice::KEY_AUDIO_CANDIDATE_ATTRIBUTE_STRING_ARRAY);

    // rtcp-xr
    m_bAudioRtcpxrEnabled =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_ENABLE_BOOL, DEFAULT_RTCPXR);
    m_bAudioRtcpxrStatisticsEnabled = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_STATISTICS_BOOL, DEFAULT_RTCPXR_STATISTICS);
    m_bAudioRtcpxrVoipMetricsEnabled =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_VOIP_METRICS_BOOL,
                    DEFAULT_RTCPXR_VOIP_METRICS);
    m_bAudioRtcpxrPacketLossRleEnabled =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_PACKET_LOSS_RLE_BOOL,
                    DEFAULT_RTCPXR_PACKET_LOSS_RLE);
    m_bAudioRtcpxrPacketDuplicateRleEnabled =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_PACKET_DUPLICATE_RLE_BOOL,
                    DEFAULT_RTCPXR_PACKET_DUPLICATE_RLE);

    // DTMF Duration Parameter
    m_nDtmfDuration =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_AUDIO_TELEPHONE_EVENT_DURATION_MILLIS_INT,
                    DEFAULT_DTMF_DURATION);

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

PROTECTED VIRTUAL IMS_BOOL AudioConfiguration::CreateCodecConfigs(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("CreateCodecConfigs", 0, 0, 0);

    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "piCc is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    ICarrierConfig* piCcBundle = piCc->GetBundle(
            CarrierConfig::ImsVoice::KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

    if (piCcBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "piCcBundle is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    ImsVector<IMS_SINT32> objEvsPayloadType =
            piCcBundle->GetIntArray(CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_TYPE_INT_ARRAY);
    ImsVector<IMS_SINT32> objAmrwbPayloadType =
            piCcBundle->GetIntArray(CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY);
    ImsVector<IMS_SINT32> objAmrnbPayloadType =
            piCcBundle->GetIntArray(CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY);
    ImsVector<IMS_SINT32> objDtmfwbPayloadType =
            piCcBundle->GetIntArray(CarrierConfig::ImsVoice::KEY_DTMFWB_PAYLOAD_TYPE_INT_ARRAY);
    ImsVector<IMS_SINT32> objDtmfnbPayloadType =
            piCcBundle->GetIntArray(CarrierConfig::ImsVoice::KEY_DTMFNB_PAYLOAD_TYPE_INT_ARRAY);

    piCcBundle->ReleaseBundle();

    IMS_UINT32 nCodecCnt = 0;

    if (m_bEvsSupported == IMS_TRUE && objEvsPayloadType.GetSize() > 0)
    {
        nCodecCnt = MakeEachCodecs(piCc, ImsCodec::AUDIO_EVS, nCodecCnt, objEvsPayloadType);
        IMS_TRACE_D("CreateCodecConfigs - Read Evs Codecs", 0, 0, 0);
        /**  TODO: evs is not supported for now */
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
        nCodecCnt = MakeEachCodecs(
                piCc, ImsCodec::AUDIO_TELEPHONE_EVENT_WB, nCodecCnt, objDtmfwbPayloadType);
    }
    if (objDtmfnbPayloadType.GetSize() > 0)
    {
        nCodecCnt = MakeEachCodecs(
                piCc, ImsCodec::AUDIO_TELEPHONE_EVENT, nCodecCnt, objDtmfnbPayloadType);
    }

    // to avoid static analysis issue (not used variable and variable scope)
    IMS_TRACE_D("CreateCodecConfigs - NumOfCodec[%d]", nCodecCnt, 0, 0);

    return IMS_TRUE;
}

PROTECTED VIRTUAL void AudioConfiguration::ToDebugString() const
{
    MediaConfiguration::ToDebugString();

    IMS_TRACE_D("Ptime[%d], MaxPtime[%d], MaxRedundancy[%d]", m_nAudioPtime, m_nAudioMaxPtime,
            m_nAudioMaxRed);
    IMS_TRACE_D("BandwidthNegoOption[%d], Dscp[%d], DtmfDuration[%d]", m_bAudioBwNegoOptionEnabled,
            m_nAudioRtpDscp, m_nDtmfDuration);
    IMS_TRACE_D("SupportEvs[%d], RtcpxrEnabled[%d], RtcpxrStatisticsEnabled[%d]", m_bEvsSupported,
            m_bAudioRtcpxrEnabled, m_bAudioRtcpxrStatisticsEnabled);
    IMS_TRACE_D("RtcpxrVoipMetricsEnabled[%d], RtcpxrPacketLossRleEnabled[%d], \
            RtcpxrPacketDuplicateRleEnabled[%d]",
            m_bAudioRtcpxrVoipMetricsEnabled, m_bAudioRtcpxrPacketLossRleEnabled,
            m_bAudioRtcpxrPacketDuplicateRleEnabled);

    for (IMS_UINT32 i = 0; i < m_objAudioCandidateAttribute.GetSize(); i++)
    {
        IMS_TRACE_D("AudioCandidateAttribute[%d] : [%s]", i,
                m_objAudioCandidateAttribute.GetAt(i).GetStr(), 0);
    }
    for (IMS_UINT32 i = 0; i < m_objAudioInactivityCallEndReasons.GetSize(); i++)
    {
        IMS_TRACE_D("AudioInactivityCallEndReasons[%d] : [%d]", i,
                m_objAudioInactivityCallEndReasons.GetAt(i), 0);
    }
    for (IMS_UINT32 i = 0; i < m_objCodecConfigs.GetSize(); ++i)
    {
        ToDebugStringCodecs(m_objCodecConfigs.GetAt(i));
    }
}

PUBLIC
IMS_BOOL AudioConfiguration::IsEvsSupported() const
{
    return m_bEvsSupported;
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

/**
 * @brief   GetRtcpXr
 * @details Get RTCP-XR enable
 */
PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrEnabled() const
{
    return m_bAudioRtcpxrEnabled;
}

/**
 * @brief   GetRtcpXrStatistics
 * @details Get RTCP-XR statistic metrics enable
 */
PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrStatisticsEnabled() const
{
    return m_bAudioRtcpxrStatisticsEnabled;
}

/**
 * @brief   GetRtcpXrVoip
 * @details Get RTCP-XR Voip metrics enable
 */
PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrVoipEnabled() const
{
    return m_bAudioRtcpxrVoipMetricsEnabled;
}

/**
 * @brief   GetRtcpXrPlr
 * @details Get RTCP-XR packet loss rle enable
 */
PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrPlrEnabled() const
{
    return m_bAudioRtcpxrPacketLossRleEnabled;
}

/**
 * @brief   GetRtcpXrPdr
 * @details Get RTCP-XR packet duplicate rle enable
 */
PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrPdrEnabled() const
{
    return m_bAudioRtcpxrPacketDuplicateRleEnabled;
}

/**
 * @brief   GetDtmfDuration
 * @details Get the dtmf duration value for playing
 */
PUBLIC
IMS_SINT32 AudioConfiguration::GetDtmfDuration() const
{
    return m_nDtmfDuration;
}

/**
 * @brief   GetAudioCandidateAttribute
 * @details Get the audioCandidateAttribute
 */
PUBLIC
const ImsVector<AString>& AudioConfiguration::GetAudioCandidateAttribute() const
{
    return m_objAudioCandidateAttribute;
}

PUBLIC
IMS_BOOL AudioConfiguration::IsAudioInactivityCallEndReason(IN IMS_SINT32 nReason) const
{
    IMS_TRACE_D("IsAudioInactivityCallEndReason()", 0, 0, 0);

    for (IMS_UINT32 i = 0; i < m_objAudioInactivityCallEndReasons.GetSize(); i++)
    {
        if (m_objAudioInactivityCallEndReasons.GetAt(i) == nReason)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
