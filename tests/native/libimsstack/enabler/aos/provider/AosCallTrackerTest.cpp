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

class AosCallTrackerTest : public ::testing::Test
{
public:
    AosCallTracker* m_pAosCallTracker;
    IAosService* m_piOriginService;
    MockIAosService m_objMockIAosService;

protected:
    virtual void SetUp() override
    {
        m_pAosCallTracker = new AosCallTracker(0);
        ASSERT_TRUE(m_pAosCallTracker != nullptr);

        // Store origin AosService
        m_piOriginService = AosProvider::GetInstance()->GetService();

        // Set MockIAosService
        AosProvider::GetInstance()->SetService(static_cast<IAosService*>(&m_objMockIAosService));
        EXPECT_CALL(m_objMockIAosService,
                RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, m_pAosCallTracker)))
                .Times(AnyNumber());
    }

    virtual void TearDown() override
    {
        if (m_pAosCallTracker)
        {
            delete m_pAosCallTracker;
        }

        AosProvider::GetInstance()->SetService(m_piOriginService);
    }

    void SetSlotId(IN IMS_SINT32 nSlotId) { m_pAosCallTracker->m_nSlotId = nSlotId; }

    void SetState(IN IMS_UINT32 nType, IN CallState eState)
    {
        m_pAosCallTracker->SetState(nType, eState);
    }

    ImsList<IAosCallTrackerListener*> GetCallTrackerListeners()
    {
        return m_pAosCallTracker->m_objListeners;
    }

    CallState GetConvertedState(IN IMtcCall::State eState)
    {
        return m_pAosCallTracker->GetConvertedState(eState);
    }

    void SetNormalCallType(IN IMS_UINT32 nNormalCallType)
    {
        m_pAosCallTracker->m_nNormalCallType = nNormalCallType;
    }

    void SetNormalCalls(IN const ImsMap<CallKey, CallState>& objNormalCalls)
    {
        m_pAosCallTracker->m_objNormalCalls = objNormalCalls;
    }

    void SetEmergenyCalls(IN const ImsMap<CallKey, CallState>& objEmergencyCalls)
    {
        m_pAosCallTracker->m_objEmergencyCalls = objEmergencyCalls;
    }

    void SetNormalCallTypes(IN const ImsMap<CallKey, CallType>& objNormalCallTypes)
    {
        m_pAosCallTracker->m_objNormalCallTypes = objNormalCallTypes;
    }

    template <typename T>
    void AddOrUpdateCall(OUT ImsMap<CallKey, T>& objCalls, IN CallKey eKey, IN T eValue)
    {
        m_pAosCallTracker->AddOrUpdateCall(objCalls, eKey, eValue);
    }

    CallState GetTotalState(IN ImsMap<CallKey, CallState>& objCalls)
    {
        return m_pAosCallTracker->GetTotalState(objCalls);
    }

    IMS_UINT32 GetTotalCallType(IN ImsMap<CallKey, CallType>& objCallTypes)
    {
        return m_pAosCallTracker->GetTotalCallType(objCallTypes);
    }

    void Notify(IN IMS_UINT32 nType, IN CallState eState)
    {
        m_pAosCallTracker->Notify(nType, eState);
    }

    void ProcessCsChanged(IN CallState eState) { m_pAosCallTracker->ProcessCsChanged(eState); }

    void ProcessEmergencyChanged(IN CallKey eKey, IN CallState eState)
    {
        m_pAosCallTracker->ProcessEmergencyChanged(eKey, eState);
    }

    void ProcessNormalChanged(IN CallKey nCallKey, IN CallState eCallState, IN CallType eCallType)
    {
        m_pAosCallTracker->ProcessNormalChanged(nCallKey, eCallState, eCallType);
    }

    void Event_NotifyEvent(IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
    {
        m_pAosCallTracker->Event_NotifyEvent(nEvent, nWParam, nLParam);
    }

    void ServicePhone_PreciseCallStateChanged(IN PreciseCallState eState)
    {
        m_pAosCallTracker->ServicePhone_PreciseCallStateChanged(eState);
    }

    void OnCallStateChanged(IN CallKey nCallKey, IN IMtcCall::State eState, IN CallType eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason)
    {
        m_pAosCallTracker->OnCallStateChanged(nCallKey, eState, eType, bEmergency, nReason);
    }

    void OnTotalCallStateChanged(IN IMtcCall::State eState)
    {
        m_pAosCallTracker->OnTotalCallStateChanged(eState);
    }

    static const IMS_CHAR* TypeToString(IN IMS_UINT32 nType)
    {
        return AosCallTracker::TypeToString(nType);
    }

    static const IMS_CHAR* StateToString(IN CallState eState)
    {
        return AosCallTracker::StateToString(eState);
    }

    static const IMS_CHAR* CallTypeToString(IN CallType eType)
    {
        return AosCallTracker::CallTypeToString(eType);
    }

    AString PrintCallTypes(IN IMS_UINT32 nCallTypes)
    {
        return m_pAosCallTracker->PrintCallTypes(nCallTypes);
    }
};

