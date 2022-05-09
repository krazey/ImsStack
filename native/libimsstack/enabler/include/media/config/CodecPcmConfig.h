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

#ifndef _CODEC_PCM_CONFIG_H_
#define _CODEC_PCM_CONFIG_H_

#include "config/CodecConfig.h"

class CodecPcmConfig : public CodecConfig
{
public:
    CodecPcmConfig(IN IMS_SINT32 nType_, IN IMS_SINT32 nPayloadTypeNum_);
    virtual ~CodecPcmConfig();

public:
    // CodecConfig class
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);
    virtual void ToDebugString() const;
};
#endif  // _CODEC_PCM_CONFIG_H_
