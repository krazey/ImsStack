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
#ifndef IMS_SERVICE_CONFIG_H_
#define IMS_SERVICE_CONFIG_H_

#include "IMSList.h"

#include "ImsServiceConfigTypeDef.h"

class ImsServiceConfig
{
public:
    ImsServiceConfig() = delete;

public:
    /**
     * @brief Gets an application identifier from the specified service identifier.
     *
     * @param eServiceId A service identifier
     * @return An application identifier.
     */
    static ImsAppId GetAppId(IN ImsServiceId eServiceId);

    /**
     * @brief Gets an application name from the specified application identifier.
     *
     * @param eAppId An application identifier
     * @return An application name as a string.
     */
    static AString GetAppName(IN ImsAppId eAppId);

    /**
     * @brief Gets a service name from the specified service identifier.
     *
     * @param eServiceId A service identifier
     * @return A service name as a string.
     */
    static AString GetServiceName(IN ImsServiceId eServiceId);

    /**
     * @brief Gets an application name from the specified application identifier.
     *
     * @param eAppId An application identifier
     * @return An application name as a null-terminated string.
     */
    static const IMS_CHAR* GetAppNameC(IN ImsAppId eAppId);

    /**
     * @brief Gets a service name from the specified service identifier.
     *
     * @param eServiceId A service identifier
     * @return A service name as a null-terminated string.
     */
    static const IMS_CHAR* GetServiceNameC(IN ImsServiceId eServiceId);

    /**
     * @brief Returns the IMS service profile for a normal service.
     *
     * @return An IMS service profile for a normal service.
     */
    static ImsServiceProfile GetServiceProfile();

    /**
     * @brief Returns the IMS service profile for an emergency service.
     *
     * @return An IMS service profile for an emergency service.
     */
    static ImsServiceProfile GetEmergencyServiceProfile();

    /**
     * @brief Converts the IMS service identifiers to those string names.
     *
     * @param objProfile The IMS service profile
     * @return List of the service name from the specified IMS service profile.
     */
    static IMSList<ImsServiceName> GetServiceNames(IN const ImsServiceProfile& objProfile);
};

#endif
