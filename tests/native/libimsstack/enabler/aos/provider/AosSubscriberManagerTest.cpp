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

#include "ImsTypeDef.h"
#include "IConfigurable.h"
#include "../../../config/interface/common/MockIConfigurable.h"
#include "../../../config/interface/common/MockISubscriberConfig.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosServicePhoneListener.h"
#include "interface/IAosSubscriber.h"
#include "interface/IAosSubscriberManagerListener.h"
#include "provider/AosProvider.h"
#include "provider/AosSubscriberManager.h"

#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosSubscriberManagerListener.h"
#include "../../interface/aos/MockIAosService.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_UINT32 TIMER_ICC_LOADED_WAITING = 100;
const IMS_UINT32 TIMER_ISIM_RECOVERY = 101;
const IMS_UINT32 TIMER_PHONE_RESTART_RECOVERY = 102;

class TestAosSubscriberManager : public AosSubscriberManager
{
public:
    AString m_strPhoneNumber;
    AString m_strTemporaryPublicUserId;
    AString m_strTemporaryPrivateUserId;
    AString m_strTemporaryHomeDomainName;

public:
    inline explicit TestAosSubscriberManager(IN IMS_SINT32 nSlotId) :
            AosSubscriberManager(nSlotId)
    {
    }

    inline void GetPhoneNumber(OUT AString& strPhoneNumber) override
    {
        strPhoneNumber = m_strPhoneNumber;
    }

    inline AString GetTemporaryPublicUserId() override { return m_strTemporaryPublicUserId; }

    inline AString GetTemporaryPrivateUserId() override { return m_strTemporaryPrivateUserId; }

    inline AString GetTemporaryHomeDomainName() override { return m_strTemporaryHomeDomainName; }

