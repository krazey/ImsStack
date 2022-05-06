/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100828  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "Service.h"
#include "ServiceManager.h"
#include "ServiceResolver.h"

__IMS_TRACE_TAG_IMS__;

/*

Remarks

*/
PUBLIC GLOBAL IRegBinding* ServiceResolver::GetRegBinding(
        IN IMS_SINT32 nSlotId, IN const AString& strAppId, IN const AString& strServiceId)
{
    Service* pService = ServiceManager::GetInstance()->GetService(nSlotId, strAppId, strServiceId);

    if (pService != IMS_NULL)
    {
        return pService->GetRegBinding();
    }

    IMS_TRACE_E(0, "Can't find a service (%d, %s, %s)", nSlotId, strAppId.GetStr(),
            strServiceId.GetStr());

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC GLOBAL IMSList<IRegBinding*> ServiceResolver::GetRegBindings(IN IMS_SINT32 nSlotId)
{
    IMSList<Service*> objServices = ServiceManager::GetInstance()->GetServices(nSlotId);
    IMSList<IRegBinding*> objIRegBindings;

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        Service* pService = objServices.GetAt(i);

        if (pService != IMS_NULL)
        {
            objIRegBindings.Append(pService->GetRegBinding());
        }
    }

    IMS_TRACE_D("Count of RegBinding (%d)", objIRegBindings.GetSize(), 0, 0);

    return objIRegBindings;
}

/*

Remarks

*/
PUBLIC GLOBAL void ServiceResolver::SetRegBinding(IN IMS_SINT32 nSlotId, IN const AString& strAppId,
        IN const AString& strServiceId, IN IRegBinding* piRegBinding)
{
    Service* pService = ServiceManager::GetInstance()->GetService(nSlotId, strAppId, strServiceId);

    if (pService != IMS_NULL)
    {
        pService->SetRegBinding(piRegBinding);
        return;
    }

    IMS_TRACE_E(0, "Can't find a service (%d, %s, %s)", nSlotId, strAppId.GetStr(),
            strServiceId.GetStr());
}
