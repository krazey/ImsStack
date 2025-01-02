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

#include "ImsEventDef.h"
#include "provider/AosCallTracker.h"
#include "provider/AosProvider.h"
#include "../../interface/aos/IAosService.h"
#include "../../interface/mtc/IMtcCallStateListener.h"

#include "interface/MockIAosCallTrackerListener.h"

#include "../../interface/aos/MockIAosService.h"

using ::testing::_;
using ::testing::AnyNumber;

const IMS_SINT32 SLOT_ID = 0;

#define DECLARE_USING(Base)                           \
    using Base::AddOrUpdateCall;                      \
    using Base::GetConvertedState;                    \
    using Base::GetTotalState;                        \
    using Base::GetTotalCallType;                     \
    using Base::SetState;                             \
    using Base::Notify;                               \
    using Base::ProcessCsChanged;                     \
    using Base::ProcessEmergencyChanged;              \
    using Base::ProcessNormalChanged;                 \
    using Base::Event_NotifyEvent;                    \
    using Base::ServicePhone_PreciseCallStateChanged; \
    using Base::OnCallStateChanged;                   \
    using Base::OnTotalCallStateChanged;              \
    using Base::TypeToString;                         \
    using Base::StateToString;                        \
    using Base::CallTypeToString;                     \
    using Base::PrintCallTypes;

class TestAosCallTracker : public AosCallTracker
{
public:
    DECLARE_USING(AosCallTracker)

    inline explicit TestAosCallTracker(IN IMS_SINT32 nSlotId) :
            AosCallTracker(nSlotId)
    {
    }

    inline void SetNormalCallType(IN IMS_UINT32 nType) { m_nNormalCallType = nType; }

    inline void SetNormalCalls(IN ImsMap<CallKey, CallState> objCalls)
    {
        m_objNormalCalls = objCalls;
    }

    inline void SetNormalCallTypes(IN ImsMap<CallKey, CallType> objCallTypes)
    {
        m_objNormalCallTypes = objCallTypes;
    }

    inline void SetSlotId(IN IMS_SINT32 nSlotId) { m_nSlotId = nSlotId; }

    inline void SetReady(IN IMS_BOOL bReady) { m_bMtcReady = bReady; }

    inline ImsList<IAosCallTrackerListener*> GetListeners() { return m_objListeners; }
};

class AosCallTrackerTest : public ::testing::Test
{
public:
    TestAosCallTracker* m_pAosCallTracker;
    IAosService* m_piOriginService;
    MockIAosService m_objMockIAosService;

protected:
    virtual void SetUp() override
    {
        m_pAosCallTracker = new TestAosCallTracker(SLOT_ID);
        ASSERT_TRUE(m_pAosCallTracker != nullptr);

        // Store origin AosService
        m_piOriginService = AosProvider::GetInstance()->GetService();

        // Set MockIAosService
        AosProvider::GetInstance()->SetService(&m_objMockIAosService);
    }

    virtual void TearDown() override
    {
        if (m_pAosCallTracker)
        {
            delete m_pAosCallTracker;
        }

        AosProvider::GetInstance()->SetService(m_piOriginService);
    }
};

TEST_F(AosCallTrackerTest, ShouldAddCallStateListenerIfNotAdded)
{
    // WHEN
    IMS_BOOL bResult = m_pAosCallTracker->SetMtcReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosCallTrackerTest, ShouldNotAddCallStateListenerIfAlreadyAdded)
{
    // GIVEN
    m_pAosCallTracker->SetReady(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult = m_pAosCallTracker->SetMtcReady();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosCallTrackerTest, ReturnsFalseWhenCsCallStateIsSmallerThenActiveCsState)
{
    // GIVEN
    m_pAosCallTracker->SetActiveCsCallState(CallState::RINGING);

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    IMS_BOOL bResult1 = m_pAosCallTracker->IsCsCallActive();

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::NEW);
    IMS_BOOL bResult2 = m_pAosCallTracker->IsCsCallActive();

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::RINGBACK);
    IMS_BOOL bResult3 = m_pAosCallTracker->IsCsCallActive();

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::RINGING);
    IMS_BOOL bResult4 = m_pAosCallTracker->IsCsCallActive();

    // THEN
    EXPECT_FALSE(bResult1);
    EXPECT_FALSE(bResult2);
    EXPECT_FALSE(bResult3);
    EXPECT_FALSE(bResult4);
}

