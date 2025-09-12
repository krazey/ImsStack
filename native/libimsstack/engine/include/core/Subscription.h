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
#ifndef SUBSCRIPTION_H_
#define SUBSCRIPTION_H_

#include "ServiceMethod.h"
#include "util/IDialogMethod.h"
#include "util/IForkedDialogMethod.h"
#include "util/IRefreshable.h"

class IOnSubscriptionListener;
class IRefreshListener;
class SubState;
class SubscriberRefreshHelper;

class Subscription :
        public ServiceMethod,
        public IDialogMethod,
        public IForkedDialogMethod,
        public IRefreshable
{
public:
    Subscription(IN Service* pService, IN const AString& strEvent,
            IN IMS_BOOL bImplicitRoutingRequired = IMS_TRUE);
    ~Subscription() override;

    Subscription(IN const Subscription&) = delete;
    Subscription& operator=(IN const Subscription&) = delete;

public:
    // Method class
    void Destroy() override;
    void SetMessageMediator(IN IMessageMediator* piMediator) override;

    // ISubscription interface
    inline const AString& GetEvent() const { return m_strEvent; }
    inline IMS_SINT32 GetState() const { return m_nState; }
    IMS_RESULT Poll();
    inline void SetListener(IN IOnSubscriptionListener* piListener) { m_piListener = piListener; }
    IMS_RESULT Subscribe();
    IMS_RESULT Unsubscribe();

    void SetImplicitRoutingRequired(IN IMS_BOOL bFlag);
    inline void SetRefreshListener(IN IRefreshListener* piListener)
    {
        m_piRefreshListener = piListener;
    }
    void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt);

protected:
    // Activity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // Method class
    IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piScc) override;
    // Handle the exceptions
    void Exception_NotifyError(IN IMS_SINT32 nErrorCode) override;
    IMS_BOOL InitInstance() override;
    // Handle to the outgoing request / incoming response message
    void NotifySipResponse(IN ISipClientConnection* piScc) override;
    void NotifySipError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    // IDialogMethod interface
    IMS_BOOL Dialog_Compare(IN ISipServerConnection* piSsc) const override;
    IMS_BOOL Dialog_NotifyRequest(IN ISipServerConnection* piSsc) override;

    // IForkedDialogMethod interface
    IMS_BOOL ForkedDialog_Compare(IN ISipDialog* piOrigDialog) const override;
    IMS_BOOL ForkedDialog_NotifyRequest(IN ISipServerConnection* piSsc) override;

    // IRefreshable interface
    void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piScc, IN IMS_SINT32 nCode = 0) override;
    IMS_BOOL Refreshable_RefreshStarted() override;
    void Refreshable_RefreshTerminated() override;

private:
    void CheckDialogNCallListener();
    void CleanupOnDestroy();
    void CloseConnection();
    ISipClientConnection* CreateConnectionL(IN ISipDialog* piDialog, IN const SipMethod& objMethod);
    void SetState(IN IMS_SINT32 nState);
    void UpdateResponse(IN ISipClientConnection* piScc);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    /// Refer to ISubscription class
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING = 2,
        STATE_ACTIVE = 3
    };

    /// Policy for subscription refresh
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
    IMS_SINT32 m_nState;
    // Event package name for this subscription
    AString m_strEvent;
    // Storage for pending operation for subscription
    IMS_SINT32 m_nPendingOperation;
    // Listener for this subscription
    IOnSubscriptionListener* m_piListener;
    // Subscription information for subscriber behavior
    SubState* m_pSubState;
    // Subscription refresh timer
    IRefreshListener* m_piRefreshListener;
    SubscriberRefreshHelper* m_pRefreshHelper;
    // Queue for NOTIFY request messages
    ImsList<Message*> m_objNotifyMessages;
    // For forked NOTIFY request
    ImsList<Subscription*> m_objForkedSubscriptions;
    // Flag to indicate that the subscription is created inside of any dialog (INVITE)
    IMS_BOOL m_bSubscriptionInOtherDialog;
    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    IMS_BOOL m_bImplicitRoutingRequired;
};

#endif
