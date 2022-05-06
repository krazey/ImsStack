/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090608  toastops@                 Created
    </table>

    Description
     Service is the base class for IMS services, and follows the Generic Connection Framework
    (GCF).
*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#include "Service.h"
#include "ServiceManager.h"

__IMS_TRACE_TAG_IMS__;

class ServiceManagerPrivate : public EngineActivity
{
public:
    ServiceManagerPrivate();
    virtual ~ServiceManagerPrivate();

private:
    ServiceManagerPrivate(IN const ServiceManagerPrivate& objRHS);
    ServiceManagerPrivate& operator=(IN const ServiceManagerPrivate& objRHS);

public:
    IMS_BOOL AttachService(IN Service* pService);
    void DetachService(IN Service* pService);
    Service* GetService(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId) const;
    void GetServices(IN IMS_SINT32 nSlotId, OUT IMSList<Service*>& objServicesOnSlot) const;

private:
    // EngineActivity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

private:
    friend class ServiceManager;

    IMutex* piLock;
    IMSList<Service*> objServices;
};

PUBLIC
ServiceManagerPrivate::ServiceManagerPrivate() :
        EngineActivity(),
        piLock(IMS_NULL)
{
    piLock = MutexService::GetMutexService()->CreateMutex();
}

PUBLIC
ServiceManagerPrivate::~ServiceManagerPrivate()
{
    MutexService::GetMutexService()->DestroyMutex(piLock);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL ServiceManagerPrivate::AttachService(IN Service* pService)
{
    if (pService == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Add the service to the pending service storage.
    // The registration will be triggered by the application after opening the service.

    LockGuard objLock(piLock);

    if (!objServices.Append(pService))
    {
        IMS_TRACE_E(0, "Appending a Service (%s, %d) failed", pService->GetAppId().GetStr(),
                pService->GetSlotId(), 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC
void ServiceManagerPrivate::DetachService(IN Service* pService)
{
    LockGuard objLock(piLock);

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        Service* pTmpService = objServices.GetAt(i);

        if (pTmpService->Equals(pService))
        {
            objServices.RemoveAt(i);
            break;
        }
    }

    if (objServices.IsEmpty())
    {
        IMS_TRACE_I("ServiceManager :: No services", 0, 0, 0);
    }
}

/*

Remarks

*/
PUBLIC
Service* ServiceManagerPrivate::GetService(
        IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId) const
{
    LockGuard objLock(piLock);

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        Service* pService = objServices.GetAt(i);

        if ((pService->GetSlotId() == nSlotId) && strAppId.Equals(pService->GetAppId()) &&
                strServiceId.Equals(pService->GetServiceId()))
        {
            return pService;
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC
void ServiceManagerPrivate::GetServices(
        IN IMS_SINT32 nSlotId, OUT IMSList<Service*>& objServicesOnSlot) const
{
    LockGuard objLock(piLock);

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        Service* pService = objServices.GetAt(i);

        if (pService->GetSlotId() == nSlotId)
        {
            objServicesOnSlot.Append(pService);
        }
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL ServiceManagerPrivate::DispatchMessage(IN IMSMSG& objMSG)
{
    return EngineActivity::DispatchMessage(objMSG);
}

PUBLIC
ServiceManager::ServiceManager() :
        pSvcMngrP(new ServiceManagerPrivate())
{
}

PUBLIC VIRTUAL ServiceManager::~ServiceManager()
{
    if (pSvcMngrP != IMS_NULL)
    {
        delete pSvcMngrP;
    }
}

/*

Remarks

*/
PUBLIC
IMS_BOOL ServiceManager::AttachService(IN Service* pService)
{
    return pSvcMngrP->AttachService(pService);
}

/*

Remarks

*/
PUBLIC
void ServiceManager::DetachService(IN Service* pService)
{
    pSvcMngrP->DetachService(pService);
}

/*

Remarks

*/
PUBLIC
Service* ServiceManager::GetService(
        IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId) const
{
    return pSvcMngrP->GetService(nSlotId, strAppId, strServiceId);
}

/*

Remarks

*/
PUBLIC
const IMSList<Service*>& ServiceManager::GetServices() const
{
    return pSvcMngrP->objServices;
}

/*

Remarks

*/
PUBLIC
IMSList<Service*> ServiceManager::GetServices(IN IMS_SINT32 nSlotId) const
{
    IMSList<Service*> objServices;

    pSvcMngrP->GetServices(nSlotId, objServices);

    return objServices;
}

/*

Remarks

*/
PUBLIC GLOBAL ServiceManager* ServiceManager::GetInstance()
{
    static ServiceManager* pSvcMngr = IMS_NULL;

    if (pSvcMngr == IMS_NULL)
    {
        pSvcMngr = new ServiceManager();
        pSvcMngr->StartUp();
    }

    return pSvcMngr;
}

/*

Remarks

*/
PRIVATE VIRTUAL void ServiceManager::ServiceClosed(IN Service* pService)
{
    DetachService(pService);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL ServiceManager::StartUp()
{
    // Register the URI scheme for the creation of Service (CoreService)

    // Activate the packet data connection ...

    // Waits ISIM ready
    // Read the IMPI/HOME DOMAIN NAME/IMPU from ISIM ...

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void ServiceManager::CleanUp()
{
    // ProtocolPermission::GetInstance()->Deregister(strName);
}