TEST_F(AosCallTrackerTest, ReturnsTrueWhenCsCallStateBiggerThenActiveCsState)
{
    // GIVEN
    m_pAosCallTracker->SetActiveCsCallState(CallState::RINGING);

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::ALERTING);
    IMS_BOOL bResult1 = m_pAosCallTracker->IsCsCallActive();

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    IMS_BOOL bResult2 = m_pAosCallTracker->IsCsCallActive();

    // THEN
    EXPECT_TRUE(bResult1);
    EXPECT_TRUE(bResult2);
}

TEST_F(AosCallTrackerTest, ReturnsFalseWhenActiveCallStateBiggerThenCsCallState)
{
    // GIVEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::RINGING);

    // WHEN
    m_pAosCallTracker->SetActiveCsCallState(CallState::RINGING);
    IMS_BOOL bResult1 = m_pAosCallTracker->IsCsCallActive();

    m_pAosCallTracker->SetActiveCsCallState(CallState::ALERTING);
    IMS_BOOL bResult2 = m_pAosCallTracker->IsCsCallActive();

    m_pAosCallTracker->SetActiveCsCallState(CallState::OFFHOOK);
    IMS_BOOL bResult3 = m_pAosCallTracker->IsCsCallActive();

    // THEN
    EXPECT_FALSE(bResult1);
    EXPECT_FALSE(bResult2);
    EXPECT_FALSE(bResult3);
}

TEST_F(AosCallTrackerTest, ReturnsTrueWhenActiveCallStateSmallerThenCsCallState)
{
    // GIVEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::RINGING);

    // WHEN
    m_pAosCallTracker->SetActiveCsCallState(CallState::IDLE);
    IMS_BOOL bResult1 = m_pAosCallTracker->IsCsCallActive();

    m_pAosCallTracker->SetActiveCsCallState(CallState::NEW);
    IMS_BOOL bResult2 = m_pAosCallTracker->IsCsCallActive();

    m_pAosCallTracker->SetActiveCsCallState(CallState::RINGBACK);
    IMS_BOOL bResult3 = m_pAosCallTracker->IsCsCallActive();

    // THEN
    EXPECT_TRUE(bResult1);
    EXPECT_TRUE(bResult2);
    EXPECT_TRUE(bResult3);
}

TEST_F(AosCallTrackerTest, IsNormalCallActiveReturnsTrueWhenNormalCallStateIsNew)
{
    // GIVEN
    CallState eCallState = CallState::NEW;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, eCallState);

    // THEN
    EXPECT_TRUE(m_pAosCallTracker->IsNormalCallActive());
}

TEST_F(AosCallTrackerTest, IsNormalCallActiveReturnsTrueWhenNormalCallStateIsRingback)
{
    // GIVEN
    CallState eCallState = CallState::RINGBACK;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, eCallState);

    // THEN
    EXPECT_TRUE(m_pAosCallTracker->IsNormalCallActive());
}

TEST_F(AosCallTrackerTest, IsNormalCallActiveReturnsTrueWhenNormalCallStateIsRinging)
{
    // GIVEN
    CallState eCallState = CallState::RINGING;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, eCallState);

    // THEN
    EXPECT_TRUE(m_pAosCallTracker->IsNormalCallActive());
}

TEST_F(AosCallTrackerTest, IsNormalCallActiveReturnsTrueWhenNormalCallStateIsAlerting)
{
    // GIVEN
    CallState eCallState = CallState::ALERTING;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, eCallState);

    // THEN
    EXPECT_TRUE(m_pAosCallTracker->IsNormalCallActive());
}

TEST_F(AosCallTrackerTest, IsNormalCallActiveReturnsTrueWhenNormalCallStateIsOffhook)
{
    // GIVEN
    CallState eCallState = CallState::OFFHOOK;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, eCallState);

    // THEN
    EXPECT_TRUE(m_pAosCallTracker->IsNormalCallActive());
}

TEST_F(AosCallTrackerTest, IsNormalCallActiveReturnsFalseWhenNormalCallStateIsIdle)
{
    // GIVEN
    CallState eCallState = CallState::IDLE;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, eCallState);

    // THEN
    EXPECT_FALSE(m_pAosCallTracker->IsNormalCallActive());
}

TEST_F(AosCallTrackerTest, IsEmergencyCallActiveReturnsTrueWhenEmergencyCallStateIsNew)
{
    // GIVEN
    CallState eCallState = CallState::NEW;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, eCallState);

    // THEN
    EXPECT_TRUE(m_pAosCallTracker->IsEmergencyCallActive());
}