    FRIEND_TEST(AosSubscriberManagerTest, IsReady_IsProvisionedForNormalType);
    FRIEND_TEST(AosSubscriberManagerTest, IsReady_IsNotProvisionedForNormalType);
    FRIEND_TEST(AosSubscriberManagerTest, IsReady_IsProvisionedForFakeType);
    FRIEND_TEST(AosSubscriberManagerTest, IsReady_IsNotProvisionedForFakeType);
    FRIEND_TEST(AosSubscriberManagerTest, AddListener_ListenerIsNull);
    FRIEND_TEST(AosSubscriberManagerTest, AddListener);
    FRIEND_TEST(AosSubscriberManagerTest, RemoveListener_ListenerIsNull);
    FRIEND_TEST(AosSubscriberManagerTest, RemoveListener);
    FRIEND_TEST(AosSubscriberManagerTest, AddListenerForMonitor_ListenerIsNull);
    FRIEND_TEST(AosSubscriberManagerTest, AddListenerForMonitor);
    FRIEND_TEST(AosSubscriberManagerTest, RemoveListenerForMonitor_ListenerIsNull);
    FRIEND_TEST(AosSubscriberManagerTest, RemoveListenerForMonitor);
    FRIEND_TEST(AosSubscriberManagerTest, GetConfiguredImpusForFake);
    FRIEND_TEST(AosSubscriberManagerTest, GetConfiguredImpusForNormal);
    FRIEND_TEST(AosSubscriberManagerTest, GetFakeImpus_SubscriberConfigIsNull);
    FRIEND_TEST(AosSubscriberManagerTest, GetFakeImpus_SubscriberConfigIsNotNull);
    FRIEND_TEST(AosSubscriberManagerTest, Init_SubscriberConfigIsNotNull);
    FRIEND_TEST(AosSubscriberManagerTest, IsTimerRunning_TimerIsNotRunning);
    FRIEND_TEST(AosSubscriberManagerTest, IsTimerRunning_TimerIsRunning);
    FRIEND_TEST(AosSubscriberManagerTest, IsTimerRunning_TimerIsInvalid);
    FRIEND_TEST(AosSubscriberManagerTest, GetIsimAt_ConfigurationNull);
    FRIEND_TEST(AosSubscriberManagerTest, ClearIsimRecovery);
    FRIEND_TEST(AosSubscriberManagerTest, ConfigureAsDefault_Isim_SubscriberConfigIsNull);
    FRIEND_TEST(AosSubscriberManagerTest, ConfigureAsDefault_IsimIsTrueAndProvisioningIsNotDone);
    FRIEND_TEST(AosSubscriberManagerTest, ConfigureAsDefault_IsimTrue_ProvisioningDone_ImpuTrue);
    FRIEND_TEST(AosSubscriberManagerTest, ConfigureAsDefault_IsimTrue_ProvisioningDone_ImpuFalse);
    FRIEND_TEST(AosSubscriberManagerTest, ConfigureAsDefault_Usim);
    FRIEND_TEST(AosSubscriberManagerTest, ConfigureAsDefault_IsimUsimFalse_ValidPuids);
    FRIEND_TEST(AosSubscriberManagerTest, ConfigureAsDefault_IsimUsimFalse_InValidPuids_EmptyImpu);
    FRIEND_TEST(AosSubscriberManagerTest, ConfigureAsFake_SubscriberConfigFakeIsNull);
    FRIEND_TEST(AosSubscriberManagerTest, ConfigureAsFake_ValidPuids);
    FRIEND_TEST(AosSubscriberManagerTest, ConfigureAsFake_InvalidPuids);
    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_IsimIsNotSupport);
    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_ImpuIsEmpty);
    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_ImpuIsInvalid);
    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_InvalidImpi);
    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_InvalidHomeDomainName);
    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_ReturnTrue);
    FRIEND_TEST(AosSubscriberManagerTest, GetImpuFromIsim_InvalidImpu);
    FRIEND_TEST(AosSubscriberManagerTest, GetImpuFromIsim_LimitedAdminSmsMode_ValidImpuIsOne);
    FRIEND_TEST(AosSubscriberManagerTest, GetImpuFromIsim_LimitedAdminSmsMode_InvalidPrimaryImpu);
    FRIEND_TEST(AosSubscriberManagerTest, GetImpuFromIsim_LimitedAdminSmsMode_ImpuIsSipUri);
    FRIEND_TEST(AosSubscriberManagerTest, GetTemporaryImpu_ImpuIsInvalid);
    FRIEND_TEST(AosSubscriberManagerTest, GetTemporaryImpu_ImpiIsInvalid);
    FRIEND_TEST(AosSubscriberManagerTest, GetTemporaryImpu_HomeDomainNameIsInvalid);
    FRIEND_TEST(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_Impu);
    FRIEND_TEST(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_Impi);
    FRIEND_TEST(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_HomeDomainName);
    FRIEND_TEST(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_PhoneContext);
    FRIEND_TEST(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_AuthUserName);
    FRIEND_TEST(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_AuthRealm);
    FRIEND_TEST(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_ServerScscf);
    FRIEND_TEST(AosSubscriberManagerTest,
            GetTemporaryImpu_ConfigurableUpdateFailed_WriteProvisioningSubscriber);
    FRIEND_TEST(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateSuccessWithWritable);
    FRIEND_TEST(
            AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateSuccessWithoutWritable);
    FRIEND_TEST(AosSubscriberManagerTest, UpdateImsIdentity_PriorityIsim_UpdateFail);
    FRIEND_TEST(AosSubscriberManagerTest, UpdateImsIdentity_PriorityUsim_UpdateFail);
    FRIEND_TEST(AosSubscriberManagerTest, UpdateImsIdentity_UpdateSuccess);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessFallback_UpdateImsIdentityReturnFalse);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessFallback_UpdateImsIdentityReturnTrue);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessFallbackToImsiBasedIsim_NotSupportIsimFallback);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_TimerIsRunning);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_IsNotReady);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_IsReadyButIsimSupport);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_IsReadyButUsimNotSupport);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_IsReadyUsimSupport);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessIsimRecovery_SupportIsimImsiFallback);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessIsimRecovery_TimerIsRunning);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessPhoneRestarted);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessIsimRecoveryTimerExpired);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessPhoneRestartRecoveryTimerExpired_IsUsimTrue);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessPhoneRestartRecoveryTimerExpired_RefreshStarted);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessPhoneRestartRecoveryTimerExpired_ProcessFallback);
    FRIEND_TEST(AosSubscriberManagerTest, IsPrimaryImpuValid_PhoneNumberLengthZero);
    FRIEND_TEST(AosSubscriberManagerTest, IsSipUri);
    FRIEND_TEST(AosSubscriberManagerTest, NotifyState);
    FRIEND_TEST(AosSubscriberManagerTest, NotifyMonitorState);
    FRIEND_TEST(AosSubscriberManagerTest, NConfiguration_NotifyConfigChanged);
    FRIEND_TEST(AosSubscriberManagerTest, SubscriberConfig_InitCompleted_IsimUsimNotSupport);
    FRIEND_TEST(AosSubscriberManagerTest, SubscriberConfig_RefreshCompleted_IsimUsimNotSupport);
    FRIEND_TEST(AosSubscriberManagerTest, SubscriberConfig_RefreshStarted);
    FRIEND_TEST(AosSubscriberManagerTest, SubscriberConfig_NotifyError_IsimUsimNotSupport);
    FRIEND_TEST(AosSubscriberManagerTest, SubscriberConfig_NotifyError_TimerIsRunning);
    FRIEND_TEST(AosSubscriberManagerTest, SubscriberConfig_NotifyError_ProcessFallback_False);
    FRIEND_TEST(AosSubscriberManagerTest, SubscriberConfig_NotifyError_ProcessFallback_True);
    FRIEND_TEST(AosSubscriberManagerTest, ConfigUpdate_NotifyUpdate);
    FRIEND_TEST(AosSubscriberManagerTest, Timer_TimerExpired_TimerIsNull);
    FRIEND_TEST(AosSubscriberManagerTest, Timer_TimerExpired_IccLoadedWaiting);
    FRIEND_TEST(AosSubscriberManagerTest, Timer_TimerExpired_IsimRecovery);
    FRIEND_TEST(AosSubscriberManagerTest, Timer_TimerExpired_PhoneRestartRecovery);
    FRIEND_TEST(AosSubscriberManagerTest, ServicePhone_PhoneNumberStateChanged_IsNotReady);
    FRIEND_TEST(AosSubscriberManagerTest, ServicePhone_IsimStateChangedLoaded);
    FRIEND_TEST(AosSubscriberManagerTest, ServicePhone_IsimStateChangedRefreshStarted);
    FRIEND_TEST(AosSubscriberManagerTest, ServicePhone_IsimStateChangedRefreshCompleted);
    FRIEND_TEST(AosSubscriberManagerTest, ServicePhone_IsimStateChangedReturnFalse);
    FRIEND_TEST(AosSubscriberManagerTest, IdentityPriorityToString);
    FRIEND_TEST(AosSubscriberManagerTest, PrintIdentity);
    FRIEND_TEST(AosSubscriberManagerTest, UpdateEventToString);
    FRIEND_TEST(AosSubscriberManagerTest, TimerToString);
    FRIEND_TEST(AosSubscriberManagerTest, StateToString);

    ImsList<IAosSubscriberManagerListener*> GetSubscriberManagerListeners()
    {
        return m_objListeners;
    }

    ImsList<IAosSubscriberManagerListener*> GetSubscriberManagerMonitorListeners()
    {
        return m_objMonitorListeners;
    }

    void SetPuids(IN const AStringArray& objPuids) { m_objPuids = objPuids; }

    void SetPuidsForFake(IN const AStringArray& objPuids) { m_objPuidsForFake = objPuids; }

    void SetSubscriberConfig(IN ISubscriberConfig* piSubscriberConfig)
    {
        m_piSubscriberConfig = piSubscriberConfig;
    }

    void SetSubscriberConfigForFake(IN ISubscriberConfig* piSubscriberConfig)
    {
        m_piSubscriberConfigFake = piSubscriberConfig;
    }

    ITimer* GetTimer_IccLoadedWaiting() { return m_piTimerToIccLoadedWaiting; }

    ITimer* GetTimer_IsimRecovery() { return m_piTimerToIsimRecovery; }

    ITimer* GetTimer_PhoneRestartRecovery() { return m_piTimerToPhoneRestartRecovery; }

    void SetTimerToIccLoadedWaiting(IN ITimer* piTimer) { m_piTimerToIccLoadedWaiting = piTimer; }

    void SetTimerToIsimRecovery(IN ITimer* piTimer) { m_piTimerToIsimRecovery = piTimer; }

    void SetTimerToPhoneRestartRecovery(IN ITimer* piTimer)
    {
        m_piTimerToPhoneRestartRecovery = piTimer;
    }

    void SetImsIdentityPriority(IN const ImsVector<IMS_SINT32>& objImsIdentityPriority)
    {
        m_objImsIdentityPriority = objImsIdentityPriority;
    }

    void SetIsimIndex(IN IMS_UINT32 nIsimIndex) { m_nIsimIndexForImpu = nIsimIndex; }

    void SetIAosNConfiguration(IN IAosNConfiguration* piNConfig) { m_piNConfig = piNConfig; }

    void SetSupportLimitedAdminSmsMode(IN IMS_BOOL bSupport)
    {
        m_bSupportLimitedAdminSmsMode = bSupport;
    }
};

