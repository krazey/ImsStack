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
const IMS_UINT32 TIMER_PHONE_RESTART_RECOVERY = 101;

#define DECLARE_USING(Base)                              \
    using Base::CheckIsimValues;                         \
    using Base::ConfigUpdate_NotifyUpdate;               \
    using Base::ConfigureAsDefault;                      \
    using Base::ConfigureAsFake;                         \
    using Base::GetIsimAt;                               \
    using Base::IdentityPriorityToString;                \
    using Base::Init;                                    \
    using Base::IsPrimaryImpuValid;                      \
    using Base::IsProvisioned;                           \
    using Base::IsRefreshStarted;                        \
    using Base::IsSipUri;                                \
    using Base::IsTimerRunning;                          \
    using Base::NConfiguration_NotifyConfigChanged;      \
    using Base::NotifyMonitorState;                      \
    using Base::NotifyState;                             \
    using Base::PrintIdentity;                           \
    using Base::ProcessIsimStateChange;                  \
    using Base::ProcessSimStateChange;                   \
    using Base::ProcessPhoneNumberAvailable;             \
    using Base::ProcessPhoneRestarted;                   \
    using Base::ProcessPhoneRestartRecoveryTimerExpired; \
    using Base::ReconfigureFallback;                     \
    using Base::ServicePhone_IsimStateChanged;           \
    using Base::ServicePhone_SimStateChanged;            \
    using Base::ServicePhone_PhoneNumberStateChanged;    \
    using Base::SetIsim;                                 \
    using Base::SetProvisioned;                          \
    using Base::SetUsim;                                 \
    using Base::StateToString;                           \
    using Base::StartTimer;                              \
    using Base::SubscriberConfig_InitCompleted;          \
    using Base::SubscriberConfig_NotifyError;            \
    using Base::SubscriberConfig_RefreshCompleted;       \
    using Base::SubscriberConfig_RefreshStarted;         \
    using Base::Timer_TimerExpired;                      \
    using Base::TimerToString;                           \
    using Base::UpdateImpuFromIsim;                      \
    using Base::UpdateImsi;                              \
    using Base::UpdateSubscriberInfoWithTempImpu;

class TestAosSubscriberManager : public AosSubscriberManager
{
public:
    AString m_strPhoneNumber;
    AString m_strTemporaryPublicUserId;
    AString m_strTemporaryPrivateUserId;
    AString m_strTemporaryHomeDomainName;

public:
    DECLARE_USING(AosSubscriberManager)

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

    inline IMS_BOOL GetUsimFallback() { return m_bUsimFallback; }

    inline void SetUsimFallback(IN IMS_BOOL bUsimFallback) { m_bUsimFallback = bUsimFallback; }

    inline IMS_UINT32 GetNotifyState() { return m_nNotifyState; }

    inline void SetNotifyState(IMS_UINT32 state) { m_nNotifyState = state; }

    inline ImsList<IAosSubscriberManagerListener*> getListeners() { return m_objListeners; }

    inline ImsList<IAosSubscriberManagerListener*> getMonitorListeners()
    {
        return m_objMonitorListeners;
    }

    inline void SetPuids(IN const AStringArray& objPuids) { m_objPuids = objPuids; }

    inline void SetOrderedPuids(IN const AStringArray& objPuids) { m_objOrderedPuids = objPuids; }

    inline void SetPuidsForFake(IN const AStringArray& objPuids) { m_objPuidsForFake = objPuids; }

    inline ITimer* GetTimerToIccLoadedWaiting() { return m_piTimerToIccLoadedWaiting; }

    inline void SetTimerToIccLoadedWaiting(IN ITimer* piTimer)
    {
        m_piTimerToIccLoadedWaiting = piTimer;
    }

    inline ITimer* GetTimerToPhoneRestartRecovery() { return m_piTimerToPhoneRestartRecovery; }

    inline void SetTimerToPhoneRestartRecovery(IN ITimer* piTimer)
    {
        m_piTimerToPhoneRestartRecovery = piTimer;
    }

    inline void SetNConfig(IN IAosNConfiguration* piNConfig) { m_piNConfig = piNConfig; }

    inline void SetIsimIndexForImpu(IN IMS_UINT32 nIsimIndexForImpu)
    {
        m_nIsimIndexForImpu = nIsimIndexForImpu;
    };

    inline void SetSupportLimitedAdminSmsMode(IN IMS_BOOL bSupportLimitedAdminSmsMode)
    {
        m_bSupportLimitedAdminSmsMode = bSupportLimitedAdminSmsMode;
    }

    inline void SetImsIdentityPriority(IN const ImsVector<IMS_SINT32>& objImsIdentityPriority)
    {
        m_objImsIdentityPriority = objImsIdentityPriority;
    }

    inline void SetImsiBasedUriPrioritized(IN IMS_BOOL bEnable)
    {
        m_bPrioritizeImsiBasedUri = bEnable;
    }
};

class AosSubscriberManagerTest : public ::testing::Test
{
public:
    TestAosSubscriberManager* m_pSubscriberManager;

    TestPhoneInfoService m_objPhoneInfoService;
    IAosService* m_piAosService;

    MockISubscriberConfig m_objMockISubscriberConfig;
    MockISubscriberConfig m_objMockISubscriberConfigFake;
    MockISubscriberInfo m_objMockISubscriberInfo;
    MockIConfigurable m_objMockIConfigurable;
    MockIAosService m_objMockIAosService;

    // cppcheck-suppress unusedStructMember
    MockIAosSubscriberManagerListener m_objListener1;
    // cppcheck-suppress unusedStructMember
    MockIAosSubscriberManagerListener m_objListener2;
    // cppcheck-suppress unusedStructMember
    MockIAosSubscriberManagerListener m_objListener3;
    // cppcheck-suppress unusedStructMember
    MockIAosSubscriberManagerListener m_objListener4;
    // cppcheck-suppress unusedStructMember
    MockIAosSubscriberManagerListener m_objListener5;

    // cppcheck-suppress unusedStructMember
    MockIAosNConfiguration m_objMockIAosNConfiguration;

