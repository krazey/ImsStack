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
#ifndef INTERFACE_NETWORK_PING_H_
#define INTERFACE_NETWORK_PING_H_

#include "IpAddress.h"

class INetworkPingListener;

class INetworkPing
{
protected:
    virtual ~INetworkPing() = default;

public:
    /**
     * @brief Destroys INetworkPing object.
     */
    virtual void Destroy() = 0;

    /**
     * @brief Checks the connection status if the destination node is alive or not.
     *
     * @param objSrcIp The source IP address
     * @param objDstIp The destination IP address
     * @param nDstPort The destination port number
     * @param nWaitTime The waiting tiime for ping test (milli-seconds)
     * @return The result status of ping test.\n
     *         #PING_STATUS_OK\n
     *         #PING_STATUS_PENDING\n
     *         #PING_STATUS_NOK
     */
    virtual IMS_SINT32 Ping(IN const IPAddress& objSrcIp, IN const IPAddress& objDstIp,
            IN IMS_SINT32 nDstPort, IN IMS_SINT32 nWaitTime) = 0;

    /**
     * @brief Sets the listener to receive the connection status.
     *
     * @param piListener The listener to be notified the result of connection status
     */
    virtual void SetListener(IN INetworkPingListener* piListener) = 0;

public:
    /// Ping result status
    enum
    {
        /// Peer is alive
        PING_STATUS_OK = 0,
        /// PING check pending
        PING_STATUS_PENDING = 1,
        /// Internal operation failure
        PING_STATUS_NOK = 2,
        /// PING check and dead peer detected
        PING_STATUS_DEAD_PEER = 3,
        /// PING check and timed out
        PING_STATUS_TIMEDOUT = 4
    };
};

#endif
