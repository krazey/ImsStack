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
#include "config/CodecAvcConfig.h"

__IMS_TRACE_TAG_USER_DECL__("MED.CONF");

#define DEFAULT_AVC_SPROP_PARAMS "Z0LAFukDwKMg,aM4G4g=="
#define DEFAULT_AVC_PROFILE_ID   "42C00C"
#define DEFAULT_AVC_IMAGE_ATTR \
    "send [x=320,y=240] [x=640,y=480] recv [x=320,y=240] [x=640,y=480] [x=1280,y=720]"
#define DEFAULT_AVC_FRAME_SIZE "NEED_TO_CHECK"

PUBLIC
CodecAvcConfig::CodecAvcConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum_) :
        CodecConfig(nType_, nPayloadTypeNum_),
        m_nChannel(DEFAULT_CHANNEL),
        m_nResolutionWidth(DEFAULT_AVC_RESOLUTION_WIDTH),
        m_nResolutionHeight(DEFAULT_AVC_RESOLUTION_HEIGHT),
        m_nFramerate(DEFAULT_AVC_FRAMERATE),
        m_nBitrate(DEFAULT_AVC_BITRATE),
        m_nPacketizationMode(DEFAULT_PACKETIZATION_MODE),
        m_bIncludeSpropParameterSets(DEFAULT_INCLUDE_SPROP),
        m_strSpropParameterSets(DEFAULT_AVC_SPROP_PARAMS),
        m_strProfileLevelId(DEFAULT_AVC_PROFILE_ID),
        m_strImageAttr(DEFAULT_AVC_IMAGE_ATTR),
        m_strFrameSize(DEFAULT_AVC_FRAME_SIZE)

{
    IMS_TRACE_D("+CodecAvcConfig Type[%d]", nType_, 0, 0);
}

PUBLIC VIRTUAL CodecAvcConfig::~CodecAvcConfig()
{
    IMS_TRACE_D("~CodecAvcConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecAvcConfig::Create(IN ICarrierConfig* piCc, IN IMS_SINT32 nCodecIdx)
{
    if (piCc == IMS_NULL || nCodecIdx < 0)
    {
        IMS_TRACE_E(0, "Create - piBuffer is NULL", 0, 0, 0);
        return IMS_FALSE;
    }
#if 0 /** TODO_MEDIA need to check bundle */
    ICarrierConfig* piCcBundle =
            piCc->GetBundle(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE);

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

    IMSVector<IMS_SINT32> objVideoCodecAttributeResolution = piCcSubBundle->GetIntArray(
            CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY);
    if (!objVideoCodecAttributeResolution.IsEmpty())
    {
        m_nResolutionWidth = objVideoCodecAttributeResolution.GetAt(0);
        if (objVideoCodecAttributeResolution.GetSize() > 1)
        {
            m_nResolutionHeight = objVideoCodecAttributeResolution.GetAt(1);
        }
    }

    m_nFramerate =
            piCcSubBundle->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT);
    m_nBitrate = piCcSubBundle->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_BITRATE_INT_ARRAY);
    m_nPacketizationMode = piCcSubBundle->GetInt(
            CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT);

    m_strProfileLevelId = piCcSubBundle->GetString(
            CarrierConfig::ImsVt::KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING);

    /** TODO_MEDIA need to add after creating items in CarrierConfig */
    // m_strImageAttr = piCcSubBundle->GetString(
    //         CarrierConfig::ImsVt::KEY_VIDEO_CODEC_AVC_IMAGE_ATTR_STRING);
    // m_strFrameSize = piCcSubBundle->GetString(
    //         CarrierConfig::ImsVt::KEY_VIDEO_CODEC_AVC_FRAME_SIZE_STRING);

    /** TODO_MEDIA - need to implement */
    // objVideoCodecBitrate =
    //         piCc->GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_BITRATE_INT_ARRAY);
    // objVideoCodecImageAttr =
    //         piCc->GetStringArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_IMAGE_ATTR_STRING_ARRAY);
    // objVideoCodecFrameSize =
    //         piCc->GetStringArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_FRAME_SIZE_STRING_ARRAY);
    // objVideoCodecHevcProfile =
    //         piCc->GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_HEVC_PROFILE_INT_ARRAY);
    // objVideoCodecHevcLevel =
    //         piCc->GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_HEVC_LEVEL_INT_ARRAY);

    piCcSubBundle->ReleaseBundle();
    piCcSubBundle = IMS_NULL;

    piCcBundle->ReleaseBundle();
    piCcBundle = IMS_NULL;
#else
    IMSVector<IMS_SINT32> objVideoCodecAttributeResolution =
            piCc->GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY);
    if (!objVideoCodecAttributeResolution.IsEmpty())
    {
        m_nResolutionWidth = objVideoCodecAttributeResolution.GetAt(0);
        if (objVideoCodecAttributeResolution.GetSize() > 1)
        {
            m_nResolutionHeight = objVideoCodecAttributeResolution.GetAt(1);
        }
    }

    m_nFramerate = piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT);
    IMSVector<IMS_SINT32> objVideoBitrate =
            piCc->GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_BITRATE_INT_ARRAY);
    if (!objVideoBitrate.IsEmpty())
    {
        m_nBitrate = objVideoBitrate.GetAt(0);
        // if (objVideoBitrate.GetSize() > 1)
        // {
        //     m_nBitrate = objVideoBitrate.GetAt(1);
        // }
    }

    m_nPacketizationMode =
            piCc->GetInt(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT);
    m_strProfileLevelId = piCc->GetString(
            CarrierConfig::ImsVt::KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING,
            AString::ConstNull());

    IMSVector<AString> objImageAttr =
            piCc->GetStringArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_IMAGE_ATTR_STRING_ARRAY);
    if (objImageAttr.IsEmpty())
    {
        m_strImageAttr = AString::ConstNull();
    }
    else
    {
        m_strImageAttr = objImageAttr.GetAt(0);
    }
    // m_strFrameSize = piCcSubBundle->GetString(
    //         CarrierConfig::ImsVt::KEY_VIDEO_CODEC_AVC_FRAME_SIZE_STRING);

