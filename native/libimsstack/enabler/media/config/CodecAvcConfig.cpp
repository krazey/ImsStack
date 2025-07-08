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

__IMS_TRACE_TAG_MEDIA__;

#define DEFAULT_AVC_PROFILE_ID "42C00C"
#define DEFAULT_AVC_IMAGE_ATTR \
    "send [x=320,y=240] [x=640,y=480] recv [x=320,y=240] [x=640,y=480] [x=1280,y=720]"
#define DEFAULT_AVC_FRAME_SIZE "NEED_TO_CHECK"
#define DEFAULT_AVC_SPROP_PARAMS "Z0LAFtoHgUZA,aM4G8g=="

PUBLIC
CodecAvcConfig::CodecAvcConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
        CodecVideoConfig(nType, nPayloadTypeNum, DEFAULT_AVC_RESOLUTION_WIDTH,
                DEFAULT_AVC_RESOLUTION_HEIGHT, DEFAULT_AVC_FRAMERATE, DEFAULT_AVC_BITRATE,
                AString::ConstNull(), DEFAULT_AVC_IMAGE_ATTR, DEFAULT_AVC_FRAME_SIZE),
        m_bIncludeSpropParameterSets(DEFAULT_INCLUDE_SPROP),
        m_strProfileLevelId(DEFAULT_AVC_PROFILE_ID)
{
    IMS_TRACE_I("+CodecAvcConfig - Type[%d]", nType, 0, 0);
}

PUBLIC VIRTUAL CodecAvcConfig::~CodecAvcConfig()
{
    IMS_TRACE_I("~CodecAvcConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecAvcConfig::Create(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Create - Invalid params", 0, 0, 0);
        return IMS_FALSE;
    }

    CodecVideoConfig::Create(piCc);

    ICarrierConfig* piCcBundle =
            piCc->GetBundle(CarrierConfig::ImsVt::KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE);

    if (piCcBundle == IMS_NULL)
    {
        IMS_TRACE_D("Create - H264 codec is invalid", 0, 0, 0);
        CreateDefaultAvcCodec();
    }
    else
    {
        IMS_TRACE_D("Create - H264 codec is valid: PayloadType[%d]", m_nPayloadType, 0, 0);

        AString strPayloadTypeNumber;
        strPayloadTypeNumber.SetNumber(m_nPayloadType);
        ICarrierConfig* piCcSubBundle = piCcBundle->GetBundle(strPayloadTypeNumber.GetStr());

        if (piCcSubBundle == IMS_NULL)
        {
            IMS_TRACE_D("Create - H.264 SubBundle[%d] is invalid", m_nPayloadType, 0, 0);
            CreateDefaultAvcCodec();
        }
        else
        {
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
            else
            {
                m_nResolutionWidth = DEFAULT_AVC_RESOLUTION_WIDTH;
                m_nResolutionHeight = DEFAULT_AVC_RESOLUTION_HEIGHT;
                IMS_TRACE_D("Create - Default Resolution width[%d], height[%d]", m_nResolutionWidth,
                        m_nResolutionHeight, 0);
            }

            m_nFramerate = piCcSubBundle->GetInt(
                    CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_FRAME_RATE_INT,
                    DEFAULT_AVC_FRAMERATE);
            m_nPacketizationMode = piCcSubBundle->GetInt(
                    CarrierConfig::ImsVt::KEY_VIDEO_CODEC_ATTRIBUTE_PACKETIZATION_MODE_INT,
                    DEFAULT_PACKETIZATION_MODE);
            m_strSpropParameterSets = piCcSubBundle->GetString(
                    CarrierConfig::ImsVt::KEY_AVC_SPROP_PARAMETER_SETS_STRING,
                    AString::ConstNull());
            m_bIncludeSpropParameterSets =
                    (m_strSpropParameterSets.GetLength() > 0) ? IMS_TRUE : IMS_FALSE;

            if (!m_bIncludeSpropParameterSets)
            {
                m_strSpropParameterSets = DEFAULT_AVC_SPROP_PARAMS;
            }

            m_strProfileLevelId = piCcSubBundle->GetString(
                    CarrierConfig::ImsVt::KEY_H264_VIDEO_CODEC_ATTRIBUTE_PROFILE_LEVEL_ID_STRING,
                    DEFAULT_AVC_PROFILE_ID);

            piCcSubBundle->ReleaseBundle();
        }

        piCcBundle->ReleaseBundle();
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecAvcConfig::ToDebugString() const
{
    CodecVideoConfig::ToDebugString();

    IMS_TRACE_D("IncludeSpropParameterSets[%d], ProfileLevelId[%s]", m_bIncludeSpropParameterSets,
            m_strProfileLevelId.GetStr(), 0);
}

PUBLIC
IMS_BOOL CodecAvcConfig::GetIncludeSpropParameterSets() const
{
    return m_bIncludeSpropParameterSets;
}

PUBLIC
const AString& CodecAvcConfig::GetProfileLevelId() const
{
    return m_strProfileLevelId;
}

PUBLIC VIRTUAL void CodecAvcConfig::CreateDefaultAvcCodec()
{
    IMS_TRACE_D(
            "CreateDefaultAvcCodec: codec[%d], payloadTypeNumber[%d]", m_nCodec, m_nPayloadType, 0);

    m_nResolutionWidth = DEFAULT_AVC_RESOLUTION_WIDTH;
    m_nResolutionHeight = DEFAULT_AVC_RESOLUTION_HEIGHT;
    m_nFramerate = DEFAULT_AVC_FRAMERATE;
    m_nPacketizationMode = DEFAULT_PACKETIZATION_MODE;
    m_bIncludeSpropParameterSets = IMS_FALSE;
    m_strSpropParameterSets = DEFAULT_AVC_SPROP_PARAMS;
    m_strProfileLevelId = DEFAULT_AVC_PROFILE_ID;
}
