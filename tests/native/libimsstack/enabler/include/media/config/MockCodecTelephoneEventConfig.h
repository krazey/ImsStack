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

#ifndef MOCK_CODEC_TELEPHONE_EVENT_CONFIG_H_
#define MOCK_CODEC_TELEPHONE_EVENT_CONFIG_H_

#include <gmock/gmock.h>

#include "config/CodecTelephoneEventConfig.h"

class MockCodecTelephoneEventConfig : public CodecTelephoneEventConfig
{
public:
    MockCodecTelephoneEventConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
            CodecTelephoneEventConfig(nType, nPayloadTypeNum)
    {
    }
    ~MockCodecTelephoneEventConfig() override = default;

    // Mock virtual methods from CodecTelephoneEventConfig and its base classes
    MOCK_METHOD(IMS_BOOL, Create, (IN ICarrierConfig * piCc), (override));
    MOCK_METHOD(void, ToDebugString, (), (const, override));
    MOCK_METHOD(const AString&, GetEvents, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRedundancyCount, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSamplingRate, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetCodec, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPayloadType, (), (const, override));
};

#endif  // MOCK_CODEC_TELEPHONE_EVENT_CONFIG_H_
