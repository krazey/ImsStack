#ifndef _INTERFACE_SIP_SERVER_CONNECTION_LISTENER_H_
#define _INTERFACE_SIP_SERVER_CONNECTION_LISTENER_H_

class ISIPConnectionNotifier;

/**
 * @brief This class provides a listener interface for an incoming SIP requests.
 *
 * @see ISIPConnectionNotifier
 */
class ISIPServerConnectionListener
{
public:
    /**
     * @brief This method will notify the listener that a new request is received.
     *
     * This method gives the ISIPConnectionNotifier instance.\n
     * The user has to call the ISIPConnectionNotifier::AcceptAndOpen() to get
     * the ISIPServerConnection object that holds the server transaction and the request received.
     *
     * @param piSCN Pointer to ISIPConnectionNotifier object carrying ISIPServerConnection
     * @param bIsForked Flag to indicate that an incoming request is forked or not
     */
    virtual void ServerConnection_NotifyRequest(IN ISIPConnectionNotifier *piSCN,
            IN IMS_BOOL bIsForked = IMS_FALSE) = 0;
};

#endif // _INTERFACE_SIP_SERVER_CONNECTION_LISTENER_H_
