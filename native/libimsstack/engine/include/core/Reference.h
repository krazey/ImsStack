/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100413  hwangoo.park@             Created
    </table>

    Description

     RFC 3515, RFC 3420, RFC 3725, RFC 3891, RFC 4488, RFC 4508, RFC 5368, RFC 3265
*/

#ifndef _REFERENCE_H_
#define _REFERENCE_H_

#include "SipMethod.h"
#include "ServiceMethod.h"
#include "util/IDialogMethod.h"
#include "ISubscriptionState.h"
#include "util/IReferredMessageListener.h"
#include "Replaces.h"

class IOnNotificationListener;
class IOnReferenceListener;
class SubState;



class Reference
    : public ServiceMethod
    , public IDialogMethod
    , public IReferredMessageListener
{
public:
    class NotifierState
    {
    public:
        NotifierState();
        ~NotifierState();

    private:
        NotifierState(IN CONST NotifierState &objRHS);
        NotifierState& operator=(IN CONST NotifierState &objRHS);

    public:
        void AddSCC(IN ISIPClientConnection *piSCC);
        void RemoveSCC(IN ISIPClientConnection *piSCC);

    private:
        IMSList<ISIPClientConnection*> objSCCs;
    };

public:
    Reference(IN Service *pService_, IN CONST AString &strReferToURI_,
            IN CONST AString &strReferMethod_, IN CONST Replaces &objReplaces_,
            IN IMS_BOOL bImplicitRoutingRequired_ = IMS_FALSE);
    virtual ~Reference();

private:
    Reference(IN CONST Reference &objReference);
    Reference& operator=(IN CONST Reference &objReference);

public:
    // Method class
    virtual void Destroy();

    // IReference interface
    IMS_RESULT Accept();
    IMS_RESULT ConnectReferMethod(IN Method *pReferMethod);
    const SIPMethod& GetReferMethod() const;
    const AString& GetReferToUserId() const;
    const AString& GetReplaces() const;
    IMS_SINT32 GetState() const;
    IMS_RESULT Refer(IN IMS_BOOL bImplicitSubscription);
    IMS_RESULT Reject();
    void SetListener(IN IOnReferenceListener *piListener);
    IMS_RESULT SetReplaces(IN CONST AString &strSessionId);

    //// IMS extensions
    IMS_RESULT AcceptEx(IN IMS_SINT32 nStatusCode = 202, IN IMS_BOOL b100Trying = IMS_TRUE);
    IMS_RESULT ReferEx(IN IMS_BOOL bImplicitSubscription,
            IN CONST AString &strHeadersForReferTo = AString::ConstNull());
    IMS_RESULT RejectEx(IN IMS_SINT32 nStatusCode);
    IMS_RESULT SendNotification(IN IMS_SINT32 nSubState, IN CONST ByteArray &objContent,
            IN IMS_SINT32 nReason = ISubscriptionState::REASON_NONE,
            IN IMS_SINT32 nExpires = (-1));
    void SetNotificationListener(IN IOnNotificationListener *piListener);
    void SetImplicitRoutingRequired(IN IMS_BOOL bFlag);

protected:
    // Activity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG &objMSG);

    // Method class
    // IMS_AUTH_SIP_DIGEST
    virtual IMS_BOOL SendRequestToChallenge(IN ISIPClientConnection *piSCC);

    // Handle the exceptions
    virtual void Exception_NotifyError(IN IMS_SINT32 nErrorCode);
    virtual IMS_BOOL InitInstance();

    // Handle the incoming request / outgoing response message
    virtual IMS_BOOL NotifySIPRequest(IN ISIPServerConnection *piSSC);

    // Handle to the outgoing request / incoming response message
    virtual void NotifySIPResponse(IN ISIPClientConnection *piSCC);
    virtual void NotifySIPError(IN ISIPConnection *piSC, IN IMS_SINT32 nCode,
            IN CONST AString &strMessage);

    // IDialogMethod interface
    virtual IMS_BOOL Dialog_Compare(IN ISIPServerConnection *piSSC) const;
    virtual IMS_BOOL Dialog_NotifyRequest(IN ISIPServerConnection *piSSC);

    // IReferredMessageListener interface
    virtual void ReferredMessage_NotifyOnActive(IN ISIPMessage *piSIPMsg);
    virtual void ReferredMessage_NotifyOnTerminated(
            IN IMS_SINT32 nReasonCode = SubState::REASON_NONE,
            IN ISIPMessage *piSIPMsg = IMS_NULL);

private:
    void CleanupOnDestroy();
    ISIPClientConnection* CreateConnectionL(IN ISIPDialog *piDialog,
            IN CONST SIPMethod &objMethod);
    IMS_RESULT DoNotification(IN IMS_SINT32 nSubState, IN CONST ByteArray &objContent,
            IN IMS_SINT32 nReasonCode = SubState::REASON_NONE,
            IN IMS_SINT32 nExpires = (-1));
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // Refer to IReference class
    enum
    {
        STATE_INITIATED = 1,
        STATE_PROCEEDING = 2,
        STATE_REFERRING = 3,
        STATE_TERMINATED = 4
    };

    static const IMS_CHAR EVENT_REFER[];
    static const IMS_CHAR MEDIA_TYPE[];

protected:
    enum
    {
        AMSG_REFERENCE_DELIVERED = AMSG_USER,
        AMSG_REFERENCE_DELIVERY_FAILED,
        AMSG_REFERENCE_NOTIFY,
        AMSG_REFERENCE_TERMINATED,
        AMSG_NOTIFICATION_DELIVERED,
        AMSG_NOTIFICATION_DELIVERY_FAILED,
        AMSG_REFERENCE_MAX
    };

private:
    // State of Reference
    IMS_SINT32 nState;
    // Target URI in Refer-To header
    AString strReferToURI;
    // method uri parameter in Refer-To header
    SIPMethod objReferMethod;
    // replaces uri-header parameter in Refer-To header
    // It contains callid, from-tag, to-tag
    Replaces *pReplaces;
    // User's request to the implicit subscription
    IMS_BOOL bFlag_ImplicitSubscription;
    // Referred method
    Method *pReferredMethod;
    // Listener for this reference
    IOnReferenceListener *piListener;

    // Subscription state
    SubState *pSubState;
    // Flag to indicate that the reference is created inside of any dialog (INVITE)
    IMS_BOOL bFlag_ReferenceInOtherDialog;
    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    IMS_BOOL bFlag_ImplicitRoutingRequired;

    // Queue for NOTIFY request messages
    IMSList<Message*> objNotifyMessages;

    // Notification listener for notifier's behavior
    IOnNotificationListener *piNotificationListener;
    // Notifier's state
    NotifierState *pNotifierState;
};

#endif // _REFERENCE_H_
