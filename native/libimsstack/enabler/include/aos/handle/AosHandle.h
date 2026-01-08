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
#ifndef AOS_HANDLE_H_
#define AOS_HANDLE_H_

#include "IAosService.h"
#include "IImsAos.h"
#include "IImsAosInfo.h"
#include "ImsStateMachine.h"
#include "IEventListener.h"
#include "IWifiWatcher.h"
#include "interface/IAosCallTrackerListener.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfigurationListener.h"
#include "interface/IAosNetTrackerListener.h"
#include "interface/IAosNetTrackerTimerListener.h"
#include "interface/IAosRegistrationControlListener.h"
#include "interface/IAosServiceSettingListener.h"

#include "handle/AosFeatureTag.h"

#define TRACE_HDL_I(NAME, VAL) TRACE_AOS_I(m_piAppContext, AosDomain::HDL, (NAME), (VAL))

class IAosAppContext;

class AosHandle :
        public IAosHandle,
        public IAosCallTrackerListener,
        public IAosNetTrackerListener,
        public IAosNConfigurationListener,
        public IEventListener,
        public IImsAos,
        public ImsStateMachine,
        public AosNetTrackerTimerListener,
        public AosRegistrationControlListener,
        public AosServiceSettingListener
{
    DECLARE_STATE_MAP()

    // m_nReqType : DETACH
    DECLARE_STATE_MSG_MAP(STATE_DISCONNECTED)
    // m_nReqType : ATTACH, m_bBind : false (not registered)
    DECLARE_STATE_MSG_MAP(STATE_CONNECTING)
    // m_nReqType : ATTACH, m_bBind : true (registered)
    DECLARE_STATE_MSG_MAP(STATE_CONNECTED)
    /*
        1. m_nReqType : DETACH, this service will be removed in registration
        2. m_nReqType : ATTACH, de-registration will be initiated
    */
    DECLARE_STATE_MSG_MAP(STATE_DISCONNECTING)

public:
    AosHandle(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_UINT32 nServiceType);
    ~AosHandle() override;

    // IAosHandle
    const AString& GetAppId() const override;
    const AString& GetServiceId() const override;
    IMS_UINT32 GetServiceType() const override;

    IImsAosMonitor* GetMonitor() override;

    IMS_SINT32 GetRequestType() const override;
    void SetRequestType(IN IMS_SINT32 nReqType) override;

    IMS_BOOL IsRegBinded() const override;
    void SetRegBinded(IN IMS_BOOL bBind) override;

    IMS_BOOL IsRegFeatureTagRequired() const override;
    IMS_BOOL IsRegToNextPcscfRequested() const override;

    AosFeatureTagList& GetFeatureTagList() override;
    AosFeatureTagList& GetBindedFeatureTagList() override;

    void ProcessFeatureTagChange() override;

    void Request(IN IMS_UINT32 nType, IN IMS_UINT32 nState = 0) override;

    IMS_BOOL App_StateChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nParam) override;
    IMS_BOOL App_Notify() override;

    virtual void Handle_Notify(IN IMS_UINT32 nType, IN IMS_BOOL bBlocked) override;

    // IImsAos
    IMS_BOOL Control(IN IMS_UINT32 nType) override;
    IImsAosInfo* GetAosInfo() override;
    IMS_UINT32 GetFeatures() const override;
    IMS_UINT32 GetSuspendedReason() const override;
    IMS_BOOL IsFeatureConnected(IN IMS_UINT32 nFeature) const override;
    IMS_BOOL IsImsConnected() const override;
    IMS_BOOL IsImsSuspended() const override;
    void SetListener(IN IImsAosListener* piListener) override;
    void SetMonitor(IN IImsAosMonitor* piMonitor) override;
    IMS_BOOL SetReady(IN IMS_BOOL bReady, IN IMS_UINT32 nService) override;
    void UpdateFeature(IN IMS_UINT32 nFeatures) override;
    void UpdateFeature(IN ImsList<ImsAosFeatureTag*>& objFeatureTag) override;
    void RegisterWithNextPcscf(IN IMS_UINT32 nUnavailableTimeForCurrentPcscf) override;
    void ReinitiateRegistration(IN IMS_UINT32 nAfterSec) override;

    // IAosCallTrackerListener
    void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState) override;
    inline void CallTracker_ECallSessionReleased(IN IMS_BOOL /* bEstablished */) override {};

    // IAosNetTrackerListener
    void NetTracker_StatusChanged() override;

    // IAosNetTrackerTimerListener
    void NetTracker_TimerInGuardChanged(IN NetTrackerTimerState eState) override;

    // IAosNConfigurationListener
    void NConfiguration_NotifyConfigChanged() override;

    // IEventListener
    void Event_NotifyEvent(
            IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) override;

    // IAosRegistrationControlListener
    inline void RegistrationControl_UpdateSipDelegateRegistration() override{};
    inline void RegistrationControl_TriggerSipDelegateDeregistration() override{};
    inline void RegistrationControl_TriggerFullNetworkRegistration(
            IN IMS_SINT32 /*nSipCode*/, IN const AString& /*strTarget*/) override{};
    void RegistrationControl_NotifyCapabilitiesChanged(
            IN const ImsMap<IMS_UINT32, IMS_UINT32>& /*objCapabilities*/) override;

    // IAosServiceSettingListener
    void ServiceSetting_RoamingPreferredVoiceNetworkChanged(
            IN RoamingPreferredVoiceNetwork eState) override;

    enum
    {
        STATE_DISCONNECTED = 0,
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_DISCONNECTING,
        STATE_INVALID
    };

    enum
    {
        APP_STATE_DISCONNECTED = 0,
        APP_STATE_CONNECTED,
        APP_STATE_DISCONNECTING
    };

    enum
    {
        BLOCK_NONE = 0,

        // Capabilities
        BLOCK_VOLTE_CAPABILITY = 0x1,
        BLOCK_VILTE_CAPABILITY = 0x2,
        BLOCK_VOWIFI_CAPABILITY = 0x4,
        BLOCK_VIWIFI_CAPABILITY = 0x8,
        BLOCK_CALL_COMPOSER_CAPABILITY = 0x10,
        BLOCK_SMS_CAPABILITY = 0x20,
        BLOCK_TEXT_CAPABILITY = 0x40,

        // Network
        BLOCK_VOPS = 0x80,
        BLOCK_SSAC = 0x100,
        BLOCK_NETWORK = 0x200,
        BLOCK_3G = 0x400,

        // Limited features
        BLOCK_LIMITED_MMTEL = 0x800,
        BLOCK_LIMITED_VIDEO = 0x1000,
        BLOCK_LIMITED_TEXT = 0x2000,
        BLOCK_LIMITED_SMS = 0x4000
    };

