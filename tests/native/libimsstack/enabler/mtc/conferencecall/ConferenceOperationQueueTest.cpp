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

#include "ImsList.h"
#include "MockIMtcContext.h"
#include "MockITimer.h"
#include "MtcDef.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "call/IMtcCall.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/MockIConferenceOperationQueueListener.h"
#include <gtest/gtest.h>
#include <vector>

namespace android
{

class ConferenceOperationQueueTest : public ::testing::Test
{
public:
    inline ConferenceOperationQueueTest() :
            pOperationQueue(IMS_NULL),
            pTimerService(new TestTimerService()),
            objTimer(pTimerService->GetMockTimer())
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, pTimerService);
    }

    inline virtual ~ConferenceOperationQueueTest()
    {
        delete pTimerService;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

    ConferenceOperationQueue* pOperationQueue;
    TestTimerService* pTimerService;
    MockITimer& objTimer;
    // cppcheck-suppress unusedStructMember
    MockIConferenceOperationQueueListener objListener;

protected:
    virtual void SetUp() override { pOperationQueue = new ConferenceOperationQueue(); }

    virtual void TearDown() override { delete pOperationQueue; }
};

TEST_F(ConferenceOperationQueueTest, GetNextOperationWithoutPutReturnsNull)
{
    const ConferenceOperationQueue::ConferenceOperation* pNextOperation =
            pOperationQueue->GetNextOperation();
    EXPECT_EQ(nullptr, pNextOperation);
}

TEST_F(ConferenceOperationQueueTest, CreateAndPutWithoutDelayThenGetNextReturnsOperation)
{
    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;
    pOperationQueue->CreateNPut(nAnyType, IMS_FALSE);
    ConferenceOperationQueue::ConferenceOperation* pNextOperation =
            pOperationQueue->GetNextOperation();
    EXPECT_EQ(pNextOperation->GetType(), nAnyType);
    EXPECT_FALSE(pOperationQueue->HasPendingOperation());
}

TEST_F(ConferenceOperationQueueTest, CreateAndPutWithDelayThenGetNextReturnsNull)
{
    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;
    IMS_UINT32 nAnyDelay = 10000;
    pOperationQueue->AddDelay(nAnyDelay);
    pOperationQueue->CreateNPut(nAnyType, IMS_FALSE);

    EXPECT_CALL(objTimer, SetTimer(nAnyDelay, pOperationQueue));

    const ConferenceOperationQueue::ConferenceOperation* pNextOperation =
            pOperationQueue->GetNextOperation();
    EXPECT_EQ(nullptr, pNextOperation);
}

TEST_F(ConferenceOperationQueueTest, CreateAndPutWithDelayHasPendingOperation)
{
    EXPECT_FALSE(pOperationQueue->HasPendingOperation());

    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;
    IMS_UINT32 nAnyDelay = 10000;
    pOperationQueue->AddDelay(nAnyDelay);
    pOperationQueue->CreateNPut(nAnyType, IMS_FALSE);
    EXPECT_TRUE(pOperationQueue->HasPendingOperation());
}

TEST_F(ConferenceOperationQueueTest, CreateAndPutStandAloneOperationInvokesOnOperationReady)
{
    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;
    pOperationQueue->SetListener(&objListener);

    EXPECT_CALL(objListener, OnOperationReady);
    pOperationQueue->CreateNPut(nAnyType, IMS_TRUE);
}

TEST_F(ConferenceOperationQueueTest,
        CreateAndPutNonStandAloneOperationAndSetCompletedInvokesOnOperationReady)
{
    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;
    pOperationQueue->SetListener(&objListener);

    EXPECT_CALL(objListener, OnOperationReady).Times(0);
    pOperationQueue->CreateNPut(nAnyType, IMS_FALSE);

    EXPECT_CALL(objListener, OnOperationReady);
    pOperationQueue->SetAddingOperationSetCompleted();
}

TEST_F(ConferenceOperationQueueTest, CreateAndPutWithDelayAndTimerExpiredInvokesOnOperationReady)
{
    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;
    IMS_UINT32 nAnyDelay = 10000;
    pOperationQueue->AddDelay(nAnyDelay);
    pOperationQueue->SetListener(&objListener);
    pOperationQueue->CreateNPut(nAnyType, IMS_FALSE);
    pOperationQueue->GetNextOperation();

    EXPECT_CALL(objTimer, KillTimer);
    EXPECT_CALL(objListener, OnOperationReady);
    pOperationQueue->Timer_TimerExpired(&objTimer);
}

TEST_F(ConferenceOperationQueueTest, GetCurrentOperationReturnsCurrentOperation)
{
    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;

    EXPECT_EQ(pOperationQueue->GetTypeOfCurrentOperation(), CONTROL_OPERATION_NONE);
    pOperationQueue->CreateNPut(nAnyType, IMS_FALSE);
    ConferenceOperationQueue::ConferenceOperation* pCurrentOperation =
            pOperationQueue->GetCurrentOperation();
    EXPECT_NE(nullptr, pCurrentOperation);
    EXPECT_EQ(pCurrentOperation->GetType(), nAnyType);
    EXPECT_EQ(pOperationQueue->GetTypeOfCurrentOperation(), nAnyType);
}

