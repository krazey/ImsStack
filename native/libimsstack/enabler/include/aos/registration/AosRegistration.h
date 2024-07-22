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
#ifndef AOS_REGISTRATION_H_
#define AOS_REGISTRATION_H_

#include "ITimer.h"

#include "ImsActivityEx.h"
#include "IpAddress.h"
#include "SipProfile.h"

#include "IRegistrationListener.h"
#include "base/IMessageMediator.h"

#include "interface/AosInternalMsgDef.h"
#include "interface/IAosBlockListener.h"
#include "interface/IAosCallTrackerListener.h"
#include "interface/IAosNConfigurationListener.h"
#include "interface/IAosNetTrackerListener.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosSubscriptionListener.h"
#include "interface/IAosTransaction.h"

#include "provider/AosUtil.h"

class IRegContact;
class IRegistration;
class IRegistrationManager;
class IRegParameter;
class IRegSubscription;

class IAosAppContext;
class IAosHandle;
class IAosRegistrationListener;

class AosIpsecHelper;
class AosSubscription;

enum class AosNetworkType;
enum class AosReasonCode;

class AosRegistration :
        public ImsActivityEx,
        public IAosRegistration,
        public IRegistrationListener,
        public IAosSubscriptionListener,
        public IAosBlockListener,
        public IAosCallTrackerListener,
        public IAosNConfigurationListener,
        public IAosNetTrackerListener,
        public ITimerListener,
        public IMessageMediator,
        public IAosTransactionListener
{
public:
    AosRegistration(IN IAosAppContext* piAppContext, IN AString& strRegId);
    virtual ~AosRegistration();

    /// IAosRegistration
    void Start() override;
    void Stop() override;
    void Update(IN IMS_BOOL bIgnoreRetryTimer = IMS_FALSE,
            IN IMS_BOOL bExplicitUpdate = IMS_TRUE) override;
    void Reconfig() override;

    void Destroy() override;

    void SetListener(IN IAosRegistrationListener* piRegListener) override;

    void RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason = 0) override;

    IMS_UINT32 GetMode() override;
    IMS_UINT32 GetProperty(
            IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue) override;
    IMS_UINT32 GetState() override;
    AosRegistrationType GetRegType() override;
    IMS_SINT32 GetImsRegType() override;

    IMS_BOOL IsRegistered() override;
    IMS_BOOL IsRefreshing() override;
    IMS_BOOL IsRetryTimer() override;
    IMS_BOOL IsRetryHeld() override;
    IMS_BOOL IsTerminated() override;

    void SetAppReady(IN IMS_BOOL bReady) override;

