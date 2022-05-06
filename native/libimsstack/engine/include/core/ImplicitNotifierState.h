/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100420  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _IMPLICIT_NOTIFIER_STATE_H_
#define _IMPLICIT_NOTIFIER_STATE_H_

#include "SubState.h"

/*
This class defines a state & behavior for notifier (implicit subscription).

Example

See Also

*/
class ImplicitNotifierState : public SubState
{
public:
    ImplicitNotifierState();
    virtual ~ImplicitNotifierState();

private:
    ImplicitNotifierState(IN CONST ImplicitNotifierState& objRHS);
    ImplicitNotifierState& operator=(IN CONST ImplicitNotifierState& objRHS);

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

    // STATE for Notifier
    static IMS_SINT32 STATE[STATE_MAX][MESSAGE_MAX];
};

#endif  // _IMPLICIT_NOTIFIER_STATE_H_