#endif
    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecAvcConfig::ToDebugString() const
{
    CodecConfig::ToDebugString();

    IMS_TRACE_D("ResolutionWidth (%d), ResolutionHeight(%d)", m_nResolutionWidth,
            m_nResolutionHeight, 0);
    IMS_TRACE_D("Framerate (%d), Bitrate(%d)", m_nFramerate, m_nBitrate, 0);
    IMS_TRACE_D("PacketizationMode (%d), bIncludeSpropParameterSets(%d), strSpropParameterSets(%s)",
            m_nPacketizationMode, m_bIncludeSpropParameterSets, m_strSpropParameterSets.GetStr());
    IMS_TRACE_D("strProfileLevelId(%s), strVideoCodecImageAttr(%s), strVideoCodecFrameSize(%s)",
            m_strProfileLevelId.GetStr(), m_strImageAttr.GetStr(), m_strFrameSize.GetStr());
}

PUBLIC
IMS_SINT32 CodecAvcConfig::GetChannel() const
{
    return m_nChannel;
}

PUBLIC
IMS_SINT32 CodecAvcConfig::GetResolutionWidth() const
{
    return m_nResolutionWidth;
}

PUBLIC
IMS_SINT32 CodecAvcConfig::GetResolutionHeight() const
{
    return m_nResolutionHeight;
}

PUBLIC
IMS_SINT32 CodecAvcConfig::GetFramerate() const
{
    return m_nFramerate;
}

PUBLIC
IMS_SINT32 CodecAvcConfig::GetBitrate() const
{
    return m_nBitrate;
}

PUBLIC
IMS_SINT32 CodecAvcConfig::GetPacketizationMode() const
{
    return m_nPacketizationMode;
}

PUBLIC
IMS_BOOL CodecAvcConfig::GetIncludeSpropParameterSets() const
{
    /** TODO - need to check later */
    return m_bIncludeSpropParameterSets;
}

PUBLIC
const AString& CodecAvcConfig::GetSpropParameterSets() const
{
    /** TODO - need to check later */
    return m_strSpropParameterSets;
}

PUBLIC
const AString& CodecAvcConfig::GetProfileLevelId() const
{
    return m_strProfileLevelId;
}

PUBLIC
const AString& CodecAvcConfig::GetImageAttr() const
{
    return m_strImageAttr;
}

PUBLIC
const AString& CodecAvcConfig::GetFrameSize() const
{
    return m_strFrameSize;
}
