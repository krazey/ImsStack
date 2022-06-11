/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090622  toastops@                 Created
    </table>

    Description

*/

#ifndef _SESSION_H_
#define _SESSION_H_

#include "ServiceMethod.h"
#include "ISessionState.h"
#include "util/ICancellableMethod.h"
#include "util/IDialogMethod.h"
#include "util/IRefreshable.h"
#include "IRetransmissionHelperListener.h"
#include "CallState.h"
#define __IMS_SESSION_RETRY_TASK__
#ifdef __IMS_SESSION_RETRY_TASK__
#include "RetryCmd.h"
#include "RetryTaskHelper.h"
#include "IRetryTaskHelperListener.h"
#endif
#include "VirtualSession.h"

class ISipAckPackage;
class IRefreshListener;
class IOnSessionListener;
class ISessionParameter;
class MethodManager;
class SdpOaState;
class SessionDescriptor;
class SessionRefreshHelper;
class RetransmissionHelper;
class Capabilities;
class Reference;
class Subscription;
class Media;
class Replaces;

class Session :
        public ServiceMethod,
        public ISessionState,
        public ICancellableMethod,
        public IDialogMethod,
        public IRefreshable,
        public IRetransmissionHelperListener
#ifdef __IMS_SESSION_RETRY_TASK__
        ,
        public RetryCmd,
        public IRetryTaskHelperListener
#endif
{
public:
    explicit Session(IN Service* pService_);
    virtual ~Session();

private:
    Session(IN CONST Session& objRHS);
    Session& operator=(IN CONST Session& objRHS);

public:
    // Method class
    virtual void Destroy();
    // SIP_MESSAGE_MEDIATOR
    virtual void SetMessageMediator(IN IMessageMediator* piMediator);

    // ISession interface
    IMS_RESULT Accept();
    Capabilities* CreateCapabilities();
    Media* CreateMedia(IN CONST AString& strType, IN IMS_SINT32 nDirection,
            IN IMS_SINT32 nCountOfDescriptor = 0, IN IMS_BOOL bIMSExtension = IMS_TRUE);
    Reference* CreateReference(IN CONST AString& strReferTo, IN CONST AString& strReferMethod);
    // IMS extensions - To handle midcall or in-dialog transactions
    Subscription* CreateSubscription(IN CONST AString& strEvent);
    ISipClientConnection* CreateTransaction(IN CONST SipMethod& objMethod);
    IMS_SINT32 GetConfiguration() const;
    const ISipHeader* GetContactHeader() const;
    const Replaces* GetReplaces() const;
    const AString& GetSessionId() const;
    IMS_SINT32 GetTerminationReason() const;
    const IMSList<Media*>& GetMedia() const;
    SessionDescriptor* GetSessionDescriptor();
    IMS_SINT32 GetState() const;
    IMS_BOOL HasPendingUpdate() const;
    IMS_BOOL IsFinalResponseReceivedForInitialInviteRequest() const;
    IMS_BOOL IsReliableProvResponseSupported() const;
    IMS_BOOL IsSDPNegotiationAllowedForNonRPR() const;
    IMS_RESULT Reject();
    IMS_RESULT Reject(IN IMS_SINT32 nStatusCode);
    IMS_RESULT RejectEx(
            IN IMS_SINT32 nStatusCode, IN CONST AString& strReasonPhrase = AString::ConstNull());
    IMS_RESULT RejectWithDiversion(IN CONST AString& strAlternativeUserAddress);
    IMS_RESULT RemoveMedia(IN Media* pMedia);
    IMS_RESULT RemoveMedia(IN IMS_UINT32 nIndex);
    IMS_RESULT Restore();
    IMS_RESULT SendAck();
    IMS_RESULT SendProvisionalResponse(IN IMS_SINT32 nStatusCode,
            IN CONST AString& strReason = AString::ConstNull(), IN IMS_SINT32 nFlags = 0);
    void Set100TryingNotification(IN IMS_BOOL b100TryingNotification);
    IMS_RESULT SetCallerPreference(IN CONST IMSList<AString>& objCallerPreference);
    void SetConfiguration(IN IMS_SINT32 nConfigValue);
    // CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
    IMS_RESULT SetContactParameter(
            IN CONST AString& strParameter, IN IMS_SINT32 nOperation = 0 /* (0: ADD, 1: REMOVE) */);
    void SetImplicitRoutingRequired(IN IMS_BOOL bFlag);
    void SetReasonForCallTermination(IN IMS_SINT32 nReason);
    void SetRefreshListener(IN IRefreshListener* piListener);
    void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT);
    void SetListener(IN IOnSessionListener* piListener);
    IMS_RESULT Start();
    IMS_RESULT Terminate();
    IMS_RESULT TerminateEx(IN IMS_BOOL bTerminateMethodBYE = IMS_FALSE);
    IMS_RESULT Update();
    IMS_RESULT UpdateEx(
            IN IMS_SINT32 nMethod = SipMethod::INVALID, IN IMS_BOOL bSessionRefresh = IMS_FALSE);

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
    IMS_RESULT CreateFailureSdp();
    void DestroyFailureSdp();
    ISessionParameter* GetFailureSdp() const;
    // }
    inline VirtualSession* GetVirtualEarlySession() const { return pVirtualEarlySession.Get(); }

