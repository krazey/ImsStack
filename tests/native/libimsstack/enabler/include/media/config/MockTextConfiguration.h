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

#ifndef MOCK_TEXT_CONFIGURATION_H_
#define MOCK_TEXT_CONFIGURATION_H_

#include <gmock/gmock.h>

#include "config/TextConfiguration.h"

class MockTextConfiguration : public TextConfiguration
{
public:
    MockTextConfiguration() :
            TextConfiguration(MEDIA_TYPE_TEXT) {};
    explicit MockTextConfiguration(MEDIA_CONTENT_TYPE eSessionType) :
            TextConfiguration(eSessionType) {};

    virtual ~MockTextConfiguration() = default;

    // --- Mocked Methods from MediaConfiguration (Base Class) ---
    MOCK_METHOD(IMS_BOOL, Create, (IN ICarrierConfig * piCc), (override));
    MOCK_METHOD(IMS_BOOL, Update, (IN ICarrierConfig * piCc), (override));
    MOCK_METHOD(CodecConfig*, GetCodecConfig, (IN IMS_UINT32 nCodec), (const, override));
    MOCK_METHOD(const ImsList<CodecConfig*>&, GetCodecConfigs, (), (const, override));
    MOCK_METHOD(MEDIA_CONTENT_TYPE, GetSessionType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPortRtp, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPortRtpEnd, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPortRtcp, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRtcpIntervalOnActive, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRtcpIntervalOnHold, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetAsBandwidthKbps, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRsBandwidthBps, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRrBandwidthBps, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRtpInactivityTimerMillis, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRtcpInactivityTimerMillis, (), (const, override));

    // --- Mocked Methods specific to TextConfiguration ---
    MOCK_METHOD(IMS_SINT32, GetT140PayloadType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRedPayloadType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetTextDscp, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsTextCodecEmptyRedundantEnabled, (), (const, override));
};

#endif  // MOCK_TEXT_CONFIGURATION_H_
