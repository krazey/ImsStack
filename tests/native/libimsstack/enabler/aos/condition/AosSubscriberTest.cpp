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

#include "interface/IAosAppContext.h"
#include "interface/IAosSubscriberListener.h"
#include "condition/AosSubscriber.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosRegistration.h"
#include "interface/MockIAosSubscriberListener.h"
#include "interface/MockIAosSubscriberManager.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnPointee;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;
const AString PROFILE_ID = AString("test");

#define DECLARE_USING(Base) \
    using Base::Init;       \
    using Base::CleanUp;    \
    using Base::AosSubscriberManager_NotifyState;

class TestAosSubscriber : public AosSubscriber
{
public:
    DECLARE_USING(AosSubscriber)

    inline explicit TestAosSubscriber(IN IAosAppContext* piAppContext) :
            AosSubscriber(piAppContext)
    {
    }

    inline IAosSubscriberManager* GetSubscriberManager() { return m_piSubscriberManager; }

    inline void SetSubscriberManager(IN IAosSubscriberManager* piSubscriberManager)
    {
        m_piSubscriberManager = piSubscriberManager;
    }

    inline void SetRegType(IN AosRegistrationType eRegType) { m_eRegType = eRegType; }
};

class AosSubscriberTest : public ::testing::Test
{
public:
    TestAosSubscriber* m_pAosSubscriber;

    IAosNConfiguration* m_piAosNConfiguration;

    NiceMock<MockIAosAppContext> m_objMockIAosAppContext;
    NiceMock<MockIAosRegistration> m_objMockIAosRegistration;
    NiceMock<MockIAosSubscriberListener> m_objMockListener;
    NiceMock<MockIAosNConfiguration> m_objMockIAosNConfiguration;

protected:
    void SetUp() override
    {
        ReplaceOriginWithMock();

        // MockIAosAppContext
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(PROFILE_ID));
        ON_CALL(m_objMockIAosAppContext, GetRegistration())
                .WillByDefault(Return(&m_objMockIAosRegistration));

        // MockIAosNConfiguration
        ON_CALL(m_objMockIAosNConfiguration, IsERegUsingFirstImpuInIsim())
                .WillByDefault(Return(IMS_FALSE));

        m_pAosSubscriber = new TestAosSubscriber(&m_objMockIAosAppContext);
        ASSERT_TRUE(m_pAosSubscriber != nullptr);

        ON_CALL(m_objMockListener, Subscriber_StateChanged(_, _)).WillByDefault(Return());
        m_pAosSubscriber->SetListener(&m_objMockListener);
    }

    void TearDown() override
    {
        RestoreOriginInstance();

        if (m_pAosSubscriber)
        {
            delete m_pAosSubscriber;
        }
    }

    void ReplaceOriginWithMock()
    {
        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration);
    }

    void RestoreOriginInstance()
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration);
    }
};

TEST_F(AosSubscriberTest, IsReady_ManagerNull)
{
    m_pAosSubscriber->SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(m_pAosSubscriber->GetSubscriberManager(), nullptr);

    EXPECT_FALSE(m_pAosSubscriber->IsReady());
}

TEST_F(AosSubscriberTest, IsReady_ManagerReturn)
{
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, IsReady(_))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);
    EXPECT_NE(m_pAosSubscriber->GetSubscriberManager(), nullptr);

    EXPECT_TRUE(m_pAosSubscriber->IsReady());
    EXPECT_FALSE(m_pAosSubscriber->IsReady());
}