    AStringArray m_objValidPuids;
    AStringArray m_objOrderedPuids;
    AStringArray m_objEmptyPuids;
    AStringArray m_objOutPuids;

    AString m_objStrTestImsi;
    AString m_objStrTestMcc;
    AString m_objStrTestMnc;

protected:
    void SetUp() override
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

        m_piAosService = AosProvider::GetInstance()->GetService();
        AosProvider::GetInstance()->SetService(&m_objMockIAosService);

        m_pSubscriberManager = new TestAosSubscriberManager(IMS_SLOT_0);
        ASSERT_TRUE(m_pSubscriberManager != nullptr);

        m_pSubscriberManager->SetSubscriberConfig(&m_objMockISubscriberConfig);
        m_pSubscriberManager->SetSubscriberConfigForFake(&m_objMockISubscriberConfigFake);

        m_pSubscriberManager->m_strTemporaryPublicUserId = AString("TEMP_PUID");
        m_pSubscriberManager->m_strTemporaryPrivateUserId = AString("TEMP_IMPI");
        m_pSubscriberManager->m_strTemporaryHomeDomainName = AString("TEMP_HDN");
    }

    void TearDown() override
    {
        if (m_pSubscriberManager)
        {
            delete m_pSubscriberManager;
        }

        AosProvider::GetInstance()->SetService(m_piAosService);
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

        m_objOrderedPuids.AddElement(AString("sip:user0@ims.com"));
        m_objOrderedPuids.AddElement(AString("sip:user1@ims.com"));
        m_objOrderedPuids.AddElement(AString("sip:user2@ims.com"));
        m_objOrderedPuids.AddElement(AString("sip:user3@ims.com"));
        m_objOrderedPuids.AddElement(AString("sip:user4@ims.com"));

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

TEST_F(AosSubscriberManagerTest, FailedAddListenerWhenListenerIsNull)
{
    // GIVEN
    ASSERT_EQ(m_pSubscriberManager->getListeners().GetSize(), 0);

    // WHEN
    m_pSubscriberManager->AddListener(IMS_NULL);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, SucceedsAddListener)
{
    // GIVEN
    ASSERT_EQ(m_pSubscriberManager->getListeners().GetSize(), 0);

    // WHEN
    m_pSubscriberManager->AddListener(&m_objListener1);
    m_pSubscriberManager->AddListener(&m_objListener2);
    m_pSubscriberManager->AddListener(&m_objListener3);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, FailedAddListenerWhenDuplicatedListener)
{
    // GIVEN
    m_pSubscriberManager->AddListener(&m_objListener1);
    m_pSubscriberManager->AddListener(&m_objListener2);
    m_pSubscriberManager->AddListener(&m_objListener3);

    EXPECT_EQ(m_pSubscriberManager->getListeners().GetSize(), 3);

    // WHEN
    m_pSubscriberManager->AddListener(&m_objListener1);
    m_pSubscriberManager->AddListener(&m_objListener2);
    m_pSubscriberManager->AddListener(&m_objListener3);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, FailedRemoveListenerWhenListenerIsNull)
{
    // GIVEN
    m_pSubscriberManager->AddListener(&m_objListener1);
    m_pSubscriberManager->AddListener(&m_objListener2);
    m_pSubscriberManager->AddListener(&m_objListener3);

    EXPECT_EQ(m_pSubscriberManager->getListeners().GetSize(), 3);

    // WHEN
    m_pSubscriberManager->RemoveListener(IMS_NULL);
    m_pSubscriberManager->RemoveListener(IMS_NULL);
    m_pSubscriberManager->RemoveListener(IMS_NULL);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, SucceedsRemoveListener)
{
    // GIVEN
    m_pSubscriberManager->AddListener(&m_objListener1);
    m_pSubscriberManager->AddListener(&m_objListener2);
    m_pSubscriberManager->AddListener(&m_objListener3);

    EXPECT_EQ(m_pSubscriberManager->getListeners().GetSize(), 3);

    // WHEN
    m_pSubscriberManager->RemoveListener(&m_objListener3);
    m_pSubscriberManager->RemoveListener(&m_objListener2);
    m_pSubscriberManager->RemoveListener(&m_objListener1);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, FailedRemoveListenerWhenNoMatchedListener)
{
    // GIVEN
    m_pSubscriberManager->AddListener(&m_objListener1);
    m_pSubscriberManager->AddListener(&m_objListener2);
    m_pSubscriberManager->AddListener(&m_objListener3);

    EXPECT_EQ(m_pSubscriberManager->getListeners().GetSize(), 3);

    // WHEN
    m_pSubscriberManager->RemoveListener(&m_objListener4);
    m_pSubscriberManager->RemoveListener(&m_objListener5);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, FailedAddListenerForMonitorWhenListenerIsNull)
{
    // GIVEN
    ASSERT_EQ(m_pSubscriberManager->getMonitorListeners().GetSize(), 0);

    // WHEN
    m_pSubscriberManager->AddListenerForMonitor(IMS_NULL);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getMonitorListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, SucceedsAddListenerForMonitor)
{
    // GIVEN & WHEN
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener1);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener2);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener3);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getMonitorListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, FailedAddListenerForMonitorWhenDuplicatedListener)
{
    // GIVEN
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener1);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener2);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener3);

    EXPECT_EQ(m_pSubscriberManager->getMonitorListeners().GetSize(), 3);

    // WHEN
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener1);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener2);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener3);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getMonitorListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, FailedRemoveListenerForMonitorWhenListenerIsNull)
{
    // GIVEN
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener1);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener2);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener3);

    EXPECT_EQ(m_pSubscriberManager->getMonitorListeners().GetSize(), 3);

    // WHEN
    m_pSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    m_pSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    m_pSubscriberManager->RemoveListenerForMonitor(IMS_NULL);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getMonitorListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, SucceedsRemoveListenerForMonitor)
{
    // GIVEN
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener1);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener2);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener3);

    EXPECT_EQ(m_pSubscriberManager->getMonitorListeners().GetSize(), 3);

    // WHEN
    m_pSubscriberManager->RemoveListenerForMonitor(&m_objListener3);
    m_pSubscriberManager->RemoveListenerForMonitor(&m_objListener2);
    m_pSubscriberManager->RemoveListenerForMonitor(&m_objListener1);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getMonitorListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, FailedRemoveListenerForMonitorWhenNoMatchedListener)
{
    // GIVEN
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener1);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener2);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener3);

    ASSERT_EQ(m_pSubscriberManager->getMonitorListeners().GetSize(), 3);

    // WHEN
    m_pSubscriberManager->RemoveListenerForMonitor(&m_objListener4);
    m_pSubscriberManager->RemoveListenerForMonitor(&m_objListener5);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->getMonitorListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, SucceedsGetConfiguredImpusForFake)
{
    // GIVEN
    m_pSubscriberManager->SetPuidsForFake(m_objEmptyPuids);

    ASSERT_EQ(m_pSubscriberManager->GetConfiguredImpusForFake().GetCount(), 0);

    // WHEN
    m_pSubscriberManager->SetPuidsForFake(m_objValidPuids);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpusForFake().GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, SucceedsGetConfiguredImpusForNormal)
{
    // GIVEN
    m_pSubscriberManager->SetPuids(m_objEmptyPuids);

    ASSERT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);

    // WHEN
    m_pSubscriberManager->SetPuids(m_objValidPuids);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, SucceedsGetOrderedImpus)
{
    // GIVEN
    m_pSubscriberManager->SetOrderedPuids(m_objOrderedPuids);
    IMS_UINT32 nExpectCount = m_objOrderedPuids.GetCount();

    // WHEN
    IMS_UINT32 nResultCount = m_pSubscriberManager->GetOrderedImpus().GetCount();

    // THEN
    EXPECT_EQ(nResultCount, nExpectCount);
}

