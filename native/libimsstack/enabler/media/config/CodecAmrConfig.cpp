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
CodecAmrConfig::CodecAmrConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
        CodecConfig(nType, nPayloadTypeNum),
        m_nChannel(DEFAULT_CHANNEL),
        m_bShowModeSet(IMS_FALSE),
        m_nModeSetList(DEFAULT_MODESET_AMR_WB),
        m_nDefaultModeSetList(DEFAULT_MODESET_AMR_WB),
        m_nOctetAlign(DEFAULT_OCTET_ALIGN),
        m_nSamplingRate(DEFAULT_SAMPLING_RATE_AMRWB),
        m_bDtx(DEFAULT_AMR_DTX)
{
    IMS_TRACE_D("+CodecAmrConfig Type[%d]", nType, 0, 0);
}

PUBLIC VIRTUAL CodecAmrConfig::~CodecAmrConfig()
{
    IMS_TRACE_D("~CodecAmrConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecAmrConfig::Create(IN ICarrierConfig* piCc, IN IMS_SINT32 nCodecIdx)
{
    IMS_TRACE_D("Create - nCodec[%d %s]", m_nCodec, ImsCodec::CodecToString(m_nCodec), 0);

    IMSVector<IMS_SINT32> objCodecAttributeModeset;
    IMSVector<IMS_SINT32> objCodecAttributeDefaultModeset;

    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piCc is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    m_bShowModeSet =
            piCc->GetBoolean(CarrierConfig::Assets::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL);

    /** TODO Media - Start - Need to change to carrier configuration bundle later */
    if (m_nCodec == ImsCodec::AUDIO_AMR)
    {
        m_nSamplingRate = DEFAULT_SAMPLING_RATE_AMR;
        IMSVector<IMS_SINT32> objOctetAlign = piCc->GetIntArray(
                CarrierConfig::Assets::KEY_ASSET_AMRNB_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT_ARRAY);

        IMS_TRACE_D("Create - Codec[%d] nCodecIdx[%d] octet size[%d]", m_nCodec, nCodecIdx,
                objOctetAlign.GetSize());
        if (objOctetAlign.GetSize() == 0)
        {
            m_nOctetAlign = 0;
        }
        else if (nCodecIdx < objOctetAlign.GetSize())
        {
            m_nOctetAlign = objOctetAlign.GetAt(nCodecIdx);
        }
        else
        {
            m_nOctetAlign = objOctetAlign.GetAt(objOctetAlign.GetAt(0));
        }
        IMS_TRACE_D("AMR OctetAlign: %d", m_nOctetAlign, 0, 0);

        objCodecAttributeModeset = piCc->GetIntArray(
                CarrierConfig::Assets::KEY_ASSET_AMR_AMRNB_CODEC_ATTRIBUTE_MODESET_INT_ARRAY);
        objCodecAttributeDefaultModeset = piCc->GetIntArray(
                CarrierConfig::Assets::KEY_AUDIO_AMRNB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY);
    }
    else if (m_nCodec == ImsCodec::AUDIO_AMR_WB)
    {
        m_nSamplingRate = DEFAULT_SAMPLING_RATE_AMRWB;
        IMSVector<IMS_SINT32> objOctetAlign = piCc->GetIntArray(
                CarrierConfig::Assets::KEY_ASSET_AMRWB_CODEC_ATTRIBUTE_PAYLOAD_FORMAT_INT_ARRAY);

        IMS_TRACE_D("Create - Codec[%d] nCodecIdx[%d] octet size[%d]", m_nCodec, nCodecIdx,
                objOctetAlign.GetSize());
        if (objOctetAlign.GetSize() == 0)
        {
            m_nOctetAlign = 0;
        }
        else if (nCodecIdx < objOctetAlign.GetSize())
        {
            m_nOctetAlign = objOctetAlign.GetAt(nCodecIdx);
        }
        else
        {
            m_nOctetAlign = objOctetAlign.GetAt(objOctetAlign.GetAt(0));
        }
        IMS_TRACE_D("AMRWB OctetAlign: %d", m_nOctetAlign, 0, 0);

        objCodecAttributeModeset = piCc->GetIntArray(
                CarrierConfig::Assets::KEY_ASSET_AMR_AMRWB_CODEC_ATTRIBUTE_MODESET_INT_ARRAY);
        objCodecAttributeDefaultModeset = piCc->GetIntArray(
                CarrierConfig::Assets::KEY_AUDIO_AMRWB_CODEC_ATTRIBUTE_DEFAULT_MODESET_INT_ARRAY);
    }

    m_nModeSetList = 0;
    IMS_SINT32 nModeSetNum = objCodecAttributeModeset.GetSize();
    IMS_TRACE_D("nModeSetNum: %d", nModeSetNum, 0, 0);

    for (IMS_SINT32 i = 0; i < nModeSetNum; i++)
    {
        IMS_SINT32 nModeSet = objCodecAttributeModeset.GetAt(i);
        if (nModeSet < 0)
        {
            IMS_TRACE_D("Invalid ModeSet value", 0, 0, 0);
            break;
        }
        m_nModeSetList = (m_nModeSetList | (1 << nModeSet));
    }
    /** TODO Media - End - Need to change to carrier configuration bundle later */

    m_nDefaultModeSetList = 0;
    IMS_SINT32 nDefaultModeSetNum = objCodecAttributeDefaultModeset.GetSize();

    for (IMS_SINT32 i = 0; i < nDefaultModeSetNum; i++)
    {
        IMS_SINT32 nModeSet = objCodecAttributeDefaultModeset.GetAt(i);
        if (nModeSet < 0)
        {
            IMS_TRACE_D("Invalid ModeSet value", 0, 0, 0);
            break;
        }
        m_nDefaultModeSetList = (m_nDefaultModeSetList | (1 << nModeSet));
    }
    IMS_TRACE_D("nDefaultModeSetNum: %d m_nDefaultModeSetList: %d", nDefaultModeSetNum,
            m_nDefaultModeSetList, 0);

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
    IMS_TRACE_D("default mode-set(0x%04x)", m_nDefaultModeSetList, 0, 0);
    IMS_TRACE_D("m_nSamplingRate(%d), m_bDtx(%d), m_bShowModeSet(%d)", m_nSamplingRate, m_bDtx,
            m_bShowModeSet);
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
IMS_BOOL CodecAmrConfig::GetShowModeSet() const
{
    return m_bShowModeSet;
}

PUBLIC
IMS_UINT32 CodecAmrConfig::GetModeSetList() const
{
    return m_nModeSetList;
}

PUBLIC
IMS_UINT32 CodecAmrConfig::GetDefaultModeSetList() const
{
    return m_nDefaultModeSetList;
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
