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

#ifndef _CODEC_CONFIG_H_
#define _CODEC_CONFIG_H_

#include "CarrierConfig.h"
#include "ICarrierConfig.h"
#include "config/ImsCodec.h"

class CodecConfig
{
public:
    CodecConfig(IN IMS_SINT32 nCodec_, IN IMS_SINT32 nPayloadTypeNum_);
    virtual ~CodecConfig();

public:
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);
    virtual void ToDebugString() const;

    IMS_SINT32 GetCodec() const;
    IMS_SINT32 GetPayloadType() const;

public:

protected:
    IMS_SINT32 m_nCodec;
    IMS_SINT32 m_nPayloadType;
};
#endif                                              // _CODEC_CONFIG_H_
