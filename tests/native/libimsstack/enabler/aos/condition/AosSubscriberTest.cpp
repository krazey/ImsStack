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

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosRegistration.h"
#include "interface/MockIAosSubscriberListener.h"
#include "interface/MockIAosSubscriberManager.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnPointee;
using ::testing::ReturnRef;

class TestAosSubscriber : public AosSubscriber
{
public:
    inline explicit TestAosSubscriber(IN IAosAppContext* piAppContext) :
            AosSubscriber(piAppContext)
    {
    }

    // TODO : Remove friend class.
    friend class AosSubscriberTest;
};

class AosSubscriberTest : public ::testing::Test
{
public:
    TestAosSubscriber* m_pAosSubscriber;
    MockIAosAppContext objIMockAosAppContext;
    MockIAosRegistration* pMockIAosRegistration;

protected:
    virtual void SetUp() override
    {
        pMockIAosRegistration = new MockIAosRegistration();

        EXPECT_CALL(objIMockAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(objIMockAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(objIMockAosAppContext, GetRegistration())
                .Times(AnyNumber())
                .WillRepeatedly(Return(pMockIAosRegistration));

        m_pAosSubscriber = new TestAosSubscriber(&objIMockAosAppContext);
        ASSERT_TRUE(m_pAosSubscriber != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pAosSubscriber)
        {
            delete m_pAosSubscriber;
        }
    }

    void SetSubscriberManager(IN IAosSubscriberManager* piSubscriberManager)
    {
        m_pAosSubscriber->m_piSubscriberManager = piSubscriberManager;
    }

    void SetAosSubscriberListener(IN IAosSubscriberListener* piSubscriberListener)
    {
        m_pAosSubscriber->m_piListener = piSubscriberListener;
    }

    IAosSubscriberManager* GetSubscriberManager()
    {
        return m_pAosSubscriber->m_piSubscriberManager;
    }

    void SetRegType(IN AosRegistrationType eRegType) { m_pAosSubscriber->m_eRegType = eRegType; }

    IMS_BOOL Init() { return m_pAosSubscriber->Init(); }

    IMS_BOOL CleanUp() { return m_pAosSubscriber->CleanUp(); }

    void AosSubscriberManager_NotifyState(IN IMS_UINT32 nState)
    {
        m_pAosSubscriber->AosSubscriberManager_NotifyState(nState);
    }
};

TEST_F(AosSubscriberTest, IsReady_ManagerNull)
{
    SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(GetSubscriberManager(), nullptr);

    EXPECT_FALSE(m_pAosSubscriber->IsReady());
}

TEST_F(AosSubscriberTest, IsReady_ManagerReturn)
{
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, IsReady(_))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));
    EXPECT_NE(GetSubscriberManager(), nullptr);

    EXPECT_TRUE(m_pAosSubscriber->IsReady());
    EXPECT_FALSE(m_pAosSubscriber->IsReady());
}

TEST_F(AosSubscriberTest, SetListener_IsReadyReturn)
{
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, IsReady(_))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));
    EXPECT_NE(GetSubscriberManager(), nullptr);

    MockIAosSubscriberListener objMockListener1;
    EXPECT_CALL(objMockListener1, Subscriber_StateChanged(IAosSubscriber::READY, _)).Times(1);

    MockIAosSubscriberListener objMockListener2;
    EXPECT_CALL(objMockListener2, Subscriber_StateChanged(IAosSubscriber::NOT_READY, _)).Times(1);

    m_pAosSubscriber->SetListener(static_cast<IAosSubscriberListener*>(&objMockListener1));
    m_pAosSubscriber->SetListener(static_cast<IAosSubscriberListener*>(&objMockListener2));
}

TEST_F(AosSubscriberTest, GetConfiguredImpus_ManagerNull)
{
    SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(GetSubscriberManager(), nullptr);
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

    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));
    EXPECT_NE(GetSubscriberManager(), nullptr);

    EXPECT_EQ(m_pAosSubscriber->GetConfiguredImpus().GetCount(), 3);
}

TEST_F(AosSubscriberTest, GetFakeImpus_ManagerNull)
{
    SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(GetSubscriberManager(), nullptr);
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

    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));
    EXPECT_NE(GetSubscriberManager(), nullptr);

    EXPECT_EQ(m_pAosSubscriber->GetFakeImpus().GetCount(), 3);
}

TEST_F(AosSubscriberTest, GetSubscriberConfig_ManagerNull)
{
    SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(GetSubscriberManager(), nullptr);
    EXPECT_EQ(m_pAosSubscriber->GetSubscriberConfig(), nullptr);
}

TEST_F(AosSubscriberTest, GetSubscriberConfig_ManagerReturn)
{
    ISubscriberConfig* piSubscriberConfig;
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, GetSubscriberConfig(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnPointee(&piSubscriberConfig));

    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));
    EXPECT_NE(GetSubscriberManager(), nullptr);

    EXPECT_EQ(m_pAosSubscriber->GetSubscriberConfig(), piSubscriberConfig);
}

TEST_F(AosSubscriberTest, Init_SubscriberManagerNull)
{
    EXPECT_CALL(*pMockIAosRegistration, GetRegType())
            .Times(1)
            .WillOnce(Return(AosRegistrationType::NORMAL));
    EXPECT_FALSE(Init());
}

TEST_F(AosSubscriberTest, Init_RegTypeFake)
{
    EXPECT_CALL(*pMockIAosRegistration, GetRegType())
            .Times(1)
            .WillOnce(Return(AosRegistrationType::FAKE));

    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, AddListenerForMonitor(_)).Times(1);

    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));

    EXPECT_TRUE(Init());
}

TEST_F(AosSubscriberTest, Init_RegTypeNormal)
{
    EXPECT_CALL(*pMockIAosRegistration, GetRegType())
            .Times(1)
            .WillOnce(Return(AosRegistrationType::NORMAL));

    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, AddListener(_)).Times(1);

    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));

    EXPECT_TRUE(Init());
}

TEST_F(AosSubscriberTest, CleanUp_SubscriberManagerNull)
{
    EXPECT_FALSE(CleanUp());
}

TEST_F(AosSubscriberTest, CleanUp_RegTypeFake)
{
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));

    SetRegType(AosRegistrationType::FAKE);

    EXPECT_TRUE(CleanUp());
}

TEST_F(AosSubscriberTest, CleanUp_RegTypeNormal)
{
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));

    SetRegType(AosRegistrationType::NORMAL);

    EXPECT_TRUE(CleanUp());
}

TEST_F(AosSubscriberTest, AosSubscriberManager_NotifyState)
{
    MockIAosSubscriberListener objMockListener;
    EXPECT_CALL(objMockListener, Subscriber_StateChanged(_, _)).Times(5);

    SetAosSubscriberListener(static_cast<IAosSubscriberListener*>(&objMockListener));

    AosSubscriberManager_NotifyState(IAosSubscriber::NOT_READY);
    AosSubscriberManager_NotifyState(IAosSubscriber::READY);
    AosSubscriberManager_NotifyState(IAosSubscriber::REFRESH_STARTED);
    AosSubscriberManager_NotifyState(IAosSubscriber::REFRESH_COMPLETED);
    AosSubscriberManager_NotifyState(IAosSubscriber::REFRESH_FAILED);
}