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
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosServicePhoneListener.h"
#include "interface/IAosSubscriber.h"
#include "interface/IAosSubscriberManagerListener.h"
#include "provider/AosProvider.h"
#include "provider/AosSubscriberManager.h"

#include "../../../config/interface/common/MockIConfigurable.h"
#include "../../../config/interface/common/MockISubscriberConfig.h"
#include "../../../platform/interface/MockIPhoneInfoSubscriber.h"
#include "../../interface/aos/MockIAosService.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosSubscriberManagerListener.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;
using ::testing::SetArgPointee;

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

    inline void SetSubscriberConfig(IN ISubscriberConfig* piSubscriberConfig)
    {
        m_piSubscriberConfig = piSubscriberConfig;
    }

    inline void SetSubscriberConfigForFake(IN ISubscriberConfig* piSubscriberConfig)
    {
        m_piSubscriberConfigFake = piSubscriberConfig;
    }

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
    // TEST : ConfigureAsDefault
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsConfigureAsDefaultWhenIsimTrueProvisioningDone);
    FRIEND_TEST(AosSubscriberManagerTest, FailedConfigureAsDefaultWithoutSubscriberConfig);
    FRIEND_TEST(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenIsimTrueProvisioningNotDone);
    FRIEND_TEST(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenFailedGetImpuFromIsim);
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsConfigureAsDefaultWhenUsimAndValidTempImpu);
    FRIEND_TEST(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenUsimAndFailedGetTempImpu);
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsConfigureAsDefaultWhenConf);
    FRIEND_TEST(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenInvalidPuids);
    // TEST : ConfigureAsFake
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsConfigureAsFake);
    FRIEND_TEST(AosSubscriberManagerTest, FailedConfigureAsFakeWhenSubscriberConfigFakeIsNull);
    FRIEND_TEST(AosSubscriberManagerTest, FailedConfigureAsFakeWhenGetInvalidPuids);
    FRIEND_TEST(AosSubscriberManagerTest, FailedConfigureAsFakeWhenGetInvalidImpu);

    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_IsimIsNotSupport);
    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_ImpuIsEmpty);
    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_ImpuIsInvalid);
    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_InvalidImpi);
    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_InvalidHomeDomainName);
    FRIEND_TEST(AosSubscriberManagerTest, CheckIsimValues_ReturnTrue);
    // TEST : GetImpuFromIsim
    FRIEND_TEST(AosSubscriberManagerTest, FailedGetImpuFromIsimWhenGetEmptyPuids);
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsGetImpuWhenLimitedAdminSmsModeAndOneValidImpu);
    FRIEND_TEST(
            AosSubscriberManagerTest, SucceedsGetImpuWhenLimitedAdminSmsModeAndInvalidPrimaryImpu);
    FRIEND_TEST(
            AosSubscriberManagerTest, SucceedsGetImpuWhenSipImpuAndPhoneNumberIsGreaterThenMsisdn);
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsGetImpuWhenSipImpuAndPhoneNumberIsLessThenMsisdn);
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsGetImpuWhenSecondImpuIsNotSip);

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
    // TEST : UpdateImsi
    FRIEND_TEST(AosSubscriberManagerTest, FailedUpdateImsiWhenSubsInfoIsNull);
    // TEST : UpdateImsIdentity
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsUpdateImsIdentity);
    FRIEND_TEST(
            AosSubscriberManagerTest, ReturnsFalseWhenFailedUpdateImsIdentityWithoutIConfigurable);
    FRIEND_TEST(AosSubscriberManagerTest, ReturnsFalseWhenFailedUpdateImsIdentityWithIsimIdentity);
    FRIEND_TEST(AosSubscriberManagerTest, ReturnsFalseWhenFailedUpdateImsIdentityWithUsimIdentity);

    FRIEND_TEST(AosSubscriberManagerTest, ProcessFallback_UpdateImsIdentityReturnFalse);
    FRIEND_TEST(AosSubscriberManagerTest, ProcessFallback_UpdateImsIdentityReturnTrue);
    // TEST : ProcessFallbackToImsiBasedIsim
    FRIEND_TEST(
            AosSubscriberManagerTest, FailedProcessFallbackToImsiBasedIsimWhenNotSupportFallBack);
    FRIEND_TEST(AosSubscriberManagerTest, FailedProcessFallbackToImsiBasedIsimWhenInvalidCpi);
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsProcessFallbackToImsiBasedIsimWithImpu);
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsProcessFallbackToImsiBasedIsimWithImpi);
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsProcessFallbackToImsiBasedIsimWithHdn);
    // TEST : ProcessPhoneNumberAvailable
    FRIEND_TEST(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenTimerIsRunning);
    FRIEND_TEST(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenIsNotReady);
    FRIEND_TEST(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenSupportIsim);
    FRIEND_TEST(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenNotSupportUsim);
    FRIEND_TEST(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenInvalidImpus);
    FRIEND_TEST(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenInvalidTempImpus);
    FRIEND_TEST(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenEqualsTempImpu);
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsProcessPhoneNumberAvailable);

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
    // TEST : NConfiguration_NotifyConfigChanged
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsUpdateNConfiguration);
    FRIEND_TEST(AosSubscriberManagerTest, SucceedsUpdateNConfigurationWhenPrioritySizeIsSame);
    FRIEND_TEST(AosSubscriberManagerTest, FailedUpdateNConfigurationWhenNConfigIsNull);
    FRIEND_TEST(AosSubscriberManagerTest, FailedUpdateNConfigurationWhenSameConfiguration);

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
};

class AosSubscriberManagerTest : public ::testing::Test
{
public:
    TestAosSubscriberManager* m_pSubscriberManager;

    TestPhoneInfoService m_objPhoneInfoService;
    IAosNConfiguration* m_piOriginConfiguration;
    MockISubscriberConfig m_objMockISubscriberConfig;
    MockISubscriberConfig m_objMockISubscriberConfigFake;
    MockISubscriberInfo m_objMockISubscriberInfo;
    MockIConfigurable m_objMockIConfigurable;
    MockIAosService m_objMockIAosService;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

    AStringArray m_objValidPuids;
    AStringArray m_objEmptyPuids;
    AStringArray m_objOutPuids;

    AString m_objStrTestImsi;
    AString m_objStrTestMcc;
    AString m_objStrTestMnc;

protected:
    virtual void SetUp() override
    {
        m_objStrTestImsi = AString("123456789");
        m_objStrTestMcc = AString("123");
        m_objStrTestMnc = AString("456");

        SetUpDefaultISubscriberInfo();
        SetUpDefaultISubscriberConfig();
        SetUpDefaultIConfigurable();

        m_objPhoneInfoService.SetSubscriberInfo(&m_objMockISubscriberInfo);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);

        m_pSubscriberManager = new TestAosSubscriberManager(IMS_SLOT_0);
        ASSERT_TRUE(m_pSubscriberManager != nullptr);