TEST_F(AosSubscriberTest, IsIsimReturnsTrueWhenIsimIsTrue)
{
    // GIVEN
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    ON_CALL(objMockIAosSubscriberManager, IsIsim()).WillByDefault(Return(IMS_TRUE));
    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);

    // WHEN
    IMS_BOOL bResult = m_pAosSubscriber->IsIsim();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberTest, IsIsimReturnsFalseWhenIsimIsFalse)
{
    // GIVEN
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    ON_CALL(objMockIAosSubscriberManager, IsIsim()).WillByDefault(Return(IMS_FALSE));
    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);

    // WHEN
    IMS_BOOL bResult = m_pAosSubscriber->IsIsim();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberTest, IsUsimReturnsTrueWhenUsimIsTrue)
{
    // GIVEN
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    ON_CALL(objMockIAosSubscriberManager, IsUsim()).WillByDefault(Return(IMS_TRUE));
    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);

    // WHEN
    IMS_BOOL bResult = m_pAosSubscriber->IsUsim();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosSubscriberTest, IsUsimReturnsFalseWhenUsimIsFalse)
{
    // GIVEN
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    ON_CALL(objMockIAosSubscriberManager, IsUsim()).WillByDefault(Return(IMS_FALSE));
    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);

    // WHEN
    IMS_BOOL bResult = m_pAosSubscriber->IsUsim();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosSubscriberTest, SetListener_IsReadyReturn)
{
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, IsReady(_))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);
    EXPECT_NE(m_pAosSubscriber->GetSubscriberManager(), nullptr);

    MockIAosSubscriberListener objMockListener1;
    EXPECT_CALL(objMockListener1, Subscriber_StateChanged(IAosSubscriber::READY, _)).Times(1);

    MockIAosSubscriberListener objMockListener2;
    EXPECT_CALL(objMockListener2, Subscriber_StateChanged(IAosSubscriber::NOT_READY, _)).Times(1);

    m_pAosSubscriber->SetListener(&objMockListener1);
    m_pAosSubscriber->SetListener(&objMockListener2);
}

TEST_F(AosSubscriberTest, GetConfiguredImpus_ManagerNull)
{
    m_pAosSubscriber->SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(m_pAosSubscriber->GetSubscriberManager(), nullptr);
    EXPECT_EQ(m_pAosSubscriber->GetConfiguredImpus().GetCount(), 0);
}

TEST_F(AosSubscriberTest, GetConfiguredImpus_ManagerReturn)
{
    AStringArray objPuids;
    objPuids.AddElement(AString("PUID1"));
    objPuids.AddElement(AString("PUID2"));
    objPuids.AddElement(AString("PUID3"));

    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, GetConfiguredImpus())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPuids));

    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);
    EXPECT_NE(m_pAosSubscriber->GetSubscriberManager(), nullptr);

    EXPECT_EQ(m_pAosSubscriber->GetConfiguredImpus().GetCount(), 3);
}

TEST_F(AosSubscriberTest, ReturnsOrderedImpusWhenEmergencyTypeAndUseERegUsingFirstImpuInIsim)
{
    // GIVEN
    AStringArray objOrderedPuids;
    objOrderedPuids.AddElement(AString("PUID0"));
    objOrderedPuids.AddElement(AString("PUID1"));
    objOrderedPuids.AddElement(AString("PUID2"));
    objOrderedPuids.AddElement(AString("PUID3"));

    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, GetOrderedImpus())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objOrderedPuids));

    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);
    m_pAosSubscriber->SetRegType(AosRegistrationType::EMERGENCY);

    ON_CALL(m_objMockIAosNConfiguration, IsERegUsingFirstImpuInIsim())
            .WillByDefault(Return(IMS_TRUE));

    // WHEN
    IMS_UINT32 nCount = m_pAosSubscriber->GetConfiguredImpus().GetCount();

    // THEN
    EXPECT_EQ(nCount, objOrderedPuids.GetCount());
}

TEST_F(AosSubscriberTest, GetFakeImpus_ManagerNull)
{
    m_pAosSubscriber->SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(m_pAosSubscriber->GetSubscriberManager(), nullptr);
    EXPECT_EQ(m_pAosSubscriber->GetFakeImpus().GetCount(), 0);
}

TEST_F(AosSubscriberTest, GetFakeImpus_ManagerReturn)
{
    AStringArray objPuids;
    objPuids.AddElement(AString("PUID1"));
    objPuids.AddElement(AString("PUID2"));
    objPuids.AddElement(AString("PUID3"));

    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, GetFakeImpus())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(objPuids));

    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);
    EXPECT_NE(m_pAosSubscriber->GetSubscriberManager(), nullptr);

    EXPECT_EQ(m_pAosSubscriber->GetFakeImpus().GetCount(), 3);
}

TEST_F(AosSubscriberTest, GetSubscriberConfig_ManagerNull)
{
    m_pAosSubscriber->SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(m_pAosSubscriber->GetSubscriberManager(), nullptr);
    EXPECT_EQ(m_pAosSubscriber->GetSubscriberConfig(), nullptr);
}