TEST_F(AosSubscriberManagerTest, SucceedsGetOrderedImpusWhenOrderedImpuIsEmpty)
{
    // GIVEN
    m_pSubscriberManager->SetOrderedPuids(m_objEmptyPuids);
    m_pSubscriberManager->SetPuids(m_objValidPuids);
    IMS_UINT32 nExpectCount = m_objValidPuids.GetCount();

    // WHEN
    IMS_UINT32 nResultCount = m_pSubscriberManager->GetOrderedImpus().GetCount();

    // THEN
    EXPECT_EQ(nResultCount, nExpectCount);
}

TEST_F(AosSubscriberManagerTest, FailedGetFakeImpusWhenSubscriberConfigIsNull)
{
    // GIVEN
    m_pSubscriberManager->SetSubscriberConfigForFake(IMS_NULL);

    // WHEN
    IMS_UINT32 nCount = m_pSubscriberManager->GetFakeImpus().GetCount();

    // THEN
    EXPECT_EQ(nCount, 0);
}

TEST_F(AosSubscriberManagerTest, SucceedsGetFakeImpus)
{
    // GIVEN

    // WHEN
    IMS_UINT32 nCount = m_pSubscriberManager->GetFakeImpus().GetCount();

    // THEN
    EXPECT_EQ(nCount, 3);
}

TEST_F(AosSubscriberManagerTest, SucceedsInitWhenSubscriberConfigIsNotNull)
{
    // GIVEN
    EXPECT_CALL(m_objMockISubscriberConfig, SetListener(_, _)).Times(1);

    // WHEN
    m_pSubscriberManager->Init();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosSubscriberManagerTest, SucceedsSetProvisionedTrueWithNormalType)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_FALSE);

    ASSERT_FALSE(m_pSubscriberManager->IsReady());

    // WHEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);

    // THEN
    EXPECT_TRUE(m_pSubscriberManager->IsReady());
}

TEST_F(AosSubscriberManagerTest, SucceedsSetProvisionedFalseWithNormalType)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    ASSERT_TRUE(m_pSubscriberManager->IsReady(IMS_FALSE));

    // WHEN
    m_pSubscriberManager->SetProvisioned(IMS_FALSE);

    // THEN
    EXPECT_FALSE(m_pSubscriberManager->IsReady(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, SucceedsSetProvisionedTrueWithFakeType)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_FALSE, IAosSubscriber::FAKE);

    ASSERT_FALSE(m_pSubscriberManager->IsReady(IMS_TRUE));

    // WHEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE, IAosSubscriber::FAKE);

    // THEN
    EXPECT_TRUE(m_pSubscriberManager->IsReady(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, SucceedsSetProvisionedFalseWithFakeType)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE, IAosSubscriber::FAKE);
    ASSERT_TRUE(m_pSubscriberManager->IsReady(IMS_TRUE));

    // WHEN
    m_pSubscriberManager->SetProvisioned(IMS_FALSE, IAosSubscriber::FAKE);

    // THEN
    EXPECT_FALSE(m_pSubscriberManager->IsReady(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseWhenTimerIsNotRunning)
{
    // GIVEN
    m_pSubscriberManager->SetTimerToIccLoadedWaiting(IMS_NULL);
    m_pSubscriberManager->SetTimerToPhoneRestartRecovery(IMS_NULL);

    // WHEN
    // THEN
    EXPECT_FALSE(m_pSubscriberManager->IsTimerRunning(TIMER_ICC_LOADED_WAITING));
    EXPECT_FALSE(m_pSubscriberManager->IsTimerRunning(TIMER_PHONE_RESTART_RECOVERY));
}

TEST_F(AosSubscriberManagerTest, ReturnsTrueWhenTimerIsRunning)
{
    // GIVEN
    m_pSubscriberManager->StartTimer(TIMER_ICC_LOADED_WAITING, 5000);
    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 5000);

    // WHEN
    // THEN
    EXPECT_TRUE(m_pSubscriberManager->IsTimerRunning(TIMER_ICC_LOADED_WAITING));
    EXPECT_TRUE(m_pSubscriberManager->IsTimerRunning(TIMER_PHONE_RESTART_RECOVERY));
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseWhenTimerIsInvalid)
{
    // GIVEN
    const IMS_UINT32 TIMER_INVALID = 999;
    m_pSubscriberManager->StartTimer(TIMER_INVALID, 5000);

    // WHEN
    // THEN
    EXPECT_FALSE(m_pSubscriberManager->IsTimerRunning(TIMER_INVALID));
}

