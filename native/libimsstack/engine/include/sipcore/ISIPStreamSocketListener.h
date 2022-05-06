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

#ifndef _INTERFACE_SIP_STREAM_SOCKET_LISTENER_H_
#define _INTERFACE_SIP_STREAM_SOCKET_LISTENER_H_

class SIPSocket;

/*
SIP stream socket listener interface

Example

See Also
SIPSocket, ByteArray
*/
class ISIPStreamSocketListener
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
    virtual void StreamSocket_ConnectionReceived(IN SIPSocket* pSocket) = 0;

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
    virtual void StreamSocket_DataReceived(IN SIPSocket* pSocket, IN_OUT ByteArray& objBuffer) = 0;

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
    virtual void StreamSocket_KeepAliveExpired(IN SIPSocket* pSocket) = 0;

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
    virtual void StreamSocket_PassiveClosed(IN SIPSocket* pSocket) = 0;
};

#endif  // _INTERFACE_SIP_STREAM_SOCKET_LISTENER_H_
