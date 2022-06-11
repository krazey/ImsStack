/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100412  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SUB_STATE_H_
#define _SUB_STATE_H_

#include "ISipMessage.h"
#include "util/EventPackage.h"
#include "SipHeaderProperty.h"

/*
This class defines a state & behavior for a subscription state.

Example

See Also

*/
class SubState
{
public:
    SubState();
    virtual ~SubState();

private:
    SubState(IN CONST SubState& objRHS);
    SubState& operator=(IN CONST SubState& objRHS);

public:
    virtual void Clear();
    virtual IMS_BOOL UpdateState(IN CONST ISipMessage* piSIPMsg) = 0;

    IMS_BOOL CreateEventPackage(IN CONST AString& strEvent);
    IMS_SINT32 GetConfiguration() const;
    IMS_SINT32 GetDuration() const;
    EventPackage* GetEventPackage();
    ISipMessage* GetInitialMessage() const;
    IMS_SINT32 GetOperation() const;
    IMS_SINT32 GetState() const;
    IMS_SINT32 GetSubState() const;
    IMS_BOOL IsInstantSubscription() const;
    IMS_BOOL IsSubscriptionDurationUpdated() const;
    IMS_BOOL IsTerminated() const;

#if 0
    IMS_BOOL SetHeadersAndBodyParts(IN_OUT ISipMessage *&piSIPMsg);
#endif
    void SetConfiguration(IN IMS_SINT32 nConfigValue);
    void SetOperation(IN IMS_SINT32 nOperation);

    static IMS_SINT32 ExtractExpiresParameter(IN CONST ISipHeader* piHeader);
    static IMS_SINT32 ExtractReasonParameter(IN CONST ISipHeader* piHeader);
    static IMS_SINT32 ExtractSubStateValue(IN CONST ISipHeader* piHeader);

    // Gets the constant values from ISubscriptionState
    static IMS_SINT32 GetSubStateFromSubscriptionState(IN IMS_SINT32 nSubState);
    static IMS_SINT32 GetReasonFromSubscriptionState(IN IMS_SINT32 nReason);

protected:
    virtual const SipHeaderProperty* GetRestrictedHeaders(OUT IMS_UINT32& nCount) const;

    inline IMS_BOOL IsConfigurationSet(IN IMS_SINT32 nValue) const
    {
        return (nConfigValue & nValue) != 0;
    }
    void SetDuration(IN IMS_SINT32 nDuration);
    void SetDurationUpdated(IN IMS_BOOL bDurationUpdated);
    void SetInstantSubscription(IN IMS_BOOL bInstantSubscription);
    void SetState(IN CONST ISipMessage* piSIPMsg, IN IMS_SINT32 nState);
    void SetSubState(IN IMS_SINT32 nSubState);
    void StoreMessage(IN CONST ISipMessage* piSIPMsg);

private:
    static const IMS_CHAR* OperationToString(IN IMS_SINT32 nOperation);
    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // ONLY OUTGOING SUBSCRIPTION WILL BE CONCERNED ...
    // Internal states for tracking the subscription state
    enum
    {
        // INVALID
        STATE_INVALID = (-1),
        // INIT
        STATE_INIT = 0,
        // SUBSCRIBE (new) sent/received
        STATE_SUBSCRIBING,
        // PENDING
        STATE_PENDING,
        // ACTIVE
        STATE_ACTIVE,
        // TERMINATED
        STATE_TERMINATED,

        STATE_MAX
    };

    // Type of subscription operation
    enum
    {
        NO_OPERATION = 0,

        OPERATION_CREATE,
        OPERATION_REFRESH,
        OPERATION_FETCH,
        OPERATION_REMOVE,
        // Refreshed by the SubscriberRefreshHelper
        OPERATION_IMPLICIT_REFRESH
    };

    // substate in Subscription-State header
    enum
    {
        SUB_STATE_INIT = 0,

        SUB_STATE_ACTIVE,
        SUB_STATE_PENDING,
        SUB_STATE_TERMINATED
    };

    // "reason" parameter in Subscription-State header
    enum
    {
        REASON_NONE = 0,

        REASON_DEACTIVATED,
        REASON_PROBATION,
        REASON_REJECTED,
        REASON_TIMEOUT,
        REASON_GIVEUP,
        REASON_NORESOURCE,

        REASON_MAX
    };

    enum
    {
        NO_EXPIRES = (-1)
    };

    // Runtime configuration for sub-state
    enum
    {
        CONFIG_NONE = 0x00000000,
        CONFIG_USE_INITIAL_EXPIRES_ON_NO_EXPIRES_IN_200_OK = 0x00000001,
    };

    static const IMS_CHAR STR_REASON[];
    static const IMS_CHAR STR_REASON_DEACTIVATED[];
    static const IMS_CHAR STR_REASON_PROBATION[];
    static const IMS_CHAR STR_REASON_REJECTED[];
    static const IMS_CHAR STR_REASON_TIMEOUT[];
    static const IMS_CHAR STR_REASON_GIVEUP[];
    static const IMS_CHAR STR_REASON_NORESOURCE[];

private:
    // Event package for the subscription
    EventPackage objEventPackage;

    // Main state of the subscription
    IMS_SINT32 nState;

    // Current operation for the subscription: ADD/FETCH/REFRESH/REMOVE/IMPLICIT_REFRESH
    IMS_SINT32 nOperation;

    // Runtime configuration for sub-state
    IMS_SINT32 nConfigValue;

    // Authoritative subscription duration
    //    - "Expires" header in 2XX to SUBSCRIBE request
    //    - "expires" parameter in NOTIFY request
    IMS_SINT32 nSubscriptionDuration;

    // Subscription state in NOTIFY request: active/pending/terminated
    IMS_SINT32 nSubStateValue;

    // Flag for updating the refresh timer
    IMS_BOOL bFlag_SubscriptionDurationUpdated;
    // Flag for an instant subscription
    IMS_BOOL bFlag_InstantSubscription;

    // Manages an initial SIP message for refresh/removal operation
    ISipMessage* piSIPMsg;
};

#endif  // _SUB_STATE_H_