TEST_F(AosSubscriberManagerTest, SucceedsGetIsimAt)
{
    // GIVEN
    IMS_UINT32 isimIndex = 1;
    m_pSubscriberManager->SetIsimIndexForImpu(isimIndex);

    // WHEN
    IMS_UINT32 nIndex = m_pSubscriberManager->GetIsimAt();

    // THEN
    EXPECT_EQ(nIndex, isimIndex);
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsDefaultWithoutSubscriberConfig)
{
    // GIVEN
    m_pSubscriberManager->SetIsim(IMS_TRUE);
    m_pSubscriberManager->SetSubscriberConfig(IMS_NULL);
    m_pSubscriberManager->SetPuids(m_objEmptyPuids);

    EXPECT_CALL(m_objMockISubscriberConfig, IsProvisioningDone()).Times(0);

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->GetConfiguredImpus().GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenIsimTrueProvisioningNotDone)
{
    // GIVEN : ISIM is true and Provisioning is not done
    m_pSubscriberManager->SetIsim(IMS_TRUE);
    m_pSubscriberManager->SetPuids(m_objEmptyPuids);
    EXPECT_CALL(m_objMockISubscriberConfig, IsProvisioningDone())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->GetConfiguredImpus().GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenFailedGetImpuFromIsim)
{
    // GIVEN : ISIM is true and Provisioning is done, UpdateImpuFromIsim is false
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
    EXPECT_EQ(0, m_pSubscriberManager->GetConfiguredImpus().GetCount());
}

TEST_F(AosSubscriberManagerTest, SucceedsConfigureAsDefaultWhenUsimAndValidTempImpu)
{
    // GIVEN : Valid PUIDs
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_NE(0, m_pSubscriberManager->GetConfiguredImpus().GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenFailedUpdateSubscriberInfo)
{
    // GIVEN : ISIM is false, USIM is true, UpdateSubscriberInfoWithTempImpu is false
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);
    m_pSubscriberManager->SetPuids(m_objEmptyPuids);
    m_pSubscriberManager->m_strTemporaryPublicUserId = AString::ConstNull();

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->GetConfiguredImpus().GetCount());
}

TEST_F(AosSubscriberManagerTest, SucceedsConfigureAsDefaultWhenConf)
{
    // GIVEN : Invalid PUIDs
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_FALSE);

    m_pSubscriberManager->SetPuids(m_objEmptyPuids);

    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objValidPuids));

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_NE(0, m_pSubscriberManager->GetConfiguredImpus().GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsDefaultWhenInvalidPuids)
{
    // GIVEN : Invalid PUIDs
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_FALSE);

    m_pSubscriberManager->SetPuids(m_objEmptyPuids);

    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objEmptyPuids));

    // WHEN
    m_pSubscriberManager->ConfigureAsDefault();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->GetConfiguredImpus().GetCount());
}

TEST_F(AosSubscriberManagerTest, SucceedsConfigureAsFake)
{
    // GIVEN
    m_pSubscriberManager->SetPuidsForFake(m_objEmptyPuids);

    // WHEN
    m_pSubscriberManager->ConfigureAsFake();

    // THEN
    EXPECT_NE(0, m_pSubscriberManager->GetConfiguredImpusForFake().GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsFakeWhenSubscriberConfigFakeIsNull)
{
    // GIVEN
    m_pSubscriberManager->SetPuidsForFake(m_objEmptyPuids);
    m_pSubscriberManager->SetSubscriberConfigForFake(IMS_NULL);

    // WHEN
    m_pSubscriberManager->ConfigureAsFake();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->GetConfiguredImpusForFake().GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsFakeWhenGetInvalidPuids)
{
    // GIVEN
    m_pSubscriberManager->SetPuidsForFake(m_objEmptyPuids);

    ON_CALL(m_objMockISubscriberConfigFake, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objEmptyPuids));

    // WHEN
    m_pSubscriberManager->ConfigureAsFake();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->GetConfiguredImpusForFake().GetCount());
}

