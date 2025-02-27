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

#ifndef INTERFACE_MTS_SERVICE_STATE_H_
#define INTERFACE_MTS_SERVICE_STATE_H_

#include "ImsTypeDef.h"

class IImsAos;

class IMtsServiceState
{
public:
    virtual ~IMtsServiceState() {}

    /**
     * @brief Initializes the service state.
     *
     * @param piImsAos A pointer to the IImsAos interface.
     */
    virtual void Init(IN IImsAos* piImsAos) = 0;

    /**
     * @brief Gets the current state of the service.
     *
     * @return The current state of the service. (e.g. MtsDef::STATE_INIT)
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Indicate that the service is connected.
     */
    virtual void OnImsConnected() = 0;

    /**
     * @brief Indicate that the service is disconnected with reason.
     *
     * @param nReason Indicate the reason that results that the service is disconnected.
     *                (e.g. ImsAosReason::DATA_DISCONNECTED)
     */
    virtual void OnImsDisconnected(IN IMS_UINT32 nReason) = 0;

    /**
     * @brief Indicate that the service is disconnected with reason.
     *
     * @param nReason Indicate the reason that results that the service is disconnected.
     *                (e.g. ImsAosReason::DATA_DISCONNECTED)
     */
    virtual void OnImsDisconnecting(IN IMS_UINT32 nReason) = 0;

    /**
     * @brief Indicate that the service is suspended with reason like out-of-servce
     *        even though registration is kept.
     *
     * @param nReason Indicate the reason that results that the service is suspended.
     *                (e.g. ImsAosReason::DATA_DISCONNECTED)
     */
    virtual void OnImsSuspended(IN IMS_UINT32 nReason) = 0;

    /**
     * @brief Indicate that the service is resumed without the suspended reasons
     */
    virtual void OnImsResumed() = 0;

    /**
     * @brief Sets the IMS registration connection status.
     *
     * @param bConnected {@code IMS_TRUE} If the IMS registration is connected,
     *                   {@code IMS_FALSE} otherwise.
     */
    virtual void SetImsRegConnected(IN IMS_BOOL bConnected) = 0;

    /**
     * @brief Gets the IMS registration connection status.
     *
     * @return {@code IMS_TRUE} If the IMS registration is connected,
     *         {@code IMS_FALSE} otherwise.
     */
    virtual IMS_BOOL GetImsRegConnected() = 0;

    /**
     * @brief Checks if the MO service is currently blocked.
     *
     * @return {@code IMS_TRUE} If the MO service is blocked,
     *         {@code IMS_FALSE} otherwise.
     */
    virtual IMS_BOOL IsMoServiceBlocked() const = 0;

    /**
     * @brief Checks if the MT service is currently blocked.
     *
     * @return {@code IMS_TRUE} If the MT service is blocked,
     *         {@code IMS_FALSE} otherwise.
     */
    virtual IMS_BOOL IsMtServiceBlocked() const = 0;
};

#endif
