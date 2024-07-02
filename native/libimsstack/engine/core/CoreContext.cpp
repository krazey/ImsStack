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
#include "CoreContext.h"
#include "ImsCoreProtocol.h"

PUBLIC GLOBAL CoreContext* CoreContext::s_pContext = IMS_NULL;

PRIVATE CoreContext::CoreContext() :
        m_pImsCoreProtocol(new ImsCoreProtocol()),
        m_piCoreContext(IMS_NULL)
{
}

PRIVATE VIRTUAL CoreContext::~CoreContext()
{
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