TEST_F(AosCallTrackerTest, SetMtcReady)
{
    EXPECT_TRUE(m_pAosCallTracker->SetMtcReady());
}

TEST_F(AosCallTrackerTest, IsCsCallActive_CsStateChange)
{
    m_pAosCallTracker->SetActiveCsCallState(CallState::RINGING);

    // Expect return False
    SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    EXPECT_FALSE(m_pAosCallTracker->IsCsCallActive());

    SetState(IAosCallTracker::TYPE_CS, CallState::RINGBACK);
    EXPECT_FALSE(m_pAosCallTracker->IsCsCallActive());

    SetState(IAosCallTracker::TYPE_CS, CallState::RINGING);
    EXPECT_FALSE(m_pAosCallTracker->IsCsCallActive());

    // Expect return True
    SetState(IAosCallTracker::TYPE_CS, CallState::ALERTING);
    EXPECT_TRUE(m_pAosCallTracker->IsCsCallActive());

    SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    EXPECT_TRUE(m_pAosCallTracker->IsCsCallActive());
}

TEST_F(AosCallTrackerTest, IsCsCallActive_ActiveCsStateChange)
{
    SetState(IAosCallTracker::TYPE_CS, CallState::RINGING);

    // Expect return True
    m_pAosCallTracker->SetActiveCsCallState(CallState::IDLE);
    EXPECT_TRUE(m_pAosCallTracker->IsCsCallActive());

    m_pAosCallTracker->SetActiveCsCallState(CallState::RINGBACK);
    EXPECT_TRUE(m_pAosCallTracker->IsCsCallActive());

    // Expect return False
    m_pAosCallTracker->SetActiveCsCallState(CallState::RINGING);
    EXPECT_FALSE(m_pAosCallTracker->IsCsCallActive());

    m_pAosCallTracker->SetActiveCsCallState(CallState::ALERTING);
    EXPECT_FALSE(m_pAosCallTracker->IsCsCallActive());

    m_pAosCallTracker->SetActiveCsCallState(CallState::OFFHOOK);
    EXPECT_FALSE(m_pAosCallTracker->IsCsCallActive());
}

TEST_F(AosCallTrackerTest, IsNormalCallActive)
{
    // Expect return False
    SetState(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
    EXPECT_FALSE(m_pAosCallTracker->IsNormalCallActive());

    // Expect return True
    SetState(IAosCallTracker::TYPE_NORMAL, CallState::RINGBACK);
    EXPECT_TRUE(m_pAosCallTracker->IsNormalCallActive());

    SetState(IAosCallTracker::TYPE_NORMAL, CallState::RINGING);
    EXPECT_TRUE(m_pAosCallTracker->IsNormalCallActive());

    SetState(IAosCallTracker::TYPE_NORMAL, CallState::ALERTING);
    EXPECT_TRUE(m_pAosCallTracker->IsNormalCallActive());

    SetState(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK);
    EXPECT_TRUE(m_pAosCallTracker->IsNormalCallActive());
}

TEST_F(AosCallTrackerTest, IsEmergencyCallActive)
{
    // Expect return False
    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE);
    EXPECT_FALSE(m_pAosCallTracker->IsEmergencyCallActive());

    // Expect return True
    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::RINGBACK);
    EXPECT_TRUE(m_pAosCallTracker->IsEmergencyCallActive());

    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::RINGING);
    EXPECT_TRUE(m_pAosCallTracker->IsEmergencyCallActive());

    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::ALERTING);
    EXPECT_TRUE(m_pAosCallTracker->IsEmergencyCallActive());

    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::OFFHOOK);
    EXPECT_TRUE(m_pAosCallTracker->IsEmergencyCallActive());
}

