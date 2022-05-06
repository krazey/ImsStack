/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20130103  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_CONFIG_UPDATE_LISTENER_H_
#define _INTERFACE_CONFIG_UPDATE_LISTENER_H_

#include "AString.h"

class IConfigUpdateListener
{
public:
    /*
     Notifies the application that the specified configuration parameter item is updated.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nCPI                    Configurable parameter item to be notified (refer to IConfigurable.h)
    strConfName             Configuration name to be updated
    strExtraParam           Extra parameter info. for each configuration
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void ConfigUpdate_NotifyUpdate(IN IMS_SINT32 nCPI,
            IN const AString& strConfName = AString::ConstNull(),
            IN const AString& strExtraParam = AString::ConstNull()) = 0;
};

#endif  // _INTERFACE_CONFIG_UPDATE_LISTENER_H_
