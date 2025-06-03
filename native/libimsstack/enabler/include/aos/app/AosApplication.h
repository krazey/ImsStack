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
#ifndef AOS_APPLICATION_H_
#define AOS_APPLICATION_H_

#include "ImsActivityEx.h"
#include "ImsStateMachine.h"
#include "IEventListener.h"
#include "ITimer.h"

#include "interface/AosInternalMsgDef.h"
#include "AosReason.h"
#include "interface/IAosApplication.h"
#include "interface/IAosBlock.h"
#include "interface/IAosCallTrackerListener.h"
#include "interface/IAosConditionListener.h"
#include "interface/IAosConnectorListener.h"
#include "interface/IAosNConfigurationListener.h"
#include "interface/IAosNetTrackerListener.h"
#include "interface/IAosRegistrationControlListener.h"
#include "interface/IAosRegistrationListener.h"
#include "interface/IAosServicePhoneListener.h"
#include "provider/AosStaticProfile.h"

class IAosAppContext;
class IAosRegistration;
class IAosCallTracker;
class IAosNetTracker;
class AosCondition;
class AosConnector;
class AosUtil;

enum class AosNetworkType;
enum class AosReasonCode;

class AosApplication :
        public ImsActivityEx,
        public ImsStateMachine,
        public IAosApplication,
        public IAosConditionListener,
        public IAosConnectorListener,
        public IAosRegistrationListener,
        public IAosCallTrackerListener,
        public IEventListener,
        public ITimerListener,
        public IAosNetTrackerListener,
        public IAosNConfigurationListener,
        public AosRegistrationControlListener,
        public AosServicePhoneListener
{
    DECLARE_STATE_MAP()

    DECLARE_STATE_MSG_MAP(STATE_NOTREADY)
    DECLARE_STATE_MSG_MAP(STATE_READY)
    DECLARE_STATE_MSG_MAP(STATE_CONNECTING)
    DECLARE_STATE_MSG_MAP(STATE_CONNECTED)
    DECLARE_STATE_MSG_MAP(STATE_UPDATING)
    DECLARE_STATE_MSG_MAP(STATE_DISCONNECTING)

public:
    AosApplication(IN IAosAppContext* piAppContext, IN AString& strAppId);
    ~AosApplication() override;

    // IAosApplication
    void Reconfig() override;
    IMS_BOOL RequestCmd(IN IMS_UINT32 nCmdType, IN IMS_UINT32 nReason = 0) override;

    const AString& GetActivityName() override;
    void GetProperty(IN IMS_UINT32 nType, OUT IMS_UINT32& nValue, OUT AString& strValue) override;
    IMS_UINT32 GetAppState() override;
    IMS_UINT32 GetOffReason() override;

    IMS_BOOL IsActivated() override;
    IMS_BOOL IsOn() override;
    IMS_BOOL IsCrossSimConnected() override;

    void SetActivation(IN IMS_BOOL bActivation) override;
    void NotifyEpsFallbackCallState(IN IMS_UINT32 nState) override;
    void NotifyPublishState(IN IMS_BOOL bStart) override;

protected:
    void ClearOffReason();
    void ClearPending();
    void ClearWifiRegBlock();
    void ClearDataFailureReason();

    AosNetworkType GetNetworkTypeForImsRegState() const;

    void SetOffReason(IN IMS_UINT32 nReason);
    void SetImsCall(IN IMS_BOOL bActive);
    void SetPublishState(IN IMS_BOOL bActive);
    void SetRegRecoveryHeld(IN IMS_BOOL bHeld);
    void SetDataFailureReason(IN IMS_SINT32 nReason);

    void ResetBlock(IN BLOCK_REASON nReason);
    void NotifyDeregistered(IN AosReasonCode eReason);
    void AddRatBlock();
    void ClearRatBlocks();
    void PerformRatBlockActions(IN IMS_BOOL bStart);

    IMS_BOOL IsEmergency() const;
    IMS_BOOL IsStateMessage(IN IMS_UINT32 nMsg) const;
    IMS_BOOL IsNotReady();
    IMS_BOOL IsEqualOrLessState(IN IMS_UINT32 nState);
    IMS_BOOL IsRegRecoveryHeld() const;
    IMS_BOOL IsImsCall() const;
    IMS_BOOL IsPublished() const;
    IMS_BOOL IsAllDetached() const;
    IMS_BOOL IsTimerRunning(IN IMS_UINT32 nType) const;
    IMS_BOOL IsRegTypeNormal() const;
    IMS_BOOL IsRegStateUpdatedByNrLteRatChange() const;
    IMS_BOOL IsPdnDisconnectRequired() const;
    IMS_BOOL IsPlmnBlockRequired() const;
    IMS_BOOL IsBlockRat(IN IMS_UINT32 nRat) const;
    IMS_BOOL IsReasonBlockedForImsEstablishmentTimer() const;
    IMS_BOOL IsImsEstablishmentTimerStopRequired() const;
    IMS_BOOL IsPdnDeactivationRequired() const;

    IMS_SINT32 GetImsEstablishmentTime() const;

    // Create
    virtual void CreateAosCondition();
    virtual void CreateAosConnector();
    virtual void CreateAosLocationStarter(IN IMS_BOOL bInitiation = IMS_TRUE);

    virtual void AddEventListener();
    virtual void RemoveEventListener();
    virtual void SetNetTrackerListener();

    virtual void SetAppType(IN AosRegistrationType eRegType);
    virtual void SetAppState(IN IMS_UINT32 nState);
    virtual void SetCleanState();

    virtual IMS_BOOL IsUpdateAvailable();
    virtual IMS_BOOL IsRegReconfigAvailable() const;
    virtual IMS_BOOL IsReconfigHandleChanged() const;
    virtual IMS_BOOL IsRequestCmdHeldByCondition(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason = 0);
    virtual IMS_BOOL IsAllHandleDetached() const;
    virtual IMS_BOOL IsConditionTimerSkippedDueToTimer() const;
    virtual IMS_BOOL IsRegUpdatedByNrLteRatChange() const;

    // Clean
    virtual void CleanAll(IN IMS_UINT32 nOffReason = AosReason::NONE);
    virtual void ClearConnection();
    virtual void ClearConnector();

    virtual IMS_UINT32 GetReportState();
    virtual IMS_SINT32 GetDataFailureReason() const;

    // ImsActivityEx
    IMS_BOOL OnMessage(IN IMSMSG& objMsg) override;

    // Message
    virtual IMS_BOOL ProcessMessage(IN IMSMSG& objMsg);
    virtual void ProcessRegStart(IN IMSMSG& objMsg);
    virtual void ProcessRegUpdate(IN IMSMSG& objMsg);
    virtual void ProcessRegStop(IN IMSMSG& objMsg);
    virtual void ProcessRegReconfig(IN IMSMSG& objMsg);
    virtual void ProcessRegRecovery(IN IMSMSG& objMsg);
    virtual void ProcessIpcanChanged(IN IMSMSG& objMsg);
    virtual void ProcessDestroy(IN IMSMSG& objMsg);
    virtual void ProcessImsEstablishmentControl(IN IMSMSG& objMsg);
    virtual void ProcessRegExchange(IN IMSMSG& objMsg);
    virtual void ProcessAutoConfigurationComplete(IN IMSMSG& objMsg);
    virtual void ProcessPcscfRecovery(IN IMSMSG& objMsg);
    virtual void ProcessScscfRestoration(IN IMSMSG& objMsg);
    virtual void ProcessRegRetryCount(IN IMSMSG& objMsg);
    virtual void ProcessOthers(IN IMSMSG& objMsg);

    // StateMachine
    virtual IMS_BOOL PreprocessStateMessage(IN IMSMSG& objMsg);
    virtual IMS_BOOL PreprocessStateMessage_Connection(IN IMSMSG& objMsg);
    virtual IMS_BOOL PreprocessStateMessage_Condition(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateNotReady_Condition(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateNotReady_Connection(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateReady_Condition(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateReady_Connection(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateConnecting_Condition(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateConnecting_Connection(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateConnecting_Registration(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateConnected_Condition(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateConnected_Connection(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateConnected_Registration(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateUpdating_Condition(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateUpdating_Connection(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateUpdating_Registration(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateDisconnecting_Condition(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateDisconnecting_Connection(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateDisconnecting_Registration(IN IMSMSG& objMsg);

    virtual void ProcessRegTrying_StateConnecting(IN IMS_UINT32 nReason);
    virtual void ProcessRegFailed_StateConnecting(IN IMS_UINT32 nReason);
    virtual void ProcessRegTrying_StateConnected(IN IMS_UINT32 nReason);
    virtual void ProcessRegFailed_StateConnected(IN IMS_UINT32 nReason);
    virtual void ProcessRegTrying_StateUpdating(IN IMS_UINT32 nReason);
    virtual void ProcessRegFailed_StateUpdating(IN IMS_UINT32 nReason);
    virtual void ProcessConnectionUpdated_StateDisconnecting(IN IMS_UINT32 nReason);

    virtual void ProcessConnectionDeactivated(IN IMS_UINT32 nReason);
    virtual void ProcessConnectionUpdated(IN IMS_UINT32 nReason);
    virtual void ProcessConnectionUpdated_Pcscf();

    virtual void ProcessRegSucceeded(IN IMS_UINT32 nReason);
    virtual void ProcessRegFailed_Start(IN IMS_UINT32 nReason);
    virtual void ProcessRegFailed_Update(IN IMS_UINT32 nReason);
    virtual void ProcessRegFailed_Terminated();

    virtual void ProcessDisconnectingState(IN IMS_UINT32 nReason = 0);
    virtual void ProcessNetworkEvent(
            IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_UINT32 nStateEx);
    virtual void ProcessStateStart(IN IMS_UINT32 nTime = 0);
    virtual void ProcessRegControlEvent(IN IMS_UINT32 nType, IN IMS_UINT32 nReason);
    virtual void ProcessRegInternalFailed(IN IMS_UINT32 nReason = 0);
    virtual void ProcessRegAuthenticationFailed();
    virtual void ProcessRegForbiddenInWifi();
    virtual void ProcessRegUsimAuthenticationFailed();
    virtual void ProcessRegTerminated();
    virtual void ProcessPingCommand();
    virtual void ProcessRegTerminating();
    virtual void ProcessPdnDisconnect();
    virtual void ProcessRoamingState(IN IMS_BOOL bRoaming);

    virtual void ProcessAppActivatedTimerExpired();
    virtual void ProcessAppConnectedTimerExpired();
    virtual void ProcessAppTerminatedTimerExpired();
    virtual void ProcessReconfigTimerExpired();
    virtual void ProcessRegBlockedTimerExpired();
    virtual void ProcessRegStopTimerExpired();
    virtual void ProcessPdnBlockedTimerExpired();
    virtual void ProcessImsEstablishmentTimerExpired();
    virtual void ProcessRatBlockTimerExpired();
    virtual void ProcessPdnBlock();
    virtual void ProcessPdnBlockWithTime();

    virtual void ProcessImsEstablishmentStart();
    virtual void ProcessPlmnBlock(IN AosReasonCode eReason);

    // Report to Handle
    virtual void Report_StateChanged(IN IMS_BOOL bIsStateChecked = IMS_TRUE);
    virtual void Report_Notify();
    virtual void Report_Request(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

    virtual IMS_BOOL UpdateRegRecoveryHeld();
    virtual IMS_BOOL UpdateRegStopHeld();

    // Timer
    virtual void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    virtual void StopTimer(IN IMS_UINT32 nType);
    virtual void ClearTimers();

    // To External Interface
    virtual void UpdateRegState();
    virtual IMS_UINT32 UpdateConnectedServices(IN IMS_BOOL bEnforceUpdateRegService);
    virtual void UpdateRegisteredRat(IN IMS_UINT32 nRegisteredRat);
    virtual void UpdateMonitorNotify(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

    // IAosApplication
    void Init() override;
    void CleanUp() override;

    // IAosConditionListener
    void Condition_Changed(IN IMS_UINT32 nReason = 0) override;
    void Condition_RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason = 0) override;

    // IAosConnectorListener
    void Connector_Activated() override;
    void Connector_Deactivated(IN IMS_UINT32 nReason) override;
    void Connector_Updated(IN IMS_UINT32 nReason) override;

    // IAosRegistrationListener
    void Registration_StateChanged(IN IMS_UINT32 nResult, IN IMS_UINT32 nReason = 0) override;

    // IAosCallTrackerListener
    void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState) override;
    inline void CallTracker_ECallSessionReleased(IN IMS_BOOL /* bEstablished */) override {};

    // IAosNetTrackerListener
    void NetTracker_StatusChanged() override;

    // IAosNConfigurationListener
    void NConfiguration_NotifyConfigChanged() override;

    // IEventListener
    void Event_NotifyEvent(
            IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) override;

    // ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    // AosRegistrationControlListener
    void RegistrationControl_ControlRegistration(IN AosRegRequestType eType,
            IN AosPcscfOrder eOrder, IN AosControlCause eCause) override;
    void RegistrationControl_UpdateDataFailureReason(IN IMS_SINT32 nReason) override;

    // AosServicePhoneListener
    void ServicePhone_LocationInfoChanged(IN LocationInfo eState) override;

public:
    static const IMS_UINT32 RECONFIG_GUARD_TIME_MILLIS = 1000;
    static const IMS_UINT32 REG_STOP_WAITING_TIME_MILLIS = 1000;
    static const IMS_UINT32 APP_START_WAITING_TIME_MILLIS = 4000;
    static const IMS_UINT32 DELAY_STOPPING_PDN_TO_KEEP_SESSION_TIME_SECONDS = 2;
    static const IMS_UINT32 UNEXPECTED_ERROR_APP_START_WAITING_TIME_MILLIS = 10000;
    static const IMS_UINT32 PLMN_BLOCK_PDN_STOP_WAITING_TIME_SECONDS = 5;
    static const IMS_UINT32 RAT_BLOCK_TIME_MILLIS = 720000;  // 12 MIN

protected:
    enum
    {
        TYPE_NORMAL = 0,
        TYPE_EMERGENCY
    };

    enum
    {
        // State-Machine MSG
        MSG_CONDITION = AOSMSG_SERVICE_INTERNAL,
        MSG_CONNECTION,
        MSG_REGISTRATION,

        // NO State-Machine MSG
        MSG_INIT = AOSMSG_SERVICE_INTERNAL + 10,
        MSG_REG_START,
        MSG_REG_UPDATE,
        MSG_REG_STOP,
        MSG_REG_RECONFIG,
        MSG_REG_RECOVER,
        MSG_IPCAN_CHANGED,
        MSG_PUB_TERMINATED,
        MSG_DESTROY,
        MSG_IMS_EST_TIMER_CONTROL,
        MSG_REG_EXCHANGE,
        MSG_AC_CONFIGURED,
        MSG_PCSCF_RECOVER,
        MSG_SCSCF_RESTORATION,
        MSG_PLMN_BLOCK_WITH_TIMEOUT,
        MSG_RETRY_COUNT_INCREASE,
        MSG_OTHERS
    };

    // INTMSG CONNECTION wParam
    enum
    {
        CONNECTION_ACTIVATED = 10,
        CONNECTION_DEACTIVATED,
        CONNECTION_UPDATED
    };

    // INTMSG REGISTRATION
    // wParam : nReason , lParam : nReasonEx

    // MSG_REG_EXCHANGE wparam
    enum
    {
        REG_EXCHANGE_NEED = 0,
        REG_EXCHANGE_NO_NEED,
        REG_EXCHANGE_AVAILABLE
    };

    // MSG_RETRY_COUNT_INCREASE wparam
    enum
    {
        RETRY_COUNT_REG_NONE = 0,
        RETRY_COUNT_REG_RECOVER
    };

    enum
    {
        TIMER_RECONFIG_GUARD = 0,
        TIMER_MSG_CONDITION,
        TIMER_REG_STOP,
        TIMER_REG_BLOCKED,
        TIMER_APP_ACTIVATED,
        TIMER_APP_CONNECTED,
        TIMER_APP_TERMINATED,
        TIMER_PDN_BLOCKED,
        TIMER_IMS_ESTABLISHMENT,
        TIMER_RAT_BLOCK
    };

    enum
    {
        PENDING_NONE = 0x0,

        // REG PENDING
        PENDING_REG_RECOVERY_HELD = 0x1,

        // REG STOP PENDING
        PENDING_REG_STOP_HELD = 0x2,

        // APP PENDING
        PENDING_APP_DESTROY_HELD = 0x4,

        // REG RECONFIG PENDING
        PENDING_REG_RECONFIG_HELD = 0x8,

        // After CSFB
        PENDING_REG_AFTER_CSFB_COMPLETE = 0x10,

        // IPCAN PENDING
        PENDING_IPCAN_HELD = 0x20,

        // REG UPDATE PENDING
        PENDING_REG_UPDATE_HELD = 0x40
    };

protected:
    IAosAppContext* m_piContext;
    IAosRegistration* m_piRegistration;
    IAosCallTracker* m_piCallTracker;
    IAosNetTracker* m_piNetTracker;
    AosCondition* m_pCondition;
    AosConnector* m_pConnector;
    AosUtil* m_pUtil;

    ITimer* m_piReconfigTimer;
    ITimer* m_piMsgConditionTimer;
    ITimer* m_piRegStopTimer;
    ITimer* m_piRegBlockedTimer;
    ITimer* m_piAppActivatedTimer;
    ITimer* m_piAppConnectedTimer;
    ITimer* m_piAppTerminatedTimer;
    ITimer* m_piPdnBlockedTimer;
    ITimer* m_piImsEstablishmentTimer;
    ITimer* m_piRatBlockTimer;

    AString m_strAppId;
    IMS_UINT32 m_nAppType;
    IMS_UINT32 m_nOffReason;
    IMS_UINT32 m_nRat;
    IMS_UINT32 m_nBlockedRats;
    AosRegistrationType m_eRegType;
    IMS_UINT32 m_nReportState;
    IMS_UINT32 m_nRegPending;
    IMS_UINT32 m_nRegisteredRat;
    IMS_UINT32 m_nRecoverReason;
    IMS_UINT32 m_nLteAttachState;
    IMS_UINT32 m_nLteExtraInfo;
    IMS_UINT32 m_nVoiceServiceState;
    IMS_SINT32 m_nSlotId;
    IMS_SINT32 m_nDataFailureReason;

    IMS_BOOL m_bConnected;
    IMS_BOOL m_bRegRecoveryHeld;
    IMS_BOOL m_bIsImsCall;
    IMS_BOOL m_bIsPublished;
    IMS_BOOL m_bIsActivated;
    IMS_BOOL m_bEpdgEnabled;
    IMS_BOOL m_bDataRoaming;
    IMS_BOOL m_bPdnDeactivationRequired;

    AString m_strTag;
};
#endif  // AOS_APPLICATION_H_
