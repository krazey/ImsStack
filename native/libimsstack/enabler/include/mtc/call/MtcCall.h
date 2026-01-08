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

#ifndef MTC_CALL_H_
#define MTC_CALL_H_

#include "AString.h"
#include "ISessionListener.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "base/IRefreshListener.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/MtcPendingOperationHolder.h"
#include "call/MtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/message/MtcMessageMediator.h"
#include "call/radio/IMtcRadioChecker.h"
#include "call/state/IMtcCallState.h"
#include "call/state/MtcCallStateMachine.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/IMtcNetworkWatcherListener.h"
#include "helper/IMtcTimerListener.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "media/IMediaReportEventListener.h"
#include "media/MtcMediaManager.h"
#include "precondition/IMtcPreconditionListener.h"
#include "precondition/MtcPreconditionManager.h"
#include <functional>
#include <memory>

class CallConnectionIdManager;
class CurrentLocationDiscoveryController;
class EpsFallbackTrigger;
class IConferenceManager;
class IEctManager;
class ILastComeFirstServedHelper;
class IMtcAosConnector;
class IMtcCallController;
class IMtcContext;
class IMtcDialingPlan;
class IMtcEmergencyServiceManager;
class IMtcMediaManager;
class IMtcPreconditionManager;
class IMtcService;
class IMtcSession;
class IMtcSipInterfaceFactory;
class IMultiEndpointManager;
class IMutex;
class IPassiveTimerHolder;
class IReference;
class ISession;
class ISipClientConnection;
class MessageSender;
class MessageUtils;
class MtcConfigurationProxy;
class MtcLocationRefresher;
class OperationAsyncRunner;
class SuppService;
class UdpKeepAliveSender;
class UpdatingInfo;
class UssiController;
struct CallReasonInfo;
struct ConfUser;
struct MediaInfo;