        m_pSubscriberManager->SetSubscriberConfig(&m_objMockISubscriberConfig);
        m_pSubscriberManager->SetSubscriberConfigForFake(&m_objMockISubscriberConfigFake);

        m_pSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_PUID");
        m_pSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
        m_pSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");

        m_piOriginConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piOriginConfiguration, 0);

        if (m_pSubscriberManager)
        {
            delete m_pSubscriberManager;
        }

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
    }

    void SetUpDefaultISubscriberConfig()
    {
        ON_CALL(m_objMockISubscriberConfig, GetConfigurable())
                .WillByDefault(Return(&m_objMockIConfigurable));

        ON_CALL(m_objMockISubscriberConfig, GetIndexOfPrimaryPublicUserId())
                .WillByDefault(Return(1));

        m_objValidPuids.AddElement(AString("sip:user1@ims.com"));
        m_objValidPuids.AddElement(AString("sip:user2@ims.com"));
        m_objValidPuids.AddElement(AString("sip:user3@ims.com"));

        ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
                .WillByDefault(ReturnRef(m_objValidPuids));

        ON_CALL(m_objMockISubscriberConfigFake, GetPublicUserIds())
                .WillByDefault(ReturnRef(m_objValidPuids));

        ON_CALL(m_objMockISubscriberConfig, RemoveListener(_)).WillByDefault(Return());
    }

    void SetUpDefaultIConfigurable()
    {
        ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPU_0, _))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_IMPI, _))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_HOME_DOMAIN_NAME, _))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_PHONE_CONTEXT, _))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_USERNAME, _))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_AUTH_REALM, _))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SERVER_SCSCF, _))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(m_objMockIConfigurable,
                Update(IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER, _))
                .WillByDefault(Return(IMS_TRUE));
    }

    void SetUpDefaultISubscriberInfo()
    {
        ON_CALL(m_objMockISubscriberInfo, GetSubscriberIdInternal(_))
                .WillByDefault(DoAll(SetArgPointee<0>(m_objStrTestImsi), Return(IMS_TRUE)));
        ON_CALL(m_objMockISubscriberInfo, GetSimMccInternal(_))
                .WillByDefault(DoAll(SetArgPointee<0>(m_objStrTestMcc), Return(IMS_TRUE)));
        ON_CALL(m_objMockISubscriberInfo, GetSimMncInternal(_))
                .WillByDefault(DoAll(SetArgPointee<0>(m_objStrTestMnc), Return(IMS_TRUE)));
    }
};

TEST_F(AosSubscriberManagerTest, IsReady_IsProvisionedForNormalType)
{
    // IsProvisioned True, Normal Type
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_TRUE(m_pSubscriberManager->IsReady());
}

