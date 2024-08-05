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
#include "CallControlHelper.h"
#include "CoreContext.h"
#include "ImsCoreProtocol.h"
#include "util/CallerPreferenceManager.h"

PUBLIC GLOBAL CoreContext* CoreContext::s_pContext = IMS_NULL;

PRIVATE CoreContext::CoreContext() :
        m_pImsCoreProtocol(new ImsCoreProtocol()),
        m_pCallControlHelper(IMS_NULL),
        m_pCallerPreferenceManager(IMS_NULL),
        m_piCoreContext(IMS_NULL)
{
}

PRIVATE VIRTUAL CoreContext::~CoreContext()
{
    if (m_pCallerPreferenceManager != IMS_NULL)
    {
        delete m_pCallerPreferenceManager;
    }

    if (m_pCallControlHelper != IMS_NULL)
    {
        delete m_pCallControlHelper;
    }

    delete m_pImsCoreProtocol;
}

PUBLIC ServiceProtocol* CoreContext::GetImsCoreProtocol() const
{
    if (m_piCoreContext != IMS_NULL)
    {
        return m_piCoreContext->GetImsCoreProtocol();
    }

    return m_pImsCoreProtocol;
}

PUBLIC CallControlHelper* CoreContext::GetCallControlHelper()
{
    if (m_piCoreContext != IMS_NULL)
    {
        return m_piCoreContext->GetCallControlHelper();
    }

    if (m_pCallControlHelper == IMS_NULL)
    {
        m_pCallControlHelper = new CallControlHelper();
    }

    return m_pCallControlHelper;
}

PUBLIC CallerPreferenceManager* CoreContext::GetCallerPreferenceManager()
{
    if (m_piCoreContext != IMS_NULL)
    {
        return m_piCoreContext->GetCallerPreferenceManager();
    }

    if (m_pCallerPreferenceManager == IMS_NULL)
    {
        m_pCallerPreferenceManager = new CallerPreferenceManager();
    }

    return m_pCallerPreferenceManager;
}

PUBLIC GLOBAL CoreContext* CoreContext::GetInstance()
{
    if (s_pContext == IMS_NULL)
    {
        s_pContext = new CoreContext();
    }

    return s_pContext;
}

PUBLIC GLOBAL void CoreContext::DestroyInstance()
{
    if (s_pContext != IMS_NULL)
    {
        delete s_pContext;
        s_pContext = IMS_NULL;
    }
}
