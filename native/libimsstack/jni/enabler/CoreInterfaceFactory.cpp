/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20131220  hwangoo.park@             Created
    </table>

    Description
     This class provides the method to create the core interfaces.
*/

#define IMS_STL_USE

#include "IMSTypeDef.h"
#include "AString.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IUIMS.h"
#include "AString.h"
#include "JniMtcCall.h"
#include "JniMtcService.h"
#include "JniMtsService.h"

// IMS_SERVICE_AOS
#include "JniAosService.h"
#include "JniUceService.h"
#include "CoreInterfaceFactory.h"

__IMS_TRACE_TAG_USER_DECL__("CoreInterfaceFactory");



PUBLIC GLOBAL
BaseService* CoreInterfaceFactory::GetInterface(IN IMS_SINT32 nInterfaceType,
        IN CBServiceNoti pNotifier, IN IMS_SINT32 nSlotId)
{
    BaseService *pService = IMS_NULL;

    IMS_TRACE_D("GetInterface - interface=%d, slotId=%d", nInterfaceType, nSlotId, 0);

    switch (nInterfaceType)
    {
    case IUIMS::APP_MTC:
        pService = new JniMtcService(pNotifier, nSlotId);
        break;

    case IUIMS::MTC_CALL:
        pService = new JniMtcCall(pNotifier, -1, nSlotId);
        break;

    case IUIMS::APP_UCE:
        pService = new JniUceService(pNotifier, nSlotId);
        break;

    case IUIMS::APP_MTS:
        pService = new JniMtsService(pNotifier, nSlotId);
        break;

    case IUIMS::MTS_EMERGENCY_SERVICE:
        // TODO: If need, a JniService for E911 SMS will be added
        break;


    // IMS_SERVICE_AOS
    case IUIMS::AOS_SERVICE:
        pService = new JniAosService(pNotifier, nSlotId);
        break;

    default:
        IMS_TRACE_D("Invalid interface type (%d)", nInterfaceType, 0, 0);
        return IMS_NULL;
    }

    return pService;
}
