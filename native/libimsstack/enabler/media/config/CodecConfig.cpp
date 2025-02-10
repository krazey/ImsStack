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

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "MediaDef.h"
#include "config/CodecConfig.h"
#include "ICarrierConfig.h"
#include "config/ImsCodec.h"
#include "config/MediaSessionConfig.h"
#include "config/AudioConfiguration.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC
CodecConfig::CodecConfig(IN IMS_SINT32 nCodec, IN IMS_SINT32 nPayloadTypeNum) :
        m_nCodec(nCodec),
        m_nPayloadType(nPayloadTypeNum)
{
    IMS_TRACE_I("+CodecConfig - Codec[%d], PayloadType[%d]", m_nCodec, m_nPayloadType, 0);
}

PUBLIC VIRTUAL CodecConfig::~CodecConfig()
{
    IMS_TRACE_I("~CodecConfig", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL CodecConfig::Create(IN ICarrierConfig* piCc)
{
    (void)piCc;
    return IMS_TRUE;
}

PUBLIC VIRTUAL void CodecConfig::ToDebugString() const
{
    IMS_TRACE_D("Codec[%s], PayloadTypeNumber[%d]", ImsCodec::CodecToString(m_nCodec),
            m_nPayloadType, 0);
}

PUBLIC VIRTUAL IMS_SINT32 CodecConfig::GetCodec() const
{
    return m_nCodec;
}

PUBLIC VIRTUAL IMS_SINT32 CodecConfig::GetPayloadType() const
{
    return m_nPayloadType;
}
