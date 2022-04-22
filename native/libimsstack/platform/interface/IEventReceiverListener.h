
/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101020  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_EVENT_RECEIVER_LISTENER_H_
#define _INTERFACE_IMS_EVENT_RECEIVER_LISTENER_H_

#include "ImsTypeDef.h"

class IEventReceiverListener
{
public:
    /*

    Notifies the application for the event which is received on the event receiver.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_TRUE                The specified event is handled successfully
    IMS_FALSE               The specified event is not handled
    </table>

    */
    virtual IMS_BOOL EventReceiver_NotifyEvent(IN IMS_SINT32 nEvent,
            IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) = 0;
};

#endif // _INTERFACE_IMS_EVENT_RECEIVER_LISTENER_H_