class AosSubscriberManagerTest : public ::testing::Test
{
public:
    TestAosSubscriberManager* m_pTestAosSubscriberManager;
    IAosNConfiguration* m_piOriginConfiguration;
    MockISubscriberConfig m_objMockISubscriberConfig;
    MockIConfigurable m_objMockIConfigurable;
    MockIAosService m_objMockIAosService;
    AStringArray m_objPuids;

protected:
    virtual void SetUp() override
    {
        m_pTestAosSubscriberManager = new TestAosSubscriberManager(IMS_SLOT_0);
        ASSERT_TRUE(m_pTestAosSubscriberManager != nullptr);

        EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        EXPECT_CALL(m_objMockISubscriberConfig, GetIndexOfPrimaryPublicUserId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(1));

        m_objPuids.AddElement(AString("sip:user1@ims.com"));
        m_objPuids.AddElement(AString("sip:user2@ims.com"));
        m_objPuids.AddElement(AString("sip:user3@ims.com"));

        EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_objPuids));

        EXPECT_CALL(m_objMockISubscriberConfig, RemoveListener(_)).Times(AnyNumber());

        m_piOriginConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piOriginConfiguration, 0);

        if (m_pTestAosSubscriberManager)
        {
            delete m_pTestAosSubscriberManager;
        }
    }
};

TEST_F(AosSubscriberManagerTest, IsReady_IsProvisionedForNormalType)
{
    // IsProvisioned True, Normal Type
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsReady());
}

TEST_F(AosSubscriberManagerTest, IsReady_IsNotProvisionedForNormalType)
{
    // IsProvisioned False, Normal Type
    m_pTestAosSubscriberManager->SetProvisioned(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsReady(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, IsReady_IsProvisionedForFakeType)
{
    // IsProvisioned True, Fake Type
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE, IAosSubscriber::FAKE);
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsReady(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, IsReady_IsNotProvisionedForFakeType)
{
    // IsProvisioned False, Fake Type
    m_pTestAosSubscriberManager->SetProvisioned(IMS_FALSE, IAosSubscriber::FAKE);
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsReady(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, AddListener_ListenerIsNull)
{
    m_pTestAosSubscriberManager->AddListener(IMS_NULL);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListener)
{
    // Test1 : Add success
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pTestAosSubscriberManager->AddListener(piListener1);
    m_pTestAosSubscriberManager->AddListener(piListener2);
    m_pTestAosSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerListeners().GetSize(), 3);

    // Test2 : duplicated listener
    m_pTestAosSubscriberManager->AddListener(piListener1);
    m_pTestAosSubscriberManager->AddListener(piListener2);
    m_pTestAosSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListener_ListenerIsNull)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pTestAosSubscriberManager->AddListener(piListener1);
    m_pTestAosSubscriberManager->AddListener(piListener2);
    m_pTestAosSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerListeners().GetSize(), 3);

    m_pTestAosSubscriberManager->RemoveListener(IMS_NULL);
    m_pTestAosSubscriberManager->RemoveListener(IMS_NULL);
    m_pTestAosSubscriberManager->RemoveListener(IMS_NULL);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListener)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener4 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener5 = new MockIAosSubscriberManagerListener();

    m_pTestAosSubscriberManager->AddListener(piListener1);
    m_pTestAosSubscriberManager->AddListener(piListener2);
    m_pTestAosSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerListeners().GetSize(), 3);

    // Test1 : Not matched listener
    m_pTestAosSubscriberManager->RemoveListener(piListener4);
    m_pTestAosSubscriberManager->RemoveListener(piListener5);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerListeners().GetSize(), 3);

    // Test2 : Remove success
    m_pTestAosSubscriberManager->RemoveListener(piListener3);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerListeners().GetSize(), 2);
    m_pTestAosSubscriberManager->RemoveListener(piListener2);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerListeners().GetSize(), 1);
    m_pTestAosSubscriberManager->RemoveListener(piListener1);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListenerForMonitor_ListenerIsNull)
{
    m_pTestAosSubscriberManager->AddListenerForMonitor(IMS_NULL);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerMonitorListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListenerForMonitor)
{
    // Test1 : Add success
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener1);
    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener2);
    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerMonitorListeners().GetSize(), 3);

    // Test2 : duplicated listener
    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener1);
    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener2);
    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerMonitorListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListenerForMonitor_ListenerIsNull)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener1);
    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener2);
    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerMonitorListeners().GetSize(), 3);

    m_pTestAosSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    m_pTestAosSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    m_pTestAosSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerMonitorListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListenerForMonitor)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener4 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener5 = new MockIAosSubscriberManagerListener();

    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener1);
    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener2);
    m_pTestAosSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerMonitorListeners().GetSize(), 3);

    // Test1 : Not matched listener
    m_pTestAosSubscriberManager->RemoveListenerForMonitor(piListener4);
    m_pTestAosSubscriberManager->RemoveListenerForMonitor(piListener5);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerMonitorListeners().GetSize(), 3);

    // Test2 : Remove success
    m_pTestAosSubscriberManager->RemoveListenerForMonitor(piListener3);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerMonitorListeners().GetSize(), 2);
    m_pTestAosSubscriberManager->RemoveListenerForMonitor(piListener2);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerMonitorListeners().GetSize(), 1);
    m_pTestAosSubscriberManager->RemoveListenerForMonitor(piListener1);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetSubscriberManagerMonitorListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, GetConfiguredImpusForFake)
{
    AStringArray m_objPuidsEmpty;

    m_pTestAosSubscriberManager->SetPuidsForFake(m_objPuidsEmpty);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_TRUE).GetCount(), 0);
    m_pTestAosSubscriberManager->SetPuidsForFake(m_objPuids);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_TRUE).GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, GetConfiguredImpusForNormal)
{
    AStringArray m_objPuidsEmpty;

    m_pTestAosSubscriberManager->SetPuids(m_objPuidsEmpty);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 0);
    m_pTestAosSubscriberManager->SetPuids(m_objPuids);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, GetFakeImpus_SubscriberConfigIsNull)
{
    // SubscriberConfig is null
    m_pTestAosSubscriberManager->SetSubscriberConfigForFake(IMS_NULL);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetFakeImpus().GetCount(), 0);
}

TEST_F(AosSubscriberManagerTest, GetFakeImpus_SubscriberConfigIsNotNull)
{
    // SubscriberConfig is not null
    m_pTestAosSubscriberManager->SetSubscriberConfigForFake(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_EQ(m_pTestAosSubscriberManager->GetFakeImpus().GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, Init_SubscriberConfigIsNotNull)
{
    EXPECT_CALL(m_objMockISubscriberConfig, SetListener(_)).Times(1);
    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    m_pTestAosSubscriberManager->Init();
}

TEST_F(AosSubscriberManagerTest, IsTimerRunning_TimerIsNotRunning)
{
    m_pTestAosSubscriberManager->SetTimerToIccLoadedWaiting(IMS_NULL);
    m_pTestAosSubscriberManager->SetTimerToIsimRecovery(IMS_NULL);
    m_pTestAosSubscriberManager->SetTimerToPhoneRestartRecovery(IMS_NULL);

    EXPECT_FALSE(m_pTestAosSubscriberManager->IsTimerRunning(TIMER_ICC_LOADED_WAITING));
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsTimerRunning(TIMER_ISIM_RECOVERY));
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsTimerRunning(TIMER_PHONE_RESTART_RECOVERY));
}

