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
#include "interface/MockIAosSubscriberListener.h"
#include "interface/MockIAosSubscriberManager.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnPointee;
using ::testing::ReturnRef;

class AosSubscriberTest : public ::testing::Test {
public:
    AosSubscriber* pAosSubscriber;

protected:
    virtual void SetUp() override {
        MockIAosAppContext objMockIAosAppContext;
        EXPECT_CALL(objMockIAosAppContext, GetSlotId())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(objMockIAosAppContext, GetProfileId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strValue));

        pAosSubscriber = new AosSubscriber(static_cast<IAosAppContext*>(&objMockIAosAppContext));
        ASSERT_TRUE(pAosSubscriber != nullptr);
    }

    virtual void TearDown() override {
        if (pAosSubscriber) {
            delete pAosSubscriber;
        }
    }

    void SetSubscriberManager(IN IAosSubscriberManager* piSubscriberManager) {
        pAosSubscriber->m_piSubscriberManager = piSubscriberManager;
    }

    IAosSubscriberManager* GetSubscriberManager() {
        return pAosSubscriber->m_piSubscriberManager;
    }
};

TEST_F(AosSubscriberTest, IsReady_ManagerNull) {
    SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(GetSubscriberManager(), nullptr);

    EXPECT_FALSE(pAosSubscriber->IsReady());
}

TEST_F(AosSubscriberTest, IsReady_ManagerReturn) {
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, IsReady(_))
        .Times(2)
        .WillOnce(Return(IMS_TRUE))
        .WillOnce(Return(IMS_FALSE));

    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));
    EXPECT_NE(GetSubscriberManager(), nullptr);

    EXPECT_TRUE(pAosSubscriber->IsReady());
    EXPECT_FALSE(pAosSubscriber->IsReady());
}

TEST_F(AosSubscriberTest, SetListener_IsReadyReturn) {
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, IsReady(_))
        .Times(2)
        .WillOnce(Return(IMS_TRUE))
        .WillOnce(Return(IMS_FALSE));

    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));
    EXPECT_NE(GetSubscriberManager(), nullptr);

    MockIAosSubscriberListener objMockListener1;
        EXPECT_CALL(objMockListener1, Subscriber_StateChanged(IAosSubscriber::READY, _))
        .Times(1);

    MockIAosSubscriberListener objMockListener2;
        EXPECT_CALL(objMockListener2, Subscriber_StateChanged(IAosSubscriber::NOT_READY, _))
        .Times(1);

    pAosSubscriber->SetListener(static_cast<IAosSubscriberListener*>(&objMockListener1));
    pAosSubscriber->SetListener(static_cast<IAosSubscriberListener*>(&objMockListener2));
}

TEST_F(AosSubscriberTest, GetConfiguredImpus_ManagerNull) {
    SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(GetSubscriberManager(), nullptr);
    EXPECT_EQ(pAosSubscriber->GetConfiguredImpus().GetCount(), 0);
}

TEST_F(AosSubscriberTest, GetConfiguredImpus_ManagerReturn) {
    AStringArray objPuids;
    objPuids.AddElement(AString("PUID1"));
    objPuids.AddElement(AString("PUID2"));
    objPuids.AddElement(AString("PUID3"));

    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, GetConfiguredImpus(_))
        .Times(AnyNumber())
        .WillRepeatedly(ReturnRef(objPuids));

    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));
    EXPECT_NE(GetSubscriberManager(), nullptr);

    EXPECT_EQ(pAosSubscriber->GetConfiguredImpus().GetCount(), 3);
}

TEST_F(AosSubscriberTest, GetFakeImpus_ManagerNull) {
    SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(GetSubscriberManager(), nullptr);
    EXPECT_EQ(pAosSubscriber->GetFakeImpus().GetCount(), 0);
}

TEST_F(AosSubscriberTest, GetFakeImpus_ManagerReturn) {
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

    EXPECT_EQ(pAosSubscriber->GetFakeImpus().GetCount(), 3);
}

TEST_F(AosSubscriberTest, GetSubscriberConfig_ManagerNull) {
    SetSubscriberManager(IMS_NULL);
    EXPECT_EQ(GetSubscriberManager(), nullptr);
    EXPECT_EQ(pAosSubscriber->GetSubscriberConfig(), nullptr);
}

TEST_F(AosSubscriberTest, GetSubscriberConfig_ManagerReturn) {
    ISubscriberConfig* piSubscriberConfig;
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    EXPECT_CALL(objMockIAosSubscriberManager, GetSubscriberConfig(_))
        .Times(AnyNumber())
        .WillRepeatedly(ReturnPointee(&piSubscriberConfig));

    SetSubscriberManager(static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));
    EXPECT_NE(GetSubscriberManager(), nullptr);

    EXPECT_EQ(pAosSubscriber->GetSubscriberConfig(), piSubscriberConfig);
}