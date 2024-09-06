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
#include "config/CodecTelephoneEventConfig.h"

__IMS_TRACE_TAG_MEDIA__;

#define DEFAULT_EVENT "0-15"

PUBLIC
CodecTelephoneEventConfig::CodecTelephoneEventConfig(
        IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
        CodecConfig(nType, nPayloadTypeNum),
        m_strEvents(DEFAULT_EVENT),
        m_nRedundancyCount(DEFAULT_REDUNDANT_COUNT),
        m_nSamplingRate(DEFAULT_SAMPLING_RATE_WB)
{
    IMS_TRACE_D("+CodecTelephoneEventConfig Type[%d]", nType, 0, 0);
}

PUBLIC VIRTUAL CodecTelephoneEventConfig::~CodecTelephoneEventConfig()
{
    IMS_TRACE_D("~CodecTelephoneEventConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecTelephoneEventConfig::Create(IN ICarrierConfig* /* piCc */)
{
    m_strEvents = DEFAULT_EVENT;
    m_nRedundancyCount = DEFAULT_REDUNDANT_COUNT;
    m_nSamplingRate = (GetCodec() == ImsCodec::AUDIO_TELEPHONE_EVENT) ? DEFAULT_SAMPLING_RATE
                                                                      : DEFAULT_SAMPLING_RATE_WB;

    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecTelephoneEventConfig::ToDebugString() const
{
    CodecConfig::ToDebugString();

    IMS_TRACE_D("strEvents(%s)", m_strEvents.GetStr(), 0, 0);
    IMS_TRACE_D("nRedundancyCount(%d), nSamplingRate(%d)", m_nRedundancyCount, m_nSamplingRate, 0);
}

PUBLIC
const AString& CodecTelephoneEventConfig::GetEvents() const
{
    return m_strEvents;
}

PUBLIC
IMS_SINT32 CodecTelephoneEventConfig::GetRedundancyCount() const
{
    return m_nRedundancyCount;
}

PUBLIC
IMS_SINT32 CodecTelephoneEventConfig::GetSamplingRate() const
{
    return m_nSamplingRate;
}
