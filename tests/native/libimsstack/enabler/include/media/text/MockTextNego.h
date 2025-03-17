/*
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

#ifndef MOCK_TEXT_NEGO_H_
#define MOCK_TEXT_NEGO_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include <text/TextNego.h>

class MockTextNego : public TextNego
{
public:
    explicit MockTextNego(IMS_SINT32 nSlotId) :
            TextNego(nSlotId) {};
    MOCK_METHOD(void, CreateProfiles,
            (IN std::shared_ptr<MediaEnvironment> pEnvironment, IN MediaConfiguration* pConfig),
            (override));
    MOCK_METHOD(IMS_BOOL, FormSdp,
            (IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
                    OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir,
                    IN IMS_BOOL bDisable, IN IMS_BOOL bEnforceReofferMode),
            (override));
    MOCK_METHOD(IMS_BOOL, IsMediaCodecFromSdpSupported,
            (IN ISessionDescriptor * pSessionDescriptor, IN IMediaDescriptor* pDescriptor),
            (override));
    MOCK_METHOD(void, NegotiateSdp,
            (IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
                    IN IMediaDescriptor* pDescriptor, OUT IMS_SINT32& nDirection),
            (override));
    MOCK_METHOD(void, FinalizeSdp,
            (IN ISessionDescriptor * pSessionDescriptor, NEGO_STATE eNegoState), (override));
    MOCK_METHOD(const IpAddress&, GetLocalAddress, (), (override));
    MOCK_METHOD(IMS_UINT32, GetLocalPort, (), (override));
    MOCK_METHOD(const IpAddress&, GetNegotiatedRemoteAddress, (), (override));
    MOCK_METHOD(IMS_SINT32, GetRemotePort, (), (override));
    MOCK_METHOD(TextProfile*, GetNegotiatedLocalProfile, (), (override));
    MOCK_METHOD(TextProfile*, GetNegotiatedNegoProfile, (), (override));
    MOCK_METHOD(TextProfile*, GetNegotiatedPeerProfile, (), (override));
    MOCK_METHOD(MEDIA_DIRECTION, GetNegotiatedDirection, (), (override));
    MOCK_METHOD(TEXT_CODEC, GetNegotiatedCodec, (), (override));
    MOCK_METHOD(IMS_SINT32, GetNegotiatedRtpPort, (), (override));
    MOCK_METHOD(IMS_SINT32, GetNegotiatedBandwidth, (), (override));
};

#endif
