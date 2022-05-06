/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100518  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_SUBSCRIBER_H_
#define _REG_SUBSCRIBER_H_

#include "base/Method.h"
#include "IRegSubscription.h"
#include "util/IDialogMethod.h"
#include "util/IRefreshable.h"
#include "IRegInfoListener.h"
#include "RegKey.h"
#include "RegStateTracker.h"

class ISipConnectionNotifier;
class SipTimerValues;
class IRegistration;
class IRegSubscriptionListener;
class SubState;
class SubscriberRefreshHelper;

class RegSubscription :
        public Method,
        public IRegSubscription,
        public IDialogMethod,
        public IRefreshable,
        public IRegInfoListener
{
public:
    RegSubscription(IN CONST RegKey& objRegKey_, IN RegStateTracker* pRegStateTracker_,
            IN IMS_UINT32 nExpiresValue_ = 0, IN CONST SipTimerValues* pSIPTVs_ = IMS_NULL);
    virtual ~RegSubscription();

private:
    RegSubscription(IN CONST RegSubscription& objRHS);
    RegSubscription& operator=(IN CONST RegSubscription& objRHS);

public:
    // IRegBase interface
    virtual ISipMessage* GetNextRequest();
    virtual ISipMessage* GetPreviousRequest() const;
    virtual ISipMessage* GetPreviousResponse() const;
    // SIP_MESSAGE_MEDIATOR
    virtual void SetSipMessageMediator(IN IMessageMediator* piMediator);

    // IRegSubscription interface
    virtual void DestroyEx();
    virtual IMS_SINT32 DisableFeatures(IN IMS_SINT32 nFeatures);
    virtual IMS_SINT32 EnableFeatures(IN IMS_SINT32 nFeatures);
    virtual IMS_UINT32 GetExpires() const;
    virtual const IRegInfo* GetRegInfo() const;
    virtual IMS_SINT32 GetState() const;
    virtual IMS_RESULT SetContactParameter(
            IN CONST AString& strParameter, IN IMS_SINT32 nOperation = 0 /* (0: ADD, 1: REMOVE) */);
    virtual void SetExpires(IN IMS_UINT32 nExpires);
    virtual void SetListener(IN IRegSubscriptionListener* piListener);
    virtual void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT);
    virtual IMS_RESULT Subscribe();
    virtual IMS_RESULT Unsubscribe();

private:
    // Activity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    // Method class
    virtual IMS_BOOL InitInstance();
    // IMS_AUTH_SIP_DIGEST
    virtual IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piSCC);

    // Method class - Handle to the outgoing request / incoming response message
    virtual void NotifySIPResponse(IN ISipClientConnection* piSCC);
    virtual void NotifySIPError(
            IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage);

    // IDialogMethod interface
    virtual IMS_BOOL Dialog_Compare(IN ISipServerConnection* piSSC) const;
    virtual IMS_BOOL Dialog_NotifyRequest(IN ISipServerConnection* piSSC);

    // IRefreshable interface
    virtual void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode = 0);
    virtual IMS_BOOL Refreshable_RefreshStarted();
    virtual void Refreshable_RefreshTerminated();

    // IRegInfoListener interface
    virtual void RegInfo_Updated(IN IMS_BOOL bStale = IMS_FALSE);
    virtual void RegInfo_UpdateFailed();

    void CheckDialogNCallListener();
    void ClearNextRequest();
    IMS_BOOL CopyHeadersAndBodyParts(IN_OUT ISipMessage*& piSIPMsg);
    IMS_BOOL IsFeatureEnabled(IN IMS_SINT32 nFeature) const;
    void SetOngoingConnection(IN ISipClientConnection* piSCC);
    void SetPreviousRequest(IN ISipMessage* piSIPMsg);
    void SetPreviousResponse(IN ISipMessage* piSIPMsg);
    IMS_BOOL SendResponse(IN ISipServerConnection* piSSC, IN IMS_SINT32 nStatusCode);
    IMS_BOOL SetContactHeader(IN_OUT ISipMessage*& piSIPMsg, OUT IMS_BOOL& bIsContactGRUU);
    IMS_BOOL SetHeaders(IN_OUT ISipMessage*& piSIPMsg);
    void SetState(IN IMS_SINT32 nState);
    IMS_BOOL SubscribeOnImplicitRefresh();

    // IMS_REQUEST_URI_VALIDATION_IN_MID_DIALOG
    IMS_BOOL ValidateRequestURI(IN CONST SipAddress& objRequestURI, IN ISipDialog* piDialog) const;

    static ISipClientConnection* CreateConnection(IN RegSubscription* pRegSub);
    static IMS_UINT16 GetReasonParameter(IN ISipMessage* piMessage);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

