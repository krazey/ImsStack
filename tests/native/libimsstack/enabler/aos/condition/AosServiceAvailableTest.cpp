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

#include "AosCounter.h"
#include "AosReason.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "condition/AosCondition.h"
#include "condition/AosServiceAvailable.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosServiceAvailableListener.h"

using ::testing::_;
using ::testing::NiceMock;

const IMS_SINT32 SLOT_ID = 0;
const AString NAME = AString("AosServiceAvailable");

#define DECLARE_USING(Base) \
    using Base::SetBlock;   \
    using Base::RequestCommand;

class TestAosServiceAvailable : public AosServiceAvailable
{
private:
    AosCounter* m_pCounter;

public:
    DECLARE_USING(AosServiceAvailable)

    inline explicit TestAosServiceAvailable(const AString& strName) :
            AosServiceAvailable(strName)
    {
        m_pCounter = new AosCounter();
    }

    inline ~TestAosServiceAvailable() override { delete m_pCounter; }

    inline TestAosServiceAvailable(IN const TestAosServiceAvailable&) = delete;
    inline TestAosServiceAvailable& operator=(IN const TestAosServiceAvailable&) = delete;

    inline void SetAppContext(IN IAosAppContext* piContext) { m_piAppContext = piContext; }

    inline ImsList<IAosServiceAvailableListener*> GetListeners() { return m_objListeners; }

    inline void SetListeners(IN ImsList<IAosServiceAvailableListener*> objListeners)
    {
        m_objListeners = objListeners;
    }

    inline IAosBlock* GetBlock() { return m_piBlock; }

    inline void SetAvailableLastNotified(IN IMS_BOOL bAvailable)
    {
        m_bAvailableLastNotified = bAvailable;
    }

    IMS_UINT32 GetInvokedCount(IN const AString& strName) { return m_pCounter->GetCount(strName); }

    // Functions where calls are being counted
    void HandleCallStateChanged(IN IMS_UINT32 nState, IN IMS_SINT32 nStateEx) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosServiceAvailable::HandleCallStateChanged(nState, nStateEx);
    }

    void HandleNetworkStateChanged() override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosServiceAvailable::HandleNetworkStateChanged();
    }

    void HandleBlockChanged(IN IMS_UINT32 nState, IN IMS_UINT32 nStateEx) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosServiceAvailable::HandleBlockChanged(nState, nStateEx);
    }

    void HandleRoamingChanged(IN IMS_UINT32 nState) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosServiceAvailable::HandleRoamingChanged(nState);
    }

    void HandleAirplaneModeChanged(IN IMS_UINT32 nState) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosServiceAvailable::HandleAirplaneModeChanged(nState);
    }

    void HandleVopsChanged(IN IMS_UINT32 nState) override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosServiceAvailable::HandleVopsChanged(nState);
    }

    void HandleWifiConnectionChanged() override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosServiceAvailable::HandleWifiConnectionChanged();
    }

    void HandleLocationInfoChanged() override
    {
        m_pCounter->AddCount(__IMS_FUNC__);
        AosServiceAvailable::HandleLocationInfoChanged();
    }
};

class AosServiceAvailableTest : public ::testing::Test
{
public:
    TestAosServiceAvailable* m_pAosServiceAvailable;

    IAosCallTracker* m_pOriginIAosCallTracker;

    NiceMock<MockIAosCallTracker> m_objMockIAosCallTracker;
    NiceMock<MockIAosAppContext> m_objMockIAosAppContext;
    NiceMock<MockIAosBlock> m_objMockIAosBlock;

    MockIAosServiceAvailableListener m_objListener1;
    MockIAosServiceAvailableListener m_objListener2;
    MockIAosServiceAvailableListener m_objListener3;
    MockIAosServiceAvailableListener m_objListener4;
    MockIAosServiceAvailableListener m_objListener5;

protected:
    void SetUp() override
    {
        m_pAosServiceAvailable = new TestAosServiceAvailable(AString(NAME));
        ASSERT_TRUE(m_pAosServiceAvailable != nullptr);

        // save origin pointer
        m_pOriginIAosCallTracker = AosProvider::GetInstance()->GetCallTracker();
        AosProvider::GetInstance()->SetCallTracker(&m_objMockIAosCallTracker, SLOT_ID);

        m_pAosServiceAvailable->SetBlock(&m_objMockIAosBlock);
    }

