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
const IMS_SINT32 CodecAudioConfig::DEFAULT_MODESET_AMRNB = 7;
const IMS_SINT32 CodecAudioConfig::DEFAULT_MODESET_AMRWB = 8;
const IMS_SINT32 CodecAudioConfig::FULL_MODESET_AMRNB = 255;  // 0xFF
const IMS_SINT32 CodecAudioConfig::FULL_MODESET_AMRWB = 511;  // 0x1FF
const IMS_SINT32 CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY = 1;
const IMS_SINT32 CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD = 1;
const IMS_SINT32 CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR = 0;
const IMS_SINT32 CodecAudioConfig::DEFAULT_MAXRED = 0;
const IMS_BOOL CodecAudioConfig::DEFAULT_DTX = IMS_TRUE;

PUBLIC
CodecAudioConfig::CodecAudioConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
        CodecConfig(nType, nPayloadTypeNum),
        m_bDtx(DEFAULT_DTX),
        m_bVisibleModeSet(IMS_FALSE),
        m_bVisibleModeChangeCapability(IMS_FALSE),
        m_bVisibleModeChangePeriod(IMS_FALSE),
        m_bVisibleModeChangeNeighbor(IMS_FALSE),
        m_nChannel(DEFAULT_CHANNEL),
        m_nModeSetList(DEFAULT_MODESET_AMRWB),
        m_nDefaultModeSetList(DEFAULT_MODESET_AMRWB),
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

    m_bVisibleModeSet = piCc->GetBoolean(
            CarrierConfig::ImsVoice::KEY_AUDIO_SHOW_CODEC_ATTRIBUTE_MODESET_BOOL, IMS_FALSE);

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecAudioConfig::ToDebugString() const
{
    CodecConfig::ToDebugString();
    IMS_TRACE_D("ChannelCount[%d], ModeSetList[0x%04x], Default ModeSetList[0x%04x]", m_nChannel,
            m_nModeSetList, m_nDefaultModeSetList);
    IMS_TRACE_D("Visible ModeSet[%d]", m_bVisibleModeSet, 0, 0);
    IMS_TRACE_D("ModeChangeCapability[%d], ModeChangePeriod[%d], ModeChangeNeighbor[%d]",
            m_nModeChangeCapability, m_nModeChangePeriod, m_nModeChangeNeighbor);
}

PUBLIC
IMS_SINT32 CodecAudioConfig::ConvertModeSetList(ImsVector<IMS_SINT32> objCodecModeset)
{
    IMS_SINT32 nModeSetList = 0;
    IMS_SINT32 nModeSetNum = objCodecModeset.GetSize();

    for (IMS_SINT32 i = 0; i < nModeSetNum; i++)
    {
        IMS_SINT32 nModeSet = objCodecModeset.GetAt(i);
        if (nModeSet < 0)
        {
            IMS_TRACE_D("ConvertModeSetList - Invalid ModeSet value", 0, 0, 0);
            break;
        }
        nModeSetList = (nModeSetList | (1 << nModeSet));
    }

    IMS_TRACE_D("ConvertModeSetList - ModeSetList size[%d] ListValue[%d]", nModeSetNum,
            nModeSetList, 0);

    return nModeSetList;
}

PUBLIC
void CodecAudioConfig::SetVisibleModeChangeCapability(IMS_BOOL bVisibleModeChangeCapability)
{
    m_bVisibleModeChangeCapability = bVisibleModeChangeCapability;
}

PUBLIC
void CodecAudioConfig::SetVisibleModeChangePeriod(IMS_BOOL bVisibleModeChangePeriod)
{
    m_bVisibleModeChangePeriod = bVisibleModeChangePeriod;
}

PUBLIC
void CodecAudioConfig::SetVisibleModeChangeNeighbor(IMS_BOOL bVisibleModeChangeNeighbor)
{
    m_bVisibleModeChangeNeighbor = bVisibleModeChangeNeighbor;
}

PUBLIC
void CodecAudioConfig::SetVisibleModeSet(IMS_BOOL bVisibleModeSet)
{
    m_bVisibleModeSet = bVisibleModeSet;
}

PUBLIC
void CodecAudioConfig::SetModeSetList(IMS_UINT32 nModeSetList)
{
    m_nModeSetList = nModeSetList;
}

PUBLIC
void CodecAudioConfig::SetModeChangeCapability(IMS_SINT32 nModeChangeCapability)
{
    m_nModeChangeCapability = nModeChangeCapability;
}

PUBLIC
void CodecAudioConfig::SetModeChangePeriod(IMS_SINT32 nModeChangePeriod)
{
    m_nModeChangePeriod = nModeChangePeriod;
}

PUBLIC
void CodecAudioConfig::SetModeChangeNeighbor(IMS_SINT32 nModeChangeNeighbor)
{
    m_nModeChangeNeighbor = nModeChangeNeighbor;
}

PUBLIC
void CodecAudioConfig::SetDefaultModeSetList(IMS_UINT32 nDefaultModeSetList)
{
    m_nDefaultModeSetList = nDefaultModeSetList;
}

PUBLIC
void CodecAudioConfig::SetChannel(IMS_SINT32 nChannel)
{
    m_nChannel = nChannel;
}

PUBLIC
void CodecAudioConfig::SetDtx(IMS_BOOL bDtx)
{
    m_bDtx = bDtx;
}
