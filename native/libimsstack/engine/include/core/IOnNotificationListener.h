/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20150824  hwangoo.park@             Created
    </table>

    Description
     This class defines a listener which is used to notify the application about
    the delivery result of notification (as notifier for REFER-NOTIFY, SUBSCRIBE-NOTIFY).
*/

#ifndef _INTERFACE_ON_NOTIFICATION_LISTENER_H_
#define _INTERFACE_ON_NOTIFICATION_LISTENER_H_

class ServiceMethod;

/*
This listener type is used to notify the application about
the delivery result of notification (as notifier for REFER-NOTIFY, SUBSCRIBE-NOTIFY).

Example

See Also
IReference

*/
class IOnNotificationListener
{
public:
    /*
     Notifies the application that the notification was successfully delivered.
    This can be notified by ISubscription or IReference.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pMethod                 Pointer to ServiceMethod object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnNotification_Delivered(IN ServiceMethod* pMethod) = 0;

    /*
     Notifies the application that the notification was not successfully delivered.
    This can be notified by ISubscription or IReference.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pMethod                 Pointer to ServiceMethod object
    nStatusCode             SIP status code
                            0(zero) indicates that SIP transaction timeout is occurred
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void OnNotification_DeliveryFailed(
            IN ServiceMethod* pMethod, IN IMS_SINT32 nStatusCode) = 0;
};

#endif  // _INTERFACE_ON_NOTIFICATION_LISTENER_H_