TEST_F(AosSubscriberManagerTest, IsReady_IsNotProvisionedForNormalType)
{
    // IsProvisioned False, Normal Type
    m_pSubscriberManager->SetProvisioned(IMS_FALSE);
    EXPECT_FALSE(m_pSubscriberManager->IsReady(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, IsReady_IsProvisionedForFakeType)
{
    // IsProvisioned True, Fake Type
    m_pSubscriberManager->SetProvisioned(IMS_TRUE, IAosSubscriber::FAKE);
    EXPECT_TRUE(m_pSubscriberManager->IsReady(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, IsReady_IsNotProvisionedForFakeType)
{
    // IsProvisioned False, Fake Type
    m_pSubscriberManager->SetProvisioned(IMS_FALSE, IAosSubscriber::FAKE);
    EXPECT_FALSE(m_pSubscriberManager->IsReady(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, AddListener_ListenerIsNull)
{
    m_pSubscriberManager->AddListener(IMS_NULL);
    EXPECT_EQ(m_pSubscriberManager->m_objListeners.GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListener)
{
    // Test1 : Add success
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pSubscriberManager->AddListener(piListener1);
    m_pSubscriberManager->AddListener(piListener2);
    m_pSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(m_pSubscriberManager->m_objListeners.GetSize(), 3);

    // Test2 : duplicated listener
    m_pSubscriberManager->AddListener(piListener1);
    m_pSubscriberManager->AddListener(piListener2);
    m_pSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(m_pSubscriberManager->m_objListeners.GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListener_ListenerIsNull)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pSubscriberManager->AddListener(piListener1);
    m_pSubscriberManager->AddListener(piListener2);
    m_pSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(m_pSubscriberManager->m_objListeners.GetSize(), 3);

    m_pSubscriberManager->RemoveListener(IMS_NULL);
    m_pSubscriberManager->RemoveListener(IMS_NULL);
    m_pSubscriberManager->RemoveListener(IMS_NULL);
    EXPECT_EQ(m_pSubscriberManager->m_objListeners.GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListener)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener4 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener5 = new MockIAosSubscriberManagerListener();

    m_pSubscriberManager->AddListener(piListener1);
    m_pSubscriberManager->AddListener(piListener2);
    m_pSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(m_pSubscriberManager->m_objListeners.GetSize(), 3);

    // Test1 : Not matched listener
    m_pSubscriberManager->RemoveListener(piListener4);
    m_pSubscriberManager->RemoveListener(piListener5);
    EXPECT_EQ(m_pSubscriberManager->m_objListeners.GetSize(), 3);

    // Test2 : Remove success
    m_pSubscriberManager->RemoveListener(piListener3);
    EXPECT_EQ(m_pSubscriberManager->m_objListeners.GetSize(), 2);
    m_pSubscriberManager->RemoveListener(piListener2);
    EXPECT_EQ(m_pSubscriberManager->m_objListeners.GetSize(), 1);
    m_pSubscriberManager->RemoveListener(piListener1);
    EXPECT_EQ(m_pSubscriberManager->m_objListeners.GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListenerForMonitor_ListenerIsNull)
{
    m_pSubscriberManager->AddListenerForMonitor(IMS_NULL);
    EXPECT_EQ(m_pSubscriberManager->m_objMonitorListeners.GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListenerForMonitor)
{
    // Test1 : Add success
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pSubscriberManager->AddListenerForMonitor(piListener1);
    m_pSubscriberManager->AddListenerForMonitor(piListener2);
    m_pSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(m_pSubscriberManager->m_objMonitorListeners.GetSize(), 3);

    // Test2 : duplicated listener
    m_pSubscriberManager->AddListenerForMonitor(piListener1);
    m_pSubscriberManager->AddListenerForMonitor(piListener2);
    m_pSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(m_pSubscriberManager->m_objMonitorListeners.GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListenerForMonitor_ListenerIsNull)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    m_pSubscriberManager->AddListenerForMonitor(piListener1);
    m_pSubscriberManager->AddListenerForMonitor(piListener2);
    m_pSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(m_pSubscriberManager->m_objMonitorListeners.GetSize(), 3);

    m_pSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    m_pSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    m_pSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    EXPECT_EQ(m_pSubscriberManager->m_objMonitorListeners.GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListenerForMonitor)
{
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener4 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener5 = new MockIAosSubscriberManagerListener();

    m_pSubscriberManager->AddListenerForMonitor(piListener1);
    m_pSubscriberManager->AddListenerForMonitor(piListener2);
    m_pSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(m_pSubscriberManager->m_objMonitorListeners.GetSize(), 3);

    // Test1 : Not matched listener
    m_pSubscriberManager->RemoveListenerForMonitor(piListener4);
    m_pSubscriberManager->RemoveListenerForMonitor(piListener5);
    EXPECT_EQ(m_pSubscriberManager->m_objMonitorListeners.GetSize(), 3);

    // Test2 : Remove success
    m_pSubscriberManager->RemoveListenerForMonitor(piListener3);
    EXPECT_EQ(m_pSubscriberManager->m_objMonitorListeners.GetSize(), 2);
    m_pSubscriberManager->RemoveListenerForMonitor(piListener2);
    EXPECT_EQ(m_pSubscriberManager->m_objMonitorListeners.GetSize(), 1);
    m_pSubscriberManager->RemoveListenerForMonitor(piListener1);
    EXPECT_EQ(m_pSubscriberManager->m_objMonitorListeners.GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, GetConfiguredImpusForFake)
{
    m_pSubscriberManager->m_objPuidsForFake = m_objEmptyPuids;
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpusForFake().GetCount(), 0);
    m_pSubscriberManager->m_objPuidsForFake = m_objValidPuids;
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpusForFake().GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, GetConfiguredImpusForNormal)
{
    m_pSubscriberManager->m_objPuids = m_objEmptyPuids;
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, GetFakeImpus_SubscriberConfigIsNull)
{
    // SubscriberConfig is null
    m_pSubscriberManager->SetSubscriberConfigForFake(IMS_NULL);
    EXPECT_EQ(m_pSubscriberManager->GetFakeImpus().GetCount(), 0);
}

TEST_F(AosSubscriberManagerTest, GetFakeImpus_SubscriberConfigIsNotNull)
{
    EXPECT_EQ(m_pSubscriberManager->GetFakeImpus().GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, Init_SubscriberConfigIsNotNull)
{
    EXPECT_CALL(m_objMockISubscriberConfig, SetListener(_)).Times(1);

    m_pSubscriberManager->Init();
}

TEST_F(AosSubscriberManagerTest, IsTimerRunning_TimerIsNotRunning)
{
    m_pSubscriberManager->m_piTimerToIccLoadedWaiting = IMS_NULL;
    m_pSubscriberManager->m_piTimerToIsimRecovery = IMS_NULL;
    m_pSubscriberManager->m_piTimerToPhoneRestartRecovery = IMS_NULL;

    EXPECT_FALSE(m_pSubscriberManager->IsTimerRunning(TIMER_ICC_LOADED_WAITING));
    EXPECT_FALSE(m_pSubscriberManager->IsTimerRunning(TIMER_ISIM_RECOVERY));
    EXPECT_FALSE(m_pSubscriberManager->IsTimerRunning(TIMER_PHONE_RESTART_RECOVERY));
}

TEST_F(AosSubscriberManagerTest, IsTimerRunning_TimerIsRunning)
{
    m_pSubscriberManager->StartTimer(TIMER_ICC_LOADED_WAITING, 5000);
    m_pSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 5000);
    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 5000);

    EXPECT_TRUE(m_pSubscriberManager->IsTimerRunning(TIMER_ICC_LOADED_WAITING));
    EXPECT_TRUE(m_pSubscriberManager->IsTimerRunning(TIMER_ISIM_RECOVERY));
    EXPECT_TRUE(m_pSubscriberManager->IsTimerRunning(TIMER_PHONE_RESTART_RECOVERY));
}

TEST_F(AosSubscriberManagerTest, IsTimerRunning_TimerIsInvalid)
{
    const IMS_UINT32 TIMER_INVALID = 999;
    m_pSubscriberManager->StartTimer(TIMER_INVALID, 5000);
    EXPECT_FALSE(m_pSubscriberManager->IsTimerRunning(TIMER_INVALID));

    m_pSubscriberManager->StartTimer(TIMER_ICC_LOADED_WAITING, 5000);
    m_pSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 5000);
    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 5000);

    // Stopping an invalid timer does not affect other timers.
    m_pSubscriberManager->StopTimer(TIMER_INVALID);

    EXPECT_TRUE(m_pSubscriberManager->IsTimerRunning(TIMER_ICC_LOADED_WAITING));
    EXPECT_TRUE(m_pSubscriberManager->IsTimerRunning(TIMER_ISIM_RECOVERY));
    EXPECT_TRUE(m_pSubscriberManager->IsTimerRunning(TIMER_PHONE_RESTART_RECOVERY));
}

TEST_F(AosSubscriberManagerTest, GetIsimAt_ConfigurationNull)
{
    IMS_UINT32 isimIndex = 1;
    m_pSubscriberManager->m_nIsimIndexForImpu = isimIndex;
    EXPECT_EQ(m_pSubscriberManager->GetIsimAt(), isimIndex);
}

TEST_F(AosSubscriberManagerTest, ClearIsimRecovery)
{
    m_pSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToIsimRecovery, nullptr);

    m_pSubscriberManager->ClearIsimRecovery();
    EXPECT_EQ(m_pSubscriberManager->m_piTimerToIsimRecovery, nullptr);
}

TEST_F(AosSubscriberManagerTest, SucceedsConfigureAsDefaultWhenIsimTrueProvisioningDone)
{
    // GIVEN : ISIM is true and Provisioning is done, GetImpuFromIsim is true
    m_pSubscriberManager->SetIsim(IMS_TRUE);
    m_pSubscriberManager->m_objPuids = m_objEmptyPuids;

    EXPECT_CALL(m_objMockISubscriberConfig, IsProvisioningDone())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_NE(0, m_pSubscriberManager->m_objPuids.GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsDefaultWithoutSubscriberConfig)
{
    // GIVEN
    m_pSubscriberManager->SetIsim(IMS_TRUE);
    m_pSubscriberManager->SetSubscriberConfig(IMS_NULL);
    m_pSubscriberManager->m_objPuids = m_objEmptyPuids;

    EXPECT_CALL(m_objMockISubscriberConfig, IsProvisioningDone()).Times(0);

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->m_objPuids.GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenIsimTrueProvisioningNotDone)
{
    // GIVEN : ISIM is true and Provisioning is not done
    m_pSubscriberManager->SetIsim(IMS_TRUE);
    m_pSubscriberManager->m_objPuids = m_objEmptyPuids;
    EXPECT_CALL(m_objMockISubscriberConfig, IsProvisioningDone())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->m_objPuids.GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenFailedGetImpuFromIsim)
{
    // GIVEN : ISIM is true and Provisioning is done, GetImpuFromIsim is false
    m_pSubscriberManager->SetIsim(IMS_TRUE);

    EXPECT_CALL(m_objMockISubscriberConfig, IsProvisioningDone())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objEmptyPuids));

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->m_objPuids.GetCount());
}

TEST_F(AosSubscriberManagerTest, SucceedsConfigureAsDefaultWhenUsimAndValidTempImpu)
{
    // GIVEN : Valid PUIDs
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_NE(0, m_pSubscriberManager->m_objPuids.GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenUsimAndFailedGetTempImpu)
{
    // GIVEN : ISIM is false, USIM is true, GetTemporaryImpu is false
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);
    m_pSubscriberManager->m_objPuids = m_objEmptyPuids;
    m_pSubscriberManager->m_strTemporaryPublicUserId = AString::ConstNull();

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->m_objPuids.GetCount());
}

TEST_F(AosSubscriberManagerTest, SucceedsConfigureAsDefaultWhenConf)
{
    // GIVEN : Invalid PUIDs
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_FALSE);

    m_pSubscriberManager->m_objPuids = m_objEmptyPuids;

    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objValidPuids));

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_NE(0, m_pSubscriberManager->m_objPuids.GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenInvalidPuids)
{
    // GIVEN : Invalid PUIDs
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_FALSE);

    m_pSubscriberManager->m_objPuids = m_objEmptyPuids;

    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objEmptyPuids));

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->m_objPuids.GetCount());
}

