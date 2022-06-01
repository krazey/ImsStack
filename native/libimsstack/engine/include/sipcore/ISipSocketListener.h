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

#ifndef _INTERFACE_SIP_SOCKET_LISTENER_H_
#define _INTERFACE_SIP_SOCKET_LISTENER_H_

class SIPSocket;

/*
SIP socket listener interface

Example

See Also

*/
class ISIPSocketListener
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
    virtual void Socket_NotifyError(IN SIPSocket* pSocket, IN IMS_SINT32 nErrorCode) = 0;

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
    virtual void Socket_SendEnabled(IN SIPSocket* pSocket) = 0;
};

#endif  // _INTERFACE_SIP_SOCKET_LISTENER_H_
