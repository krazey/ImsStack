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
#include "ServiceMemory.h"

#include "CallControlHelper.h"
#include "Configuration.h"
#include "ImsCoreContext.h"
#include "ImsCoreProtocol.h"
#include "RegInfoManager.h"
#include "RegistrationManager.h"
#include "ServiceManager.h"
#include "util/CallerPreferenceManager.h"
#include "util/SipConnectionNotifierManager.h"

PUBLIC GLOBAL ImsCoreContext* ImsCoreContext::s_pContext = IMS_NULL;

PRIVATE ImsCoreContext::ImsCoreContext() :
        m_pConfiguration(new Configuration()),
        m_pServiceManager(IMS_NULL),
        m_pRegistrationManager(IMS_NULL),
        m_pRegInfoManager(IMS_NULL),
        m_pImsCoreProtocol(new ImsCoreProtocol()),
        m_pCallControlHelper(IMS_NULL),
        m_pCallerPreferenceManager(IMS_NULL),
        m_pScnManager(IMS_NULL),
        m_piImsCoreContext(IMS_NULL)
{
}

PRIVATE VIRTUAL ImsCoreContext::~ImsCoreContext()
{
    if (m_pCallerPreferenceManager != IMS_NULL)
    {
        delete m_pCallerPreferenceManager;
    }

    if (m_pCallControlHelper != IMS_NULL)
    {
        delete m_pCallControlHelper;
    }

    if (m_pScnManager != IMS_NULL)
    {
        delete m_pScnManager;
    }

    delete m_pImsCoreProtocol;

    if (m_pRegInfoManager != IMS_NULL)
    {
        delete m_pRegInfoManager;
    }

    if (m_pRegistrationManager != IMS_NULL)
    {
        delete m_pRegistrationManager;
    }

    if (m_pServiceManager != IMS_NULL)
    {
        delete m_pServiceManager;
    }

    delete m_pConfiguration;
}

PUBLIC VIRTUAL IConfiguration* ImsCoreContext::GetConfiguration() const
{
    if (m_piImsCoreContext != IMS_NULL)
    {
        return m_piImsCoreContext->GetConfiguration();
    }

    return m_pConfiguration;
}

PUBLIC VIRTUAL IServiceManager* ImsCoreContext::GetServiceManager()
{
    if (m_piImsCoreContext != IMS_NULL)
    {
        return m_piImsCoreContext->GetServiceManager();
    }

    if (m_pServiceManager == IMS_NULL)
    {
        m_pServiceManager = new ServiceManager();
    }

    return m_pServiceManager;
}

PUBLIC VIRTUAL IRegistrationManager* ImsCoreContext::GetRegistrationManager()
{
    if (m_piImsCoreContext != IMS_NULL)
    {
        return m_piImsCoreContext->GetRegistrationManager();
    }

    if (m_pRegistrationManager == IMS_NULL)
    {
        m_pRegistrationManager = new RegistrationManager();
    }

    return m_pRegistrationManager;
}

PUBLIC VIRTUAL IRegInfoManager* ImsCoreContext::GetRegInfoManager()
{
    if (m_piImsCoreContext != IMS_NULL)
    {
        return m_piImsCoreContext->GetRegInfoManager();
    }

    if (m_pRegInfoManager == IMS_NULL)
    {
        m_pRegInfoManager = new RegInfoManager();
    }

    return m_pRegInfoManager;
}

PUBLIC VIRTUAL ServiceProtocol* ImsCoreContext::GetImsCoreProtocol() const
{
    if (m_piImsCoreContext != IMS_NULL)
    {
        return m_piImsCoreContext->GetImsCoreProtocol();
    }

    return m_pImsCoreProtocol;
}

PUBLIC VIRTUAL CallControlHelper* ImsCoreContext::GetCallControlHelper()
{
    if (m_piImsCoreContext != IMS_NULL)
    {
        return m_piImsCoreContext->GetCallControlHelper();
    }

    if (m_pCallControlHelper == IMS_NULL)
    {
        m_pCallControlHelper = new CallControlHelper();
    }

    return m_pCallControlHelper;
}

PUBLIC VIRTUAL CallerPreferenceManager* ImsCoreContext::GetCallerPreferenceManager()
{
    if (m_piImsCoreContext != IMS_NULL)
    {
        return m_piImsCoreContext->GetCallerPreferenceManager();
    }

    if (m_pCallerPreferenceManager == IMS_NULL)
    {
        m_pCallerPreferenceManager = new CallerPreferenceManager();
    }

    return m_pCallerPreferenceManager;
}

PUBLIC VIRTUAL ISipConnectionNotifierManager* ImsCoreContext::GetSipConnectionNotifierManager()
{
    if (m_piImsCoreContext != IMS_NULL)
    {
        return m_piImsCoreContext->GetSipConnectionNotifierManager();
    }

    if (m_pScnManager == IMS_NULL)
    {
        m_pScnManager = new SipConnectionNotifierManager();
    }

    return m_pScnManager;
}

PUBLIC GLOBAL ImsCoreContext* ImsCoreContext::GetInstance()
{
    if (s_pContext == IMS_NULL)
    {
        s_pContext = new ImsCoreContext();
    }

    return s_pContext;
}

PUBLIC GLOBAL void ImsCoreContext::DestroyInstance()
{
    if (s_pContext != IMS_NULL)
    {
        delete s_pContext;
        s_pContext = IMS_NULL;
    }
}
