/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100328  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SUBSCRIBER_STATE_H_
#define _SUBSCRIBER_STATE_H_

#include "SubState.h"

/*
This class defines a state & behavior for subscriber (outgoing subscription).

Example

See Also

*/
class SubscriberState : public SubState
{
public:
    SubscriberState();
    virtual ~SubscriberState();

private:
    SubscriberState(IN CONST SubscriberState& objRHS);
    SubscriberState& operator=(IN CONST SubscriberState& objRHS);

public:
    // SubState class
    virtual IMS_BOOL UpdateState(IN CONST ISipMessage* piSIPMsg);

protected:
    // SubState class
    virtual const SIPHeaderProperty* GetRestrictedHeaders(OUT IMS_UINT32& nCount) const;

private:
    IMS_SINT32 TranslateMessage(IN CONST ISipMessage* piSIPMsg);

    IMS_BOOL UpdateOnNOTIFYRequest(IN CONST ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOnNOTIFYResponse(IN CONST ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOnSUBSCRIBERequest(IN CONST ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOnSUBSCRIBEResponse(IN CONST ISipMessage* piSIPMsg);

    static void InitializeStateTable();

private:
    // Trigger events for subscription state transition
    enum
    {
        MESSAGE_INVALID = (-1),

        // SUBSCRIBE
        MESSAGE_SUBSCRIBE = 0,
        MESSAGE_SUBSCRIBE_1XX,
        MESSAGE_SUBSCRIBE_200,
        MESSAGE_SUBSCRIBE_202,
        MESSAGE_SUBSCRIBE_481,
        MESSAGE_SUBSCRIBE_NON2XX,  // Except for 481

        // re-SUBSCRIBE
        MESSAGE_RESUBSCRIBE,
        MESSAGE_RESUBSCRIBE_1XX,
        MESSAGE_RESUBSCRIBE_200,
        MESSAGE_RESUBSCRIBE_202,
        MESSAGE_RESUBSCRIBE_481,
        MESSAGE_RESUBSCRIBE_NON2XX,  // Except for 481

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

    static const SIPHeaderProperty RESTRICTED_HEADER_PROPERTIES[];
};

#endif  // _SUBSCRIBER_STATE_H_
