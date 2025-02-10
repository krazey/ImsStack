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
#include "ImsList.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "call/IMtcCall.h"
#include "call/MtcCall.h"
#include "call/MtcCallManager.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockMtcTimerWrapper.h"
#include <gtest/gtest.h>
#include <memory>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;

class MtcCallManagerTest : public ::testing::Test
{
public:
    MockIMtcContext objContext;
    MockICallStateProxy objCallStateProxy;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MtcCallManager* pCallManager;
    MockIMtcService objService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(&objService));
        ON_CALL(objService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));

        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objContext, CreateTimer)
                .WillByDefault(Invoke(
                        []()
                        {
                            return std::make_unique<MockMtcTimerWrapper>();
                        }));

        pCallManager = new MtcCallManager(objContext);
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pCallManager;
    }

    void ClearAllCalls()
    {
        ImsList<IMtcCall*> lstCalls = pCallManager->GetCalls();
        for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
        {
            pCallManager->RemoveCall(lstCalls.GetAt(nIndex)->GetKey());
        }
    }
};

TEST_F(MtcCallManagerTest, DeInitClearsAllCalls)
{
    CallInfo objCallInfo;
    pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);
    pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);
    ASSERT_EQ(2, pCallManager->GetCalls().GetSize());

    pCallManager->DeInit();
    ASSERT_EQ(0, pCallManager->GetCalls().GetSize());
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

TEST_F(MtcCallManagerTest, CreateCallReturnsNullCallIfServiceIsSuspended)
{
    ON_CALL(objService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_SUSPENDED));

    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    EXPECT_EQ(DYNAMIC_CAST(IMtcCall*, MtcCallManager::s_pNullCall), pCall);
}

TEST_F(MtcCallManagerTest, CreateCallReturnsNullCallIfServiceIsIdle)
{
    ON_CALL(objService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_IDLE));

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
    ImsList<IMtcCall*> lstCalls = pCallManager->GetCalls();

    EXPECT_EQ(0, lstCalls.GetSize());
}

TEST_F(MtcCallManagerTest, GetCallsExcludingReturnsCallListOfMatchingCall)
{
    CallInfo objCallInfo;
    IMtcCall* pCall1 = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);
    IMtcCall* pCall2 = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    ImsList<IMtcCall*> lstResult = pCallManager->GetCallsExcluding(pCall1->GetKey());
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

    ImsList<IMtcCall*> lstResult = pCallManager->GetCallsByType(CallType::VOIP);
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
    ON_CALL(objNormalService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));
    ON_CALL(objContext, GetServiceByType(ServiceType::EMERGENCY))
            .WillByDefault(Return(&objEmergencyService));
    ON_CALL(objEmergencyService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));

    CallInfo objCallInfo;
    IMtcCall* pNormalCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);
    IMtcCall* pEmergencyCall = pCallManager->CreateCall(ServiceType::EMERGENCY, objCallInfo);

    ImsList<IMtcCall*> lstNormalResult = pCallManager->GetCallsByServiceType(ServiceType::NORMAL);
    EXPECT_EQ(1, lstNormalResult.GetSize());
    EXPECT_EQ(pNormalCall, lstNormalResult.GetAt(0));

    ImsList<IMtcCall*> lstEmergencyResult =
            pCallManager->GetCallsByServiceType(ServiceType::EMERGENCY);
    EXPECT_EQ(1, lstEmergencyResult.GetSize());
    EXPECT_EQ(pEmergencyCall, lstEmergencyResult.GetAt(0));

    // to delete MtcCall before MockIMtcService is deleted
    ClearAllCalls();
}

TEST_F(MtcCallManagerTest, GetCallsInConferenceReturnsCallListOfMatchingCall)
{
    CallInfo objCallInfo1;
    objCallInfo1.bConference = IMS_TRUE;
    IMtcCall* pConferenceCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo1);

    CallInfo objCallInfo2;
    objCallInfo2.bConference = IMS_FALSE;
    pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo2);

    ImsList<IMtcCall*> lstResult = pCallManager->GetCallsInConference();
    EXPECT_EQ(1, lstResult.GetSize());
    EXPECT_EQ(pConferenceCall, lstResult.GetAt(0));
}

TEST_F(MtcCallManagerTest, GetCallsByStateReturnsCallListOfMatchingCall)
{
    CallInfo objCallInfo;
    IMtcCall* pIdleCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    ImsList<IMtcCall*> lstIdleCalls = pCallManager->GetCallsByState(IMtcCall::State::IDLE);
    EXPECT_EQ(1, lstIdleCalls.GetSize());
    EXPECT_EQ(pIdleCall, lstIdleCalls.GetAt(0));

    ImsList<IMtcCall*> lstTerminatingCalls =
            pCallManager->GetCallsByState(IMtcCall::State::TERMINATING);
    EXPECT_EQ(0, lstTerminatingCalls.GetSize());
}

TEST_F(MtcCallManagerTest, RemoveCallRemovesCall)
{
    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    pCallManager->RemoveCall(pCall->GetKey());
    EXPECT_EQ(0, pCallManager->GetCalls().GetSize());
}

TEST_F(MtcCallManagerTest, RemoveCallDoesNothingIfInvalidCallKey)
{
    CallInfo objCallInfo;
    IMtcCall* pCall = pCallManager->CreateCall(ServiceType::NORMAL, objCallInfo);

    pCallManager->RemoveCall(pCall->GetKey() + 1);
    EXPECT_EQ(1, pCallManager->GetCalls().GetSize());
}
