/*
 * Copyright (C) 2024 The Android Open Source Project
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
#include "config/CodecAudioConfig.h"

__IMS_TRACE_TAG_MEDIA__;

// Define static const members
const IMS_SINT32 CodecAudioConfig::DEFAULT_CHANNEL = 1;
const IMS_SINT32 CodecAudioConfig::DEFAULT_MODESET_AMR = 7;
const IMS_SINT32 CodecAudioConfig::DEFAULT_MODESET_AMR_WB = 8;
const IMS_SINT32 CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY = 2;
const IMS_SINT32 CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD = 1;
const IMS_SINT32 CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR = 0;
const IMS_SINT32 CodecAudioConfig::DEFAULT_MAXRED = 0;
const IMS_BOOL CodecAudioConfig::DEFAULT_DTX = IMS_TRUE;

PUBLIC
CodecAudioConfig::CodecAudioConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
        CodecConfig(nType, nPayloadTypeNum),
        m_bDtx(DEFAULT_DTX),
        m_bShowAmrModeSet(IMS_FALSE),
        m_nChannel(DEFAULT_CHANNEL),
        m_nAmrModeSetList(DEFAULT_MODESET_AMR_WB),
        m_nDefaultAmrModeSetList(DEFAULT_MODESET_AMR_WB),
        m_nModeChangeCapability(DEFAULT_MODECHANGE_CAPABILITY),
        m_nModeChangePeriod(DEFAULT_MODECHANGE_PERIOD),
        m_nModeChangeNeighbor(DEFAULT_MODECHANGE_NEIGHBOR)
{
    IMS_TRACE_I("+CodecAudioConfig - Type[%d]", nType, 0, 0);
}

PUBLIC VIRTUAL CodecAudioConfig::~CodecAudioConfig()
{
    IMS_TRACE_I("~CodecAudioConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecAudioConfig::Create(IN ICarrierConfig* piCc)
{
    IMS_TRACE_D("Create - Codec[%s]", ImsCodec::CodecToString(m_nCodec), 0, 0);

    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piCc is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    m_bShowAmrModeSet = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL, IMS_FALSE);

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecAudioConfig::ToDebugString() const
{
    CodecConfig::ToDebugString();
    IMS_TRACE_D("ChannelCount[%d], AmrModeSetList[0x%04x], default AmrModeSetList[0x%04x]",
            m_nChannel, m_nAmrModeSetList, m_nDefaultAmrModeSetList);
    IMS_TRACE_D("show AmrModeSet[%d]", m_bShowAmrModeSet, 0, 0);
    IMS_TRACE_D("ModeChangeCapability[%d], ModeChangePeriod[%d], ModeChangeNeighbor[%d]",
            m_nModeChangeCapability, m_nModeChangePeriod, m_nModeChangeNeighbor);
}

PUBLIC
IMS_SINT32 CodecAudioConfig::GetChannel() const
{
    return m_nChannel;
}

PUBLIC
IMS_SINT32 CodecAudioConfig::GetAmrModeSet() const
{
    IMS_SINT32 nModeSet;

    if (m_nAmrModeSetList == 0)
    {
        return (GetCodec() == ImsCodec::AUDIO_AMR) ? DEFAULT_MODESET_AMR : DEFAULT_MODESET_AMR_WB;
    }

    for (nModeSet = DEFAULT_MODESET_AMR_WB; nModeSet >= 0; nModeSet--)
    {
        if (m_nAmrModeSetList & (1 << nModeSet))
        {
            return nModeSet;
        }
    }
    return 0;
}

PUBLIC
IMS_BOOL CodecAudioConfig::GetShowAmrModeSet() const
{
    return m_bShowAmrModeSet;
}

PUBLIC
IMS_UINT32 CodecAudioConfig::GetAmrModeSetList() const
{
    return m_nAmrModeSetList;
}

PUBLIC
IMS_UINT32 CodecAudioConfig::GetDefaultAmrModeSetList() const
{
    return m_nDefaultAmrModeSetList;
}

PUBLIC
IMS_SINT32 CodecAudioConfig::GetModeChangeCapability() const
{
    return m_nModeChangeCapability;
}

PUBLIC
IMS_SINT32 CodecAudioConfig::GetModeChangePeriod() const
{
    return m_nModeChangePeriod;
}

PUBLIC
IMS_SINT32 CodecAudioConfig::GetModeChangeNeighbor() const
{
    return m_nModeChangeNeighbor;
}

PUBLIC
IMS_BOOL CodecAudioConfig::GetDtx() const
{
    return m_bDtx;
}
