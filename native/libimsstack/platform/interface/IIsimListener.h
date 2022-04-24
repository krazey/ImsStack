
/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100927  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_ISIM_LISTENER_H_
#define _INTERFACE_IMS_ISIM_LISTENER_H_

#include "IMSList.h"
#include "ByteArray.h"

class IIsimListener
{
public:
    /*

    Notifies the application that the specified field value is retrieved.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nField                  Field type to be requested
    objValues               Field values
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void Isim_OnField(IN IMS_SINT32 nField, IN CONST IMSList<ByteArray> &objValues) = 0;

    /*

    Notifies the application that the home domain name is retrieved.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    objDomainName           Home domain name
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void Isim_OnHomeDomainName(IN CONST ByteArray &objDomainName) = 0;

    /*

    Notifies the application that the private user identity is retrieved.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    objIMPI                 Private user identity
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void Isim_OnImpi(IN CONST ByteArray &objIMPI) = 0;

    /*

    Notifies the application that the public user identities are retrieved.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    objIMPUs                List of public user identity
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void Isim_OnImpu(IN CONST IMSList<ByteArray> &objIMPUs) = 0;

    /*

    Notifies the application that the error occurrs in the ISIM module.

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
    virtual void Isim_OnError(IN IMS_SINT32 nErrorCode) = 0;

    /*

    Notifies the application that the ISIM state is changed.

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
    virtual void Isim_OnStateChanged(IN IMS_SINT32 nState) = 0;
};

#endif // _INTERFACE_IMS_ISIM_LISTENER_H_