protected:
    void Init() override;
    void CleanUp() override;

    void SetHandleState(IN IMS_UINT32 nState);
    void SetReason(IN IMS_UINT32 nReason);
    void ClearSuspendedReason();

    IMS_UINT32 GetAppState();
    IMS_UINT32 GetImsAosReason(IN IMS_UINT32 nAosReason) const;
    IMS_UINT32 GetImsAosReasonForConnecting(IN IMS_UINT32 nAosReason) const;
    IMS_UINT32 GetImsAosReasonForSuspend(IN IMS_UINT32 nAosReason) const;

    IMS_BOOL IsEpdgEnabled() const;
    IMS_BOOL IsEqualNetworkType(IN IMS_UINT32 nType, IN AosNetworkType eType) const;
    IMS_BOOL IsCapabilityExisted(IN IMS_UINT32 nCapabilities, IN AosCapability eCapability) const;
    IMS_BOOL IsCapabilityExistedForNetworkType(
            IN IMS_UINT32 nNetworkType, IN AosCapability eCapability) const;
    IMS_BOOL IsNetworkTypeMatchedToRat(IMS_UINT32 nNetworkType, IMS_UINT32 nRat) const;
    IMS_BOOL IsWifiConnected() const;
    IMS_BOOL IsDataConnected() const;
    IMS_BOOL IsEmergencyService() const;
    IMS_BOOL IsRoaming() const;
    IMS_BOOL IsFeatureUnavailableInLimitedReg(IN IMS_UINT32 nFeature) const;

    IMS_UINT32 GetNetworkType() const;
    IMS_UINT32 GetMobileNetworkType() const;
    IMS_UINT32 GetMobileChangingNetworkType() const;
    IMS_UINT32 GetAosFeature(IN IMS_UINT32 nBlock) const;

    void AddBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks);
    void RemoveBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks);
    void SetBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks, IN IMS_BOOL bAdd);

    IMS_BOOL PreProcessBlock(IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded);
    void ProcessBlock(IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded, IN IMS_BOOL bPreProcess = IMS_TRUE);
    IMS_BOOL ProcessCheckBlock(IN IMS_BOOL bRunStateMachine = IMS_TRUE);
    void ProcessUnavailableFeature(IN IMS_UINT32 nFeature, IN IMS_BOOL bAdd);
    void ProcessFeatureChangedWithoutReg();

    void BackupAllBlocks();
    void BackupBlocks(
            IN ImsList<IMS_UINT32>& objHoldingBlocksPolicy, IN_OUT IMS_UINT32& nHoldingBlocks);
    void RestoreBlocks(
            IN ImsList<IMS_UINT32>& objHoldingBlocksPolicy, IN_OUT IMS_UINT32& nHoldingBlocks);
    IMS_BOOL HoldBlockForInvalidNetwork(IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded);
    void ReevaluateBlocks();
    IMS_BOOL UpdateIpcan();
    void UpdateRegToNextPcscfRequested();

    void NotifyEmergencyInitiated();
    void NotifyEmergencyInitiationDone();

    IMS_BOOL IsHandleBlocked(IN IMS_UINT32 nType) const;
    IMS_BOOL IsHandleBlocked(IN const IMS_UINT32& nBlocks, IN IMS_UINT32 nType) const;

    virtual IMS_BOOL IsHandleBlocked() const;
    virtual IMS_BOOL IsFeatureBlocked(IN IMS_UINT32 nFeature) const;
    virtual void ProcessBlockChanged();

    virtual IMS_BOOL IsBlockForMobile(IN IMS_UINT32 nBlock) const;
    virtual IMS_BOOL IsBlockForWifi(IN IMS_UINT32 nBlock) const;

    virtual void ReevaluateCapabilities();
    virtual void ReevaluateUnavailableFeature();

    virtual void ProcessFeatureBlock(IN IMS_UINT32 nFeature, IN IMS_BOOL bBlocked);
    virtual void ProcessCapabilitiesChanged(
            IN const ImsMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities);
    virtual void ProcessDataConnectionChanged();
    virtual void ProcessNetworkChanged();
    virtual void ProcessPsRoamingStateChanged(IN IMS_UINT32 nState);
    virtual void ProcessNetworkEvent(
            IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_UINT32 nExtraInfo);

    virtual void AddListeners();
    virtual void RemoveListeners();

    // State Machine
    virtual IMS_BOOL StateDisconnected(IN IMSMSG& objMSG);
    virtual IMS_BOOL StateConnecting(IN IMSMSG& objMSG);
    virtual IMS_BOOL StateConnected(IN IMSMSG& objMSG);
    virtual IMS_BOOL StateDisconnecting(IN IMSMSG& objMSG);

    virtual IMS_BOOL IsBlocked() const;
    virtual IMS_BOOL IsSupportedNetworkType(IN IMS_UINT32 nType) const;
    virtual IMS_BOOL IsSupportedNetworkTypeForCellular(IN IMS_UINT32 nType) const;

    virtual void InitializeHoldingBlocksPolicy();
    virtual void InitializeServiceBlock();
    virtual void InitializeServiceFeature();
    virtual void InitializeFeatureTags();

    virtual IMS_BOOL ProcessImsSuspended(IN IMS_UINT32 nReason = 0);
    virtual IMS_BOOL ProcessImsResumed(IN IMS_UINT32 nReason = 0);

    virtual void CheckSuspended();
    virtual void SetSuspendedReason(IN IMS_UINT32 nReason);
    virtual void ResetSuspendedReason(IN IMS_UINT32 nReason);

    virtual void ReportRegState();
    virtual IMS_BOOL Is3G(IN IMS_UINT32 nNetworkType) const;

    static const IMS_CHAR* StateToString(IN IMS_UINT32 nState);
    static const IMS_CHAR* MsgToString(IN IMS_UINT32 nMsg);
    static const IMS_CHAR* RadioTypeToString(IN IMS_UINT32 nType);
    static AString BlocksToString(IN IMS_UINT32 nBlocks);
    const IMS_CHAR* ServiceTypeToString();

    enum
    {
        HANDLE_MSG_BLOCK_STATUS = 0,
        HANDLE_MSG_APP_STATUS,
        HANDLE_MSG_INVALID
    };

