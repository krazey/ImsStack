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

#include "IMtcService.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "call/IMtcCall.h"
#include "call/MtcCall.h"
#include "call/MtcCallManager.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockICallStateProxy.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class MtcCallManagerTest : public ::testing::Test
{
public:
    MockIMtcContext objContext;
    MockICallStateProxy objCallStateProxy;
    MtcConfigurationProxy* pConfigurationProxy;
    MtcCallManager* pCallManager;
    MockIMtcService objService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(&objService));

        pConfigurationProxy = new MtcConfigurationProxy(new MockIMtcConfigurationManager());
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        pCallManager = new MtcCallManager(objContext);
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pCallManager;
    }
};

TEST_F(MtcCallManagerTest, InitRegistersThisToCallStateProxy)
{
    EXPECT_CALL(objCallStateProxy, AddListener(pCallManager)).Times(1);

    pCallManager->Init();
}

TEST_F(MtcCallManagerTest, DeInitUnregistersThisFromCallStateProxy)
{
    EXPECT_CALL(objCallStateProxy, RemoveListener(pCallManager)).Times(1);

    pCallManager->DeInit();
}

TEST_F(MtcCallManagerTest, CreateCallReturnsNewCall)
{
    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    EXPECT_NE(nullptr, pCall);
}

TEST_F(MtcCallManagerTest, CreateCallAddsNewCallToList)
{
    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    EXPECT_EQ(pCall, pCallManager->GetCalls().GetAt(0));
}

TEST_F(MtcCallManagerTest, CreateCallReturnsNullCallIfServiceIsNull)
{
    ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(nullptr));

    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    EXPECT_EQ(DYNAMIC_CAST(IMtcCall*, MtcCallManager::s_pNullCall), pCall);
}

TEST_F(MtcCallManagerTest, CreateCallNotAddsNullCallIfServiceIsNull)
{
    ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(nullptr));

    CallInfo objCallInfo;
    pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    EXPECT_EQ(0, pCallManager->GetCalls().GetSize());
}

TEST_F(MtcCallManagerTest, RemoveCallRemovesMatchingCall)
{
    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    pCallManager->RemoveCall(pCall->GetKey());
    EXPECT_EQ(0, pCallManager->GetCalls().GetSize());
}

TEST_F(MtcCallManagerTest, RemoveCallNotRemovesCallIfNoMatchingCall)
{
    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    const CallKey nNotExistingCallKey = pCall->GetKey() + 1;

    pCallManager->RemoveCall(nNotExistingCallKey);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());
}

TEST_F(MtcCallManagerTest, GetCallByCallKeyReturnsMatchingCall)
{
    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    EXPECT_EQ(pCall, pCallManager->GetCallByCallKey(pCall->GetKey()));
}

TEST_F(MtcCallManagerTest, GetCallByCallKeyReturnsNullCallIfNoMatchingCall)
{
    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    const CallKey nNotExistingCallKey = pCall->GetKey() + 1;

    EXPECT_EQ(DYNAMIC_CAST(IMtcCall*, MtcCallManager::s_pNullCall),
            pCallManager->GetCallByCallKey(nNotExistingCallKey));
}

TEST_F(MtcCallManagerTest, GetCallsReturnsEmptyCallListInitially)
{
    IMSList<IMtcCall*> lstCalls = pCallManager->GetCalls();

    EXPECT_EQ(0, lstCalls.GetSize());
}

TEST_F(MtcCallManagerTest, GetCallsExcludingReturnsCallListOfMatchingCall)
{
    CallInfo objCallInfo;
    IMtcCall* pCall1 = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);
    IMtcCall* pCall2 = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    IMSList<IMtcCall*> lstResult = pCallManager->GetCallsExcluding(pCall1->GetKey());
    EXPECT_EQ(1, lstResult.GetSize());
    EXPECT_EQ(pCall2, lstResult.GetAt(0));
}

TEST_F(MtcCallManagerTest, GetCallsByTypeReturnsCallListOfMatchingCall)
{
    CallInfo objCallInfo1;
    objCallInfo1.eInitialCallType = CallType::VOIP;
    IMtcCall* pVoipCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo1);

    CallInfo objCallInfo2;
    objCallInfo2.eInitialCallType = CallType::VT;
    pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo2);

    IMSList<IMtcCall*> lstResult = pCallManager->GetCallsByType(CallType::VOIP);
    EXPECT_EQ(1, lstResult.GetSize());
    EXPECT_EQ(pVoipCall, lstResult.GetAt(0));
}

