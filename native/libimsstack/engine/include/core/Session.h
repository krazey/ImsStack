/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SESSION_H_
#define SESSION_H_

#include "CallState.h"
#include "IRetryTaskHelperListener.h"
#include "ISessionState.h"
#include "RetryCmd.h"
#include "RetryTaskHelper.h"
#include "ServiceMethod.h"
#include "SipStatusCode.h"
#include "VirtualSession.h"
#include "util/ICancellableMethod.h"
#include "util/IDialogMethod.h"
#include "util/IRefreshable.h"
#include "util/MethodManager.h"

class Capabilities;
class IOnSessionListener;
class IReasonHeaderSetter;
class IRefreshListener;
class ISessionParameter;
class Media;
class Publication;
class Reference;
class Replaces;
class SdpOaState;
class SdpReader;
class SessionDescriptor;
class SessionRefreshHelper;
class Subscription;

class Session :
        public ServiceMethod,
        public ISessionState,
        public ICancellableMethod,
        public IDialogMethod,
        public IRefreshable,
        public RetryCmd,
        public IRetryTaskHelperListener
{
public:
    explicit Session(IN Service* pService);
    ~Session() override;

    Session(IN const Session&) = delete;
    Session& operator=(IN const Session&) = delete;

public:
    // Method class
    void Destroy() override;
    void SetMessageMediator(IN IMessageMediator* piMediator) override;

    // ISession interface
    IMS_RESULT Accept();
    Capabilities* CreateCapabilities();
    Media* CreateMedia(IN const AString& strType, IN IMS_SINT32 nDirection,
            IN IMS_SINT32 nCountOfDescriptor = 0);
    Reference* CreateReference(IN const AString& strReferTo, IN const AString& strReferMethod);
    // To handle midcall or in-dialog transactions
    Subscription* CreateSubscription(IN const AString& strEvent);
    Publication* CreatePublication(IN const AString& strEvent);
    ISipClientConnection* CreateTransaction(IN const SipMethod& objMethod);
    const ISipHeader* GetContactHeader() const;
    const Replaces* GetReplaces() const;
    inline const AString& GetSessionId() const { return m_strSessionIdForCallControl; }
    IMS_SINT32 GetTerminationReason() const;
    const ImsList<Media*>& GetMedia() const;
    SessionDescriptor* GetSessionDescriptor();
    inline IMS_SINT32 GetState() const { return m_nState; }
    IMS_BOOL HasPendingUpdate() const;
    IMS_BOOL IsFinalResponseReceivedForInitialInviteRequest() const;
    IMS_BOOL IsReliableProvResponseSupported() const;
    inline IMS_BOOL IsSdpNegotiationAllowedForNonRpr() const { return m_bSdpNonRprAllowed; }
    IMS_BOOL IsSdpOaInPreviewMode() const;
    IMS_RESULT Reject();
    IMS_RESULT Reject(IN IMS_SINT32 nStatusCode);
    IMS_RESULT RejectEx(
            IN IMS_SINT32 nStatusCode, IN const AString& strReasonPhrase = AString::ConstNull());
    IMS_RESULT RejectWithDiversion(IN const AString& strAlternativeUserAddress);
    IMS_RESULT RemoveMedia(IN Media* pMedia);
    IMS_RESULT RemoveMedia(IN IMS_UINT32 nIndex);
    IMS_RESULT Restore();
    IMS_RESULT SendAck();
    IMS_RESULT SendProvisionalResponse(IN IMS_SINT32 nStatusCode,
            IN const AString& strReason = AString::ConstNull(), IN IMS_SINT32 nFlags = 0);
    IMS_RESULT SetCallerPreference(IN const ImsList<AString>& objCallerPreference);
    // CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
    IMS_RESULT SetContactParameter(
            IN const AString& strParameter, IN IMS_SINT32 nOperation = 0 /* (0: ADD, 1: REMOVE) */);
    void SetImplicitRoutingRequired(IN IMS_BOOL bFlag);
    inline void SetReasonForCallTermination(IN IMS_SINT32 nReason)
    {
        m_strTerminationReasonFromApp.Sprintf("%x", nReason);
    }
    inline void SetRefreshListener(IN IRefreshListener* piListener)
    {
        m_piRefreshListener = piListener;
    }
    void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt);
    inline void SetListener(IN IOnSessionListener* piListener) { m_piSessionListener = piListener; }
    IMS_RESULT Start();
    IMS_RESULT Terminate();
    IMS_RESULT TerminateEx(IN IMS_BOOL bTerminateMethodBye = IMS_FALSE);
    IMS_RESULT Update();
    IMS_RESULT UpdateEx(
            IN IMS_SINT32 nMethod = SipMethod::INVALID, IN IMS_BOOL bSessionRefresh = IMS_FALSE);

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
    IMS_RESULT CreateFailureSdp();
    void DestroyFailureSdp();
    ISessionParameter* GetFailureSdp() const;
    // }
    inline VirtualSession* GetVirtualEarlySession() const { return m_pVirtualEarlySession.Get(); }

    inline IMS_SINT32 GetConfiguration() const { return m_nConfigValue; }
    inline void SetConfiguration(IN IMS_SINT32 nConfigValue) { m_nConfigValue = nConfigValue; }
    inline IMS_BOOL IsConfigurationSet(IN IMS_SINT32 nValue) const
    {
        return (m_nConfigValue & nValue) != 0;
    }
    IMS_BOOL IsSessionRefreshInProgress() const;
    inline void SetReasonHeaderSetter(IN IReasonHeaderSetter* piSetter)
    {
        m_piReasonHeaderSetter = piSetter;
    }
    inline SdpReader* GetRemoteMediaCapabilities() const { return m_pRemoteMediaCapabilities; }
    inline IMS_BOOL IsSessionCanceledOnAccepted() const { return m_bSessionCanceledOnAccepted; }

