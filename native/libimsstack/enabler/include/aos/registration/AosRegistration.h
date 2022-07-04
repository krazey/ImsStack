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

#include "IRegistrationListener.h"
#include "base/IMessageMediator.h"

#include "interface/AosInternalMsgDef.h"
#include "interface/IAosBlockListener.h"
#include "interface/IAosCallTrackerListener.h"
#include "interface/IAosNConfigurationListener.h"
#include "interface/IAosNetTrackerListener.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosSubscriptionListener.h"
#include "interface/IAosTrm.h"
#include "interface/IAosVonr.h"

#include "provider/AosKeepAlive.h"
#include "provider/AosUtil.h"

class IRegContact;
class IRegistration;
class IRegParameter;
class IRegSubscription;

class RegistrationManager;
class SipProfile;

class IAosAppContext;
class IAosHandle;
class IAosRegistrationListener;

class AosIpsecHelper;
class AosStaticProfile;
class AosSubscription;

enum class AosNetworkType;

class AosRegistration :
        public ImsActivityEx,
        public IAosRegistration,
        public IRegistrationListener,
        public IAosSubscriptionListener,
        public IAosBlockListener,
        public IAosCallTrackerListener,
        public IAosNConfigurationListener,
        public IAosNetTrackerListener,
        public IAosTrmListener,
        public ITimerListener,
        public IAosKeepAliveListener,
        public IMessageMediator
{
public:
    AosRegistration(IN IAosAppContext* piAppContext, IN AString& strRegId);
    virtual ~AosRegistration();

    /// IAosRegistration
    virtual void Start();
    virtual void Stop();
    virtual void Update(
            IN IMS_BOOL bIgnoreRetryTimer = IMS_FALSE, IN IMS_BOOL bExplicitUpdate = IMS_TRUE);
    virtual void Reconfig();

    virtual void Destroy();

    virtual void SetListener(IN IAosRegistrationListener* piRegListener);

    virtual void RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason = 0);

    virtual IMS_UINT32 GetMode();
    virtual IMS_UINT32 GetProperty(
            IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue);
    virtual IMS_UINT32 GetState();
    virtual AosRegistrationType GetRegType();

    virtual IMS_BOOL IsRegistered();
    virtual IMS_BOOL IsRefreshing();
    virtual IMS_BOOL IsRetryTimer();
    virtual IMS_BOOL IsRetryHeld();
    virtual IMS_BOOL IsTerminated();

    virtual void SetAppReady(IN IMS_BOOL bReady);