TEST_F(MtcCallManagerTest, GetCallsByServiceTypeReturnsCallListOfMatchingCall)
{
    MockIMtcService objNormalService;
    ON_CALL(objNormalService, GetServiceType).WillByDefault(Return(ServiceType::NORMAL));
    MockIMtcService objEmergencyService;
    ON_CALL(objEmergencyService, GetServiceType).WillByDefault(Return(ServiceType::EMERGENCY));

    ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL))
            .WillByDefault(Return(&objNormalService));
    ON_CALL(objContext, GetServiceByType(ServiceType::EMERGENCY))
            .WillByDefault(Return(&objEmergencyService));

    CallInfo objCallInfo;
    IMtcCall* pNormalCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);
    IMtcCall* pEmergencyCall = pCallManager->CreateCall(ServiceType::EMERGENCY, objCallInfo);

    IMSList<IMtcCall*> lstResult = pCallManager->GetCallsByServiceType(ServiceType::NORMAL);
    EXPECT_EQ(1, lstResult.GetSize());
    EXPECT_EQ(pNormalCall, lstResult.GetAt(0));

    // to delete MtcCall before MockIMtcService is deleted
    pCallManager->RemoveCall(pNormalCall->GetKey());
    pCallManager->RemoveCall(pEmergencyCall->GetKey());
}

TEST_F(MtcCallManagerTest, GetCallsInConferenceReturnsCallListOfMatchingCall)
{
    CallInfo objCallInfo1;
    objCallInfo1.bConference = IMS_TRUE;
    IMtcCall* pConferenceCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo1);

    CallInfo objCallInfo2;
    objCallInfo2.bConference = IMS_FALSE;
    pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo2);

    IMSList<IMtcCall*> lstResult = pCallManager->GetCallsInConference();
    EXPECT_EQ(1, lstResult.GetSize());
    EXPECT_EQ(pConferenceCall, lstResult.GetAt(0));
}

TEST_F(MtcCallManagerTest, GetCallsByStateReturnsCallListOfMatchingCall)
{
    CallInfo objCallInfo;
    IMtcCall* pIdleCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    IMSList<IMtcCall*> lstIdleCalls = pCallManager->GetCallsByState(IMtcCall::State::IDLE);
    EXPECT_EQ(1, lstIdleCalls.GetSize());
    EXPECT_EQ(pIdleCall, lstIdleCalls.GetAt(0));

    IMSList<IMtcCall*> lstTerminatingCalls =
            pCallManager->GetCallsByState(IMtcCall::State::TERMINATING);
    EXPECT_EQ(0, lstTerminatingCalls.GetSize());
}

TEST_F(MtcCallManagerTest, OnCallStateChangedDoNothingIfNotTerminating)
{
    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    const CallType eType = CallType::VOIP;
    const IMS_BOOL bEmergency = IMS_FALSE;
    const IMS_SINT32 nReason = 0;

    pCallManager->OnCallStateChanged(
            pCall->GetKey(), IMtcCall::State::IDLE, eType, bEmergency, nReason);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());

    pCallManager->OnCallStateChanged(
            pCall->GetKey(), IMtcCall::State::OUTGOING, eType, bEmergency, nReason);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());

    pCallManager->OnCallStateChanged(
            pCall->GetKey(), IMtcCall::State::INCOMING, eType, bEmergency, nReason);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());

    pCallManager->OnCallStateChanged(
            pCall->GetKey(), IMtcCall::State::ALERTING, eType, bEmergency, nReason);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());

    pCallManager->OnCallStateChanged(
            pCall->GetKey(), IMtcCall::State::ESTABLISHED, eType, bEmergency, nReason);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());

    pCallManager->OnCallStateChanged(
            pCall->GetKey(), IMtcCall::State::UPDATING, eType, bEmergency, nReason);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());
}

TEST_F(MtcCallManagerTest, OnCallStateChangedRemoveCallIfTerminating)
{
    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    const CallType eType = CallType::VOIP;
    const IMS_BOOL bEmergency = IMS_FALSE;
    const IMS_SINT32 nReason = 0;

    pCallManager->OnCallStateChanged(
            pCall->GetKey(), IMtcCall::State::TERMINATING, eType, bEmergency, nReason);
    EXPECT_EQ(0, pCallManager->GetCalls().GetSize());
}

TEST_F(MtcCallManagerTest, OnTotalCallStateChangedDoNothing)
{
    CallInfo objCallInfo;
    pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    pCallManager->OnTotalCallStateChanged(IMtcCall::State::IDLE);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());

    pCallManager->OnTotalCallStateChanged(IMtcCall::State::OUTGOING);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());

    pCallManager->OnTotalCallStateChanged(IMtcCall::State::INCOMING);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());

    pCallManager->OnTotalCallStateChanged(IMtcCall::State::ALERTING);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());

    pCallManager->OnTotalCallStateChanged(IMtcCall::State::ESTABLISHED);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());

    pCallManager->OnTotalCallStateChanged(IMtcCall::State::UPDATING);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());

    pCallManager->OnTotalCallStateChanged(IMtcCall::State::TERMINATING);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());
}
