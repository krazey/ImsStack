/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MOCK_CODEC_AUDIO_CONFIG_H_
#define MOCK_CODEC_AUDIO_CONFIG_H_

#include <gmock/gmock.h>

#include "ICarrierConfig.h"
#include "MockICarrierConfig.h"
#include "config/MediaCarrierConfigBundle.h"
#include "config/CodecAmrConfig.h"

class MockCodecAudioConfig : public CodecAudioConfig
{
public:
    MockCodecAudioConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
            CodecAudioConfig(nType, nPayloadTypeNum)
    {
    }
    virtual ~MockCodecAudioConfig() = default;

    MOCK_METHOD(IMS_BOOL, Create, (ICarrierConfig * piCc), (override));
    MOCK_METHOD(void, ToDebugString, (), (const, override));
};
#endif