TEST_F(AosCallTrackerTest, IsEmergencyCallActiveReturnsTrueWhenEmergencyCallStateIsRingback)
{
    // GIVEN
    CallState eCallState = CallState::RINGBACK;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, eCallState);

    // THEN
    EXPECT_TRUE(m_pAosCallTracker->IsEmergencyCallActive());
}

TEST_F(AosCallTrackerTest, IsEmergencyCallActiveReturnsTrueWhenEmergencyCallStateIsRinging)
{
    // GIVEN
    CallState eCallState = CallState::RINGING;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, eCallState);

    // THEN
    EXPECT_TRUE(m_pAosCallTracker->IsEmergencyCallActive());
}

TEST_F(AosCallTrackerTest, IsEmergencyCallActiveReturnsTrueWhenEmergencyCallStateIsAlerting)
{
    // GIVEN
    CallState eCallState = CallState::ALERTING;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, eCallState);

    // THEN
    EXPECT_TRUE(m_pAosCallTracker->IsEmergencyCallActive());
}

TEST_F(AosCallTrackerTest, IsEmergencyCallActiveReturnsTrueWhenEmergencyCallStateIsOffhook)
{
    // GIVEN
    CallState eCallState = CallState::OFFHOOK;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, eCallState);

    // THEN
    EXPECT_TRUE(m_pAosCallTracker->IsEmergencyCallActive());
}

TEST_F(AosCallTrackerTest, IsEmergencyCallActiveReturnsFalseWhenEmergencyCallStateIsIdle)
{
    // GIVEN
    CallState eCallState = CallState::IDLE;

    // WHEN
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, eCallState);

    // THEN
    EXPECT_FALSE(m_pAosCallTracker->IsEmergencyCallActive());
}

TEST_F(AosCallTrackerTest, IsVideoCallingActive_returnTrue)
{
    IMS_UINT32 nNormalCallType = (0x1 << static_cast<IMS_UINT32>(CallType::VT)) |
            (0x1 << static_cast<IMS_UINT32>(CallType::VOIP));
    m_pAosCallTracker->SetNormalCallType(nNormalCallType);

    ImsMap<CallKey, CallState> objNormalCalls;
    ImsMap<CallKey, CallType> objNormalCallTypes;
    IMS_ULONG nKey1 = 1001;
    IMS_ULONG nKey2 = 1002;

    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey1, CallState::RINGBACK);
    m_pAosCallTracker->AddOrUpdateCall(objNormalCallTypes, nKey1, CallType::VOIP);

    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey2, CallState::OFFHOOK);
    m_pAosCallTracker->AddOrUpdateCall(objNormalCallTypes, nKey2, CallType::VT);

    m_pAosCallTracker->SetNormalCalls(objNormalCalls);
    m_pAosCallTracker->SetNormalCallTypes(objNormalCallTypes);

    EXPECT_TRUE(m_pAosCallTracker->IsVideoCallingActive());
}

TEST_F(AosCallTrackerTest, IsVideoCallingActive_returnFalse_VtIsNotOffhook)
{
    IMS_UINT32 nNormalCallType = (0x1 << static_cast<IMS_UINT32>(CallType::VT)) |
            (0x1 << static_cast<IMS_UINT32>(CallType::VOIP));
    m_pAosCallTracker->SetNormalCallType(nNormalCallType);

    ImsMap<CallKey, CallState> objNormalCalls;
    ImsMap<CallKey, CallType> objNormalCallTypes;
    IMS_ULONG nKey1 = 1001;
    IMS_ULONG nKey2 = 1002;

    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey1, CallState::RINGING);
    m_pAosCallTracker->AddOrUpdateCall(objNormalCallTypes, nKey1, CallType::VOIP);

    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey2, CallState::IDLE);
    m_pAosCallTracker->AddOrUpdateCall(objNormalCallTypes, nKey2, CallType::VT);

    m_pAosCallTracker->SetNormalCalls(objNormalCalls);
    m_pAosCallTracker->SetNormalCallTypes(objNormalCallTypes);

    EXPECT_FALSE(m_pAosCallTracker->IsVideoCallingActive());
}

TEST_F(AosCallTrackerTest, IsVideoCallingActive_returnFalse_NormalCallTypesEmpty)
{
    IMS_UINT32 nNormalCallType = (0x1 << static_cast<IMS_UINT32>(CallType::VT)) |
            (0x1 << static_cast<IMS_UINT32>(CallType::VOIP));
    m_pAosCallTracker->SetNormalCallType(nNormalCallType);

    ImsMap<CallKey, CallState> objNormalCalls;
    ImsMap<CallKey, CallType> objNormalCallTypes;

    m_pAosCallTracker->SetNormalCalls(objNormalCalls);
    m_pAosCallTracker->SetNormalCallTypes(objNormalCallTypes);

    EXPECT_FALSE(m_pAosCallTracker->IsVideoCallingActive());
}

