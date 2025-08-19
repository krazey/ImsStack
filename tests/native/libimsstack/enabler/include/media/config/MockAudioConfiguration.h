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

#ifndef MOCK_AUDIO_CONFIGURATION_H_
#define MOCK_AUDIO_CONFIGURATION_H_

#include <gmock/gmock.h>

#include "config/AudioConfiguration.h"

class MockAudioConfiguration : public AudioConfiguration
{
public:
    MockAudioConfiguration() :
            AudioConfiguration(MEDIA_TYPE_AUDIO) {};
    explicit MockAudioConfiguration(MEDIA_CONTENT_TYPE eSessionType) :
            AudioConfiguration(eSessionType) {};

    ~MockAudioConfiguration() override = default;

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

    // --- Mocked Methods specific to AudioConfiguration ---
    MOCK_METHOD(IMS_BOOL, IsEvsSupported, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPtime, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetMaxPtime, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetMaxRed, (), (const, override));
    MOCK_METHOD(IMS_BOOL, GetBandwidthNegoOption, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRtpDscp, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetJitterBufferMinSize, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetJitterBufferMaxSize, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetJitterBufferAdjustTime, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetJitterBufferStepSize, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRtcpXrEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRtcpXrStatisticsEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRtcpXrVoipEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRtcpXrPlrEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRtcpXrPdrEnabled, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetDtmfDuration, (), (const, override));
    MOCK_METHOD(const ImsVector<AString>&, GetAudioCandidateAttribute, (), (const, override));
    MOCK_METHOD(
            IMS_BOOL, IsAudioInactivityCallEndReason, (IN IMS_SINT32 nReason), (const, override));
};

#endif  // MOCK_AUDIO_CONFIGURATION_H_
