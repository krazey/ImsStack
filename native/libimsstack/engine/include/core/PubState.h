/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100424  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _PUB_STATE_H_
#define _PUB_STATE_H_

#include "ISipMessage.h"
#include "util/EventPackage.h"
#include "SipHeaderProperty.h"

/*
This class defines a state & behavior for a published event state.

Example

See Also

*/
class PubState
{
public:
    PubState();
    virtual ~PubState();

private:
    PubState(IN CONST PubState& objRHS);
    PubState& operator=(IN CONST PubState& objRHS);

public:
    void Clear();
    IMS_BOOL CreateEventPackage(IN CONST AString& strEvent);
    IMS_SINT32 GetDuration() const;
    const AString& GetEntityTag() const;
    EventPackage* GetEventPackage();
    IMS_SINT32 GetOperation() const;
    IMS_SINT32 GetState() const;
    IMS_BOOL IsTerminated() const;
    IMS_BOOL SetHeaders(IN_OUT ISipMessage*& piSIPMsg);
    void SetOperation(IN IMS_SINT32 nOperation);
    IMS_BOOL UpdateState(IN CONST ISipMessage* piSIPMsg);
    IMS_BOOL UpdateStateOnTxnTimerExpired();

private:
    void SetState(IN IMS_SINT32 nState);
    void StoreMessage(IN CONST ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOnPUBLISHRequest(IN CONST ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOnPUBLISHResponse(IN CONST ISipMessage* piSIPMsg);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // State of publication
    enum
    {
        STATE_INIT = 0,
        STATE_PENDING,
        STATE_ACTIVE,
        STATE_TERMINATED
    };

    // Type of publication operation
    enum
    {
        NO_OPERATION = 0,

        OPERATION_CREATE,
        OPERATION_MODIFY,
        // Refreshed by the PublicationRefreshHelper
        OPERATION_REFRESH,
        OPERATION_REMOVE
    };

    enum
    {
        NO_EXPIRES = (-1)
    };

    static const SipHeaderProperty RESTRICTED_HEADER_PROPERTIES[];

private:
    // Event package for the publication
    EventPackage objEventPackage;

    // State of publication
    IMS_SINT32 nState;

    // Current operation for the publication: ADD/MODIFY/REFRESH/REMOVE
    //    Operation        Body        SIP-If-Match        Expires
    //    INITIAL          yes         no                  > 0
    //    REFRESH          no          yes                 > 0
    //    MODIFY           yes         yes                 > 0
    //    REMOVE           no          yes                 0
    IMS_SINT32 nOperation;

    // Authoritative publication duration
    //    - "Expires" header in 2XX to PUBLISH request
    IMS_SINT32 nPublicationDuration;

    // SIP-ETag: received in the 2xx response message to the event publication
    AString strEntityTag;

    // Manages an initial SIP message for refresh/removal operation
    ISipMessage* piSIPMsg;
};

#endif  // _PUB_STATE_H_