TEST_F(AosCallTrackerTest, IsVideoCallingActive_returnTrue)
{
    IMS_UINT32 nNormalCallType = (0x1 << static_cast<IMS_UINT32>(CallType::VT)) |
            (0x1 << static_cast<IMS_UINT32>(CallType::VOIP));
    SetNormalCallType(nNormalCallType);

    ImsMap<CallKey, CallState> objNormalCalls;
    ImsMap<CallKey, CallType> objNormalCallTypes;
    IMS_ULONG nKey1 = 1001;
    IMS_ULONG nKey2 = 1002;

    AddOrUpdateCall(objNormalCalls, nKey1, CallState::RINGBACK);
    AddOrUpdateCall(objNormalCallTypes, nKey1, CallType::VOIP);

    AddOrUpdateCall(objNormalCalls, nKey2, CallState::OFFHOOK);
    AddOrUpdateCall(objNormalCallTypes, nKey2, CallType::VT);

    SetNormalCalls(objNormalCalls);
    SetNormalCallTypes(objNormalCallTypes);

    EXPECT_TRUE(m_pAosCallTracker->IsVideoCallingActive());
}

TEST_F(AosCallTrackerTest, IsVideoCallingActive_returnFalse_VtIsNotOffhook)
{
    IMS_UINT32 nNormalCallType = (0x1 << static_cast<IMS_UINT32>(CallType::VT)) |
            (0x1 << static_cast<IMS_UINT32>(CallType::VOIP));
    SetNormalCallType(nNormalCallType);

    ImsMap<CallKey, CallState> objNormalCalls;
    ImsMap<CallKey, CallType> objNormalCallTypes;
    IMS_ULONG nKey1 = 1001;
    IMS_ULONG nKey2 = 1002;

    AddOrUpdateCall(objNormalCalls, nKey1, CallState::RINGING);
    AddOrUpdateCall(objNormalCallTypes, nKey1, CallType::VOIP);

    AddOrUpdateCall(objNormalCalls, nKey2, CallState::IDLE);
    AddOrUpdateCall(objNormalCallTypes, nKey2, CallType::VT);

    SetNormalCalls(objNormalCalls);
    SetNormalCallTypes(objNormalCallTypes);

    EXPECT_FALSE(m_pAosCallTracker->IsVideoCallingActive());
}

TEST_F(AosCallTrackerTest, IsVideoCallingActive_returnFalse_NormalCallTypesEmpty)
{
    IMS_UINT32 nNormalCallType = (0x1 << static_cast<IMS_UINT32>(CallType::VT)) |
            (0x1 << static_cast<IMS_UINT32>(CallType::VOIP));
    SetNormalCallType(nNormalCallType);

    ImsMap<CallKey, CallState> objNormalCalls;
    ImsMap<CallKey, CallType> objNormalCallTypes;

    SetNormalCalls(objNormalCalls);
    SetNormalCallTypes(objNormalCallTypes);

    EXPECT_FALSE(m_pAosCallTracker->IsVideoCallingActive());
}

TEST_F(AosCallTrackerTest, IsVideoCallingActive_returnFalse_VtCallTypeIsNotExist)
{
    IMS_UINT32 nNormalCallType = (0x1 << static_cast<IMS_UINT32>(CallType::RTT)) |
            (0x1 << static_cast<IMS_UINT32>(CallType::VOIP));
    SetNormalCallType(nNormalCallType);

    EXPECT_FALSE(m_pAosCallTracker->IsVideoCallingActive());
}

