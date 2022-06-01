/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description
     This class defines a SIP transport listener interface.
*/

#ifndef _INTERFACE_SIP_TRANSPORT_LISTENER_H_
#define _INTERFACE_SIP_TRANSPORT_LISTENER_H_

#include "SipTransportAddress.h"

/*
SIP transport listener interface

Example

See Also
SIPTransportAddress
*/
class ISIPTransportListener
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
    virtual void Transport_PacketReceived(IN IMS_SINT32 nSlotId, IN CONST ByteArray& objBuffer,
            IN CONST SIPTransportAddress& objNearEnd, IN CONST SIPTransportAddress& objFarEnd) = 0;
};

#endif  // _INTERFACE_SIP_TRANSPORT_LISTENER_H_
