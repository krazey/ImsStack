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
#ifndef AOS_SUBSCRIPTION_H_
#define AOS_SUBSCRIPTION_H_

#include "AString.h"
#include "ServicePhoneInfo.h"
#include "ITimer.h"
#include "IRegSubscriptionListener.h"

#include "interface/IAosTransaction.h"

class IAosSubscriptionListener;
class IRegSubscription;
class IRegInfoContact;
class IAosAppContext;

/**
 * @brief This class provides a IMS subscription for reg event package.
 */
class AosSubscription :
        public IRegSubscriptionListener,
        public ITimerListener,
        public IAosTransactionListener
{
public:
    AosSubscription(IN IAosAppContext* piContext, IN IRegSubscription* piRegSubscription,
            IN const AString& strAoR, IN const SipAddress& objContactAddress);

    virtual ~AosSubscription();

    virtual void Initialize();

    virtual IMS_BOOL Start(IN IMS_BOOL bIsRadioCheckRequired);
    virtual void Stop();
    virtual void Destroy();

    virtual void SetListener(IN IAosSubscriptionListener* piListener);
    virtual void SetRetryTimer(IN IMS_BOOL bCheckRetryAfter);

    virtual IMS_UINT32 GetState();

protected:
    void ClearThrottlingCount();

    IMS_BOOL IsSubTrying() const;
    IMS_BOOL IsTerminated() const;

    void ReportState(
            IN IMS_SINT32 nReason, IN IMS_SINT32 nCommand = 0, IN IMS_BOOL bAwt = IMS_FALSE);
    void ReportNotifyEvent(IN IMS_SINT32 nEvent, IN IMS_SINT32 nRetryAfter = 0);

    void SetState(IN IMS_UINT32 nState);
    void SetTerminated(IN IMS_BOOL bTerminated);

    void StartTimer(IN IMS_UINT32 nDuration);
    void StopTimer();

    IMS_BOOL CheckRadioReadyAndSetRadioWaiting();
    IMS_BOOL IsRadioWaiting() const;
    void SetRadioWaiting(IN IMS_BOOL bWaiting);

    // Print Log
    void PrintRegInfo(IN ImsList<IRegInfoContact*>& objRegInfo);

    virtual IMS_BOOL SendSubscribe();
    virtual IMS_BOOL ProcessFailureResponse_423(IN IMS_BOOL bIsRefreshed);
    virtual IMS_BOOL ProcessFailureResponse_504(IN IMS_BOOL bIsRefreshed);

public:
    virtual IMS_BOOL IsRetryActionDueToRetrycounter(IN IMS_BOOL bIsRefreshed);
    virtual IMS_BOOL IsSubscriptionTerminated(IN IMS_SINT32 nStatusCode);
    virtual IMS_BOOL IsInitialRegistrationRequired(
            IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed);
    virtual IMS_BOOL IsInitialRegistrationWithNextPcscfRequired(
            IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed);
    virtual IMS_BOOL IsInitialRegistrationRequiredInWifi(
            IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed);
    virtual IMS_BOOL IsResubscriptionStopped(IN IMS_SINT32 nStatusCode);
    virtual IMS_BOOL ProcessFailed_StatusCode(IN IMS_SINT32 nStatusCode, IN IMS_BOOL bIsRefreshed);

    virtual IMS_BOOL IsRegRequiredByNotify(IN IMS_UINT32 nFeature);
    virtual IMS_BOOL IsRegAfterWaitRequiredByNotify(IN IMS_UINT32 nFeature);
    virtual IMS_BOOL IsWfcErrorMessageSupportedWithStateChecked(IN IMS_SINT32 nError);

protected:
    virtual void SetRequestCommand(IN IMS_BOOL bIsRefreshed, IN IMS_SINT32 nCommand);
    virtual void RequestCommand(IN IMS_SINT32 nReason, IN IMS_SINT32 nCommand);

    virtual void ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode);
    virtual void ProcessStartFailed_Others(IN IMS_SINT32 nReason);
    virtual void ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode);
    virtual void ProcessUpdateFailed_Others(IN IMS_SINT32 nReason);

    virtual IMS_SINT32 GetRetryAfter();
    virtual IMS_SINT32 GetNextThrottlingTime(IN const ImsVector<IMS_SINT32>& objInterval);
    virtual void ProcessTimerExpired();
    virtual void SetRefreshPolicy();

    virtual IRegInfoContact* GetRegInfoContact(IN const ImsList<IRegInfoContact*>& objContact);
    virtual IMS_BOOL CompareUriAssociatedWithContact(IN const SipAddress& objUri);

    virtual IMS_SINT32 ConvertRegInfoEvent(IN IMS_SINT32 nEvent);
    virtual void ProcessNotifyState_Terminated(IN IMS_SINT32 nEvent);
    virtual void ProcessNotifyState_Active(IN IMS_SINT32 nState);
    virtual void ProcessNotifyState_InvalidBody();

    // IRegSubscriptionListener
    void RegSubscription_NotifyReceived(
            IN IMS_SINT32 nSubState, IN IMS_SINT32 nReasonParam, IN IMS_BOOL bHasBody) override;
    void RegSubscription_RefreshTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) override;
    void RegSubscription_Started() override;
    void RegSubscription_StartFailed(IN IMS_SINT32 nReason) override;
    void RegSubscription_Updated() override;
    void RegSubscription_UpdateFailed(IN IMS_SINT32 nReason) override;
    void RegSubscription_Removed() override;
    void RegSubscription_Terminated(IN IMS_SINT32 nReason) override;

    /// IAosTransactionListener
    void Transaction_OnConnectionFailed(IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) override;
    void Transaction_OnConnectionSetupPrepared() override;
    void Transaction_OnTrafficPriorityChanged() override;