TEST_F(AosCallTrackerTest, GetSlotId)
{
    SetSlotId(2);
    EXPECT_EQ(m_pAosCallTracker->GetSlotId(), 2);

    SetSlotId(1);
    EXPECT_EQ(m_pAosCallTracker->GetSlotId(), 1);

    SetSlotId(0);
    EXPECT_EQ(m_pAosCallTracker->GetSlotId(), 0);
}

TEST_F(AosCallTrackerTest, GetCallState_TypeCs)
{
    SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_CS), CallState::IDLE);

    SetState(IAosCallTracker::TYPE_CS, CallState::RINGBACK);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_CS), CallState::RINGBACK);

    SetState(IAosCallTracker::TYPE_CS, CallState::RINGING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_CS), CallState::RINGING);

    SetState(IAosCallTracker::TYPE_CS, CallState::ALERTING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_CS), CallState::ALERTING);

    SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_CS), CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, GetCallState_TypeNormal)
{
    SetState(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_NORMAL), CallState::IDLE);

    SetState(IAosCallTracker::TYPE_NORMAL, CallState::RINGBACK);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_NORMAL), CallState::RINGBACK);

    SetState(IAosCallTracker::TYPE_NORMAL, CallState::RINGING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_NORMAL), CallState::RINGING);

    SetState(IAosCallTracker::TYPE_NORMAL, CallState::ALERTING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_NORMAL), CallState::ALERTING);

    SetState(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_NORMAL), CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, GetCallState_TypeEmergency)
{
    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY), CallState::IDLE);

    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::RINGBACK);
    EXPECT_EQ(
            m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY), CallState::RINGBACK);

    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::RINGING);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY), CallState::RINGING);

    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::ALERTING);
    EXPECT_EQ(
            m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY), CallState::ALERTING);

    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::OFFHOOK);
    EXPECT_EQ(m_pAosCallTracker->GetCallState(IAosCallTracker::TYPE_EMERGENCY), CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, GetCallState_TypeInvalid)
{
    // Set CallState to all Types
    SetState(IAosCallTracker::TYPE_CS, CallState::RINGING);
    SetState(IAosCallTracker::TYPE_NORMAL, CallState::RINGING);
    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::RINGING);

    // Expect : The CallState is not set with invalid type.
    IMS_UINT32 TYPE_INVALID = 10;
    SetState(TYPE_INVALID, CallState::OFFHOOK);

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
    GetCallTrackerListeners().Clear();

    IAosCallTrackerListener* piListener1 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener2 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener3 = new MockIAosCallTrackerListener();

    m_pAosCallTracker->SetListener(piListener1);
    m_pAosCallTracker->SetListener(piListener2);
    m_pAosCallTracker->SetListener(piListener3);
    EXPECT_EQ(GetCallTrackerListeners().GetSize(), 3);
}

TEST_F(AosCallTrackerTest, SetListener_ExistSameListener)
{
    GetCallTrackerListeners().Clear();

    IAosCallTrackerListener* piListener1 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener2 = new MockIAosCallTrackerListener();

    m_pAosCallTracker->SetListener(piListener1);
    m_pAosCallTracker->SetListener(piListener2);

    // Sets with existing Listener
    m_pAosCallTracker->SetListener(piListener2);

    EXPECT_EQ(GetCallTrackerListeners().GetSize(), 2);
}

TEST_F(AosCallTrackerTest, RemoveListener_Success)
{
    GetCallTrackerListeners().Clear();

    IAosCallTrackerListener* piListener1 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener2 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener3 = new MockIAosCallTrackerListener();

    m_pAosCallTracker->SetListener(piListener1);
    m_pAosCallTracker->SetListener(piListener2);
    m_pAosCallTracker->SetListener(piListener3);
    EXPECT_EQ(GetCallTrackerListeners().GetSize(), 3);

    m_pAosCallTracker->RemoveListener(piListener3);
    EXPECT_EQ(GetCallTrackerListeners().GetSize(), 2);

    m_pAosCallTracker->RemoveListener(piListener2);
    EXPECT_EQ(GetCallTrackerListeners().GetSize(), 1);

    m_pAosCallTracker->RemoveListener(piListener1);
    EXPECT_EQ(GetCallTrackerListeners().GetSize(), 0);
}