TEST_F(AosSubscriberManagerTest, FailedConfigureAsFakeWhenGetInvalidImpu)
{
    // GIVEN
    m_objValidPuids.SetElementAt(AString::ConstEmpty(), 0);

    // WHEN
    m_pSubscriberManager->ConfigureAsFake();

    // THEN
    EXPECT_EQ(0, m_pSubscriberManager->GetConfiguredImpusForFake().GetCount());
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseCheckIsimValuesWhenIsimIsNotSupport)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, IsIsimSupported()).WillByDefault(Return(IMS_FALSE));

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->CheckIsimValues();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseCheckIsimValuesWhenImpuIsEmpty)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, IsIsimSupported()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objEmptyPuids));

    AString strEmptyImpi;
    ON_CALL(m_objMockISubscriberConfig, GetPrivateUserId()).WillByDefault(ReturnRef(strEmptyImpi));

    AString strEmptyHomeDomainName;
    ON_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .WillByDefault(ReturnRef(strEmptyHomeDomainName));

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->CheckIsimValues();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseCheckIsimValuesWhenImpuIsInvalid)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, IsIsimSupported()).WillByDefault(Return(IMS_TRUE));

    AStringArray objInvalidPuids;
    objInvalidPuids.AddElement(AString("INVALID_IMPU"));
    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(objInvalidPuids));

    AString strEmptyImpi;
    ON_CALL(m_objMockISubscriberConfig, GetPrivateUserId()).WillByDefault(ReturnRef(strEmptyImpi));

    AString strEmptyHomeDomainName;
    ON_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .WillByDefault(ReturnRef(strEmptyHomeDomainName));

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->CheckIsimValues();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseCheckIsimValuesWhenInvalidImpi)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, IsIsimSupported()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objValidPuids));

    AString strEmptyImpi;
    ON_CALL(m_objMockISubscriberConfig, GetPrivateUserId()).WillByDefault(ReturnRef(strEmptyImpi));

    AString strHomeDomainName = "HOME_DOMAIN_NAME";
    ON_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .WillByDefault(ReturnRef(strHomeDomainName));

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->CheckIsimValues();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseCheckIsimValuesWhenInvalidHdn)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, IsIsimSupported()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objValidPuids));

    AString strImpi = "IMPI";
    ON_CALL(m_objMockISubscriberConfig, GetPrivateUserId()).WillByDefault(ReturnRef(strImpi));

    AString strEmptyHomeDomainName;
    ON_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .WillByDefault(ReturnRef(strEmptyHomeDomainName));

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->CheckIsimValues();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsTrueCheckIsimValues)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, IsIsimSupported()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objValidPuids));

    AString strImpi = "IMPI";
    ON_CALL(m_objMockISubscriberConfig, GetPrivateUserId()).WillByDefault(ReturnRef(strImpi));

    AString strHomeDomainName = "HOME_DOMAIN_NAME";
    ON_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .WillByDefault(ReturnRef(strHomeDomainName));

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->CheckIsimValues();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsTrueCheckIsimValuesWhenInvalidImpuButUseImsiBasedUri)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, IsIsimSupported()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objEmptyPuids));

    AString strEmptyImpi;
    ON_CALL(m_objMockISubscriberConfig, GetPrivateUserId()).WillByDefault(ReturnRef(strEmptyImpi));

    AString strEmptyHomeDomainName;
    ON_CALL(m_objMockISubscriberConfig, GetHomeDomainName())
            .WillByDefault(ReturnRef(strEmptyHomeDomainName));

    m_pSubscriberManager->SetImsiBasedUriPrioritized(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->CheckIsimValues();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedGetImpuFromIsimWhenGetEmptyPuids)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds())
            .WillByDefault(ReturnRef(m_objEmptyPuids));

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->UpdateImpuFromIsim(m_objOutPuids);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsGetImpuWhenLimitedAdminSmsModeAndOneValidImpu)
{
    // GIVEN
    AStringArray objOnePuid;
    objOnePuid.AddElement(AString("sip:user1@ims.com"));

    ON_CALL(m_objMockISubscriberConfig, GetPublicUserIds()).WillByDefault(ReturnRef(objOnePuid));

    m_pSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->UpdateImpuFromIsim(m_objOutPuids);

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

    m_pSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->UpdateImpuFromIsim(m_objOutPuids);

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

    m_pSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_TRUE);

    // strPhoneNumber.GetLength() > USIM_MSISDN_LENGTH
    m_pSubscriberManager->m_strPhoneNumber = AString("1231234567892");

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->UpdateImpuFromIsim(m_objOutPuids);

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

    m_pSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_TRUE);

    // strPhoneNumber.GetLength() > USIM_MSISDN_LENGTH
    m_pSubscriberManager->m_strPhoneNumber = AString("1234567892");

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->UpdateImpuFromIsim(m_objOutPuids);

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

    m_pSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_TRUE);
    m_pSubscriberManager->m_strPhoneNumber = AString("1231234567892");

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->UpdateImpuFromIsim(m_objOutPuids);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, UpdateSubscriberInfoWithTempImpuReturnsFalseWhenInvalidImpu)
{
    // GIVEN
    m_pSubscriberManager->m_strTemporaryPublicUserId = AString::ConstNull();

    ASSERT_TRUE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    ASSERT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    ASSERT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    AStringArray objPuids;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->UpdateSubscriberInfoWithTempImpu(objPuids);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, UpdateSubscriberInfoWithTempImpuReturnsFalseWhenInvalidImpi)
{
    // GIVEN
    m_pSubscriberManager->m_strTemporaryPrivateUserId = AString::ConstNull();

    ASSERT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    ASSERT_TRUE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    ASSERT_FALSE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    AStringArray objPuids;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->UpdateSubscriberInfoWithTempImpu(objPuids);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, UpdateSubscriberInfoWithTempImpuReturnsFalseWhenInvalidHdn)
{
    // GIVEN
    m_pSubscriberManager->m_strTemporaryHomeDomainName = AString::ConstNull();

    ASSERT_FALSE(m_pSubscriberManager->GetTemporaryPublicUserId().GetLength() == 0);
    ASSERT_FALSE(m_pSubscriberManager->GetTemporaryPrivateUserId().GetLength() == 0);
    ASSERT_TRUE(m_pSubscriberManager->GetTemporaryHomeDomainName().GetLength() == 0);

    AStringArray objPuids;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->UpdateSubscriberInfoWithTempImpu(objPuids);

    // THEN
    EXPECT_FALSE(bResult);
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

TEST_F(AosSubscriberManagerTest, ReturnsTrueProcessFallback)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .WillByDefault(Return(&m_objMockIConfigurable));

    ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, _))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, _))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockISubscriberConfig, IsIsimSupported()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockISubscriberConfig, IsUsimSupported()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ReconfigureFallback(IMS_TRUE);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReconfigureFallbackReturnsFalseWhenNoFallbackNeeded)
{
    // GIVEN
    m_pSubscriberManager->SetUsimFallback(IMS_FALSE);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ReconfigureFallback(IMS_FALSE);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenTimerIsRunning)
{
    // GIVEN
    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenIsNotReady)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_FALSE);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable();

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
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable();

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
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenInvalidImpus)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);
    m_pSubscriberManager->SetPuids(m_objEmptyPuids);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, FailedProcessPhoneNumberAvailableWhenInvalidTempImpus)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);

    m_pSubscriberManager->SetPuids(m_objValidPuids);

    // Setting for CreateTemporaryPublicUserId
    m_objStrTestImsi = AString::ConstNull();
    m_objStrTestMcc = AString::ConstNull();
    m_objStrTestMnc = AString::ConstNull();
    SetUpDefaultISubscriberInfo();

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable();

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
    m_pSubscriberManager->SetPuids(m_objValidPuids);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsProcessPhoneNumberAvailable)
{
    // GIVEN
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_FALSE);
    m_pSubscriberManager->SetUsim(IMS_TRUE);

    m_pSubscriberManager->SetPuids(m_objValidPuids);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessPhoneNumberAvailable();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsTrueWhenProcessIsimStateChangeWithLoaded)
{
    // GIVEN
    IsimState state = IsimState::LOADED;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessIsimStateChange(state);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsTrueWhenProcessIsimStateChangeWithRefreshCompleted)
{
    // GIVEN
    IsimState state = IsimState::REFRESH_COMPLETED;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessIsimStateChange(state);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsTrueWhenProcessIsimStateChangeWithNotReady)
{
    // GIVEN
    IsimState state = IsimState::NOT_READY;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessIsimStateChange(state);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsTrueWhenProcessSimStateChangeWithAbsentForUsim)
{
    // GIVEN
    SimState state = SimState::ABSENT;

    m_pSubscriberManager->SetUsim(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_FALSE);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessSimStateChange(state);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsTrueWhenProcessSimStateChangeWithNotReadyForUsim)
{
    // GIVEN
    SimState state = SimState::NOT_READY;

    m_pSubscriberManager->SetUsim(IMS_TRUE);
    m_pSubscriberManager->SetIsim(IMS_FALSE);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessSimStateChange(state);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseWhenProcessSimStateChangeWithAbsentForIsim)
{
    // GIVEN
    SimState state = SimState::ABSENT;

    m_pSubscriberManager->SetUsim(IMS_FALSE);
    m_pSubscriberManager->SetIsim(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessSimStateChange(state);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseWhenProcessSimStateChangeWithReady)
{
    // GIVEN
    SimState state = SimState::READY;

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->ProcessSimStateChange(state);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, StartRestartTimerWhenProcessPhoneRestarted)
{
    // GIVEN
    ASSERT_EQ(m_pSubscriberManager->GetTimerToPhoneRestartRecovery(), nullptr);

    // WHEN
    m_pSubscriberManager->ProcessPhoneRestarted();

    // THEN
    EXPECT_NE(m_pSubscriberManager->GetTimerToPhoneRestartRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, ShouldStopTimerPhoneRestartRecoveryWhenTimerExpired)
{
    // GIVEN
    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->GetTimerToPhoneRestartRecovery(), nullptr);

    // WHEN
    m_pSubscriberManager->ProcessPhoneRestartRecoveryTimerExpired();

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetTimerToPhoneRestartRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, SucceedsNotifyState)
{
    // GIVEN
    EXPECT_CALL(m_objListener1, AosSubscriberManager_NotifyState(_)).Times(1);
    EXPECT_CALL(m_objListener2, AosSubscriberManager_NotifyState(_)).Times(1);
    EXPECT_CALL(m_objListener3, AosSubscriberManager_NotifyState(_)).Times(1);

    m_pSubscriberManager->AddListener(&m_objListener1);
    m_pSubscriberManager->AddListener(&m_objListener2);
    m_pSubscriberManager->AddListener(&m_objListener3);

    // WHEN
    m_pSubscriberManager->NotifyState(IAosSubscriber::READY);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosSubscriberManagerTest, FailedNotifyStateWhenEmptyListener)
{
    // GIVEN: No AddListenr
    EXPECT_CALL(m_objListener1, AosSubscriberManager_NotifyState(_)).Times(0);
    EXPECT_CALL(m_objListener2, AosSubscriberManager_NotifyState(_)).Times(0);
    EXPECT_CALL(m_objListener3, AosSubscriberManager_NotifyState(_)).Times(0);

    // WHEN
    m_pSubscriberManager->NotifyState(IAosSubscriber::READY);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosSubscriberManagerTest, SucceedsNotifyMonitorState)
{
    // GIVEN
    EXPECT_CALL(m_objListener1, AosSubscriberManager_NotifyState(_)).Times(1);
    EXPECT_CALL(m_objListener2, AosSubscriberManager_NotifyState(_)).Times(1);
    EXPECT_CALL(m_objListener3, AosSubscriberManager_NotifyState(_)).Times(1);

    m_pSubscriberManager->AddListenerForMonitor(&m_objListener1);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener2);
    m_pSubscriberManager->AddListenerForMonitor(&m_objListener3);

    // WHEN
    m_pSubscriberManager->NotifyMonitorState(IAosSubscriber::READY);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosSubscriberManagerTest, FailedNotifyMonitorStateWhenEmptyListener)
{
    // GIVEN
    EXPECT_CALL(m_objListener1, AosSubscriberManager_NotifyState(_)).Times(0);
    EXPECT_CALL(m_objListener2, AosSubscriberManager_NotifyState(_)).Times(0);
    EXPECT_CALL(m_objListener3, AosSubscriberManager_NotifyState(_)).Times(0);

    // No AddListenr

    // WHEN
    m_pSubscriberManager->NotifyMonitorState(IAosSubscriber::READY);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosSubscriberManagerTest, ReturnFalsePrimaryImpuValidWhenInvalidPhoneNumber)
{
    // GIVEN
    m_pSubscriberManager->m_strPhoneNumber = AString::ConstNull();

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->IsPrimaryImpuValid(m_objValidPuids);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsFalseWhenIsSipUriWithZeroImpu)
{
    // GIVEN
    AString strImpu = AString::ConstNull();

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->IsSipUri(strImpu);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsTrueWhenIsSipUriWithSip)
{
    // GIVEN
    AString strImpu = AString("sip:user1@ims.com");

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->IsSipUri(strImpu);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, ReturnsTrueWhenIsSipUriWithSips)
{
    // GIVEN
    AString strImpu = AString("sips:user1@ims.com");

    // WHEN
    IMS_BOOL bResult = m_pSubscriberManager->IsSipUri(strImpu);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberManagerTest, SucceedsUpdateNConfiguration)
{
    // GIVEN
    m_pSubscriberManager->SetPuids(m_objValidPuids);
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);

    ImsVector<IMS_SINT32> objUpdatedImsIdentityPriority;
    objUpdatedImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objUpdatedImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->SetIsimIndexForImpu(0);
    m_pSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_FALSE);
    m_pSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);

    ON_CALL(m_objMockIAosNConfiguration, GetIsimIndexForImpu()).WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetImsIdentityPriority())
            .WillByDefault(ReturnRef(objUpdatedImsIdentityPriority));
    ON_CALL(m_objMockIAosNConfiguration, RemoveListener(_)).WillByDefault(Return());
    m_pSubscriberManager->SetNConfig(&m_objMockIAosNConfiguration);

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
    m_pSubscriberManager->SetPuids(m_objValidPuids);
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    ImsVector<IMS_SINT32> objUpdatedImsIdentityPriority;
    objUpdatedImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objUpdatedImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->SetIsimIndexForImpu(0);
    m_pSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_FALSE);
    m_pSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);

    ON_CALL(m_objMockIAosNConfiguration, GetIsimIndexForImpu()).WillByDefault(Return(1));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMockIAosNConfiguration, GetImsIdentityPriority())
            .WillByDefault(ReturnRef(objUpdatedImsIdentityPriority));
    ON_CALL(m_objMockIAosNConfiguration, RemoveListener(_)).WillByDefault(Return());

    m_pSubscriberManager->SetNConfig(&m_objMockIAosNConfiguration);

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
    m_pSubscriberManager->SetPuids(m_objValidPuids);
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    m_pSubscriberManager->SetNConfig(IMS_NULL);

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    m_pSubscriberManager->SetIsimIndexForImpu(0);
    m_pSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_FALSE);
    m_pSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);

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
    m_pSubscriberManager->SetPuids(m_objValidPuids);
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->SetIsimIndexForImpu(0);
    m_pSubscriberManager->SetSupportLimitedAdminSmsMode(IMS_FALSE);
    m_pSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);

    ON_CALL(m_objMockIAosNConfiguration, GetIsimIndexForImpu()).WillByDefault(Return(0));
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(m_objMockIAosNConfiguration, GetImsIdentityPriority())
            .WillByDefault(ReturnRef(objImsIdentityPriority));
    ON_CALL(m_objMockIAosNConfiguration, RemoveListener(_)).WillByDefault(Return());

    m_pSubscriberManager->SetNConfig(&m_objMockIAosNConfiguration);

    m_pSubscriberManager->SetSubscriberConfig(IMS_NULL);

    // WHEN
    m_pSubscriberManager->NConfiguration_NotifyConfigChanged();

    // THEN
    EXPECT_TRUE(m_pSubscriberManager->IsProvisioned());
    EXPECT_NE(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
}

