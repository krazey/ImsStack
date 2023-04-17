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

#define DEFAULT_HEVC_SPROP_PARAMS_480X640                   \
    "AAAAAUABDAH//wFgAAADALAAAAMAAAMAWqxZ,"                 \
    "AAAAAUIBAQFgAAADALAAAAMAAAMAWqAPCAKBZa7kyS7gC7QoSg==," \
    "AAAAAUQBwPPAAhA="
#define DEFAULT_HEVC_SPROP_PARAMS_720X1280                  \
    "AAAAAUABDAH//wFgAAADALAAAAMAAAMAXaxZ,"                 \
    "AAAAAUIBAQFgAAADALAAAAMAAAMAXaAFwgBQHE+Wu5Mku4Au0KEo," \
    "AAAAAUQBwPPAAhA="
#define DEFAULT_HEVC_IMAGE_ATTR \
    "send [x=480,y=640] [x=720,y=1280] recv [x=480,y=640] [x=720,y=1280]"
#define DEFAULT_HEVC_FRAME_SIZE "NEED_TO_CHECK"

PUBLIC
CodecHevcConfig::CodecHevcConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum_) :
        CodecConfig(nType_, nPayloadTypeNum_),
        m_nChannel(DEFAULT_CHANNEL),
        m_nResolutionWidth(DEFAULT_HEVC_RESOLUTION_WIDTH),
        m_nResolutionHeight(DEFAULT_HEVC_RESOLUTION_HEIGHT),
        m_nFramerate(DEFAULT_HEVC_FRAMERATE),
        m_nBitrate(DEFAULT_HEVC_BITRATE),
        m_nPacketizationMode(DEFAULT_PACKETIZATION_MODE),
        m_strSpropParameterSets(DEFAULT_HEVC_SPROP_PARAMS_480X640),
        m_nHevcProfile(DEFAULT_HEVC_PROFILE),
        m_nHevcLevel(DEFAULT_HEVC_LEVEL),
        m_strImageAttr(DEFAULT_HEVC_IMAGE_ATTR),
        m_strFrameSize(DEFAULT_HEVC_FRAME_SIZE)
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
            piCc->GetBundle(CarrierConfig::Assets::KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE);

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

    ImsVector<IMS_SINT32> objVideoCodecAttributeResolution = piCcSubBundle->GetIntArray(
            CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_RESOLUTION_INT_ARRAY);
    if (!objVideoCodecAttributeResolution.IsEmpty())
    {
        m_nResolutionWidth = objVideoCodecAttributeResolution.GetAt(0);
        if (objVideoCodecAttributeResolution.GetSize() > 1)
        {
            m_nResolutionHeight = objVideoCodecAttributeResolution.GetAt(1);
        }
    }

    m_nFramerate = piCcSubBundle->GetInt(
            CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT, DEFAULT_HEVC_FRAMERATE);
    ImsVector<IMS_SINT32> objVideoBitrate =
            piCc->GetIntArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_BITRATE_INT_ARRAY);
    if (!objVideoBitrate.IsEmpty())
    {
        m_nBitrate = objVideoBitrate.GetAt(0);
    }

    m_nPacketizationMode = piCcSubBundle->GetInt(
            CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT,
            DEFAULT_PACKETIZATION_MODE);

    m_strSpropParameterSets = piCcSubBundle->GetString(
            CarrierConfig::Assets::KEY_HEVC_SPROP_PARAMETER_SETS_STRING, AString::ConstNull());

    m_nHevcProfile = piCcSubBundle->GetInt(
            CarrierConfig::Assets::KEY_HEVC_PROFILE_INT, DEFAULT_HEVC_PROFILE);

    m_nHevcLevel =
            piCcSubBundle->GetInt(CarrierConfig::Assets::KEY_HEVC_LEVEL_INT, DEFAULT_HEVC_LEVEL);

    ImsVector<AString> objImageAttr =
            piCc->GetStringArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_IMAGE_ATTR_STRING_ARRAY);
    m_strImageAttr = (objImageAttr.IsEmpty()) ? AString::ConstNull() : objImageAttr.GetAt(0);

    ImsVector<AString> objFrameSize =
            piCc->GetStringArray(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_FRAME_SIZE_STRING_ARRAY);
    m_strFrameSize = (objFrameSize.IsEmpty()) ? AString::ConstNull() : objFrameSize.GetAt(0);

    piCcSubBundle->ReleaseBundle();
    piCcBundle->ReleaseBundle();

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecHevcConfig::ToDebugString() const
{
    CodecConfig::ToDebugString();

    IMS_TRACE_D("m_nResolutionWidth(%d), m_nResolutionHeight(%d)", m_nResolutionWidth,
            m_nResolutionHeight, 0);
    IMS_TRACE_D("m_nFramerate(%d), m_nBitrate(%d), m_nPacketizationMode(%d)", m_nFramerate,
            m_nBitrate, m_nPacketizationMode);
    IMS_TRACE_D("m_strSpropParameterSets (%s)", m_strSpropParameterSets.GetStr(), 0, 0);
    IMS_TRACE_D("m_nHevcProfile (%d), m_nHevcLevel(%d)", m_nHevcProfile, m_nHevcLevel, 0);
    IMS_TRACE_D("strVideoCodecImageAttr(%s), strVideoCodecFrameSize(%s)", m_strImageAttr.GetStr(),
            m_strFrameSize.GetStr(), 0);
}

PUBLIC
IMS_SINT32 CodecHevcConfig::GetChannel() const
{
    return m_nChannel;
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
const AString& CodecHevcConfig::GetSpropParameterSets() const
{
    return m_strSpropParameterSets;
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
