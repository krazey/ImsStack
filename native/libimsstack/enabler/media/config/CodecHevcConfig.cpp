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

__IMS_TRACE_TAG_MEDIA__;

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
CodecHevcConfig::CodecHevcConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
        CodecVideoConfig(nType, nPayloadTypeNum, DEFAULT_HEVC_RESOLUTION_WIDTH,
                DEFAULT_HEVC_RESOLUTION_HEIGHT, DEFAULT_HEVC_FRAMERATE, DEFAULT_HEVC_BITRATE,
                DEFAULT_HEVC_SPROP_PARAMS_480X640, DEFAULT_HEVC_IMAGE_ATTR,
                DEFAULT_HEVC_FRAME_SIZE),
        m_nHevcProfile(DEFAULT_HEVC_PROFILE),
        m_nHevcLevel(DEFAULT_HEVC_LEVEL)
{
    IMS_TRACE_I("+CodecHevcConfig - Type[%d]", nType, 0, 0);
}

PUBLIC VIRTUAL CodecHevcConfig::~CodecHevcConfig()
{
    IMS_TRACE_I("~CodecHevcConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecHevcConfig::Create(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - piBuffer is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    CodecVideoConfig::Create(piCc);

    ICarrierConfig* piCcBundle =
            piCc->GetBundle(CarrierConfig::ImsVt::KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE);

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

    m_nPacketizationMode = piCcSubBundle->GetInt(
            CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT,
            DEFAULT_PACKETIZATION_MODE);

    m_strSpropParameterSets = piCcSubBundle->GetString(
            CarrierConfig::ImsVt::KEY_HEVC_SPROP_PARAMETER_SETS_STRING, AString::ConstNull());

    m_nHevcProfile =
            piCcSubBundle->GetInt(CarrierConfig::ImsVt::KEY_HEVC_PROFILE_INT, DEFAULT_HEVC_PROFILE);

    m_nHevcLevel =
            piCcSubBundle->GetInt(CarrierConfig::ImsVt::KEY_HEVC_LEVEL_INT, DEFAULT_HEVC_LEVEL);

    piCcSubBundle->ReleaseBundle();
    piCcBundle->ReleaseBundle();

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecHevcConfig::ToDebugString() const
{
    CodecVideoConfig::ToDebugString();

    IMS_TRACE_D("HevcProfile[%d], HevcLevel[%d]", m_nHevcProfile, m_nHevcLevel, 0);
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