protected:
    void SetState(IN IMS_UINT32 nState);
    void SetMode(IN IMS_UINT32 nMode);
    void SetFakeReg(IN IMS_BOOL bFake);
    void SetBlocked(IN IMS_BOOL bBlocked);
    void SetHeldByCall(IN IMS_BOOL bHeld);
    void SetImsCall(IN IMS_BOOL bStarted);
    void SetRadioWaiting(IN IMS_BOOL bWaiting);
    void SetTrafficPriorityBlocked(IN IMS_BOOL bBlocked);
    void SetRetryTime();
    IMS_BOOL SetTraffic(IN IMS_BOOL bStarted);
    void SetTrafficListener(IN IMS_BOOL bSet);
    void SetReregFailureReportOnIpcanChangeRequired(IN IMS_BOOL bRequired);
    void UpdateRegIpcanCategory();
    void ClearPending();
    void ClearCallingNumberVerification();

    IMS_BOOL IsFakeRegistration() const;
    IMS_BOOL IsIpsecSupported() const;
    IMS_BOOL IsAuthChallengedAgain() const;
    IMS_BOOL IsAuthChallengeMoreAllowed();
    IMS_BOOL IsTransactionStarted() const;
    IMS_BOOL IsBlocked() const;
    IMS_BOOL IsHeldByCall() const;
    IMS_BOOL IsImsCall() const;
    IMS_BOOL IsAppReady() const;
    IMS_BOOL IsExtraCapabilityRequiredForFeatureTag(IN IMS_UINT32 nOption) const;
    IMS_BOOL IsReconnectingServerSocketErrorAllowed() const;
    IMS_BOOL IsRegTypeEqual(IN AosRegistrationType eType) const;
    IMS_BOOL IsRegTrying() const;
    IMS_BOOL IsNetworkBindingSupported(IN IAosHandle* piHandle);
    IMS_BOOL IsCallStateRequired() const;
    IMS_BOOL IsRadioWaiting() const;
    IMS_BOOL IsTrafficPriorityBlocked() const;
    IMS_BOOL IsReregFailureReportOnIpcanChangeRequired() const;

    IMS_SINT32 GetRegExpires();

    void IncreaseConsecutiveFailCount();
    IMS_BOOL UpdatePreloadedRoute(
            IN const AString& strPcscf = AString::ConstNull(), IN const IMS_UINT32 nPcscfPort = 0);

    AosNetworkType GetNetworkTypeForImsRegState() const;
    AosReasonCode GetReasonCode() const;
    IMS_SINT32 GetRegIpcanCategory() const;
    IMS_UINT32 GetRegFeatures();

    void NotifyFailureWithImsReason(IN IMS_SINT32 nReason);
    void NotifyDeregistered();

    /// Set Detail State
    void UpdateDetailState(IN IMS_UINT32 nState);

    /// Log
    AString FeatureToString();

    /// ImsActivityEx
    IMS_BOOL OnMessage(IN IMSMSG& objMsg) override;

    /// Initialize
    void Init() override;
    virtual void InitFeatures();

    void CleanUp() override;
    virtual void DestroyEx();

    virtual IMS_BOOL IsGeolocationInfoRequired();
    virtual IMS_BOOL IsHandlingServerSocketErrorRequired(IN IMS_SINT32 nReason);

    /// Notify
    virtual void ReportStateChanged(IN IMS_UINT32 nResult, IN IMS_UINT32 nReason = 0);
    virtual void ReportTryingState();

    /// Registration
    virtual void PrepareRegistration();
    virtual IMS_BOOL CreateRegistration();
    virtual void DestroyRegistration();
    virtual IRegistration* GetRegistration();

    virtual IMS_BOOL StartRegBinding();
    virtual IMS_BOOL UpdateRegBinding();
    virtual IMS_BOOL UpdateNetworkRegBinding();
    virtual IMS_BOOL UpdateNetworkRegFeatureBinding();

    virtual IMS_BOOL Register(IN IMS_SINT32 nMinExpireValue);
    virtual IMS_BOOL SendRegister(
            IN IMS_BOOL bRestore = IMS_FALSE, IN IMS_BOOL bInitial = IMS_FALSE);
    virtual IMS_BOOL SendRegisterEx(
            IN IMS_SINT32 nMinExpireValue, IN IMS_BOOL bAddHalfExpireValue = IMS_FALSE);
    virtual IMS_BOOL SendDeregister();

    virtual IMS_BOOL AddOperation_OnSendRegister();
    virtual IMS_BOOL AddOperation_OnSendDeregister();
    virtual IMS_BOOL AddOperation_OnNotifyAkaResponse();

    virtual void CreateContact();
    virtual void AddSpecificOperation();
    virtual void AddAccesstypeFeatureTag();

    virtual void AddFeatureTag(IN IAosHandle* piHandle);
    virtual IMS_BOOL AddFeatureTagForMtc(IN IMS_UINT32 nRegFeatures, IN IMS_BOOL bFinalFeatureTag);
    virtual void RemoveFeatureTag(IN IAosHandle* piHandle);
    virtual IMS_BOOL RemoveFeatureTagForMtc(IN IMS_UINT32 nRegFeatures);
    virtual IMS_BOOL UpdateFeatureTag(IN IAosHandle* piHandle);
    virtual void UpdateFinalAddFeatureTag();

    virtual IMS_BOOL SetAor();
    virtual IMS_BOOL SetPcscf();
    virtual void SetRefreshPolicy();
    virtual void SetFailureState();
    virtual void SetRetryState();
    virtual void SetTcpCriterionLength();
    virtual void SetStaticIpQos();
    virtual void SetDynamicIpQos();
    virtual void SetActiveBindingsRestorationUsage();

    virtual void UpdateTransactionStarted();
    virtual void UpdateIpsecSupported(IN IMS_BOOL bSupported, IN IMS_UINT32 nReason = 0);

    /// Recovery
    virtual IMS_UINT32 GetActualWaitTime();
    virtual IMS_BOOL SetFirstPcscf(IN IMS_BOOL bUpdateParameter = IMS_TRUE);
    virtual IMS_BOOL SetNextPcscf(IN IMS_BOOL bUpdateParameter = IMS_TRUE);
    virtual IMS_BOOL TryNextPcscf();
    virtual IMS_BOOL TryNextPcscf(
            IN IMS_BOOL bFlowRecoveryOnAllFail, IN IMS_BOOL bHonorRetryAfter = IMS_FALSE);
    virtual IMS_BOOL IsRetryStopped();
    virtual IMS_BOOL IsRetryOnSamePcscfRequired() const;

    /// Clear
    virtual void ClearRegParameters(IN IMS_BOOL bClearPcscf = IMS_TRUE);
    virtual void ClearPcscf();
    virtual void ClearRetryCount(IN IMS_BOOL bForced = IMS_FALSE);
    virtual void ClearRetryValues(IN IMS_BOOL bRegSuccess = IMS_FALSE);
    virtual void ClearAuthChallengedCount();
    virtual void ClearAuthIpsecCount();
    virtual void ClearErrorCount();
    virtual void ClearNetworkBindingFeatures();
    virtual void ClearIpsecBlock();

    virtual void CheckPending();
    virtual IMS_BOOL CheckRadioReadyAndSetTxnPending();
    virtual IMS_BOOL ProcessPendingPlmnBlockOnUpdateFailure();
    virtual IMS_BOOL ProcessPlmnBlockWithPcoLimitedModeOnStartFailure();
    virtual IMS_BOOL ProcessPlmnBlockOnUpdateFailure();
    virtual void ProcessSetIpsec(IN IMS_UINT32 nReason);
    virtual void ProcessRefreshRegInfo();
    virtual void ProcessIpcanChanged();
    virtual void ProcessUpdateIpcan();
    virtual void ProcessScscfRestoration(IN IMS_UINT32 nUnavailableTimeForCurrentPcscf);
    virtual void ProcessPendingTransaction();
    virtual void ProcessRetryInRegStopped(IN IMS_BOOL bIgnoreTimer = IMS_FALSE);
    virtual void ProcessReregister();
    virtual void ProcessReinitiate(IN IMS_BOOL bClearPcscf = IMS_TRUE);
    virtual void ProcessUpdatePending();
    virtual void ProcessReconfigPending();
    virtual void ProcessUnpredictableFailure();
    virtual IMS_BOOL ProcessUnpredictableFailureHeldByCall();
    virtual void ProcessRegTerminated();
    virtual void ProcessRegTerminatedByNotify();
    virtual void ProcessAuthenticationFailed();
    virtual void ProcessRegRequiredWithWaitTime(IN IMS_SINT32 nWaitTime);
    virtual void ProcessRegRequiredWithNextPcscf();
    virtual void ProcessRegRequiredWithAvailableNextPcscf(
            IN IMS_BOOL bSetCurrentPcscfInvalid, IN IMS_UINT32 nReconnectTime = 0);
    virtual IMS_BOOL ProcessRegRequiredWithIpVersionChange();
    virtual void ProcessSubReinitiate();
    virtual IMS_BOOL ProcessForbiddenFailed(IN IMS_SINT32 nStatusCode);
    virtual IMS_BOOL ProcessSubscriberFailed(IN IMS_SINT32 nStatusCode);

    virtual IMS_BOOL ProcessAkaResponseFailed();
    virtual void ProcessAwtRecovery();
    virtual void ProcessIpsecFallback(IN IMS_BOOL bIsIpsecRetry);
    virtual void ProcessRequiredWfcErrMessage(IN IMS_SINT32 nStatusCode);

    virtual void ProcessDefaultFlowRecovery_Start(IN IMS_SINT32 nStatusCode = 0);
    virtual void ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(IN IMS_UINT32 nRetryAfter);
    virtual void ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(
            IN IMS_UINT32 nRetryAfter);
    virtual void ProcessDefaultFlowRecovery_Update(IN IMS_SINT32 nStatusCode = 0);
    virtual void ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            IN IMS_SINT32 nStatusCode, IN IMS_UINT32 nRetryAfter);

    virtual IMS_BOOL ProcessStartFailed_305();
    virtual void ProcessStartFailed_420();
    virtual void ProcessStartFailed_421();
    virtual void ProcessStartFailed_423();
    virtual void ProcessStartFailed_503();
    virtual void ProcessRequiredWfcErrMessage_403();
    virtual void ProcessRequiredWfcErrMessage_500();
    virtual void ProcessRequiredWfcErrMessage_Others();

    virtual IMS_BOOL ProcessUpdateFailed_305();
    virtual void ProcessUpdateFailed_423();

    virtual void ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode);
    virtual void ProcessStartFailed_TxnTimeout();
    virtual void ProcessStartFailed_Others(IN IMS_SINT32 nReason);

    virtual void ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode);
    virtual void ProcessUpdateFailed_TxnTimeout();
    virtual void ProcessUpdateFailed_Others(IN IMS_SINT32 nReason);

    virtual void ProcessStandardPcscfSelection(IN IMS_UINT32 nRetryAfter = 0);
    virtual IMS_BOOL ProcessIpVersionChange();
    virtual void ProcessRegEventChange(IN IMS_UINT32 nStatusCode);

    virtual void RecordImpu();

    /// IRegistrationListener
    void Registration_AuthenticationChallenged(
            IN IMS_SINT32 nAlgorithm, OUT IMS_BOOL& bResponseToChallenge) override;
    void Registration_NotifyAkaResponse(IN IMS_SINT32 nResult, IN const ByteArray& objIK,
            IN const ByteArray& objCK, OUT IMS_BOOL& bResultOfSA) override;
    void Registration_RefreshTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) override;
    void Registration_Started() override;
    void Registration_StartFailed(IN IMS_SINT32 nReason) override;
    void Registration_Updated() override;
    void Registration_UpdateFailed(IN IMS_SINT32 nReason) override;
    void Registration_Removed() override;
    void Registration_Terminated(IN IMS_SINT32 nReason) override;

    /// Timer
    virtual void ProcessOfflineRecoverTimerExpired();
    virtual void ProcessStopRetryTimerExpired();
    virtual void ProcessRefreshTimerExpired();
    virtual void ProcessExpiredTimerExpired();
    virtual void ProcessModeTimerExpired();
    virtual void ProcessTransactionTimerExpired();
    virtual void ProcessInternalErrorTimerExpired();

    virtual void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    virtual void StopTimer(IN IMS_UINT32 nType);
    virtual void ClearRetryTimers();
    virtual void ClearTimers();

    /// ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    // Subscription
    virtual IMS_BOOL CreateSubscription();
    virtual IMS_BOOL DestroySubscription();
    virtual IMS_BOOL StartSubscription(IN IMS_BOOL bIsRadioCheckRequired = IMS_TRUE);
    virtual IMS_BOOL StopSubscription();

    virtual AosSubscription* GetSubscription(IN IRegSubscription* piRegSubscription);

    virtual void ProcessSubscription_Success();
    virtual void ProcessSubscription_Failed();
    virtual void ProcessSubscription_Terminated(IN IMS_SINT32 nTerminateType = 0);

    virtual void ProcessRegEventRegistered();

    /// IAosSubscriptionListener
    void Subscription_StateChanged(IN IMS_SINT32 nState, IN IMS_SINT32 nReason = 0) override;
    IMS_BOOL Subscription_CanBeTransmitted() override;
    void Subscription_NotifyReceived(IN IMS_SINT32 nEvent) override;
    void Subscription_Request(IN IMS_SINT32 nCommand, IN IMS_SINT32 nRetryAfter = 0,
            IN IMS_BOOL bAwt = IMS_FALSE) override;

    /// Ipsec Helper
    virtual void CreateIpsecHelper();
    virtual void DestroyIpsecHelper();

    /// IAosBlockListener
    void Block_Changed(IN IMS_UINT32 nType = 0, IN IMS_UINT32 nParam = 0) override;

    /// IAosCallTrackerListener
    void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState) override;

    /// IAosNConfigurationListener
    void NConfiguration_NotifyConfigChanged() override;

    /// IAosNetTrackerListener
    void NetTracker_StatusChanged() override{};

    /// IAosTransactionListener
    void Transaction_OnConnectionFailed(IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode,
            IN IMS_UINT32 nWaitTimeMillis) override;
    void Transaction_OnConnectionSetupPrepared() override;
    void Transaction_OnTrafficPriorityChanged() override;

    /// IMessageMediator
    IMS_RESULT MessageMediator_AdjustMessage(
            IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nMessage = MESSAGE_NORMAL) override;

    virtual IMS_BOOL AddLocationHeaderBody(
            IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nMessage = MESSAGE_NORMAL);

