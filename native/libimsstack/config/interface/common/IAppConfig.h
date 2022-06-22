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
#ifndef INTERFACE_APP_CONFIG_H_
#define INTERFACE_APP_CONFIG_H_

#include "ICoreServiceConfig.h"
#include "ImsRegistry.h"

class IAppConfig
{
public:
    /**
     * @brief Returns the application id that this configuration was created with.
     *
     * @return The application id of this configuration.
     */
    virtual const AString& GetAppId() const = 0;

    /**
     * @brief Returns the Core Service configuration from the AppConfig.
     *
     * @param strServiceId The service id to be found
     * @return An ICoreServiceConfig instance or null if not present.
     */
    virtual const ICoreServiceConfig* GetCoreServiceConfig(
            IN const AString& strServiceId) const = 0;

    /**
     * @brief Returns the IMS registry format from the AppConfig.
     *
     * @return An ImsRegistry instance of this configuration.
     */
    virtual ImsRegistry* ToRegistry() const = 0;
};

#endif