    void TearDown() override
    {
        AosProvider::GetInstance()->SetCallTracker(m_pOriginIAosCallTracker, SLOT_ID);
        if (m_pAosServiceAvailable)
        {
            delete m_pAosServiceAvailable;
        }
    }
};

TEST_F(AosServiceAvailableTest, ShouldUpdateMemberVariableWhenCachedContextIsNull)
{
    // GIVEN
    m_pAosServiceAvailable->SetAppContext(IMS_NULL);

    EXPECT_CALL(m_objMockIAosAppContext, GetSlotId());
    EXPECT_CALL(m_objMockIAosAppContext, GetBlock());
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration());
    EXPECT_CALL(m_objMockIAosAppContext, GetConnection());

    // WHEN
    m_pAosServiceAvailable->Init(&m_objMockIAosAppContext);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosServiceAvailableTest, ShouldNotUpdateMemberVariableWhenCachedContextIsNotNull)
{
    // GIVEN
    m_pAosServiceAvailable->SetAppContext(&m_objMockIAosAppContext);

    EXPECT_CALL(m_objMockIAosAppContext, GetSlotId()).Times(0);
    EXPECT_CALL(m_objMockIAosAppContext, GetBlock()).Times(0);
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration()).Times(0);
    EXPECT_CALL(m_objMockIAosAppContext, GetConnection()).Times(0);

    // WHEN
    m_pAosServiceAvailable->Init(&m_objMockIAosAppContext);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosServiceAvailableTest, FailsSetListenerWhenListenerIsNull)
{
    // GIVEN
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 0);

    // WHEN
    m_pAosServiceAvailable->SetListener(IMS_NULL);

    // THEN
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 0);
}

TEST_F(AosServiceAvailableTest, FailsSetListenerWhenSameListener)
{
    // GIVEN
    m_pAosServiceAvailable->SetListener(&m_objListener1);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 1);

    // WHEN
    m_pAosServiceAvailable->SetListener(&m_objListener1);

    // THEN
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 1);
}

TEST_F(AosServiceAvailableTest, SucceedsSetListener)
{
    // GIVEN
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 0);

    // WHEN
    m_pAosServiceAvailable->SetListener(&m_objListener1);
    m_pAosServiceAvailable->SetListener(&m_objListener2);
    m_pAosServiceAvailable->SetListener(&m_objListener3);
    m_pAosServiceAvailable->SetListener(&m_objListener4);
    m_pAosServiceAvailable->SetListener(&m_objListener5);

    // THEN
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 5);
}

TEST_F(AosServiceAvailableTest, FailsRemoveListenerWhenListenerIsNull)
{
    // GIVEN
    m_pAosServiceAvailable->SetListener(&m_objListener1);
    m_pAosServiceAvailable->SetListener(&m_objListener2);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);

    // WHEN
    m_pAosServiceAvailable->RemoveListener(IMS_NULL);

    // THEN
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);
}

TEST_F(AosServiceAvailableTest, FailsRemoveListenerWhenNotMatchedListener)
{
    // GIVEN
    m_pAosServiceAvailable->SetListener(&m_objListener1);
    m_pAosServiceAvailable->SetListener(&m_objListener2);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);

    // WHEN
    m_pAosServiceAvailable->RemoveListener(&m_objListener3);
    m_pAosServiceAvailable->RemoveListener(&m_objListener4);
    m_pAosServiceAvailable->RemoveListener(&m_objListener5);

    // THEN
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 2);
}

TEST_F(AosServiceAvailableTest, SucceedsRemoveListener)
{
    // GIVEN
    m_pAosServiceAvailable->SetListener(&m_objListener1);
    m_pAosServiceAvailable->SetListener(&m_objListener2);
    m_pAosServiceAvailable->SetListener(&m_objListener3);
    m_pAosServiceAvailable->SetListener(&m_objListener4);
    m_pAosServiceAvailable->SetListener(&m_objListener5);
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 5);

    // WHEN
    m_pAosServiceAvailable->RemoveListener(&m_objListener5);
    m_pAosServiceAvailable->RemoveListener(&m_objListener4);
    m_pAosServiceAvailable->RemoveListener(&m_objListener3);
    m_pAosServiceAvailable->RemoveListener(&m_objListener2);
    m_pAosServiceAvailable->RemoveListener(&m_objListener1);

    // THEN
    EXPECT_EQ(m_pAosServiceAvailable->GetListeners().GetSize(), 0);
}

