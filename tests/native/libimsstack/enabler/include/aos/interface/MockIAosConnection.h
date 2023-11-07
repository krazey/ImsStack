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

#ifndef MOCK_I_AOS_CONNECTION_H_
#define MOCK_I_AOS_CONNECTION_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "AStringArray.h"
#include "IpAddress.h"
#include "interface/IAosConnection.h"

class MockIAosConnection : public IAosConnection {
public:
    MockIAosConnection()
    {
        ON_CALL(*this, GetHostByName)
                .WillByDefault(
                        [this](IN const AString& strHostName, OUT ImsList<IpAddress>& objIps,
                                IN IMS_SINT32 nIpVersion)
                        {
                            return GetHostByNameInternal(strHostName, &objIps, nIpVersion);
                        });
    }

    MOCK_METHOD(IMS_BOOL, Activate, (), (override));
    MOCK_METHOD(void, Deactivate, (), (override));
    MOCK_METHOD(IMS_BOOL, IsActivationRequested, (), (override));
    MOCK_METHOD(IMS_UINT32, GetState, (), (override));
    MOCK_METHOD(IMS_SINT32, GetConnectionType, (), (override));
    MOCK_METHOD(void, SetListener, (IN IAosConnectionListener* piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosConnectionListener* piListener), (override));
    MOCK_METHOD(IMS_SINT32, GetMtu, (), (override));
    MOCK_METHOD(const IpAddress&, GetLocalAddress, (IN IMS_SINT32 nIpVersion), (override));
    MOCK_METHOD(const AStringArray&, GetPcscfAddress, (IN IMS_SINT32 nIpVersion), (override));
    MOCK_METHOD(IMS_SINT32, GetHostByName,
            (IN const AString& strHostName, OUT ImsList<IpAddress>& objIps,
                    IN IMS_SINT32 nIpVersion),
            (override));
    MOCK_METHOD(const AString&, GetIfaceName, (), (override));
    MOCK_METHOD(IMS_BOOL, IsEpdgEnabled, (), (override));
    MOCK_METHOD(IMS_BOOL, IsIpv6Preferred, (), (override));
    MOCK_METHOD(IMS_SINT32, GetIpcanCategory, (), (override));
    MOCK_METHOD(IMS_BOOL, IsLimitedServicePcoValue, (), (override));
    MOCK_METHOD(IMS_SINT32, GetCarrierSignalPcoValue, (), (override));
    MOCK_METHOD(void, SetCarrierSignalPcoValue, (IN IMS_SINT32 nValue), (override));

    // Add mock method that can set OUT parameter
    MOCK_METHOD(IMS_SINT32, GetHostByNameInternal,
            (IN const AString& strHostName, OUT ImsList<IpAddress>* objIps,
                    IN IMS_SINT32 nIpVersion));
};

#endif // MOCK_I_AOS_CONNECTION_H_
