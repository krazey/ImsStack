#ifndef _INTERFACE_NOTIFICATION_LISTENER_H_
#define _INTERFACE_NOTIFICATION_LISTENER_H_

class IServiceMethod;

/**
 * @brief This class provides a listener interface which is used to notify the application about
 *        the delivery result of notification (as notifier for REFER-NOTIFY, SUBSCRIBE-NOTIFY).
 *
 * @see IReference
 */
class INotificationListener
{
public:
    /**
     * @brief Notifies the application that the notification was successfully delivered.
     *
     * This can be notified by ISubscription or IReference.
     *
     * @param piMethod Service method to be notified
     */
    virtual void NotificationDelivered(IN IServiceMethod* piMethod) = 0;

    /**
     * @brief Notifies the application that the notification was not successfully delivered.
     *
     * This can be notified by ISubscription or IReference.
     *
     * @param piMethod Service method to be notified
     * @param nStatusCode SIP status code\n
     *                    0(zero) indicates that SIP transaction timeout is occurred
     */
    virtual void NotificationDeliveryFailed(
            IN IServiceMethod* piMethod, IN IMS_SINT32 nStatusCode) = 0;
};

#endif  // _INTERFACE_NOTIFICATION_LISTENER_H_