TEST_F(AosSubscriberManagerTest, IsTimerRunning_TimerIsRunning)
{
    m_pTestAosSubscriberManager->StartTimer(TIMER_ICC_LOADED_WAITING, 5000);
    m_pTestAosSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 5000);
    m_pTestAosSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 5000);

    EXPECT_TRUE(m_pTestAosSubscriberManager->IsTimerRunning(TIMER_ICC_LOADED_WAITING));
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsTimerRunning(TIMER_ISIM_RECOVERY));
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsTimerRunning(TIMER_PHONE_RESTART_RECOVERY));
}

TEST_F(AosSubscriberManagerTest, IsTimerRunning_TimerIsInvalid)
{
    const IMS_UINT32 TIMER_INVALID = 999;
    m_pTestAosSubscriberManager->StartTimer(TIMER_INVALID, 5000);
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsTimerRunning(TIMER_INVALID));

    m_pTestAosSubscriberManager->StartTimer(TIMER_ICC_LOADED_WAITING, 5000);
    m_pTestAosSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 5000);
    m_pTestAosSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 5000);

    // Stopping an invalid timer does not affect other timers.
    m_pTestAosSubscriberManager->StopTimer(TIMER_INVALID);

    EXPECT_TRUE(m_pTestAosSubscriberManager->IsTimerRunning(TIMER_ICC_LOADED_WAITING));
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsTimerRunning(TIMER_ISIM_RECOVERY));
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsTimerRunning(TIMER_PHONE_RESTART_RECOVERY));
}

TEST_F(AosSubscriberManagerTest, GetIsimAt_ConfigurationNull)
{
    IMS_UINT32 isimIndex = 1;
    m_pTestAosSubscriberManager->SetIsimIndex(isimIndex);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetIsimAt(), isimIndex);
}

TEST_F(AosSubscriberManagerTest, ClearIsimRecovery)
{
    m_pTestAosSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IsimRecovery(), nullptr);

    m_pTestAosSubscriberManager->ClearIsimRecovery();
    EXPECT_EQ(m_pTestAosSubscriberManager->GetTimer_IsimRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_Isim_SubscriberConfigIsNull)
{
    m_pTestAosSubscriberManager->SetSubscriberConfig(IMS_NULL);
    EXPECT_FALSE(m_pTestAosSubscriberManager->ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_IsimIsTrueAndProvisioningIsNotDone)
{
    // ISIM is true and Provisioning is not done
    EXPECT_CALL(m_objMockISubscriberConfig, IsProvisioningDone())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    m_pTestAosSubscriberManager->SetIsim(IMS_TRUE);

    EXPECT_FALSE(m_pTestAosSubscriberManager->ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_IsimTrue_ProvisioningDone_ImpuTrue)
{
    // ISIM is true and Provisioning is done, GetImpuFromIsim is true
    EXPECT_CALL(m_objMockISubscriberConfig, IsProvisioningDone())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    m_pTestAosSubscriberManager->SetIsim(IMS_TRUE);

    EXPECT_TRUE(m_pTestAosSubscriberManager->ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_IsimTrue_ProvisioningDone_ImpuFalse)
{
    // ISIM is true and Provisioning is done, GetImpuFromIsim is false
    EXPECT_CALL(m_objMockISubscriberConfig, IsProvisioningDone())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    AStringArray objEmptyPuids;
    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objEmptyPuids));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    m_pTestAosSubscriberManager->SetIsim(IMS_TRUE);

    EXPECT_FALSE(m_pTestAosSubscriberManager->ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_Usim)
{
    // Test : ISIM is false, USIM is true, GetTemporaryImpu is false
    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    m_pTestAosSubscriberManager->SetIsim(IMS_FALSE);
    m_pTestAosSubscriberManager->SetUsim(IMS_TRUE);
    m_pTestAosSubscriberManager->SetPuids(m_objPuids);

    EXPECT_FALSE(m_pTestAosSubscriberManager->ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_IsimUsimFalse_ValidPuids)
{
    // Valid PUIDs
    m_pTestAosSubscriberManager->SetIsim(IMS_FALSE);
    m_pTestAosSubscriberManager->SetUsim(IMS_FALSE);
    m_pTestAosSubscriberManager->SetPuids(m_objPuids);

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_TRUE(m_pTestAosSubscriberManager->ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsDefault_IsimUsimFalse_InValidPuids_EmptyImpu)
{
    // Invalid PUIDs
    m_pTestAosSubscriberManager->SetIsim(IMS_FALSE);
    m_pTestAosSubscriberManager->SetUsim(IMS_FALSE);

    AStringArray objEmptyPuids;
    m_pTestAosSubscriberManager->SetPuids(objEmptyPuids);

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objEmptyPuids));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_FALSE(m_pTestAosSubscriberManager->ConfigureAsDefault());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsFake_SubscriberConfigFakeIsNull)
{
    // SubscriberConfigFake is null
    m_pTestAosSubscriberManager->SetSubscriberConfigForFake(IMS_NULL);
    EXPECT_FALSE(m_pTestAosSubscriberManager->ConfigureAsFake());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsFake_ValidPuids)
{
    // SubscriberConfigFake is not null, Valid PUIDs
    m_pTestAosSubscriberManager->SetSubscriberConfigForFake(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_TRUE(m_pTestAosSubscriberManager->ConfigureAsFake());
}

TEST_F(AosSubscriberManagerTest, ConfigureAsFake_InvalidPuids)
{
    // SubscriberConfigFake is not null, Invalid PUIDs
    AStringArray objEmptyPuids;
    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objEmptyPuids));
    m_pTestAosSubscriberManager->SetSubscriberConfigForFake(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_FALSE(m_pTestAosSubscriberManager->ConfigureAsFake());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_IsimIsNotSupport)
{
    // Isim is not supported
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_FALSE(m_pTestAosSubscriberManager->CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_ImpuIsEmpty)
{
    // Isim is supported, IMPU is empty
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    AStringArray objEmptyPuids;
    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objEmptyPuids));

    AString strEmptyImpi;
    EXPECT_CALL(m_objMockISubscriberConfig, GetPrivateUserId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strEmptyImpi));

    AString strEmptyHomeDomainName;
    EXPECT_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strEmptyHomeDomainName));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_FALSE(m_pTestAosSubscriberManager->CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_ImpuIsInvalid)
{
    // Isim is supported, IMPU is invalid
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    AStringArray objInvalidPuids;
    objInvalidPuids.AddElement(AString("INVALID_IMPU"));
    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objInvalidPuids));

    AString strEmptyImpi;
    EXPECT_CALL(m_objMockISubscriberConfig, GetPrivateUserId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strEmptyImpi));

    AString strEmptyHomeDomainName;
    EXPECT_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strEmptyHomeDomainName));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_FALSE(m_pTestAosSubscriberManager->CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_InvalidImpi)
{
    // Isim is supported, IMPU is not empty, Invalid IMPI
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objPuids));

    AString strEmptyImpi;
    EXPECT_CALL(m_objMockISubscriberConfig, GetPrivateUserId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strEmptyImpi));

    AString strHomeDomainName = "HOME_DOMAIN_NAME";
    EXPECT_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strHomeDomainName));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_FALSE(m_pTestAosSubscriberManager->CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_InvalidHomeDomainName)
{
    // Isim is supported, IMPU is not empty, Invalid HomeDomainName
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objPuids));

    AString strImpi = "IMPI";
    EXPECT_CALL(m_objMockISubscriberConfig, GetPrivateUserId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strImpi));

    AString strEmptyHomeDomainName;
    EXPECT_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strEmptyHomeDomainName));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_FALSE(m_pTestAosSubscriberManager->CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_ReturnTrue)
{
    // Isim is supported, IMPU is not empty, Valid IMPI, Valid HomeDomainName
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objPuids));

    AString strImpi = "IMPI";
    EXPECT_CALL(m_objMockISubscriberConfig, GetPrivateUserId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strImpi));

    AString strHomeDomainName = "HOME_DOMAIN_NAME";
    EXPECT_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strHomeDomainName));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_TRUE(m_pTestAosSubscriberManager->CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, GetImpuFromIsim_InvalidImpu)
{
    AStringArray objEmptyPuids;
    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objEmptyPuids));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    AStringArray objOutPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetImpuFromIsim(objOutPuids));
}