TEST_F(AosCallTrackerTest, RemoveListener_NotExistListener)
{
    GetCallTrackerListeners().Clear();

    IAosCallTrackerListener* piListener1 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener2 = new MockIAosCallTrackerListener();
    IAosCallTrackerListener* piListener3 = new MockIAosCallTrackerListener();

    m_pAosCallTracker->SetListener(piListener1);
    m_pAosCallTracker->SetListener(piListener2);
    m_pAosCallTracker->SetListener(piListener3);
    EXPECT_EQ(GetCallTrackerListeners().GetSize(), 3);

    m_pAosCallTracker->RemoveListener(piListener3);
    EXPECT_EQ(GetCallTrackerListeners().GetSize(), 2);

    m_pAosCallTracker->RemoveListener(piListener2);
    EXPECT_EQ(GetCallTrackerListeners().GetSize(), 1);

    // Removes with listener that doesn't already exist.
    m_pAosCallTracker->RemoveListener(piListener2);
    EXPECT_EQ(GetCallTrackerListeners().GetSize(), 1);
}

TEST_F(AosCallTrackerTest, AddOrUpdateCall)
{
    ImsMap<CallKey, CallState> objNormalCalls;
    CallKey nKey1 = 1001;
    CallKey nKey2 = 1002;
    CallKey nKey3 = 1003;

    AddOrUpdateCall(objNormalCalls, nKey1, CallState::IDLE);
    AddOrUpdateCall(objNormalCalls, nKey2, CallState::NEW);
    AddOrUpdateCall(objNormalCalls, nKey3, CallState::RINGBACK);

    EXPECT_EQ(objNormalCalls.GetSize(), 3);

    AddOrUpdateCall(objNormalCalls, nKey3, CallState::OFFHOOK);
    EXPECT_EQ(objNormalCalls.GetSize(), 3);
}

TEST_F(AosCallTrackerTest, GetConvertedState)
{
    EXPECT_EQ(GetConvertedState(IMtcCall::State::IDLE), CallState::NEW);
    EXPECT_EQ(GetConvertedState(IMtcCall::State::OUTGOING), CallState::RINGBACK);
    EXPECT_EQ(GetConvertedState(IMtcCall::State::INCOMING), CallState::RINGING);
    EXPECT_EQ(GetConvertedState(IMtcCall::State::ALERTING), CallState::ALERTING);
    EXPECT_EQ(GetConvertedState(IMtcCall::State::ESTABLISHED), CallState::OFFHOOK);
    EXPECT_EQ(GetConvertedState(IMtcCall::State::UPDATING), CallState::OFFHOOK);
    EXPECT_EQ(GetConvertedState(IMtcCall::State::TERMINATING), CallState::IDLE);
}

