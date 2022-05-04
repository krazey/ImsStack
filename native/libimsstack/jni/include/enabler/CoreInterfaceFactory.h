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

#ifndef _CORE_INTERFACE_FACTORY_H_
#define _CORE_INTERFACE_FACTORY_H_

#include "BaseService.h"

class CoreInterfaceFactory
{
private:
    CoreInterfaceFactory();
    ~CoreInterfaceFactory();

public:
    static BaseService* GetInterface(
            IN IMS_SINT32 nInterfaceType, IN CBServiceNoti pNotifier, IN IMS_SINT32 nSlotId);
};

#endif  // _CORE_INTERFACE_FACTORY_H_
