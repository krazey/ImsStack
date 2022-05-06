/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101220  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_REG_CAPABILITY_CHANGE_LISTENER_H_
#define _INTERFACE_REG_CAPABILITY_CHANGE_LISTENER_H_

#include "AString.h"

class IRegCapabilityChangeListener
{
public:
    /*
     Notifies the application that the registration binding is updated by adding a service.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strAppId                Application identifier
    strServiceId            Service identifier
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void RegCapabilityChange_ServiceAdded(
            IN CONST AString& strAppId, IN CONST AString& strServiceId) = 0;

    /*
     Notifies the application that the registration binding is updated by removing a service.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strAppId                Application identifier
    strServiceId            Service identifier
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void RegCapabilityChange_ServiceRemoved(
            IN CONST AString& strAppId, IN CONST AString& strServiceId) = 0;
};

#endif  // _INTERFACE_REG_CAPABILITY_CHANGE_LISTENER_H_