TEST_F(ConferenceOperationQueueTest, CreateAndPutWithUsersProvidesSameUser)
{
    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;
    ImsList<ConfUser*> objUsers;
    ConfUser objUser;
    objUsers.Append(&objUser);
    pOperationQueue->CreateNPutWithUsers(nAnyType, objUsers);

    const ConferenceOperationQueue::ConferenceOperation* pNextOperation =
            pOperationQueue->GetNextOperation();

    EXPECT_EQ(pNextOperation->GetUsers().GetAt(0), &objUser);
    EXPECT_EQ(pOperationQueue->GetUsersOfCurrentOperation().GetAt(0), &objUser);

    objUsers.Clear();
}

TEST_F(ConferenceOperationQueueTest, CreateAndPutWithUserProvidesSameUser)
{
    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;
    ConfUser objUser;
    pOperationQueue->CreateNPutWithUser(nAnyType, &objUser);

    const ConferenceOperationQueue::ConferenceOperation* pNextOperation =
            pOperationQueue->GetNextOperation();

    EXPECT_EQ(pNextOperation->GetUsers().GetAt(0), &objUser);
}

TEST_F(ConferenceOperationQueueTest, CreateAndPutWithStartParamProvidesSameParam)
{
    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;
    CallInfo objCallInfo;
    MediaInfo objMediaInfo;
    ImsList<ConfUser*> objUsers;
    ImsMap<SuppType, SuppService*> objSuppServices;
    CallStartOperationParams* pParam =
            new CallStartOperationParams(CONTROL_OPERATION_CREATE_CONFERENCE_CALL, objCallInfo,
                    objMediaInfo, objUsers, objSuppServices);
    pOperationQueue->CreateNPutWithStartParam(nAnyType, pParam);

    ConferenceOperationQueue::ConferenceOperation* pNextOperation =
            pOperationQueue->GetNextOperation();

    EXPECT_EQ(pNextOperation->GetParam(), pParam);

    objUsers.Clear();
    objSuppServices.Clear();
}

TEST_F(ConferenceOperationQueueTest, CreateAndPutWithConnectionIdProvidesSameConnectionId)
{
    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;
    IMS_UINT32 nAnyConnectionId = 100;
    pOperationQueue->CreateNPutWithId(nAnyType, nAnyConnectionId);

    ConferenceOperationQueue::ConferenceOperation* pNextOperation =
            pOperationQueue->GetNextOperation();

    EXPECT_EQ(pNextOperation->GetConnectionId(), nAnyConnectionId);
}

TEST_F(ConferenceOperationQueueTest, CreateAndPutWithTerminateReasonProvidesSameTerminateReason)
{
    IMS_UINT32 nAnyType = CONTROL_OPERATION_CREATE_CONFERENCE_CALL;
    IMS_SINT32 nTerminateReason = 100;
    pOperationQueue->CreateNPutWithReason(nAnyType, nTerminateReason);

    ConferenceOperationQueue::ConferenceOperation* pNextOperation =
            pOperationQueue->GetNextOperation();

    EXPECT_EQ(pNextOperation->GetTerminateReason(), nTerminateReason);
}

TEST_F(ConferenceOperationQueueTest, CreateAndCompleteCurrentOperationRemovesOperation)
{
    std::vector<IMS_UINT32> objOperationTypes{
            CONTROL_OPERATION_NONE, CONTROL_OPERATION_CREATE_CONFERENCE_CALL,
            CONTROL_OPERATION_SUBSCRIBE, CONTROL_OPERATION_UNSUBSCRIBE,
            CONTROL_OPERATION_REFER_INVITE, CONTROL_OPERATION_REFER_BYE,
            CONTROL_OPERATION_CHECK_CONNECTED, CONTROL_OPERATION_NOTIFY_RESULT_TO_UI,
            CONTROL_OPERATION_TERMINATE_1TO1_CALL, CONTROL_OPERATION_TERMINATE_CONFERENCE,
            CONTROL_OPERATION_DESTROY_CONTROLLER, CONTROL_OPERATION_NOTIFY_RESULT_TO_MTCCALL,
            100  // to cover default in ConvertOperationToString()
    };

    for (IMS_UINT32 eType : objOperationTypes)
    {
        IMS_UINT32 nAnyType = eType;
        pOperationQueue->CreateNPut(nAnyType, IMS_FALSE);
        const ConferenceOperationQueue::ConferenceOperation* pNextOperation =
                pOperationQueue->GetNextOperation();
        EXPECT_NE(nullptr, pNextOperation);
        pOperationQueue->CompleteCurrentOperation(nAnyType);
        pNextOperation = pOperationQueue->GetNextOperation();
        EXPECT_EQ(nullptr, pNextOperation);
    }
}

}  // namespace android
