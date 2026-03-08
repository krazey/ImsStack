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

#include "IPhoneInfoSubscriber.h"
#include "ServicePhoneInfo.h"
#include "ServiceTimer.h"
#include "ISubscriberConfigListener.h"
#include "IConfigUpdateListener.h"
#include "ImsIdentity.h"
#include "IAosService.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNConfigurationListener.h"
#include "interface/IAosServicePhoneListener.h"
#include "interface/IAosSubscriber.h"
#include "interface/IAosSubscriberManager.h"
#include "interface/IAosSubscriberManagerListener.h"

class AosSubscriberManager :
        public IAosSubscriberManager,
        public IAosNConfigurationListener,
        public ISubscriberConfigListener,
        public IConfigUpdateListener,
        public ITimerListener,
        public AosServicePhoneListener
{
public:
    explicit AosSubscriberManager(IN IMS_SINT32 nSlotId);
    ~AosSubscriberManager() override;

public:
    IMS_BOOL IsReady(IN IMS_BOOL bIsFake = IMS_FALSE) const override;
    IMS_BOOL IsIsim() const override;
    IMS_BOOL IsUsim() const override;

    // IAosSubscriberManager
    void AddListener(IN IAosSubscriberManagerListener* piListener) override;
    void RemoveListener(IN IAosSubscriberManagerListener* piListener) override;
    void AddListenerForMonitor(IN IAosSubscriberManagerListener* piListener) override;
    void RemoveListenerForMonitor(IN IAosSubscriberManagerListener* piListener) override;

    const AStringArray& GetConfiguredImpus() const override;
    const AStringArray& GetOrderedImpus() const override;
    const AStringArray& GetConfiguredImpusForFake() const override;
    const AStringArray& GetFakeImpus() const override;

    const ISubscriberConfig* GetSubscriberConfig(
            IN IMS_SINT32 nType = IAosSubscriber::NORMAL) const override;
    SimState GetSimState() const override;

protected:
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
        TIMER_PHONE_RESTART_RECOVERY
    };

