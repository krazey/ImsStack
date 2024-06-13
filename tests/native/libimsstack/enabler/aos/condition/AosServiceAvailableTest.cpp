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

    inline void RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason)
    {
        AosServiceAvailable::RequestCommand(nCommand, nReason);
    }

    inline void SetAppContext(IN IAosAppContext* piContext) { m_piAppContext = piContext; }

    inline ImsList<IAosServiceAvailableListener*> GetListeners() { return m_objListeners; }

    inline void SetListeners(IN ImsList<IAosServiceAvailableListener*> objListeners)
    {
        m_objListeners = objListeners;
    }

    inline IAosBlock* GetBlock() { return m_piBlock; }

    inline void SetBlock(IN IAosBlock* piBlock) { m_piBlock = piBlock; }

    inline void SetAvailableLastNotified(IN IMS_BOOL bAvailable)
    {
        m_bAvailableLastNotified = bAvailable;
    }
};

class AosServiceAvailableTest : public ::testing::Test
{
public:
    TestAosServiceAvailable* m_pAosServiceAvailable;

    IAosCallTracker* m_pOriginIAosCallTracker;
    MockIAosCallTracker m_objMockIAosCallTracker;

    MockIAosServiceAvailableListener m_objListener1;
    MockIAosServiceAvailableListener m_objListener2;
    MockIAosServiceAvailableListener m_objListener3;
    MockIAosServiceAvailableListener m_objListener4;
    MockIAosServiceAvailableListener m_objListener5;

protected:
    virtual void SetUp() override
    {
        m_pAosServiceAvailable = new TestAosServiceAvailable(AString("AosServiceAvailable"));
        ASSERT_TRUE(m_pAosServiceAvailable != nullptr);

        // save origin pointer
        m_pOriginIAosCallTracker = AosProvider::GetInstance()->GetCallTracker();
        AosProvider::GetInstance()->SetCallTracker(&m_objMockIAosCallTracker, SLOT_ID);
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetCallTracker(m_pOriginIAosCallTracker, SLOT_ID);
        if (m_pAosServiceAvailable)
        {
            delete m_pAosServiceAvailable;
        }
    }
};

TEST_F(AosServiceAvailableTest, Init_MemberContextIsNull)
{
    m_pAosServiceAvailable->SetAppContext(IMS_NULL);

    MockIAosAppContext objMockIAosAppContext;
    EXPECT_CALL(objMockIAosAppContext, GetSlotId()).Times(1);
    EXPECT_CALL(objMockIAosAppContext, GetBlock()).Times(1);
    EXPECT_CALL(objMockIAosAppContext, GetRegistration()).Times(1);
    EXPECT_CALL(objMockIAosAppContext, GetConnection()).Times(1);

    m_pAosServiceAvailable->Init(&objMockIAosAppContext);
}

TEST_F(AosServiceAvailableTest, Init_MemberContextIsNotNull)
{
    MockIAosAppContext objMockIAosAppContext;
    EXPECT_CALL(objMockIAosAppContext, GetSlotId()).Times(0);
    EXPECT_CALL(objMockIAosAppContext, GetBlock()).Times(0);
    EXPECT_CALL(objMockIAosAppContext, GetRegistration()).Times(0);
    EXPECT_CALL(objMockIAosAppContext, GetConnection()).Times(0);

    m_pAosServiceAvailable->SetAppContext(&objMockIAosAppContext);

    m_pAosServiceAvailable->Init(&objMockIAosAppContext);
}

TEST_F(AosServiceAvailableTest, SetListener_ParamIsNull)
{
    m_pAosServiceAvailable->CleanUp();
    m_pAosServiceAvailable->SetListener(IMS_NULL);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 0);
}

TEST_F(AosServiceAvailableTest, SetListener_AlreadyExist)
{
    m_pAosServiceAvailable->CleanUp();

    m_pAosServiceAvailable->SetListener(&m_objListener1);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 1);

    m_pAosServiceAvailable->SetListener(&m_objListener1);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 1);

    m_pAosServiceAvailable->SetListener(&m_objListener1);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 1);
}

TEST_F(AosServiceAvailableTest, SetListener_Success)
{
    m_pAosServiceAvailable->CleanUp();

    m_pAosServiceAvailable->SetListener(&m_objListener1);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 1);

    m_pAosServiceAvailable->SetListener(&m_objListener2);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);

    m_pAosServiceAvailable->SetListener(&m_objListener3);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 3);

    m_pAosServiceAvailable->SetListener(&m_objListener4);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 4);

    m_pAosServiceAvailable->SetListener(&m_objListener5);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 5);
}

TEST_F(AosServiceAvailableTest, RemoveListener_ParamIsNull)
{
    m_pAosServiceAvailable->CleanUp();

    m_pAosServiceAvailable->SetListener(&m_objListener1);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 1);

    m_pAosServiceAvailable->SetListener(&m_objListener2);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);

    m_pAosServiceAvailable->RemoveListener(IMS_NULL);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);
}

TEST_F(AosServiceAvailableTest, RemoveListener_NotExist)
{
    m_pAosServiceAvailable->CleanUp();

    m_pAosServiceAvailable->SetListener(&m_objListener1);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 1);

    m_pAosServiceAvailable->SetListener(&m_objListener2);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);

    m_pAosServiceAvailable->RemoveListener(&m_objListener3);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);

    m_pAosServiceAvailable->RemoveListener(&m_objListener4);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);

    m_pAosServiceAvailable->RemoveListener(&m_objListener5);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);
}