TEST_F(AosSubscriberManagerTest, SucceedsConfigureAsFake)
{
    // GIVEN
    m_pSubscriberManager->m_objPuidsForFake = m_objEmptyPuids;

    // WHEN
    m_pSubscriberManager->ConfigureAsFake();

    // THEN
    EXPECT_NE(0, m_pSubscriberManager->m_objPuidsForFake.GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsFakeWhenSubscriberConfigFakeIsNull)
{
    // GIVEN
    m_pSubscriberManager->m_objPuidsForFake = m_objEmptyPuids;
    m_pSubscriberManager->SetSubscriberConfigForFake(IMS_NULL);

    // WHEN
    m_pSubscriberManager->ConfigureAsFake();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->m_objPuidsForFake.GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsFakeWhenGetInvalidPuids)
{
    // GIVEN
    m_pSubscriberManager->m_objPuidsForFake = m_objEmptyPuids;

    ON_CALL(m_objMockISubscriberConfigFake, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objEmptyPuids));

    // WHEN
    m_pSubscriberManager->ConfigureAsFake();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->m_objPuidsForFake.GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsFakeWhenGetInvalidImpu)
{
    // GIVEN
    m_objValidPuids.SetElementAt(AString::ConstEmpty(), 0);

    // WHEN
    m_pSubscriberManager->ConfigureAsFake();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->m_objPuidsForFake.GetCount());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_IsimIsNotSupport)
{
    // Isim is not supported
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_FALSE(m_pSubscriberManager->CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_ImpuIsEmpty)
{
    // Isim is supported, IMPU is empty
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objEmptyPuids));

    AString strEmptyImpi;
    EXPECT_CALL(m_objMockISubscriberConfig, GetPrivateUserId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strEmptyImpi));

    AString strEmptyHomeDomainName;
    EXPECT_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strEmptyHomeDomainName));

    EXPECT_FALSE(m_pSubscriberManager->CheckIsimValues());
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

    EXPECT_FALSE(m_pSubscriberManager->CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_InvalidImpi)
{
    // Isim is supported, IMPU is not empty, Invalid IMPI
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objValidPuids));

    AString strEmptyImpi;
    EXPECT_CALL(m_objMockISubscriberConfig, GetPrivateUserId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strEmptyImpi));

    AString strHomeDomainName = "HOME_DOMAIN_NAME";
    EXPECT_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strHomeDomainName));

    EXPECT_FALSE(m_pSubscriberManager->CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_InvalidHomeDomainName)
{
    // Isim is supported, IMPU is not empty, Invalid HomeDomainName
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objValidPuids));

    AString strImpi = "IMPI";
    EXPECT_CALL(m_objMockISubscriberConfig, GetPrivateUserId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strImpi));

    AString strEmptyHomeDomainName;
    EXPECT_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strEmptyHomeDomainName));

    EXPECT_FALSE(m_pSubscriberManager->CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, CheckIsimValues_ReturnTrue)
{
    // Isim is supported, IMPU is not empty, Valid IMPI, Valid HomeDomainName
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(m_objValidPuids));

    AString strImpi = "IMPI";
    EXPECT_CALL(m_objMockISubscriberConfig, GetPrivateUserId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strImpi));

    AString strHomeDomainName = "HOME_DOMAIN_NAME";
    EXPECT_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strHomeDomainName));

    EXPECT_TRUE(m_pSubscriberManager->CheckIsimValues());
}

