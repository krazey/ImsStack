#ifndef _INTERFACE_SUBSCRIPTION_LISTENER_H_
#define _INTERFACE_SUBSCRIPTION_LISTENER_H_

class ISubscription;
class IMessage;

/**
 * @brief This class provides a listener interface to notify the application
 *        about subscription status and the event state of the subscribed event package.
 *
 * @see ISubscription, IMessage
 */
class ISubscriptionListener
{
public:
    /**
     * @brief Notifies the application that the event notification
     *        (to the forked SUBSCRIBE request) received.
     *
     * If the subscription is durative, the state remains at STATE_ACTIVE,
     * otherwise the state will transit to STATE_INACTIVE.
     *
     * @param piSubscription The concerned ISubscription
     * @param piForkedSubscription The forked ISubscription
     */
    virtual void SubscriptionForkedNotify(
            IN ISubscription* piSubscription, IN ISubscription* piForkedSubscription) = 0;

    /**
     * @brief Notifies the application that the event notification received.
     *
     * If the subscription is durative, the state remains at STATE_ACTIVE,
     * otherwise the state will transit to STATE_INACTIVE.
     *
     * @param piSubscription The concerned ISubscription
     * @param piNotify Pointer to IMessage which has an event state
     *                 of the subscribed event package
     */
    virtual void SubscriptionNotify(IN ISubscription* piSubscription, IN IMessage* piNotify) = 0;

    /**
     * @brief Notifies the application that the durative subscription was successfully
     *        started or updated.
     *
     * @param piSubscription The concerned ISubscription
     */
    virtual void SubscriptionStarted(IN ISubscription* piSubscription) = 0;

    /**
     * @brief Notifies the application that the durative subscription failed to start or update.
     *
     * @param piSubscription The concerned ISubscription
     */
    virtual void SubscriptionStartFailed(IN ISubscription* piSubscription) = 0;

    /**
     * @brief Notifies the application that the subscription was terminated.
     *
     * @param piSubscription The concerned ISubscription
     */
    virtual void SubscriptionTerminated(IN ISubscription* piSubscription) = 0;
};

#endif  // _INTERFACE_SUBSCRIPTION_LISTENER_H_