protected:
    // Activity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    // Method class

    // Handle the exceptions
    virtual void Exception_NotifyError(IN IMS_SINT32 nErrorCode);
    virtual IMS_BOOL InitInstance();

    // Handle the incoming request / outgoing response message
    virtual IMS_BOOL NotifySIPRequest(IN ISipServerConnection* piSSC);

    // Handle to the outgoing request / incoming response message
    virtual IMS_BOOL NotifySIPForkedResponse(
            IN ISipClientConnection* piSCC, IN ISipClientConnection* piForkedSCC);
    virtual void NotifySIPResponse(IN ISipClientConnection* piSCC);
    virtual void NotifySIPError(
            IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage);

    // IMS_AUTH_SIP_DIGEST
    virtual IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piSCC);

    // Handle the reference
    virtual IMS_BOOL SetReferredMessageListener(IN IReferredMessageListener* piListener);

    // ISessionState interface
    virtual const AString& GetConnectionAddress() const;
    virtual IMS_SINT32 GetSessionState() const;
    virtual SdpSessionParameter* GetSessionParameter() const;
    virtual const AString& GetPeerConnectionAddress() const;
    virtual SdpSessionParameter* GetPeerSessionParameter() const;
    virtual SdpSessionParameter* GetProposalSessionParameter();

    // ICancellableMethod interface
    virtual IMS_BOOL Cancellable_Compare(IN ISipServerConnection* piSSC_CANCEL) const;
    virtual IMS_BOOL Cancellable_NotifyRequest(IN ISipServerConnection* piSSC_CANCEL);

    // IDialogMethod interface
    virtual IMS_BOOL Dialog_Compare(IN ISipServerConnection* piSSC) const;
    virtual IMS_BOOL Dialog_NotifyRequest(IN ISipServerConnection* piSSC);

    // IRefreshable interface
    virtual void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode = 0);
    virtual IMS_BOOL Refreshable_RefreshStarted();
    virtual void Refreshable_RefreshTerminated();

    // IRetransmissionHelperListener interface
    virtual IMS_RESULT RetransmissionHelper_NotifyStatus(IN IMS_SINT32 nStatus);

#ifdef __IMS_SESSION_RETRY_TASK__
    // RetryCmd class
    virtual IMS_RESULT ExecuteCmd();
    // IRetryTaskHelperListener interface
    virtual void RetryTaskHelper_OnCompleted(
            IN RetryTaskHelper* pTaskHelper, IN RetryCmd* pCmd, IN IMS_SINT32 nCode = 0);
