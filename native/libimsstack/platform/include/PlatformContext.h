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
#ifndef PLATFORM_CONTEXT_H_
#define PLATFORM_CONTEXT_H_

#include "IOsFactory.h"
#include "ISystem.h"
#include "ImsMap.h"
#include "PlatformService.h"

class OsFactory;

class PlatformContext
{
public:
    /// Defines the service types that are supported by the platform layer.
    /// Do not re-align the order.
    enum PlatformServiceType
    {
        SERVICE_NONE = 0,
        SERVICE_MUTEX,
        SERVICE_TRACE,
        SERVICE_FILE,
        SERVICE_SYSTEM_TIME,
        SERVICE_PHONE_INFO,
        SERVICE_UTIL,
        SERVICE_CONFIG,
        SERVICE_TIMER,
        SERVICE_THREAD,
        SERVICE_EVENT,
        SERVICE_NETWORK,
        SERVICE_NETWORK_POLICY,
        SERVICE_MAX
    };

private:
    PlatformContext();
    ~PlatformContext();

public:
    /**
     * @brief Returns the platform service that matches with the given service type.
     *
     * If there is a specific service instance, it will be returned instead of a default one.
     */
    PlatformService* GetService(IN PlatformServiceType eType) const;
    /**
     * @brief Sets the platform service that matches with the given service type.
     *
     * The old platform service will be returned after setting new one.
     * The memory of the returned instance should be handled by the caller.
     */
    PlatformService* SetService(IN PlatformServiceType eType, IN PlatformService* pService);

    /**
     * @brief Returns the factory instance to create and destroy the OS dependent components.
     *
     * If there is a specific factory instance, it will be returned instead of a default one.
     */
    IOsFactory* GetOsFactory() const;
    /**
     * @brief Sets a specific factory instance to create and destroy the OS dependent components.
     *
     * The old factory instance will be returned after setting new one.
     * The memory of the returned instance should be handled by the caller.
     */
    IOsFactory* SetOsFactory(IN IOsFactory* piOsFactory);

    /**
     * @brief Returns the default system instance.
     *
     * If there is a specific system instance, it will be returned instead of a default one.
     */
    ISystem* GetSystem() const;
    /**
     * @brief Sets a specific system instance.
     *
     * The old system instance will be returned after setting new one.
     * The memory of the returned instance should be handled by the caller.
     */
    ISystem* SetSystem(IN ISystem* piSystem);

    static PlatformContext* GetInstance();
    static void DestroyInstance();

private:
    PlatformService* GetDefaultService(IN PlatformServiceType eType) const;

private:
    // Default OS factory will be created initially and maintained it permanently.
    // It's supporting now for backward compatibility.
    IOsFactory* m_piDefaultOsFactory;
    // If the caller sets a specific OS factory, then it will be used first
    // instead of the default OS factory.
    IOsFactory* m_piOsFactory;

    // Default system instance will be created initially and maintained it permanently.
    // It's supporting now for backward compatibility.
    mutable ISystem* m_piDefaultSystem;
    // If the caller sets a specific system instance, then it will be used first
    // instead of the default system instance.
    ISystem* m_piSystem;

    // Default platform services: always created and maintained permanently.
    // It's supporting now for backward compatibility.
    mutable ImsMap<PlatformServiceType, PlatformService*> m_objDefaultServices;
    // If the caller sets any platform service, it will be used first
    // instead of the corresponding default service.
    ImsMap<PlatformServiceType, PlatformService*> m_objServices;

    static PlatformContext* s_pContext;
};

#endif
