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

#include "AosReason.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosServiceAvailableListener.h"
#include "condition/AosBlock.h"
#include "condition/AosCondition.h"
#include "condition/AosServiceAvailable.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosServiceAvailableListener.h"

using ::testing::_;

const IMS_SINT32 SLOT_ID = 0;

class TestAosServiceAvailable : public AosServiceAvailable
{
public:
    inline explicit TestAosServiceAvailable(const AString& strName) :
            AosServiceAvailable(strName)
    {
    }

    FRIEND_TEST(AosServiceAvailableTest, Init_MemberContextIsNull);
    FRIEND_TEST(AosServiceAvailableTest, Init_MemberContextIsNotNull);
    FRIEND_TEST(AosServiceAvailableTest, SetListener_ParamIsNull);
    FRIEND_TEST(AosServiceAvailableTest, SetListener_AlreadyExist);
    FRIEND_TEST(AosServiceAvailableTest, SetListener_Success);
    FRIEND_TEST(AosServiceAvailableTest, RemoveListener_ParamIsNull);
    FRIEND_TEST(AosServiceAvailableTest, RemoveListener_NotExist);
    FRIEND_TEST(AosServiceAvailableTest, RemoveListener_Success);
    FRIEND_TEST(AosServiceAvailableTest, RefreshServiceAvailablility_NotifySameAsBefore);
    FRIEND_TEST(AosServiceAvailableTest, RefreshServiceAvailablility_NotifyToListeners);
    FRIEND_TEST(AosServiceAvailableTest, HandleEvent_Valid);
    FRIEND_TEST(AosServiceAvailableTest, HandleEvent_Invalid);
    FRIEND_TEST(AosServiceAvailableTest, RequestCommand);
};

class AosServiceAvailableTest : public ::testing::Test
{
public:
    TestAosServiceAvailable* pAosServiceAvailable;

    IAosCallTracker* pOriginAosCallTracker;
    MockIAosCallTracker objMockAosCallTracker;

protected:
    virtual void SetUp() override
    {
        pAosServiceAvailable = new TestAosServiceAvailable(AString("AosServiceAvailable"));
        ASSERT_TRUE(pAosServiceAvailable != nullptr);

        // save origin pointer
        pOriginAosCallTracker = AosProvider::GetInstance()->GetCallTracker();
        AosProvider::GetInstance()->SetCallTracker(&objMockAosCallTracker, SLOT_ID);
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetCallTracker(pOriginAosCallTracker, SLOT_ID);
        if (pAosServiceAvailable)
        {
            delete pAosServiceAvailable;
        }
    }
};

TEST_F(AosServiceAvailableTest, Init_MemberContextIsNull)
{
    pAosServiceAvailable->m_piAppContext = IMS_NULL;

    MockIAosAppContext objMockIAosAppContext;
    EXPECT_CALL(objMockIAosAppContext, GetSlotId()).Times(1);
    EXPECT_CALL(objMockIAosAppContext, GetBlock()).Times(1);
    EXPECT_CALL(objMockIAosAppContext, GetRegistration()).Times(1);
    EXPECT_CALL(objMockIAosAppContext, GetConnection()).Times(1);

    pAosServiceAvailable->Init(&objMockIAosAppContext);
}

TEST_F(AosServiceAvailableTest, Init_MemberContextIsNotNull)
{
    MockIAosAppContext objMockIAosAppContext;
    EXPECT_CALL(objMockIAosAppContext, GetSlotId()).Times(0);
    EXPECT_CALL(objMockIAosAppContext, GetBlock()).Times(0);
    EXPECT_CALL(objMockIAosAppContext, GetRegistration()).Times(0);
    EXPECT_CALL(objMockIAosAppContext, GetConnection()).Times(0);

    pAosServiceAvailable->m_piAppContext = &objMockIAosAppContext;

    pAosServiceAvailable->Init(&objMockIAosAppContext);
}

TEST_F(AosServiceAvailableTest, SetListener_ParamIsNull)
{
    pAosServiceAvailable->CleanUp();
    pAosServiceAvailable->SetListener(IMS_NULL);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 0);
}

TEST_F(AosServiceAvailableTest, SetListener_AlreadyExist)
{
    pAosServiceAvailable->CleanUp();

    IAosServiceAvailableListener* piListener = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 1);

    pAosServiceAvailable->SetListener(piListener);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 1);

    pAosServiceAvailable->SetListener(piListener);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 1);
}

TEST_F(AosServiceAvailableTest, SetListener_Success)
{
    pAosServiceAvailable->CleanUp();

    IAosServiceAvailableListener* piListener1 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener1);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 1);

    IAosServiceAvailableListener* piListener2 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener2);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 2);

    IAosServiceAvailableListener* piListener3 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener3);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 3);

    IAosServiceAvailableListener* piListener4 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener4);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 4);

    IAosServiceAvailableListener* piListener5 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener5);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 5);
}

TEST_F(AosServiceAvailableTest, RemoveListener_ParamIsNull)
{
    pAosServiceAvailable->CleanUp();

    IAosServiceAvailableListener* piListener1 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener1);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 1);

    IAosServiceAvailableListener* piListener2 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener2);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 2);

    pAosServiceAvailable->RemoveListener(IMS_NULL);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 2);
}

TEST_F(AosServiceAvailableTest, RemoveListener_NotExist)
{
    pAosServiceAvailable->CleanUp();

    IAosServiceAvailableListener* piListener1 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener1);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 1);

    IAosServiceAvailableListener* piListener2 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener2);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 2);

    IAosServiceAvailableListener* piListener3 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->RemoveListener(piListener3);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 2);

    IAosServiceAvailableListener* piListener4 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->RemoveListener(piListener4);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 2);

    IAosServiceAvailableListener* piListener5 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->RemoveListener(piListener5);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 2);
}

