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
#ifndef MOCK_INTERFACE_NETWORK_PING_LISTENER_H_
#define MOCK_INTERFACE_NETWORK_PING_LISTENER_H_

#include <gmock/gmock.h>
#include "INetworkPingListener.h"

class MockINetworkPingListener : public INetworkPingListener
{
public:
    inline MockINetworkPingListener() {}
    inline virtual ~MockINetworkPingListener() {}

    MOCK_METHOD(void, NetworkPing_NotifyResult, (IN INetworkPing * piPing, IN IMS_SINT32 nResult),
            (override));
};

#endif
