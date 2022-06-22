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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "IConfigurable.h"
#include "../../../config/interface/common/MockISubscriberConfig.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosServicePhoneListener.h"
#include "interface/IAosSubscriberManagerListener.h"
#include "provider/AosSubscriberManager.h"

#include "interface/MockIAosSubscriberManagerListener.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;
const IMS_UINT32 TIMER_ICC_LOADED_WAITING = 100;
const IMS_UINT32 TIMER_ISIM_RECOVERY = 101;
const IMS_UINT32 TIMER_PHONE_RESTART_RECOVERY = 102;

class AosSubscriberManagerTest : public ::testing::Test
{
public:
    AosSubscriberManager* m_pAosSubscriberManager;
    MockISubscriberConfig m_objMockISubscriberConfig;
    AStringArray m_objPuids;

protected:
    virtual void SetUp() override
    {
        m_pAosSubscriberManager = new AosSubscriberManager(SLOT_ID);
        ASSERT_TRUE(m_pAosSubscriberManager != nullptr);

        EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        EXPECT_CALL(m_objMockISubscriberConfig, GetIndexOfPrimaryPublicUserId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(1));

        m_objPuids.AddElement(AString("PUID1"));
        m_objPuids.AddElement(AString("PUID2"));
        m_objPuids.AddElement(AString("PUID3"));

        EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objPuids));

        EXPECT_CALL(m_objMockISubscriberConfig, RemoveListener(_)).Times(AnyNumber());
    }

    virtual void TearDown() override
    {
        if (m_pAosSubscriberManager)
        {
            delete m_pAosSubscriberManager;
        }
    }

    void SetProvisioned(IN IMS_BOOL bIsFake, IN IMS_BOOL bIsProvisioned)
    {
        if (bIsFake)
        {
            m_pAosSubscriberManager->m_bIsProvisionedForFake = bIsProvisioned;
        }
        else
        {
            m_pAosSubscriberManager->m_bIsProvisioned = bIsProvisioned;
        }
    }

    IMS_BOOL IsProvisioned(IN IMS_BOOL bIsFake)
    {
        return m_pAosSubscriberManager->IsProvisioned(bIsFake);
    }

    IMSList<IAosSubscriberManagerListener*> GetSubscriberManagerListeners()
    {
        return m_pAosSubscriberManager->m_objListeners;
    }

    IMSList<IAosSubscriberManagerListener*> GetSubscriberManagerMonitorListeners()
    {
        return m_pAosSubscriberManager->m_objMonitorListeners;
    }

    void SetPuids(IN IMS_BOOL bIsFake, IN AStringArray& objPuids)
    {
        if (bIsFake)
        {
            m_pAosSubscriberManager->m_objPuidsForFake = objPuids;
        }
        else
        {
            m_pAosSubscriberManager->m_objPuids = objPuids;
        }
    }

    void SetSubscriberConfig(IN IMS_BOOL bIsFake, IN ISubscriberConfig* piSubscriberConfig)
    {
        if (bIsFake)
        {
            m_pAosSubscriberManager->m_piSubscriberConfigFake = piSubscriberConfig;
        }
        else
        {
            m_pAosSubscriberManager->m_piSubscriberConfig = piSubscriberConfig;
        }
    }

    void SetIsim(IN IMS_BOOL bIsim) { m_pAosSubscriberManager->SetIsim(bIsim); }

    void SetUsim(IN IMS_BOOL bUsim) { m_pAosSubscriberManager->SetUsim(bUsim); }

    void Init() { m_pAosSubscriberManager->Init(); }

    IMS_UINT32 GetIsimAt() const { return m_pAosSubscriberManager->GetIsimAt(); }

    void ClearIsimRecovery() { m_pAosSubscriberManager->ClearIsimRecovery(); }

    IMS_BOOL ConfigureAsDefault() { return m_pAosSubscriberManager->ConfigureAsDefault(); }

    IMS_BOOL ConfigureAsFake() { return m_pAosSubscriberManager->ConfigureAsFake(); }

    IMS_BOOL CheckIsimValues() { return m_pAosSubscriberManager->CheckIsimValues(); }

    IMS_BOOL GetTemporaryImpu(OUT AStringArray& objImpus, IN IMS_BOOL bDbWritable)
    {
        return m_pAosSubscriberManager->GetTemporaryImpu(objImpus, bDbWritable);
    }

    void SubscriberConfig_InitCompleted()
    {
        m_pAosSubscriberManager->SubscriberConfig_InitCompleted();
    }

    void SubscriberConfig_RefreshCompleted()
    {
        m_pAosSubscriberManager->SubscriberConfig_RefreshCompleted();
    }

    void SubscriberConfig_RefreshStarted()
    {
        m_pAosSubscriberManager->SubscriberConfig_RefreshStarted();
    }

    void SubscriberConfig_NotifyError(IN IMS_SINT32 nErrorCode)
    {
        m_pAosSubscriberManager->SubscriberConfig_NotifyError(nErrorCode);
    }

    IMS_BOOL ProcessFallbackToImsiBasedIsim(IN IMS_SINT32 nCpi)
    {
        return m_pAosSubscriberManager->ProcessFallbackToImsiBasedIsim(nCpi);
    }

    void ServicePhone_PhoneNumberStateChanged(IN IMS_BOOL bIsRefresh, IN PhoneNumberState eState)
    {
        m_pAosSubscriberManager->ServicePhone_PhoneNumberStateChanged(bIsRefresh, eState);
    }

    IMS_BOOL ProcessPhoneNumberAvailable(IN IMS_BOOL bIsRefresh, IN PhoneNumberState eState)
    {
        return m_pAosSubscriberManager->ProcessPhoneNumberAvailable(bIsRefresh, eState);
    }

    void ServicePhone_IsimStateChanged(IN IsimState eState)
    {
        m_pAosSubscriberManager->ServicePhone_IsimStateChanged(eState);
    }

    IMS_BOOL ProcessIsimStateChange(IN IsimState eState)
    {
        return m_pAosSubscriberManager->ProcessIsimStateChange(eState);
    }

    void ProcessPhoneRestarted() { m_pAosSubscriberManager->ProcessPhoneRestarted(); }

    IMS_BOOL IsPrimaryImpuValid(IN const AStringArray& objImpus) const
    {
        return m_pAosSubscriberManager->IsPrimaryImpuValid(objImpus);
    }

    IMS_BOOL IsSipUri(IN const AString& strImpu) const
    {
        return m_pAosSubscriberManager->IsSipUri(strImpu);
    }

    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
    {
        m_pAosSubscriberManager->StartTimer(nType, nDuration);
    }

    ITimer* GetTimer(IN IMS_UINT32 nType)
    {
        switch (nType)
        {
            case TIMER_ICC_LOADED_WAITING:
                return m_pAosSubscriberManager->m_piTimerToIccLoadedWaiting;

            case TIMER_ISIM_RECOVERY:
                return m_pAosSubscriberManager->m_piTimerToIsimRecovery;

            case TIMER_PHONE_RESTART_RECOVERY:
                return m_pAosSubscriberManager->m_piTimerToPhoneRestartRecovery;

            default:
                return nullptr;
        }
    }

    void ConfigUpdate_NotifyUpdate(
            IN IMS_SINT32 nCpi, IN const AString& strConfName, IN const AString& strExtraParam)
    {
        m_pAosSubscriberManager->ConfigUpdate_NotifyUpdate(nCpi, strConfName, strExtraParam);
    }

    void Timer_TimerExpired(IN ITimer* piTimer)
    {
        m_pAosSubscriberManager->Timer_TimerExpired(piTimer);
    }

    const IMS_CHAR* UpdateEventToString(IN IMS_UINT32 nEvent)
    {
        return m_pAosSubscriberManager->UpdateEventToString(nEvent);
    }

    const IMS_CHAR* StateToString(IN IMS_SINT32 nState)
    {
        return m_pAosSubscriberManager->StateToString(nState);
    }

    IMS_BOOL IsRefreshStarted()
    {
        return m_pAosSubscriberManager->IsRefreshStarted();
        ;
    }
};

