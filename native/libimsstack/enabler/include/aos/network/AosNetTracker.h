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
#ifndef AOS_NET_TRACKER_H_
#define AOS_NET_TRACKER_H_

#include "AString.h"
#include "ITimer.h"
#include "INetworkWatcher.h"
#include "IWifiWatcher.h"
#include "interface/IAosConnectionListener.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNConfigurationListener.h"
#include "interface/IAosNetTracker.h"

class IAosAppContext;
class IAosConnection;
class AosUtil;

class AosNetTracker :
        public IAosNetTracker,
        public IAosConnectionListener,
        public INetworkWatcherListener,
        public IWifiWatcherListener,
        public IAosNConfigurationListener,
        public IEventListener,
        public ITimerListener
{
public:
    explicit AosNetTracker(IN IAosAppContext* piAppContext);
    ~AosNetTracker() override;

public:
    // IAosNetTracker
    IMS_BOOL IsServiceIn(IN IMS_UINT32 nType = TYPE_DEFAULT) override;
    IMS_BOOL IsDataIn() override;
    IMS_BOOL IsNetworkIn() override;
    IMS_BOOL IsEmergencyAttach() override;
    IMS_BOOL IsSuspended() override;
    IMS_BOOL IsSessionContinuitySupported() override;
    IMS_BOOL IsServiceTimerRunning() override;
    IMS_BOOL IsImsVoiceCallSupported() override;
    IMS_BOOL IsRoaming() override;

    IMS_UINT32 GetMobileChangingNetworkType() override;
    IMS_UINT32 GetMobileNetworkType() override;
    IMS_UINT32 GetMobileNetworkRegistrationRejectCause() override;
    IMS_SINT32 GetMobileVoiceServiceState() override;
    IMS_UINT32 GetMobileVoiceNetworkType() override;
    IMS_UINT32 GetNetworkType() override;

    void SetRatGuardTime(IN IMS_UINT32 nGuardTime) override;
    void SetSrvOutGuardTime(IN IMS_UINT32 nGuardTime) override;
    void SetSrvInGuardTime(IN IMS_UINT32 nGuardTime) override;

    void SetListener(IN IAosNetTrackerListener* piListener) override;
    void RemoveListener(IN IAosNetTrackerListener* piListener) override;

    // INetworkWatcherListener
    void NetworkWatcher_NotifyStatus(IN INetworkWatcher* piNetWatcherInfo) override;

    // IWifiWatcherListener
    void WifiWatcher_NotifyStateChanged(IN IWifiWatcher* piWifiWatcher) override;

    // IAosNConfigurationListener
    void NConfiguration_NotifyConfigChanged() override;

    // IEventListener
    void Event_NotifyEvent(
            IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam) override;

    enum
    {
        FEATURE_NONE = (0x0),
        FEATURE_IN_GUARD = (0x01 << 0),
        FEATURE_OUT_GUARD = (0x01 << 1),
        FEATURE_RAT_GUARD = (0x01 << 2),
        FEATURE_VOICE_RAT_GUARD = (0x01 << 3)
    };

    enum
    {
        TIMER_IN_GUARD = 100,
        TIMER_OUT_GUARD,
        TIMER_RAT_GUARD,
        TIMER_VOICE_RAT_GUARD
    };

    enum
    {
        REASON_NONE = 0,
        REASON_NET_STATE_CHANGED,
        REASON_ROAMING_SATAE_CHANGED
    };

    static const IMS_UINT32 SERVICE_IN_TIME_MILLI_SEC = 2000;
    static const IMS_UINT32 SERVICE_OUT_TIME_MILLI_SEC = 1000;

protected:
    void Init() override;
    void InitConfig();
    void InitCnxPolicy(IN ImsVector<IMS_SINT32>& objRats);
    void InitRoamingCnxPolicy(IN ImsVector<IMS_SINT32>& objRoamingRats);
    void InitObject();

    IMS_BOOL UpdateNetworkStatus();
    void Notify();

    void GetStatus(OUT IMS_SINT32& nService, OUT IMS_UINT32& nRadioTech, OUT IMS_BOOL& bIsIn);
    IMS_UINT32 GetAccessPolicy() const;

    void ProcessNetworkChanged(IMS_SINT32 nReason);
    void ProcessVoiceNetworkChanged();

    IMS_BOOL IsRadioTechAvailable(IN IMS_UINT32 nPolicy, IN IMS_UINT32 nRadioTech);
    IMS_BOOL IsServiceAvailable(IN IMS_UINT32 nPolicy, IN IMS_UINT32 nService);
    IMS_BOOL IsNetworkChanged(IN IMS_UINT32 nCurrRat, IN IMS_SINT32 nCurrService);

    IMS_BOOL IsCnxTypeEqual(IN IMS_SINT32 nType) const;
    IMS_BOOL IsDataConnected() const;
    IMS_BOOL IsEpdgEnabled() const;
    IMS_BOOL IsWlanEnabled() const;
    IMS_BOOL IsWifiConnected() const;
    IMS_BOOL IsVonrSupported();
    IMS_BOOL IsRoamingAccessPolicyRequired() const;

    void SetDataConnected(IN IMS_BOOL bConnected);
    void SetEpdgEnabled(IN IMS_BOOL bEnabled);
    void SetWifiConnected(IN IMS_BOOL bConnected);
    void UpdateWifiObserver();

    // Timer
    void ProcessInTimerExpired();
    void ProcessOutTimerExpired();
    void ProcessRatTimerExpired();
    void ProcessVoiceRatTimerExpired();

    void ClearTimers();
    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    void StopTimer(IN IMS_UINT32 nType);

    // IAosConnectionListener
    void AosConnection_StateChanged(IN IMS_UINT32 nState) override;
    void AosConnection_IpChanged() override;
    void AosConnection_IpcanCatChanged() override;
    void AosConnection_PcscfChanged() override;
    void AosConnection_ConnectionFailed() override;

    // ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    // LOG
    AString FeaturesToString();

    static const IMS_CHAR* RadioTypeToString(IN IMS_UINT32 nState);
    static const IMS_CHAR* ServiceTypeToString(IN IMS_UINT32 nState);
    static const IMS_CHAR* TimerToString(IN IMS_UINT32 nType);

protected:
    // access_policy (ex 0x20000004)
    IMS_UINT32 m_nCnxPolicy;
    IMS_UINT32 m_nCnxPolicyInRoaming;

    INetworkWatcher* m_piNetWatcherInfo;
    IWifiWatcher* m_piWifiWatcher;
    IAosConnection* m_piConnection;
    AosUtil* m_pUtil;
    IAosNConfiguration* m_piAosNConfig;

    IMS_SINT32 m_nSlotId;
    IMS_SINT32 m_nNetServiceType;
    IMS_UINT32 m_nNetRadioType;
    IMS_UINT32 m_nChangingRat;

    IMS_UINT32 m_nNetVoiceRadioType;
    IMS_UINT32 m_nNetChangingVoiceRadioType;

    // IN or OUT (data service & radio tech)
    IMS_BOOL m_bIsNetAvailable;

    // roaming network
    IMS_BOOL m_bIsRoaming;

    // ePDG network (IPCAN_WLAN)
    IMS_BOOL m_bIsEpdgEnabled;

    // wifi network
    IMS_BOOL m_bIsWifiConnected;

    // mobile network
    IMS_BOOL m_bIsDataConnected;

    // Guard Time Feature
    IMS_UINT32 m_nFeature;
    IMS_UINT32 m_nServiceInTime;
    IMS_UINT32 m_nServiceOutTime;
    IMS_UINT32 m_nRatTime;
    IMS_UINT32 m_nVoiceRatGuardTime;

    ITimer* m_piServiceInTimer;
    ITimer* m_piServiceOutTimer;
    ITimer* m_piRatTimer;
    ITimer* m_piVoiceRatTimer;

    ImsList<IAosNetTrackerListener*> m_objListeners;

    AString m_strTag;
};

#endif  // AOS_NET_TRACKER_H_
