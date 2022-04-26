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
#ifndef INTERFACE_NETWORK_IPSEC_H_
#define INTERFACE_NETWORK_IPSEC_H_

#include "ISocket.h"
#include "SocketAddress.h"

class ISocket;
class IIpSecPolicy;

class INetworkIpSec
{
public:
    /**
     * @brief Creates IpSec policy.
     *
     * @return The pointer of IIPSecPolicy.
     */
    virtual IIpSecPolicy* CreatePolicy() = 0;

    /**
     * @brief Destroys IpSec policy.
     *
     * @param piPolicy The IpSec policy to be destroyed
     */
    virtual void DestroyPolicy(IN IIpSecPolicy* piPolicy) = 0;

    /**
     * @brief Destroys all the security associations.
     */
    virtual void DestroyAllPolicies() = 0;

    /**
     * @brief Adds IpSec policy.
     *
     * @param piPolicy The IpSec policy to be added
     */
    virtual IMS_BOOL AddPolicy(IN IIpSecPolicy* piPolicy) = 0;

    /**
     * @brief Deletes IpSec policy.
     *
     * @param piPolicy The IpSec policy to be deleted
     */
    virtual void DeletePolicy(IN IIpSecPolicy* piPolicy) = 0;

    /**
     * @brief Flushes all the IpSec policies.
     */
    virtual void FlushPolicies() = 0;

    /**
     * @brief Dumps the security associations.
     *
     * @param piPolicy The IpSec policy to be dumped
     */
    virtual void DumpPolicy(IN IIpSecPolicy* piPolicy) = 0;

    /**
     * @brief Gets the IIpSecPolicy which matches with the given IpSec identifier.
     */
    virtual IIpSecPolicy* GetPolicy(IN IMS_SINT32 nId) const = 0;

    /**
     * @brief Applies an IPSec SA to the specified socket information.
     *
     * @param piSocket The socket object to be applied
     * @param objLocal The local socket address (IP & Port)
     * @param pRemote The remote socket address if the socket is TCP client socket
     *                (device initiated client socket)
     * @return Returns IMS_TRUE if the operation is successfully done.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL ApplyIpSecTransform(IN ISocket* piSocket,
            IN const SocketAddress& objLocal, IN const SocketAddress* pRemote = IMS_NULL) = 0;

    /**
     * @brief Applies an IPSec SA to the specified socket information.
     *
     * @param piSocket The socket object to be applied
     * @param piServerSocket The socket object to be found
     * @return Returns IMS_TRUE if the operation is successfully done.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL ApplyIpSecTransform(IN ISocket* piSocket,
            IN ISocket* piServerSocket) = 0;

    /**
     * @brief Removes IPSec SA from the specified socket information.
     *
     * @param nSocketId A socket identifier
     */
    virtual void RemoveIpSecTransforms(IN IMS_SINT32 nSocketId) = 0;

    /**
     * @brief Sets the flag to support SDB flush for Ims IpSec.
     *
     * If this flag is enabled, SP/SA will be tracked and those are used to delete SP/SA
     * toward the kernel when IMS process is abnormally terminated.
     *
     * @param bCapability The flush capability flag
     */
    virtual void SetSdbFlushCapability(IN IMS_BOOL bCapability) = 0;
};

#endif