protected:
    void Init();
    void Restart();
    void CleanUp();

    void SetIsim(IN IMS_BOOL bOn);
    void SetUsim(IN IMS_BOOL bOn);
    void SetProvisioned(IN IMS_BOOL bProvision, IN IMS_UINT32 nType = IAosSubscriber::NORMAL);

    void ClearAll();

    IMS_BOOL IsProvisioned(IN IMS_BOOL bIsFake = IMS_FALSE) const;
    IMS_BOOL IsRefreshStarted() const;
    IMS_BOOL IsTimerRunning(IN IMS_UINT32 nType) const;
    IMS_BOOL IsSupportFallback(IN IMS_UINT32 nIdentity) const;
    IMS_UINT32 GetIsimAt() const;

    void ConfigureAsDefault();
    void ConfigureAsFake();

    IMS_BOOL CheckIsimValues();
    IMS_BOOL IsValidImpu(IN const AStringArray& objImpus);

    ISubscriberConfig* GetSubscriberConfiguration(
            IN IMS_SINT32 nType = IAosSubscriber::NORMAL) const;
    IMS_BOOL UpdateImpuFromIsim(OUT AStringArray& objImpus);
    IMS_BOOL UpdateSubscriberInfoWithTempImpu(
            OUT AStringArray& objImpus, IN IMS_BOOL bIsIsim = IMS_FALSE);

    void RemoveImpu() const;
    IMS_BOOL UpdateImsi() const;
    void UpdateImsIdentity(IN IMS_UINT32 nIdentity);
    IMS_UINT32 GetIdentity(IN Index eIndex) const;

    IMS_BOOL ReconfigureFallback(IN IMS_BOOL bToUsim);
    IMS_BOOL ProcessPhoneNumberAvailable();
    IMS_BOOL ProcessIsimStateChange(IN IsimState eState);
    IMS_BOOL ProcessSimStateChange(IN SimState eState);

    void ProcessPhoneRestarted();
    void ProcessIccLoadedWaitingTimerExpired();
    void ProcessPhoneRestartRecoveryTimerExpired();
    void ProcessValidIsimOnCompleted(IN IMS_BOOL bIsRefresh);
    void ProcessInvalidIsimOnCompleted(IN IMS_BOOL bIsRefresh);

    IMS_BOOL CheckAndTryUsimFallback();
    IMS_BOOL CheckAndTryIsimImsiFallback();

    IMS_BOOL UpdateNConfiguration();

    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    void StopTimer(IN IMS_UINT32 nType);
    void ClearTimers();

    void NotifyState(IN IMS_UINT32 nState);
    void NotifyMonitorState(IN IMS_UINT32 nState);
    void NotifyAosIsimState(IN AosIsimState eState);

    IMS_BOOL IsPrimaryImpuValid(IN const AStringArray& objImpus);
    IMS_BOOL IsSipUri(IN const AString& strImpu) const;
    void RequestStop() const;

    virtual inline void GetPhoneNumber(OUT AString& strPhoneNumber)
    {
        PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(m_nSlotId)->GetPhoneNumber(
                strPhoneNumber);
    }

    virtual inline AString GetTemporaryPublicUserId()
    {
        return ImsIdentity::CreateTemporaryPublicUserId(m_nSlotId);
    }

    virtual inline AString GetTemporaryPrivateUserId()
    {
        return ImsIdentity::CreateTemporaryPrivateUserId(m_nSlotId);
    }

    virtual inline AString GetTemporaryHomeDomainName()
    {
        return ImsIdentity::CreateTemporaryHomeDomainName(m_nSlotId);
    }

    // IAosNConfigurationListener
    void NConfiguration_NotifyConfigChanged() override;

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
    void ServicePhone_PhoneNumberStateChanged(
            IN IMS_BOOL bIsRefresh, IN PhoneNumberState eState) override;
    void ServicePhone_IsimStateChanged(IN IsimState eState) override;
    void ServicePhone_SimStateChanged(IN SimState eState) override;

    // Log
    const IMS_CHAR* IdentityPriorityToString();
    static const IMS_CHAR* PrintIdentity(IN IMS_UINT32 nIdentity);
    static const IMS_CHAR* TimerToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

protected:
    IMS_SINT32 m_nSlotId;

    ImsList<IAosSubscriberManagerListener*> m_objListeners;
    ImsList<IAosSubscriberManagerListener*> m_objMonitorListeners;

    ISubscriberConfig* m_piSubscriberConfig;
    ISubscriberConfig* m_piSubscriberConfigFake;

    IMS_BOOL m_bIsim;
    IMS_BOOL m_bUsim;
    IMS_BOOL m_bUsimFallback;
    IMS_BOOL m_bIsRefreshStarted;

    ITimer* m_piTimerToIccLoadedWaiting;
    ITimer* m_piTimerToPhoneRestartRecovery;

    IMS_BOOL m_bIsProvisioned;
    IMS_BOOL m_bIsProvisionedForFake;

    IMS_UINT32 m_nNotifyState;
    IMS_UINT32 m_nNotifyStateForFake;
    AosIsimState m_eNotifyIsimState;

    AStringArray m_objPuids;
    AStringArray m_objOrderedPuids;
    AStringArray m_objPuidsForFake;

    AString m_strPriority;

    IAosNConfiguration* m_piNConfig;

    // Carrier Configuration
    IMS_UINT32 m_nIsimIndexForImpu;
    IMS_BOOL m_bSupportLimitedAdminSmsMode;
    IMS_BOOL m_bPrioritizeImsiBasedUri;
    ImsVector<IMS_SINT32> m_objImsIdentityPriority;
    SimState m_eSimState;

    static const IMS_UINT32 PHONE_RESTART_RECOVERY_INTERVAL = 15000;
    static const IMS_UINT32 DEFAULT_ISIM_INDEX_FOR_IMPU = 1;
    static const IMS_SINT32 USIM_MSISDN_LENGTH = 10;

    AString m_strTag;
};

#endif  // AOS_SUBSCRIBER_MANAGERH_