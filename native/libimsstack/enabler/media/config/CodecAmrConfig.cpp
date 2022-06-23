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
#include "config/CodecAmrConfig.h"

__IMS_TRACE_TAG_USER_DECL__("MED.CONF");

PUBLIC
CodecAmrConfig::CodecAmrConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum) :
        CodecConfig(nType_, nPayloadTypeNum),
        m_nChannel(DEFAULT_CHANNEL),
        m_nModeSetList(DEFAULT_MODESET_AMR_WB),
        m_nOctetAlign(DEFAULT_OCTET_ALIGN),
        m_nSamplingRate(DEFAULT_SAMPLING_RATE_AMRWB),
        m_bDtx(DEFAULT_AMR_DTX)
{
    IMS_TRACE_D("+CodecAmrConfig Type[%d]", nType_, 0, 0);
}

PUBLIC VIRTUAL CodecAmrConfig::~CodecAmrConfig()
{
    IMS_TRACE_D("~CodecAmrConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecAmrConfig::Create(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("Create - nCodec[%d %s]", m_nCodec, ImsCodec::CodecToString(m_nCodec), 0);

    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piCc is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    // m_nChannel = DEFAULT_CHANNEL;    // already set by default at creator

    ICarrierConfig* piCcBundle = IMS_NULL;
    if (m_nCodec == ImsCodec::AUDIO_AMR)
    {
        m_nSamplingRate = DEFAULT_SAMPLING_RATE_AMR;
        piCcBundle = piCc->GetBundle(CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE);
    }
    else if (m_nCodec == ImsCodec::AUDIO_AMR_WB)
    {
        m_nSamplingRate = DEFAULT_SAMPLING_RATE_AMRWB;
        piCcBundle = piCc->GetBundle(CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE);
    }

    if (piCcBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piCcBundle is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strPayloadTypeNumber;
    strPayloadTypeNumber.SetNumber(m_nPayloadType);
    ICarrierConfig* piCcSubBundle = piCcBundle->GetBundle(strPayloadTypeNumber.GetStr());

    if (piCcSubBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piCcSubBundle is NULL", 0, 0, 0);
        piCcBundle->ReleaseBundle();
        piCcBundle = IMS_NULL;
        return IMS_FALSE;
    }

    m_nOctetAlign = piCcSubBundle->GetInt(
            CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT);

    IMSVector<IMS_SINT32> objCodecAttributeModeset = piCcSubBundle->GetIntArray(
            CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY);
    m_nModeSetList = 0;
    IMS_SINT32 nModeSetNum = objCodecAttributeModeset.GetSize();

    for (IMS_UINT32 i = 0; i < nModeSetNum; i++)
    {
        IMS_SINT32 nModeSet = objCodecAttributeModeset.GetAt(i);
        if (nModeSet < 0)
        {
            break;
        }
        m_nModeSetList = (m_nModeSetList | (1 << nModeSet));
    }

    // m_bDtx = DEFAULT_AMR_DTX;    // already set by default at creator

    piCcSubBundle->ReleaseBundle();
    piCcSubBundle = IMS_NULL;

    piCcBundle->ReleaseBundle();
    piCcBundle = IMS_NULL;

    if (m_nCodec == ImsCodec::AUDIO_AMR && m_nSamplingRate == 16000)
    {
        m_nCodec = ImsCodec::AUDIO_AMR_WB;
        IMS_TRACE_D("Create - Invalid SamplingRate : Codec will be changed to AMR_WB", 0, 0, 0);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecAmrConfig::ToDebugString() const
{
    CodecConfig::ToDebugString();
    IMS_TRACE_D("m_nChannel(%d), mode-set(0x%04x), m_nOctetAlign(%d)", m_nChannel, m_nModeSetList,
            m_nOctetAlign);
    IMS_TRACE_D("m_nSamplingRate(%d), m_bDtx(%d)", m_nSamplingRate, m_bDtx, 0);
}

PUBLIC
IMS_SINT32 CodecAmrConfig::GetChannel() const
{
    return m_nChannel;
}

PUBLIC
IMS_SINT32 CodecAmrConfig::GetModeSet() const
{
    IMS_SINT32 nModeSet;

    if (m_nModeSetList == 0)
    {
        if (GetCodec() == ImsCodec::AUDIO_AMR)
        {
            return DEFAULT_MODESET_AMR;
        }
        else if (GetCodec() == ImsCodec::AUDIO_AMR_WB)
        {
            return DEFAULT_MODESET_AMR_WB;
        }

        return (-1);
    }

    for (nModeSet = DEFAULT_MODESET_AMR_WB; nModeSet >= 0; nModeSet--)
    {
        if (m_nModeSetList & (1 << nModeSet))
        {
            return nModeSet;
        }
    }
    return 0;
}

PUBLIC
IMS_UINT32 CodecAmrConfig::GetModeSetList() const
{
    return m_nModeSetList;
}

PUBLIC
IMS_SINT32 CodecAmrConfig::GetOctetAlign() const
{
    return m_nOctetAlign;
}

PUBLIC
IMS_SINT32 CodecAmrConfig::GetSamplingRate() const
{
    return m_nSamplingRate;
}

PUBLIC
IMS_BOOL CodecAmrConfig::GetDtx() const
{
    return m_bDtx;
}
