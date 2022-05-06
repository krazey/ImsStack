/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090608  toastops@                 Created
    </table>

    Description

*/

#ifndef _SERVICE_MANAGER_H_
#define _SERVICE_MANAGER_H_

#include "IMSList.h"
#include "IServiceManagerListener.h"

class Service;
class ServiceManagerPrivate;

class ServiceManager : public IServiceManagerListener
{
private:
    ServiceManager();
    ServiceManager(IN const ServiceManager& objRHS);
    ServiceManager& operator=(IN const ServiceManager& objRHS);

public:
    virtual ~ServiceManager();

public:
    IMS_BOOL AttachService(IN Service* pService);
    void DetachService(IN Service* pService);
    Service* GetService(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
            IN const AString& strServiceId) const;
    const IMSList<Service*>& GetServices() const;
    IMSList<Service*> GetServices(IN IMS_SINT32 nSlotId) const;

    static ServiceManager* GetInstance();

private:
    // IServiceManagerListener interface
    virtual void ServiceClosed(IN Service* pService);

    IMS_BOOL StartUp();
    void CleanUp();

private:
    ServiceManagerPrivate* pSvcMngrP;
};

#endif  // _SERVICE_MANAGER_H_
