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
#ifndef INTERFACE_SIP_IPSEC_STATE_H_
#define INTERFACE_SIP_IPSEC_STATE_H_

#include "IPAddress.h"

class ISipIpSecStateListener;

/**
 * @brief This class provides an interface to handle IpSec SA(Security Association) state.
 */
class ISipIpSecState
{
public:
    /**
     * @brief Clears the existing IpSec security association.
     *
     * After de-registration, the application SHALL invoke this method and clear all the SAs.
     *
     * @param nSaType A security association type to be cleared\n
     *                #SA_NEW\n
     *                #SA_OLD
     */
    virtual void ClearIpSecSa(IN IMS_SINT32 nSaType) = 0;

    /**
     * @brief Returns the current state of the specified IpSec security association.
     *
     * @param nSaType A security association type to be returned\n
     *                #SA_NEW\n
     *                #SA_OLD
     * @return A current state of the specified IPSec type.\n
     *         #STATE_INACTIVE\n
     *         #STATE_CREATED\n
     *         #STATE_PENDING\n
     *         #STATE_ACTIVE\n
     *         #STATE_TERMINATED\n
     *         #STATE_TERMINATED_PENDING
     */
    virtual IMS_SINT32 GetState(IN IMS_SINT32 nSaType) const = 0;

    /**
     * @brief Checks if the pending transaction exists or not.
     *
     * @param nSaType A security association type to be checked\n
     *                #SA_NEW\n
     *                #SA_OLD
     * @return If the pending transaction exists, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL HasPendingTransaction(IN IMS_SINT32 nSaType) const = 0;

    /**
     * @brief Sets the IpSec SA table for the specified security association.
     *
     * It internally makes the SA table for the composition of each direction/transport.\n
     * When setting the new SA, it will internally copy the existing new SA to the old SA.
     *
     * @param nSaType A security association type to be set\n
     *                #SA_NEW\n
     *                #SA_OLD
     * @param objIpAddrU UE's IP address
     * @param nPortUc UE's client port
     * @param nPortUs UE's server port
     * @param objIpAddrP P-CSCF's IP address
     * @param nPortPc P-CSCF's client port
     * @param nPortPs P-CSCF's server port
     */
    virtual void SetIpSecSa(IN IMS_SINT32 nSaType, IN const IPAddress& objIpAddrU,
            IN IMS_SINT32 nPortUc, IN IMS_SINT32 nPortUs, IN const IPAddress& objIpAddrP,
            IN IMS_SINT32 nPortPc, IN IMS_SINT32 nPortPs) = 0;

    /**
     * @brief Sets the listener to monitor the state of the IPSec security association.
     *
     * @param piListener Listener to be set
     */
    virtual void SetListener(IN ISipIpSecStateListener* piListener) = 0;

public:
    /// Type of IPSec security association
    enum
    {
        /// New IPSec security association
        SA_NEW = 1,
        /// Old IPSec security association
        SA_OLD = 2
    };

    /// State of IPSec security association
    enum
    {
        /// New/Old SA (default state)
        STATE_INACTIVE = 0,
        /// New SA
        /// - When SA is newly added
        /// - After the initial transaction is failed by transaction timeout
        STATE_CREATED = 1,
        /// New SA
        /// - When the initial transaction is ongoing
        STATE_PENDING = 2,
        /// New/Old SA
        /// - When one transaction is detected on new SA
        /// - When outgoing response is sent on new SA in CREATED state
        /// - When old SA has a pending transaction
        STATE_ACTIVE = 3,
        /// Old SA only
        /// - When old SA has no pending transaction and new SA is transited to active state
        STATE_TERMINATED = 4,
        /// Old SA only
        /// - When old SA is in TERMINATED state and SA lifetime is not expired
        ///  and old SA detects a pending transaction
        STATE_TERMINATED_PENDING = 5
    };
};

#endif