TEST_F(AosCallTrackerTest, GetTotalState_GreaterThanOne)
{
    ImsMap<CallKey, CallState> objNormalCalls;
    IMS_ULONG nKey1 = 1001;
    IMS_ULONG nKey2 = 1002;

    AddOrUpdateCall(objNormalCalls, nKey1, CallState::OFFHOOK);
    AddOrUpdateCall(objNormalCalls, nKey2, CallState::RINGBACK);

    EXPECT_EQ(GetTotalState(objNormalCalls), CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, GetTotalState_SizeOne)
{
    ImsMap<CallKey, CallState> objNormalCalls;
    IMS_ULONG nKey = 1000;

    AddOrUpdateCall(objNormalCalls, nKey, CallState::IDLE);
    EXPECT_EQ(GetTotalState(objNormalCalls), CallState::IDLE);

    objNormalCalls.Clear();
    AddOrUpdateCall(objNormalCalls, nKey, CallState::NEW);
    EXPECT_EQ(GetTotalState(objNormalCalls), CallState::NEW);

    objNormalCalls.Clear();
    AddOrUpdateCall(objNormalCalls, nKey, CallState::RINGBACK);
    EXPECT_EQ(GetTotalState(objNormalCalls), CallState::RINGBACK);

    objNormalCalls.Clear();
    AddOrUpdateCall(objNormalCalls, nKey, CallState::RINGING);
    EXPECT_EQ(GetTotalState(objNormalCalls), CallState::RINGING);

    objNormalCalls.Clear();
    AddOrUpdateCall(objNormalCalls, nKey, CallState::ALERTING);
    EXPECT_EQ(GetTotalState(objNormalCalls), CallState::ALERTING);

    objNormalCalls.Clear();
    AddOrUpdateCall(objNormalCalls, nKey, CallState::OFFHOOK);
    EXPECT_EQ(GetTotalState(objNormalCalls), CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, GetTotalState_Empty)
{
    ImsMap<CallKey, CallState> objNormalCalls;
    EXPECT_EQ(GetTotalState(objNormalCalls), CallState::IDLE);
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
    AddOrUpdateCall(objNormalCallTypes, nKey1, CallType::VOIP);
    EXPECT_EQ(GetTotalCallType(objNormalCallTypes), nExpectCallType);

    nExpectCallType |= 0x1 << static_cast<IMS_UINT32>(CallType::VT);
    AddOrUpdateCall(objNormalCallTypes, nKey2, CallType::VT);
    EXPECT_EQ(GetTotalCallType(objNormalCallTypes), nExpectCallType);

    nExpectCallType |= 0x1 << static_cast<IMS_UINT32>(CallType::RTT);
    AddOrUpdateCall(objNormalCallTypes, nKey3, CallType::RTT);
    EXPECT_EQ(GetTotalCallType(objNormalCallTypes), nExpectCallType);

    nExpectCallType |= 0x1 << static_cast<IMS_UINT32>(CallType::VIDEO_RTT);
    AddOrUpdateCall(objNormalCallTypes, nKey4, CallType::VIDEO_RTT);
    EXPECT_EQ(GetTotalCallType(objNormalCallTypes), nExpectCallType);
}

TEST_F(AosCallTrackerTest, GetTotalCallType_IsEmpty)
{
    ImsMap<CallKey, CallType> objNormalCallTypes;

    IMS_UINT32 nExpectCallType = static_cast<IMS_UINT32>(CallType::UNKNOWN);
    EXPECT_EQ(GetTotalCallType(objNormalCallTypes), nExpectCallType);
}

TEST_F(AosCallTrackerTest, Notify)
{
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener1;
    MockIAosCallTrackerListener objListener2;
    MockIAosCallTrackerListener objListener3;

    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener1));
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener2));
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener3));

    EXPECT_CALL(objListener1, CallTracker_StateChanged(_, _)).Times(1);

    EXPECT_CALL(objListener2, CallTracker_StateChanged(_, _)).Times(1);

    EXPECT_CALL(objListener3, CallTracker_StateChanged(_, _)).Times(1);

    Notify(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);
}

TEST_F(AosCallTrackerTest, ProcessCsChanged)
{
    SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::OFFHOOK))
            .Times(1);

    ProcessCsChanged(CallState::OFFHOOK);
}

TEST_F(AosCallTrackerTest, ProcessEmergencyChanged)
{
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(objListener,
            CallTracker_StateChanged(IAosCallTracker::TYPE_EMERGENCY, CallState::OFFHOOK))
            .Times(1);

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE))
            .Times(1);

    IMS_ULONG nKey = 1000;
    ProcessEmergencyChanged(nKey, CallState::OFFHOOK);
    ProcessEmergencyChanged(nKey, CallState::IDLE);
}

TEST_F(AosCallTrackerTest, ProcessNormalChanged)
{
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK))
            .Times(1);

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::IDLE))
            .Times(1);

    IMS_ULONG nKey = 1000;
    ProcessNormalChanged(nKey, CallState::OFFHOOK, CallType::VOIP);
    ProcessNormalChanged(nKey, CallState::IDLE, CallType::VOIP);
}

