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
#ifndef INTERFACE_NET_IPSEC_H_
#define INTERFACE_NET_IPSEC_H_

#include "SocketAddress.h"
#include "ISocket.h"

class INetSocket;
class IIPSecPolicy;

class INetIPSec
{
public:
    /*

    Creates IPSec Policy

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IIPSecPolicy*           Pointer to IIPSecPolicy
    </table>

    */
    virtual IIPSecPolicy* CreatePolicy() = 0;

    /*

    Destroy IPSec Policy

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>
    IIPSecPolicy*           Pointer to IIPSecPolicy

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void DestroyPolicy(IN IIPSecPolicy* piIPSecPolicy) = 0;

    /*

    Destroy all SAs that Policies have to

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void DestroyAllPolicies() = 0;

    /*

    Add IPSec SAs that policy has to IPSec Library

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>
    IIPSecPolicy*           Pointer to IIPSecPolicy

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual IMS_BOOL AddPolicy(IN IIPSecPolicy* piIPSecPolicy) = 0;

    /*

    Delete IPSec SAs that policy has to IPSec Library

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>
    IIPSecPolicy*           Pointer to IIPSecPolicy

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void DeletePolicy(IN IIPSecPolicy* piIPSecPolicy) = 0;

    /*

    Delete all SAs that Policies have to IPSEC library

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void FlushPolicies() = 0;

    /*

    Display SA Dumps

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void DumpSAs(IN IIPSecPolicy* piPolicy) = 0;

    /**
     * @brief Gets the IIPSecPolicy which matches with the given IPSec identifier.
     */
    virtual IIPSecPolicy* GetPolicy(IN IMS_SINT32 nId) const = 0;

    /**
     * @brief Applies an IPSec SA to the specified socket information.
     *
     * @param piSocket a socket object to be applied
     * @param objLocal a local socket address (IP & Port)
     * @param pRemote a remote socket address if the socket is TCP client socket(device-initiated)
     *
     * @return Returns IMS_TRUE if the operation is successfully done.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL ApplyIpSecTransform(IN INetSocket* piSocket,
            IN const SocketAddress& objLocal, IN const SocketAddress* pRemote = IMS_NULL) = 0;

    /**
     * @brief Applies an IPSec SA to the specified socket information.
     *
     * @param piSocket a socket object to be applied
     * @param piServerSocket a socket object to be found
     *
     * @return Returns IMS_TRUE if the operation is successfully done.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL ApplyIpSecTransform(IN INetSocket* piSocket,
            IN INetSocket* piServerSocket) = 0;

    /**
     * @brief Removes IPSec SA from the specified socket information.
     *
     * @param nSocketId a socket identifier
     */
    virtual void RemoveIpSecTransforms(IN IMS_SINT32 nSocketId) = 0;

    /*

    Sets the flag to support SDB flush for IMS IPSec.
    If this flag is enabled, SP/SA will be tracked and those are used to delete SP/SA
    toward the kernel when IMS process is abnormally terminated.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void SetSDBFlushCapability(IN IMS_BOOL bCapability) = 0;
};

#endif // INTERFACE_NET_IPSEC_H_
