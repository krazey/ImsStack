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

PUBLIC GLOBAL ServiceContext* ServiceContext::s_pContext = IMS_NULL;

PRIVATE ServiceContext::ServiceContext() :
        m_pConfiguration(new Configuration()),
        m_pServiceManager(IMS_NULL),
        m_piServiceContext(IMS_NULL)
{
}

PRIVATE VIRTUAL ServiceContext::~ServiceContext()
{
    if (m_pServiceManager != IMS_NULL)
    {
        delete m_pServiceManager;
    }

    delete m_pConfiguration;
}

PUBLIC VIRTUAL IConfiguration* ServiceContext::GetConfiguration() const
{
    if (m_piServiceContext != IMS_NULL)
    {
        return m_piServiceContext->GetConfiguration();
    }

    return m_pConfiguration;
}

PUBLIC VIRTUAL IServiceManager* ServiceContext::GetServiceManager()
{
    if (m_piServiceContext != IMS_NULL)
    {
        return m_piServiceContext->GetServiceManager();
    }

    if (m_pServiceManager == IMS_NULL)
    {
        m_pServiceManager = new ServiceManager();
    }

    return m_pServiceManager;
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
