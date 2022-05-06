/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100328  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SUBSCRIPTION_H_
#define _SUBSCRIPTION_H_

#include "ServiceMethod.h"
#include "util/IDialogMethod.h"
#include "util/IForkedDialogMethod.h"
#include "util/IRefreshable.h"

class IRefreshListener;
class IOnSubscriptionListener;
class SubState;
class SubscriberRefreshHelper;

class Subscription :
        public ServiceMethod,
        public IDialogMethod,
        public IForkedDialogMethod,
        public IRefreshable
{
public:
    Subscription(IN Service* pService_, IN CONST AString& strEvent_,
            IN IMS_BOOL bImplicitRoutingRequired_ = IMS_FALSE);
    virtual ~Subscription();

private:
    Subscription(IN CONST Subscription& objRHS);
    Subscription& operator=(IN CONST Subscription& objRHS);

public:
    // Method class
    virtual void Destroy();
    // SIP_MESSAGE_MEDIATOR
    virtual void SetMessageMediator(IN IMessageMediator* piMediator);

    // ISubscription interface
    const AString& GetEvent() const;
    IMS_SINT32 GetState() const;
    IMS_RESULT Poll();
    void SetListener(IN IOnSubscriptionListener* piListener);
    IMS_RESULT Subscribe();
    IMS_RESULT Unsubscribe();

    //// IMS extensions
    void SetImplicitRoutingRequired(IN IMS_BOOL bFlag);
    void SetRefreshListener(IN IRefreshListener* piListener);
    void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT);

protected:
    // Activity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    // Method class
    // IMS_AUTH_SIP_DIGEST
    virtual IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piSCC);

    // Handle the exceptions
    virtual void Exception_NotifyError(IN IMS_SINT32 nErrorCode);
    virtual IMS_BOOL InitInstance();

    // Handle to the outgoing request / incoming response message
    virtual void NotifySIPResponse(IN ISipClientConnection* piSCC);
    virtual void NotifySIPError(
            IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage);

    // IDialogMethod interface
    virtual IMS_BOOL Dialog_Compare(IN ISipServerConnection* piSSC) const;
    virtual IMS_BOOL Dialog_NotifyRequest(IN ISipServerConnection* piSSC);

    // IForkedDialogMethod interface
    virtual IMS_BOOL ForkedDialog_Compare(IN ISipDialog* piOrigDialog) const;
    virtual IMS_BOOL ForkedDialog_NotifyRequest(IN ISipServerConnection* piSSC);

    // IRefreshable interface
    virtual void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode = 0);
    virtual IMS_BOOL Refreshable_RefreshStarted();
    virtual void Refreshable_RefreshTerminated();

private:
    void CheckDialogNCallListener();
    void CleanupOnDestroy();
    void CloseConnection();
    ISipClientConnection* CreateConnectionL(IN ISipDialog* piDialog, IN CONST SipMethod& objMethod);
    void SetState(IN IMS_SINT32 nState);
    void UpdateResponse(IN ISipClientConnection* piSCC);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // Refer to ISubscription class
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING = 2,
        STATE_ACTIVE = 3
    };

    // Policy for subscription refresh
    enum
    {
        // No refresh by engine
        REFRESH_POLICY_NO_REFRESH = (-1),

        // Default policy; Select the refresh time according to 3GPP spec.
        //     nCriteriaInterval : Criteria value for the refresh duration
        //    nValueEorLT : Ratio when the refresh duration is equal or less
        //              than the criteria interval (1 ~ 100; default 50)
        //    nValueGT : Interval value when the refresh duration is greater
        //              than the criteria interval
        REFRESH_POLICY_SPEC = 0,

        // Set the remain time before it is expired
        //    nCriteriaInterval : Criteria value for the refresh duration
        //    nValueEorLT : Interval value when the refresh duration is equal or less
        //              than the criteria interval
        //    nValueGT : Interval value when the refresh duration is greater
        //              than the criteria interval
        REFRESH_POLICY_REMAIN_TIME,

        // Set the ratio before it is expired
        //    nCriteriaInterval : Criteria value for the refresh duration
        //    nValueEorLT : Ratio when the refresh duration is equal or less
        //              than the criteria interval (1 ~ 100)
        //    nValueGT : Ratio when the refresh duration is greater
        //              than the criteria interval (1 ~ 100)
        // Ex) Expires: 3600, Ratio: 10
        //        -> Refresh timer is expired after 3240s
        REFRESH_POLICY_RATIO
    };

protected:
    enum
    {
        AMSG_SUBSCRIPTION_NOTIFY_RECEIVED = AMSG_USER,
        AMSG_SUBSCRIPTION_STARTED,
        AMSG_SUBSCRIPTION_START_FAILED,
        AMSG_SUBSCRIPTION_TERMINATED,
        AMSG_SUBSCRIPTION_FORKED_NOTIFY_RECEIVED,
        AMSG_SUBSCRIPTION_MAX
    };

private:
    // State of Subscription
    IMS_SINT32 nState;
    // Event package name for this subscription
    AString strEvent;
    // Storage for pending operation for subscription
    IMS_SINT32 nPendingOperation;
    // Listener for this subscription
    IOnSubscriptionListener* piListener;

    // Subscription information for subscriber behavior
    SubState* pSubState;

    // Subscription refresh timer
    IRefreshListener* piRefreshListener;
    SubscriberRefreshHelper* pRefreshHelper;

    // Queue for NOTIFY request messages
    IMSList<Message*> objNotifyMessages;

    // For forked NOTIFY request
    IMSList<Subscription*> objForkedSubscriptions;

    // Flag to indicate that the subscription is created inside of any dialog (INVITE)
    IMS_BOOL bFlag_SubscriptionInOtherDialog;
    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    IMS_BOOL bFlag_ImplicitRoutingRequired;
};

#endif  // _SUBSCRIPTION_H_