TEST_F(AosServiceAvailableTest, RemoveListener_Success)
{
    m_pAosServiceAvailable->CleanUp();

    m_pAosServiceAvailable->SetListener(&m_objListener1);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 1);

    m_pAosServiceAvailable->SetListener(&m_objListener2);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);

    m_pAosServiceAvailable->SetListener(&m_objListener3);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 3);

    m_pAosServiceAvailable->SetListener(&m_objListener4);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 4);

    m_pAosServiceAvailable->SetListener(&m_objListener5);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 5);

    m_pAosServiceAvailable->RemoveListener(&m_objListener1);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 4);

    m_pAosServiceAvailable->RemoveListener(&m_objListener4);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 3);

    m_pAosServiceAvailable->RemoveListener(&m_objListener2);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);

    m_pAosServiceAvailable->RemoveListener(&m_objListener5);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 1);

    m_pAosServiceAvailable->RemoveListener(&m_objListener3);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 0);
}

TEST_F(AosServiceAvailableTest, RefreshServiceAvailablility_NotifySameAsBefore)
{
    MockIAosBlock objMockIAosBlock;
    m_pAosServiceAvailable->SetBlock(&objMockIAosBlock);

    EXPECT_NE(m_pAosServiceAvailable->GetBlock(), nullptr);
    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, PrintBlockReasons()).Times(0);

    m_pAosServiceAvailable->SetAvailableLastNotified(IMS_TRUE);
    m_pAosServiceAvailable->RefreshServiceAvailablility();
}

TEST_F(AosServiceAvailableTest, RefreshServiceAvailablility_NotifyToListeners)
{
    MockIAosBlock objMockIAosBlock;
    m_pAosServiceAvailable->SetBlock(&objMockIAosBlock);

    EXPECT_NE(m_pAosServiceAvailable->GetBlock(), nullptr);
    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, PrintBlockReasons()).Times(1);

    ImsList<IAosServiceAvailableListener*> objListeners;
    objListeners.Append(&m_objListener1);
    objListeners.Append(&m_objListener2);
    objListeners.Append(&m_objListener3);
    m_pAosServiceAvailable->SetListeners(objListeners);

    EXPECT_CALL(m_objListener1, ServiceAvailable_Changed()).Times(1);
    EXPECT_CALL(m_objListener2, ServiceAvailable_Changed()).Times(1);
    EXPECT_CALL(m_objListener3, ServiceAvailable_Changed()).Times(1);

    m_pAosServiceAvailable->SetAvailableLastNotified(IMS_FALSE);
    m_pAosServiceAvailable->RefreshServiceAvailablility();
}

TEST_F(AosServiceAvailableTest, HandleEvent_Valid)
{
    EXPECT_EQ(m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_AIRPLANE, 1, 1),
            AosServiceAvailable::EVENT_AIRPLANE);
    EXPECT_EQ(m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_AIRPLANE, 0, 1),
            AosServiceAvailable::EVENT_AIRPLANE);
    EXPECT_EQ(m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_ROAMING, 1, 1),
            AosServiceAvailable::EVENT_ROAMING);
    EXPECT_EQ(m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_ROAMING, 0, 1),
            AosServiceAvailable::EVENT_ROAMING);

    EXPECT_EQ(m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_VOPS, 1, 1),
            AosServiceAvailable::EVENT_VOPS);
    EXPECT_EQ(m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_LOCATION, 1, 1),
            AosServiceAvailable::EVENT_LOCATION);
    EXPECT_EQ(m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_CALL, 1, 1),
            AosServiceAvailable::EVENT_CALL);
    EXPECT_EQ(m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_NETWORK, 1, 1),
            AosServiceAvailable::EVENT_NETWORK);
    EXPECT_EQ(m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_WIFI_STATE, 1, 1),
            AosServiceAvailable::EVENT_WIFI_STATE);
}

TEST_F(AosServiceAvailableTest, HandleEvent_Invalid)
{
    EXPECT_EQ(m_pAosServiceAvailable->HandleEvent(-1, 1, 1), -1);
    EXPECT_EQ(m_pAosServiceAvailable->HandleEvent(100, 1, 1), -1);
}

TEST_F(AosServiceAvailableTest, RequestCommand)
{
    m_pAosServiceAvailable->CleanUp();

    EXPECT_CALL(m_objListener1, ServiceAvailable_RequestCommand(_, _)).Times(4);

    EXPECT_CALL(m_objListener2, ServiceAvailable_RequestCommand(_, _)).Times(4);

    EXPECT_CALL(m_objListener3, ServiceAvailable_RequestCommand(_, _)).Times(4);

    m_pAosServiceAvailable->SetListener(&m_objListener1);
    m_pAosServiceAvailable->SetListener(&m_objListener2);
    m_pAosServiceAvailable->SetListener(&m_objListener3);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 3);

    m_pAosServiceAvailable->RequestCommand(AosCondition::REQUEST_STOP, AosReason::NOT_SPECIFIED);
    m_pAosServiceAvailable->RequestCommand(AosCondition::REQUEST_DESTROY, AosReason::NOT_SPECIFIED);
    m_pAosServiceAvailable->RequestCommand(AosCondition::REQUEST_RECOVER, AosReason::NOT_SPECIFIED);
    m_pAosServiceAvailable->RequestCommand(
            AosCondition::REQUEST_PDN_DISCONNECT, AosReason::NOT_SPECIFIED);
}