protected:
    // Activity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // Method class
    // Handle the exceptions
    void Exception_NotifyError(IN IMS_SINT32 nErrorCode) override;
    IMS_BOOL InitInstance() override;
    // Handle the incoming request / outgoing response message
    IMS_BOOL NotifySipRequest(IN ISipServerConnection* piSsc) override;
    // Handle to the outgoing request / incoming response message
    IMS_BOOL NotifySipForkedResponse(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc) override;
    void NotifySipResponse(IN ISipClientConnection* piScc) override;
    void NotifySipError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;
    IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piScc) override;
    IMS_BOOL SetReferredMessageListener(IN IReferredMessageListener* piListener) override;

    // ISessionState interface
    const AString& GetConnectionAddress() const override;
    IMS_SINT32 GetSessionState() const override;
    SdpSessionParameter* GetSessionParameter() const override;
    const AString& GetPeerConnectionAddress() const override;
    SdpSessionParameter* GetPeerSessionParameter() const override;
    SdpSessionParameter* GetProposalSessionParameter() override;

    // ICancellableMethod interface
    IMS_BOOL Cancellable_Compare(IN ISipServerConnection* piSscCancel) const override;
    IMS_BOOL Cancellable_NotifyRequest(IN ISipServerConnection* piSscCancel) override;

    // IDialogMethod interface
    IMS_BOOL Dialog_Compare(IN ISipServerConnection* piSsc) const override;
    IMS_BOOL Dialog_NotifyRequest(IN ISipServerConnection* piSsc) override;

    // IRefreshable interface
    void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piScc, IN IMS_SINT32 nCode = 0) override;
    IMS_BOOL Refreshable_RefreshStarted() override;
    void Refreshable_RefreshTerminated() override;

    // RetryCmd class
    IMS_RESULT ExecuteCmd() override;
    // IRetryTaskHelperListener interface
    void RetryTaskHelper_OnCompleted(
            IN RetryTaskHelper* pTaskHelper, IN RetryCmd* pCmd, IN IMS_SINT32 nCode = 0) override;

    // Session class
    virtual Session* CreateSession();
    virtual SessionRefreshHelper* CreateRefreshHelper();
    virtual IMS_RESULT HandleProvisionalResponse(
            IN ISipClientConnection* piScc, IN IMS_SINT32 nServiceMethod);
    virtual IMS_RESULT HandleRequestToUpdate(IN ISipServerConnection* piSsc);
    virtual IMS_RESULT HandleResponseToUpdate(IN ISipClientConnection* piScc);
    inline virtual IMS_BOOL HasPendingPrack() const { return IMS_FALSE; }
    inline virtual IMS_BOOL IsEarlyUpdateInProgress() const { return IMS_FALSE; }

    IMS_BOOL AddRefreshSpecificHeaders(IN ISipConnection* piSc);
    IMS_BOOL CheckNSetListenerCall(IN IMS_SINT32 nListenerCall);
    // Methods for handling SDP & Session Descriptor related operations
    IMS_BOOL CheckNCreateSessionDescriptor();
    IMS_BOOL CheckNSetSdpBodyPart(IN_OUT ISipMessage*& piSipMsg);
    IMS_BOOL CheckNTerminateSession(IN const ISipMessage* piSipMsg);
    ISipClientConnection* CreateConnectionL(IN ISipDialog* piDialog, IN const SipMethod& objMethod);
    inline IMS_SINT32 GetCallState() const { return m_objCallState.GetState(); }
    IMS_SINT32 GetOfferAnswerState() const;
    inline SessionRefreshHelper* GetRefreshHelper() const { return m_pRefreshHelper; }
    IMS_SINT32 HandleSdpOfferAnswer(IN const ISipMessage* piSipMsg);
    IMS_BOOL IsInviteFinalResponseReceived(IN IMS_SINT32 nServiceMethod) const;
    IMS_BOOL IsMidDialogTransactionCreatable() const;
    inline IMS_BOOL IsTerminatePending() const { return m_bTerminatePending; }
    void NotifyAlerting();
    void RestoreEx();
    IMS_RESULT SendResponseToRefreshUpdate(IN ISipServerConnection* piSsc);
    IMS_BOOL SetSdpBodyPartFromCurrentView(IN_OUT ISipMessage*& piSipMsg);
    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
    IMS_BOOL SetSdpBodyPartFromRefusedView(IN_OUT ISipMessage*& piSipMsg);
    // }
    void SetTerminationReason(IN IMS_SINT32 nReason);

    inline void UpdateCallStateOnMessageReceived(
            IN const ISipMessage* piSipMsg, IN IMS_SINT32 nMode = CallState::MODE_RECEIVED)
    {
        m_objCallState.UpdateState(piSipMsg, nMode);
    }
    inline void UpdateCallStateOnMessageSent(IN const ISipMessage* piSipMsg)
    {
        m_objCallState.UpdateState(piSipMsg, CallState::MODE_SENT);
    }
    IMS_BOOL UpdateMedia(IN IMS_SINT32 nTrigger);

    IMS_BOOL RestoreOfferAnswerState();
    IMS_BOOL UpdateOfferAnswerStateOnMessageReceived(IN const ISipMessage* piSipMsg);
    IMS_BOOL UpdateOfferAnswerStateOnMessageSent(IN const ISipMessage* piSipMsg);
    // CALLER_PREFERENCE_MANAGER
    void UpdateCallerPreference(
            IN const ISipMessage* piPrevSipMsg, IN IMS_SINT32 nStatusCode = SipStatusCode::SC_200);

    void SetState(IN IMS_SINT32 nState);
    // UAC behavior
    IMS_RESULT SendRequestToByeInternal();

    static void RemoveRecordRouteHeaders(IN ISipMessage* piSipMsg);

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
    IMS_RESULT HandleRequestToAck(IN ISipServerConnection* piSsc);
    IMS_RESULT HandleRequestToBye(IN ISipServerConnection* piSsc);
    IMS_RESULT HandleRequestToCancel(IN ISipServerConnection* piSsc);
    IMS_RESULT HandleRequestToInvite(IN ISipServerConnection* piSsc);
    IMS_RESULT HandleRequestToInviteWithinDialog(IN ISipServerConnection* piSsc);
    IMS_RESULT HandleRequestToRefer(IN ISipServerConnection* piSsc);

    // For UAC behavior
    IMS_RESULT HandleResponseToBye(IN ISipClientConnection* piScc);
    IMS_RESULT HandleResponseToCancel(IN const ISipClientConnection* piScc);
    IMS_RESULT HandleResponseToInvite(IN ISipClientConnection* piScc);

    // RACE_CONDITION : SESSION_UPDATE
    inline IMS_BOOL IsSessionUpdateNotificationInProgress() const
    {
        return m_bSessionUpdateNotificationInProgress;
    }
    inline void SetSessionUpdateNotificationInProgress(IN IMS_BOOL bInProgress)
    {
        m_bSessionUpdateNotificationInProgress = bInProgress;
    }

    SipMethod SelectUpdateMethod() const;

    // For UAC behavior
    IMS_RESULT SendRequestForRefresh(IN IMS_SINT32 nMethod = SipMethod::INVALID);
    IMS_RESULT SendRequestToAck(IN ISipClientConnection* piScc, IN IMS_SINT32 nServiceMethod);
    IMS_RESULT SendRequestToBye();
    IMS_RESULT SendRequestToCancel();
    IMS_RESULT SendRequestToInvite(IN IMS_BOOL bSessionRefresh = IMS_FALSE);
    IMS_RESULT SendRequestToInviteOn422Received();
    IMS_RESULT SendRequestToUpdate(IN IMS_BOOL bSessionRefresh = IMS_FALSE);

    // For UAS behavior
    IMS_RESULT SendResponseEx(IN ISipServerConnection* piSsc, IN IMS_SINT32 nServiceMethod,
            IN IMS_SINT32 nStatusCode);

    void SetReasonHeaderFromPreviousRequest(IN IMS_SINT32 nRequest);
    void Start2xxRetransmission();
    void Stop2xxRetransmission();

    void TerminateOnNegotiating();
    inline void TerminateOnEstablishing() { m_bTerminatePending = IMS_TRUE; }
    void TerminateOnReNegotiating();
    inline void TerminateOnReEstablishing() { m_bTerminatePending = IMS_TRUE; }

    // RACE_CONDITION : 200 OK to CANCEL and 200 OK to forked INVITE
    void TerminateForkedSessionsOnNegotiating();
    void TerminateForkedSession();
    void ClearForkedSessionsByTerminated();
    void HandleForkedSessionTerminated();

    // Methods for handling SDP & Media related operations
    IMS_BOOL AddMedia(IN Media* pMedia);
    void CleanupMedia();
    void CreateMediaFromSdp();
    IMS_BOOL IsMediaInitializationDone() const;
    IMS_BOOL UpdateMediaOnAnswerReceived(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnAnswerSent(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnOfferReceived(IN IMS_SINT32 nTrigger);
    IMS_BOOL UpdateMediaOnOfferSent(IN IMS_SINT32 nTrigger);
    void CreateRemoteMediaCapabilities(IN const ISipMessage* piSipMsg);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    /// Refer to ISession class
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

    /// Refer to ISession class
    enum
    {
        STATUSCODE_433_ANONYMITY_DISALLOWED = 433,
        STATUSCODE_480_TEMPORARILY_UNAVAILABLE = 480,
        STATUSCODE_486_BUSY_HERE = 486,
        STATUSCODE_488_NOT_ACCEPTABLE_HERE = 488,
        STATUSCODE_600_BUSY_EVERYWHERE = 600,
        STATUSCODE_603_DECLINE = 603
    };

    /// Refer to ISession class
    enum
    {
        TERMINATION_REASON_INVALID = (-1),

        /// Unknown reason
        TERMINATION_REASON_UNKNOWN = 0,
        /// CANCEL or BYE is sent by the user
        TERMINATION_REASON_USER_ACTION,
        /// CANCEL or BYE is received from the remote user or CSCF
        TERMINATION_REASON_REMOTE_ACTION,
        /// Session refresh was failed by 408 response
        TERMINATION_REASON_REFRESH_408,
        /// Session refresh was failed by 481 response
        TERMINATION_REASON_REFRESH_481,
        /// Session refresh was failed by the transaction timeout
        TERMINATION_REASON_REFRESH_TXN_TIMEOUT,
        /// Session refresh was failed by the session refresh timeout
        TERMINATION_REASON_REFRESH_TIMEOUT,
        /// Session is terminated by the de-registration (by user or network)
        TERMINATION_REASON_SERVICE_CLOSED,

        TERMINATION_REASON_MAX
    };

    /// Policy for session refresh
    enum
    {
        /// No refresh by engine
        REFRESH_POLICY_NO_REFRESH = (-1),

        /// Default policy; Select the refresh time according to 3GPP spec.
        ///     nCriteriaInterval : Criteria value for the refresh duration
        ///    nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100; default 50)
        ///    nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_SPEC = 0,

        /// Set the remain time before it is expired
        ///    nCriteriaInterval : Criteria value for the refresh duration
        ///    nValueEorLT : Interval value when the refresh duration is equal or less
        ///              than the criteria interval
        ///    nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_REMAIN_TIME,

        /// Set the ratio before it is expired
        ///    nCriteriaInterval : Criteria value for the refresh duration
        ///    nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100)
        ///    nValueGT : Ratio when the refresh duration is greater
        ///              than the criteria interval (1 ~ 100)
        /// Ex) Expires: 3600, Ratio: 10
        ///        -> Refresh timer is expired after 3240s
        REFRESH_POLICY_RATIO
    };

    /// Option flags for sending SIP request/response
    enum
    {
        FLAG_NONE = 0x0000,
        FLAG_REMOVE_RECORD_ROUTES = 0x0001
    };

    /// Runtime configuration for session control
    enum
    {
        CONFIG_NONE = 0,
        /// RFC 6337, Section 3.1.1
        /// This will be applied for MO signaling.
        CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE = 1 << 0,
        /// This is to avoid the session update failure when the service is disconnected by re-REG.
        CONFIG_IGNORE_DEREG_ON_SESSION_UPDATE = 1 << 1,
        /// This is to override SIP configuration for SDP handling of non-RPR.
        CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR = 1 << 2,
        /// This is to indicate that early session model is supported.
        /// ex. VRBT (Video RingBack Tone) for KR operators
        CONFIG_SUPPORT_EARLY_SESSION_MODEL = 1 << 3,
        /// This is to specify whether the 100 Trying response needs to be notified to the enabler.
        CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED = 1 << 4,
        /// Specify whether the subsequent SDP answer should be ignored
        /// when SDP OA state is in preview mode.
        CONFIG_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE = 1 << 5
    };

    /// Index for the most recent response message
    enum
    {
        INDEX_MOST_RECENT_MESSAGE = 0xFFFFFFFF
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
        AMSG_SESSION_CANCELED_ON_ACCEPTED,
        AMSG_SESSION_CANCEL_DELIVERED,
        AMSG_SESSION_CANCEL_DELIVERY_FAILED,
        AMSG_SESSION_FORKED_RESPONSE_RECEIVED,
        AMSG_SESSION_PROVISIONAL_RESPONSE_RECEIVED,
        AMSG_SESSION_TRANSACTION_RECEIVED,
        AMSG_SESSION_DELAYED_DIALOG_TRANSACTION_RECEIVED,
        AMSG_SESSION_MAX
    };

    /// To guard the duplicated listener call
    enum
    {
        LISTENER_CALL_START_FAILED = 0x00000001,
        LISTENER_CALL_TERMINATED = 0x00000002,
        LISTENER_CALL_STARTED = 0x00000004
    };

    static constexpr const IMS_CHAR* WARNING_304 = "304 IMS-client \"Media Type Not Available\"";

private:
    IMS_SINT32 m_nState;
    // Call state
    CallState m_objCallState;
    IMS_BOOL m_bAlerting;
    IMS_BOOL m_bSdpInInitialInvite;
    IMS_BOOL m_bTerminatePending;
    IMS_BOOL m_bUpdateRequestor;
    IMS_BOOL m_bSdpNonRprAllowed;
    // TERMINATE_METHOD - default : CANCEL; it can be a BYE by enabler
    IMS_BOOL m_bTerminateMethodBye;
    // RACE_CONDITION : SESSION_UPDATE
    // Checks if the result of session-update request is notified or not
    IMS_BOOL m_bSessionUpdateNotificationInProgress;
    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    IMS_BOOL m_bImplicitRoutingRequired;
    // Flag specifying whether the incoming session received CANCEL request
    // while the session is in STATE_ESTABLISHING state.
    IMS_BOOL m_bSessionCanceledOnAccepted;
    // Runtime configuration for session control
    IMS_SINT32 m_nConfigValue;
    // Manages completed listener calls
    IMS_SINT32 m_nCompletedListenerCalls;
    // Reason of session termination (refer to ISession::TERMINATION_REASON_XXX)
    IMS_SINT32 m_nTerminationReason;
    AString m_strTerminationReasonFromApp;

    SdpOaState* m_pOaState;
    SessionDescriptor* m_pSessionDescriptor;
    ImsList<Media*> m_objMedias;
    // This is used for containing the SDP body part when a failure response is received
    // such as 488 or 606 with SDP body part.
    SdpReader* m_pRemoteMediaCapabilities;

    IOnSessionListener* m_piSessionListener;
    // For session refresh
    IRefreshListener* m_piRefreshListener;
    SessionRefreshHelper* m_pRefreshHelper;
    // For call transfer/hold/ ...
    IReferredMessageListener* m_piReferredMessageListener;
    RetryTaskHelper* m_pRetransmissionTask;
    // Remote session id for 3rd-party call control
    AString m_strSessionIdForCallControl;
    // For internal BYE transaction
    ISipClientConnection* m_piSccBye;
    // CALLER_PREFERENCE_MANAGER
    ImsList<AString> m_objPreviousCallerPreference;
    // Management of forked session to handle PRACK
    RcPtr<MethodManager> m_pForkedSessions;
    // EARLY_SESSION_MODEL
    RcPtr<VirtualSession> m_pVirtualEarlySession;
    // Setter for Reason header
    IReasonHeaderSetter* m_piReasonHeaderSetter;
};

#endif
