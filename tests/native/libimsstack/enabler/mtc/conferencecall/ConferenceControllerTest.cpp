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
 * distributed under the License is distributed on an "AS IS" B ASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "conferencecall/ConferenceController.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceFactory.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/CallConnectionIdManager.h"
#include "conferencecall/IConferenceSubscriptionListener.h"
#include "call/MockIMtcCallManager.h"
#include "call/IMtcCall.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "conferencecall/MockConferenceOperationQueue.h"
#include "conferencecall/MockCallConnectionIdManager.h"
#include "conferencecall/MockConferenceFactory.h"
#include "conferencecall/MockConferenceParticipantList.h"
#include "conferencecall/MockIConferenceControllerListener.h"
#include "helper/MockICallStateProxy.h"
#include "../../../engine/interface/core/MockICoreService.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

LOCAL CallKey CONFERENCE_CALL_KEY = 100;

class ConferenceControllerTest : public ::testing::Test
{
public:
    MockIMtcContext objMockContext;
    MockIMtcCallManager objMockCallManager;
    MockICallStateProxy objMockCallStateProxy;
    MockIMtcService objMtcService;
    MockICoreService objCoreService;
    MockCallConnectionIdManager* pMockIdManager;
    MockConferenceOperationQueue* pMockQueue;
    MockConferenceFactory* pMockFactory;
    MockConferenceParticipantList* pMockParticipantList;

    MockIMtcCall* piMockConferenceCall;
    ConferenceController* pController;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockContext, GetCallManager).WillByDefault(ReturnRef(objMockCallManager));
        ON_CALL(objMockContext, GetCallStateProxy).WillByDefault(ReturnRef(objMockCallStateProxy));

        ON_CALL(objMtcService, GetICoreService).WillByDefault(Return(&objCoreService));

        pMockFactory = new MockConferenceFactory(objMockContext);
        pMockQueue = new MockConferenceOperationQueue();
        pMockParticipantList = new MockConferenceParticipantList();
        ON_CALL(*pMockFactory, CreateOperatrionQueue).WillByDefault(Return(pMockQueue));
        ON_CALL(*pMockFactory, CreateParticipantList).WillByDefault(Return(pMockParticipantList));

        piMockConferenceCall = CreateMockIMtcCall(CONFERENCE_CALL_KEY);
        pController = CreateController(piMockConferenceCall);
    }

    virtual void TearDown() override
    {
        delete pMockIdManager;
        delete pMockFactory;
        // delete pParticipantList;

        delete piMockConferenceCall;
        delete pController;
    }

    MockIMtcCall* CreateMockIMtcCall(IN CallKey nKey)
    {
        MockIMtcCall* pCall = new MockIMtcCall();
        ON_CALL(*pCall, GetKey()).WillByDefault(Return(nKey));
        return pCall;
    }

    ConferenceController* CreateController(IN MockIMtcCall* piConferenceCall)
    {
        pMockIdManager = new MockCallConnectionIdManager(objMockContext);

        MockIMtcCallContext objMockCallContext;
        ON_CALL(*piConferenceCall, GetCallContext()).WillByDefault(ReturnRef(objMockCallContext));
        ON_CALL(objMockCallContext, GetService()).WillByDefault(ReturnRef(objMtcService));
        ON_CALL(objMockCallManager, GetCallByCallKey(piConferenceCall->GetKey()))
                .WillByDefault(Return(piConferenceCall));

        return new ConferenceController(
                piConferenceCall->GetKey(), objMockContext, *pMockIdManager, *pMockFactory);
    }
};

TEST_F(ConferenceControllerTest, GetIndividualCallStateReturnsHost)
{
    EXPECT_EQ(
            pController->GetCallStatusInConference(CONFERENCE_CALL_KEY), IndividualCallState::HOST);
}

TEST_F(ConferenceControllerTest, GetIndividualCallStateReturnsIdle)
{
    const CallKey IDLE_CALL_KEY = 0;
    EXPECT_EQ(pController->GetCallStatusInConference(IDLE_CALL_KEY), IndividualCallState::IDLE);
}

TEST_F(ConferenceControllerTest, GetIndividualCallStateReturnsJoining)
{
    const CallKey JOINING_CALL_KEY = 1000;

    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_INVITE));
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(0, 0);
    pOperation->SetConfUser(new ConfUser());
    ON_CALL(*pMockQueue, GetCurrentOperation).WillByDefault(Return(pOperation));

    ON_CALL(*pMockIdManager, GetCallKey(_)).WillByDefault(Return(JOINING_CALL_KEY));

    EXPECT_EQ(
            pController->GetCallStatusInConference(JOINING_CALL_KEY), IndividualCallState::JOINING);
}

TEST_F(ConferenceControllerTest, OnConferenceCallStateEstablished)
{
    EXPECT_CALL(
            *pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_CREATE_CONFERENCE_SESSION, _))
            .Times(1);

    pController->OnCallStateChanged(
            CONFERENCE_CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_TRUE, 0);
}

TEST_F(ConferenceControllerTest, OnConferenceCallStateTerminatingWithPendingOperation)
{
    ON_CALL(*pMockQueue, HasPendingOperation).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_DESTROY_CONTROLLER, IMS_TRUE)).Times(1);

    pController->OnCallStateChanged(
            CONFERENCE_CALL_KEY, IMtcCall::State::TERMINATING, CallType::VOIP, IMS_TRUE, 0);
}

TEST_F(ConferenceControllerTest, OnConferenceCallStateTerminatingWithoutPendingOperation)
{
    ON_CALL(*pMockQueue, HasPendingOperation).WillByDefault(Return(IMS_FALSE));
    MockIConferenceControllerListener* pListener = new MockIConferenceControllerListener();
    pController->SetListener(pListener);
    EXPECT_CALL(*pListener, OnClosed(_)).Times(1);

    pController->OnCallStateChanged(
            CONFERENCE_CALL_KEY, IMtcCall::State::TERMINATING, CallType::VOIP, IMS_TRUE, 0);
}

TEST_F(ConferenceControllerTest, OnIndividualCallStateTerminating)
{
    const CallKey PARTICIPANT_CALL_KEY = 1000;
    const IMS_UINT32 PARTICIPANT_CONNECTION_ID = 9999;
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_TERMINATE_1TO1_SESSION));
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(0, 0);
    pOperation->SetConnectionId(PARTICIPANT_CONNECTION_ID);
    ON_CALL(*pMockQueue, GetCurrentOperation).WillByDefault(Return(pOperation));
    ON_CALL(*pMockIdManager, GetCallKey(PARTICIPANT_CONNECTION_ID))
            .WillByDefault(Return(PARTICIPANT_CALL_KEY));
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_TERMINATE_1TO1_SESSION, _))
            .Times(1);

    pController->OnCallStateChanged(
            PARTICIPANT_CALL_KEY, IMtcCall::State::TERMINATING, CallType::VOIP, IMS_TRUE, 0);
}

TEST_F(ConferenceControllerTest, OnSubscriptionStateSucceeded)
{
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_SUBSCRIBE, _)).Times(1);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::SUCCEEDED);
}

TEST_F(ConferenceControllerTest, OnSubscriptionStateFailed)
{
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_SUBSCRIBE, _)).Times(1);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::FAILED);
}

TEST_F(ConferenceControllerTest, OnSubscriptionStateUnsubscribed)
{
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_UNSUBSCRIBE, _)).Times(1);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::UNSUBSCRIBED);
}

TEST_F(ConferenceControllerTest, OnSubscriptionStateTerminated)
{
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_UNSUBSCRIBE, _)).Times(1);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::TERMINATED);
}

}  // namespace android
