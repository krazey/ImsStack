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

#include "interface/IAosHandle.h"
#include "IImsAos.h"
#include "IImsAosInfo.h"
#include "IMSStateMachine.h"
#include "IEventListener.h"
#include "interface/IAosCallTrackerListener.h"
#include "interface/IAosNetTrackerListener.h"
#include "interface/IAosRegistrationControlListener.h"
#include "interface/IAosServiceSettingListener.h"
#include "interface/IAosService.h"

#include "handle/AosFeatureTag.h"

class IAosAppContext;
class AosServicePhoneListener;
class AosServiceSettingListener;

class AosHandle
    : public IAosHandle
    , public IAosCallTrackerListener
    , public IAosNetTrackerListener
    , public IEventListener
    , public IImsAos
    , public IMSStateMachine
    , public AosRegistrationControlListener
    , public AosServiceSettingListener
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
    AosHandle
    (
        IN IAosAppContext* piAppContext,
        IN const AString& strAppId,
        IN const AString& strServiceId,
        IN const IMS_UINT32 nServiceType
    );
    virtual ~AosHandle();

    // IAosHandle
    virtual AString& GetAppId();
    virtual AString& GetServiceId();
    virtual IMS_UINT32 GetServiceType();

    virtual IImsAosMonitor* GetMonitor();

    virtual IMS_SINT32 GetRequestType();
    virtual void SetRequestType(IN IMS_SINT32 nReqType);

    virtual IMS_BOOL IsRegBinded();
    virtual void SetRegBinded(IN IMS_BOOL bBind);

    virtual IMS_BOOL IsNetworkRegBinded();
    virtual void SetNetworkRegBinded(IN IMS_BOOL bNetworkBind);

    virtual AosFeatureTagList& GetFeatureTagList();
    virtual AosFeatureTagList& GetBindedFeatureTagList();

    virtual void ProcessFeatureTagChange();

    virtual void Request(IN IMS_UINT32 nType, IN IMS_UINT32 nState = 0);

    virtual void App_StateChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nParam);
    virtual void App_Notify();

    virtual void Handle_Notify(IN IMS_UINT32 nType, IN IMS_BOOL bBlocked);

    // IImsAos
    virtual IMS_BOOL Control(IN IMS_UINT32 nType);
    virtual IImsAosInfo* GetAosInfo();
    virtual IMS_UINT32 GetFeatures();
    virtual IMS_UINT32 GetSuspendedReason();
    virtual IMS_BOOL IsFeatureConnected(IN IMS_UINT32 nFeature);
    virtual IMS_BOOL IsImsConnected();
    virtual IMS_BOOL IsImsSuspended();
    virtual void SetListener(IN IImsAosListener* piListener);
    virtual void SetMonitor(IN IImsAosMonitor* piMonitor);
    virtual void UpdateFeature(IN IMS_UINT32 nFeatures);
    virtual void UpdateFeature(IN IMSList<ImsAosFeatureTag*>& objFeatureTag);

    // IAosCallTrackerListener
    virtual void CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

    // IAosNetTrackerListener
    virtual void NetTracker_StatusChanged();

    // IEventListener
    virtual void Event_NotifyEvent(IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam,
            IN IMS_UINT32 nLParam);

    // IAosRegistrationControlListener
    inline virtual void RegistrationControl_UpdateSipDelegateRegistration(){};
    inline virtual void RegistrationControl_TriggerSipDelegateDeregistration(){};
    inline virtual void RegistrationControl_TriggerFullNetworkRegistration(
            IN IMS_SINT32 /*nSipCode*/, IN const AString& /*strTarget*/){};
    virtual void RegistrationControl_NotifyCapabilitiesChanged(
            IN const IMSMap<IMS_UINT32, IMS_UINT32>& /*objCapabilities*/);

    // IAosServiceSettingListener
    virtual void ServiceSetting_RoamingPreferredVoiceNetworkChanged(
            IN RoamingPreferredVoiceNetwork eState);

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

