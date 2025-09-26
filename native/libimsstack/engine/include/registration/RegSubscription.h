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
#ifndef REG_SUBSCRIPTION_H_
#define REG_SUBSCRIPTION_H_

#include "IRegInfoListener.h"
#include "IRegSubscription.h"
#include "RegKey.h"
#include "RegStateTracker.h"
#include "base/Method.h"
#include "util/IDialogMethod.h"
#include "util/IRefreshable.h"

class IRegSubscriptionListener;
class ISipConnectionNotifier;
class SipTimerValues;
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
    RegSubscription(IN const RegKey& objRegKey, IN RegStateTracker* pRegStateTracker,
            IN IMS_UINT32 nExpiresValue = 0, IN const SipTimerValues* pTimerValues = IMS_NULL);
    ~RegSubscription() override;

    RegSubscription(IN const RegSubscription&) = delete;
    RegSubscription& operator=(IN const RegSubscription&) = delete;

public:
    // IRegBase interface
    ISipMessage* GetNextRequest() override;
    inline ISipMessage* GetPreviousRequest() const override { return m_piPreviousRequest; }
    inline ISipMessage* GetPreviousResponse() const override { return m_piPreviousResponse; }
    void SetSipMessageMediator(IN IMessageMediator* piMediator) override;

    // IRegSubscription interface
    void DestroyEx() override;
    IMS_SINT32 DisableFeatures(IN IMS_SINT32 nFeatures) override;
    IMS_SINT32 EnableFeatures(IN IMS_SINT32 nFeatures) override;
    inline IMS_UINT32 GetExpires() const override { return m_nExpiresValue; }
    const IRegInfo* GetRegInfo() const override;
    inline IMS_SINT32 GetState() const override { return m_nState; }
    IMS_RESULT SetContactParameter(IN const AString& strParameter,
            IN IMS_SINT32 nOperation = CONTACT_PARAM_OP_ADD) override;
    void SetExpires(IN IMS_UINT32 nExpires) override;
    inline void SetListener(IN IRegSubscriptionListener* piListener) override
    {
        m_piListener = piListener;
    }
    void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt) override;
    IMS_RESULT Subscribe() override;
    IMS_RESULT Unsubscribe() override;

private:
    // Activity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // Method class
    IMS_BOOL InitInstance() override;
    IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piScc) override;

    // Method class - Handle to the outgoing request / incoming response message
    void NotifySipResponse(IN ISipClientConnection* piScc) override;
    void NotifySipError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    // IDialogMethod interface
    IMS_BOOL Dialog_Compare(IN ISipServerConnection* piSsc) const override;
    IMS_BOOL Dialog_NotifyRequest(IN ISipServerConnection* piSsc) override;

    // IRefreshable interface
    void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piScc, IN IMS_SINT32 nCode = 0) override;
    IMS_BOOL Refreshable_RefreshStarted() override;
    void Refreshable_RefreshTerminated() override;

    // IRegInfoListener interface
    inline void RegInfo_Updated(IN IMS_BOOL bStale = IMS_FALSE) override { (void)bStale; }
    inline void RegInfo_UpdateFailed() override {}

    void CheckDialogNCallListener();
    void ClearNextRequest();
    IMS_BOOL CopyHeadersAndBodyParts(IN_OUT ISipMessage*& piSipMsg);
    inline IMS_BOOL IsFeatureEnabled(IN IMS_SINT32 nFeature) const
    {
        return (m_nFeatureSet & nFeature) != 0;
    }
    void SetOngoingConnection(IN ISipClientConnection* piScc);
    void SetPreviousRequest(IN const ISipMessage* piSipMsg);
    void SetPreviousResponse(IN const ISipMessage* piSipMsg);
    IMS_BOOL SendResponse(IN ISipServerConnection* piSsc, IN IMS_SINT32 nStatusCode);
    IMS_BOOL SetContactHeader(IN_OUT ISipMessage*& piSipMsg, OUT IMS_BOOL& bIsContactGruu);
    IMS_BOOL SetHeaders(IN_OUT ISipMessage*& piSipMsg);
    void SetState(IN IMS_SINT32 nState);
    IMS_BOOL SubscribeOnImplicitRefresh();
    // IMS_REQUEST_URI_VALIDATION_IN_MID_DIALOG
    IMS_BOOL ValidateRequestUri(
            IN const SipAddress& objRequestUri, IN const ISipDialog* piDialog) const;

    static ISipClientConnection* CreateConnection(IN RegSubscription* pRegSub);
    static IMS_UINT16 GetReasonParameter(IN const ISipMessage* piSipMsg);
    static IMS_UINT16 GetReasonFromSubStateReason(IN IMS_UINT16 nReason);
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

    // Runtime features
    IMS_SINT32 m_nFeatureSet;
    // State of Subscription
    IMS_SINT32 m_nState;
    IMS_UINT32 m_nExpiresValue;
    // Storage for pending operation for subscription
    IMS_SINT32 m_nPendingOperation;
    // Listener for this RegSubscription
    IRegSubscriptionListener* m_piListener;
    // Registration key
    RegKey m_objRegKey;
    // Registration State Tracker
    RcPtr<RegStateTracker> m_pRegStateTracker;
    // Subscription information for subscriber behavior
    SubState* m_pSubState;
    // Subscription refresh timer
    SubscriberRefreshHelper* m_pRefreshHelper;
    // Current SIP connection for abnormal cases
    ISipClientConnection* m_piOngoingScc;
    // Message for the next SUBSCIRBE request
    ISipMessage* m_piNextRequest;
    // Message for the previous SUBSCRIBE request
    ISipMessage* m_piPreviousRequest;
    // Message for the previous SUBSCRIBE response
    ISipMessage* m_piPreviousResponse;
    // Timer values of SIP transaction layer
    SipTimerValues* m_pSipTimerValues;
    // NOTIFY_REQUEST_HANDLING_AFTER_DE_REG
    // To handle a notification properly after de-REG
    ISipConnectionNotifier* m_piReferredScn;
};

#endif