TEST_F(AosSubscriberManagerTest, IsReady_True)
{
    SetProvisioned(IMS_FALSE, IMS_TRUE);
    EXPECT_TRUE(m_pAosSubscriberManager->IsReady(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, IsReady_False)
{
    SetProvisioned(IMS_FALSE, IMS_FALSE);
    EXPECT_FALSE(m_pAosSubscriberManager->IsReady(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, IsReady_FakeTrue)
{
    SetProvisioned(IMS_TRUE, IMS_TRUE);
    EXPECT_TRUE(m_pAosSubscriberManager->IsReady(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, IsReady_FakeFalse)
{
    SetProvisioned(IMS_TRUE, IMS_FALSE);
    EXPECT_FALSE(m_pAosSubscriberManager->IsReady(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, AddListener_Null)
{
    m_pAosSubscriberManager->AddListener(IMS_NULL);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListener_ExistListener)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();

    m_pAosSubscriberManager->AddListener(piListener1);
    m_pAosSubscriberManager->AddListener(piListener1);
    m_pAosSubscriberManager->AddListener(piListener2);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 2);
}

TEST_F(AosSubscriberManagerTest, AddListener_Sussess)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pAosSubscriberManager->AddListener(piListener1);
    m_pAosSubscriberManager->AddListener(piListener2);
    m_pAosSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListener_Null)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pAosSubscriberManager->AddListener(piListener1);
    m_pAosSubscriberManager->AddListener(piListener2);
    m_pAosSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 3);

    m_pAosSubscriberManager->RemoveListener(IMS_NULL);
    m_pAosSubscriberManager->RemoveListener(IMS_NULL);
    m_pAosSubscriberManager->RemoveListener(IMS_NULL);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListener_NotNull)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pAosSubscriberManager->AddListener(piListener1);
    m_pAosSubscriberManager->AddListener(piListener2);
    m_pAosSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 3);

    m_pAosSubscriberManager->RemoveListener(piListener3);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 2);
    m_pAosSubscriberManager->RemoveListener(piListener2);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 1);
    m_pAosSubscriberManager->RemoveListener(piListener1);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListenerForMonitor_Null)
{
    m_pAosSubscriberManager->AddListenerForMonitor(IMS_NULL);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListenerForMonitor_ExistListener)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();

    m_pAosSubscriberManager->AddListenerForMonitor(piListener1);
    m_pAosSubscriberManager->AddListenerForMonitor(piListener1);
    m_pAosSubscriberManager->AddListenerForMonitor(piListener2);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 2);
}

