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

#ifndef MOCK_I_TEXT_NEGO_H_
#define MOCK_I_TEXT_NEGO_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include <text/TextNego.h>

class MockTextNego : public TextNego
{
public:
    MockTextNego(IMS_SINT32 nSlotId) :
            TextNego(nSlotId){};
    MOCK_METHOD(void, CreateProfiles,
            (IN MediaEnvironment * pEnvironment, IN TextConfiguration* pConfig), (override));
    MOCK_METHOD(IMS_BOOL, FormSDP,
            (IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
                    OUT IMediaDescriptor* pDescriptor, IN MEDIA_DIRECTION eDir,
                    IN IMS_BOOL bDisable),
            (override));
    MOCK_METHOD(IMS_BOOL, NegotiateSDP,
            (IN NEGO_STATE eNegoState, IN ISessionDescriptor* pSessionDescriptor,
                    IN IMediaDescriptor* pDescriptor, OUT MEDIA_DIRECTION* eDir),
            (override));
    MOCK_METHOD(void, FinalizeSDP,
            (IN ISessionDescriptor * pSessionDescriptor, NEGO_STATE eNegoState), (override));
    MOCK_METHOD(IMS_BOOL, SetPort, (IN IMS_UINT32 nPort), (override));
    MOCK_METHOD(const IPAddress&, GetLocalAddress, (), (override));
    MOCK_METHOD(IMS_UINT32, GetLocalPort, (), (override));
    MOCK_METHOD(const IPAddress&, GetNegotiatedRemoteAddr, (), (override));
    MOCK_METHOD(IMS_UINT32, GetNegotiatedRemotePort, (), (override));
    MOCK_METHOD(TextProfile*, GetNegotiatedLocalProfile, (), (override));
    MOCK_METHOD(TextProfile*, GetNegotiatedNegoProfile, (), (override));
    MOCK_METHOD(TextProfile*, GetNegotiatedPeerProfile, (), (override));
    MOCK_METHOD(MEDIA_DIRECTION, GetNegotiatedDirection, (), (override));
    MOCK_METHOD(TEXT_CODEC, GetNegotiatedCodec, (), (override));
    MOCK_METHOD(IMS_SINT32, GetNegotiatedRtpPort, (), (override));
    MOCK_METHOD(IMS_SINT32, GetMediaBandwidth, (), (override));
};

#endif