TEST_F(AosSubscriberManagerTest, GetImpuFromIsim_LimitedAdminSmsMode_ValidImpuIsOne)
{
    AStringArray objOnePuid;
    objOnePuid.AddElement(AString("sip:user1@ims.com"));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objOnePuid));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    m_pTestAosSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_TRUE);

    AStringArray objOutPuids;
    EXPECT_TRUE(m_pTestAosSubscriberManager->GetImpuFromIsim(objOutPuids));
}

TEST_F(AosSubscriberManagerTest, GetImpuFromIsim_LimitedAdminSmsMode_InvalidPrimaryImpu)
{
    AStringArray objInvalidPuids;
    objInvalidPuids.AddElement(AString("INVALID_IMPU1"));
    objInvalidPuids.AddElement(AString("INVALID_IMPU2"));
    objInvalidPuids.AddElement(AString("INVALID_IMPU3"));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objInvalidPuids));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    m_pTestAosSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_TRUE);

    AStringArray objOutPuids;
    EXPECT_TRUE(m_pTestAosSubscriberManager->GetImpuFromIsim(objOutPuids));
}

TEST_F(AosSubscriberManagerTest, GetImpuFromIsim_LimitedAdminSmsMode_ImpuIsSipUri)
{
    AStringArray objSipPuids;
    objSipPuids.AddElement(AString("sip:1234567891@ims.com"));
    objSipPuids.AddElement(AString("sip:1234567892@ims.com"));
    objSipPuids.AddElement(AString("sip:1234567893@ims.com"));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objSipPuids));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    m_pTestAosSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_TRUE);

    AStringArray objOutPuids;
    // strPhoneNumber.GetLength() > USIM_MSISDN_LENGTH
    m_pTestAosSubscriberManager->m_strPhoneNumber = AString("1231234567892");
    EXPECT_TRUE(m_pTestAosSubscriberManager->GetImpuFromIsim(objOutPuids));

    // strPhoneNumber.GetLength() =< USIM_MSISDN_LENGTH
    m_pTestAosSubscriberManager->m_strPhoneNumber = AString("1234567892");
    EXPECT_TRUE(m_pTestAosSubscriberManager->GetImpuFromIsim(objOutPuids));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ImpuIsInvalid)
{
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString::ConstNull();
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_TRUE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ImpiIsInvalid)
{
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_PUID");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString::ConstNull();
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_TRUE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_HomeDomainNameIsInvalid)
{
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_IMPU");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString::ConstNull();

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_TRUE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_Impu)
{
    // Set IMPU, IMPI, HDN
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_IMPU");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPU_0, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPI, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_PHONE_CONTEXT, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_USERNAME, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_REALM, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SERVER_SCSCF, _)).Times(0);
    EXPECT_CALL(
            m_objMockIConfigurable, Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER, _))
            .Times(0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_Impi)
{
    // Set IMPU, IMPI, HDN
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_IMPU");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPU_0, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPI, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_PHONE_CONTEXT, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_USERNAME, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_REALM, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SERVER_SCSCF, _)).Times(0);
    EXPECT_CALL(
            m_objMockIConfigurable, Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER, _))
            .Times(0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_HomeDomainName)
{
    // Set IMPU, IMPI, HDN
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_IMPU");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPU_0, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPI, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_PHONE_CONTEXT, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_USERNAME, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_REALM, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SERVER_SCSCF, _)).Times(0);
    EXPECT_CALL(
            m_objMockIConfigurable, Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER, _))
            .Times(0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_PhoneContext)
{
    // Set IMPU, IMPI, HDN
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_IMPU");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPU_0, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPI, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_PHONE_CONTEXT, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_USERNAME, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_REALM, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SERVER_SCSCF, _)).Times(0);
    EXPECT_CALL(
            m_objMockIConfigurable, Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER, _))
            .Times(0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_AuthUserName)
{
    // Set IMPU, IMPI, HDN
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_IMPU");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPU_0, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPI, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_PHONE_CONTEXT, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_USERNAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_REALM, _)).Times(0);
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SERVER_SCSCF, _)).Times(0);
    EXPECT_CALL(
            m_objMockIConfigurable, Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER, _))
            .Times(0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_AuthRealm)
{
    // Set IMPU, IMPI, HDN
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_IMPU");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPU_0, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPI, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_PHONE_CONTEXT, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_USERNAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_REALM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SERVER_SCSCF, _)).Times(0);
    EXPECT_CALL(
            m_objMockIConfigurable, Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER, _))
            .Times(0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_ServerScscf)
{
    // Set IMPU, IMPI, HDN
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_IMPU");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPU_0, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPI, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_PHONE_CONTEXT, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_USERNAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_REALM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SERVER_SCSCF, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(
            m_objMockIConfigurable, Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER, _))
            .Times(0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest,
        GetTemporaryImpu_ConfigurableUpdateFailed_WriteProvisioningSubscriber)
{
    // Set IMPU, IMPI, HDN
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_IMPU");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPU_0, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPI, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_PHONE_CONTEXT, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_USERNAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_REALM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SERVER_SCSCF, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(
            m_objMockIConfigurable, Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));

    AStringArray objPuids;
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateSuccessWithWritable)
{
    // Set IMPU, IMPI, HDN
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_IMPU");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPU_0, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPI, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_PHONE_CONTEXT, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_USERNAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_REALM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SERVER_SCSCF, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(
            m_objMockIConfigurable, Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    AStringArray objPuids;
    EXPECT_TRUE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateSuccessWithoutWritable)
{
    // Set IMPU, IMPI, HDN
    m_pTestAosSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_IMPU");
    m_pTestAosSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
    m_pTestAosSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPU_0, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPI, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_PHONE_CONTEXT, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_USERNAME, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_REALM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SERVER_SCSCF, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(
            m_objMockIConfigurable, Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER, _))
            .Times(0);

    AStringArray objPuids;
    EXPECT_TRUE(m_pTestAosSubscriberManager->GetTemporaryImpu(objPuids, IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, ProcessFallbackToImsiBasedIsim_NotSupportIsimFallback)
{
    EXPECT_FALSE(m_pTestAosSubscriberManager->ProcessFallbackToImsiBasedIsim(
            IConfigurable::CP_I_IMPU_0));
}

TEST_F(AosSubscriberManagerTest, UpdateImsIdentity_PriorityIsim_UpdateFail)
{
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported()).Times(0);
    EXPECT_CALL(m_objMockISubscriberConfig, IsUsimSupported()).Times(0);

    EXPECT_FALSE(m_pTestAosSubscriberManager->UpdateImsIdentity(
            CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM));
}

