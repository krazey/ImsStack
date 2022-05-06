#ifndef _INTERFACE_SUBSCRIPTION_STATE_H_
#define _INTERFACE_SUBSCRIPTION_STATE_H_

/**
 * @brief This class defines the constant values for the subscription event package.
 */
class ISubscriptionState
{
public:
    /// "substate" for Subscription-State header
    enum
    {
        STATE_INIT = 0,
        /// The subscription has been accepted and (in general) has been authorized.
        STATE_ACTIVE,
        /// The subscription has been received by the notifier,
        /// but there is insufficient policy information to grant or deny the subscription yet.
        STATE_PENDING,
        /// The subscription terminated.
        STATE_TERMINATED
    };

    /// "reason" parameter for Subscription-State header
    enum
    {
        REASON_NONE = 0,

        /// The subscription has been terminated, but the subscriber
        /// SHOULD retry immediately with a new subscription.\n
        /// One primary use of such a status code is to allow migration
        /// of subscriptions between nodes.
        REASON_DEACTIVATED,
        /// The subscription has been terminated, but the client
        /// SHOULD retry at some later time.\n
        /// If a "retry-after" parameter is also present,
        /// the client SHOULD wait at least the number of seconds specified by
        /// that parameter before attempting to re-subscribe.
        REASON_PROBATION,
        /// The subscription has been terminated due to change in authorization policy.\n
        /// Clients SHOULD NOT attempt to re-subscribe.
        REASON_REJECTED,
        /// The subscription has been terminated because it was not refreshed
        /// before it expired.\n
        /// Clients MAY re-subscribe immediately.
        REASON_TIMEOUT,
        /// The subscription has been terminated because the notifier
        /// could not obtain authorization in a timely fashion.\n
        /// If a "retry-after" parameter is also present, the client SHOULD wait at least
        /// the number of seconds specified by that parameter before attempting to
        /// re-subscribe;\n
        /// otherwise, the client MAY retry immediately,
        /// but will likely get put back into pending state.
        REASON_GIVEUP,
        /// The subscription has been terminated because the resource state
        /// which was being monitored no longer exists.\n
        /// Clients SHOULD NOT attempt to re-subscribe.
        REASON_NORESOURCE,

        REASON_MAX
    };
};

#endif  // _INTERFACE_SUBSCRIPTION_STATE_H_
