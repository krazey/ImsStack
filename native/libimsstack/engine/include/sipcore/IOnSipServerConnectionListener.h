/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100525  hwangoo.park@             Created
    </table>

    Description
     This class defines a listener interface for an incoming SIP requests.
*/

#ifndef _INTERFACE_ON_SIP_SERVER_CONNECTION_LISTENER_H_
#define _INTERFACE_ON_SIP_SERVER_CONNECTION_LISTENER_H_

class SIPConnectionNotifier;

/*
SIP server connection listener interface

Example

See Also
SIPConnectionNotifier

*/
class IOnSIPServerConnectionListener
{
public:
    /*
     This method will notify the listener that a new request is received. This method gives the
    SIPConnectionNotifier instance. The user has to call the SIPConnectionNotifier::AcceptAndOpen()
    to get the SIPServerConnection object that holds the server transaction
    and the request received.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSCN                    Pointer to SIPConnectionNotifier object carrying SIPServerConnection
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void OnServerConnection_NotifyRequest(IN SIPConnectionNotifier* pSCN) = 0;

    //// IMS extensions

    /*
     This method will notify the listener that a new request is received. This method gives the
    SIPConnectionNotifier instance. The user has to call the SIPConnectionNotifier::AcceptAndOpen()
    to get the SIPServerConnection object that holds the server transaction
    and the request received.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSCN                    Pointer to SIPConnectionNotifier object carrying SIPServerConnection
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void OnServerConnection_NotifyForkedRequest(IN SIPConnectionNotifier* pSCN) = 0;
};

#endif  // _INTERFACE_ON_SIP_SERVER_CONNECTION_LISTENER_H_