public:
    // ITimerListener Interface
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    static const IMS_CHAR* StateToString(IN IMS_UINT32 nState);
    static const IMS_CHAR* RegSubReasonToString(IN IMS_SINT32 nReason);
    static const IMS_CHAR* RegInfoStateToString(IN IMS_SINT32 nState);
    static const IMS_CHAR* RegInfoEventToString(IN IMS_SINT32 nEvent);

public:
    enum
    {
        STATE_OFFLINE = 0,
        STATE_SUBSCRIBING,
        STATE_SUBSTOP,
        STATE_SUBSCRIBED,
        STATE_SUBREFRESHING,
        STATE_SUBREFRESHSTOP,
        STATE_UNSUBSCRIBING
    };

    enum
    {
        REASON_NONE = 0,
        REASON_SUB_ESTABLISHED,
        REASON_SUB_FAILED,
        REASON_SUB_REMOVED,
        REASON_SUB_TERMINATED
    };

    enum
    {
        EVENT_REGISTERED = 0,
        EVENT_EXPIRED,
        EVENT_DEACTIVATED,
        EVENT_PROBATION,
        EVENT_UNREGISTERED,
        EVENT_REJECTED,
        EVENT_UNKNOWN
    };

    enum
    {
        CMD_NONE = 0,
        CMD_REG_REQUIRED,
        CMD_REG_REQUIRED_WITH_NEXT_PCSCF,
        CMD_REG_REQUIRED_WITH_SUB_403_MSG,
        CMD_REG_REQUIRED_WITH_NOTIFY_TERMINATED_MSG,
        CMD_REG_TERMINATED,
        CMD_SUB_REQUIRED,
        CMD_SUB_TERMINATED
    };

protected:
    IRegSubscription* m_piRegSubscription;

    IAosAppContext* m_piContext;
    ITimer* m_piRetryTimer;

    IMS_UINT32 m_nThrottlingCount;

    // for matching reg info contact
    SipAddress m_objContactAddress;

    AString m_strTag;

    // public user identity for getting reg info
    AString m_strAor;
    // state of AoR of NOTIFY
    IMS_SINT32 m_nAorState;

private:
    IAosSubscriptionListener* m_piListener;
    IMS_SINT32 m_nState;
    IMS_BOOL m_bIsTerminated;
    IMS_BOOL m_bIsErrChecked;
    IMS_BOOL m_bIsRadioWaiting;

    IMS_UINT32 m_nRetryCountSubTerminated;
    IMS_UINT32 m_nRetryCountRegRequired;

    static const IMS_UINT32 RETRY_DEFAULT_WAIT_TIME = 30;
    static const IMS_UINT32 REFRESH_POLICY_CRITERIA_INTERVAL_FOR_RETRY = 1200;
    static const IMS_UINT32 REFRESH_POLICY_RATIO_VALUE_BELOW_THE_CRITERIA = 50;
    static const IMS_UINT32 REFRESH_POLICY_INTERVAL_VALUE_ABOVE_THE_CRITERIA = 600;

private:
    friend class AosSubscriptionTest;
};

#endif  // AOS_SUBSCRIPTION_H_
