
/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100927  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_DIGEST_AKA_H_
#define _INTERFACE_IMS_DIGEST_AKA_H_

#include "ByteArray.h"

class IDigestAKAListener;

class IDigestAKA
{
public:
    /*

    Destroys the Digest AKA.

    Parameters
    <table>
    parameter                   description
    ----------                  ----------
    </table>

    Returns
    <table>
    return                      description
    ----------                  ----------

    </table>

    */
    virtual void Destroy() = 0;

    /*

    Authenticates the client & server from the Digest AKA authentication challenge.
    It extracts the RAND and AUTN from the "nonce" parameter, and accesses the AUTN token provided
    by the server. If the client successfully authenticates the server with the AUTN,
    and determines that the SQN used in generating the challenge is within expected range,
    the AKA algorithms are run with the RAND challenge and shared secret K.

    The resulting AKA RES parameter is treated as a "password" when calculating the response
    directive.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    objChallenge            Authentication challenge
                            (Length of RAND (1) + RAND (16) + Length of AUTN (1) + AUTN (16))
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SUCCESS             The operation is done successfully
    IMS_FAILURE             The operation is failed
    </table>

    */
    virtual IMS_RESULT GetAuthResponse(IN const ByteArray &objChallenge) = 0;

    /*

    Sets the listener to receive the result of authentication.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piListener              Listener to be set
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void SetListener(IN IDigestAKAListener *piListener) = 0;
};

#endif // _INTERFACE_IMS_DIGEST_AKA_H_