TEST_F(AosCallTrackerTest, IsVideoCallingActive_returnFalse_VtCallTypeIsNotExist)
{
    IMS_UINT32 nNormalCallType = (0x1 << static_cast<IMS_UINT32>(CallType::RTT)) |
            (0x1 << static_cast<IMS_UINT32>(CallType::VOIP));
    m_pAosCallTracker->SetNormalCallType(nNormalCallType);

    EXPECT_FALSE(m_pAosCallTracker->IsVideoCallingActive());
}

TEST_F(AosCallTrackerTest, GetSlotId)
{
    m_pAosCallTracker->SetSlotId(2);
    EXPECT_EQ(m_pAosCallTracker->GetSlotId(), 2);

    m_pAosCallTracker->SetSlotId(1);
    EXPECT_EQ(m_pAosCallTracker->GetSlotId(), 1);

    m_pAosCallTracker->SetSlotId(0);
    EXPECT_EQ(m_pAosCallTracker->GetSlotId(), 0);
}

TEST_F(AosCallTrackerTest, GetCallState_TypeCs)
{
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_CS), CallState::IDLE);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::RINGBACK);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_CS), CallState::RINGBACK);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::RINGING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_CS), CallState::RINGING);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::ALERTING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_CS), CallState::ALERTING);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_CS), CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, GetCallState_TypeNormal)
{
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_NORMAL), CallState::IDLE);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, CallState::RINGBACK);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_NORMAL), CallState::RINGBACK);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, CallState::RINGING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_NORMAL), CallState::RINGING);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, CallState::ALERTING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_NORMAL), CallState::ALERTING);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_NORMAL), CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, GetCallState_TypeEmergency)
{
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY), CallState::IDLE);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::RINGBACK);
    EXPECT_EQ(
            m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY), CallState::RINGBACK);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::RINGING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY), CallState::RINGING);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::ALERTING);
    EXPECT_EQ(
            m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY), CallState::ALERTING);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::OFFHOOK);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY), CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, GetCallState_TypeInvalid)
{
    // Set CallState to all Types
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::RINGING);
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, CallState::RINGING);
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::RINGING);

    // Expect : The CallState is not set with invalid type.
    IMS_UINT32 TYPE_INVALID = 10;
    m_pAosCallTracker->SetState(TYPE_INVALID, CallState::OFFHOOK);

    // Expect : GetCallState(TYPE_INVALID) always returns IDLE.
    EXPECT_EQ(m_pAosCallTracker->GetCallState(TYPE_INVALID), CallState::IDLE);

    // Expect : The state for all types is unchanged.
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_CS), CallState::RINGING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_NORMAL), CallState::RINGING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY), CallState::RINGING);
}

TEST_F(AosCallTrackerTest, SetCsCallStateWatchMode)
{
    EXPECT_CALL(m_objMockIAosService,
            AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, m_pAosCallTracker)))
            .Times(1);

    m_pAosCallTracker->SetCsCallStateWatchMode();
}

TEST_F(AosCallTrackerTest, SetListener_Success)
{
    m_pAosCallTracker->GetListeners().Clear();

    IAosCallTrackerListener* piListener1 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener2 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener3 = new MockIAosCallTrackerListener();

    m_pAosCallTracker->SetListener(piListener1);
    m_pAosCallTracker->SetListener(piListener2);
    m_pAosCallTracker->SetListener(piListener3);
    EXPECT_EQ(m_pAosCallTracker->GetListeners().GetSize(), 3);
}

TEST_F(AosCallTrackerTest, SetListener_ExistSameListener)
{
    m_pAosCallTracker->GetListeners().Clear();

    IAosCallTrackerListener* piListener1 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener2 = new MockIAosCallTrackerListener();

    m_pAosCallTracker->SetListener(piListener1);
    m_pAosCallTracker->SetListener(piListener2);

    // Sets with existing Listener
    m_pAosCallTracker->SetListener(piListener2);

    EXPECT_EQ(m_pAosCallTracker->GetListeners().GetSize(), 2);
}

