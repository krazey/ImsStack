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

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
CodecAmrConfig::CodecAmrConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
        CodecAudioConfig(nType, nPayloadTypeNum),
        m_nOctetAlign(DEFAULT_OCTET_ALIGN),
        m_nSamplingRate(DEFAULT_SAMPLING_RATE_AMRWB)
{
    IMS_TRACE_I("+CodecAmrConfig - Type[%d]", nType, 0, 0);
}

PUBLIC VIRTUAL CodecAmrConfig::~CodecAmrConfig()
{
    IMS_TRACE_I("~CodecAmrConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecAmrConfig::Create(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("Create - Codec[%s]", ImsCodec::CodecToString(m_nCodec), 0, 0);

    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piCc is NULL or invalid codec index", 0, 0, 0);
        return IMS_FALSE;
    }

    CodecAudioConfig::Create(piCc);

    ICarrierConfig* piCcBundle;

    if (m_nCodec == ImsCodec::AUDIO_AMR)
    {
        piCcBundle = piCc->GetBundle(CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE);
        m_nSamplingRate = DEFAULT_SAMPLING_RATE_AMR;
        m_nDefaultAmrModeSetList = (1 << DEFAULT_MODESET_AMR);
    }
    else  // (m_nCodec == ImsCodec::AUDIO_AMR_WB)
    {
        piCcBundle = piCc->GetBundle(CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE);
        m_nSamplingRate = DEFAULT_SAMPLING_RATE_AMRWB;
        m_nDefaultAmrModeSetList = (1 << DEFAULT_MODESET_AMR_WB);
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
        return IMS_FALSE;
    }

    m_nOctetAlign = piCcSubBundle->GetInt(
            CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
            DEFAULT_OCTET_ALIGN);

    ImsVector<IMS_SINT32> objCodecAttributeModeset = piCcSubBundle->GetIntArray(
            CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY);

    m_nAmrModeSetList = 0;
    IMS_SINT32 nModeSetNum = objCodecAttributeModeset.GetSize();
    IMS_TRACE_D("Create - AmrModeSetList size[%d]", nModeSetNum, 0, 0);

    for (IMS_SINT32 i = 0; i < nModeSetNum; i++)
    {
        IMS_SINT32 nModeSet = objCodecAttributeModeset.GetAt(i);
        if (nModeSet < 0)
        {
            IMS_TRACE_D("Create - Invalid ModeSet value", 0, 0, 0);
            break;
        }
        m_nAmrModeSetList = (m_nAmrModeSetList | (1 << nModeSet));
    }

    m_nModeChangeCapability = piCcSubBundle->GetInt(
            CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
            CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY);
    m_nModeChangePeriod = piCcSubBundle->GetInt(
            CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT,
            CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD);
    m_nModeChangeNeighbor = piCcSubBundle->GetInt(
            CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT,
            CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR);

    if (m_nCodec == ImsCodec::AUDIO_AMR && m_nSamplingRate == 16000)
    {
        m_nCodec = ImsCodec::AUDIO_AMR_WB;
        IMS_TRACE_D("Create - Invalid SamplingRate : Codec will be changed to AMR_WB", 0, 0, 0);
    }

    piCcSubBundle->ReleaseBundle();
    piCcBundle->ReleaseBundle();

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecAmrConfig::ToDebugString() const
{
    CodecAudioConfig::ToDebugString();
    IMS_TRACE_D("SamplingRate[%d], OctetAlign[%d]", m_nSamplingRate, m_nOctetAlign, 0);
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
