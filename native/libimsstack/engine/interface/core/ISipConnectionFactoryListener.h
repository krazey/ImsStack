#ifndef _INTERFACE_SIP_CONNECTION_FACTORY_LISTENER_H_
#define _INTERFACE_SIP_CONNECTION_FACTORY_LISTENER_H_

class ISipConnectionFactory;

/**
 * @brief This class provides a listener interface to monitor an incoming SIP transaction.
 *
 * @see ISipServerConnection
 */
class ISipConnectionFactoryListener
{
public:
    /**
     * @brief Notifies the application that a new incoming request inside of SIP dialog
     *        is received.
     *
     * @param piSCFactory Pointer to ISipConnectionFactory
     * @param piSSC Pointer to ISipServerConnection; for incoming SIP request
     */
    virtual void ConnectionFactory_NotifyRequest(
            IN ISipConnectionFactory* piSCFactory, IN ISipServerConnection* piSSC) = 0;
};

#endif  // _INTERFACE_SIP_CONNECTION_FACTORY_LISTENER_H_