protected:
    void SetState(IN IMS_UINT32 nState);
    void SetMode(IN IMS_UINT32 nMode);
    void SetFakeReg(IN IMS_BOOL bFake);
    void SetIsIpsecSupported(IN IMS_BOOL bSupported);
    void SetIsIpsecInit(IN IMS_BOOL bInit);
    void SetBlocked(IN IMS_BOOL bBlocked);
    void SetTrmBlocked(IN IMS_BOOL bTrmBlocked);
    void SetHeldByCall(IN IMS_BOOL bHeld);
    void SetImsCall(IN IMS_BOOL bStarted);
    void SetRetryTime();

    void ClearPending();
    void ClearCallingNumberVerification();

    IMS_BOOL IsFakeRegistration() const;
    IMS_BOOL IsIpsecSupported() const;
    IMS_BOOL IsIpsecInit() const;
    IMS_BOOL IsAuthChallengedAgain() const;
    IMS_BOOL IsAuthChallengeMoreAllowed();
    IMS_BOOL IsTransactionStarted() const;
    IMS_BOOL IsBlocked() const;
    IMS_BOOL IsTrmBlocked() const;
    IMS_BOOL IsHeldByCall() const;
    IMS_BOOL IsImsCall() const;
    IMS_BOOL IsAppReady() const;
    IMS_BOOL IsExtraCapabilityRequiredForFeatureTag(IN IMS_UINT32 nOption) const;
    IMS_BOOL IsReconnectingServerSocketErrorAllowed() const;
    IMS_BOOL IsRegTypeEqual(IN AosRegistrationType eType) const;
    IMS_BOOL IsRegTrying() const;
    IMS_BOOL IsNetworkBindingSupported(IN IAosHandle* piHandle);
    IMS_BOOL IsNetworkFeatureBindingSupported(IN IAosHandle* piHandle);
    IMS_BOOL IsCallStateRequired() const;

    IMS_SINT32 GetRegExpires();

    void IncreaseConsecutiveFailCount();
    IMS_BOOL UpdatePreloadedRoute(
            IN const AString& strPcscf = AString::ConstNull(), IN const IMS_UINT32 nPcscfPort = 0);

    AosNetworkType GetNetworkTypeForImsRegState();
    IMS_SINT32 GetRegIpcanCategory();
    IMS_UINT32 GetRegFeatures();

    /// Set Detail State
    void UpdateDetailState(IN IMS_UINT32 nState);

    /// Log
    AString FeatureToString();

    /// ImsActivityEx
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMsg);

    /// Initialize
    virtual void Init();
    virtual void InitFeatures();

    virtual void CleanUp();
    virtual void DestroyEx();

    virtual IMS_BOOL IsGeolocationInfoRequired();
    virtual IMS_BOOL IsHandlingServerSocketErrorRequired(IN IMS_SINT32 nReason);

    /// Notify
    virtual void ReportStateChanged(IN IMS_UINT32 nResult, IN IMS_UINT32 nReason = 0);
    virtual void PreNotify(IN IMS_UINT32 nReason);
    virtual void ReportTryingState();

    /// Registration
    virtual void PrepareRegistration();
    virtual IMS_BOOL CreateRegistration();
    virtual void DestroyRegistration();

    virtual IMS_BOOL StartRegBinding();
    virtual IMS_BOOL UpdateRegBinding();
    virtual IMS_BOOL UpdateNetworkRegBinding();
    virtual IMS_BOOL UpdateNetworkRegFeatureBinding();

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
    virtual void AddFeatureTagForMtc(IN IMS_UINT32 nRegFeatures, IN IMS_BOOL bFinalFeatureTag);
    virtual void RemoveFeatureTag(IN IAosHandle* piHandle);
    virtual void RemoveFeatureTagForMtc(IN IMS_UINT32 nRegFeatures);
    virtual IMS_BOOL UpdateFeatureTag(IN IAosHandle* piHandle);
    virtual void UpdateFinalAddFeatureTag();

    virtual IMS_BOOL SetAor();
    virtual IMS_BOOL SetPcscf();
    virtual void SetRefreshPolicy();
    virtual void SetFailureState();
    virtual void SetRetryState();
    virtual void SetTcpCriterionLength();
    virtual void SetDefaultTransport();
    virtual void SetStaticIpQos();
    virtual void SetDynamicIpQos();
    virtual void SetActiveBindingsRestorationUsage();

    virtual void UpdateTransactionStarted();

    /// Recovery
    virtual IMS_UINT32 GetActualWaitTime();
    virtual IMS_UINT32 GetUpperBoundTime();
    virtual IMS_BOOL SetFirstPcscf(IN IMS_BOOL bUpdateParameter = IMS_TRUE);
    virtual IMS_BOOL SetNextPcscf(IN IMS_BOOL bUpdateParameter = IMS_TRUE);
    virtual IMS_BOOL TryNextPcscf();
    virtual IMS_BOOL TryNextPcscf(
            IN IMS_BOOL bFlowRecoveryOnAllFail, IN IMS_BOOL bHonorRetryAfter = IMS_FALSE);
    virtual IMS_BOOL IsNextPcscf();
    virtual IMS_BOOL IsRetryStopped();

    /// Clear
    virtual void ClearRegParameters(IN IMS_BOOL bClearPcscf = IMS_TRUE);
    virtual void ClearPcscf();
    virtual void ClearRetryCount();
    virtual void ClearRetryValues(IN IMS_BOOL bRegSuccess = IMS_FALSE);
    virtual void ClearAuthChallengedCount();
    virtual void ClearErrorCount();
    virtual void ClearNetworkBindingFeatures();
    virtual void DestroySocket();

    virtual void ReportRegState();

    virtual void CheckPending();
    virtual IMS_BOOL CheckTrmReadyAndSetTxnPending();
    virtual void ProcessSetIpsec(IN IMS_UINT32 nReason);
    virtual void ProcessRefreshRegInfo();
    virtual void ProcessIpcanChanged();
    virtual void ProcessScscfRestoration();
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
    virtual void ProcessRegRequiredWithAvailableNextPcscf(IN IMS_BOOL bSetCurrentPcscfInvalid);
    virtual void ProcessSubReinitiate();
    virtual IMS_BOOL ProcessForbiddenFailed(IN IMS_SINT32 nStatusCode);
    virtual IMS_BOOL ProcessSubscriberFailed(IN IMS_SINT32 nStatusCode);

    virtual IMS_BOOL ProcessAkaResponseFailed();
    virtual void ProcessAwtRecovery();
    virtual void ProcessIpsecFallback(IN IMS_BOOL bIsIpsecRetry);

    virtual void ProcessDefaultFlowRecovery_Start(IN IMS_SINT32 nStatusCode = 0);
    virtual void ProcessDefaultFlowRecovery_StartWithEveryPcscfPolicy(IN IMS_UINT32 nRetryAfter);
    virtual void ProcessDefaultFlowRecovery_StartWithSpecifiedIntervalPolicy(
            IN IMS_UINT32 nRetryAfter);
    virtual void ProcessDefaultFlowRecovery_Update(IN IMS_SINT32 nStatusCode = 0);
    virtual void ProcessDefaultFlowRecovery_UpdateWithSpecifiedIntervalPolicy(
            IN IMS_SINT32 nStatusCode, IN IMS_UINT32 nRetryAfter);

    virtual IMS_BOOL ProcessStartFailed_305();
    virtual void ProcessStartFailed_403();
    virtual void ProcessStartFailed_420();
    virtual void ProcessStartFailed_421();
    virtual void ProcessStartFailed_423();
    virtual void ProcessStartFailed_503();

    virtual IMS_BOOL ProcessUpdateFailed_305();
    virtual void ProcessUpdateFailed_403();
    virtual void ProcessUpdateFailed_423();

    virtual void ProcessStartFailed_StatusCode(IN IMS_SINT32 nStatusCode);
    virtual void ProcessStartFailed_TxnTimeout();
    virtual void ProcessStartFailed_Others(IN IMS_SINT32 nReason);

    virtual void ProcessUpdateFailed_StatusCode(IN IMS_SINT32 nStatusCode);
    virtual void ProcessUpdateFailed_TxnTimeout();
    virtual void ProcessUpdateFailed_Others(IN IMS_SINT32 nReason);

    virtual void ProcessStandardPcscfSelection();

    virtual void RecordImpu();

    /// IRegistrationListener
    virtual void Registration_AuthenticationChallenged(
            IN IMS_SINT32 nAlgorithm, OUT IMS_BOOL& bResponseToChallenge);
    virtual void Registration_NotifyAkaResponse(IN IMS_SINT32 nResult, IN const ByteArray& objIK,
            IN const ByteArray& objCK, OUT IMS_BOOL& bResultOfSA);
    virtual void Registration_RefreshTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh);
    virtual void Registration_Started();
    virtual void Registration_StartFailed(IN IMS_SINT32 nReason);
    virtual void Registration_Updated();
    virtual void Registration_UpdateFailed(IN IMS_SINT32 nReason);
    virtual void Registration_Removed();
    virtual void Registration_Terminated(IN IMS_SINT32 nReason);

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
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

    // Subscription
    virtual IMS_BOOL CreateSubscription();
    virtual IMS_BOOL DestroySubscription();
    virtual IMS_BOOL StartSubscription();
    virtual IMS_BOOL StopSubscription();

    virtual AosSubscription* GetSubscription(IN IRegSubscription* piRegSubscription);

    virtual void ProcessSubscription_Success();
    virtual void ProcessSubscription_Failed();
    virtual void ProcessSubscription_Terminated(IN IMS_SINT32 nTerminateType = 0);

    virtual void ProcessRegEventRegistered();
    virtual void UpdateReason();

    /// IAosSubscriptionListener
    virtual void Subscription_StateChanged(IN IMS_SINT32 nState, IN IMS_SINT32 nReason = 0);
    virtual IMS_BOOL Subscription_CanBeTransmitted();
    virtual void Subscription_NotifyReceived(IN IMS_SINT32 nEvent);
    virtual void Subscription_Request(IN IMS_SINT32 nCommand, IN IMS_SINT32 nRetryAfter = 0);

    /// Ipsec Helper
    virtual void CreateIpsecHelper();
    virtual void DestroyIpsecHelper();

    /// KeepAlive
    virtual void StartKeepAlive();
    virtual void StopKeepAlive();

    /// IAosKeepAliveListener
    virtual void KeepAlive_DetectedFlowFailed();

    /// IAosBlockListener
    virtual void Block_Changed(IN IMS_UINT32 nType = 0, IN IMS_UINT32 nParam = 0);

    /// IAosCallTrackerListener
    virtual void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState);

    /// IAosNConfigurationListener
    virtual void NConfiguration_NotifyConfigChanged();

    /// IAosNetTrackerListener
    virtual void NetTracker_StatusChanged(){};

    /// IAosTrmListener
    virtual void Trm_PriorityChanged();

    virtual IMS_RESULT MessageMediator_AdjustMessage(
            IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nMessage = MESSAGE_NORMAL);

    virtual IMS_BOOL AddLocationHeaderBody(
            IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nMessage = MESSAGE_NORMAL);

