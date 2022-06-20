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
#include "ImsApp.h"
#include "ServiceMemory.h"

PUBLIC
ImsApp::ImsApp(IN const AString& strName) :
        ImsActivity(strName),
        m_objServices(IMSList<ImsService*>())
{
}

PUBLIC VIRTUAL ImsApp::~ImsApp()
{
    while (!m_objServices.IsEmpty())
    {
        ImsService* pService = m_objServices.GetAt(0);

        if (pService != IMS_NULL)
        {
            delete pService;
        }

        m_objServices.RemoveAt(0);
    }
}

PUBLIC
IMS_BOOL ImsApp::AttachService(IN ImsService* pService)
{
    if (pService == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_objServices.Append(pService);
}

PUBLIC
void ImsApp::DetachService(IN ImsService* pService)
{
    if (pService == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objServices.GetSize(); i++)
    {
        ImsService* pTempService = m_objServices.GetAt(i);

        if (pTempService == pService)
        {
            m_objServices.RemoveAt(i);
            break;
        }
    }
}

PUBLIC
ImsService* ImsApp::GetService(IN const AString& strServiceName)
{
    if (strServiceName.IsNULL())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objServices.GetSize(); i++)
    {
        ImsService* pService = m_objServices.GetAt(i);

        if (pService->GetName().Equals(strServiceName))
        {
            return pService;
        }
    }

    return IMS_NULL;
}

PRIVATE VIRTUAL IMS_BOOL ImsApp::DispatchMessage(IN ImsMessage& objMsg)
{
    IMS_BOOL bResult = IMS_FALSE;

    (void)OnPreprocess(objMsg);
    bResult = OnMessage(objMsg);
    (void)OnPostprocess(objMsg);

    return bResult;
}
