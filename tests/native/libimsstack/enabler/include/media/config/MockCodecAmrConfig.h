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

#ifndef MOCK_CODEC_AMR_CONFIG_H_
#define MOCK_CODEC_AMR_CONFIG_H_

#include <gmock/gmock.h>

#include "config/CodecAmrConfig.h"

class MockCodecAmrConfig : public CodecAmrConfig
{
public:
    MockCodecAmrConfig(IN IMS_SINT32 nType, IN IMS_SINT32 nPayloadTypeNum) :
            CodecAmrConfig(nType, nPayloadTypeNum)
    {
    }
    ~MockCodecAmrConfig() override = default;

    // Mock virtual methods from CodecAmrConfig and its base classes
    MOCK_METHOD(IMS_BOOL, Create, (IN ICarrierConfig * piCc), (override));
    MOCK_METHOD(void, CreateDefaultAmrCodec, (), (override));

    // Mock methods from CodecConfig
    MOCK_METHOD(IMS_SINT32, GetCodec, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPayloadType, (), (const, override));

    // Mock getters from CodecAudioConfig
    MOCK_METHOD(IMS_SINT32, GetChannel, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetModeSetList, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetDefaultModeSetList, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetModeChangeCapability, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetModeChangePeriod, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetModeChangeNeighbor, (), (const, override));
    MOCK_METHOD(IMS_BOOL, GetVisibleModeSet, (), (const, override));
    MOCK_METHOD(IMS_BOOL, GetVisibleModeChangeCapability, (), (const, override));
    MOCK_METHOD(IMS_BOOL, GetVisibleModeChangePeriod, (), (const, override));
    MOCK_METHOD(IMS_BOOL, GetVisibleModeChangeNeighbor, (), (const, override));
    MOCK_METHOD(IMS_BOOL, GetDtx, (), (const, override));

    // Mock getters from CodecAmrConfig
    MOCK_METHOD(IMS_SINT32, GetOctetAlign, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSamplingRate, (), (const, override));
};

#endif  // MOCK_CODEC_AMR_CONFIG_H_