class MtcCall final :
        public IMtcCall,
        public IMtcCallContext,
        public ISessionListener,
        public IRefreshListener,
        public IMtcTimerListener,
        public IMtcBlockCheckListener,
        public IMtcPreconditionListener,
        public IMtcCallStateWatcher,
        public IMediaReportEventListener,
        public ISrvccStateListener,
        public IMtcAosStateListener,
        public IMtcNetworkWatcherListener,
        public IMtcRadioCheckerListener
{
public:
    MtcCall(IN IMtcContext& objContext, IN IMtcService& objService, IN const CallInfo& objCallInfo,
            IN std::unique_ptr<IMtcCallStateFactory> pStateFactory, IN const AString& strLogTag);
    virtual ~MtcCall() override;
    MtcCall(IN const MtcCall&) = delete;
    MtcCall& operator=(IN const MtcCall&) = delete;

    void Attach() override;

    void Start(IN CallType eCallType, IN const AString& strTarget, IN MediaInfo& objMediaInfo,
            IN const ImsList<SuppService*>& objSuppServices) override;
    void StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& objUsers) override;
    void StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN const ImsList<ConfUser*>& objUsers) override;
    void HandleIncoming(IN ISession* piSession) override;
    void HandleUserAlert() override;
    void Accept(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    void Reject(IN const CallReasonInfo& objReason) override;
    void Hold(IN MediaInfo& objMediaInfo) override;
    void Resume(IN MediaInfo& objMediaInfo) override;
    void AcceptResume(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    void RejectResume(IN const CallReasonInfo& objReason) override;
    void Update(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    void AcceptUpdate(IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    void RejectUpdate(IN const CallReasonInfo& objReason) override;
    void CancelUpdate(IN const CallReasonInfo& objReason) override;
    void Terminate(IN const CallReasonInfo& objReason) override;
    void SendUssd(IN const AString& strUssd) override;

    inline CallKey GetKey() const override { return m_nKey; }
    CallType GetCallType() const override;
    inline CallStateName GetState() const override { return m_objStateMachine.GetState(); }

    inline IMtcCallContext& GetCallContext() override
    {
        return *(static_cast<IMtcCallContext*>(this));
    }

    inline IMS_UINTP GetCallKey() const override { return m_nKey; }
    inline const AString& GetLogTag() const override { return m_strLogTag; }
    inline IMS_BOOL IsEstablished() const override { return m_bEstablished; }
    inline IMS_BOOL IsHeldByMe() const override { return m_bHeldByMe; }
    inline IMS_BOOL IsOnUnconfirmedRemoteHold() const override { return m_bUnconfirmedRemoteHold; }
    inline IMS_BOOL IsUssi() const override { return m_objCallInfo.bUssi; }
    IMS_BOOL IsCsfbAvailable() override;
    inline CallInfo& GetCallInfo() override { return m_objCallInfo; }
    inline ParticipantInfo& GetParticipantInfo() override { return m_objParticipantInfo; }
    IMtcSession* GetSession(IN const ISession* piSession) const override;
    IMtcSession* GetSession() const override;
    inline const ImsList<IMtcSession*>& GetSessions() const override { return m_lstSessions; }
    inline IMtcService& GetService() const override { return m_objService; }
    inline IMtcUiNotifier& GetUiNotifier() override { return m_objUiNotifier; }
    inline IMtcMediaManager& GetMediaManager() override { return m_objMediaManager; }
    inline IMtcPreconditionManager& GetPreconditionManager() override
    {
        return m_objPreconditionManager;
    }
    inline UssiController* GetUssiController() override { return m_pUssiController; }
    inline MtcPendingOperationHolder& GetPendingOperationHolder() override
    {
        return m_objPendingOperationHolder;
    }
    inline IMtcCall& GetCall() override { return *(static_cast<IMtcCall*>(this)); }
    inline ImsList<IMtcCall*> GetOtherCalls() override
    {
        return GetCallManager().GetCallsExcluding(GetKey());
    }
    UpdatingInfo& GetUpdatingInfo() override;
    EpsFallbackTrigger& GetEpsFallbackTrigger() override;
    CurrentLocationDiscoveryController& GetCurrentLocationDiscoveryController() override;
    IMtcSession* CreateSession(IN ISession* piSession) override;
    IMtcSession* CreateSession() override;
    IMtcBlockChecker* CreateBlockChecker(IN const ImsList<IMtcBlockRule*>& lstRules) override;
    JniCallInfo CreateJniCallInfo() override;
    UdpKeepAliveSender* CreateUdpKeepAliveSender() override;
    void RemoveSession(IN IMtcSession& objSession) override;
    void RemoveAllSessions() override;
    void DeleteUpdatingInfo() override;
    void RunPendingOperationIfPossible() override;

    inline MtcTimerWrapper& GetTimer() const override { return *m_pTimer; }
    inline MtcSupplementaryService& GetSupplementaryService() override
    {
        return m_objSupplementaryService;
    }
    inline IMS_SINT32 GetSlotId() const override { return m_objContext.GetSlotId(); }
    inline const ISubscriberConfig* GetSubscriberConfig() const override
    {
        return m_objContext.GetSubscriberConfig();
    }
    inline IMtcDialingPlan& GetDialingPlan() override { return m_objContext.GetDialingPlan(); }
    inline IMtcService* GetServiceByType(IN ServiceType eServiceType) override
    {
        return m_objContext.GetServiceByType(eServiceType);
    }
    inline IMtcCallManager& GetCallManager() override { return m_objContext.GetCallManager(); }
    inline IMtcRadioChecker& GetRadioChecker() override { return m_objContext.GetRadioChecker(); }
    inline IMtcCallController& GetCallController() override
    {
        return m_objContext.GetCallController();
    }
    inline MtcConfigurationProxy& GetConfigurationProxy() override
    {
        return m_objContext.GetConfigurationProxy();
    }
    inline ICallStateProxy& GetCallStateProxy() override
    {
        return m_objContext.GetCallStateProxy();
    }
    inline IMtcImsEventReceiver& GetImsEventReceiver() override
    {
        return m_objContext.GetImsEventReceiver();
    }
    inline IMtcAosConnector* GetAosConnector(IN ServiceType eServiceType) override
    {
        return m_objContext.GetAosConnector(eServiceType);
    }
    inline IMtcSipInterfaceFactory& GetSipInterfaceFactory() override
    {
        return m_objContext.GetSipInterfaceFactory();
    }
    inline IConferenceManager& GetConferenceManager() override
    {
        return m_objContext.GetConferenceManager();
    }
    inline IEctManager& GetEctManager() override { return m_objContext.GetEctManager(); }
    inline IMtcEmergencyServiceManager& GetEmergencyServiceManager() override
    {
        return m_objContext.GetEmergencyServiceManager();
    }
    inline void RunAsyncOperation(IN void* pOwner, IN std::function<void()> objOperation) override
    {
        m_objContext.RunAsyncOperation(pOwner, objOperation);
    }
    inline void ReleaseAsyncOperation(IN void* pOwner) override
    {
        m_objContext.ReleaseAsyncOperation(pOwner);
    }
    inline IMessageUtils& GetMessageUtils() override { return m_objContext.GetMessageUtils(); }
    inline std::unique_ptr<MtcTimerWrapper> CreateTimer() override
    {
        return m_objContext.CreateTimer();
    }
    inline IPassiveTimerHolder& GetPassiveTimerHolder() override
    {
        return m_objContext.GetPassiveTimerHolder();
    }
    inline IMultiEndpointManager* GetMultiEndpointManager() override
    {
        return m_objContext.GetMultiEndpointManager();
    }
    inline ILastComeFirstServedHelper& GetLastComeFirstServedHelper() override
    {
        return m_objContext.GetLastComeFirstServedHelper();
    }
    inline CallConnectionIdManager& GetCallConnectionIdManager() override
    {
        return m_objContext.GetCallConnectionIdManager();
    }
    inline MtcLocationRefresher& GetLocationRefresher() override
    {
        return m_objContext.GetLocationRefresher();
    }
    inline void CreateRttAutoUpgrader() override { m_objContext.CreateRttAutoUpgrader(); }
    inline void DestroyRttAutoUpgrader() override { m_objContext.DestroyRttAutoUpgrader(); }
    inline IMS_BOOL IsWifiTestMode() override { return m_objContext.IsWifiTestMode(); }
    // end of IMtcContext

    inline void SetHeldByMe(IN IMS_BOOL bHeldByMe) override { m_bHeldByMe = bHeldByMe; }
    inline void SetUnconfirmedRemoteHold(IN IMS_BOOL bUnconfirmedRemoteHold) override
    {
        m_bUnconfirmedRemoteHold = bUnconfirmedRemoteHold;
    }

    void SessionAlerting(IN ISession* piSession) override;
    void SessionReferenceReceived(IN ISession* piSession, IN IReference* piReference) override;
    void SessionStarted(IN ISession* piSession) override;
    void SessionStartFailed(IN ISession* piSession) override;
    void SessionTerminated(IN ISession* piSession) override;
    void SessionUpdated(IN ISession* piSession) override;
    void SessionUpdateFailed(IN ISession* piSession) override;
    void SessionUpdateReceived(IN ISession* piSession) override;
    void SessionCanceledOnAccepted(IN ISession* piSession) override;
    void SessionCancelDelivered(IN ISession* piSession) override;
    void SessionCancelDeliveryFailed(IN ISession* piSession) override;
    void SessionEarlyMediaUpdated(IN ISession* piSession) override;
    void SessionEarlyMediaUpdateFailed(IN ISession* piSession) override;
    void SessionEarlyMediaUpdateReceived(IN ISession* piSession) override;
    void SessionForkedResponseReceived(
            IN ISession* piSession, IN ISession* piForkedSession) override;
    void SessionPrackDelivered(IN ISession* piSession) override;
    void SessionPrackDeliveryFailed(IN ISession* piSession) override;
    void SessionPrackReceived(IN ISession* piSession) override;
    void SessionProvisionalResponseReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    void SessionRprDeliveryFailed(IN ISession* piSession) override;
    void SessionRprReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    void SessionTransactionReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) override;

    void Refresh_NotifyCompleted(IN ISipClientConnection* piScc) override;
    void Refresh_NotifyTerminated() override;
    void Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) override;

    void OnTimerExpired(IN IMS_SINT32 nType) override;

    void OnBlockChecked(IN IMtcBlockChecker::Result objResult) override;

    void QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    void QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) override;

    void OnStateTransition(IN CallStateName eState) override;

    void OnReceivingMediaDataStarted(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) override;
    void OnReceivingMediaDataFailed(IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) override;
    void OnVideoLowestBitRate() override;
    void OnReceivingNetworkToneStarted() override;
    void OnReceivingNetworkToneFailed() override;
    void OnMediaFailed(IN const CallReasonInfo& objReason) override;

    void OnSrvccStateUpdated(IN SrvccState eState) override;

    void OnAosStateChanged(IN IMtcService& objMtcService, IN MtcAosState eState,
            IN IMS_UINT32 eAosReason, IN IMS_SINT32 nDataFailureReason) override;
    void OnEventNotify(IN IMS_UINT32 nType, IN IMS_UINT32 nState) override;

    void OnRatChanged(IN ServiceType eServiceType, IN IMS_SINT32 eOldRatType,
            IN IMS_SINT32 eRatType) override;

    inline void OnConnectionSetupPrepared() override {}
    void OnConnectionFailed(IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis) override;
    const AString ToString() const;

private:
    static IMutex* s_pKeyCreationLock;

    static CallKey CreateCallKey();
    AString CreateLogTag(IN const AString& strLogTag) const;
    void OnInternalFailure();
    IMS_BOOL IsInUpdateAfterConnectedDelay() const;

    IMtcContext& m_objContext;
    IMtcService& m_objService;

    CallKey m_nKey;
    AString m_strLogTag;

    IMS_BOOL m_bEstablished;
    IMS_BOOL m_bHeldByMe;
    IMS_BOOL m_bUnconfirmedRemoteHold;

    CallInfo m_objCallInfo;
    ParticipantInfo m_objParticipantInfo;
    UpdatingInfo* m_pUpdatingInfo;
    ImsList<IMtcSession*> m_lstSessions;
    MtcCallStateMachine m_objStateMachine;
    MtcPendingOperationHolder m_objPendingOperationHolder;
    std::unique_ptr<MtcTimerWrapper> m_pTimer;
    MtcUiNotifier m_objUiNotifier;
    MtcMediaManager m_objMediaManager;
    MtcPreconditionManager m_objPreconditionManager;
    MtcSupplementaryService m_objSupplementaryService;
    MtcMessageMediator m_objMessageMediator;
    UssiController* m_pUssiController;
    EpsFallbackTrigger* m_pEpsFallbackTrigger;
    CurrentLocationDiscoveryController* m_pCurrentLocationDiscoveryController;
};

#endif
