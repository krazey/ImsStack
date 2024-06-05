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
#include "config/CodecT140Config.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
CodecT140Config::CodecT140Config(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
        CodecConfig(nType, nPayloadTypeNum),
        m_nRedLevel(DEFAULT_RED_LEVEL),
        m_nTextSamplingRate(DEFAULT_TEXT_SAMPLING_RATE)
{
    IMS_TRACE_D("+CodecT140Config Type[%d]", nType, 0, 0);
}

PUBLIC VIRTUAL CodecT140Config::~CodecT140Config()
{
    IMS_TRACE_D("~CodecT140Config", 0, 0, 0);
}

PUBLIC
IMS_BOOL CodecT140Config::Create(IN ICarrierConfig* piCc)
{
    if (piCc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // T140/RED parameters
    if (GetCodec() == ImsCodec::TEXT_RED)
    {
        m_nRedLevel = piCc->GetInt(CarrierConfig::Assets::KEY_TEXT_CODEC_REDUNDANCY_LEVEL_INT);
    }
    m_nTextSamplingRate = DEFAULT_TEXT_SAMPLING_RATE;

    if (GetCodec() == ImsCodec::TEXT_RED && m_nRedLevel <= 1)
    {
        IMS_TRACE_E(0, "'red' attribute needs more than 1 of redundancy(%d>1)", m_nRedLevel, 0, 0);
        return IMS_FALSE;
    }
    else if (GetCodec() == ImsCodec::TEXT_T140)
    {
        m_nRedLevel = DEFAULT_RED_LEVEL_NONE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecT140Config::ToDebugString() const
{
    CodecConfig::ToDebugString();

    IMS_TRACE_D("RedLevel (%d), SamplingRate (%d)", m_nRedLevel, m_nTextSamplingRate, 0);
}

PUBLIC
IMS_SINT32 CodecT140Config::GetRedLevel() const
{
    return m_nRedLevel;
}

PUBLIC
IMS_SINT32 CodecT140Config::GetSamplingRate() const
{
    return m_nTextSamplingRate;
}