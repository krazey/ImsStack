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
#ifndef MOCK_I_NETWORK_IP_SEC_H_
#define MOCK_I_NETWORK_IP_SEC_H_

#include <gmock/gmock.h>

#include "IIpSecPolicy.h"
#include "INetworkIpSec.h"
#include "ISocket.h"
#include "SocketAddress.h"

class MockINetworkIpSec : public INetworkIpSec
{
public:
    inline MockINetworkIpSec() {}
    inline virtual ~MockINetworkIpSec() {}

    MOCK_METHOD(IIpSecPolicy*, CreatePolicy, (), (override));
    MOCK_METHOD(void, DestroyPolicy, (IN IIpSecPolicy * piPolicy), (override));
    MOCK_METHOD(void, DestroyAllPolicies, (), (override));
    MOCK_METHOD(IMS_BOOL, AddPolicy, (IN IIpSecPolicy * piPolicy), (override));
    MOCK_METHOD(void, DeletePolicy, (IN IIpSecPolicy * piPolicy), (override));
    MOCK_METHOD(void, FlushPolicies, (), (override));
    MOCK_METHOD(void, DumpPolicy, (IN IIpSecPolicy * piPolicy), (override));
    MOCK_METHOD(IIpSecPolicy*, GetPolicy, (IN IMS_SINT32 nId), (const, override));
    MOCK_METHOD(IMS_BOOL, ApplyIpSecTransform,
            (IN ISocket * piSocket, IN const SocketAddress& objLocal,
                    IN const SocketAddress* pRemote),
            (override));
    MOCK_METHOD(IMS_BOOL, ApplyIpSecTransform, (IN ISocket * piSocket, IN ISocket* piServerSocket),
            (override));
    MOCK_METHOD(void, RemoveIpSecTransforms, (IN IMS_SINT32 nSocketId), (override));
    MOCK_METHOD(void, SetSdbFlushCapability, (IN IMS_BOOL bCapability), (override));
};

#endif