TEST_F(AosSubscriberManagerTest, UpdateImsIdentity_PriorityUsim_UpdateFail)
{
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported()).Times(0);
    EXPECT_CALL(m_objMockISubscriberConfig, IsUsimSupported()).Times(0);

    EXPECT_FALSE(m_pTestAosSubscriberManager->UpdateImsIdentity(
            CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM));
}

TEST_F(AosSubscriberManagerTest, UpdateImsIdentity_UpdateSuccess)
{
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockISubscriberConfig, IsUsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTestAosSubscriberManager->UpdateImsIdentity(
            CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM));
}

// ProcessFallback
TEST_F(AosSubscriberManagerTest, ProcessFallback_UpdateImsIdentityReturnFalse)
{
    EXPECT_FALSE(m_pTestAosSubscriberManager->ProcessFallback(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, ProcessFallback_UpdateImsIdentityReturnTrue)
{
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockISubscriberConfig, IsUsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTestAosSubscriberManager->ProcessFallback(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_TimerIsRunning)
{
    m_pTestAosSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_FALSE(m_pTestAosSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED));
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_IsNotReady)
{
    m_pTestAosSubscriberManager->SetProvisioned(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED));
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_IsReadyButIsimSupport)
{
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pTestAosSubscriberManager->SetIsim(IMS_TRUE);
    EXPECT_FALSE(m_pTestAosSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED));
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_IsReadyButUsimNotSupport)
{
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pTestAosSubscriberManager->SetIsim(IMS_FALSE);
    m_pTestAosSubscriberManager->SetUsim(IMS_FALSE);
    EXPECT_FALSE(m_pTestAosSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED));
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneNumberAvailable_IsReadyUsimSupport)
{
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pTestAosSubscriberManager->SetIsim(IMS_FALSE);
    m_pTestAosSubscriberManager->SetUsim(IMS_TRUE);
    m_pTestAosSubscriberManager->SetPuids(m_objPuids);
    EXPECT_FALSE(m_pTestAosSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED));
}

TEST_F(AosSubscriberManagerTest, ProcessIsimRecovery_SupportIsimImsiFallback)
{
    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    m_pTestAosSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IsimRecovery(), nullptr);

    AosProvider::GetInstance()->SetService(static_cast<IAosService*>(&m_objMockIAosService), 0);
    EXPECT_CALL(m_objMockIAosService, NotifyAosIsimState(AosIsimState::INVALID)).Times(1);

    m_pTestAosSubscriberManager->ProcessIsimRecovery();
    EXPECT_EQ(m_pTestAosSubscriberManager->GetTimer_IsimRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessIsimRecovery_TimerIsRunning)
{
    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);

    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[ISIM]");

    m_pTestAosSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IsimRecovery(), nullptr);

    AosProvider::GetInstance()->SetService(static_cast<IAosService*>(&m_objMockIAosService), 0);
    EXPECT_CALL(m_objMockIAosService, NotifyAosIsimState(AosIsimState::INVALID)).Times(0);

    m_pTestAosSubscriberManager->ProcessIsimRecovery();
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IsimRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneRestarted)
{
    EXPECT_EQ(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);

    m_pTestAosSubscriberManager->ProcessPhoneRestarted();
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessIsimRecoveryTimerExpired)
{
    m_pTestAosSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IsimRecovery(), nullptr);

    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIBER_ALL, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    m_pTestAosSubscriberManager->ProcessIsimRecoveryTimerExpired();
    EXPECT_EQ(m_pTestAosSubscriberManager->GetTimer_IsimRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneRestartRecoveryTimerExpired_IsUsimTrue)
{
    m_pTestAosSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);

    m_pTestAosSubscriberManager->SetIsim(IMS_FALSE);
    m_pTestAosSubscriberManager->SetUsim(IMS_TRUE);

    m_pTestAosSubscriberManager->ProcessPhoneRestartRecoveryTimerExpired();
    EXPECT_EQ(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneRestartRecoveryTimerExpired_RefreshStarted)
{
    m_pTestAosSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);

    m_pTestAosSubscriberManager->SetIsim(IMS_TRUE);
    m_pTestAosSubscriberManager->SetUsim(IMS_FALSE);
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pTestAosSubscriberManager->m_bIsRefreshStarted = IMS_TRUE;

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    // ProcessFallback(IMS_TRUE) and UpdateImsIdentity(IMS_TRUE) should return IMS_TRUE.
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));
    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockISubscriberConfig, IsUsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    m_pTestAosSubscriberManager->ProcessPhoneRestartRecoveryTimerExpired();
    EXPECT_EQ(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneRestartRecoveryTimerExpired_ProcessFallback)
{
    m_pTestAosSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);

    m_pTestAosSubscriberManager->SetIsim(IMS_TRUE);
    m_pTestAosSubscriberManager->SetUsim(IMS_FALSE);
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pTestAosSubscriberManager->m_bIsRefreshStarted = IMS_TRUE;

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    m_pTestAosSubscriberManager->ProcessPhoneRestartRecoveryTimerExpired();
    EXPECT_EQ(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, IsPrimaryImpuValid_PhoneNumberLengthZero)
{
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsPrimaryImpuValid(m_objPuids));
}

TEST_F(AosSubscriberManagerTest, NotifyState)
{
    MockIAosSubscriberManagerListener objListener1;
    MockIAosSubscriberManagerListener objListener2;
    MockIAosSubscriberManagerListener objListener3;

    EXPECT_CALL(objListener1, AosSubscriberManager_NotifyState(_)).Times(2);
    EXPECT_CALL(objListener2, AosSubscriberManager_NotifyState(_)).Times(2);
    EXPECT_CALL(objListener3, AosSubscriberManager_NotifyState(_)).Times(2);
    m_pTestAosSubscriberManager->AddListener(
            static_cast<IAosSubscriberManagerListener*>(&objListener1));
    m_pTestAosSubscriberManager->AddListener(
            static_cast<IAosSubscriberManagerListener*>(&objListener2));
    m_pTestAosSubscriberManager->AddListener(
            static_cast<IAosSubscriberManagerListener*>(&objListener3));

    m_pTestAosSubscriberManager->NotifyState(IAosSubscriber::READY);
    m_pTestAosSubscriberManager->NotifyState(IAosSubscriber::NOT_READY);
}

