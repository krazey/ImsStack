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

#ifndef _MOCK_VIDEO_CONFIGURATION_H_
#define _MOCK_VIDEO_CONFIGURATION_H_

#include <gmock/gmock.h>

#include "config/VideoConfiguration.h"

class MockVideoConfiguration : public VideoConfiguration
{
public:
    MockVideoConfiguration() :
            MockVideoConfiguration(MEDIA_TYPE_VIDEO) {};
    explicit MockVideoConfiguration(MEDIA_CONTENT_TYPE eSessionType) :
            VideoConfiguration(eSessionType) {};

    ~MockVideoConfiguration() override = default;

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

    // --- Mocked Methods specific to VideoConfiguration ---
    MOCK_METHOD(IMS_SINT32, GetVideoDscp, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetVideoSendPeriodicSpsPps, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetCvoId, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoAvpfEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoAvpfTrrEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoAvpfNackEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoAvpfTmmbrEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoAvpfPliEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoAvpfFirEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsAvpfCapabilityNegotiationEnabled, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSdpOfferCapNegoForAvpf, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetVideoIframeIntervalSec, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetChannel, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetVideoSamplingRate, (), (const, override));
    MOCK_METHOD(IMS_BOOL, GetBandwidthNegoOption, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetVideoLowestBitrateBps, (), (const, override));
    MOCK_METHOD(IMS_BOOL, isVideoDirectionHoldUsingInactive, (), (const, override));
};

#endif