TEST_F(AosCallTrackerTest, RemoveListener_Success)
{
    m_pAosCallTracker->GetListeners().Clear();

    IAosCallTrackerListener* piListener1 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener2 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener3 = new MockIAosCallTrackerListener();

    m_pAosCallTracker->SetListener(piListener1);
    m_pAosCallTracker->SetListener(piListener2);
    m_pAosCallTracker->SetListener(piListener3);
    EXPECT_EQ(m_pAosCallTracker->GetListeners().GetSize(), 3);

    m_pAosCallTracker->RemoveListener(piListener3);
    EXPECT_EQ(m_pAosCallTracker->GetListeners().GetSize(), 2);

    m_pAosCallTracker->RemoveListener(piListener2);
    EXPECT_EQ(m_pAosCallTracker->GetListeners().GetSize(), 1);

    m_pAosCallTracker->RemoveListener(piListener1);
    EXPECT_EQ(m_pAosCallTracker->GetListeners().GetSize(), 0);
}

TEST_F(AosCallTrackerTest, RemoveListener_NotExistListener)
{
    m_pAosCallTracker->GetListeners().Clear();

    IAosCallTrackerListener* piListener1 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener2 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener3 = new MockIAosCallTrackerListener();

    m_pAosCallTracker->SetListener(piListener1);
    m_pAosCallTracker->SetListener(piListener2);
    m_pAosCallTracker->SetListener(piListener3);
    EXPECT_EQ(m_pAosCallTracker->GetListeners().GetSize(), 3);

    m_pAosCallTracker->RemoveListener(piListener3);
    EXPECT_EQ(m_pAosCallTracker->GetListeners().GetSize(), 2);

    m_pAosCallTracker->RemoveListener(piListener2);
    EXPECT_EQ(m_pAosCallTracker->GetListeners().GetSize(), 1);

    // Removes with listener that doesn't already exist.
    m_pAosCallTracker->RemoveListener(piListener2);
    EXPECT_EQ(m_pAosCallTracker->GetListeners().GetSize(), 1);
}

TEST_F(AosCallTrackerTest, AddOrUpdateCall)
{
    ImsMap<CallKey, CallState> objNormalCalls;
    CallKey nKey1 = 1001;
    CallKey nKey2 = 1002;
    CallKey nKey3 = 1003;

    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey1, CallState::IDLE);
    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey2, CallState::NEW);
    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey3, CallState::RINGBACK);

    EXPECT_EQ(objNormalCalls.GetSize(), 3);

    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey3, CallState::OFFHOOK);
    EXPECT_EQ(objNormalCalls.GetSize(), 3);
}

TEST_F(AosCallTrackerTest, GetConvertedState)
{
    EXPECT_EQ(m_pAosCallTracker->GetConvertedState(IMtcCall::State::IDLE), CallState::NEW);
    EXPECT_EQ(m_pAosCallTracker->GetConvertedState(IMtcCall::State::OUTGOING), CallState::RINGBACK);
    EXPECT_EQ(m_pAosCallTracker->GetConvertedState(IMtcCall::State::INCOMING), CallState::RINGING);
    EXPECT_EQ(m_pAosCallTracker->GetConvertedState(IMtcCall::State::ALERTING), CallState::ALERTING);
    EXPECT_EQ(
            m_pAosCallTracker->GetConvertedState(IMtcCall::State::ESTABLISHED), CallState::OFFHOOK);
    EXPECT_EQ(m_pAosCallTracker->GetConvertedState(IMtcCall::State::UPDATING), CallState::OFFHOOK);
    EXPECT_EQ(m_pAosCallTracker->GetConvertedState(IMtcCall::State::TERMINATING), CallState::IDLE);
}

TEST_F(AosCallTrackerTest, GetTotalState_GreaterThanOne)
{
    ImsMap<CallKey, CallState> objNormalCalls;
    IMS_ULONG nKey1 = 1001;
    IMS_ULONG nKey2 = 1002;

    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey1, CallState::OFFHOOK);
    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey2, CallState::RINGBACK);

    EXPECT_EQ(m_pAosCallTracker->GetTotalState(objNormalCalls), CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, GetTotalState_SizeOne)
{
    ImsMap<CallKey, CallState> objNormalCalls;
    IMS_ULONG nKey = 1000;

    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey, CallState::IDLE);
    EXPECT_EQ(m_pAosCallTracker->GetTotalState(objNormalCalls), CallState::IDLE);

    objNormalCalls.Clear();
    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey, CallState::NEW);
    EXPECT_EQ(m_pAosCallTracker->GetTotalState(objNormalCalls), CallState::NEW);

    objNormalCalls.Clear();
    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey, CallState::RINGBACK);
    EXPECT_EQ(m_pAosCallTracker->GetTotalState(objNormalCalls), CallState::RINGBACK);

    objNormalCalls.Clear();
    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey, CallState::RINGING);
    EXPECT_EQ(m_pAosCallTracker->GetTotalState(objNormalCalls), CallState::RINGING);

    objNormalCalls.Clear();
    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey, CallState::ALERTING);
    EXPECT_EQ(m_pAosCallTracker->GetTotalState(objNormalCalls), CallState::ALERTING);

    objNormalCalls.Clear();
    m_pAosCallTracker->AddOrUpdateCall(objNormalCalls, nKey, CallState::OFFHOOK);
    EXPECT_EQ(m_pAosCallTracker->GetTotalState(objNormalCalls), CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, GetTotalState_Empty)
{
    ImsMap<CallKey, CallState> objNormalCalls;
    EXPECT_EQ(m_pAosCallTracker->GetTotalState(objNormalCalls), CallState::IDLE);
}

