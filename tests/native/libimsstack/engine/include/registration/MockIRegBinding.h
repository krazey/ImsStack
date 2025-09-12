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
#ifndef MOCK_I_REG_BINDING_H_
#define MOCK_I_REG_BINDING_H_

#include <gmock/gmock.h>

#include "IRegBinding.h"

class MockIRegBinding : public IRegBinding
{
public:
    MockIRegBinding() = default;
    ~MockIRegBinding() override = default;

    MOCK_METHOD(const AStringArray&, GetAssociatedUris, (), (const, override));
    MOCK_METHOD(const SipAddress&, GetAuthorizedAor, (), (const, override));
    MOCK_METHOD(const SipAddress&, GetContactAddress, (), (const, override));
    MOCK_METHOD(const SipAddress*, GetContactAddressForOutgoingMessage, (), (const, override));
    MOCK_METHOD(const IpAddress&, GetIpAddress, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetPathHeaders, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPortFlowControl, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPortUc, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPortUs, (), (const, override));
    MOCK_METHOD(const IRegInfo*, GetRegInfo, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetSecurityClients, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetSecurityVerifys, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetServiceRoutes, (), (const, override));
    MOCK_METHOD(SipProfile*, GetSipProfile, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(const AString&, GetSubscriberId, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetTransportExt, (), (const, override));
    MOCK_METHOD(const SipParameter*, GetInstanceParameter, (), (const, override));
    MOCK_METHOD(const SipAddress*, GetPublicGruu, (), (const, override));
    MOCK_METHOD(const SipAddress*, GetTemporaryGruu, (), (const, override));
    MOCK_METHOD(const ImsList<SipAddress*>&, GetTemporaryGruus, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsBehindNat, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyRegistration, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsWithinTrustDomain, (), (const, override));
    MOCK_METHOD(void, NotifyCallerCapabilityChanged, (), (override));
    MOCK_METHOD(void, SetListener, (IN IRegBindingListener * piListener), (override));
};

#endif
