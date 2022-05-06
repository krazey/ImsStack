/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100428  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_ON_PAGE_MESSAGE_LISTENER_H_
#define _INTERFACE_ON_PAGE_MESSAGE_LISTENER_H_

class PageMessage;

/*

This listener type is used to notify the application about the status of sent page messages.

Example

See Also
PageMessage

*/
class IOnPageMessageListener
{
public:
    /*

    Notifies the application that the PageMessage was successfully delivered.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pPageMessage            Pointer to PageMessage object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnPageMessage_Delivered(IN PageMessage* pPageMessage) = 0;

    /*

    Notifies the application that the PageMessage was not successfully delivered.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    pPageMessage            Pointer to PageMessage object
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void OnPageMessage_DeliveryFailed(IN PageMessage* pPageMessage) = 0;
};

#endif  // _INTERFACE_ON_PAGE_MESSAGE_LISTENER_H_
