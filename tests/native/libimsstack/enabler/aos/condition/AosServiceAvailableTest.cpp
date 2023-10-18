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
class AosServiceAvailableTest : public ::testing::Test
{
public:
    AosServiceAvailable* pAosServiceAvailable;

    IAosCallTracker* pOriginAosCallTracker;
    MockIAosCallTracker objMockAosCallTracker;

protected:
    virtual void SetUp() override
    {
        pAosServiceAvailable = new AosServiceAvailable(AString("AosServiceAvailable"));
        ASSERT_TRUE(pAosServiceAvailable != nullptr);

        // save origin pointer
        pOriginAosCallTracker = AosProvider::GetInstance()->GetCallTracker();
        AosProvider::GetInstance()->SetCallTracker(
                static_cast<IAosCallTracker*>(&objMockAosCallTracker), SLOT_ID);
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetCallTracker(pOriginAosCallTracker, SLOT_ID);
        if (pAosServiceAvailable)
        {
            delete pAosServiceAvailable;
        }
    }

    void SetAppContext(IN IAosAppContext* piAppContext)
    {
        pAosServiceAvailable->m_piAppContext = piAppContext;
    }

    void SetListeners(IN const ImsList<IAosServiceAvailableListener*>& objListeners)
    {
        pAosServiceAvailable->m_objListeners = objListeners;
    }

    ImsList<IAosServiceAvailableListener*> GetListeners()
    {
        return pAosServiceAvailable->m_objListeners;
    }

    void SetAvailableLastNotified(IN IMS_BOOL bNotified)
    {
        pAosServiceAvailable->m_bAvailableLastNotified = bNotified;
    }

    void SetAosBlock(IN IAosBlock* piBlock) { pAosServiceAvailable->m_piBlock = piBlock; }

    IAosBlock* GetAosBlock() { return pAosServiceAvailable->m_piBlock; }

    void RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason)
    {
        pAosServiceAvailable->RequestCommand(nCommand, nReason);
    }
};

TEST_F(AosServiceAvailableTest, Init_MemberContextIsNull)
{
    SetAppContext(IMS_NULL);

    MockIAosAppContext objMockIAosAppContext;
    EXPECT_CALL(objMockIAosAppContext, GetSlotId()).Times(1);
    EXPECT_CALL(objMockIAosAppContext, GetBlock()).Times(1);
    EXPECT_CALL(objMockIAosAppContext, GetRegistration()).Times(1);
    EXPECT_CALL(objMockIAosAppContext, GetConnection()).Times(1);

    pAosServiceAvailable->Init(static_cast<IAosAppContext*>(&objMockIAosAppContext));
}

TEST_F(AosServiceAvailableTest, Init_MemberContextIsNotNull)
{
    MockIAosAppContext objMockIAosAppContext;
    EXPECT_CALL(objMockIAosAppContext, GetSlotId()).Times(0);
    EXPECT_CALL(objMockIAosAppContext, GetBlock()).Times(0);
    EXPECT_CALL(objMockIAosAppContext, GetRegistration()).Times(0);
    EXPECT_CALL(objMockIAosAppContext, GetConnection()).Times(0);

    SetAppContext(static_cast<IAosAppContext*>(&objMockIAosAppContext));

    pAosServiceAvailable->Init(static_cast<IAosAppContext*>(&objMockIAosAppContext));
}

TEST_F(AosServiceAvailableTest, SetListener_ParamIsNull)
{
    pAosServiceAvailable->CleanUp();
    pAosServiceAvailable->SetListener(IMS_NULL);
    EXPECT_EQ(GetListeners().GetSize(), 0);
}

TEST_F(AosServiceAvailableTest, SetListener_AlreadyExist)
{
    pAosServiceAvailable->CleanUp();

    IAosServiceAvailableListener* piListener = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener);
    EXPECT_EQ(GetListeners().GetSize(), 1);

    pAosServiceAvailable->SetListener(piListener);
    EXPECT_EQ(GetListeners().GetSize(), 1);

    pAosServiceAvailable->SetListener(piListener);
    EXPECT_EQ(GetListeners().GetSize(), 1);
}

TEST_F(AosServiceAvailableTest, SetListener_Success)
{
    pAosServiceAvailable->CleanUp();

    IAosServiceAvailableListener* piListener1 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener1);
    EXPECT_EQ(GetListeners().GetSize(), 1);

    IAosServiceAvailableListener* piListener2 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener2);
    EXPECT_EQ(GetListeners().GetSize(), 2);

    IAosServiceAvailableListener* piListener3 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener3);
    EXPECT_EQ(GetListeners().GetSize(), 3);

    IAosServiceAvailableListener* piListener4 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener4);
    EXPECT_EQ(GetListeners().GetSize(), 4);

    IAosServiceAvailableListener* piListener5 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener5);
    EXPECT_EQ(GetListeners().GetSize(), 5);
}

TEST_F(AosServiceAvailableTest, RemoveListener_ParamIsNull)
{
    pAosServiceAvailable->CleanUp();

    IAosServiceAvailableListener* piListener1 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener1);
    EXPECT_EQ(GetListeners().GetSize(), 1);

    IAosServiceAvailableListener* piListener2 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener2);
    EXPECT_EQ(GetListeners().GetSize(), 2);

    pAosServiceAvailable->RemoveListener(IMS_NULL);
    EXPECT_EQ(GetListeners().GetSize(), 2);
}