TEST_F(AosCallTrackerTest, GetTotalCallType_IsNotEmpty)
{
    ImsMap<CallKey, CallType> objNormalCallTypes;
    IMS_ULONG nKey1 = 1001;
    IMS_ULONG nKey2 = 1002;
    IMS_ULONG nKey3 = 1003;
    IMS_ULONG nKey4 = 1004;

    IMS_UINT32 nExpectCallType = static_cast<IMS_UINT32>(CallType::UNKNOWN);

    nExpectCallType |= 0x1 << static_cast<IMS_UINT32>(CallType::VOIP);
    m_pAosCallTracker->AddOrUpdateCall(objNormalCallTypes, nKey1, CallType::VOIP);
    EXPECT_EQ(m_pAosCallTracker->GetTotalCallType(objNormalCallTypes), nExpectCallType);

    nExpectCallType |= 0x1 << static_cast<IMS_UINT32>(CallType::VT);
    m_pAosCallTracker->AddOrUpdateCall(objNormalCallTypes, nKey2, CallType::VT);
    EXPECT_EQ(m_pAosCallTracker->GetTotalCallType(objNormalCallTypes), nExpectCallType);

    nExpectCallType |= 0x1 << static_cast<IMS_UINT32>(CallType::RTT);
    m_pAosCallTracker->AddOrUpdateCall(objNormalCallTypes, nKey3, CallType::RTT);
    EXPECT_EQ(m_pAosCallTracker->GetTotalCallType(objNormalCallTypes), nExpectCallType);

    nExpectCallType |= 0x1 << static_cast<IMS_UINT32>(CallType::VIDEO_RTT);
    m_pAosCallTracker->AddOrUpdateCall(objNormalCallTypes, nKey4, CallType::VIDEO_RTT);
    EXPECT_EQ(m_pAosCallTracker->GetTotalCallType(objNormalCallTypes), nExpectCallType);
}

TEST_F(AosCallTrackerTest, GetTotalCallType_IsEmpty)
{
    ImsMap<CallKey, CallType> objNormalCallTypes;

    IMS_UINT32 nExpectCallType = static_cast<IMS_UINT32>(CallType::UNKNOWN);
    EXPECT_EQ(m_pAosCallTracker->GetTotalCallType(objNormalCallTypes), nExpectCallType);
}

TEST_F(AosCallTrackerTest, Notify)
{
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener1;
    MockIAosCallTrackerListener objListener2;
    MockIAosCallTrackerListener objListener3;

    m_pAosCallTracker->SetListener(&objListener1);
    m_pAosCallTracker->SetListener(&objListener2);
    m_pAosCallTracker->SetListener(&objListener3);

    EXPECT_CALL(objListener1, CallTracker_StateChanged(_, _)).Times(1);

    EXPECT_CALL(objListener2, CallTracker_StateChanged(_, _)).Times(1);

    EXPECT_CALL(objListener3, CallTracker_StateChanged(_, _)).Times(1);

    m_pAosCallTracker->Notify(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
}

TEST_F(AosCallTrackerTest, ProcessCsChanged)
{
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::OFFHOOK))
            .Times(1);

    m_pAosCallTracker->ProcessCsChanged(CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, ProcessEmergencyChanged)
{
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(objListener,
            CallTracker_StateChanged(IAosCallTracker::TYPE_EMERGENCY, CallState::OFFHOOK))
            .Times(1);

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE))
            .Times(1);

    IMS_ULONG nKey = 1000;
    m_pAosCallTracker->ProcessEmergencyChanged(nKey, CallState::OFFHOOK);
    m_pAosCallTracker->ProcessEmergencyChanged(nKey, CallState::IDLE);
}