TEST_F(AosServiceAvailableTest, ShouldNotNotifyToListenersWhenAvailableStateIsSame)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, GetBlockReasons(_, _));
    EXPECT_CALL(m_objMockIAosBlock, PrintBlockReasons()).Times(0);

    m_pAosServiceAvailable->SetAvailableLastNotified(IMS_TRUE);

    // WHEN
    m_pAosServiceAvailable->RefreshServiceAvailability();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosServiceAvailableTest, ShouldNotifyToListenersWhenRefreshServiceAvailability)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, GetBlockReasons(_, _));
    EXPECT_CALL(m_objMockIAosBlock, PrintBlockReasons());

    ImsList<IAosServiceAvailableListener*> objListeners;
    objListeners.Append(&m_objListener1);
    objListeners.Append(&m_objListener2);
    objListeners.Append(&m_objListener3);
    m_pAosServiceAvailable->SetListeners(objListeners);

    EXPECT_CALL(m_objListener1, ServiceAvailable_Changed(_));
    EXPECT_CALL(m_objListener2, ServiceAvailable_Changed(_));
    EXPECT_CALL(m_objListener3, ServiceAvailable_Changed(_));

    m_pAosServiceAvailable->SetAvailableLastNotified(IMS_FALSE);

    // WHEN
    m_pAosServiceAvailable->RefreshServiceAvailability();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosServiceAvailableTest, SucceedsInvokeValidFunctionWhenHandleEvent)
{
    // GIVEN
    // WHEN
    m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_CALL, 1, 1);
    m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_NETWORK, 1, 1);
    m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_BLOCK, 1, 1);
    m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_BLOCK_SILENT, 1, 1);
    m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_AIRPLANE, 1, 1);
    m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_ROAMING, 1, 1);
    m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_VOPS, 1, 1);
    m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_LOCATION, 1, 1);
    m_pAosServiceAvailable->HandleEvent(AosServiceAvailable::EVENT_WIFI_STATE, 1, 1);

    // THEN
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleCallStateChanged"), 1);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleNetworkStateChanged"), 1);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleBlockChanged"), 2);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleAirplaneModeChanged"), 1);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleRoamingChanged"), 1);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleVopsChanged"), 1);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleLocationInfoChanged"), 1);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleWifiConnectionChanged"), 1);
}

TEST_F(AosServiceAvailableTest, FailsInvokeFunctionWhenHandleEventWithInvalid)
{
    // GIVEN
    IMS_SINT32 nInvalidEvent = AosServiceAvailable::EVENT_MAX;

    // WHEN
    m_pAosServiceAvailable->HandleEvent(nInvalidEvent, 1, 1);

    // THEN
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleCallStateChanged"), 0);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleNetworkStateChanged"), 0);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleBlockChanged"), 0);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleAirplaneModeChanged"), 0);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleRoamingChanged"), 0);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleVopsChanged"), 0);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleLocationInfoChanged"), 0);
    EXPECT_EQ(m_pAosServiceAvailable->GetInvokedCount("HandleWifiConnectionChanged"), 0);
}

TEST_F(AosServiceAvailableTest, SucceedsNotifyToListenersWhenRequestCommand)
{
    // GIVEN
    IMS_UINT32 nAnyCommand = AosCondition::REQUEST_STOP;
    IMS_UINT32 nAnyReason = AosReason::NOT_SPECIFIED;

    EXPECT_CALL(m_objListener1, ServiceAvailable_RequestCommand(nAnyCommand, nAnyReason)).Times(1);
    EXPECT_CALL(m_objListener2, ServiceAvailable_RequestCommand(nAnyCommand, nAnyReason)).Times(1);
    EXPECT_CALL(m_objListener3, ServiceAvailable_RequestCommand(nAnyCommand, nAnyReason)).Times(1);
    EXPECT_CALL(m_objListener4, ServiceAvailable_RequestCommand(nAnyCommand, nAnyReason)).Times(1);
    EXPECT_CALL(m_objListener5, ServiceAvailable_RequestCommand(nAnyCommand, nAnyReason)).Times(1);

    m_pAosServiceAvailable->SetListener(&m_objListener1);
    m_pAosServiceAvailable->SetListener(&m_objListener2);
    m_pAosServiceAvailable->SetListener(&m_objListener3);
    m_pAosServiceAvailable->SetListener(&m_objListener4);
    m_pAosServiceAvailable->SetListener(&m_objListener5);

    // WHEN
    m_pAosServiceAvailable->RequestCommand(nAnyCommand, nAnyReason);

    // THEN : GIVEN conditions should be met.
}
