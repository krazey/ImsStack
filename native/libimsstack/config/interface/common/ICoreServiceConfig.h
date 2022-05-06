/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091106  toastops@                 Created
    </table>

    Description

*/

#ifndef _INTERFACE_CORE_SERVICE_CONFIG_H_
#define _INTERFACE_CORE_SERVICE_CONFIG_H_

#include "ServiceIdentifier.h"

class ICoreServiceConfig
{
public:
    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual const AString& GetServiceId() const = 0;

    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual IMS_BOOL IsIARISupported() const = 0;

    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual const ServiceIdentifier& GetIARI() const = 0;

    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual const IMSList<ServiceIdentifier>& GetICSIs() const = 0;

    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual const IMSList<ServiceIdentifier>& GetFeatureTags() const = 0;

    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual const AString& GetMediaProfile() const = 0;
};

#endif  // _INTERFACE_CORE_SERVICE_CONFIG_H_
