/*
 * Copyright (C) 2024 The Android Open Source Project
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
#ifndef INTERFACE_SERVICE_MANAGER_H_
#define INTERFACE_SERVICE_MANAGER_H_

#include "AString.h"
#include "ImsList.h"

#include "IServiceCloseListener.h"

class Service;

/**
 * @brief An interface for managing the IMS services.
 */
class IServiceManager : public IServiceCloseListener
{
protected:
    ~IServiceManager() override = default;

public:
    /**
     * @brief Adds the Service instance to the managed pool.
     *
     * @param pService A Service instance
     * @return IMS_TRUE if the service is successfully added, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL AttachService(IN Service* pService) = 0;

    /**
     * @brief Removes the Service instance from the managed pool.
     *
     * @param pService A Service instance.
     */
    virtual void DetachService(IN Service* pService) = 0;

    /**
     * @brief Returns the Service instance that matches the specified slot and service identifier.
     *
     * @param nSlotId A slot id
     * @param strAppId An application identifier
     * @param strServiceId A service identifier
     * @return A Service instance if present, IMS_NULL otherwise.
     */
    virtual Service* GetService(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId) const = 0;

    /**
     * @brief Returns all Service instances.
     *
     * @return List of Service instances.
     */
    virtual const ImsList<Service*>& GetServices() const = 0;

    /**
     * @brief Returns all Service instances that match the specified slot.
     *
     * @param nSlotId A slot id
     * @return List of Service instances.
     */
    virtual ImsList<Service*> GetServices(IN IMS_SINT32 nSlotId) const = 0;
};

#endif
