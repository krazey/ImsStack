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
#ifndef INTERFACE_IPSEC_POLICY_H_
#define INTERFACE_IPSEC_POLICY_H_

#include "ImsList.h"

class IIpSecSa;
class IIpSecSp;
class IIpSecPolicyListener;

class IIpSecPolicy
{
protected:
    virtual ~IIpSecPolicy() = default;

public:
    /**
     * @brief Returns an Identifier of this IIPSecPolicy.
     *
     * @return An identifier of this IpSec policy.
     */
    virtual IMS_SINT32 GetId() const = 0;

    /**
     * @brief Creates SP configuration.
     *
     * @return A pointer of IIPSecSP.
     */
    virtual IIpSecSp* CreateSp() = 0;

    /**
     * @brief Destroys SP configuration.
     *
     * @param piSP The pointer of IIPSecSP
     */
    virtual void DestroySp(IN IIpSecSp* piSp) = 0;

    /**
     * @brief Creates SA configuration.
     *
     * @return A pointer of IIPSecSA.
     */
    virtual IIpSecSa* CreateSa() = 0;

    /**
     * @brief Destroys SA configuration.
     *
     * @param piSA The pointer of IIPSecSA
     */
    virtual void DestroySa(IN IIpSecSa* piSa) = 0;

    /**
     * @brief Manages the lifetime of this security association.
     *
     * @param nDuration The duration of this security association
     */
    virtual void ManageLifetime(IN IMS_UINT32 nDuration) = 0;

    /**
     * @brief Sets the listener for IpSec policy.
     *
     * @param piListener The listener to be set
     */
    virtual void SetListener(IN IIpSecPolicyListener* piListener) = 0;
};

#endif
