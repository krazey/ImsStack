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

__IMS_TRACE_TAG_IMS__;

class ServiceManagerPrivate : public EngineActivity
{
public:
    ServiceManagerPrivate();
    virtual ~ServiceManagerPrivate();

    ServiceManagerPrivate(IN const ServiceManagerPrivate&) = delete;
    ServiceManagerPrivate& operator=(IN const ServiceManagerPrivate&) = delete;

public:
    IMS_BOOL AttachService(IN Service* pService);
    void DetachService(IN Service* pService);
    Service* GetService(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId) const;
    void GetServices(IN IMS_SINT32 nSlotId, OUT ImsList<Service*>& objServicesOnSlot) const;

private:
    // EngineActivity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

private:
    friend class ServiceManager;

    IMutex* m_piLock;
    ImsList<Service*> m_objServices;
};

PUBLIC
ServiceManagerPrivate::ServiceManagerPrivate() :
        EngineActivity(),
        m_piLock(IMS_NULL)
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC
ServiceManagerPrivate::~ServiceManagerPrivate()
{
    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
IMS_BOOL ServiceManagerPrivate::AttachService(IN Service* pService)
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

PUBLIC
void ServiceManagerPrivate::DetachService(IN Service* pService)
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objServices.GetSize(); ++i)
    {
        Service* pTmpService = m_objServices.GetAt(i);

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

PUBLIC
Service* ServiceManagerPrivate::GetService(
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

PUBLIC
void ServiceManagerPrivate::GetServices(
        IN IMS_SINT32 nSlotId, OUT ImsList<Service*>& objServicesOnSlot) const
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objServices.GetSize(); ++i)
    {
        Service* pService = m_objServices.GetAt(i);

        if (pService->GetSlotId() == nSlotId)
        {
            objServicesOnSlot.Append(pService);
        }
    }
}

PRIVATE VIRTUAL IMS_BOOL ServiceManagerPrivate::DispatchMessage(IN ImsMessage& objMsg)
{
    return EngineActivity::DispatchMessage(objMsg);
}

PUBLIC
ServiceManager::ServiceManager() :
        m_pServiceMngrPrivate(new ServiceManagerPrivate())
{
}

PUBLIC VIRTUAL ServiceManager::~ServiceManager()
{
    if (m_pServiceMngrPrivate != IMS_NULL)
    {
        delete m_pServiceMngrPrivate;
    }
}

PUBLIC
IMS_BOOL ServiceManager::AttachService(IN Service* pService)
{
    return m_pServiceMngrPrivate->AttachService(pService);
}

PUBLIC
void ServiceManager::DetachService(IN Service* pService)
{
    m_pServiceMngrPrivate->DetachService(pService);
}

PUBLIC
Service* ServiceManager::GetService(
        IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId) const
{
    return m_pServiceMngrPrivate->GetService(nSlotId, strAppId, strServiceId);
}

PUBLIC
const ImsList<Service*>& ServiceManager::GetServices() const
{
    return m_pServiceMngrPrivate->m_objServices;
}

PUBLIC
ImsList<Service*> ServiceManager::GetServices(IN IMS_SINT32 nSlotId) const
{
    ImsList<Service*> objServices;

    m_pServiceMngrPrivate->GetServices(nSlotId, objServices);

    return objServices;
}

PUBLIC GLOBAL ServiceManager* ServiceManager::GetInstance()
{
    static ServiceManager* s_pServiceManager = IMS_NULL;

    if (s_pServiceManager == IMS_NULL)
    {
        s_pServiceManager = new ServiceManager();
    }

    return s_pServiceManager;
}

PRIVATE VIRTUAL void ServiceManager::ServiceClosed(IN Service* pService)
{
    DetachService(pService);
}