TEST_F(AosSubscriberManagerTest, FailedGetImpuFromIsimWhenGetEmptyPuids)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objEmptyPuids));

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->GetImpuFromIsim(m_objOutPuids);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsGetImpuWhenLimitedAdminSmsModeAndOneValidImpu)
{
    // GIVEN
    AStringArray objOnePuid;
    objOnePuid.AddElement(AString("sip:user1@ims.com"));

    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds()).WillByDefault(ReturnRef(objOnePuid));

    m_pSubscriberManager->m_bSupportLimitedAdminSmsMode = IMS_TRUE;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->GetImpuFromIsim(m_objOutPuids);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsGetImpuWhenLimitedAdminSmsModeAndInvalidPrimaryImpu)
{
    // GIVEN
    AStringArray objInvalidPuids;
    objInvalidPuids.AddElement(AString("INVALID_IMPU1"));
    objInvalidPuids.AddElement(AString("INVALID_IMPU2"));
    objInvalidPuids.AddElement(AString("INVALID_IMPU3"));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objInvalidPuids));

    m_pSubscriberManager->m_bSupportLimitedAdminSmsMode = IMS_TRUE;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->GetImpuFromIsim(m_objOutPuids);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsGetImpuWhenSipImpuAndPhoneNumberIsGreaterThenMsisdn)
{
    // GIVEN
    AStringArray objSipPuids;
    objSipPuids.AddElement(AString("sip:1234567891@ims.com"));
    objSipPuids.AddElement(AString("sip:1234567892@ims.com"));
    objSipPuids.AddElement(AString("sip:1234567893@ims.com"));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objSipPuids));

    m_pSubscriberManager->m_bSupportLimitedAdminSmsMode = IMS_TRUE;

    // strPhoneNumber.GetLength() > USIM_MSISDN_LENGTH
    m_pSubscriberManager->m_strPhoneNumber = AString("1231234567892");

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->GetImpuFromIsim(m_objOutPuids);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsGetImpuWhenSipImpuAndPhoneNumberIsLessThenMsisdn)
{
    // GIVEN
    AStringArray objSipPuids;
    objSipPuids.AddElement(AString("sip:1234567891@ims.com"));
    objSipPuids.AddElement(AString("sip:1234567892@ims.com"));
    objSipPuids.AddElement(AString("sip:1234567893@ims.com"));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objSipPuids));

    m_pSubscriberManager->m_bSupportLimitedAdminSmsMode = IMS_TRUE;

    // strPhoneNumber.GetLength() > USIM_MSISDN_LENGTH
    m_pSubscriberManager->m_strPhoneNumber = AString("1234567892");

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->GetImpuFromIsim(m_objOutPuids);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsGetImpuWhenSecondImpuIsNotSip)
{
    // GIVEN
    AStringArray objSipPuids;
    objSipPuids.AddElement(AString("sip:1234567891@ims.com"));
    objSipPuids.AddElement(AString("INVALID_IMPU"));
    objSipPuids.AddElement(AString("sip:1234567893@ims.com"));

    EXPECT_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objSipPuids));

    m_pSubscriberManager->m_bSupportLimitedAdminSmsMode = IMS_TRUE;
    m_pSubscriberManager->m_strPhoneNumber = AString("1231234567892");

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->GetImpuFromIsim(m_objOutPuids);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ImpuIsInvalid)
{
    m_pSubscriberManager->m_strTemporaryPublicUserId = AString::ConstNull();

    EXPECT_TRUE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ImpiIsInvalid)
{
    m_pSubscriberManager->m_strTemporaryPrivateUserId = AString::ConstNull();

    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_TRUE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_HomeDomainNameIsInvalid)
{
    m_pSubscriberManager->m_strTemporaryHomeDomainName = AString::ConstNull();

    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_TRUE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    AStringArray objPuids;
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_Impu)
{
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_Impi)
{
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_HomeDomainName)
{
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_PhoneContext)
{
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_AuthUserName)
{
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_AuthRealm)
{
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateFailed_ServerScscf)
{
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest,
        GetTemporaryImpu_ConfigurableUpdateFailed_WriteProvisioningSubscriber)
{
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateSuccessWithWritable)
{
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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
    EXPECT_TRUE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, GetTemporaryImpu_ConfigurableUpdateSuccessWithoutWritable)
{
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    EXPECT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    // Set IConfigurable
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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
    EXPECT_TRUE(m_pSubscriberManager->GetTemporaryImpu(objPuids, IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, FailedUpdateImsiWhenSubsInfoIsNull)
{
    // GIVEN
    m_objPhoneInfoService.SetSubscriberInfo(IMS_NULL);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->UpdateImsi();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsUpdateImsIdentity)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .WillByDefault(Return(&m_objMockIConfigurable));

    ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, _))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, _))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported()).Times(1).WillOnce(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockISubscriberConfig, IsUsimSupported()).Times(1).WillOnce(Return(IMS_TRUE));

    // WHEN
    IMS_BOOL bResult =
            m_pSubscriberManager->UpdateImsIdentity(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseWhenFailedUpdateImsIdentityWithoutIConfigurable)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, GetConfigurable()).WillByDefault(Return(nullptr));

    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported()).Times(0);
    EXPECT_CALL(m_objMockISubscriberConfig, IsUsimSupported()).Times(0);

    // WHEN
    IMS_BOOL bResult =
            m_pSubscriberManager->UpdateImsIdentity(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseWhenFailedUpdateImsIdentityWithIsimIdentity)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .WillByDefault(Return(&m_objMockIConfigurable));

    ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, _))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported()).Times(0);
    EXPECT_CALL(m_objMockISubscriberConfig, IsUsimSupported()).Times(0);

    // WHEN
    IMS_BOOL bResult =
            m_pSubscriberManager->UpdateImsIdentity(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseWhenFailedUpdateImsIdentityWithUsimIdentity)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .WillByDefault(Return(&m_objMockIConfigurable));

    ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, _))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, _))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported()).Times(0);
    EXPECT_CALL(m_objMockISubscriberConfig, IsUsimSupported()).Times(0);

    // WHEN
    IMS_BOOL bResult =
            m_pSubscriberManager->UpdateImsIdentity(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    // THEN
    EXPECT_FALSE(bResult);
}