TEST_F(AosSubscriberManagerTest, NotifyMonitorState)
{
    MockIAosSubscriberManagerListener objListener1;
    MockIAosSubscriberManagerListener objListener2;
    MockIAosSubscriberManagerListener objListener3;

    EXPECT_CALL(objListener1, AosSubscriberManager_NotifyState(_)).Times(2);
    EXPECT_CALL(objListener2, AosSubscriberManager_NotifyState(_)).Times(2);
    EXPECT_CALL(objListener3, AosSubscriberManager_NotifyState(_)).Times(2);
    m_pTestAosSubscriberManager->AddListenerForMonitor(
            static_cast<IAosSubscriberManagerListener*>(&objListener1));
    m_pTestAosSubscriberManager->AddListenerForMonitor(
            static_cast<IAosSubscriberManagerListener*>(&objListener2));
    m_pTestAosSubscriberManager->AddListenerForMonitor(
            static_cast<IAosSubscriberManagerListener*>(&objListener3));

    m_pTestAosSubscriberManager->NotifyMonitorState(IAosSubscriber::READY);
    m_pTestAosSubscriberManager->NotifyMonitorState(IAosSubscriber::NOT_READY);
}

TEST_F(AosSubscriberManagerTest, IsSipUri)
{
    // Test1 : Impu length is zero
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsSipUri(AString::ConstNull()));

    // Test2 : Impu scheme is Sip
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsSipUri(AString("sip:user1@ims.com")));

    // Test3 : Impu scheme is Sips
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsSipUri(AString("sips:user1@ims.com")));
}

TEST_F(AosSubscriberManagerTest, NConfiguration_NotifyConfigChanged)
{
    // Test1: UpdateNConfiguration is false
    EXPECT_FALSE(m_pTestAosSubscriberManager->UpdateNConfiguration());

    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pTestAosSubscriberManager->SetPuids(m_objPuids);
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsProvisioned());
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);

    m_pTestAosSubscriberManager->NConfiguration_NotifyConfigChanged();
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsProvisioned());
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);

    // Test2: UpdateNConfiguration is true
    MockIAosNConfiguration objMockIAosNConfiguration;
    EXPECT_CALL(objMockIAosNConfiguration, GetIsimIndexForImpu()).WillRepeatedly(Return(0));
    EXPECT_CALL(objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillRepeatedly(Return(IMS_TRUE));
    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF);
    EXPECT_CALL(objMockIAosNConfiguration, GetImsIdentityPriority())
            .WillRepeatedly(ReturnRef(objImsIdentityPriority));
    m_pTestAosSubscriberManager->SetIAosNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration));

    m_pTestAosSubscriberManager->SetSubscriberConfig(IMS_NULL);
    m_pTestAosSubscriberManager->NConfiguration_NotifyConfigChanged();
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsProvisioned());
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 0);

    // for CleanUp()
    m_pTestAosSubscriberManager->SetIAosNConfiguration(IMS_NULL);
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_InitCompleted_IsimUsimNotSupport)
{
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    m_pTestAosSubscriberManager->SetPuids(m_objPuids);
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));

    m_pTestAosSubscriberManager->SubscriberConfig_InitCompleted();
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_RefreshCompleted_IsimUsimNotSupport)
{
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));
    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));

    m_pTestAosSubscriberManager->SetPuids(m_objPuids);
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));

    m_pTestAosSubscriberManager->SubscriberConfig_RefreshCompleted();
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_RefreshStarted)
{
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsRefreshStarted());

    m_pTestAosSubscriberManager->SubscriberConfig_RefreshStarted();
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsRefreshStarted());
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_NotifyError_IsimUsimNotSupport)
{
    m_pTestAosSubscriberManager->SetPuids(m_objPuids);
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));

    m_pTestAosSubscriberManager->SubscriberConfig_NotifyError(0);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_NotifyError_TimerIsRunning)
{
    m_pTestAosSubscriberManager->SetPuids(m_objPuids);
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    m_pTestAosSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);

    m_pTestAosSubscriberManager->SubscriberConfig_NotifyError(0);

    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_NotifyError_ProcessFallback_False)
{
    m_pTestAosSubscriberManager->SetPuids(m_objPuids);
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    m_pTestAosSubscriberManager->SubscriberConfig_NotifyError(0);

    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_NotifyError_ProcessFallback_True)
{
    m_pTestAosSubscriberManager->SetPuids(m_objPuids);
    m_pTestAosSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
    EXPECT_TRUE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    // ProcessFallback(IMS_TRUE) and UpdateImsIdentity(IMS_TRUE) should return IMS_TRUE.
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));
    m_pTestAosSubscriberManager->SetSubscriberConfig(
            static_cast<ISubscriberConfig*>(&m_objMockISubscriberConfig));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockISubscriberConfig, IsUsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    m_pTestAosSubscriberManager->SubscriberConfig_NotifyError(0);

    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 0);
    EXPECT_FALSE(m_pTestAosSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, ConfigUpdate_NotifyUpdate)
{
    // Currently, there is no logic that requires tests.
    m_pTestAosSubscriberManager->ConfigUpdate_NotifyUpdate(
            0, AString::ConstNull(), AString::ConstNull());
}

TEST_F(AosSubscriberManagerTest, Timer_TimerExpired_TimerIsNull)
{
    m_pTestAosSubscriberManager->StartTimer(TIMER_ICC_LOADED_WAITING, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IccLoadedWaiting(), nullptr);

    m_pTestAosSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IccLoadedWaiting(), nullptr);

    m_pTestAosSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IccLoadedWaiting(), nullptr);

    // Expiration of an invalid timer does not affect other timers.
    m_pTestAosSubscriberManager->Timer_TimerExpired(IMS_NULL);

    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IccLoadedWaiting(), nullptr);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IsimRecovery(), nullptr);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, Timer_TimerExpired_IccLoadedWaiting)
{
    m_pTestAosSubscriberManager->StartTimer(TIMER_ICC_LOADED_WAITING, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IccLoadedWaiting(), nullptr);

    m_pTestAosSubscriberManager->Timer_TimerExpired(
            m_pTestAosSubscriberManager->GetTimer_IccLoadedWaiting());
    EXPECT_EQ(m_pTestAosSubscriberManager->GetTimer_IccLoadedWaiting(), nullptr);
}

