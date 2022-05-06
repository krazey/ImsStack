/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100428  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_ON_CAPABILITIES_LISTENER_H_
#define _INTERFACE_ON_CAPABILITIES_LISTENER_H_

class Capabilities;

/*

This listener type is used to notify the application about responses to capability queries.

Example

See Also
Capabilities

*/
class IOnCapabilitiesListener
{
public:
    /*

    Notifies the application that the capability query response
    from the remote endpoint was successfully received.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pCapabilities           Pointer to Capabilities object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnCapabilities_QueryDelivered(IN Capabilities* pCapabilities) = 0;

    /*

    Notifies the application that the capability query response
    from the remote endpoint was not successfully received.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pCapabilities           Pointer to Capabilities object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnCapabilities_QueryDeliveryFailed(IN Capabilities* pCapabilities) = 0;
};

#endif  // _INTERFACE_ON_CAPABILITIES_LISTENER_H_