// ProcessFallback
TEST_F(AosSubscriberManagerTest, ProcessFallback_UpdateImsIdentityReturnFalse)
{
    EXPECT_FALSE(m_pSubscriberManager->ProcessFallback(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, ProcessFallback_UpdateImsIdentityReturnTrue)
{
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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

    EXPECT_TRUE(m_pSubscriberManager->ProcessFallback(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, FailedProcessFallbackToImsiBasedIsimWhenNotSupportFallBack)
{
    // GIVEN
    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;

    // WHEN
    IMS_BOOL bResult =
            m_pSubscriberManager->ProcessFallbackToImsiBasedIsim(IConfigurable::CP_I_IMPU_0);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessFallbackToImsiBasedIsimWhenInvalidCpi)
{
    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;

    ON_CALL(m_objMockIConfigurable, Update(_, _)).WillByDefault(Return(IMS_TRUE));

    // WHEN
    IMS_BOOL bResult =
            m_pSubscriberManager->ProcessFallbackToImsiBasedIsim(IConfigurable::CP_I_BASE);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsProcessFallbackToImsiBasedIsimWithImpu)
{
    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;

    ON_CALL(m_objMockIConfigurable, Update(_, _)).WillByDefault(Return(IMS_TRUE));

    // WHEN
    IMS_BOOL bResult =
            m_pSubscriberManager->ProcessFallbackToImsiBasedIsim(IConfigurable::CP_I_IMPU_0);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsProcessFallbackToImsiBasedIsimWithImpi)
{
    // GIVEN
    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;

    ON_CALL(m_objMockIConfigurable, Update(_, _)).WillByDefault(Return(IMS_TRUE));

    // WHEN
    IMS_BOOL bResult =
            m_pSubscriberManager->ProcessFallbackToImsiBasedIsim(IConfigurable::CP_I_IMPI);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsProcessFallbackToImsiBasedIsimWithHdn)
{
    // GIVEN
    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;

    ON_CALL(m_objMockIConfigurable, Update(_, _)).WillByDefault(Return(IMS_TRUE));

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessFallbackToImsiBasedIsim(
            IConfigurable::CP_I_HOME_DOMAIN_NAME);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenTimerIsRunning)
{
    // GIVEN
    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenIsNotReady)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_FALSE);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenSupportIsim)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenNotSupportUsim)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_FALSE);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenInvalidImpus)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);
    m_pSubscriberManager->m_objPuids = m_objEmptyPuids;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenInvalidTempImpus)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);

    m_pSubscriberManager->m_objPuids = m_objValidPuids;

    // Setting for CreateTemporaryPublicUserId
    m_objStrTestImsi = AString::ConstNull();
    m_objStrTestMcc = AString::ConstNull();
    m_objStrTestMnc = AString::ConstNull();
    SetUpDefaultISubscriberInfo();

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenEqualsTempImpu)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);

    m_objValidPuids.SetElementAt(AString("sip:123456789@ims.mnc456.mcc123.3gppnetwork.org"), 0);
    m_pSubscriberManager->m_objPuids = m_objValidPuids;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsProcessPhoneNumberAvailable)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);

    m_pSubscriberManager->m_objPuids = m_objValidPuids;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable(
            IMS_FALSE, PhoneNumberState::SIM_LOADED);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, ProcessIsimRecovery_SupportIsimImsiFallback)
{
    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    m_pSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToIsimRecovery, nullptr);

    AosProvider::GetInstance()->SetService(static_cast<IAosService*>(&m_objMockIAosService), 0);
    EXPECT_CALL(m_objMockIAosService, NotifyAosIsimState(AosIsimState::INVALID)).Times(1);

    m_pSubscriberManager->ProcessIsimRecovery();
    EXPECT_EQ(m_pSubscriberManager->m_piTimerToIsimRecovery, nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessIsimRecovery_TimerIsRunning)
{
    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);

    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM]");

    m_pSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToIsimRecovery, nullptr);

    AosProvider::GetInstance()->SetService(static_cast<IAosService*>(&m_objMockIAosService), 0);
    EXPECT_CALL(m_objMockIAosService, NotifyAosIsimState(AosIsimState::INVALID)).Times(0);

    m_pSubscriberManager->ProcessIsimRecovery();
    EXPECT_NE(m_pSubscriberManager->m_piTimerToIsimRecovery, nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneRestarted)
{
    EXPECT_EQ(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);

    m_pSubscriberManager->ProcessPhoneRestarted();
    EXPECT_NE(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessIsimRecoveryTimerExpired)
{
    m_pSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToIsimRecovery, nullptr);

    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

    EXPECT_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIBER_ALL, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    m_pSubscriberManager->ProcessIsimRecoveryTimerExpired();
    EXPECT_EQ(m_pSubscriberManager->m_piTimerToIsimRecovery, nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneRestartRecoveryTimerExpired_IsUsimTrue)
{
    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);

    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);

    m_pSubscriberManager->ProcessPhoneRestartRecoveryTimerExpired();
    EXPECT_EQ(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneRestartRecoveryTimerExpired_RefreshStarted)
{
    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);

    m_pSubscriberManager->SetIsim(IMS_TRUE);
    m_pSubscriberManager->SetUsim(IMS_FALSE);
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->m_bIsRefreshStarted = IMS_TRUE;

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    // ProcessFallback(IMS_TRUE) and UpdateImsIdentity(IMS_TRUE) should return IMS_TRUE.
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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

    m_pSubscriberManager->ProcessPhoneRestartRecoveryTimerExpired();
    EXPECT_EQ(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);
}

TEST_F(AosSubscriberManagerTest, ProcessPhoneRestartRecoveryTimerExpired_ProcessFallback)
{
    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);

    m_pSubscriberManager->SetIsim(IMS_TRUE);
    m_pSubscriberManager->SetUsim(IMS_FALSE);
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->m_bIsRefreshStarted = IMS_TRUE;

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    m_pSubscriberManager->ProcessPhoneRestartRecoveryTimerExpired();
    EXPECT_EQ(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);
}

TEST_F(AosSubscriberManagerTest, IsPrimaryImpuValid_PhoneNumberLengthZero)
{
    EXPECT_FALSE(m_pSubscriberManager->IsPrimaryImpuValid(m_objValidPuids));
}

TEST_F(AosSubscriberManagerTest, NotifyState)
{
    MockIAosSubscriberManagerListener objListener1;
    MockIAosSubscriberManagerListener objListener2;
    MockIAosSubscriberManagerListener objListener3;

    EXPECT_CALL(objListener1, AosSubscriberManager_NotifyState(_)).Times(2);
    EXPECT_CALL(objListener2, AosSubscriberManager_NotifyState(_)).Times(2);
    EXPECT_CALL(objListener3, AosSubscriberManager_NotifyState(_)).Times(2);
    m_pSubscriberManager->AddListener(static_cast<IAosSubscriberManagerListener*>(&objListener1));
    m_pSubscriberManager->AddListener(static_cast<IAosSubscriberManagerListener*>(&objListener2));
    m_pSubscriberManager->AddListener(static_cast<IAosSubscriberManagerListener*>(&objListener3));

    m_pSubscriberManager->NotifyState(IAosSubscriber::READY);
    m_pSubscriberManager->NotifyState(IAosSubscriber::NOT_READY);
}

TEST_F(AosSubscriberManagerTest, NotifyMonitorState)
{
    MockIAosSubscriberManagerListener objListener1;
    MockIAosSubscriberManagerListener objListener2;
    MockIAosSubscriberManagerListener objListener3;

    EXPECT_CALL(objListener1, AosSubscriberManager_NotifyState(_)).Times(2);
    EXPECT_CALL(objListener2, AosSubscriberManager_NotifyState(_)).Times(2);
    EXPECT_CALL(objListener3, AosSubscriberManager_NotifyState(_)).Times(2);
    m_pSubscriberManager->AddListenerForMonitor(
            static_cast<IAosSubscriberManagerListener*>(&objListener1));
    m_pSubscriberManager->AddListenerForMonitor(
            static_cast<IAosSubscriberManagerListener*>(&objListener2));
    m_pSubscriberManager->AddListenerForMonitor(
            static_cast<IAosSubscriberManagerListener*>(&objListener3));

    m_pSubscriberManager->NotifyMonitorState(IAosSubscriber::READY);
    m_pSubscriberManager->NotifyMonitorState(IAosSubscriber::NOT_READY);
}

TEST_F(AosSubscriberManagerTest, IsSipUri)
{
    // Test1 : Impu length is zero
    EXPECT_FALSE(m_pSubscriberManager->IsSipUri(AString::ConstNull()));

    // Test2 : Impu scheme is Sip
    EXPECT_TRUE(m_pSubscriberManager->IsSipUri(AString("sip:user1@ims.com")));

    // Test3 : Impu scheme is Sips
    EXPECT_TRUE(m_pSubscriberManager->IsSipUri(AString("sips:user1@ims.com")));
}

TEST_F(AosSubscriberManagerTest, SucceedsUpdateNConfiguration)
{
    // GIVEN
    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);

    ImsVector<IMS_SINT32> objUpdatedImsIdentityPriority;
    objUpdatedImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objUpdatedImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->m_nIsimIndexForImpu = 0;
    m_pSubscriberManager->m_bSupportLimitedAdminSmsMode = IMS_FALSE;
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;

    ON_CALL(m_objMockIAosNConfiguration, GetIsimIndexForImpu()).WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetImsIdentityPriority())
            .WillByDefault(ReturnRef(objUpdatedImsIdentityPriority));
    ON_CALL(m_objMockIAosNConfiguration, RemoveListener(_)).WillByDefault(Return());
    m_pSubscriberManager->m_piNConfig = &m_objMockIAosNConfiguration;

    m_pSubscriberManager->SetSubscriberConfig(IMS_NULL);

    // WHEN
    m_pSubscriberManager->NConfiguration_NotifyConfigChanged();

    // THEN
    EXPECT_FALSE(m_pSubscriberManager->IsProvisioned());
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
}