TEST_F(AosSubscriberManagerTest, ClearAllWhenInitCompletedWithNotSupportUsim)
{
    // GIVEN
    ON_CALL(m_objMockISubscriberConfig, IsIsimSupported()).WillByDefault(Return(IMS_FALSE));

    m_pSubscriberManager->SetPuids(m_objValidPuids);
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    ASSERT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    ASSERT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));

    // WHEN
    m_pSubscriberManager->SubscriberConfig_InitCompleted();

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
    EXPECT_FALSE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, ClearAllWhenRefreshCompletedWithNotSupportUsim)
{
    // GIVEN
    EXPECT_CALL(m_objMockISubscriberConfig, IsIsimSupported())
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));

    m_pSubscriberManager->SetPuids(m_objValidPuids);
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    ASSERT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    ASSERT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));

    // WHEN
    m_pSubscriberManager->SubscriberConfig_RefreshCompleted();

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
    EXPECT_FALSE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, EnableRefreshStartedValueWhenInvokesRefreshStarted)
{
    // GIVEN
    ASSERT_FALSE(m_pSubscriberManager->IsRefreshStarted());

    // WHEN
    m_pSubscriberManager->SubscriberConfig_RefreshStarted();

    // THEN
    EXPECT_TRUE(m_pSubscriberManager->IsRefreshStarted());
}