private:
    void ControlPrivateHeader();
    IMS_UINT32 GetSpecificErrWaitTime();
    void ProcessImsiBasedSubscriber();
    void SetContactAddressConfiguration(IN IMS_BOOL bAdd);
    void SetPlaniHeader();
    void UpdateUserInfoInContact();
    void UpdateCallingNumberVerification();
    void NotifyTechnologyChangeFailed();

    IMS_BOOL IsErrorCodeExisted(
            IN const ImsVector<IMS_SINT32>& objErrorCode, IN IMS_SINT32 nCode) const;
    IMS_BOOL IsErrorCodeExistedForSpecificRegistration(IN IMS_SINT32 nCode) const;
    IMS_BOOL IsPdnReactivationRequired();
    IMS_BOOL IsRegExpiredDuringAwt(IN IMS_UINT32 nAwt);
    IMS_BOOL IsNeedToSetLimitedMode();
    IMS_BOOL IsUsimAuthFailureHandlingNeeded();

public:
    enum
    {
        MSG_REG_START = AOSMSG_SERVICE_INTERNAL,

        MSG_REG_REINITIATE,
        MSG_REG_UPDATE,
        MSG_REG_RECONFIG,

        MSG_REG_REQUIRED_WITH_WAIT_TIME,
        MSG_REG_REQUIRED_WITH_NEXT_PCSCF,
        MSG_REG_REQUIRED_WITH_SCSCF_RESTORATION,
        MSG_REG_REINITIATE_WITH_REG_STATE,
        MSG_REG_TERMINATED_BY_NOTIFY,

        MSG_SUB_REINITIATE,
        MSG_SUB_TERMINATED,

        MSG_REG_EVENT_REGISTERED
    };

    enum
    {
        TIMER_OFFLINE_RECOVER = 100,
        TIMER_STOP_RETRY,
        TIMER_REFRESH,
        TIMER_EXPIRED,
        TIMER_MODE,
        TIMER_TRANSACTION,
        TIMER_INTERNAL_ERROR
    };

    enum
    {
        FEATURE_NONE = 0x0,
        FEATURE_SUBSCRIPTION = 0x01,
        FEATURE_IPSEC = 0x02
    };

    enum
    {
        PENDING_NONE = 0x0,
        PENDING_START = 0x1,
        PENDING_TRANSACTION = 0x2,
        PENDING_UPDATE = 0x4,
        PENDING_RECONFIG = 0x8,
        PENDING_UPDATE_HELD_BY_CALL = 0x10,
        PENDING_PLMN_BLOCK_HELD_BY_CALL = 0x20,

        PENDING_SUBSCRIPTION = 0x40,
        PENDING_TERMINATED = 0x80,

        PENDING_TRAFFIC = 0x100
    };

    enum
    {
        IMS_REG_STATE_DEREGISTERED = 0,
        IMS_REG_STATE_REGISTERING,
        IMS_REG_STATE_REGISTERED
    };

    enum
    {
        IPSEC_BLOCK_NONE = 0x0,
        IPSEC_BLOCK_ERROR = 0x1,
        IPSEC_BLOCK_AUTENTICATION = 0x2,
        IPSEC_BLOCK_NOT_ESTABLISHED = 0x4,
        IPSEC_BLOCK_ROAMING = 0x8
    };

