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

#include "ServiceTrace.h"
#include "ImsVector.h"
#include "config/CodecAmrConfig.h"

__IMS_TRACE_TAG_MEDIA__;

const IMS_SINT32 CodecAmrConfig::DEFAULT_PAYLOAD_FORMAT = CodecAmrConfig::BANDWIDTH_EFFICIENT;
const IMS_SINT32 CodecAmrConfig::DEFAULT_SAMPLING_RATE_AMR = 8000;
const IMS_SINT32 CodecAmrConfig::DEFAULT_SAMPLING_RATE_AMRWB = 16000;
const IMS_SINT32 CodecAmrConfig::NOT_DEFINED = -2;

PUBLIC
CodecAmrConfig::CodecAmrConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
        CodecAudioConfig(nType, nPayloadTypeNum),
        m_nOctetAlign(DEFAULT_PAYLOAD_FORMAT),
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
    ImsVector<IMS_SINT32> objCodecDefaultModeset;

    if (m_nCodec == ImsCodec::AUDIO_AMR)
    {
        piCcBundle = piCc->GetBundle(CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE);
        m_nSamplingRate = DEFAULT_SAMPLING_RATE_AMR;

        objCodecDefaultModeset = piCc->GetIntArray(
                CarrierConfig::ImsVoice::KEY_AUDIO_AMRNB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY);
    }
    else  // (m_nCodec == ImsCodec::AUDIO_AMR_WB)
    {
        piCcBundle = piCc->GetBundle(CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE);
        m_nSamplingRate = DEFAULT_SAMPLING_RATE_AMRWB;

        objCodecDefaultModeset = piCc->GetIntArray(
                CarrierConfig::ImsVoice::KEY_AUDIO_AMRWB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY);
    }

    if (objCodecDefaultModeset.IsEmpty())
    {
        SetDefaultModeSetList((m_nCodec == ImsCodec::AUDIO_AMR)
                        ? CodecAudioConfig::FULL_MODESET_AMRNB
                        : CodecAudioConfig::FULL_MODESET_AMRWB);
    }
    else
    {
        SetDefaultModeSetList(ConvertModeSetList(objCodecDefaultModeset));
    }

    if (piCcBundle == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - Codec Description is NULL", 0, 0, 0);
        CreateDefaultAmrCodec();
    }
    else
    {
        AString strPayloadTypeNumber;
        strPayloadTypeNumber.SetNumber(m_nPayloadType);
        ICarrierConfig* piCcSubBundle = piCcBundle->GetBundle(strPayloadTypeNumber.GetStr());

        IMS_TRACE_D("Create - current PayloadTypeNumber[%d]", m_nPayloadType, 0, 0);

        if (piCcSubBundle == IMS_NULL)
        {
            IMS_TRACE_D("Create - Codec SubBundle is NULL", 0, 0, 0);

            CreateDefaultAmrCodec();
        }
        else
        {
            SetDtx(piCc->GetBoolean(CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_DTX_BOOL,
                    CodecAudioConfig::DEFAULT_DTX));

            m_nOctetAlign = piCcSubBundle->GetInt(
                    CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT,
                    DEFAULT_PAYLOAD_FORMAT);

            ImsVector<IMS_SINT32> objCodecAttributeModeset = piCcSubBundle->GetIntArray(
                    CarrierConfig::ImsVoice::KEY_AMR_CODEC_ATTRIBUTE_MODESET_INT_ARRAY);
            IMS_SINT32 nModeSetList = 0;
            if (objCodecAttributeModeset.IsEmpty())
            {
                nModeSetList = (m_nCodec == ImsCodec::AUDIO_AMR)
                        ? CodecAudioConfig::FULL_MODESET_AMRNB
                        : CodecAudioConfig::FULL_MODESET_AMRWB;
            }
            else
            {
                nModeSetList = ConvertModeSetList(objCodecAttributeModeset);
            }

            SetModeSetList(nModeSetList);

            IMS_SINT32 nModeChangeCapability = piCcSubBundle->GetInt(
                    CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_CAPABILITY_INT,
                    NOT_DEFINED);
            if (nModeChangeCapability != NOT_DEFINED)
            {
                SetModeChangeCapability(nModeChangeCapability);
                SetVisibleModeChangeCapability(IMS_TRUE);

                IMS_BOOL bVisibleFlag = piCc->GetBoolean(
                        CarrierConfig::ImsVoice::
                                KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_CAPABILITY_BOOL,
                        IMS_TRUE);

                if (!bVisibleFlag)
                {
                    SetVisibleModeChangeCapability(IMS_FALSE);
                }
            }
            else
            {
                SetModeChangeCapability(CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY);
                SetVisibleModeChangeCapability(IMS_FALSE);
            }

            IMS_SINT32 nModeChangePeriod = piCcSubBundle->GetInt(
                    CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_PERIOD_INT,
                    NOT_DEFINED);
            if (nModeChangePeriod != NOT_DEFINED)
            {
                SetModeChangePeriod(nModeChangePeriod);
                SetVisibleModeChangePeriod(IMS_TRUE);

                IMS_BOOL bVisibleFlag = piCc->GetBoolean(
                        CarrierConfig::ImsVoice::
                                KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_PERIOD_BOOL,
                        IMS_TRUE);

                if (!bVisibleFlag)
                {
                    SetVisibleModeChangePeriod(IMS_FALSE);
                }
            }
            else
            {
                SetModeChangePeriod(CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD);
                SetVisibleModeChangePeriod(IMS_FALSE);
            }

            IMS_SINT32 nModeChangeNeighbor = piCcSubBundle->GetInt(
                    CarrierConfig::ImsVoice::KEY_CODEC_ATTRIBUTE_MODE_CHANGE_NEIGHBOR_INT,
                    NOT_DEFINED);
            if (nModeChangeNeighbor != NOT_DEFINED)
            {
                SetModeChangeNeighbor(nModeChangeNeighbor);
                SetVisibleModeChangeNeighbor(IMS_TRUE);

                IMS_BOOL bVisibleFlag = piCc->GetBoolean(
                        CarrierConfig::ImsVoice::
                                KEY_CODEC_ATTRIBUTE_VISIBLE_MODE_CHANGE_NEIGHBOR_BOOL,
                        IMS_TRUE);

                if (!bVisibleFlag)
                {
                    SetVisibleModeChangeNeighbor(IMS_FALSE);
                }
            }
            else
            {
                SetModeChangeNeighbor(CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR);
                SetVisibleModeChangeNeighbor(IMS_FALSE);
            }

            if (m_nCodec == ImsCodec::AUDIO_AMR && m_nSamplingRate == DEFAULT_SAMPLING_RATE_AMRWB)
            {
                m_nCodec = ImsCodec::AUDIO_AMR_WB;
                IMS_TRACE_D(
                        "Create - Update the codec type to AMR_WB due to the invalid SamplingRate",
                        0, 0, 0);
            }

            piCcSubBundle->ReleaseBundle();
        }

        piCcBundle->ReleaseBundle();
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecAmrConfig::CreateDefaultAmrCodec()
{
    IMS_TRACE_D(
            "CreateDefaultAmrCodec: codec[%d], payloadTypeNumber[%d]", m_nCodec, m_nPayloadType, 0);

    SetDtx(CodecAudioConfig::DEFAULT_DTX);
    m_nOctetAlign = DEFAULT_PAYLOAD_FORMAT;

    (m_nCodec == ImsCodec::AUDIO_AMR) ? SetModeSetList(CodecAudioConfig::FULL_MODESET_AMRNB)
                                      : SetModeSetList(CodecAudioConfig::FULL_MODESET_AMRWB);

    SetModeChangeCapability(CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY);
    SetVisibleModeChangeCapability(IMS_FALSE);

    SetModeChangePeriod(CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD);
    SetVisibleModeChangePeriod(IMS_FALSE);

    SetModeChangeNeighbor(CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR);
    SetVisibleModeChangeNeighbor(IMS_FALSE);
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