TEST_F(AosSubscriberTest, GetSubscriberConfig_ManagerReturn)
{
    ISubscriberConfig* piSubscriberConfig;
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, GetSubscriberConfig(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnPointee(&piSubscriberConfig));

    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);
    EXPECT_NE(m_pAosSubscriber->GetSubscriberManager(), nullptr);

    EXPECT_EQ(m_pAosSubscriber->GetSubscriberConfig(), piSubscriberConfig);
}

TEST_F(AosSubscriberTest, Init_SubscriberManagerNull)
{
    EXPECT_CALL(m_objMockIAosRegistration, GetRegType())
            .Times(1)
            .WillOnce(Return(AosRegistrationType::NORMAL));
    EXPECT_FALSE(m_pAosSubscriber->Init());
}

TEST_F(AosSubscriberTest, Init_RegTypeFake)
{
    EXPECT_CALL(m_objMockIAosRegistration, GetRegType())
            .Times(1)
            .WillOnce(Return(AosRegistrationType::FAKE));

    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, AddListenerForMonitor(_)).Times(1);

    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);

    EXPECT_TRUE(m_pAosSubscriber->Init());
}

TEST_F(AosSubscriberTest, Init_RegTypeNormal)
{
    EXPECT_CALL(m_objMockIAosRegistration, GetRegType())
            .Times(1)
            .WillOnce(Return(AosRegistrationType::NORMAL));

    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, AddListener(_)).Times(1);

    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);

    EXPECT_TRUE(m_pAosSubscriber->Init());
}

TEST_F(AosSubscriberTest, CleanUp_SubscriberManagerNull)
{
    EXPECT_FALSE(m_pAosSubscriber->CleanUp());
}

TEST_F(AosSubscriberTest, CleanUp_RegTypeFake)
{
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);

    m_pAosSubscriber->SetRegType(AosRegistrationType::FAKE);

    EXPECT_TRUE(m_pAosSubscriber->CleanUp());
}

TEST_F(AosSubscriberTest, CleanUp_RegTypeNormal)
{
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    m_pAosSubscriber->SetSubscriberManager(&objMockIAosSubscriberManager);

    m_pAosSubscriber->SetRegType(AosRegistrationType::NORMAL);

    EXPECT_TRUE(m_pAosSubscriber->CleanUp());
}

TEST_F(AosSubscriberTest, SucceedsNotifyWhenStateNotifiedWithNotReady)
{
    // GIVEN
    EXPECT_CALL(m_objMockListener, Subscriber_StateChanged(_, _));

    // WHEN
    m_pAosSubscriber->AosSubscriberManager_NotifyState(IAosSubscriber::NOT_READY);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosSubscriberTest, SucceedsNotifyWhenStateNotifiedWithReady)
{
    // GIVEN
    EXPECT_CALL(m_objMockListener, Subscriber_StateChanged(_, _));

    // WHEN
    m_pAosSubscriber->AosSubscriberManager_NotifyState(IAosSubscriber::READY);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosSubscriberTest, SucceedsNotifyWhenStateNotifiedWithRefreshStarted)
{
    // GIVEN
    EXPECT_CALL(m_objMockListener, Subscriber_StateChanged(_, _));

    // WHEN
    m_pAosSubscriber->AosSubscriberManager_NotifyState(IAosSubscriber::REFRESH_STARTED);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosSubscriberTest, SucceedsNotifyWhenStateNotifiedWithRefreshCompleted)
{
    // GIVEN
    EXPECT_CALL(m_objMockListener, Subscriber_StateChanged(_, _));

    // WHEN
    m_pAosSubscriber->AosSubscriberManager_NotifyState(IAosSubscriber::REFRESH_COMPLETED);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosSubscriberTest, SucceedsNotifyWhenStateNotifiedWithRefreshFailed)
{
    // GIVEN
    EXPECT_CALL(m_objMockListener, Subscriber_StateChanged(_, _));

    // WHEN
    m_pAosSubscriber->AosSubscriberManager_NotifyState(IAosSubscriber::REFRESH_FAILED);

    // THEN: The GIVEN condition should be met.
}