TEST_F(AosSubscriberManagerTest, ClearAllWhenNotifyErrorWithNotSupportFallback)
{
    // GIVEN
    m_pSubscriberManager->SetPuids(m_objValidPuids);
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    ASSERT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    ASSERT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));

    // WHEN
    m_pSubscriberManager->SubscriberConfig_NotifyError(0);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
    EXPECT_FALSE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, FailedFallbackOnNotifyErrorWhenTimerIsRunning)
{
    // GIVEN
    m_pSubscriberManager->SetPuids(m_objValidPuids);
    m_pSubscriberManager->SetProvisioned(IMS_TRUE);
    ASSERT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    ASSERT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));

    ImsVector<IMS_SINT32> objImsIdentityPriority;
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);

    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->GetTimerToPhoneRestartRecovery(), nullptr);

    // WHEN
    m_pSubscriberManager->SubscriberConfig_NotifyError(0);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 3);
    EXPECT_TRUE(m_pSubscriberManager->IsProvisioned(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, InvokesConfigUpdateNotifyUpdate)
{
    // Currently, there is no logic that requires tests.
    m_pSubscriberManager->ConfigUpdate_NotifyUpdate(0, AString::ConstNull(), AString::ConstNull());
}

TEST_F(AosSubscriberManagerTest, FailedStartProcessWhenTimerExpiredWithInvalidTimer)
{
    // GIVEN
    m_pSubscriberManager->StartTimer(TIMER_ICC_LOADED_WAITING, 3);
    EXPECT_NE(m_pSubscriberManager->GetTimerToIccLoadedWaiting(), nullptr);

    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->GetTimerToIccLoadedWaiting(), nullptr);

    // WHEN : Expiration of an invalid timer does not affect other timers.
    m_pSubscriberManager->Timer_TimerExpired(IMS_NULL);

    // THEN
    EXPECT_NE(m_pSubscriberManager->GetTimerToIccLoadedWaiting(), nullptr);
    EXPECT_NE(m_pSubscriberManager->GetTimerToPhoneRestartRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, SucceedsStopTimerWhenIccLoadedWaitingTimerExpired)
{
    // GIVEN
    m_pSubscriberManager->StartTimer(TIMER_ICC_LOADED_WAITING, 3);
    EXPECT_NE(m_pSubscriberManager->GetTimerToIccLoadedWaiting(), nullptr);

    // WHEN
    m_pSubscriberManager->Timer_TimerExpired(m_pSubscriberManager->GetTimerToIccLoadedWaiting());

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetTimerToIccLoadedWaiting(), nullptr);
}

