#ifndef _INTERFACE_REFRESH_LISTENER_H_
#define _INTERFACE_REFRESH_LISTENER_H_

class ISipClientConnection;

/**
 * @brief This class defines a refresh listener for refreshable SIP methods.
 */
class IRefreshListener
{
public:
    /**
     * @brief Notifies the application that the refresh operation is completed.
     *
     * @param piSCC SIP client connection for the refresh transaction
     * @see ISipClientConnection
     */
    virtual void Refresh_NotifyCompleted(IN ISipClientConnection* piSCC) = 0;

    /**
     * @brief Notifies the application that the refresh is terminated.
     */
    virtual void Refresh_NotifyTerminated() = 0;

    /**
     * @brief Notifies the application that the refresh timer is expired.
     *
     * @param bDoImplicitRefresh Flag to indicate if the refresh will be executed or not
     */
    virtual void Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) = 0;
};

#endif  // _INTERFACE_REFRESH_LISTENER_H_
