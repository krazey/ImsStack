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

#ifndef INTERFACE_MTC_EMERGENCY_SERVICE_MANAGER_H_
#define INTERFACE_MTC_EMERGENCY_SERVICE_MANAGER_H_

#include "ImsTypeDef.h"

/**
 * This class controls the emergency service before starting emergency calls for various service
 * types.
 * And notifies the service state to the Java layer.
 */
class IMtcEmergencyServiceManager
{
public:
    virtual ~IMtcEmergencyServiceManager(){};

    /**
     * Starts the process for an emergency service opening using the given service type.
     *
     * @param eServiceType Service type to open.
     */
    virtual void StartOpen(IN ServiceType eServiceType) = 0;

    /**
     * Stops the ongoing process and releases the resources.
     *
     * @param bClose Close the emergency service if true.
     */
    virtual void StopOpen(IN IMS_BOOL bClose) = 0;
};

/**
 * Controls emergency service per the device state.
 */
class IEmergencyServiceController
{
public:
    virtual ~IEmergencyServiceController(){};

    /**
     * Triggers an emergency service start.
     */
    virtual void Start() = 0;

    /**
     * Requests to close the opened emergency service.
     */
    virtual void Close() = 0;

    /**
     * Returns service type of the controller using.
     *
     * @return ServiceType Service type.
     */
    virtual ServiceType GetServiceType() const = 0;
};

#endif
