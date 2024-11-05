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
#ifndef AOS_CONDITION_H_
#define AOS_CONDITION_H_

#include "IEventListener.h"
#include "ITimer.h"
#include "interface/IAosBlock.h"
#include "interface/IAosBlockListener.h"
#include "interface/IAosBlockSilentListener.h"
#include "interface/IAosCallTrackerListener.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNConfigurationListener.h"
#include "interface/IAosNetTrackerListener.h"
#include "interface/IAosServiceAvailableListener.h"
#include "interface/IAosServicePhoneListener.h"
#include "interface/IAosServiceSettingListener.h"
#include "interface/IAosSubscriberListener.h"
#include "condition/AosServiceAvailable.h"

class IAosConditionListener;
class AosServiceAvailableCellular;
class AosServiceAvailableWifi;

class AosCondition :
        public IEventListener,
        public IAosBlockListener,
        public IAosBlockSilentListener,
        public IAosCallTrackerListener,
        public IAosNetTrackerListener,
        public IAosServiceAvailableListener,
        public IAosSubscriberListener,
        public IAosNConfigurationListener,
        public AosServicePhoneListener,
        public AosServiceSettingListener
{
public:
    explicit AosCondition(IN IAosAppContext* piAppContext);
    inline explicit AosCondition(){};
    ~AosCondition() override;

    virtual void Start();
    virtual void Stop();
    virtual void SetListener(IN IAosConditionListener* piListener);
    virtual void SetBlock(IN BLOCK_REASON eReason, IN IMS_BOOL bNotify = IMS_TRUE);
    virtual void ResetBlock(IN BLOCK_REASON eReason, IN IMS_BOOL bNotify = IMS_TRUE);
    virtual IMS_BOOL IsReasonBlocked(IN BLOCK_REASON eReason) const;
    virtual IMS_BOOL IsReady();

    virtual IMS_UINT32 CheckServiceAvailable(IN SERVICE_TYPE eType = SERVICE_WHOLE);

    // TODO : Need to check
    virtual IMS_BOOL CheckBadNetwork(IN SERVICE_TYPE eType);

    virtual void PrintBlockReasons() const;

    // Condition_RequestCommand(nCommand, eReason)
    // nCommand
    enum
    {
        REQUEST_NONE = 0,
        REQUEST_STOP,
        REQUEST_DESTROY,
        REQUEST_RECOVER,
        REQUEST_PDN_DISCONNECT,
        REQUEST_RESET_CONNECTION_RECOVERY
    };
    // eReason : AosReason

    enum
    {
        LISTENER_NONE = 0x00,
        LISTENER_BLOCK = 0x01,
        LISTENER_NETTRACKER = 0x02,
        LISTENER_SUBSCRIBER = 0x04,
        LISTENER_CALLTRACKER = 0x08,
        LISTENER_ALL = 0xFF
    };

    enum
    {
        CHECK_NONE = 0x0,
        CHECK_CELLULAR = 0x1,
        CHECK_WIFI = 0x2
    };

    enum
    {
        HOLD_EVENT_NONE = 0x00,
        HOLD_EVENT_ROAMING = 0x01,
        HOLD_EVENT_IMS_SERVICE = 0x02
    };

protected:
    virtual void AddServiceAvailable();
    virtual void RemoveServiceAvailable();

    virtual void AddAosServiceListener();
    virtual void RemoveAosServiceListener();

    virtual void AddEventListener();
    virtual void RemoveEventListener();

    // IEventListener Interface
    void Event_NotifyEvent(
            IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) override;

    // IAosCallTrackerListener
    void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState) override;

    // IAosNetTrackerListener
    void NetTracker_StatusChanged() override;

    // IAosSubscriberListener
    void Subscriber_StateChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nParam = 0) override;

    // IAosBlockListener
    void Block_Changed(IN IMS_UINT32 nType, IN IMS_UINT32 nParam) override;

    // IAosBlockSilentListener
    void Block_SilentChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nParam) override;

    // IAosServiceAvailableListener
    void ServiceAvailable_Changed(IN IMS_BOOL bNotify = IMS_TRUE) override;
    void ServiceAvailable_RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason) override;

    // IAosNConfigurationListener
    void NConfiguration_NotifyConfigChanged() override;
    // AosServicePhoneListener
    void ServicePhone_AosStart() override;
    void ServicePhone_LocationInfoChanged(IN LocationInfo eState) override;
    void ServicePhone_PhoneNumberStateChanged(
            IN IMS_BOOL bIsRefresh, IN PhoneNumberState eState) override;
    void ServicePhone_PlmnChanged() override;
    void ServicePhone_PowerOff() override;

    // AosServiceSettingListener
    void ServiceSetting_AirplaneChanged(IN IMS_BOOL bIsOn) override;
    void ServiceSetting_ServiceChanged(
            IN ServiceSetting eState, IN IMS_UINT32 nServiceBits) override;
    void ServiceSetting_TtyChanged(IN IMS_BOOL bIsOn) override;

    void Init();
    void AddListener(IN IMS_UINT32 nType);
    void RemoveListener(IN IMS_UINT32 nType);
    IMS_BOOL IsListenerEnabled(IN IMS_UINT32 nType) const;

    void AddHold(IN IMS_UINT32 nEvent, IN IMS_BOOL bIsEventReset = IMS_FALSE);
    void RemoveHold(IN IMS_UINT32 nEvent, IN IMS_BOOL bIsEventReset = IMS_FALSE);
    IMS_BOOL IsHeld(IN IMS_UINT32 nEvent) const;
    IMS_BOOL IsRefreshStarted() const;

    void SetInitialBlockReason();
    void SetStartBlockReason();

    void ProcessBlockReason(
            IN IMS_BOOL bIsBlockSet, IN BLOCK_REASON eReason, IN IMS_BOOL bNotify = IMS_TRUE);
    void ProcessAosStartEvent();
    void ProcessAirPlaneEvent(IN IMS_BOOL bIsOn);
    void ProcessPowerEvent();
    void ProcessRoamingEvent(IN IMS_UINT32 nPsState, IN IMS_UINT32 nCsState);
    void ProcessPlmnEvent();
    void ProcessPhoneNumberAvailableEvent(IN IMS_BOOL bIsRefresh, IN PhoneNumberState eState);
    void ProcessImsServiceEvent(IN ServiceSetting eState, IN IMS_UINT32 nServiceBits);
    void ProcessTtyEvent(IN IMS_BOOL bIsOn);
    void ProcessImsVopsEvent(IN IMS_UINT32 nState);
    void ProcessLocationInfo(IN LocationInfo eState);
    void ProcessLteInfoEvent(IN IMS_UINT32 nState, IN IMS_UINT32 nStateEx);

    void ClearRegistrationAndDataFailureBlocks();

    SERVICE_TYPE GetServiceType();
    void SendConditionEvent(IN IMS_UINT32 eEvent, IN IMS_UINT32 nState, IN IMS_SINT32 nStateEx = -1,
            IN SERVICE_TYPE eServiceType = SERVICE_WHOLE);

    IMS_BOOL RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason = 0) const;

    void UpdateRegistrationMode() const;
    IMS_BOOL IsServiceBlockedByMenu() const;
    IMS_BOOL IsRttSupported() const;
    IMS_BOOL IsCombinedAttached() const;
    IMS_BOOL IsDeregRequiredForTty() const;

protected:
    IAosAppContext* m_piAppContext;
    IMS_SINT32 m_nSlotId;
    IAosConditionListener* m_piListener;
    AosServiceAvailableCellular* m_pAvailableCellular;
    AosServiceAvailableWifi* m_pAvailableWifi;
    IAosBlock* m_piBlock;
    SERVICE_TYPE m_eServiceType;
    IMS_BOOL m_bIsRefreshStarted;
    IMS_BOOL m_bIsCombinedAttached;
    IMS_BOOL m_bCellServiceAvailable;
    IMS_BOOL m_bWifiServiceAvailable;
    IMS_BOOL m_bIsTtyOn;
    IMS_UINT32 m_nHoldEvents;
    IMS_UINT32 m_nListeners;

    AString m_strTag;
    static const IMS_UINT32 RAT_CHANGE_GUARD_TIME_MILLIS = 2000;
};

#endif  // AOS_CONDITION_H_
