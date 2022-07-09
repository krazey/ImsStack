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
#include "config/CodecHevcConfig.h"

__IMS_TRACE_TAG_USER_DECL__("MED.CONF");

PUBLIC
CodecHevcConfig::CodecHevcConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum_) :
        CodecConfig(nType_, nPayloadTypeNum_),
        m_nResolutionWidth(DEFAULT_RESOLUTION_WIDTH),
        m_nResolutionHeight(DEFAULT_RESOLUTION_HEIGHT),
        m_nFramerate(DEFAULT_FRAMERATE),
        m_nBitrate(DEFAULT_BITRATE),
        m_nPacketizationMode(DEFAULT_PACKETIZATION_MODE),
        m_bIncludeSpropParameterSets(DEFAULT_INCLUDE_SPROP),
        m_nHevcProfile(DEFAULT_HEVC_PROFILE),
        m_nHevcLevel(DEFAULT_HEVC_LEVEL),
        m_strImageAttr(DEFAULT_IMAGE_ATTR),
        m_strFrameSize(DEFAULT_FRAME_SIZE)

{
    IMS_TRACE_D("+CodecHevcConfig Type[%d]", nType_, 0, 0);
}

PUBLIC VIRTUAL CodecHevcConfig::~CodecHevcConfig()
{
    IMS_TRACE_D("~CodecHevcConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecHevcConfig::Create(IN ICarrierConfig* piCc, IN IMS_SINT32 nCodecIdx)
{
    if (piCc == IMS_NULL || nCodecIdx < 0)
    {
        IMS_TRACE_E(0, "Create - piBuffer is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

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

    /** TODO Media need to check this item */
    // m_bIncludeSpropParameterSets = piCcSubBundle->GetBoolean(
    //        CarrierConfig::Assets::KEY_VIDEO_CODEC_HEVC_SPROP_PARAMETER_SETS_BOOL);

    /** TODO_MEDIA need to add after creating items in CarrierConfig */
    // m_nHevcProfile = piCcSubBundle->GetInt(
    //         CarrierConfig::ImsVt::KEY_VIDEO_CODEC_HEVC_PROFILE_INT);
    // m_nHevcLevel = piCcSubBundle->GetInt(
    //         CarrierConfig::ImsVt::KEY_VIDEO_CODEC_HEVC_LEVEL_INT);

    // m_strImageAttr = piCcSubBundle->GetString(
    //         CarrierConfig::ImsVt::KEY_VIDEO_CODEC_AVC_IMAGE_ATTR_STRING);
    // m_strFrameSize = piCcSubBundle->GetString(
    //         CarrierConfig::ImsVt::KEY_VIDEO_CODEC_AVC_FRAME_SIZE_STRING);

    piCcSubBundle->ReleaseBundle();
    piCcSubBundle = IMS_NULL;

    piCcBundle->ReleaseBundle();
    piCcBundle = IMS_NULL;

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecHevcConfig::ToDebugString() const
{
    CodecConfig::ToDebugString();

    IMS_TRACE_D("m_nResolutionWidth (%d), m_nResolutionHeight(%d)", m_nResolutionWidth,
            m_nResolutionHeight, 0);
    IMS_TRACE_D("m_nFramerate (%d), m_nBitrate(%d)", m_nFramerate, m_nBitrate, 0);
    IMS_TRACE_D("m_nPacketizationMode (%d), m_bIncludeSpropParameterSets(%d)", m_nPacketizationMode,
            m_bIncludeSpropParameterSets, 0);
    IMS_TRACE_D("m_nHevcProfile (%d), m_nHevcLevel(%d)", m_nHevcProfile, m_nHevcLevel, 0);
    IMS_TRACE_D("strVideoCodecImageAttr(%s), strVideoCodecFrameSize(%s)", m_strImageAttr.GetStr(),
            m_strFrameSize.GetStr(), 0);
}

PUBLIC
IMS_SINT32 CodecHevcConfig::GetResolutionWidth() const
{
    return m_nResolutionWidth;
}

PUBLIC
IMS_SINT32 CodecHevcConfig::GetResolutionHeight() const
{
    return m_nResolutionHeight;
}

PUBLIC
IMS_SINT32 CodecHevcConfig::GetFramerate() const
{
    return m_nFramerate;
}

PUBLIC
IMS_SINT32 CodecHevcConfig::GetBitrate() const
{
    return m_nBitrate;
}

PUBLIC
IMS_SINT32 CodecHevcConfig::GetPacketizationMode() const
{
    return m_nPacketizationMode;
}

PUBLIC
IMS_BOOL CodecHevcConfig::GetIncludeSpropParameterSets() const
{
    return m_bIncludeSpropParameterSets;
}

PUBLIC
IMS_SINT32 CodecHevcConfig::GetHevcProfile() const
{
    return m_nHevcProfile;
}

PUBLIC
IMS_SINT32 CodecHevcConfig::GetHevcLevel() const
{
    return m_nHevcLevel;
}

PUBLIC
const AString& CodecHevcConfig::GetImageAttr() const
{
    return m_strImageAttr;
}

PUBLIC
const AString& CodecHevcConfig::GetFrameSize() const
{
    return m_strFrameSize;
}
