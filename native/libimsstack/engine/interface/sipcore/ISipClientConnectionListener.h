#ifndef _INTERFACE_SIP_CLIENT_CONNECTION_LISTENER_H_
#define _INTERFACE_SIP_CLIENT_CONNECTION_LISTENER_H_

class ISIPClientConnection;

/**
 * @brief This class provides an interface for an incoming SIP responses.
 *
 * @see ISIPClientConnection
 */
class ISIPClientConnectionListener
{
public:
    /**
     * @brief This method gives the ISIPClientConnection instance, which has received
     *        a new SIP response.
     *
     * The application implementing this listener interface has to call
     * ISIPClientConnection::Receive() to initialize the ISIPClientConnection object
     * with the new response.
     *
     * @param piSCC Pointer to ISIPClientConnection object carrying the response
     * @param piForkedSCC Pointer to ISIPClientConnection object carrying the forked response
     */
    virtual void ClientConnection_NotifyResponse(IN ISIPClientConnection *piSCC,
            IN ISIPClientConnection *piForkedSCC = IMS_NULL) = 0;
};

#endif // _INTERFACE_SIP_CLIENT_CONNECTION_LISTENER_H_