#endif

    // Session class
    virtual Session* CreateSession();
    virtual IMS_RESULT HandleProvisionalResponse(IN ISipClientConnection* piSCC);
    virtual IMS_RESULT HandleRequestToUPDATE(IN ISipServerConnection* piSSC);
    virtual IMS_RESULT HandleResponseToUPDATE(IN ISipClientConnection* piSCC);
    virtual IMS_BOOL HasPendingPRAck() const;
    virtual IMS_BOOL IsEarlyUpdateInProgress() const;

    IMS_BOOL AddRefreshSpecificHeaders(IN ISipConnection* piSC);
    IMS_BOOL CheckNSetListenerCall(IN IMS_SINT32 nListenerCall);
    // Methods for handling SDP & Session Descriptor related operations
    IMS_BOOL CheckNCreateSessionDescriptor();
    IMS_BOOL CheckNSetSDPBodyPart(IN_OUT ISipMessage*& piSIPMsg);
    IMS_BOOL CheckNTerminateSession(IN ISipMessage* piSIPMsg);
    ISipClientConnection* CreateConnectionL(IN ISipDialog* piDialog, IN CONST SipMethod& objMethod);
    IMS_SINT32 GetCallState() const;
    IMS_SINT32 GetOfferAnswerState() const;
    SessionRefreshHelper* GetRefreshHelper() const;
    IMS_SINT32 HandleSDPOfferAnswer(IN ISipMessage* piSIPMsg);

    inline IMS_BOOL IsConfigurationSet(IN IMS_SINT32 nValue) const
    {
        return (nConfigValue & nValue) != 0;
    }
    IMS_BOOL IsInviteFinalResponseReceived(IN IMS_SINT32 nServiceMethod) const;
    IMS_BOOL IsMidDialogTransactionCreatable() const;
    IMS_BOOL IsTerminatePending() const;
    void NotifyAlerting();
    void RestoreEx();
    IMS_RESULT SendResponseToRefreshUPDATE(IN ISipServerConnection* piSSC);
    IMS_BOOL SetSDPBodyPartFromCurrentView(IN_OUT ISipMessage*& piSIPMsg);
    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
    IMS_BOOL SetSDPBodyPartFromRefusedView(IN_OUT ISipMessage*& piSIPMsg);
    // }
    void SetTerminationReason(IN IMS_SINT32 nReason);

    void UpdateCallStateOnMessageReceived(
            IN CONST ISipMessage* piSIPMsg, IN IMS_SINT32 nMode = CallState::MODE_RECEIVED);
    void UpdateCallStateOnMessageSent(IN CONST ISipMessage* piSIPMsg);
    IMS_BOOL UpdateMedia(IN IMS_SINT32 nTrigger);

    IMS_BOOL RestoreOfferAnswerState();
    IMS_BOOL UpdateOfferAnswerStateOnMessageReceived(IN CONST ISipMessage* piSIPMsg);
    IMS_BOOL UpdateOfferAnswerStateOnMessageSent(IN CONST ISipMessage* piSIPMsg);
    // CALLER_PREFERENCE_MANAGER
    void UpdateCallerPreference(
            IN CONST ISipMessage* piPreviousSIPMsg, IN IMS_SINT32 nStatusCode = 200);
    static void RemoveRecordRouteHeaders(IN ISipMessage* piSIPMsg);

    inline void AddSessionToCallControlHelperIfNotPresent()
    {
        if (GetSessionId().GetLength() == 0)
        {
            AddSessionToCallControlHelper();
        }
    }