private:
    void ControlPrivateHeader();
    IMS_UINT32 GetSpecificErrWaitTime();
    void ProcessImsiBasedSubscriber();
    void SetContactAddressConfiguration(IN IMS_BOOL bAdd);
    void SetPlaniHeader();
    void UpdateUserInfoInContact();
    void UpdateRegIpcanCategory();
    void UpdateCallingNumberVerification();

    IMS_BOOL IsErrorCodeExisted(
            IN const IMSVector<IMS_SINT32>& objErrorCode, IN IMS_SINT32 nCode) const;
    IMS_BOOL IsErrorCodeExistedForSpecificRegistration(IN IMS_SINT32 nCode) const;
    IMS_BOOL IsPdnReactivationRequired();
    IMS_BOOL IsRegExpiredDuringAwt(IN IMS_UINT32 nAwt);

protected:
    enum
    {
        MSG_REG_START = AOSMSG_SERVICE_INTERNAL,

        MSG_REG_REINITIATE,
        MSG_REG_UPDATE,
        MSG_REG_RECONFIG,

        MSG_REG_REQUIRED_WITH_WAIT_TIME,
        MSG_REG_REQUIRED_WITH_NEXT_PCSCF,
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
        FEATURE_IPSEC = 0x02,
        FEATURE_TRM = 0x04,
        FEATURE_TRM_BLOCK = 0x08,
        FEATURE_VONR = 0x10
    };

    enum
    {
        PENDING_NONE = 0x0,
        PENDING_TRANSACTION = 0x1,
        PENDING_UPDATE = 0x2,
        PENDING_RECONFIG = 0x4,
        PENDING_UPDATE_HELD_BY_CALL = 0x8,

        PENDING_SUBSCRIPTION = 0x10,
        PENDING_TERMINATED = 0x20
    };

    enum
    {
        IMS_REG_STATE_DEREGISTERED = 0,
        IMS_REG_STATE_REGISTERING,
        IMS_REG_STATE_REGISTERED
    };

