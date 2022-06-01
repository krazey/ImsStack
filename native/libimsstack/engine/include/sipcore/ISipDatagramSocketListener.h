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

#ifndef _INTERFACE_SIP_DATAGRAM_SOCKET_LISTENER_H_
#define _INTERFACE_SIP_DATAGRAM_SOCKET_LISTENER_H_

#include "IPAddress.h"

class SIPSocket;

/*
SIP datagram socket listener interface

Example

See Also
SIPSocket
*/
class ISIPDatagramSocketListener
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
    virtual void DatagramSocket_DataReceived(IN SIPSocket* pSocket, IN CONST ByteArray& objBuffer,
            IN CONST IPAddress& objIPA, IN IMS_SINT32 nPort) = 0;
};

#endif  // _INTERFACE_SIP_DATAGRAM_SOCKET_LISTENER_H_
