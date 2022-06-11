#ifndef _INTERFACE_DIRECT_CORE_SERVICE_LISTENER_H_
#define _INTERFACE_DIRECT_CORE_SERVICE_LISTENER_H_

class ICoreService;
class ISipConnectionFactory;

/**
 * @brief This class provides a listener interface to receive an incoming SIP request
 *        notifications.
 *
 * All the transactions are managed and handled by the application.
 *
 * @see ICoreService
 */
class IDirectCoreServiceListener
{
public:
    /**
     * @brief Notifies the application when the SIP server transaction is created and received.
     *
     * @param piService Pointer to the concerned IService
     * @param piSCF Pointer to the ISipConnectionFactory
     * @return The result of direct transaction handling.\n
     *         #ICoreService#RESULT_DIRECT_TXN_HANDLED\n
     *         #ICoreService#RESULT_DIRECT_TXN_NOT_HANDLED\n
     *         #ICoreService#RESULT_DIRECT_TXN_BYPASS
     */
    virtual IMS_SINT32 DirectCoreService_TransactionReceived(
            IN ICoreService* piService, IN ISipConnectionFactory* piSCF) = 0;
};

#endif  // _INTERFACE_DIRECT_CORE_SERVICE_LISTENER_H_
