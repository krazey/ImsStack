
/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101006  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_IMS_EVENT_SENDER_H_
#define _INTERFACE_IMS_EVENT_SENDER_H_

#include "ImsTypeDef.h"

class IEventSender
{
public:
    virtual void SendEvent(IN IMS_SINT32 nEvent,
            IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0) = 0;
};

#endif // _INTERFACE_IMS_EVENT_SENDER_H_
