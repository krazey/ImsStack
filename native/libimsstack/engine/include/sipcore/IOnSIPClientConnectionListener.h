/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100525  hwangoo.park@             Created
    </table>

    Description
    This class defines a listener interface for an incoming SIP responses.
*/

#ifndef _INTERFACE_ON_SIP_CLIENT_CONNECTION_LISTENER_H_
#define _INTERFACE_ON_SIP_CLIENT_CONNECTION_LISTENER_H_

class SIPClientConnection;

/*
SIP client connection listener interface

Example

See Also
SIPClientConnection

*/
class IOnSIPClientConnectionListener
{
public:
    /*
     This method gives the SIPClientConnection instance, which has received a new SIP response.
    The application implementing this listener interface has to call SIPClientConnection::Receive()
    to initialize the SIPClientConnection object with the new response.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSCC                    Pointer to SIPClientConnection object carrying the response
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void OnClientConnection_NotifyResponse(IN SIPClientConnection* pSCC) = 0;

    //// IMS extensions

    /*
     This method gives the SIPClientConnection instance, which has received a new SIP response.
    The application implementing this listener interface has to call SIPClientConnection::Receive()
    to initialize the SIPClientConnection object with the new response.

    Remarks

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSCC                    Pointer to SIPClientConnection object carrying the response
    pForkedSCC              Pointer to SIPClientConnection object carrying the forked response
    IMS extensions
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void OnClientConnection_NotifyForkedResponse(
            IN SIPClientConnection* pSCC, IN SIPClientConnection* pForkedSCC) = 0;
};

#endif  // _INTERFACE_ON_SIP_CLIENT_CONNECTION_LISTENER_H_
