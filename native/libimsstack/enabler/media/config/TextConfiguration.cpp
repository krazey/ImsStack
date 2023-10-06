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
#include "config/ImsCodec.h"
#include "config/TextConfiguration.h"

__IMS_TRACE_TAG_USER_DECL__("MED.CONF");

PUBLIC
TextConfiguration::TextConfiguration(MEDIA_CONTENT_TYPE eSessionType) :
        MediaConfiguration(eSessionType),
        m_nT140PayloadType(DEFAULT_PAYLOAD_T140),
        m_nRedPayloadType(DEFAULT_PAYLOAD_RED),
        m_nTextDscp(DEFAULT_TEXT_DSCP),
        m_bTextCodecEmptyRedundantEnabled(DEFAULT_EMPTY_REDUNDANT)
{
    IMS_TRACE_I("+TextConfiguration eSessionType[%d]", eSessionType, 0, 0);
}

PUBLIC
TextConfiguration::~TextConfiguration()
{
    IMS_TRACE_I("~TextConfiguration", 0, 0, 0);
    Clear();
}

PUBLIC VIRTUAL IMS_BOOL TextConfiguration::Create(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_D("Create piCc is null", 0, 0, 0);
        return IMS_FALSE;
    }

    // Media Configuration attributes
    SetPorts(piCc, CarrierConfig::Assets::KEY_TEXT_RTP_PORT_RANGE_INT_ARRAY);
    SetRtcpIntervals(piCc, CarrierConfig::ImsRtt::KEY_TEXT_RTCP_INTERVAL_INT_ARRAY);

    m_nAsBandwidthKbps =
            piCc->GetInt(CarrierConfig::ImsRtt::KEY_TEXT_AS_BANDWIDTH_KBPS_INT, DEFAULT_AS);
    m_nRrBandwidthBps =
            piCc->GetInt(CarrierConfig::ImsRtt::KEY_TEXT_RR_BANDWIDTH_BPS_INT, DEFAULT_RR);
    m_nRsBandwidthBps =
            piCc->GetInt(CarrierConfig::ImsRtt::KEY_TEXT_RS_BANDWIDTH_BPS_INT, DEFAULT_RS);

    m_nRtpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::Assets::KEY_TEXT_RTP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTP_INACTIVITY);
    m_nRtcpInactivityTimerMillis =
            piCc->GetInt(CarrierConfig::Assets::KEY_TEXT_RTCP_INACTIVITY_TIMER_MILLIS_INT,
                    DEFAULT_RTCP_INACTIVITY);

    IMS_TRACE_D("Create Text Configuration: rtpinactivity: %d rtcpinactivity: %d",
            m_nRtpInactivityTimerMillis, m_nRtcpInactivityTimerMillis, 0);

    /** According to RFC 2474, six bits of the DS field are used as a codepoint (DSCP),
     * a two-bit currently unused (CU) field is reserved. So two left shift operations are required.
     */
    m_nTextDscp = piCc->GetInt(CarrierConfig::Assets::KEY_TEXT_RTP_DSCP_INT, DEFAULT_TEXT_DSCP);

    m_bTextCodecEmptyRedundantEnabled = piCc->GetBoolean(
            CarrierConfig::Assets::KEY_TEXT_CODEC_EMPTY_REDUNDANT_BOOL, DEFAULT_EMPTY_REDUNDANT);

    if (!CreateCodecConfigs(piCc))
    {
        IMS_TRACE_E(0, "Create - CreateCodecConfigs failure ", 0, 0, 0);
        return IMS_FALSE;
    }

    ToDebugString();
    return IMS_TRUE;
}

PUBLIC
IMS_BOOL TextConfiguration::Update(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    Clear();
    if (!Create(piCc))
    {
        IMS_TRACE_E(0, "Re-create TextConfiguration failure", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL TextConfiguration::CreateCodecConfigs(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCodecConfigs - piCc is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    ICarrierConfig* piCcBundle =
            piCc->GetBundle(CarrierConfig::ImsRtt::KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

    if (piCcBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCodecConfigs - piCcBundle is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    m_nT140PayloadType = piCcBundle->GetInt(CarrierConfig::ImsRtt::KEY_T140_PAYLOAD_TYPE_INT);
    m_nRedPayloadType = piCcBundle->GetInt(CarrierConfig::ImsRtt::KEY_RED_PAYLOAD_TYPE_INT);

    piCcBundle->ReleaseBundle();

    IMS_TRACE_D("m_nT140PayloadType[%d], m_nRedPayloadType[%d]", m_nT140PayloadType,
            m_nRedPayloadType, 0);

    IMS_UINT32 nCodecCnt = 0;
    if (m_nRedPayloadType > 0)
    {
        nCodecCnt = MakeCodec(piCc, ImsCodec::TEXT_RED, nCodecCnt, m_nRedPayloadType, 0);
    }

    if (m_nT140PayloadType > 0)
    {
        nCodecCnt = MakeCodec(piCc, ImsCodec::TEXT_T140, nCodecCnt, m_nT140PayloadType, 0);
    }

    // to avoid static analysis issue (not used variable and variable scope)
    IMS_TRACE_D("nCodecCnt(%d)", nCodecCnt, 0, 0);

    return IMS_TRUE;
}

PROTECTED VIRTUAL void TextConfiguration::ToDebugString() const
{
    MediaConfiguration::ToDebugString();

    IMS_TRACE_D(
            "m_nT140PayloadType[%d], m_nRedPayloadType[%d], m_bTextCodecEmptyRedundantEnabled(%d)",
            m_nT140PayloadType, m_nRedPayloadType, m_bTextCodecEmptyRedundantEnabled);
    IMS_TRACE_D("m_nAsBandwidthKbps[%d], m_nRsBandwidthBps[%d], m_nRrBandwidthBps(%d)",
            m_nAsBandwidthKbps, m_nRsBandwidthBps, m_nRrBandwidthBps);
    IMS_TRACE_D(
            "m_nTextDscp[%d], m_nRtpInactivityTimerMillis[%d], m_nRtcpInactivityTimerMillis[%d]",
            m_nTextDscp, m_nRtpInactivityTimerMillis, m_nRtcpInactivityTimerMillis);

    for (IMS_UINT32 i = 0; i < m_objCodecConfigs.GetSize(); ++i)
    {
        ToDebugStringCodecs(m_objCodecConfigs.GetAt(i));
    }
}

PUBLIC
IMS_SINT32 TextConfiguration::GetT140PayloadType() const
{
    return m_nT140PayloadType;
}

PUBLIC
IMS_SINT32 TextConfiguration::GetRedPayloadType() const
{
    return m_nRedPayloadType;
}

PUBLIC
IMS_SINT32 TextConfiguration::GetTextDscp() const
{
    return m_nTextDscp;
}

PUBLIC
IMS_BOOL TextConfiguration::IsTextCodecEmptyRedundantEnabled() const
{
    return m_bTextCodecEmptyRedundantEnabled;
}
