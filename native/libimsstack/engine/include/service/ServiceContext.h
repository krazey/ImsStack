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
#ifndef SERVICE_CONTEXT_H_
#define SERVICE_CONTEXT_H_

#include "IServiceContext.h"

class Configuration;
class ServiceManager;

/**
 * A context class for providing the singleton instances for service layer.
 */
class ServiceContext : public IServiceContext
{
private:
    ServiceContext();
    virtual ~ServiceContext();

public:
    ServiceContext(IN const ServiceContext&) = delete;
    ServiceContext& operator=(IN const ServiceContext&) = delete;

public:
    IConfiguration* GetConfiguration() const override;
    IServiceManager* GetServiceManager() override;

    /**
     * @brief Sets the specific service context to return their own instances.
     */
    inline void SetServiceContext(IN IServiceContext* piServiceContext)
    {
        m_piServiceContext = piServiceContext;
    }

    /**
     * @brief Returns a singleton instance of this class.
     */
    static ServiceContext* GetInstance();
    /**
     * @brief Destroys a singleton instance of this class.
     */
    static void DestroyInstance();

private:
    Configuration* m_pConfiguration;
    ServiceManager* m_pServiceManager;
    IServiceContext* m_piServiceContext;

    static ServiceContext* s_pContext;
};

#endif
