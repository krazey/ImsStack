/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20160215  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_SIP_CLIENT_TRANSMISSION_LISTENER_H_
#define _INTERFACE_SIP_CLIENT_TRANSMISSION_LISTENER_H_

#include "AString.h"

/*
SIP client transmission listener interface

Example

See Also
*/
class ISIPClientTransmissionListener
{
public:
    /*

    Remarks

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
    virtual void ClientTransmission_NotifyError(
            IN IMS_SINT32 nCode, IN CONST AString& strMessage) = 0;

    /*

    Remarks

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
    virtual void ClientTransmission_TransmissionCompleted() = 0;
};

#endif  // _INTERFACE_SIP_CLIENT_TRANSMISSION_LISTENER_H_
