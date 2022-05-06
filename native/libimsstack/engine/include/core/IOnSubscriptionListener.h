/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100428  hwangoo.park@             Created
    </table>

    Description
     This class defines a listener which is used to notify the application about subscription
    status and the event state of the subscribed event packages.
*/

#ifndef _INTERFACE_ON_SUBSCRIPTION_LISTENER_H_
#define _INTERFACE_ON_SUBSCRIPTION_LISTENER_H_

class Message;
class Subscription;

/*
 This listener type is used to notify the application about subscription status
and the event state of the subscribed event package.

Example

See Also
Subscription

*/
class IOnSubscriptionListener
{
public:
    /*

     Notifies the application that the forked NOTIFY request received.
    In this moment, the method just notifies that the forked subscription is created
    by the forked SUBSCRIBE request.
     The NOTIFY message will be delivered by the forked subscription (pForkedSubscription).

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSubscription           Pointer to Subscription object
    pForkedSubscription     Pointer to the forked Subscription object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual IMS_BOOL OnSubscription_ForkedNotifyReceived(
            IN Subscription* pSubscription, IN Subscription* pForkedSubscription) = 0;

    /*

     Notifies the application that the event notification received.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSubscription           Pointer to Subscription object
    pNotify                 Pointer to Message object which has an event state of
                            the subscribed event package
    bDestroyNotify          Flag to indicate if the Notify message should be destroyed
                            or not after returning the method
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnSubscription_NotifyReceived(
            IN Subscription* pSubscription, IN Message* pNotify, OUT IMS_BOOL& bDestroyNotify) = 0;

    /*

     Notifies the application that the durative subscription was successfully started or updated.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSubscription           Pointer to Subscription object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnSubscription_Started(IN Subscription* pSubscription) = 0;

    /*

     Notifies the application that the durative subscription failed to start or update.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSubscription           Pointer to Subscription object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnSubscription_StartFailed(IN Subscription* pSubscription) = 0;

    /*

     Notifies the application that the subscription was terminated.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pSubscription           Pointer to Subscription object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnSubscription_Terminated(IN Subscription* pSubscription) = 0;
};

#endif  // _INTERFACE_ON_SUBSCRIPTION_LISTENER_H_
