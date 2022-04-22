
/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100927  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_DIGEST_AKA_LISTENER_H_
#define _INTERFACE_IMS_DIGEST_AKA_LISTENER_H_

#include "ByteArray.h"

class IDigestAkaListener
{
public:
    /*

    Notifies the application that the authentication is successfully done with the RES, IK and CK.
    AKA RES parameter will be used as "password" when calculating the response.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    objRES                  Response to authentication
    objIK                   IK (Integrity Key) value
    objCK                   CK (Ciphering Key) value
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void DigestAka_OnResponse(IN const ByteArray &objRES,
            IN const ByteArray &objIK = ByteArray::ConstNull(),
            IN const ByteArray &objCK = ByteArray::ConstNull()) = 0;

    /*

    Notifies the application that the AKA sequence number synchronization is failed
    and re-synchronize the SQN in the AuC using the AUTS token.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    objAUTS                 AUTS token
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void DigestAka_OnAutsFailed(IN const ByteArray &objAUTS) = 0;

    /*

    Notifies the application that the MAC (Message Authentication Code) is failed.

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
    virtual void DigestAka_OnMacFailed() = 0;
};

#endif // _INTERFACE_IMS_DIGEST_AKA_LISTENER_H_