private:
    void AddSessionToCallControlHelper();
    void RemoveSessionFromCallControlHelper();

    void CleanupOnDestroy();

    // For UAS behavior
    IMS_RESULT HandleRequestToACK(IN ISipServerConnection* piSSC);
    IMS_RESULT HandleRequestToBYE(IN ISipServerConnection* piSSC);
    IMS_RESULT HandleRequestToCANCEL(IN ISipServerConnection* piSSC);
    IMS_RESULT HandleRequestToINVITE(IN ISipServerConnection* piSSC);
    IMS_RESULT HandleRequestToINVITEWithinDialog(IN ISipServerConnection* piSSC);
    IMS_RESULT HandleRequestToREFER(IN ISipServerConnection* piSSC);

    // For UAC behavior
    IMS_RESULT HandleResponseToBYE(IN ISipClientConnection* piSCC);
    IMS_RESULT HandleResponseToCANCEL(IN ISipClientConnection* piSCC);
    IMS_RESULT HandleResponseToINVITE(IN ISipClientConnection* piSCC);

    IMS_BOOL IsSessionUpdateNotificationInProgress() const;

    // ACK_RETRANSMISSION_TO_2XX
    void RemoveStrayAcks();
    SipMethod SelectUpdateMethod() const;

    // For UAC behavior
    IMS_RESULT SendRequestForRefresh(IN IMS_SINT32 nMethod = SipMethod::INVALID);
    IMS_RESULT SendRequestToACK(IN ISipClientConnection* piSCC, IN IMS_SINT32 nServiceMethod);
    IMS_RESULT SendRequestToBYE();
    IMS_RESULT SendRequestToBYEInternal();
    IMS_RESULT SendRequestToCANCEL();
    IMS_RESULT SendRequestToINVITE(IN IMS_BOOL bSessionRefresh = IMS_FALSE);
    IMS_RESULT SendRequestToINVITEOn422Received();
    IMS_RESULT SendRequestToUPDATE(IN IMS_BOOL bSessionRefresh = IMS_FALSE);

    // For UAS behavior
    IMS_RESULT SendResponseEx(IN ISipServerConnection* piSSC, IN IMS_SINT32 nServiceMethod,
            IN IMS_SINT32 nStatusCode);

    void SetReasonHeaderFromPreviousRequest(IN IMS_SINT32 nRequest);
    // RACE_CONDITION : SESSION_UPDATE
    void SetSessionUpdateNotificationState(IN IMS_BOOL bInProgress);
    void SetState(IN IMS_SINT32 nState);
    void Start2xxRetransmission();
    void Stop2xxRetransmission();

    void TerminateOnNegotiating();
    void TerminateOnEstablishing();
    void TerminateOnReNegotiating();
    void TerminateOnReEstablishing();

    // RACE_CONDITION : 200 OK to CANCEL and 200 OK to forked INVITE
    void TerminateForkedSessionsOnNegotiating();
    void TerminateForkedSession();

    // Methods for handling SDP & Media related operations
    IMS_BOOL AddMedia(IN Media* pMedia);
    void CleanupMedia();
    IMS_BOOL CreateMediaFromSDP();
    IMS_BOOL IsMediaInitializationDone() const;
    IMS_BOOL UpdateMediaOnAnswerReceived(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnAnswerSent(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnOfferReceived(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnOfferSent(IN IMS_SINT32 nTrigger);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // Refer to ISession class
    enum
    {
        STATE_CREATED = 0,
        STATE_INITIATED = 1,
        STATE_NEGOTIATING = 2,
        STATE_ESTABLISHING = 3,
        STATE_ESTABLISHED = 4,
        STATE_RENEGOTIATING = 5,
        STATE_REESTABLISHING = 6,
        STATE_TERMINATING = 7,
        STATE_TERMINATED = 8
    };

    // Refer to ISession class
    enum
    {
        STATUSCODE_433_ANONYMITY_DISALLOWED = 433,
        STATUSCODE_480_TEMPORARILY_UNAVAILABLE = 480,
        STATUSCODE_486_BUSY_HERE = 486,
        STATUSCODE_488_NOT_ACCEPTABLE_HERE = 488,
        STATUSCODE_600_BUSY_EVERYWHERE = 600,
        STATUSCODE_603_DECLINE = 603
    };

    // Refer to ISession class
    enum
    {
        TERMINATION_REASON_INVALID = (-1),

        // Unknown reason
        TERMINATION_REASON_UNKNOWN = 0,
        // CANCEL or BYE is sent by the user
        TERMINATION_REASON_USER_ACTION,
        // CANCEL or BYE is received from the remote user or CSCF
        TERMINATION_REASON_REMOTE_ACTION,
        // Session refresh was failed by 408 response
        TERMINATION_REASON_REFRESH_408,
        // Session refresh was failed by 481 response
        TERMINATION_REASON_REFRESH_481,
        // Session refresh was failed by the transaction timeout
        TERMINATION_REASON_REFRESH_TXN_TIMEOUT,
        // Session refresh was failed by the session refresh timeout
        TERMINATION_REASON_REFRESH_TIMEOUT,
        // Session is terminated by the de-registration (by user or network)
        TERMINATION_REASON_SERVICE_CLOSED,

        TERMINATION_REASON_MAX
    };

    // Policy for session refresh
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

    // Option flags for sending SIP request/response
    enum
    {
        FLAG_NONE = 0x0000,
        FLAG_REMOVE_RECORD_ROUTES = 0x0001
    };

    // Runtime configuration for session control
    enum
    {
        CONFIG_NONE = 0x00000000,
        // RFC 6337, Section 3.1.1
        // This will be applied for MO signaling.
        CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE = 0x00000001,
        // This is to avoid the session update failure when the service is disconnected by re-REG.
        CONFIG_IGNORE_DEREG_ON_SESSION_UPDATE = 0x00000002,
        // This is to override SIP configuration for SDP handling of non-RPR.
        CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR = 0x00000004,
        // This is to indicate that early session model is supported.
        // ex. VRBT (Video RingBack Tone) for KR operators
        CONFIG_SUPPORT_EARLY_SESSION_MODEL = 0x00000008
    };

protected:
    enum
    {
        AMSG_SESSION_INVITATION_RECEIVED = AMSG_USER,
        AMSG_SESSION_ALERTING,
        AMSG_SESSION_REFERENCE_RECEIVED,
        AMSG_SESSION_STARTED,
        AMSG_SESSION_START_FAILED,
        AMSG_SESSION_TERMINATED,
        AMSG_SESSION_UPDATED,
        AMSG_SESSION_UPDATE_FAILED,
        AMSG_SESSION_UPDATE_RECEIVED,
        AMSG_SESSION_CANCEL_DELIVERED,
        AMSG_SESSION_CANCEL_DELIVERY_FAILED,
        AMSG_SESSION_FORKED_RESPONSE_RECEIVED,
        AMSG_SESSION_PROVISIONAL_RESPONSE_RECEIVED,
        AMSG_SESSION_TRANSACTION_RECEIVED,
        AMSG_SESSION_DELAYED_DIALOG_TRANSACTION_RECEIVED,
        AMSG_SESSION_MAX
    };

    // To guard the duplicated listener call
    enum
    {
        LISTENER_CALL_START_FAILED = 0x00000001,
        LISTENER_CALL_TERMINATED = 0x00000002,
        LISTENER_CALL_STARTED = 0x00000004
    };

private:
    IMS_SINT32 nState;
    // Call state
    CallState objCallState;

    IMS_BOOL bFlag_Alerting;
    IMS_BOOL bFlag_SDPInInitialINVITE;
    IMS_BOOL bFlag_TerminatePending;
    IMS_BOOL bFlag_UpdateRequestor;
    IMS_BOOL bFlag_SDPNonRPRAllowed;
    IMS_BOOL bFlag_100TryingNotification;
    // TERMINATE_METHOD - default : CANCEL; it can be a BYE by enabler
    IMS_BOOL bFlag_TerminateMethodBYE;
    // RACE_CONDITION : SESSION_UPDATE
    // Checks if the result of session-update request is notified or not
    IMS_BOOL bFlag_SessionUpdateNotificationInProgress;
    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    IMS_BOOL bFlag_ImplicitRoutingRequired;
    // FIX_TIMING_ISSUE: ACK_WITH_SDP_IN_PROGRESS
    IMS_BOOL bFlag_AckWithSDPInProgress;

    // Runtime configuration for session control
    IMS_SINT32 nConfigValue;

    // Manages completed listener calls
    IMS_SINT32 nCompletedListenerCalls;

    // Reason of session termination (refer to ISession::TERMINATION_REASON_XXX)
    IMS_SINT32 nTerminationReason;
    AString strTerminationReasonFromApp;

    SdpOaState* pOAState;
    SessionDescriptor* pSessionDescriptor;
    IMSList<Media*> objMedias;

    IOnSessionListener* piSessionListener;

    // For session refresh
    IRefreshListener* piRefreshListener;
    SessionRefreshHelper* pRefreshHelper;
    // For 2xx retransmission when receiving an incoming INVITE request
    RetransmissionHelper* pRetransmissionHelper;
    // For call transfer/hold/ ...
    IReferredMessageListener* piReferredMessageListener;

#ifdef __IMS_SESSION_RETRY_TASK__
    RetryTaskHelper* pRetransmissionTask;
#endif
    // ACK_RETRANSMISSION_TO_2XX
    ISipAckPackage* piAckPackage;

    // Remote session id for 3rd-party call control
    AString strSessionIdForCallControl;

    // For internal BYE transaction
    ISipClientConnection* piSCC_BYE;

    // CALLER_PREFERENCE_MANAGER
    IMSList<AString> objPreviousCallerPreference;

    // Management of forked session to handle PRACK
    RCPtr<MethodManager> pForkedSessions;

    // EARLY_SESSION_MODEL
    RCPtr<VirtualSession> pVirtualEarlySession;
};

#endif  // _SESSION_H_
