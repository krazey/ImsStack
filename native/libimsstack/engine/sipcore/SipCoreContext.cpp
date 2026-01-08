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
#include "SipCoreContext.h"
#include "SipProtocol.h"

PUBLIC GLOBAL SipCoreContext* SipCoreContext::s_pContext = IMS_NULL;

PRIVATE SipCoreContext::SipCoreContext() :
        m_pSipProtocol(new SipProtocol()),
        m_piSipCoreContext(IMS_NULL)
{
}

PRIVATE VIRTUAL SipCoreContext::~SipCoreContext()
{
    delete m_pSipProtocol;
}

PUBLIC Protocol* SipCoreContext::GetSipProtocol() const
{
    if (m_piSipCoreContext != IMS_NULL)
    {
        return m_piSipCoreContext->GetSipProtocol();
    }

    return m_pSipProtocol;
}

PUBLIC GLOBAL SipCoreContext* SipCoreContext::GetInstance()
{
    if (s_pContext == IMS_NULL)
    {
        s_pContext = new SipCoreContext();
    }

    return s_pContext;
}

PUBLIC GLOBAL void SipCoreContext::DestroyInstance()
{
    if (s_pContext != IMS_NULL)
    {
        delete s_pContext;
        s_pContext = IMS_NULL;
    }
}