TEST_F(AosCallTrackerTest, ProcessNormalChanged)
{
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK))
            .Times(1);

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE))
            .Times(1);

    IMS_ULONG nKey = 1000;
    m_pAosCallTracker->ProcessNormalChanged(nKey, CallState::OFFHOOK, CallType::VOIP);
    m_pAosCallTracker->ProcessNormalChanged(nKey, CallState::IDLE, CallType::VOIP);
}

TEST_F(AosCallTrackerTest, Event_NotifyEvent_CsCallStateIdle)
{
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::IDLE))
            .Times(1);

    m_pAosCallTracker->Event_NotifyEvent(IMS_EVENT_CSCALL_STATE, IMS_CSCALL_STATE_IDLE, 0);
}

TEST_F(AosCallTrackerTest, Event_NotifyEvent_CsCallStateIncoming)
{
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::RINGING))
            .Times(1);

    m_pAosCallTracker->Event_NotifyEvent(IMS_EVENT_CSCALL_STATE, IMS_CSCALL_STATE_INCOMING, 0);
}

TEST_F(AosCallTrackerTest, Event_NotifyEvent_CsCallStateActive)
{
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::OFFHOOK))
            .Times(1);

    m_pAosCallTracker->Event_NotifyEvent(IMS_EVENT_CSCALL_STATE, IMS_CSCALL_STATE_ACTIVE, 0);
}

TEST_F(AosCallTrackerTest, ServicePhone_PreciseCallStateChanged_CallStateIdle)
{
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::IDLE))
            .Times(5);

    m_pAosCallTracker->ServicePhone_PreciseCallStateChanged(PreciseCallState::NOT_VALID);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    m_pAosCallTracker->ServicePhone_PreciseCallStateChanged(PreciseCallState::IDLE);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    m_pAosCallTracker->ServicePhone_PreciseCallStateChanged(PreciseCallState::DIALING);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    m_pAosCallTracker->ServicePhone_PreciseCallStateChanged(PreciseCallState::DISCONNECTED);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    m_pAosCallTracker->ServicePhone_PreciseCallStateChanged(PreciseCallState::DISCONNECTING);
}

TEST_F(AosCallTrackerTest, ServicePhone_PreciseCallStateChanged_CallStateRinging)
{
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::RINGING))
            .Times(3);

    m_pAosCallTracker->ServicePhone_PreciseCallStateChanged(PreciseCallState::INCOMING);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    m_pAosCallTracker->ServicePhone_PreciseCallStateChanged(PreciseCallState::WAITING);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    m_pAosCallTracker->ServicePhone_PreciseCallStateChanged(PreciseCallState::ALERTING);
}

TEST_F(AosCallTrackerTest, ServicePhone_PreciseCallStateChanged_CallStateOffhook)
{
    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::OFFHOOK))
            .Times(2);

    m_pAosCallTracker->ServicePhone_PreciseCallStateChanged(PreciseCallState::ACTIVE);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    m_pAosCallTracker->ServicePhone_PreciseCallStateChanged(PreciseCallState::HOLDING);
}

TEST_F(AosCallTrackerTest, OnCallStateChanged_Emergency)
{
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(objListener,
            CallTracker_StateChanged(IAosCallTracker::TYPE_EMERGENCY, CallState::OFFHOOK))
            .Times(1);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE);

    IMS_ULONG nKey = 1000;
    m_pAosCallTracker->OnCallStateChanged(
            nKey, IMtcCall::State::ESTABLISHED, CallType::UNKNOWN, IMS_TRUE, -1);
}

TEST_F(AosCallTrackerTest, OnCallStateChanged_CallTypeVoip)
{
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK))
            .Times(1);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    IMS_ULONG nKey = 1000;
    m_pAosCallTracker->OnCallStateChanged(
            nKey, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_FALSE, -1);
}

TEST_F(AosCallTrackerTest, OnCallStateChanged_CallTypeVt)
{
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK))
            .Times(1);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    IMS_ULONG nKey = 1000;
    m_pAosCallTracker->OnCallStateChanged(
            nKey, IMtcCall::State::ESTABLISHED, CallType::VT, IMS_FALSE, -1);
}

TEST_F(AosCallTrackerTest, OnCallStateChanged_CallTypeRtt)
{
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK))
            .Times(1);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    IMS_ULONG nKey = 1000;
    m_pAosCallTracker->OnCallStateChanged(
            nKey, IMtcCall::State::ESTABLISHED, CallType::RTT, IMS_FALSE, -1);
}

