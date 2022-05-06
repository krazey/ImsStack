/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110517  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_SUBSCRIBER_INFO_LISTENER_H_
#define _INTERFACE_SUBSCRIBER_INFO_LISTENER_H_

#include "AString.h"

class ISubscriberInfoListener
{
public:
    /*
     Notifies the application that the subscriber info. (IMPU) has been updated.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nSlotId                 Slot id
    strId                   Identifier of subscriber configuration which contains
                            the subscriber info.
    strOld                  IMPU which is previously set
    strNew                  IMPU which will be updated
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    virtual void SubscriberInfo_UpdateIMPU(IN IMS_SINT32 nSlotId, IN const AString& strId,
            IN const AString& strOld, IN const AString& strNew) = 0;
};

#endif  // _INTERFACE_SUBSCRIBER_INFO_LISTENER_H_