TEST_F(AosCallTrackerTest, Event_NotifyEvent_CsCallStateIdle)
{
    SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::IDLE))
            .Times(1);

    Event_NotifyEvent(IMS_EVENT_CSCALL_STATE, IMS_CSCALL_STATE_IDLE, 0);
}

TEST_F(AosCallTrackerTest, Event_NotifyEvent_CsCallStateIncoming)
{
    SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::RINGING))
            .Times(1);

    Event_NotifyEvent(IMS_EVENT_CSCALL_STATE, IMS_CSCALL_STATE_INCOMING, 0);
}

TEST_F(AosCallTrackerTest, Event_NotifyEvent_CsCallStateActive)
{
    SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::OFFHOOK))
            .Times(1);

    Event_NotifyEvent(IMS_EVENT_CSCALL_STATE, IMS_CSCALL_STATE_ACTIVE, 0);
}

TEST_F(AosCallTrackerTest, ServicePhone_PreciseCallStateChanged_CallStateIdle)
{
    SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::IDLE))
            .Times(5);

    ServicePhone_PreciseCallStateChanged(PreciseCallState::NOT_VALID);

    SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    ServicePhone_PreciseCallStateChanged(PreciseCallState::IDLE);

    SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    ServicePhone_PreciseCallStateChanged(PreciseCallState::DIALING);

    SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    ServicePhone_PreciseCallStateChanged(PreciseCallState::DISCONNECTED);

    SetState(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
    ServicePhone_PreciseCallStateChanged(PreciseCallState::DISCONNECTING);
}

TEST_F(AosCallTrackerTest, ServicePhone_PreciseCallStateChanged_CallStateRinging)
{
    SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::RINGING))
            .Times(3);

    ServicePhone_PreciseCallStateChanged(PreciseCallState::INCOMING);

    SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    ServicePhone_PreciseCallStateChanged(PreciseCallState::WAITING);

    SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    ServicePhone_PreciseCallStateChanged(PreciseCallState::ALERTING);
}

TEST_F(AosCallTrackerTest, ServicePhone_PreciseCallStateChanged_CallStateOffhook)
{
    SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::OFFHOOK))
            .Times(2);

    ServicePhone_PreciseCallStateChanged(PreciseCallState::ACTIVE);

    SetState(IAosCallTracker::TYPE_CS, CallState::IDLE);
    ServicePhone_PreciseCallStateChanged(PreciseCallState::HOLDING);
}

TEST_F(AosCallTrackerTest, OnCallStateChanged_Emergency)
{
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(objListener,
            CallTracker_StateChanged(IAosCallTracker::TYPE_EMERGENCY, CallState::OFFHOOK))
            .Times(1);

    SetState(IAosCallTracker::TYPE_EMERGENCY, CallState::IDLE);

    IMS_ULONG nKey = 1000;
    OnCallStateChanged(nKey, IMtcCall::State::ESTABLISHED, CallType::UNKNOWN, IMS_TRUE, -1);
}

TEST_F(AosCallTrackerTest, OnCallStateChanged_CallTypeVoip)
{
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK))
            .Times(1);

    SetState(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    IMS_ULONG nKey = 1000;
    OnCallStateChanged(nKey, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_FALSE, -1);
}

TEST_F(AosCallTrackerTest, OnCallStateChanged_CallTypeVt)
{
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK))
            .Times(1);

    SetState(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    IMS_ULONG nKey = 1000;
    OnCallStateChanged(nKey, IMtcCall::State::ESTABLISHED, CallType::VT, IMS_FALSE, -1);
}

TEST_F(AosCallTrackerTest, OnCallStateChanged_CallTypeRtt)
{
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK))
            .Times(1);

    SetState(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    IMS_ULONG nKey = 1000;
    OnCallStateChanged(nKey, IMtcCall::State::ESTABLISHED, CallType::RTT, IMS_FALSE, -1);
}

