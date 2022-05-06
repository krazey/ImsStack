/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100905  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_SUBSCRIBER_CONFIG_LISTENER_H_
#define _INTERFACE_SUBSCRIBER_CONFIG_LISTENER_H_

/*

A listener type for receiving notifications about changes to the IMS subscriber configuration.

Example

See Also
ISubscriberConfig

*/
class ISubscriberConfigListener
{
public:
    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void SubscriberConfig_InitCompleted() = 0;

    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void SubscriberConfig_RefreshCompleted() = 0;

    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void SubscriberConfig_RefreshStarted() = 0;

    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void SubscriberConfig_NotifyError(IN IMS_SINT32 nErrorCode) = 0;
};

#endif  // _INTERFACE_SUBSCRIBER_CONFIG_LISTENER_H_
