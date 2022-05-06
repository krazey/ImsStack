/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100420  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _INTERFACE_REFERRED_MESSAGE_LISTENER_H_
#define _INTERFACE_REFERRED_MESSAGE_LISTENER_H_

#include "SubState.h"

class ISipMessage;

/*

IReferredMessageListener interface

Example

See Also

*/
class IReferredMessageListener
{
public:
    /*
     Notifies the SIP response message to the remote endpoint with the substate, "active".

    Parameters
    <table>
    parameter               description
    ----------              ----------
    piSIPMsg                SIP message to be notified
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void ReferredMessage_NotifyOnActive(IN ISipMessage* piSIPMsg) = 0;

    /*
     Notifies the SIP response message to the remote endpoint with the substate, "terminated".
    If the reason code is not REASON_NONE, the reason parameter will be included in NOTIFY request.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nReasonCode             Reason code, defined in SubState.h
    piSIPMsg                SIP message to be notified
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void ReferredMessage_NotifyOnTerminated(
            IN IMS_SINT32 nReasonCode = SubState::REASON_NONE,
            IN ISipMessage* piSIPMsg = IMS_NULL) = 0;
};

#endif  // _INTERFACE_REFERRED_MESSAGE_LISTENER_H_
