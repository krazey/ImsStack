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
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"

#include "Service.h"
#include "ServiceManager.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
ServiceManager::ServiceManager() :
        m_piLock(IMS_NULL),
        m_objServices(ImsList<Service*>())
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC VIRTUAL ServiceManager::~ServiceManager()
{
    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC VIRTUAL IMS_BOOL ServiceManager::AttachService(IN Service* pService)
{
    if (pService == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Add the service to the pending service storage.
    // The registration will be triggered by the application after opening the service.

    LockGuard objLock(m_piLock);

    if (!m_objServices.Append(pService))
    {
        IMS_TRACE_E(0, "Appending a Service (%s, %d) failed", pService->GetAppId().GetStr(),
                pService->GetSlotId(), 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void ServiceManager::DetachService(IN Service* pService)
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objServices.GetSize(); ++i)
    {
        const Service* pTmpService = m_objServices.GetAt(i);

        if (pTmpService->Equals(pService))
        {
            m_objServices.RemoveAt(i);
            break;
        }
    }

    if (m_objServices.IsEmpty())
    {
        IMS_TRACE_I("ServiceManager :: No services", 0, 0, 0);
    }
}

PUBLIC VIRTUAL Service* ServiceManager::GetService(
        IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId) const
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objServices.GetSize(); ++i)
    {
        Service* pService = m_objServices.GetAt(i);

        if ((pService->GetSlotId() == nSlotId) && strAppId.Equals(pService->GetAppId()) &&
                strServiceId.Equals(pService->GetServiceId()))
        {
            return pService;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL ImsList<Service*> ServiceManager::GetServices(IN IMS_SINT32 nSlotId) const
{
    ImsList<Service*> objServicesOnSlot;
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objServices.GetSize(); ++i)
    {
        Service* pService = m_objServices.GetAt(i);

        if (pService->GetSlotId() == nSlotId)
        {
            objServicesOnSlot.Append(pService);
        }
    }

    return objServicesOnSlot;
}