TEST_F(AosSubscriberManagerTest, SucceedsUpdateNConfigurationWhenPrioritySizeIsSame)
{
    // GIVEN
    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    ImsVector<IMS_SINT32> objUpdatedImsIdentityPriority;
    objUpdatedImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objUpdatedImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->m_nIsimIndexForImpu = 0;
    m_pSubscriberManager->m_bSupportLimitedAdminSmsMode = IMS_FALSE;
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;

    ON_CALL(m_objMockIAosNConfiguration, GetIsimIndexForImpu()).WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetImsIdentityPriority())
            .WillByDefault(ReturnRef(objUpdatedImsIdentityPriority));
    ON_CALL(m_objMockIAosNConfiguration, RemoveListener(_)).WillByDefault(Return());

    m_pSubscriberManager->m_piNConfig = &m_objMockIAosNConfiguration;

    m_pSubscriberManager->SetSubscriberConfig(IMS_NULL);

    // WHEN
    m_pSubscriberManager->NConfiguration_NotifyConfigChanged();

    // THEN
    EXPECT_FALSE(m_pSubscriberManager->IsProvisioned());
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
}

TEST_F(AosSubscriberManagerTest, FailedUpdateNConfigurationWhenNConfigIsNull)
{
    // GIVEN
    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->m_piNConfig = IMS_NULL;

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    m_pSubscriberManager->m_nIsimIndexForImpu = 0;
    m_pSubscriberManager->m_bSupportLimitedAdminSmsMode = IMS_FALSE;
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;

    m_pSubscriberManager->SetSubscriberConfig(IMS_NULL);

    // WHEN
    m_pSubscriberManager->NConfiguration_NotifyConfigChanged();

    // THEN
    EXPECT_TRUE(m_pSubscriberManager->IsProvisioned());
    EXPECT_NE(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
}

TEST_F(AosSubscriberManagerTest, FailedUpdateNConfigurationWhenSameConfiguration)
{
    // GIVEN
    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->m_nIsimIndexForImpu = 0;
    m_pSubscriberManager->m_bSupportLimitedAdminSmsMode = IMS_FALSE;
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;

    ON_CALL(m_objMockIAosNConfiguration, GetIsimIndexForImpu()).WillByDefault(Return(0));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetImsIdentityPriority())
            .WillByDefault(ReturnRef(objImsIdentityPriority));
    ON_CALL(m_objMockIAosNConfiguration, RemoveListener(_)).WillByDefault(Return());

    m_pSubscriberManager->m_piNConfig = &m_objMockIAosNConfiguration;

    m_pSubscriberManager->SetSubscriberConfig(IMS_NULL);

    // WHEN
    m_pSubscriberManager->NConfiguration_NotifyConfigChanged();

    // THEN
    EXPECT_TRUE(m_pSubscriberManager->IsProvisioned());
    EXPECT_NE(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_InitCompleted_IsimUsimNotSupport)
{
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));

    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    EXPECT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));

    m_pSubscriberManager->SubscriberConfig_InitCompleted();
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
    EXPECT_FALSE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_RefreshCompleted_IsimUsimNotSupport)
{
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));

    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    EXPECT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));

    m_pSubscriberManager->SubscriberConfig_RefreshCompleted();
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
    EXPECT_FALSE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_RefreshStarted)
{
    EXPECT_FALSE(m_pSubscriberManager->IsRefreshStarted());

    m_pSubscriberManager->SubscriberConfig_RefreshStarted();
    EXPECT_TRUE(m_pSubscriberManager->IsRefreshStarted());
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_NotifyError_IsimUsimNotSupport)
{
    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    EXPECT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));

    m_pSubscriberManager->SubscriberConfig_NotifyError(0);
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
    EXPECT_FALSE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_NotifyError_TimerIsRunning)
{
    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    EXPECT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);

    m_pSubscriberManager->SubscriberConfig_NotifyError(0);

    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    EXPECT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_NotifyError_ProcessFallback_False)
{
    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    EXPECT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    m_pSubscriberManager->SubscriberConfig_NotifyError(0);

    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
    EXPECT_FALSE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SubscriberConfig_NotifyError_ProcessFallback_True)
{
    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    EXPECT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    // ProcessFallback(IMS_TRUE) and UpdateImsIdentity(IMS_TRUE) should return IMS_TRUE.
    EXPECT_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .Times(1)
            .WillRepeatedly(Return(&m_objMockIConfigurable));

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

    m_pSubscriberManager->SubscriberConfig_NotifyError(0);

    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
    EXPECT_FALSE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, ConfigUpdate_NotifyUpdate)
{
    // Currently, there is no logic that requires tests.
    m_pSubscriberManager->ConfigUpdate_NotifyUpdate(0, AString::ConstNull(), AString::ConstNull());
}

TEST_F(AosSubscriberManagerTest, Timer_TimerExpired_TimerIsNull)
{
    m_pSubscriberManager->StartTimer(TIMER_ICC_LOADED_WAITING, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToIccLoadedWaiting, nullptr);

    m_pSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToIccLoadedWaiting, nullptr);

    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToIccLoadedWaiting, nullptr);

    // Expiration of an invalid timer does not affect other timers.
    m_pSubscriberManager->Timer_TimerExpired(IMS_NULL);

    EXPECT_NE(m_pSubscriberManager->m_piTimerToIccLoadedWaiting, nullptr);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToIsimRecovery, nullptr);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);
}

TEST_F(AosSubscriberManagerTest, Timer_TimerExpired_IccLoadedWaiting)
{
    m_pSubscriberManager->StartTimer(TIMER_ICC_LOADED_WAITING, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToIccLoadedWaiting, nullptr);

    m_pSubscriberManager->Timer_TimerExpired(m_pSubscriberManager->m_piTimerToIccLoadedWaiting);
    EXPECT_EQ(m_pSubscriberManager->m_piTimerToIccLoadedWaiting, nullptr);
}

TEST_F(AosSubscriberManagerTest, Timer_TimerExpired_IsimRecovery)
{
    m_pSubscriberManager->StartTimer(TIMER_ISIM_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToIsimRecovery, nullptr);

    m_pSubscriberManager->Timer_TimerExpired(m_pSubscriberManager->m_piTimerToIsimRecovery);
    EXPECT_EQ(m_pSubscriberManager->m_piTimerToIsimRecovery, nullptr);
}

