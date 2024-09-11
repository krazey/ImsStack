/**
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
#include "config/CodecVideoConfig.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
CodecVideoConfig::CodecVideoConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum,
        IN IMS_SINT32 nResolutionWidth, IN IMS_SINT32 nResolutionHeight, IN IMS_SINT32 nFramerate,
        IN IMS_SINT32 nBitrate, IN AString strSpropParameterSets, IN AString strImageAttr,
        IN AString strFrameSize) :
        CodecConfig(nType, nPayloadTypeNum),
        m_nChannel(DEFAULT_CHANNEL),
        m_nResolutionWidth(nResolutionWidth),
        m_nResolutionHeight(nResolutionHeight),
        m_nFramerate(nFramerate),
        m_nBitrate(nBitrate),
        m_nPacketizationMode(DEFAULT_PACKETIZATION_MODE),
        m_strSpropParameterSets(strSpropParameterSets),
        m_strImageAttr(strImageAttr),
        m_strFrameSize(strFrameSize)
{
    IMS_TRACE_I("+CodecVideoConfig - Type[%d]", nType, 0, 0);
}

PUBLIC VIRTUAL CodecVideoConfig::~CodecVideoConfig()
{
    IMS_TRACE_I("~CodecVideoConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecVideoConfig::Create(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piBuffer is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    ImsVector<IMS_SINT32> objVideoBitrate =
            piCc->GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_BITRATE_INT_ARRAY);
    if (!objVideoBitrate.IsEmpty())
    {
        m_nBitrate = objVideoBitrate.GetAt(0);
    }

    ImsVector<AString> objImageAttr =
            piCc->GetStringArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_IMAGE_ATTR_STRING_ARRAY);
    m_strImageAttr = (objImageAttr.IsEmpty()) ? AString::ConstNull() : objImageAttr.GetAt(0);

    ImsVector<AString> objFrameSize =
            piCc->GetStringArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_FRAME_SIZE_STRING_ARRAY);
    m_strFrameSize = (objFrameSize.IsEmpty()) ? AString::ConstNull() : objFrameSize.GetAt(0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecVideoConfig::ToDebugString() const
{
    CodecConfig::ToDebugString();

    IMS_TRACE_D("Channel[%d], ResolutionWidth[%d], ResolutionHeight[%d]", m_nChannel,
            m_nResolutionWidth, m_nResolutionHeight);
    IMS_TRACE_D("Framerate[%d], Bitrate[%d], PacketizationMode[%d]", m_nFramerate, m_nBitrate,
            m_nPacketizationMode);
    IMS_TRACE_D("SpropParameterSets[%s], VideoCodecImageAttributes[%s], VideoCodecFrameSize[%s]",
            m_strSpropParameterSets.GetStr(), m_strImageAttr.GetStr(), m_strFrameSize.GetStr());
}

PUBLIC
IMS_SINT32 CodecVideoConfig::GetChannel() const
{
    return m_nChannel;
}

PUBLIC
IMS_SINT32 CodecVideoConfig::GetResolutionWidth() const
{
    return m_nResolutionWidth;
}

PUBLIC
IMS_SINT32 CodecVideoConfig::GetResolutionHeight() const
{
    return m_nResolutionHeight;
}

PUBLIC
IMS_SINT32 CodecVideoConfig::GetFramerate() const
{
    return m_nFramerate;
}

PUBLIC
IMS_SINT32 CodecVideoConfig::GetBitrate() const
{
    return m_nBitrate;
}

PUBLIC
IMS_SINT32 CodecVideoConfig::GetPacketizationMode() const
{
    return m_nPacketizationMode;
}

PUBLIC
const AString& CodecVideoConfig::GetSpropParameterSets() const
{
    return m_strSpropParameterSets;
}

PUBLIC
const AString& CodecVideoConfig::GetImageAttr() const
{
    return m_strImageAttr;
}

PUBLIC
const AString& CodecVideoConfig::GetFrameSize() const
{
    return m_strFrameSize;
}
