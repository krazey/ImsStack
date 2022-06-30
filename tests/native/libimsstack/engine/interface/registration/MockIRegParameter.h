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
#ifndef MOCK_I_REG_PARAMETER_H_
#define MOCK_I_REG_PARAMETER_H_

#include <gmock/gmock.h>

#include "AStringArray.h"
#include "SipAddress.h"
#include "SipSecurityHeader.h"
#include "SipTimerValues.h"
#include "IRegParameter.h"

class MockIRegParameter : public IRegParameter
{
public:
    MOCK_METHOD(IMS_BOOL, AddExtraHeaders, (IN const AStringArray& objHeaders), (override));
    MOCK_METHOD(IMS_BOOL, AddMessageBodyPart, (IN ISipMessageBodyPart * piBodyPart), (override));
    MOCK_METHOD(IMS_BOOL, AddPreloadedRoute, (IN const AString& strRoute), (override));
    MOCK_METHOD(IMS_BOOL, AddPreloadedRoute,
            (IN const AString& strHost, IN IMS_SINT32 nPort, IN const AString& strScheme),
            (override));
    MOCK_METHOD(IMS_BOOL, AddSecurityClient, (IN const SipSecurityHeader& objSecurityHeader),
            (override));
    MOCK_METHOD(IMS_SINT32, GetPort, (), (const, override));
    MOCK_METHOD(const SipSecurityHeader*, GetPreferredSecurityClient, (), (const, override));
    MOCK_METHOD(const SipSecurityHeader*, GetPreferredSecurityServer, (), (const, override));
    MOCK_METHOD(const IMSList<SipSecurityHeader>&, GetSecurityServers, (), (const, override));
    MOCK_METHOD(const SipAddress&, GetTopmostRouteAddress, (), (const, override));
    MOCK_METHOD(void, RemoveAllMessageBodyParts, (), (override));
    MOCK_METHOD(void, RemoveAllPreloadedRoutes, (), (override));
    MOCK_METHOD(void, RemoveExtraHeaders, (IN const AStringArray& objHeaders), (override));
    MOCK_METHOD(void, RemoveSecurityClients, (), (override));
    MOCK_METHOD(void, SetAuthenticationCredentials, (IN IMS_BOOL bPolicy), (override));
    MOCK_METHOD(void, SetFlowControlOption, (IN IMS_SINT32 nOption), (override));
    MOCK_METHOD(void, SetPort, (IN IMS_SINT32 nPort), (override));
    MOCK_METHOD(void, SetPortFlowControl, (IN IMS_SINT32 nPort), (override));
    MOCK_METHOD(void, SetSecurityVerifys, (IN const IMSList<SipSecurityHeader>& objSecurityVerifys),
            (override));
    MOCK_METHOD(void, SetSipTimerValues, (IN const SipTimerValues& objTimerValues), (override));
    MOCK_METHOD(void, SetTransportExt, (IN IMS_SINT32 nTransportExt), (override));
    MOCK_METHOD(void, SetTransportExtForRegOnly, (IN IMS_SINT32 nTransportExt), (override));
};

#endif  // MOCK_I_REG_PARAMETER_H_