protected:
    IAosAppContext* m_piContext;
    IAosRegistrationListener* m_piListener;

    IMS_SINT32 m_nSlotId;

    /// engine member
    RegistrationManager* m_pRegManager;
    IRegistration* m_piRegistration;
    IRegContact* m_piRegContact;
    IRegParameter* m_piRegParameter;

    /// subscription
    AosSubscription* m_pSubscription;

    /// Ipsec member
    AosIpsecHelper* m_pIpsecHelper;
    IMS_BOOL m_bIsIpsecSupported;
    IMS_BOOL m_bIsIpsecInit;

    /// keepalive
    AosKeepAlive* m_pKeepAlive;

    /// object
    AosUtil* m_pUtil;

    // Trm
    IAosTrm* m_piTrm;

    /// Vonr
    IAosVonr* m_piVonr;

    /// feature
    IMS_UINT32 m_nFeature;

    /// state
    IMS_UINT32 m_nState;

    /// update & transaction
    IMS_UINT32 m_nTxnPending;
    IMS_BOOL m_bIsTransactionStarted;
    IMS_BOOL m_bIsImsCall;
    IMS_BOOL m_bIsBlocked;
    IMS_BOOL m_bIsTrmBlocked;
    IMS_BOOL m_bIsHeldByCall;
    IMS_BOOL m_bIsAppReady;

    /// reg info
    AString m_strRegId;              /// aos_reg_0
    AosRegistrationType m_eRegType;  /// normal, emergency, fake
    IMS_UINT32 m_nFlowId;            /// 1, 2, 3, ...
    IMS_BOOL m_bFakeReg;             /// fake registration for emergency
    IMS_UINT32 m_nRegMode;           /// normal, limited, fake

    /// this is used for registration
    AString m_strPuid;        /// current
    IPAddress m_objIpa;       /// local IP
    AString m_strPcscf;       /// current Pcscf Address
    IMS_UINT32 m_nPcscfPort;  /// current Pcscf Port

    /// throttling
    IMS_UINT32 m_nRetryBaseTime;       /// base-time for flow recovery in RFC 5626
    IMS_UINT32 m_nRetryMaxTime;        /// max-time for flow recovery in RFC 5626
    IMS_UINT32 m_nUpperBoundWaitTime;  /// used for flow recovery in RFC 5626
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

    /// socket error counter
    IMS_UINT32 m_nErrorCountForServerSocket;

    /// Calling number verification supported
    IMS_BOOL m_bCallingNumberVerificationSupported;

    /// network binding features
    IMS_UINT32 m_nNetworkBindingFeatures;

    /// the state for notifying ims registration callback in telephony ims
    IMS_UINT32 m_nImsRegState;

    /// the features for notifying ims registration callback in telephony ims
    IMS_UINT32 m_nImsRegFeatures;

    /// the network that is notified with registration callback of telephony ims
    AosNetworkType m_eImsRegNetwork;

    /// this is used to set SIP Profile on run-time
    RcPtr<SipProfile> m_pSipProfile;

    AString m_strTag;

    static const IMS_UINT32 INTERNAL_ERROR_INTERVAL = 3;   // 3 Sec.
    static const IMS_UINT32 RETRY_DEFAULT_WAIT_TIME = 30;  // 30 Sec

private:
    friend class AosRegistrationTest;

    /// IPCAN category being registered
    IMS_SINT32 m_nRegIpcanCategory;
    IMS_UINT32 m_nPdnReactivateWaitTime;

    static const IMS_UINT32 RECONNECT_SERVER_SOCKET_ERROR_MAX_COUNT = 10;
    static const IMS_UINT32 AUTHENTICATION_RETRY_MAX_COUNT = 6;
    static const IMS_UINT32 SIP_MTU_MAX_SIZE_VIA_WIFI = 1280;
};

#endif  // AOS_REGISTRATION_H_
