/*
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

#include "config/AudioConfiguration.h"

#include "ImsVector.h"
#include "ServiceTrace.h"
#include "config/ImsCodec.h"
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
        m_bAudioRtcpXrEnabled(DEFAULT_RTCPXR),
        m_bAudioRtcpXrStatisticsEnabled(DEFAULT_RTCPXR_STATISTICS),
        m_bAudioRtcpXrVoipMetricsEnabled(DEFAULT_RTCPXR_VOIP_METRICS),
        m_bAudioRtcpXrPacketLossRleEnabled(DEFAULT_RTCPXR_PACKET_LOSS_RLE),
        m_bAudioRtcpXrPacketDuplicateRleEnabled(DEFAULT_RTCPXR_PACKET_DUPLICATE_RLE),
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
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_D("Create - piCc for Audio is null", 0, 0, 0);
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
    m_bRecvOnlyEarlySessionEnabled = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_MEDIA_RECVONLY_EARLY_SESSION_BOOL, IMS_TRUE);

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
    m_bAudioRtcpXrEnabled =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_ENABLE_BOOL, DEFAULT_RTCPXR);
    m_bAudioRtcpXrStatisticsEnabled = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_STATISTICS_BOOL, DEFAULT_RTCPXR_STATISTICS);
    m_bAudioRtcpXrVoipMetricsEnabled =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_VOIP_METRICS_BOOL,
                    DEFAULT_RTCPXR_VOIP_METRICS);
    m_bAudioRtcpXrPacketLossRleEnabled =
            piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_RTCPXR_PACKET_LOSS_RLE_BOOL,
                    DEFAULT_RTCPXR_PACKET_LOSS_RLE);
    m_bAudioRtcpXrPacketDuplicateRleEnabled =
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
        IMS_TRACE_D("Update - piCc for Audio is null", 0, 0, 0);
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
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCodecConfigs - piCc is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    ICarrierConfig* piCcBundle = piCc->GetBundle(
            CarrierConfig::ImsVoice::KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

    if (piCcBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCodecConfigs - piCcBundle is NULL", 0, 0, 0);
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
    IMS_TRACE_D("SupportEvs[%d], RtcpXrEnabled[%d], RtcpXrStatisticsEnabled[%d]", m_bEvsSupported,
            m_bAudioRtcpXrEnabled, m_bAudioRtcpXrStatisticsEnabled);
    IMS_TRACE_D("RtcpXrVoipMetricsEnabled[%d], RtcpXrPacketLossRleEnabled[%d], \
            RtcpXrPacketDuplicateRleEnabled[%d]",
            m_bAudioRtcpXrVoipMetricsEnabled, m_bAudioRtcpXrPacketLossRleEnabled,
            m_bAudioRtcpXrPacketDuplicateRleEnabled);
    IMS_TRACE_D("RecvOnlyEarlySessionEnabled[%d]", m_bRecvOnlyEarlySessionEnabled, 0, 0);

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

PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrEnabled() const
{
    return m_bAudioRtcpXrEnabled;
}

PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrStatisticsEnabled() const
{
    return m_bAudioRtcpXrStatisticsEnabled;
}

PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrVoipEnabled() const
{
    return m_bAudioRtcpXrVoipMetricsEnabled;
}

PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrPlrEnabled() const
{
    return m_bAudioRtcpXrPacketLossRleEnabled;
}

PUBLIC
IMS_BOOL AudioConfiguration::IsRtcpXrPdrEnabled() const
{
    return m_bAudioRtcpXrPacketDuplicateRleEnabled;
}

PUBLIC
IMS_SINT32 AudioConfiguration::GetDtmfDuration() const
{
    return m_nDtmfDuration;
}

PUBLIC
const ImsVector<AString>& AudioConfiguration::GetAudioCandidateAttribute() const
{
    return m_objAudioCandidateAttribute;
}

PUBLIC
IMS_BOOL AudioConfiguration::IsAudioInactivityCallEndReason(IN IMS_SINT32 nReason) const
{
    for (IMS_UINT32 i = 0; i < m_objAudioInactivityCallEndReasons.GetSize(); i++)
    {
        if (m_objAudioInactivityCallEndReasons.GetAt(i) == nReason)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
