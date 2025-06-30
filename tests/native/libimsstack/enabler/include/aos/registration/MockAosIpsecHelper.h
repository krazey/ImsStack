/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef MOCK_AOS_IPSEC_HELPER_H_
#define MOCK_AOS_IPSEC_HELPER_H_

#include <gmock/gmock.h>

#include "registration/AosIpsecHelper.h"

class MockAosIpsecHelper : public AosIpsecHelper
{
public:
    MockAosIpsecHelper() :
            AosIpsecHelper()
    {
    }
    ~MockAosIpsecHelper() override {}
    MOCK_METHOD(IMS_BOOL, Create, (IN IMS_BOOL bInitial), (override));
    MOCK_METHOD(void, CreateOnChallenging, (), (override));
    MOCK_METHOD(IMS_BOOL, SetPcscfPortnSpi, (), (override));
    MOCK_METHOD(IMS_BOOL, IsPcscfServerPortDifferent, (), (override));
    MOCK_METHOD(IMS_BOOL, UpdatePreloadedRoute, (IN const AString& strPcscf), (override));
    MOCK_METHOD(IMS_BOOL, MakeSas,
            (IN const AString& strPcscf, IN const IpAddress& objIpa, IN const ByteArray& objIk,
                    IN const ByteArray& objCk),
            (override));
    MOCK_METHOD(IMS_BOOL, ProcessAuthChallenged, (IN IMS_SINT32 nAlgorithm), (override));
    MOCK_METHOD(void, ProcessRegStarted, (), (override));
    MOCK_METHOD(IMS_BOOL, ProcessRegUpdated, (), (override));
    MOCK_METHOD(void, InitIpsec, (), (override));
    MOCK_METHOD(IMS_BOOL, IsEstablished, (), (override));
    MOCK_METHOD(void, SetSecurityServerPortInRegContact, (), (override));
    MOCK_METHOD(void, IgnoreCurrentPolicyExpired, (), (override));
    MOCK_METHOD(void, SetSecurityServerPortInRegistration, (), (override));
    MOCK_METHOD(void, SetUePortnSpi, (IN IMS_BOOL bInitial), (override));
    MOCK_METHOD(IMS_BOOL, SetSecurityClientHeader, (), (override));
    MOCK_METHOD(IMS_BOOL, CheckSecurityServerHeader, (), (override));
    MOCK_METHOD(void, ProcessPolicyExpired, (IN AosIpsec * pIpsec), (override));
};

#endif  // MOCK_AOS_IPSEC_HELPER_H_