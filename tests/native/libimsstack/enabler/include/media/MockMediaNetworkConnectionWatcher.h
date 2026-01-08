/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef MOCK_MEDIA_NETWORK_CONNECTION_WATCHER_H_
#define MOCK_MEDIA_NETWORK_CONNECTION_WATCHER_H_

#include <gmock/gmock.h>

#include "MediaNetworkConnectionWatcher.h"

class MockMediaNetworkConnectionWatcher : public MediaNetworkConnectionWatcher
{
public:
    explicit MockMediaNetworkConnectionWatcher(IN const IpAddress& objIpAddress) :
            MediaNetworkConnectionWatcher(objIpAddress) {};
    virtual ~MockMediaNetworkConnectionWatcher() override {}
    MOCK_METHOD(void, SetListener, (IN IMediaNetworkConnectionListener * piListener), (override));
    MOCK_METHOD(void, NetworkConnection_OnConnected, (IN INetworkConnection * pNetConnection),
            (override));
    MOCK_METHOD(void, NetworkConnection_OnDisconnected,
            (IN INetworkConnection * pNetConnection, IN IMS_SINT32 nErrorCode), (override));
    MOCK_METHOD(void, NetworkConnection_OnConnectionFailed,
            (IN INetworkConnection * pNetConnection, IN IMS_SINT32 nErrorCode), (override));
    MOCK_METHOD(void, NetworkConnection_OnIpChanged, (IN INetworkConnection * pNetConnection),
            (override));
    MOCK_METHOD(void, NetworkConnection_OnIpcanChanged, (IN INetworkConnection * pNetConnection),
            (override));
    MOCK_METHOD(void, NetworkConnection_OnPcscfChanged, (IN INetworkConnection * pNetConnection),
            (override));
    MOCK_METHOD(IMS_SINT32, GetNetworkType, (), (const));
    MOCK_METHOD(IMS_SINT32, GetMtu, (), (const));
};
#endif