TEST_F(AosServiceAvailableTest, RemoveListener_Success)
{
    pAosServiceAvailable->CleanUp();

    IAosServiceAvailableListener* piListener1 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener1);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 1);

    IAosServiceAvailableListener* piListener2 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener2);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 2);

    IAosServiceAvailableListener* piListener3 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener3);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 3);

    IAosServiceAvailableListener* piListener4 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener4);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 4);

    IAosServiceAvailableListener* piListener5 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener5);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 5);

    pAosServiceAvailable->RemoveListener(piListener1);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 4);

    pAosServiceAvailable->RemoveListener(piListener4);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 3);

    pAosServiceAvailable->RemoveListener(piListener2);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 2);

    pAosServiceAvailable->RemoveListener(piListener5);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 1);

    pAosServiceAvailable->RemoveListener(piListener3);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 0);
}

TEST_F(AosServiceAvailableTest, RefreshServiceAvailablility_NotifySameAsBefore)
{
    MockIAosBlock objMockIAosBlock;
    pAosServiceAvailable->m_piBlock = &objMockIAosBlock;

    EXPECT_NE(pAosServiceAvailable->m_piBlock, nullptr);
    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, PrintBlockReasons()).Times(0);

    pAosServiceAvailable->m_bAvailableLastNotified = IMS_TRUE;
    pAosServiceAvailable->RefreshServiceAvailablility();
}

TEST_F(AosServiceAvailableTest, RefreshServiceAvailablility_NotifyToListeners)
{
    MockIAosBlock objMockIAosBlock;
    pAosServiceAvailable->m_piBlock = &objMockIAosBlock;

    EXPECT_NE(pAosServiceAvailable->m_piBlock, nullptr);
    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, PrintBlockReasons()).Times(1);

    MockIAosServiceAvailableListener objMockIListener1;
    MockIAosServiceAvailableListener objMockIListener2;
    MockIAosServiceAvailableListener objMockIListener3;

    ImsList<IAosServiceAvailableListener*> objListeners;
    objListeners.Append(&objMockIListener1);
    objListeners.Append(&objMockIListener2);
    objListeners.Append(&objMockIListener3);
    pAosServiceAvailable->m_objListeners = objListeners;

    EXPECT_CALL(objMockIListener1, ServiceAvailable_Changed()).Times(1);
    EXPECT_CALL(objMockIListener2, ServiceAvailable_Changed()).Times(1);
    EXPECT_CALL(objMockIListener3, ServiceAvailable_Changed()).Times(1);

    pAosServiceAvailable->m_bAvailableLastNotified = IMS_FALSE;
    pAosServiceAvailable->RefreshServiceAvailablility();
}

TEST_F(AosServiceAvailableTest, HandleEvent_Valid)
{
    EXPECT_EQ(pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_AIRPLANE, 1, 1),
            AosServiceAvailable::EVENT_AIRPLANE);
    EXPECT_EQ(pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_AIRPLANE, 0, 1),
            AosServiceAvailable::EVENT_AIRPLANE);
    EXPECT_EQ(pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_ROAMING, 1, 1),
            AosServiceAvailable::EVENT_ROAMING);
    EXPECT_EQ(pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_ROAMING, 0, 1),
            AosServiceAvailable::EVENT_ROAMING);

    EXPECT_EQ(pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_VOPS, 1, 1),
            AosServiceAvailable::EVENT_VOPS);
    EXPECT_EQ(pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_LOCATION, 1, 1),
            AosServiceAvailable::EVENT_LOCATION);
    EXPECT_EQ(pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_CALL, 1, 1),
            AosServiceAvailable::EVENT_CALL);
    EXPECT_EQ(pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_NETWORK, 1, 1),
            AosServiceAvailable::EVENT_NETWORK);
    EXPECT_EQ(pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_WIFI_STATE, 1, 1),
            AosServiceAvailable::EVENT_WIFI_STATE);
}

TEST_F(AosServiceAvailableTest, HandleEvent_Invalid)
{
    EXPECT_EQ(pAosServiceAvailable->HandleEvent(-1, 1, 1), -1);
    EXPECT_EQ(pAosServiceAvailable->HandleEvent(100, 1, 1), -1);
}

TEST_F(AosServiceAvailableTest, RequestCommand)
{
    pAosServiceAvailable->CleanUp();

    MockIAosServiceAvailableListener objIListener1;
    MockIAosServiceAvailableListener objIListener2;
    MockIAosServiceAvailableListener objIListener3;

    EXPECT_CALL(objIListener1, ServiceAvailable_RequestCommand(_, _)).Times(4);

    EXPECT_CALL(objIListener2, ServiceAvailable_RequestCommand(_, _)).Times(4);

    EXPECT_CALL(objIListener3, ServiceAvailable_RequestCommand(_, _)).Times(4);

    pAosServiceAvailable->SetListener(&objIListener1);
    pAosServiceAvailable->SetListener(&objIListener2);
    pAosServiceAvailable->SetListener(&objIListener3);
    EXPECT_EQ(pAosServiceAvailable->m_objListeners.GetSize(), 3);

    pAosServiceAvailable->RequestCommand(AosCondition::REQUEST_STOP, AosReason::NOT_SPECIFIED);
    pAosServiceAvailable->RequestCommand(AosCondition::REQUEST_DESTROY, AosReason::NOT_SPECIFIED);
    pAosServiceAvailable->RequestCommand(AosCondition::REQUEST_RECOVER, AosReason::NOT_SPECIFIED);
    pAosServiceAvailable->RequestCommand(
            AosCondition::REQUEST_PDN_DISCONNECT, AosReason::NOT_SPECIFIED);
}
