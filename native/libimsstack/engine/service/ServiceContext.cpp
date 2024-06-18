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

#include "Configuration.h"
#include "ServiceContext.h"
#include "ServiceManager.h"

class ServiceContextPrivate : public IServiceContext
{
public:
    inline ServiceContextPrivate() :
            m_pConfiguration(new Configuration()),
            m_pServiceManager(new ServiceManager())
    {
    }
    inline virtual ~ServiceContextPrivate()
    {
        delete m_pConfiguration;
        delete m_pServiceManager;
    }

public:
    ServiceContextPrivate(IN const ServiceContextPrivate&) = delete;
    ServiceContextPrivate& operator=(IN const ServiceContextPrivate&) = delete;

public:
    inline Configuration* GetConfiguration() const override { return m_pConfiguration; }
    inline ServiceManager* GetServiceManager() const override { return m_pServiceManager; }

private:
    Configuration* m_pConfiguration;
    ServiceManager* m_pServiceManager;
};

PUBLIC GLOBAL ServiceContext* ServiceContext::s_pContext = IMS_NULL;

PRIVATE ServiceContext::ServiceContext() :
        m_pPrivate(new ServiceContextPrivate()),
        m_piServiceContext(IMS_NULL)
{
}

PRIVATE VIRTUAL ServiceContext::~ServiceContext()
{
    delete m_pPrivate;
}

PUBLIC VIRTUAL IConfiguration* ServiceContext::GetConfiguration() const
{
    if (m_piServiceContext != IMS_NULL)
    {
        return m_piServiceContext->GetConfiguration();
    }

    return m_pPrivate->GetConfiguration();
}

PUBLIC VIRTUAL IServiceManager* ServiceContext::GetServiceManager() const
{
    if (m_piServiceContext != IMS_NULL)
    {
        return m_piServiceContext->GetServiceManager();
    }

    return m_pPrivate->GetServiceManager();
}

PUBLIC GLOBAL ServiceContext* ServiceContext::GetInstance()
{
    if (s_pContext == IMS_NULL)
    {
        s_pContext = new ServiceContext();
    }

    return s_pContext;
}

PUBLIC GLOBAL void ServiceContext::DestroyInstance()
{
    if (s_pContext != IMS_NULL)
    {
        delete s_pContext;
        s_pContext = IMS_NULL;
    }
}
