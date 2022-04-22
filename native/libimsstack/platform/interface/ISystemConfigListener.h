/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170501  hwangoo.park@             Created
    </table>

    Description

*/
#ifndef _INTERFACE_SYSTEM_CONFIG_LISTENER_H_
#define _INTERFACE_SYSTEM_CONFIG_LISTENER_H_

#include "ImsTypeDef.h"

class ISystemConfigListener
{
public:
    /*
     Notifies the applications when system configuration is changed.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nEvent                  Event to indicate which configuration is changed
    nSlotId                 Slot id for the following events:
                                SystemConfig#EVENT_SYSTEM_CONFIGURATION_CHANGED
                                SystemConfig#EVENT_SUBSCRIPTION_CHANGED
                            If nSlotId is IMS_SLOT_ANY, then the application considers that
                            all (both SIM slots) the configurations are changed.
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void SystemConfig_ConfigurationChanged(
            IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId = IMS_SLOT_ANY) = 0;
};

#endif // _INTERFACE_SYSTEM_CONFIG_LISTENER_H_
