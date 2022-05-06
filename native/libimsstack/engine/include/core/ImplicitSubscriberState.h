/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100412  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _IMPLICIT_SUBSCRIBER_STATE_H_
#define _IMPLICIT_SUBSCRIBER_STATE_H_

#include "SubState.h"

/*
This class defines a state & behavior for subscriber (implicit subscription).

Example

See Also

*/
class ImplicitSubscriberState : public SubState
{
public:
    ImplicitSubscriberState();
    virtual ~ImplicitSubscriberState();

private:
    ImplicitSubscriberState(IN CONST ImplicitSubscriberState& objRHS);
    ImplicitSubscriberState& operator=(IN CONST ImplicitSubscriberState& objRHS);

public:
    // SubState class
    virtual IMS_BOOL UpdateState(IN CONST ISipMessage* piSIPMsg);

private:
    IMS_SINT32 TranslateMessage(IN CONST ISipMessage* piSIPMsg);

    IMS_BOOL UpdateOnNOTIFYRequest(IN CONST ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOnNOTIFYResponse(IN CONST ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOnREFERRequest(IN CONST ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOnREFERResponse(IN CONST ISipMessage* piSIPMsg);

    static void InitializeStateTable();

private:
    // Trigger events for subscription state transition
    enum
    {
        MESSAGE_INVALID = (-1),

        // REFER
        MESSAGE_REFER = 0,
        MESSAGE_REFER_1XX,
        MESSAGE_REFER_200,
        MESSAGE_REFER_202,
        MESSAGE_REFER_481,
        MESSAGE_REFER_NON2XX,  // Except for 481

        // NOTIFY
        MESSAGE_NOTIFY_ACTIVE,
        MESSAGE_NOTIFY_PENDING,
        MESSAGE_NOTIFY_TERMINATED,
        MESSAGE_NOTIFY_1XX,
        MESSAGE_NOTIFY_2XX,
        MESSAGE_NOTIFY_XXX_TERMINATED,
        MESSAGE_NOTIFY_NON2XX,

        MESSAGE_MAX
    };

    // ONLY OUTGOING SUBSCRIPTION WILL BE CONCERNED ...
    static IMS_SINT32 STATE[STATE_MAX][MESSAGE_MAX];
};

#endif  // _IMPLICIT_SUBSCRIBER_STATE_H_