private:
    enum
    {
        AMSG_REG_SUBSCRIPTION_NOTIFY_RECEIVED = AMSG_USER,
        AMSG_REG_SUBSCRIPTION_STARTED,
        AMSG_REG_SUBSCRIPTION_START_FAILED,
        AMSG_REG_SUBSCRIPTION_UPDATED,
        AMSG_REG_SUBSCRIPTION_UPDATE_FAILED,
        AMSG_REG_SUBSCRIPTION_REMOVED,
        AMSG_REG_SUBSCRIPTION_TERMINATED
    };

    enum
    {
        DEFAULT_EXPIRES = 600000 /* IETF : 3761 */
    };

    static const IMS_CHAR EVENT[];
    static const IMS_CHAR MEDIA_TYPE[];
    static const IMS_CHAR MEDIA_SUB_TYPE[];

#ifdef __IMS_ASYNC_XML_PARSER__
    class Notification
    {
    public:
        inline Notification() :
                nMSG(0),
                nWParam(0),
                nLParam(0)
        {
        }
        inline Notification(IN IMS_SINT32 nMSG_, IN IMS_SINT32 nWParam_, IN IMS_SINT32 nLParam_) :
                nMSG(nMSG_),
                nWParam(nWParam_),
                nLParam(nLParam_)
        {
        }
        inline Notification(IN CONST Notification& objRHS) :
                nMSG(objRHS.nMSG),
                nWParam(objRHS.nWParam),
                nLParam(objRHS.nLParam)
        {
        }
        inline ~Notification() {}

    public:
        inline Notification& operator=(IN CONST Notification& objRHS)
        {
            if (this != &objRHS)
            {
                nMSG = objRHS.nMSG;
                nWParam = objRHS.nWParam;
                nLParam = objRHS.nLParam;
            }

            return (*this);
        }

    public:
        IMS_SINT32 nMSG;
        IMS_SINT32 nWParam;
        IMS_SINT32 nLParam;
    };
#endif  // __IMS_ASYNC_XML_PARSER__

    // Runtime features
    IMS_SINT32 nFeatureSet;

    // State of Subscription
    IMS_SINT32 nState;
    IMS_UINT32 nExpiresValue;

    // Storage for pending operation for subscription
    IMS_SINT32 nPendingOperation;
    // Listener for this RegSubscription
    IRegSubscriptionListener* piListener;

    // Registration key
    RegKey objRegKey;
    // Registration State Tracker
    RCPtr<RegStateTracker> pRegStateTracker;

    // Subscription information for subscriber behavior
    SubState* pSubState;

    // Subscription refresh timer
    SubscriberRefreshHelper* pRefreshHelper;

    // Current SIP connection for abnormal cases
    ISipClientConnection* piOngoingSCC;

    // Message for the next SUBSCIRBE request
    ISipMessage* piNextRequest;
    // Message for the previous SUBSCRIBE request
    ISipMessage* piPreviousRequest;
    // Message for the previous SUBSCRIBE response
    ISipMessage* piPreviousResponse;

#ifdef __IMS_ASYNC_XML_PARSER__
    // Queue for reginfo notification
    IMSList<Notification> objNotifications;
#endif

    // Timer values of SIP transaction layer
    SipTimerValues* pSIPTVs;

    // NOTIFY_REQUEST_HANDLING_AFTER_DE_REG
    // To handle a notification properly after de-REG
    ISipConnectionNotifier* piReferredSCN;
};

#endif  // _REG_SUBSCRIBER_H_