TEST_F(AosSubscriberManagerTest, SucceedsStopTimerWhenRecoveryTimerExpired)
{
    // GIVEN
    m_pSubscriberManager->StartTimer(TIMER_PHONE_RESTART_RECOVERY, 3);
    EXPECT_NE(m_pSubscriberManager->GetTimerToPhoneRestartRecovery(), nullptr);

    // WHEN
    m_pSubscriberManager->Timer_TimerExpired(
            m_pSubscriberManager->GetTimerToPhoneRestartRecovery());

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetTimerToPhoneRestartRecovery(), nullptr);
}

TEST_F(AosSubscriberManagerTest, RestartsWhenPhoneNumberStateChangedWithNotReady)
{
    // GIVEN
    m_pSubscriberManager->SetPuids(m_objValidPuids);
    m_pSubscriberManager->SetUsim(IMS_TRUE);
    m_pSubscriberManager->SetProvisioned(IMS_FALSE);
    m_pSubscriberManager->SetSubscriberConfig(IMS_NULL);

    // WHEN
    m_pSubscriberManager->ServicePhone_PhoneNumberStateChanged(
            IMS_FALSE, PhoneNumberState::SIM_LOADED);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetConfiguredImpus().GetCount(), 0);
}

TEST_F(AosSubscriberManagerTest, DisableUsimFallbackValueWhenIsimStateChanged)
{
    // GIVEN
    m_pSubscriberManager->SetUsimFallback(IMS_TRUE);

    ON_CALL(m_objMockISubscriberConfig, GetConfigurable())
            .WillByDefault(Return(&m_objMockIConfigurable));

    ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_ISIM, _))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockIConfigurable, Update(IConfigurable::CP_I_SUBSCRIPTION_ATTRIBUTE_USIM, _))
            .WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockISubscriberConfig, IsIsimSupported()).WillByDefault(Return(IMS_TRUE));

    ON_CALL(m_objMockISubscriberConfig, IsUsimSupported()).WillByDefault(Return(IMS_TRUE));

    // WHEN
    m_pSubscriberManager->ServicePhone_IsimStateChanged(IsimState::LOADED);

    // THEN
    EXPECT_FALSE(m_pSubscriberManager->GetUsimFallback());
}

TEST_F(AosSubscriberManagerTest, NotifyNotReadyStateWhenSimStateChangedToAbsent)
{
    // GIVEN
    MockIAosSubscriberManagerListener objListener;

    EXPECT_CALL(objListener, AosSubscriberManager_NotifyState(_)).Times(1);

    m_pSubscriberManager->AddListener(&objListener);
    m_pSubscriberManager->SetUsim(IMS_TRUE);
    m_pSubscriberManager->SetNotifyState(IAosSubscriber::READY);

    // WHEN
    m_pSubscriberManager->ServicePhone_SimStateChanged(SimState::ABSENT);

    // THEN
    EXPECT_EQ(m_pSubscriberManager->GetNotifyState(), IAosSubscriber::NOT_READY);
}

TEST_F(AosSubscriberManagerTest, ReturnsEmptyWhenIdentityPriorityToStringWithEmpty)
{
    // GIVEN
    ImsVector<IMS_SINT32> objImsIdentityPriority;

    m_pSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);

    // WHEN
    // THEN
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[Empty]");
}

TEST_F(AosSubscriberManagerTest, ReturnsIsimUsimWhenIdentityPriorityToStringWithIsimUsim)
{
    ImsVector<IMS_SINT32> objImsIdentityPriority;

    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM);

    m_pSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);

    // WHEN
    // THEN
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM][USIM]");
}

TEST_F(AosSubscriberManagerTest,
        ReturnsIsimAndIsimImsiWhenIdentityPriorityToStringWithIsimAndIsimImsi)
{
    // GIVEN
    ImsVector<IMS_SINT32> objImsIdentityPriority;

    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM);
    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI);

    m_pSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);

    // WHEN
    // THEN
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[ISIM][ISIM_IMSI]");
}

TEST_F(AosSubscriberManagerTest, ReturnsConfWhenIdentityPriorityToStringWithConf)
{
    // GIVEN
    ImsVector<IMS_SINT32> objImsIdentityPriority;

    objImsIdentityPriority.Add(CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF);
    m_pSubscriberManager->SetImsIdentityPriority(objImsIdentityPriority);

    // WHEN
    // THEN
    EXPECT_STREQ(m_pSubscriberManager->IdentityPriorityToString(), "[CONF]");
}

TEST_F(AosSubscriberManagerTest, ReturnValidStringWhenPrintIdentity)
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

TEST_F(AosSubscriberManagerTest, ReturnsValidStringWhenTimerToString)
{
    EXPECT_STREQ(m_pSubscriberManager->TimerToString(TIMER_ICC_LOADED_WAITING),
            "TIMER_ICC_LOADED_WAITING");
    EXPECT_STREQ(m_pSubscriberManager->TimerToString(TIMER_PHONE_RESTART_RECOVERY),
            "TIMER_PHONE_RESTART_RECOVERY");
    EXPECT_STREQ(m_pSubscriberManager->TimerToString(999 /* INVALID_TIMER */), "__INVALID__");
}

TEST_F(AosSubscriberManagerTest, ReturnsStateWhenStateToString)
{
    EXPECT_STREQ(m_pSubscriberManager->StateToString(IAosSubscriber::NOT_READY), "NOT_READY");
    EXPECT_STREQ(m_pSubscriberManager->StateToString(IAosSubscriber::READY), "READY");
    EXPECT_STREQ(m_pSubscriberManager->StateToString(IAosSubscriber::REFRESH_STARTED),
            "REFRESH_STARTED");
    EXPECT_STREQ(m_pSubscriberManager->StateToString(IAosSubscriber::REFRESH_COMPLETED),
            "REFRESH_COMPLETED");
    EXPECT_STREQ(
            m_pSubscriberManager->StateToString(IAosSubscriber::REFRESH_FAILED), "REFRESH_FAILED");
    EXPECT_STREQ(m_pSubscriberManager->StateToString(100 /* INVALID_STATE */), "INVALID");
}