protected:
    virtual void Init();
    virtual void CleanUp();

    void SetHandleState(IN IMS_UINT32 nState);
    void SetReason(IN IMS_UINT32 nReason);
    void ClearSuspendedReason();

    IMS_BOOL CheckAppNotificationAndSetAppState();

    IMS_UINT32 GetAppState();
    IMS_UINT32 GetImsAosReason(IN IMS_UINT32 nAosReason);
    IMS_UINT32 GetImsAosReasonForSuspend(IN IMS_UINT32 nAosReason);

    IMS_BOOL IsEpdgEnabled() const;
    IMS_BOOL IsEqualNetworkType(IN IMS_UINT32 nType, IN AosNetworkType eType) const;
    IMS_BOOL IsCapabilityExisted(IN IMS_UINT32 nCapabilities, IN AosCapability eCapability) const;
    IMS_BOOL IsNetworkTypeMatchedToRat(IMS_UINT32 nNetworkType, IMS_UINT32 nRat);
    IMS_UINT32 GetNetworkType() const;
    IMS_UINT32 GetMobileNetworkType() const;

    IMS_UINT32 GetBlock(IN IMS_UINT32 nEvent);
    IMS_UINT32 GetAosReason(IN IMS_UINT32 nFeature);
    IMS_UINT32 GetAosFeature(IN IMS_UINT32 nBlock);

    void AddBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks);
    void RemoveBlock(IN IMS_UINT32 nBlock, IN_OUT IMS_UINT32& nBlocks);

    IMS_BOOL PreProcessBlock(IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded);
    void ProcessBlock(IN IMS_UINT32 nBlock, IN IMS_BOOL bAdded, IN IMS_BOOL bPreProcess = IMS_TRUE);
    void ProcessFeatureBlock(IN IMS_UINT32 nFeature, IN IMS_BOOL bBlocked);
    void ProcessCheckBlock(IN IMS_UINT32 nBlock = 0, IN IMS_BOOL bRunStateMachine = IMS_TRUE);

    IMS_BOOL IsHandleBlocked(IN IMS_UINT32 nType) const;
    virtual IMS_BOOL IsHandleBlocked() const;
    virtual void ProcessBlockChanged();

    virtual IMS_BOOL IsBlockForMobile(IN IMS_UINT32 nBlock) const;
    virtual IMS_BOOL IsBlockForWifi(IN IMS_UINT32 nBlock) const;

    virtual void ProcessCapabilitiesChanged(
            IN const IMSMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities);
    virtual void ProcessNetworkChanged();
    virtual void ProcessVopsStateChanged(IN IMS_UINT32 nState);

    // State Machine
    virtual IMS_BOOL StateDisconnected(IN IMSMSG& objMSG);
    virtual IMS_BOOL StateConnecting(IN IMSMSG& objMSG);
    virtual IMS_BOOL StateConnected(IN IMSMSG& objMSG);
    virtual IMS_BOOL StateDisconnecting(IN IMSMSG& objMSG);

    virtual IMS_BOOL IsBlocked() const;
    virtual IMS_BOOL IsSupportedNetworkType(IN IMS_UINT32 nType) const;
    virtual IMS_BOOL IsSupportedNetworkTypeForCellular(IN IMS_UINT32 nType) const;

    virtual void InitializeServiceBlock();
    virtual void InitializeServiceFeature();
    virtual void InitializeFeatureTags();

    virtual void UpdateFeatureTags();

    virtual void ProcessImsSuspended(IN IMS_UINT32 nReason = 0);
    virtual void ProcessImsResumed(IN IMS_UINT32 nReason = 0);

    virtual void CheckSuspended();
    virtual void SetSuspendedReason(IN IMS_UINT32 nReason);
    virtual void ResetSuspendedReason(IN IMS_UINT32 nReason);

    virtual void ReportRegState();

    static const IMS_CHAR* StateToString(IN IMS_UINT32 nState);
    static const IMS_CHAR* MsgToString(IN IMS_UINT32 nMsg);
    static const IMS_CHAR* RadioTypeToString(IN IMS_UINT32 nType);
    const IMS_CHAR* ServiceTypeToString();

    enum
    {
        HANDLE_MSG_BLOCK_STATUS = 0,
        HANDLE_MSG_APP_STATUS,
        HANDLE_MSG_INVALID
    };

    enum
    {
        BLOCK_NONE = 0,

        // Capabilities
        BLOCK_VOLTE_CAPABILITY = 0x1,
        BLOCK_VILTE_CAPABILITY = 0x2,
        BLOCK_SMS_CAPABILITY = 0x4,

        // Network
        BLOCK_NETWORK = 0x8,
        BLOCK_VOPS = 0x10,

        // DM
        BLOCK_SMS_OVER_IP_NETWORK_INDICATION = 0x20,

        // Reg
        BLOCK_LIMITED_REGISTRATION = 0x40,

        // VoWiFi
        BLOCK_VOWIFI_CAPABILITY = 0x80,
        BLOCK_VIWIFI_CAPABILITY = 0x100
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
    // m_bNetworkBind indicates whether the service is kept or removed in registration or not
    IMS_BOOL m_bNetworkBind;

    IMS_BOOL m_bNotify;

    // IImsAos Listeners
    IImsAosListener* m_piListener;
    IImsAosMonitor* m_piMonitor;
    IImsAosInfo* m_piInfo;

    IMS_SINT32 m_nReason;
    IMS_SINT32 m_nSuspendedReason;

    IMS_BOOL m_bBlocked;

    IMS_UINT32 m_nBlocks;
    IMS_UINT32 m_nHoldingBlocksForMobile;
    IMS_UINT32 m_nHoldingBlocksForWifi;
    IMS_UINT32 m_nHoldingVopsState;

    IMSMap<IMS_UINT32, IMS_UINT32> m_objCapabilities;

    IMS_BOOL m_bNetSrvIn;
    IMS_UINT32 m_nNetworkType;

    IMS_UINT32 m_nAppState;

    AString m_strTag;
};
#endif // AOS_HANDLE_H_
