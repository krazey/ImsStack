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
#ifndef SUB_STATE_H_
#define SUB_STATE_H_

#include "ISipMessage.h"
#include "util/EventPackage.h"

struct SipHeaderProperty;

/**
 * @brief This class defines a state & behavior for a subscription state.
 */
class SubState
{
public:
    SubState();
    virtual ~SubState();

    SubState(IN const SubState&) = delete;
    SubState& operator=(IN const SubState&) = delete;

public:
    virtual void Clear();
    virtual IMS_BOOL UpdateState(IN const ISipMessage* piSipMsg) = 0;

    IMS_BOOL CreateEventPackage(IN const AString& strEvent);
    inline IMS_SINT32 GetConfiguration() const { return m_nConfigValue; }
    inline IMS_SINT32 GetDuration() const { return m_nSubscriptionDuration; }
    inline EventPackage* GetEventPackage() { return &m_objEventPackage; }
    inline ISipMessage* GetInitialMessage() const { return m_piSipMsg; }
    inline IMS_SINT32 GetOperation() const { return m_nOperation; }
    inline IMS_SINT32 GetState() const { return m_nState; }
    inline IMS_SINT32 GetSubState() const { return m_nSubStateValue; }
    inline IMS_BOOL IsInstantSubscription() const { return m_bInstantSubscription; }
    inline IMS_BOOL IsSubscriptionDurationUpdated() const { return m_bSubscriptionDurationUpdated; }
    inline IMS_BOOL IsTerminated() const { return (GetState() == STATE_TERMINATED); }
    inline void SetConfiguration(IN IMS_SINT32 nConfigValue) { m_nConfigValue = nConfigValue; }
    void SetOperation(IN IMS_SINT32 nOperation);

    static IMS_SINT32 ExtractExpiresParameter(IN const ISipHeader* piHeader);
    static IMS_SINT32 ExtractReasonParameter(IN const ISipHeader* piHeader);
    static IMS_SINT32 ExtractSubStateValue(IN const ISipHeader* piHeader);

    // Gets the constant values from ISubscriptionState
    static IMS_SINT32 GetSubStateFromSubscriptionState(IN IMS_SINT32 nSubState);
    static IMS_SINT32 GetReasonFromSubscriptionState(IN IMS_SINT32 nReason);

protected:
    virtual const SipHeaderProperty* GetRestrictedHeaders(OUT IMS_UINT32& nCount) const;

    inline IMS_BOOL IsConfigurationSet(IN IMS_SINT32 nValue) const
    {
        return (m_nConfigValue & nValue) != 0;
    }
    inline void SetDuration(IN IMS_SINT32 nDuration) { m_nSubscriptionDuration = nDuration; }
    inline void SetDurationUpdated(IN IMS_BOOL bDurationUpdated)
    {
        m_bSubscriptionDurationUpdated = bDurationUpdated;
    }
    inline void SetInstantSubscription(IN IMS_BOOL bInstantSubscription)
    {
        m_bInstantSubscription = bInstantSubscription;
    }
    void SetState(IN const ISipMessage* piSipMsg, IN IMS_SINT32 nState);
    inline void SetSubState(IN IMS_SINT32 nSubState) { m_nSubStateValue = nSubState; }
    void StoreMessage(IN const ISipMessage* piSipMsg);

private:
    static const IMS_CHAR* OperationToString(IN IMS_SINT32 nOperation);
    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    /// ONLY OUTGOING SUBSCRIPTION WILL BE CONCERNED ...
    /// Internal states for tracking the subscription state
    enum
    {
        /// INVALID
        STATE_INVALID = (-1),
        /// INIT
        STATE_INIT = 0,
        /// SUBSCRIBE (new) sent/received
        STATE_SUBSCRIBING,
        /// PENDING
        STATE_PENDING,
        /// ACTIVE
        STATE_ACTIVE,
        /// TERMINATED
        STATE_TERMINATED,

        STATE_MAX
    };

    /// Type of subscription operation
    enum
    {
        NO_OPERATION = 0,

        OPERATION_CREATE,
        OPERATION_REFRESH,
        OPERATION_FETCH,
        OPERATION_REMOVE,
        /// Refreshed by the SubscriberRefreshHelper
        OPERATION_IMPLICIT_REFRESH
    };

    /// substate in Subscription-State header
    enum
    {
        SUB_STATE_INIT = 0,

        SUB_STATE_ACTIVE,
        SUB_STATE_PENDING,
        SUB_STATE_TERMINATED
    };

    /// "reason" parameter in Subscription-State header
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

    /// Runtime configuration for sub-state
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
    EventPackage m_objEventPackage;
    // Main state of the subscription
    IMS_SINT32 m_nState;
    // Current operation for the subscription: ADD/FETCH/REFRESH/REMOVE/IMPLICIT_REFRESH
    IMS_SINT32 m_nOperation;
    // Runtime configuration for sub-state
    IMS_SINT32 m_nConfigValue;
    // Authoritative subscription duration
    //    - "Expires" header in 2XX to SUBSCRIBE request
    //    - "expires" parameter in NOTIFY request
    IMS_SINT32 m_nSubscriptionDuration;
    // Subscription state in NOTIFY request: active/pending/terminated
    IMS_SINT32 m_nSubStateValue;
    // Flag for updating the refresh timer
    IMS_BOOL m_bSubscriptionDurationUpdated;
    // Flag for an instant subscription
    IMS_BOOL m_bInstantSubscription;
    // Manages an initial SIP message for refresh/removal operation
    ISipMessage* m_piSipMsg;
};

#endif
