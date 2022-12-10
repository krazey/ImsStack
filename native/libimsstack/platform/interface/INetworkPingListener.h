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
#ifndef INTERFACE_NETWORK_PING_LISTENER_H_
#define INTERFACE_NETWORK_PING_LISTENER_H_

#include "ImsTypeDef.h"

class INetworkPing;

class INetworkPingListener
{
protected:
    virtual ~INetworkPingListener() = default;

public:
    /**
     * @brief Notify the application that the specified data connection is alive or not.
     *
     * Application can query the result using INetworkPing interface.
     *
     * @param piPing The ping interface to check connection aliveness
     * @param nResult The ping status\n
     *                #INetworkPing#PING_STATUS_OK\n
     *                #INetworkPing#PING_STATUS_DEAD_PEER\n
     *                #INetworkPing#PING_STATUS_TIMEDOUT
     */
    virtual void NetworkPing_NotifyResult(IN INetworkPing* piPing, IN IMS_SINT32 nResult) = 0;
};

#endif