TEST_F(AosSubscriberManagerTest, Timer_TimerExpired_IsimRecovery)
{
    m_pTestAosSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_IsimRecovery(), nullptr);

    m_pTestAosSubscriberManager->Timer_TimerExpired(
            m_pTestAosSubscriberManager->GetTimer_IsimRecovery());
    EXPECT_EQ(m_pTestAosSubscriberManager->GetTimer_IsimRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, Timer_TimerExpired_PhoneRestartRecovery)
{
    m_pTestAosSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);

    m_pTestAosSubscriberManager->Timer_TimerExpired(
            m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery());
    EXPECT_EQ(m_pTestAosSubscriberManager->GetTimer_PhoneRestartRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, ServicePhone_PhoneNumberStateChanged_IsNotReady)
{
    m_pTestAosSubscriberManager->SetPuids(m_objPuids);
    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);

    m_pTestAosSubscriberManager->SetProvisioned(IMS_FALSE);

    m_pTestAosSubscriberManager->ServicePhone_PhoneNumberStateChanged(
            IMS_FALSE, PhoneNumberState::SIM_LOADED);

    EXPECT_EQ(m_pTestAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 0);
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedLoaded)
{
    m_pTestAosSubscriberManager->ServicePhone_IsimStateChanged(IsimState::LOADED);
    EXPECT_TRUE(m_pTestAosSubscriberManager->ProcessIsimStateChange(IsimState::LOADED));
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedRefreshStarted)
{
    m_pTestAosSubscriberManager->ServicePhone_IsimStateChanged(IsimState::REFRESH_STARTED);
    EXPECT_TRUE(m_pTestAosSubscriberManager->ProcessIsimStateChange(IsimState::REFRESH_STARTED));
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedRefreshCompleted)
{
    m_pTestAosSubscriberManager->ServicePhone_IsimStateChanged(IsimState::REFRESH_COMPLETED);
    EXPECT_TRUE(m_pTestAosSubscriberManager->ProcessIsimStateChange(IsimState::REFRESH_COMPLETED));
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedReturnFalse)
{
    m_pTestAosSubscriberManager->ServicePhone_IsimStateChanged(IsimState::NOT_READY);
    EXPECT_FALSE(m_pTestAosSubscriberManager->ProcessIsimStateChange(IsimState::NOT_READY));
}

TEST_F(AosSubscriberManagerTest, IdentityPriorityToString)
{
    ImsVector<IMS_SINT32> objImsIdentityPriority;

    // Test1 : IdentityPriority is empty
    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[Empty]");

    // Test2 : [ISIM][USIM]
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[ISIM][USIM]");

    // Test2 : [ISIM][USIM][CONF]
    objImsIdentityPriority.Clear();
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF);

    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[ISIM][USIM][CONF]");

    // Test3 : [ISIM][ISIM_IMSI]
    objImsIdentityPriority.Clear();
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);
    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    // Test4 : [USIM][ISIM]
    objImsIdentityPriority.Clear();
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[USIM][ISIM]");

    // Test4 : [CONF]
    objImsIdentityPriority.Clear();
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF);
    m_pTestAosSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);
    EXPECT_STREQ(m_pTestAosSubscriberManager->IdentityPriorityToString(), "[CONF]");
}

TEST_F(AosSubscriberManagerTest, PrintIdentity)
{
    EXPECT_STREQ(m_pTestAosSubscriberManager->PrintIdentity(
                         CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM),
            "ISIM");
    EXPECT_STREQ(m_pTestAosSubscriberManager->PrintIdentity(
                         CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM),
            "USIM");
    EXPECT_STREQ(m_pTestAosSubscriberManager->PrintIdentity(
                         CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI),
            "ISIM_IMSI");
    EXPECT_STREQ(m_pTestAosSubscriberManager->PrintIdentity(
                         CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF),
            "CONF");
}

TEST_F(AosSubscriberManagerTest, UpdateEventToString)
{
    EXPECT_STREQ(m_pTestAosSubscriberManager->UpdateEventToString(IConfigurable::CP_I_IMPU_0),
            "CP_I_IMPU_0");
    EXPECT_STREQ(m_pTestAosSubscriberManager->UpdateEventToString(IConfigurable::CP_I_IMPI),
            "CP_I_IMPI");
    EXPECT_STREQ(
            m_pTestAosSubscriberManager->UpdateEventToString(IConfigurable::CP_I_HOME_DOMAIN_NAME),
            "CP_I_HOME_DOMAIN_NAME");
    EXPECT_STREQ(
            m_pTestAosSubscriberManager->UpdateEventToString(IConfigurable::CP_I_PHONE_CONTEXT),
            "CP_I_PHONE_CONTEXT");
    EXPECT_STREQ(
            m_pTestAosSubscriberManager->UpdateEventToString(IConfigurable::CP_I_AUTH_USERNAME),
            "CP_I_AUTH_USERNAME");
    EXPECT_STREQ(m_pTestAosSubscriberManager->UpdateEventToString(IConfigurable::CP_I_AUTH_REALM),
            "CP_I_AUTH_REALM");
    EXPECT_STREQ(m_pTestAosSubscriberManager->UpdateEventToString(IConfigurable::CP_I_SERVER_SCSCF),
            "CP_I_SERVER_SCSCF");
    EXPECT_STREQ(m_pTestAosSubscriberManager->UpdateEventToString(
                         IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER),
            "CP_I_WRITE_PROVISIONING_SUBSCRIBER");
    EXPECT_STREQ(m_pTestAosSubscriberManager->UpdateEventToString(
                         IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM),
            "CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM");
    EXPECT_STREQ(m_pTestAosSubscriberManager->UpdateEventToString(
                         IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM),
            "CP_I_SUBSCRIPTION_ATTRIBUTE_USIM");
    EXPECT_STREQ(m_pTestAosSubscriberManager->UpdateEventToString(1000), "__INVALID__");
}

TEST_F(AosSubscriberManagerTest, TimerToString)
{
    EXPECT_STREQ(m_pTestAosSubscriberManager->TimerToString(TIMER_ICC_LOADED_WAITING),
            "TIMER_ICC_LOADED_WAITING");
    EXPECT_STREQ(
            m_pTestAosSubscriberManager->TimerToString(TIMER_ISIM_RECOVERY), "TIMER_ISIM_RECOVERY");
    EXPECT_STREQ(m_pTestAosSubscriberManager->TimerToString(TIMER_PHONE_RESTART_RECOVERY),
            "TIMER_PHONE_RESTART_RECOVERY");
    EXPECT_STREQ(m_pTestAosSubscriberManager->TimerToString(999), "__INVALID__");
}

TEST_F(AosSubscriberManagerTest, StateToString)
{
    EXPECT_STREQ(
            m_pTestAosSubscriberManager->StateToString(IAosSubscriber::NOT_READY), "NOT_READY");
    EXPECT_STREQ(m_pTestAosSubscriberManager->StateToString(IAosSubscriber::READY), "READY");
    EXPECT_STREQ(m_pTestAosSubscriberManager->StateToString(IAosSubscriber::REFRESH_STARTED),
            "REFRESH_STARTED");
    EXPECT_STREQ(m_pTestAosSubscriberManager->StateToString(IAosSubscriber::REFRESH_COMPLETED),
            "REFRESH_COMPLETED");
    EXPECT_STREQ(m_pTestAosSubscriberManager->StateToString(IAosSubscriber::REFRESH_FAILED),
            "REFRESH_FAILED");
    EXPECT_STREQ(m_pTestAosSubscriberManager->StateToString(100), "INVALID");
}