TEST_F(AosCallTrackerTest, OnCallStateChanged_CallTypeVideoRtt)
{
    GetCallTrackerListeners().Clear();

    MockIAosCallTrackerListener objListener;
    m_pAosCallTracker->SetListener(static_cast<IAosCallTrackerListener*>(&objListener));

    EXPECT_CALL(
            objListener, CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK))
            .Times(1);

    SetState(IAosCallTracker::TYPE_NORMAL, CallState::IDLE);

    IMS_ULONG nKey = 1000;
    OnCallStateChanged(nKey, IMtcCall::State::ESTABLISHED, CallType::VIDEO_RTT, IMS_FALSE, -1);
}

TEST_F(AosCallTrackerTest, OnTotalCallStateChanged)
{
    OnTotalCallStateChanged(IMtcCall::State::IDLE);
}

TEST_F(AosCallTrackerTest, TypeToString)
{
    EXPECT_STREQ(TypeToString(IAosCallTracker::TYPE_CS), "TYPE_CS");
    EXPECT_STREQ(TypeToString(IAosCallTracker::TYPE_NORMAL), "TYPE_NORMAL");
    EXPECT_STREQ(TypeToString(IAosCallTracker::TYPE_EMERGENCY), "TYPE_EMERGENCY");
    EXPECT_STREQ(TypeToString(-1), "__INVALID__");
}

TEST_F(AosCallTrackerTest, StateToString)
{
    EXPECT_STREQ(StateToString(CallState::IDLE), "IDLE");
    EXPECT_STREQ(StateToString(CallState::NEW), "NEW");
    EXPECT_STREQ(StateToString(CallState::RINGBACK), "RINGBACK");
    EXPECT_STREQ(StateToString(CallState::RINGING), "RINGING");
    EXPECT_STREQ(StateToString(CallState::ALERTING), "ALERTING");
    EXPECT_STREQ(StateToString(CallState::OFFHOOK), "OFFHOOK");
}

TEST_F(AosCallTrackerTest, CallTypeToString)
{
    EXPECT_STREQ(CallTypeToString(CallType::VOIP), "VOIP");
    EXPECT_STREQ(CallTypeToString(CallType::VT), "VT");
    EXPECT_STREQ(CallTypeToString(CallType::RTT), "RTT");
    EXPECT_STREQ(CallTypeToString(CallType::VIDEO_RTT), "VIDEO_RTT");
    EXPECT_STREQ(CallTypeToString(CallType::UNKNOWN), "UNKNOWN");
}

TEST_F(AosCallTrackerTest, PrintCallTypes_StringLengthIsZero)
{
    EXPECT_TRUE(PrintCallTypes(0x1 << static_cast<IMS_UINT32>(CallType::VOIP)).Equals("VOIP"));
    EXPECT_TRUE(PrintCallTypes(0x1 << static_cast<IMS_UINT32>(CallType::VT)).Equals("VT"));
    EXPECT_TRUE(PrintCallTypes(0x1 << static_cast<IMS_UINT32>(CallType::RTT)).Equals("RTT"));
    EXPECT_TRUE(PrintCallTypes(0x1 << static_cast<IMS_UINT32>(CallType::VIDEO_RTT))
                        .Equals("VIDEO_RTT"));
    EXPECT_TRUE(PrintCallTypes(0).Equals("NONE"));
}

TEST_F(AosCallTrackerTest, PrintCallTypes_StringLengthIsNotZero)
{
    IMS_UINT32 nCallTypes = 0x1 << static_cast<IMS_UINT32>(CallType::VOIP);
    EXPECT_TRUE(PrintCallTypes(nCallTypes | (0x1 << static_cast<IMS_UINT32>(CallType::VT)))
                        .Equals("VOIP, VT"));
    EXPECT_TRUE(PrintCallTypes(nCallTypes | (0x1 << static_cast<IMS_UINT32>(CallType::RTT)))
                        .Equals("VOIP, RTT"));
    EXPECT_TRUE(PrintCallTypes(nCallTypes | (0x1 << static_cast<IMS_UINT32>(CallType::VIDEO_RTT)))
                        .Equals("VOIP, VIDEO_RTT"));
}