TEST_F(AosSubscriberManagerTest, Timer_TimerExpired_PhoneRestartRecovery)
{
    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);

    m_pSubscriberManager->Timer_TimerExpired(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery);
    EXPECT_EQ(m_pSubscriberManager->m_piTimerToPhoneRestartRecovery, nullptr);
}

TEST_F(AosSubscriberManagerTest, ServicePhone_PhoneNumberStateChanged_IsNotReady)
{
    // GIVEN
    m_pSubscriberManager->m_objPuids = m_objValidPuids;
    m_pSubscriberManager->SetProvisioned(IMS_FALSE);
    m_pSubscriberManager->SetSubscriberConfig(IMS_NULL);

    // WHEN
    m_pSubscriberManager->ServicePhone_PhoneNumberStateChanged(
            IMS_FALSE, PhoneNumberState::SIM_LOADED);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedLoaded)
{
    m_pSubscriberManager->ServicePhone_IsimStateChanged(IsimState::LOADED);
    EXPECT_TRUE(m_pSubscriberManager->ProcessIsimStateChange(IsimState::LOADED));
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedRefreshStarted)
{
    m_pSubscriberManager->ServicePhone_IsimStateChanged(IsimState::REFRESH_STARTED);
    EXPECT_TRUE(m_pSubscriberManager->ProcessIsimStateChange(IsimState::REFRESH_STARTED));
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedRefreshCompleted)
{
    m_pSubscriberManager->ServicePhone_IsimStateChanged(IsimState::REFRESH_COMPLETED);
    EXPECT_TRUE(m_pSubscriberManager->ProcessIsimStateChange(IsimState::REFRESH_COMPLETED));
}

TEST_F(AosSubscriberManagerTest, ServicePhone_IsimStateChangedReturnFalse)
{
    m_pSubscriberManager->ServicePhone_IsimStateChanged(IsimState::NOT_READY);
    EXPECT_FALSE(m_pSubscriberManager->ProcessIsimStateChange(IsimState::NOT_READY));
}

TEST_F(AosSubscriberManagerTest, IdentityPriorityToString)
{
    ImsVector<IMS_SINT32> objImsIdentityPriority;

    // Test1 : IdentityPriority is empty
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[Empty]");

    // Test2 : [ISIM][USIM]
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM][USIM]");

    // Test2 : [ISIM][USIM][CONF]
    objImsIdentityPriority.Clear();
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF);

    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM][USIM][CONF]");

    // Test3 : [ISIM][ISIM_IMSI]
    objImsIdentityPriority.Clear();
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");

    // Test4 : [USIM][ISIM]
    objImsIdentityPriority.Clear();
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[USIM][ISIM]");

    // Test4 : [CONF]
    objImsIdentityPriority.Clear();
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF);
    m_pSubscriberManager->m_objImsIdentityPriority = objImsIdentityPriority;
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[CONF]");
}

TEST_F(AosSubscriberManagerTest, PrintIdentity)
{
    EXPECT_STREQ(
            m_pSubscriberManager->PrintIdentity(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM),
            "ISIM");
    EXPECT_STREQ(
            m_pSubscriberManager->PrintIdentity(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM),
            "USIM");
    EXPECT_STREQ(m_pSubscriberManager->PrintIdentity(
                         CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI),
            "ISIM_IMSI");
    EXPECT_STREQ(
            m_pSubscriberManager->PrintIdentity(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF),
            "CONF");
}

TEST_F(AosSubscriberManagerTest, UpdateEventToString)
{
    EXPECT_STREQ(
            m_pSubscriberManager->UpdateEventToString(IConfigurable::CP_I_IMPU_0), "CP_I_IMPU_0");
    EXPECT_STREQ(m_pSubscriberManager->UpdateEventToString(IConfigurable::CP_I_IMPI), "CP_I_IMPI");
    EXPECT_STREQ(m_pSubscriberManager->UpdateEventToString(IConfigurable::CP_I_HOME_DOMAIN_NAME),
            "CP_I_HOME_DOMAIN_NAME");
    EXPECT_STREQ(m_pSubscriberManager->UpdateEventToString(IConfigurable::CP_I_PHONE_CONTEXT),
            "CP_I_PHONE_CONTEXT");
    EXPECT_STREQ(m_pSubscriberManager->UpdateEventToString(IConfigurable::CP_I_AUTH_USERNAME),
            "CP_I_AUTH_USERNAME");
    EXPECT_STREQ(m_pSubscriberManager->UpdateEventToString(IConfigurable::CP_I_AUTH_REALM),
            "CP_I_AUTH_REALM");
    EXPECT_STREQ(m_pSubscriberManager->UpdateEventToString(IConfigurable::CP_I_SERVER_SCSCF),
            "CP_I_SERVER_SCSCF");
    EXPECT_STREQ(m_pSubscriberManager->UpdateEventToString(
                         IConfigurable::CP_I_WRITE_PROVISIONING_SUBSCRIBER),
            "CP_I_WRITE_PROVISIONING_SUBSCRIBER");
    EXPECT_STREQ(m_pSubscriberManager->UpdateEventToString(
                         IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM),
            "CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM");
    EXPECT_STREQ(m_pSubscriberManager->UpdateEventToString(
                         IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM),
            "CP_I_SUBSCRIPTION_ATTRIBUTE_USIM");
    EXPECT_STREQ(m_pSubscriberManager->UpdateEventToString(1000), "__INVALID__");
}

TEST_F(AosSubscriberManagerTest, TimerToString)
{
    EXPECT_STREQ(m_pSubscriberManager->TimerToString(TIMER_ICC_LOADED_WAITING),
            "TIMER_ICC_LOADED_WAITING");
    EXPECT_STREQ(m_pSubscriberManager->TimerToString(TIMER_ISIM_RECOVERY), "TIMER_ISIM_RECOVERY");
    EXPECT_STREQ(m_pSubscriberManager->TimerToString(TIMER_PHONE_RESTART_RECOVERY),
            "TIMER_PHONE_RESTART_RECOVERY");
    EXPECT_STREQ(m_pSubscriberManager->TimerToString(999), "__INVALID__");
}

TEST_F(AosSubscriberManagerTest, StateToString)
{
    EXPECT_STREQ(m_pSubscriberManager->StateToString(IAosSubscriber::NOT_READY), "NOT_READY");
    EXPECT_STREQ(m_pSubscriberManager->StateToString(IAosSubscriber::READY), "READY");
    EXPECT_STREQ(m_pSubscriberManager->StateToString(IAosSubscriber::REFRESH_STARTED),
            "REFRESH_STARTED");
    EXPECT_STREQ(m_pSubscriberManager->StateToString(IAosSubscriber::REFRESH_COMPLETED),
            "REFRESH_COMPLETED");
    EXPECT_STREQ(
            m_pSubscriberManager->StateToString(IAosSubscriber::REFRESH_FAILED), "REFRESH_FAILED");
    EXPECT_STREQ(m_pSubscriberManager->StateToString(100), "INVALID");
}
