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
#ifndef REFERENCE_H_
#define REFERENCE_H_

#include "ISubscriptionState.h"
#include "Replaces.h"
#include "ServiceMethod.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "util/IDialogMethod.h"
#include "util/IReferredMessageListener.h"

class IOnNotificationListener;
class IOnReferenceListener;
class SubState;

// Reference: RFC 3515, RFC 3420, RFC 3725, RFC 3891, RFC 4488, RFC 4508, RFC 5368, RFC 3265
class Reference : public ServiceMethod, public IDialogMethod, public IReferredMessageListener
{
public:
    class NotifierState
    {
    public:
        NotifierState();
        ~NotifierState();

        NotifierState(IN const NotifierState&) = delete;
        NotifierState& operator=(IN const NotifierState&) = delete;

    public:
        inline void AddScc(IN ISipClientConnection* piScc) { m_objSccs.Append(piScc); }
        void RemoveScc(IN const ISipClientConnection* piScc);

    private:
        ImsList<ISipClientConnection*> m_objSccs;
    };

public:
    Reference(IN Service* pService, IN const AString& strReferToUri,
            IN const AString& strReferMethod, IN const Replaces& objReplaces,
            IN IMS_BOOL bImplicitRoutingRequired = IMS_TRUE);
    ~Reference() override;

    Reference(IN const Reference&) = delete;
    Reference& operator=(IN const Reference&) = delete;

public:
    // Method class
    void Destroy() override;

    // IReference interface
    IMS_RESULT Accept();
    IMS_RESULT ConnectReferMethod(IN Method* pReferMethod);
    inline const SipMethod& GetReferMethod() const { return m_objReferMethod; }
    inline const AString& GetReferToUserId() const { return m_strReferToUri; }
    const AString& GetReplaces() const;
    inline IMS_SINT32 GetState() const { return m_nState; }
    IMS_RESULT Refer(IN IMS_BOOL bImplicitSubscription);
    IMS_RESULT Reject();
    inline void SetListener(IN IOnReferenceListener* piListener) { m_piListener = piListener; }
    IMS_RESULT SetReplaces(IN const AString& strSessionId);
    IMS_RESULT AcceptEx(
            IN IMS_SINT32 nStatusCode = SipStatusCode::SC_202, IN IMS_BOOL b100Trying = IMS_TRUE);
    IMS_RESULT ReferEx(IN IMS_BOOL bImplicitSubscription,
            IN const AString& strHeadersForReferTo = AString::ConstNull());
    IMS_RESULT RejectEx(IN IMS_SINT32 nStatusCode);
    IMS_RESULT SendNotification(IN IMS_SINT32 nSubState, IN const ByteArray& objContent,
            IN IMS_SINT32 nReason = ISubscriptionState::REASON_NONE, IN IMS_SINT32 nExpires = (-1));
    inline void SetNotificationListener(IN IOnNotificationListener* piListener)
    {
        m_piNotificationListener = piListener;
    }
    void SetImplicitRoutingRequired(IN IMS_BOOL bFlag);

protected:
    // Activity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // Method class
    IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piScc) override;
    // Handle the exceptions
    void Exception_NotifyError(IN IMS_SINT32 nErrorCode) override;
    IMS_BOOL InitInstance() override;
    // Handle the incoming request / outgoing response message
    IMS_BOOL NotifySipRequest(IN ISipServerConnection* piSsc) override;
    // Handle to the outgoing request / incoming response message
    void NotifySipResponse(IN ISipClientConnection* piScc) override;
    void NotifySipError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    // IDialogMethod interface
    IMS_BOOL Dialog_Compare(IN ISipServerConnection* piSsc) const override;
    IMS_BOOL Dialog_NotifyRequest(IN ISipServerConnection* piSsc) override;

    // IReferredMessageListener interface
    void ReferredMessage_NotifyOnActive(IN ISipMessage* piSipMsg) override;
    void ReferredMessage_NotifyOnTerminated(IN IMS_SINT32 nReasonCode = SubState::REASON_NONE,
            IN ISipMessage* piSipMsg = IMS_NULL) override;

private:
    void CleanupOnDestroy();
    ISipClientConnection* CreateConnectionL(IN ISipDialog* piDialog, IN const SipMethod& objMethod);
    IMS_RESULT DoNotification(IN IMS_SINT32 nSubState, IN const ByteArray& objContent,
            IN IMS_SINT32 nReasonCode = SubState::REASON_NONE, IN IMS_SINT32 nExpires = (-1));
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    /// Refer to IReference class
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
    IMS_SINT32 m_nState;
    // Target URI in Refer-To header
    AString m_strReferToUri;
    // method uri parameter in Refer-To header
    SipMethod m_objReferMethod;
    // replaces uri-header parameter in Refer-To header
    // It contains callid, from-tag, to-tag
    Replaces* m_pReplaces;
    // User's request to the implicit subscription
    IMS_BOOL m_bImplicitSubscription;
    // Referred method
    Method* m_pReferredMethod;
    // Listener for this reference
    IOnReferenceListener* m_piListener;
    // Subscription state
    SubState* m_pSubState;
    // Flag to indicate that the reference is created inside of any dialog (INVITE)
    IMS_BOOL m_bReferenceInOtherDialog;
    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    IMS_BOOL m_bImplicitRoutingRequired;
    // Queue for NOTIFY request messages
    ImsList<Message*> m_objNotifyMessages;
    // Notification listener for notifier's behavior
    IOnNotificationListener* m_piNotificationListener;
    // Notifier's state
    NotifierState* m_pNotifierState;
};

#endif