TEST_F(AosSubscriberManagerTest, AddListenerForMonitor_Success)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pAosSubscriberManager->AddListenerForMonitor(piListener1);
    m_pAosSubscriberManager->AddListenerForMonitor(piListener2);
    m_pAosSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListenerForMonitor_Null)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pAosSubscriberManager->AddListenerForMonitor(piListener1);
    m_pAosSubscriberManager->AddListenerForMonitor(piListener2);
    m_pAosSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 3);

    m_pAosSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    m_pAosSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    m_pAosSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListenerForMonitor_NotNull)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pAosSubscriberManager->AddListenerForMonitor(piListener1);
    m_pAosSubscriberManager->AddListenerForMonitor(piListener2);
    m_pAosSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 3);

    m_pAosSubscriberManager->RemoveListenerForMonitor(piListener3);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 2);
    m_pAosSubscriberManager->RemoveListenerForMonitor(piListener2);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 1);
    m_pAosSubscriberManager->RemoveListenerForMonitor(piListener1);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, GetConfiguredImpus_Fake)
{
    SetPuids(IMS_TRUE, m_objPuids);
    EXPECT_EQ(m_pAosSubscriberManager->GetConfiguredImpus(IMS_TRUE).GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, GetConfiguredImpus_NotFake)
{
    SetPuids(IMS_FALSE, m_objPuids);
    EXPECT_EQ(m_pAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, GetFakeImpus_SubscriberConfigNull)
{
    SetSubscriberConfig(IMS_TRUE, IMS_NULL);
    EXPECT_EQ(m_pAosSubscriberManager->GetFakeImpus().GetCount(), 0);
}

TEST_F(AosSubscriberManagerTest, GetFakeImpus_SubscriberConfigNotNull)
{
    SetSubscriberConfig(IMS_TRUE, static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_EQ(m_pAosSubscriberManager->GetFakeImpus().GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, Init_SubscriberConfigNotNull)
{
    EXPECT_CALL(m_objMockISubscriberConfig, SetListener(_)).Times(1);

    SetSubscriberConfig(IMS_FALSE, static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    SetSubscriberConfig(IMS_TRUE, static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    Init();
}

TEST_F(AosSubscriberManagerTest, GetIsimAt_ConfigurationNull)
{
    EXPECT_EQ(GetIsimAt(), 1);
}

TEST_F(AosSubscriberManagerTest, ClearIsimRecovery)
{
    StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(GetTimer(TIMER_ISIM_RECOVERY), nullptr);

    ClearIsimRecovery();
    EXPECT_EQ(GetTimer(TIMER_ISIM_RECOVERY), nullptr);
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_SubscriberConfigNull)
{
    SetSubscriberConfig(IMS_FALSE, IMS_NULL);
    EXPECT_FALSE(ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_IsimAndProvisioningIsNotDone)
{
    EXPECT_CALL(m_objMockISubscriberConfig, IsProvisioningDone())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));

    SetSubscriberConfig(IMS_FALSE, static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    SetIsim(IMS_TRUE);

    EXPECT_FALSE(ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_IsimAndProvisioningIsDone)
{
    EXPECT_CALL(m_objMockISubscriberConfig, IsProvisioningDone())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    SetSubscriberConfig(IMS_FALSE, static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    SetIsim(IMS_TRUE);

    EXPECT_FALSE(ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_UsimAndTemporaryImpuFalse)
{
    SetUsim(IMS_TRUE);
    SetPuids(IMS_FALSE, m_objPuids);

    EXPECT_FALSE(ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_IsimAndUsimFalse)
{
    SetPuids(IMS_FALSE, m_objPuids);
    EXPECT_FALSE(ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsFake_SubscriberConfigFakeNull)
{
    SetSubscriberConfig(IMS_TRUE, IMS_NULL);
    EXPECT_FALSE(ConfigureAsFake());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsFake_SubscriberConfigFakeNotNull)
{
    SetSubscriberConfig(IMS_TRUE, static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_TRUE(ConfigureAsFake());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_IsimNotSupport)
{
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));

    SetSubscriberConfig(IMS_FALSE, static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_FALSE(CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_ImpusEmpty)
{
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));

    AStringArray objPuids;
    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPuids));

    SetSubscriberConfig(IMS_FALSE, static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_FALSE(CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ImpuLengthZero)
{
    AStringArray objPuids;
    EXPECT_FALSE(GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, ProcessFallbackToImsiBasedIsim_NotSupportIsimFallback)
{
    EXPECT_FALSE(ProcessFallbackToImsiBasedIsim(IConfigurable::CP_I_IMPU_0));
}

TEST_F(AosSubscriberManagerTest, ServicePhone_PhoneNumberStateChanged_IsNotReady)
{
    SetProvisioned(IMS_FALSE, IMS_FALSE);
    ServicePhone_PhoneNumberStateChanged(IMS_FALSE, PhoneNumberState::SIM_LOADED);
    EXPECT_FALSE(ProcessPhoneNumberAvailable(IMS_FALSE, PhoneNumberState::SIM_LOADED));
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_IsReadyButIsimSupport)
{
    SetProvisioned(IMS_FALSE, IMS_TRUE);
    SetIsim(IMS_TRUE);
    EXPECT_FALSE(ProcessPhoneNumberAvailable(IMS_FALSE, PhoneNumberState::SIM_LOADED));
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_IsReadyButUsimNotSupport)
{
    SetProvisioned(IMS_FALSE, IMS_TRUE);
    SetIsim(IMS_FALSE);
    SetUsim(IMS_FALSE);
    EXPECT_FALSE(ProcessPhoneNumberAvailable(IMS_FALSE, PhoneNumberState::SIM_LOADED));
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_IsReadyUsimSupport)
{
    SetProvisioned(IMS_FALSE, IMS_TRUE);
    SetIsim(IMS_FALSE);
    SetUsim(IMS_TRUE);
    SetPuids(IMS_FALSE, m_objPuids);
    EXPECT_FALSE(ProcessPhoneNumberAvailable(IMS_FALSE, PhoneNumberState::SIM_LOADED));
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneRestarted)
{
    EXPECT_EQ(GetTimer(TIMER_PHONE_RESTART_RECOVERY), nullptr);

    ProcessPhoneRestarted();
    EXPECT_NE(GetTimer(TIMER_PHONE_RESTART_RECOVERY), nullptr);
}

TEST_F(AosSubscriberManagerTest, IsPrimaryImpuValid_PhoneNumberLengthZero)
{
    EXPECT_FALSE(IsPrimaryImpuValid(m_objPuids));
}

TEST_F(AosSubscriberManagerTest, IsSipUri_ImpuLengthZero)
{
    EXPECT_FALSE(IsSipUri(AString::ConstNull()));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_InitCompleted_IsimUsimNotSupport)
{
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    SetSubscriberConfig(IMS_FALSE, static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    SetPuids(IMS_FALSE, m_objPuids);
    SetProvisioned(IMS_FALSE, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
    EXPECT_TRUE(IsProvisioned(IMS_FALSE));

    SubscriberConfig_InitCompleted();
    EXPECT_EQ(m_pAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 0);
    EXPECT_FALSE(IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_RefreshCompleted_IsimUsimNotSupport)
{
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    SetSubscriberConfig(IMS_FALSE, static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    SetPuids(IMS_FALSE, m_objPuids);
    SetProvisioned(IMS_FALSE, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
    EXPECT_TRUE(IsProvisioned(IMS_FALSE));

    SubscriberConfig_RefreshCompleted();
    EXPECT_EQ(m_pAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 0);
    EXPECT_FALSE(IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_RefreshStarted)
{
    EXPECT_FALSE(IsRefreshStarted());

    SubscriberConfig_RefreshStarted();
    EXPECT_TRUE(IsRefreshStarted());
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_NotifyError_IsimUsimNotSupport)
{
    SetPuids(IMS_FALSE, m_objPuids);
    SetProvisioned(IMS_FALSE, IMS_TRUE);
    EXPECT_EQ(m_pAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
    EXPECT_TRUE(IsProvisioned(IMS_FALSE));

    SubscriberConfig_NotifyError(0);
    EXPECT_EQ(m_pAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 0);
    EXPECT_FALSE(IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, ConfigUpdate_NotifyUpdate)
{
    // Currently, there is no logic that requires tests.
    ConfigUpdate_NotifyUpdate(0, AString::ConstNull(), AString::ConstNull());
}

TEST_F(AosSubscriberManagerTest, Timer_TimerExpired_IccLoadedWaiting)
{
    StartTimer(TIMER_ICC_LOADED_WAITING, 3);
    EXPECT_NE(GetTimer(TIMER_ICC_LOADED_WAITING), nullptr);

    Timer_TimerExpired(GetTimer(TIMER_ICC_LOADED_WAITING));
    EXPECT_EQ(GetTimer(TIMER_ICC_LOADED_WAITING), nullptr);
}

TEST_F(AosSubscriberManagerTest, Timer_TimerExpired_IsimRecovery)
{
    StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(GetTimer(TIMER_ISIM_RECOVERY), nullptr);

    Timer_TimerExpired(GetTimer(TIMER_ISIM_RECOVERY));
    EXPECT_EQ(GetTimer(TIMER_ISIM_RECOVERY), nullptr);
}

TEST_F(AosSubscriberManagerTest, Timer_TimerExpired_PhoneRestartRecovery)
{
    StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(GetTimer(TIMER_PHONE_RESTART_RECOVERY), nullptr);

    Timer_TimerExpired(GetTimer(TIMER_PHONE_RESTART_RECOVERY));
    EXPECT_EQ(GetTimer(TIMER_PHONE_RESTART_RECOVERY), nullptr);
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedLoaded)
{
    ServicePhone_IsimStateChanged(IsimState::LOADED);
    EXPECT_TRUE(ProcessIsimStateChange(IsimState::LOADED));
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedRefreshStarted)
{
    ServicePhone_IsimStateChanged(IsimState::REFRESH_STARTED);
    EXPECT_TRUE(ProcessIsimStateChange(IsimState::REFRESH_STARTED));
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedRefreshCompleted)
{
    ServicePhone_IsimStateChanged(IsimState::REFRESH_COMPLETED);
    EXPECT_TRUE(ProcessIsimStateChange(IsimState::REFRESH_COMPLETED));
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedReturnFalse)
{
    ServicePhone_IsimStateChanged(IsimState::NOT_READY);
    EXPECT_FALSE(ProcessIsimStateChange(IsimState::NOT_READY));
}

TEST_F(AosSubscriberManagerTest, UpdateEventToString)
{
    EXPECT_EQ(UpdateEventToString(IConfigurable::CP_I_IMPU_0), "CP_I_IMPU_0");
    EXPECT_EQ(UpdateEventToString(IConfigurable::CP_I_IMPI), "CP_I_IMPI");
    EXPECT_EQ(UpdateEventToString(IConfigurable::CP_I_HOME_DOMAIN_NAME), "CP_I_HOME_DOMAIN_NAME");
    EXPECT_EQ(UpdateEventToString(IConfigurable::CP_I_PHONE_CONTEXT), "CP_I_PHONE_CONTEXT");
    EXPECT_EQ(UpdateEventToString(IConfigurable::CP_I_AUTH_USERNAME), "CP_I_AUTH_USERNAME");
    EXPECT_EQ(UpdateEventToString(IConfigurable::CP_I_AUTH_REALM), "CP_I_AUTH_REALM");
    EXPECT_EQ(UpdateEventToString(IConfigurable::CP_I_SERVER_SCSCF), "CP_I_SERVER_SCSCF");
    EXPECT_EQ(UpdateEventToString(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER),
            "CP_I_WRITE_PROVISIONING_SUBSCRIBER");
    EXPECT_EQ(UpdateEventToString(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM),
            "CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM");
    EXPECT_EQ(UpdateEventToString(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM),
            "CP_I_SUBSCRIPTION_ATTRIBUTE_USIM");
    EXPECT_EQ(UpdateEventToString(1000), "__INVALID__");
}

TEST_F(AosSubscriberManagerTest, StateToString)
{
    EXPECT_EQ(StateToString(IAosSubscriber::NOT_READY), "NOT_READY");
    EXPECT_EQ(StateToString(IAosSubscriber::READY), "READY");
    EXPECT_EQ(StateToString(IAosSubscriber::REFRESH_STARTED), "REFRESH_STARTED");
    EXPECT_EQ(StateToString(IAosSubscriber::REFRESH_COMPLETED), "REFRESH_COMPLETED");
    EXPECT_EQ(StateToString(IAosSubscriber::REFRESH_FAILED), "REFRESH_FAILED");
    EXPECT_EQ(StateToString(100), "INVALID");
}