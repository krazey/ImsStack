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
TextConfiguration::TextConfiguration(MEDIA_CONTENT_TYPE _eSessionType) :
        MediaConfiguration(_eSessionType),
        nT140PayloadType(DEFAULT_PAYLOAD_T140),
        nRedPayloadType(DEFAULT_PAYLOAD_RED),
        bTextCodecEmptyRedundantEnabled(DEFAULT_EMPTY_REDUNDANT)
{
    IMS_TRACE_I("+TextConfiguration sessiontype[%d]", _eSessionType, 0, 0);
    nAsBandwidthKbps = DEFAULT_AS;
    nRsBandwidthBps = DEFAULT_RR;
    nRrBandwidthBps = DEFAULT_RS;
}

PUBLIC
TextConfiguration::~TextConfiguration()
{
    IMS_TRACE_I("~TextConfiguration", 0, 0, 0);
    Clear();
}

/*

Remarks

*/
PUBLIC VIRTUAL
IMS_BOOL TextConfiguration::Create(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_D("Create piCc is null", 0, 0, 0);
        return IMS_FALSE;
    }

    // Media Configuration attributes
    SetPorts(piCc, CarrierConfig::ImsRtt::KEY_TEXT_PORT_RTP_INT_ARRAY);
    SetRtcpIntervals(piCc, CarrierConfig::ImsRtt::KEY_TEXT_RTCP_INTERVAL_INT_ARRAY);

    nAsBandwidthKbps = piCc->GetBoolean(CarrierConfig::ImsRtt::KEY_TEXT_AS_BANDWIDTH_KBPS_INT);
    nRrBandwidthBps = piCc->GetBoolean(CarrierConfig::ImsRtt::KEY_TEXT_RR_BANDWIDTH_BPS_INT);
    nRsBandwidthBps = piCc->GetBoolean(CarrierConfig::ImsRtt::KEY_TEXT_RS_BANDWIDTH_BPS_INT);

    nRtpInactivityTimerMillis = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_AUDIO_RTP_INACTIVITY_TIMER_MILLIS_INT); // same with audio
    nRtcpInactivityTimerMillis = piCc->GetInt(
            CarrierConfig::ImsVoice::KEY_AUDIO_RTCP_INACTIVITY_TIMER_MILLIS_INT); // same with audio

    // Text Configuration attributes
    // TODO_MEDIA need to add after creating HEVC in CarrierConfig
    // bTextCodecEmptyRedundantEnabled = piCc->GetBoolean(
    //     CarrierConfig::Assets::KEY_TEXT_CODEC_EMPTY_REDUNDANT_BOOL);

    if (!CreateCodecConfigs(piCc))
    {
        IMS_TRACE_E(0, "Create - CreateCodecConfigs failure ", 0, 0, 0);
        return IMS_FALSE;
    }

   return IMS_TRUE;
}

/*

Remarks

*/
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

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL TextConfiguration::CreateCodecConfigs(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateCodecConfigs - piCc is NULL", 0,0,0);
        return IMS_FALSE;
    }

    //MediaTextCodecCapabilityPayloadTypesBundle
    ICarrierConfig* piCcBundle = piCc->GetBundle(
        CarrierConfig::ImsRtt::KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

    if (piCcBundle == IMS_NULL)
    {
        return IMS_FALSE;
    }

    nT140PayloadType = piCcBundle->GetInt(CarrierConfig::ImsRtt::KEY_T140_PAYLOAD_TYPE_INT);
    nRedPayloadType = piCcBundle->GetInt(CarrierConfig::ImsRtt::KEY_RED_PAYLOAD_TYPE_INT);

    piCcBundle->ReleaseBundle();
    piCcBundle = IMS_NULL;

    IMS_UINT32 nCodecCnt = 0;
    if (nT140PayloadType > 0)
    {
        nCodecCnt = MakeCodec(piCc, ImsCodec::TEXT_T140, nCodecCnt, nT140PayloadType);
    }
    if (nRedPayloadType > 0)
    {
        nCodecCnt = MakeCodec(piCc, ImsCodec::TEXT_RED, nCodecCnt, nRedPayloadType);
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
void TextConfiguration::ToDebugString() const
{
    MediaConfiguration::ToDebugString();

    IMS_TRACE_D("nT140PayloadType[%d], nRedPayloadType[%d], bTextCodecEmptyRedundantEnabled(%d)",
            nT140PayloadType, nRedPayloadType, bTextCodecEmptyRedundantEnabled);

    for (IMS_UINT32 i = 0; i < objCodecConfigs.GetSize(); ++i)
    {
        ToDebugStringCodecs(objCodecConfigs.GetAt(i));
    }
}

PUBLIC
IMS_SINT32 TextConfiguration::GetT140PayloadType() const
{
    return nT140PayloadType;
}

PUBLIC
IMS_SINT32 TextConfiguration::GetRedPayloadType() const
{
    return nRedPayloadType;
}

PUBLIC
IMS_BOOL TextConfiguration::IsTextCodecEmptyRedundantEnabled() const
{
    return bTextCodecEmptyRedundantEnabled;
}
