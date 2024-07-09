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

#include "RegInfoManager.h"
#include "RegistrationContext.h"
#include "RegistrationManager.h"
#include "util/SipConnectionNotifierManager.h"

PUBLIC GLOBAL RegistrationContext* RegistrationContext::s_pContext = IMS_NULL;

PRIVATE RegistrationContext::RegistrationContext() :
        m_pRegistrationManager(IMS_NULL),
        m_pRegInfoManager(IMS_NULL),
        m_pScnManager(IMS_NULL),
        m_piRegistrationContext(IMS_NULL)
{
}

PRIVATE VIRTUAL RegistrationContext::~RegistrationContext()
{
    if (m_pRegInfoManager != IMS_NULL)
    {
        delete m_pRegInfoManager;
    }

    if (m_pRegistrationManager != IMS_NULL)
    {
        delete m_pRegistrationManager;
    }

    if (m_pScnManager != IMS_NULL)
    {
        delete m_pScnManager;
    }
}

PUBLIC VIRTUAL IRegistrationManager* RegistrationContext::GetRegistrationManager()
{
    if (m_piRegistrationContext != IMS_NULL)
    {
        return m_piRegistrationContext->GetRegistrationManager();
    }

    if (m_pRegistrationManager == IMS_NULL)
    {
        m_pRegistrationManager = new RegistrationManager();
    }

    return m_pRegistrationManager;
}

PUBLIC VIRTUAL IRegInfoManager* RegistrationContext::GetRegInfoManager()
{
    if (m_piRegistrationContext != IMS_NULL)
    {
        return m_piRegistrationContext->GetRegInfoManager();
    }

    if (m_pRegInfoManager == IMS_NULL)
    {
        m_pRegInfoManager = new RegInfoManager();
    }

    return m_pRegInfoManager;
}

PUBLIC ISipConnectionNotifierManager* RegistrationContext::GetSipConnectionNotifierManager()
{
    if (m_piRegistrationContext != IMS_NULL)
    {
        return m_piRegistrationContext->GetSipConnectionNotifierManager();
    }

    if (m_pScnManager == IMS_NULL)
    {
        m_pScnManager = new SipConnectionNotifierManager();
    }

    return m_pScnManager;
}

PUBLIC GLOBAL RegistrationContext* RegistrationContext::GetInstance()
{
    if (s_pContext == IMS_NULL)
    {
        s_pContext = new RegistrationContext();
    }

    return s_pContext;
}

PUBLIC GLOBAL void RegistrationContext::DestroyInstance()
{
    if (s_pContext != IMS_NULL)
    {
        delete s_pContext;
        s_pContext = IMS_NULL;
    }
}