TEST_F(AosCallTrackerTest, OnCallStateChanged_CallTypeVideoRtt)
{
    m_pAosCallTracker->GetListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(&objListener);

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK))
            .Times(1);

    m_pAosCallTracker->SetState(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    IMS_ULONG nKey = 1000;
    m_pAosCallTracker->OnCallStateChanged(
            nKey, IMtcCall::State::ESTABLISHED, CallType::VIDEO_RTT, IMS_FALSE, -1);
}

TEST_F(AosCallTrackerTest, OnTotalCallStateChanged)
{
    m_pAosCallTracker->OnTotalCallStateChanged(IMtcCall::State::IDLE);
}

TEST_F(AosCallTrackerTest, TypeToString)
{
    EXPECT_STREQ(m_pAosCallTracker->TypeToString(IAosCallTracker::TYPE_CS), "TYPE_CS");
    EXPECT_STREQ(m_pAosCallTracker->TypeToString(IAosCallTracker::TYPE_NORMAL), "TYPE_NORMAL");
    EXPECT_STREQ(
            m_pAosCallTracker->TypeToString(IAosCallTracker::TYPE_EMERGENCY), "TYPE_EMERGENCY");
    EXPECT_STREQ(m_pAosCallTracker->TypeToString(-1), "__INVALID__");
}

TEST_F(AosCallTrackerTest, StateToString)
{
    EXPECT_STREQ(m_pAosCallTracker->StateToString(CallState::IDLE), "IDLE");
    EXPECT_STREQ(m_pAosCallTracker->StateToString(CallState::NEW), "NEW");
    EXPECT_STREQ(m_pAosCallTracker->StateToString(CallState::RINGBACK), "RINGBACK");
    EXPECT_STREQ(m_pAosCallTracker->StateToString(CallState::RINGING), "RINGING");
    EXPECT_STREQ(m_pAosCallTracker->StateToString(CallState::ALERTING), "ALERTING");
    EXPECT_STREQ(m_pAosCallTracker->StateToString(CallState::OFFHOOK), "OFFHOOK");
}

TEST_F(AosCallTrackerTest, CallTypeToString)
{
    EXPECT_STREQ(m_pAosCallTracker->CallTypeToString(CallType::VOIP), "VOIP");
    EXPECT_STREQ(m_pAosCallTracker->CallTypeToString(CallType::VT), "VT");
    EXPECT_STREQ(m_pAosCallTracker->CallTypeToString(CallType::RTT), "RTT");
    EXPECT_STREQ(m_pAosCallTracker->CallTypeToString(CallType::VIDEO_RTT), "VIDEO_RTT");
    EXPECT_STREQ(m_pAosCallTracker->CallTypeToString(CallType::UNKNOWN), "UNKNOWN");
}

TEST_F(AosCallTrackerTest, PrintCallTypes_StringLengthIsZero)
{
    EXPECT_TRUE(m_pAosCallTracker->PrintCallTypes(0x1 << static_cast<IMS_UINT32>(CallType::VOIP))
                        .Equals("VOIP"));
    EXPECT_TRUE(m_pAosCallTracker->PrintCallTypes(0x1 << static_cast<IMS_UINT32>(CallType::VT))
                        .Equals("VT"));
    EXPECT_TRUE(m_pAosCallTracker->PrintCallTypes(0x1 << static_cast<IMS_UINT32>(CallType::RTT))
                        .Equals("RTT"));
    EXPECT_TRUE(
            m_pAosCallTracker->PrintCallTypes(0x1 << static_cast<IMS_UINT32>(CallType::VIDEO_RTT))
                    .Equals("VIDEO_RTT"));
    EXPECT_TRUE(m_pAosCallTracker->PrintCallTypes(0).Equals("NONE"));
}

TEST_F(AosCallTrackerTest, PrintCallTypes_StringLengthIsNotZero)
{
    IMS_UINT32 nCallTypes = 0x1 << static_cast<IMS_UINT32>(CallType::VOIP);
    EXPECT_TRUE(
            m_pAosCallTracker
                    ->PrintCallTypes(nCallTypes | (0x1 << static_cast<IMS_UINT32>(CallType::VT)))
                    .Equals("VOIP, VT"));
    EXPECT_TRUE(
            m_pAosCallTracker
                    ->PrintCallTypes(nCallTypes | (0x1 << static_cast<IMS_UINT32>(CallType::RTT)))
                    .Equals("VOIP, RTT"));
    EXPECT_TRUE(m_pAosCallTracker
                        ->PrintCallTypes(
                                nCallTypes | (0x1 << static_cast<IMS_UINT32>(CallType::VIDEO_RTT)))
                        .Equals("VOIP, VIDEO_RTT"));
}