TEST_F(AosServiceAvailableTest, RemoveListener_NotExist)
{
    pAosServiceAvailable->CleanUp();

    IAosServiceAvailableListener* piListener1 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener1);
    EXPECT_EQ(GetListeners().GetSize(), 1);

    IAosServiceAvailableListener* piListener2 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener2);
    EXPECT_EQ(GetListeners().GetSize(), 2);

    IAosServiceAvailableListener* piListener3 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->RemoveListener(piListener3);
    EXPECT_EQ(GetListeners().GetSize(), 2);

    IAosServiceAvailableListener* piListener4 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->RemoveListener(piListener4);
    EXPECT_EQ(GetListeners().GetSize(), 2);

    IAosServiceAvailableListener* piListener5 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->RemoveListener(piListener5);
    EXPECT_EQ(GetListeners().GetSize(), 2);
}

TEST_F(AosServiceAvailableTest, RemoveListener_Success)
{
    pAosServiceAvailable->CleanUp();

    IAosServiceAvailableListener* piListener1 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener1);
    EXPECT_EQ(GetListeners().GetSize(), 1);

    IAosServiceAvailableListener* piListener2 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener2);
    EXPECT_EQ(GetListeners().GetSize(), 2);

    IAosServiceAvailableListener* piListener3 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener3);
    EXPECT_EQ(GetListeners().GetSize(), 3);

    IAosServiceAvailableListener* piListener4 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener4);
    EXPECT_EQ(GetListeners().GetSize(), 4);

    IAosServiceAvailableListener* piListener5 = new MockIAosServiceAvailableListener();
    pAosServiceAvailable->SetListener(piListener5);
    EXPECT_EQ(GetListeners().GetSize(), 5);

    pAosServiceAvailable->RemoveListener(piListener1);
    EXPECT_EQ(GetListeners().GetSize(), 4);

    pAosServiceAvailable->RemoveListener(piListener4);
    EXPECT_EQ(GetListeners().GetSize(), 3);

    pAosServiceAvailable->RemoveListener(piListener2);
    EXPECT_EQ(GetListeners().GetSize(), 2);

    pAosServiceAvailable->RemoveListener(piListener5);
    EXPECT_EQ(GetListeners().GetSize(), 1);

    pAosServiceAvailable->RemoveListener(piListener3);
    EXPECT_EQ(GetListeners().GetSize(), 0);
}

TEST_F(AosServiceAvailableTest, RefreshServiceAvailablility_NotifySameAsBefore)
{
    MockIAosBlock objMockIAosBlock;
    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    EXPECT_NE(GetAosBlock(), nullptr);
    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, PrintBlockReasons()).Times(0);

    SetAvailableLastNotified(IMS_TRUE);
    pAosServiceAvailable->RefreshServiceAvailablility();
}

TEST_F(AosServiceAvailableTest, RefreshServiceAvailablility_NotifyToListeners)
{
    MockIAosBlock objMockIAosBlock;
    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    EXPECT_NE(GetAosBlock(), nullptr);
    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, PrintBlockReasons()).Times(1);

    MockIAosServiceAvailableListener* pMockIListener1 = new MockIAosServiceAvailableListener();
    MockIAosServiceAvailableListener* pMockIListener2 = new MockIAosServiceAvailableListener();
    MockIAosServiceAvailableListener* pMockIListener3 = new MockIAosServiceAvailableListener();

    ImsList<IAosServiceAvailableListener*> objListeners;
    objListeners.Append(static_cast<IAosServiceAvailableListener*>(pMockIListener1));
    objListeners.Append(static_cast<IAosServiceAvailableListener*>(pMockIListener2));
    objListeners.Append(static_cast<IAosServiceAvailableListener*>(pMockIListener3));
    SetListeners(objListeners);

    EXPECT_CALL(*pMockIListener1, ServiceAvailable_Changed()).Times(1);
    EXPECT_CALL(*pMockIListener2, ServiceAvailable_Changed()).Times(1);
    EXPECT_CALL(*pMockIListener3, ServiceAvailable_Changed()).Times(1);

    SetAvailableLastNotified(IMS_FALSE);
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

    pAosServiceAvailable->SetListener(static_cast<IAosServiceAvailableListener*>(&objIListener1));
    pAosServiceAvailable->SetListener(static_cast<IAosServiceAvailableListener*>(&objIListener2));
    pAosServiceAvailable->SetListener(static_cast<IAosServiceAvailableListener*>(&objIListener3));
    EXPECT_EQ(GetListeners().GetSize(), 3);

    RequestCommand(AosCondition::REQUEST_STOP, AosReason::NOT_SPECIFIED);
    RequestCommand(AosCondition::REQUEST_DESTROY, AosReason::NOT_SPECIFIED);
    RequestCommand(AosCondition::REQUEST_RECOVER, AosReason::NOT_SPECIFIED);
    RequestCommand(AosCondition::REQUEST_PDN_DISCONNECT, AosReason::NOT_SPECIFIED);
}