protected:
    IAosAppContext* m_piContext;
    IAosRegistrationListener* m_piListener;

    IMS_SINT32 m_nSlotId;

    /// engine member
    IRegistrationManager* m_piRegManager;
    IRegistration* m_piRegistration;
    IRegContact* m_piRegContact;
    IRegParameter* m_piRegParameter;

    /// subscription
    AosSubscription* m_pSubscription;

    /// Ipsec member
    AosIpsecHelper* m_pIpsecHelper;

    /// object
    AosUtil* m_pUtil;

    /// feature
    IMS_UINT32 m_nFeature;

    /// state
    IMS_UINT32 m_nState;

    /// update & transaction
    IMS_UINT32 m_nTxnPending;
    IMS_BOOL m_bIsTransactionStarted;
    IMS_BOOL m_bIsImsCall;
    IMS_BOOL m_bIsBlocked;
    IMS_BOOL m_bIsHeldByCall;
    IMS_BOOL m_bIsAppReady;
    IMS_BOOL m_bIsRadioWaiting;
    IMS_BOOL m_bIsTrafficPriorityBlocked;

    // reg update failure after handover
    IMS_BOOL m_bIsReregFailureReportOnIpcanChangeRequired;

    /// reg info
    AString m_strRegId;              /// aos_reg_0
    AosRegistrationType m_eRegType;  /// normal, emergency, fake
    IMS_UINT32 m_nFlowId;            /// 1, 2, 3, ...
    IMS_BOOL m_bFakeReg;             /// fake registration for emergency
    IMS_UINT32 m_nRegMode;           /// normal, limited, fake

    /// this is used for registration
    AString m_strPuid;        /// current
    IpAddress m_objIpa;       /// local IP
    AString m_strPcscf;       /// current Pcscf Address
    IMS_UINT32 m_nPcscfPort;  /// current Pcscf Port

    /// throttling
    IMS_UINT32 m_nRetryBaseTime;       /// base-time for flow recovery in RFC 5626
    IMS_UINT32 m_nRetryMaxTime;        /// max-time for flow recovery in RFC 5626
    IMS_UINT32 m_nConsecutiveFailure;
    IMS_UINT32 m_nConsecutiveFailureForPdnReactivated;
    IMS_UINT32 m_nForbiddenCount;

    /// timer
    /// this is used in the OFFLINE state without registration
    ITimer* m_piOfflineRecoverTimer;
    /// this is used in REGSTOP or REFRESHSTOP state with registration
    ITimer* m_piStopRetryTimer;
    /// this is used when running refresh timer within aos module(no engine operation)
    ITimer* m_piRefreshTimer;
    /// this is used when running expired timer within aos module(no engine operation)
    ITimer* m_piExpiredTimer;
    /// this is used in ECBM, etc for refresh condition
    ITimer* m_piModeTimer;
    /// this is used when running transaction timer within aos module(Timer F is not used)
    ITimer* m_piTransactionTimer;
    /// this is used when internal error happened(e.g socket error)
    ITimer* m_piInternalErrorTimer;

    /// authentication failure counter
    IMS_UINT32 m_nAuthChallengeCount;

    /// retry count for authentication failure for ipsec.
    IMS_UINT32 m_nAuthIpsecCount;

    /// socket error counter
    IMS_UINT32 m_nErrorCountForServerSocket;

    /// Calling number verification supported
    IMS_BOOL m_bCallingNumberVerificationSupported;

    /// network binding features
    IMS_UINT32 m_nNetworkBindingFeatures;

    /// attach type is EPS only or 5GS
    IMS_BOOL m_bEps5GsOnly;

    /// reason information to disable ipsec
    IMS_UINT32 m_nIpsecBlockReason;

    /// the state for notifying ims registration callback in telephony ims
    IMS_UINT32 m_nImsRegState;

    /// the features for notifying ims registration callback in telephony ims
    IMS_UINT32 m_nImsRegFeatures;

    /// the network that is notified with registration callback of telephony ims
    AosNetworkType m_eImsRegNetwork;

    /// the reason code that is notified with registration callback of telephony ims
    AosReasonCode m_eImsReasonCode;

    /// this is used to set SIP Profile on run-time
    RcPtr<SipProfile> m_pSipProfile;

    AString m_strTag;

    static const IMS_UINT32 INTERNAL_ERROR_INTERVAL = 3;   // 3 Sec.
    static const IMS_UINT32 RETRY_DEFAULT_WAIT_TIME = 30;  // 30 Sec
    static const IMS_UINT32 CONNECTION_FAILURE_RETRY_DEFAULT_WAIT_TIME = 16;  // 16 Sec
    static const IMS_UINT32 RECONNECT_SERVER_SOCKET_ERROR_MAX_COUNT = 10;
    static const IMS_UINT32 AUTHENTICATION_RETRY_MAX_COUNT = 6;
    static const IMS_UINT32 SIP_MTU_MAX_SIZE_VIA_WIFI = 1280;

private:
    /// IPCAN category being registered
    IMS_SINT32 m_nRegIpcanCategory;
    IMS_UINT32 m_nPdnReactivateWaitTime;
};

#endif  // AOS_REGISTRATION_H_