protected:
    IAosAppContext* m_piAppContext;

    IMS_SINT32 m_nSlotId;
    AString m_strAppId;
    AString m_strServiceId;

    // ImsAosService::XXX (ImsAosParameter.h)
    IMS_UINT32 m_nServiceType;

    // TO-BE
    AosFeatureTagList m_objFeatureTagList;
    // AS-IS (the extra feature is binded in registration contact currently)
    AosFeatureTagList m_objBindedFeatureTagList;

    // m_nReqType indicates whether the service will be added in registration or not
    IMS_UINT32 m_nReqType;
    // m_bBind indicates whether the service is added in registration or not
    IMS_BOOL m_bBind;
    // m_bRegFeatureTagRequired indicates whether the feature tag is required or not
    IMS_BOOL m_bRegFeatureTagRequired;

    IMS_BOOL m_bNotify;

    // IImsAos Listeners
    IImsAosListener* m_piListener;
    IImsAosMonitor* m_piMonitor;
    IImsAosInfo* m_piInfo;

    IWifiWatcher* m_piWifiWatcher;

    IMS_SINT32 m_nReason;
    IMS_SINT32 m_nSuspendedReason;

    IMS_BOOL m_bBlocked;

    IMS_UINT32 m_nBlocks;
    IMS_UINT32 m_nHoldingBlocksForMobile;
    IMS_UINT32 m_nHoldingBlocksForWifi;
    IMS_UINT32 m_nRoamingState;
    IMS_BOOL m_bCsVoiceAvailable;

    ImsMap<IMS_UINT32, IMS_UINT32> m_objCapabilities;
    ImsList<IMS_UINT32> m_objHoldingBlocksPolicyForMobile;
    ImsList<IMS_UINT32> m_objHoldingBlocksPolicyForWifi;

    IMS_BOOL m_bEpdgEnabled;
    IMS_BOOL m_bDataConnected;

    IMS_BOOL m_bNetSrvIn;
    IMS_UINT32 m_nNetworkType;

    IMS_BOOL m_bEmergencyInitiated;
    IMS_BOOL m_bRegToNextPcscfRequested;

    IMS_UINT32 m_nAppState;

    AString m_strTag;
    AString m_strTagWithServiceType;
};
#endif  // AOS_HANDLE_H_
