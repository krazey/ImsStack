/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091106  toastops@                 Created
    </table>

    Description

*/

#ifndef _INTERFACE_APP_CONFIG_H_
#define _INTERFACE_APP_CONFIG_H_

#include "ImsRegistry.h"
#include "ICoreServiceConfig.h"

class IAppConfig
{
public:
    /*
     Returns the application id that this configuration was created with.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Application id
    </table>
    */
    virtual const AString& GetAppId() const = 0;

    /*
     Returns the Core Service configuration from the AppConfig.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strServiceId            Service id to be found
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    ICoreServiceConfig*     Pointer to ICoreServiceConfig object
    </table>
    */
    virtual const ICoreServiceConfig* GetCoreServiceConfig(
            IN const AString& strServiceId) const = 0;

    /*
     Returns the IMS registry format from the AppConfig.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    ImsRegistry             Pointer to ImsRegistry object
    </table>
    */
    virtual ImsRegistry* ToRegistry() const = 0;
};

#endif  // _INTERFACE_APP_CONFIG_H_
