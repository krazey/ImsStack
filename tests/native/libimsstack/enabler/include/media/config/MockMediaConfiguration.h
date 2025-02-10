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

#ifndef _MOCK_MEDIA_CONFIGURATION_H_
#define _MOCK_MEDIA_CONFIGURATION_H_

#include <gmock/gmock.h>
#include <config/MediaConfiguration.h>

class MockMediaConfiguration : public MediaConfiguration
{
public:
    explicit MockMediaConfiguration(MEDIA_CONTENT_TYPE eSessionType) :
            MediaConfiguration(eSessionType)
    {
    }
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
    MOCK_METHOD(IMS_BOOL, CreateCodecConfigs, (IN ICarrierConfig * piCc), (override));
    MOCK_METHOD(IMS_UINT32, MakeEachCodecs,
            (IN ICarrierConfig * piCc, IN IMS_UINT32 nCodec, IN IMS_UINT32 nCodecIndex,
                    IN ImsVector<IMS_SINT32> objPayloadTypeArray),
            (override));
    MOCK_METHOD(IMS_UINT32, MakeCodec,
            (IN ICarrierConfig * piCc, IN IMS_UINT32 nCodec, IN IMS_UINT32 nCodecIndex,
                    IN IMS_SINT32 nPayloadTypeNum),
            (override));
    MOCK_METHOD(void, ToDebugString, (), (const, override));
    MOCK_METHOD(void, ToDebugStringCodecs, (IN CodecConfig * pCodecConfig), (const, override));
    MOCK_METHOD(void, Clear, (), (override));
    MOCK_METHOD(IMS_UINT32, GetCodecType, (IN IMS_UINT32 nCodec), (const, override));
    MOCK_METHOD(void, SetPorts, (IN ICarrierConfig * piCc, IN const IMS_CHAR* pszKey), (override));
    MOCK_METHOD(void, SetRtcpIntervals, (IN ICarrierConfig * piCc, IN const IMS_CHAR* pszKey),
            (override));
};

#endif
