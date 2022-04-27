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
#ifndef AOS_SUBSCRIBER_MANAGERH_
#define AOS_SUBSCRIBER_MANAGERH_

#include "ServiceTimer.h"
#include "ISubscriberConfigListener.h"
#include "IConfigUpdateListener.h"
#include "interface/IAosServicePhoneListener.h"
#include "interface/IAosSubscriber.h"
#include "interface/IAosSubscriberManager.h"
#include "interface/IAosSubscriberManagerListener.h"

class AosStaticProfile;

class AosSubscriberManager
    : public IAosSubscriberManager
    , public ISubscriberConfigListener
    , public IConfigUpdateListener
    , public ITimerListener
    , public AosServicePhoneListener
{
public:
    AosSubscriberManager(IN IMS_SINT32 nSlotId);
    virtual ~AosSubscriberManager();

public:
    IMS_BOOL IsReady(IN IMS_BOOL bIsFake = IMS_FALSE) const override;

    // IAosSubscriberManager
    void AddListener(IN IAosSubscriberManagerListener* piListener) override;
    void RemoveListener(IN IAosSubscriberManagerListener* piListener) override;
    void AddListenerForMonitor(IN IAosSubscriberManagerListener* piListener) override;
    void RemoveListenerForMonitor(IN IAosSubscriberManagerListener* piListener) override;

    const AStringArray& GetConfiguredImpus(IN IMS_BOOL bIsFake = IMS_FALSE) const override;
    const AStringArray& GetFakeImpus() const override;

    const ISubscriberConfig* GetSubscriberConfig(
            IN IMS_SINT32 nType = IAosSubscriber::NORMAL) const override;

private:
    /// Priority of Ims Identities
    enum class Index
    {
        FIRST = 0,
        SECOND = 1
    };

    /// Timer
    enum
    {
        TIMER_ICC_LOADED_WAITING = 100,
        TIMER_ISIM_RECOVERY,
        TIMER_PHONE_RESTART_RECOVERY
    };

private:
    void Init();
    void Restart();
    void CleanUp();

    void SetIsim(IN IMS_BOOL bOn);
    void SetUsim(IN IMS_BOOL bOn);
    void SetProvisioned(IN IMS_BOOL bProvision, IN IMS_UINT32 nType = IAosSubscriber::NORMAL);

    void ClearAll();

    IMS_BOOL IsIsim() const;
    IMS_BOOL IsUsim() const;
    IMS_BOOL IsProvisioned(IN IMS_BOOL bIsFake = IMS_FALSE) const;
    IMS_BOOL IsRefreshStarted() const;
    IMS_BOOL IsIsimRecoveryAllowed() const;
    IMS_BOOL IsTimerRunning(IN IMS_UINT32 nType) const;
    IMS_BOOL IsSupportFallback(IN IMS_UINT32 nIdentity) const;

    IMS_BOOL GetImpuFromNormalRegistration(OUT AStringArray& objImpus) const;
    IMS_UINT32 GetIsimAt() const;

    void ClearIsimRecovery();

    void ConfigureAsDefault();
    void ConfigureAsFake();

    IMS_BOOL CheckIsimValues();

    IMS_BOOL GetImpuFromIsim(OUT AStringArray& objImpus) const;
    IMS_BOOL GetTemporaryImpu(OUT AStringArray& objImpus, IN IMS_BOOL bDbWritable);

    void RemoveImpu() const;
    IMS_BOOL UpdateImsi() const;
    IMS_BOOL UpdateImsIdentity(IN IMS_UINT32 nIdentity) ;
    IMS_UINT32 GetIdentity(IN Index eIndex) const;

    IMS_BOOL ProcessFallback(IN IMS_BOOL bToUsim);
    IMS_BOOL ProcessFallbackToImsiBasedIsim(IN IMS_SINT32 nCpi);
    void ProcessPhoneNumberAvailable(IN IMS_BOOL bIsRefresh, IN PhoneNumberState eState);
    void ProcessIsimStateChange(IN IsimState eState);
    void ProcessIsimRecovery();
    void ProcessPhoneRestarted();
    void ProcessIccLoadedWaitingTimerExpired();
    void ProcessIsimRecoveryTimerExpired();
    void ProcessPhoneRestartRecoveryTimerExpired();

    void SetConfigUpdateListener();
    void RemoveConfigUpdateListener();

    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    void StopTimer(IN IMS_UINT32 nType);
    void ClearTimers();

    void NotifyState(IN IMS_UINT32 nState) const;
    void NotifyMonitorState(IN IMS_UINT32 nState) const;

    IMS_BOOL IsPrimaryImpuValid(IN const AStringArray& objImpus) const;
    IMS_BOOL IsSipUri(IN const AString& strImpu) const;

    // ISubscriberConfigListener
    void SubscriberConfig_InitCompleted() override;
    void SubscriberConfig_RefreshCompleted() override;
    void SubscriberConfig_RefreshStarted() override;
    void SubscriberConfig_NotifyError(IN IMS_SINT32 nErrorCode) override;

    // IConfigUpdateListener
    void ConfigUpdate_NotifyUpdate(IN IMS_SINT32 nCpi,
            IN const AString& strConfName = AString::ConstNull(),
            IN const AString& strExtraParam = AString::ConstNull()) override;

    // ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    // AosServicePhoneListener
    void ServicePhone_PhoneNumberStateChanged(IN IMS_BOOL bIsRefresh,
            IN PhoneNumberState eState) override;
    void ServicePhone_IsimStateChanged(IN IsimState eState) override;

    // Log
    static const IMS_CHAR* UpdateEventToString(IN IMS_UINT32 nEvent);
    static const IMS_CHAR* TimerToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* TypeToString(IN AosRegistrationType eType);
    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

private:
    IMS_SINT32 m_nSlotId;

    IMSList<IAosSubscriberManagerListener*> m_objListeners;
    IMSList<IAosSubscriberManagerListener*> m_objMonitorListeners;

    IMS_BOOL m_bIsim;
    IMS_BOOL m_bUsim;
    IMS_BOOL m_bUsimFallback;
    IMS_BOOL m_bIsRefreshStarted;
    IMS_UINT32 m_nIsimRecoveryCount;

    ITimer* m_piTimerToIccLoadedWaiting;
    ITimer* m_piTimerToIsimRecovery;
    ITimer* m_piTimerToPhoneRestartRecovery;

    IMS_BOOL m_bIsProvisioned;
    IMS_BOOL m_bIsProvisionedForFake;

    AStringArray m_objPuids;
    AStringArray m_objPuidsForFake;

    static const IMS_UINT32 ISIM_RECOVERY_MAX_COUNT = 2;
    static const IMS_UINT32 ISIM_RECOVERY_DEFAULT_INTERVAL = 2;
    static const IMS_UINT32 PHONE_RESTART_RECOVERY_INTERVAL = 15000;
    static const IMS_SINT32 USIM_MSISDN_LENGTH = 10;

    AString m_strTag;
};

#endif // AOS_SUBSCRIBER_MANAGERH_