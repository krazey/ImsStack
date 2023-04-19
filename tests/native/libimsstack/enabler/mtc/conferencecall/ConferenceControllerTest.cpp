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

#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockITimer.h"
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCall.h"
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "conferencecall/ConferenceController.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceFactory.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/IConferenceSubscriptionListener.h"
#include "conferencecall/MockConferenceEventNotifier.h"
#include "conferencecall/MockConferenceFactory.h"
#include "conferencecall/MockConferenceOperationQueue.h"
#include "conferencecall/MockConferenceParticipantList.h"
#include "conferencecall/MockConferenceSubscription.h"
#include "conferencecall/MockIConferenceControllerListener.h"
#include "conferencecall/MockIConferenceReference.h"
#include "conferencecall/MockIConferenceSubscriptionListener.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockICoreService.h"
#include "helper/MockICallStateProxy.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSubscriptionInterfaceHolder.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AnyOf;
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
    MockConferenceEventNotifier* pMockNotifier;
    MockIMtcConfigurationManager* pMockConfigurationManager;

    MockIMtcCall* piMockConferenceCall;
    MockIMtcCallContext objMockCallContext;
    ConferenceController* pController;
    MtcConfigurationProxy* pConfigurationProxy;

    MockIMtcSipInterfaceFactory* pMockInterfaceFactory;
    MockSubscriptionInterfaceHolder* pMockSubsHolder;
    MockIInterfaceHolderListener* pMockHolderListener;

protected:
    virtual void SetUp() override
    {
        pMockIdManager = IMS_NULL;
        pMockInterfaceFactory = IMS_NULL;
        pMockSubsHolder = IMS_NULL;

        ON_CALL(objMockContext, GetCallManager).WillByDefault(ReturnRef(objMockCallManager));
        ON_CALL(objMockContext, GetCallStateProxy).WillByDefault(ReturnRef(objMockCallStateProxy));

        ON_CALL(objMtcService, GetICoreService).WillByDefault(Return(&objCoreService));

        pMockFactory = new MockConferenceFactory(objMockContext);
        pMockQueue = new MockConferenceOperationQueue();
        pMockParticipantList = new MockConferenceParticipantList();

        ON_CALL(*pMockFactory, CreateOperationQueue).WillByDefault(Return(pMockQueue));
        ON_CALL(*pMockFactory, CreateParticipantList).WillByDefault(Return(pMockParticipantList));

        pMockConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pMockConfigurationManager);
        ON_CALL(objMockContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

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

        delete pMockInterfaceFactory;
        delete pMockSubsHolder;
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

        pMockNotifier = new MockConferenceEventNotifier(objMockCallContext, *pMockIdManager);
        ON_CALL(*pMockFactory, CreateEventNotifier(_, _)).WillByDefault(Return(pMockNotifier));
        ON_CALL(*piConferenceCall, GetCallContext()).WillByDefault(ReturnRef(objMockCallContext));
        ON_CALL(objMockCallContext, GetService()).WillByDefault(ReturnRef(objMtcService));
        IMtcSession* pMtcSession = IMS_NULL;
        ON_CALL(objMockCallContext, GetSession()).WillByDefault(Return(pMtcSession));
        ON_CALL(objMockCallManager, GetCallByCallKey(piConferenceCall->GetKey()))
                .WillByDefault(Return(piConferenceCall));

        return new ConferenceController(
                piConferenceCall->GetKey(), objMockContext, *pMockIdManager, *pMockFactory);
    }

    MockConferenceSubscription* CreateSubscription(MockIConferenceSubscriptionListener& objListener)
    {
        MockConferenceSubscription* pSubscription = new MockConferenceSubscription(objMockContext,
                CONFERENCE_CALL_KEY, *pMockParticipantList, objListener, *pMockFactory);
        ON_CALL(*pMockFactory, CreateSubscription(_, _, _)).WillByDefault(Return(pSubscription));
        ON_CALL(*pSubscription, GetState).WillByDefault(Return(SubscriptionState::ACTIVE));

        pMockInterfaceFactory = new MockIMtcSipInterfaceFactory();
        pMockHolderListener = new MockIInterfaceHolderListener();
        pMockSubsHolder = new MockSubscriptionInterfaceHolder(*pMockHolderListener);

        ON_CALL(*pMockSubsHolder, ReleaseISubscription).WillByDefault(Return());
        ON_CALL(*pMockInterfaceFactory, GetISubscriptionHolder)
                .WillByDefault(Return(pMockSubsHolder));
        ON_CALL(objMockContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(*pMockInterfaceFactory));

        return pSubscription;
    }
};

TEST_F(ConferenceControllerTest, GetIndividualCallStateReturnsHost)
{
    EXPECT_EQ(
            pController->GetCallStatusInConference(CONFERENCE_CALL_KEY), IndividualCallState::HOST);
}

TEST_F(ConferenceControllerTest, GetIndividualCallStateReturnsIdle)
{
    const CallKey IDLE_CALL_KEY = 1;
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
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_CREATE_CONFERENCE_CALL, _))
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

TEST_F(ConferenceControllerTest, OnTotalCallStateChangedDoNothing)
{
    pController->OnTotalCallStateChanged(IMtcCall::State::TERMINATING);
    EXPECT_CALL(*pMockQueue, GetNextOperation).Times(0);
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(_, _)).Times(0);
}

TEST_F(ConferenceControllerTest, IsSynchronousCallRequiredReturnsTrue)
{
    EXPECT_TRUE(pController->IsSynchronousCallRequired());
}

TEST_F(ConferenceControllerTest, NullTimerExpiresDoNothing)
{
    pController->Timer_TimerExpired(IMS_NULL);
    EXPECT_CALL(*pMockQueue, GetNextOperation).Times(0);
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(_, _)).Times(0);
}

TEST_F(ConferenceControllerTest, DifferentTimerExpiresDoNothing)
{
    MockITimer objMockTimer;
    pController->Timer_TimerExpired(&objMockTimer);
    EXPECT_CALL(*pMockQueue, GetNextOperation).Times(0);
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(_, _)).Times(0);
}

TEST_F(ConferenceControllerTest, OnIndividualCallStateTerminating)
{
    const CallKey PARTICIPANT_CALL_KEY = 1000;
    const IMS_UINT32 PARTICIPANT_CONNECTION_ID = 9999;
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_TERMINATE_1TO1_CALL));
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(0, 0);
    pOperation->SetConnectionId(PARTICIPANT_CONNECTION_ID);
    ON_CALL(*pMockQueue, GetCurrentOperation).WillByDefault(Return(pOperation));
    ON_CALL(*pMockIdManager, GetCallKey(PARTICIPANT_CONNECTION_ID))
            .WillByDefault(Return(PARTICIPANT_CALL_KEY));
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_TERMINATE_1TO1_CALL, _))
            .Times(1);

    pController->OnCallStateChanged(
            PARTICIPANT_CALL_KEY, IMtcCall::State::TERMINATING, CallType::VOIP, IMS_TRUE, 0);
}

TEST_F(ConferenceControllerTest, OnSubscriptionStateSucceededCompletesSubscribeOperation)
{
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_SUBSCRIBE, _)).Times(1);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::SUCCEEDED);
}

TEST_F(ConferenceControllerTest, OnSubscriptionStateSucceededDoesNotCompleteOperation)
{
    ON_CALL(*pMockConfigurationManager, GetConferenceSipFlowOrder).WillByDefault(Return(2));
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_SUBSCRIBE, _)).Times(0);

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

    EXPECT_CALL(*pMockQueue, CreateNPutWithReason(CONTROL_OPERATION_TERMINATE_CONFERENCE, _, _))
            .Times(1);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::TERMINATED);
}

TEST_F(ConferenceControllerTest, OnSubscriptionStateNotifyReceivedNoUsers)
{
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(1));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize(_)).WillByDefault(Return(0));

    EXPECT_CALL(*pMockQueue, CreateNPutWithReason(CONTROL_OPERATION_TERMINATE_CONFERENCE, _, _))
            .Times(1);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::NOTIFY_RECEIVED);
}

TEST_F(ConferenceControllerTest,
        OnSubscriptionStateNotifyReceivedCompletesSubscribeOperationIfSubscribeNotifyReferFlowOn)
{
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(1));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize(_)).WillByDefault(Return(1));
    ON_CALL(*pMockConfigurationManager, GetConferenceSipFlowOrder).WillByDefault(Return(2));

    EXPECT_CALL(*pMockQueue,
            CompleteCurrentOperation(
                    AnyOf(CONTROL_OPERATION_SUBSCRIBE, CONTROL_OPERATION_CHECK_CONNECTED), _))
            .Times(2);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::NOTIFY_RECEIVED);
}

TEST_F(ConferenceControllerTest, OnSubscriptionStateNotifyReceivedWithTwoUsers)
{
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(1));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize(_)).WillByDefault(Return(2));

    EXPECT_CALL(*pMockNotifier, NotifyConferenceInfo(_)).Times(1);
    EXPECT_CALL(*pMockNotifier, NotifyUsersInfo(_)).Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPutWithReason(CONTROL_OPERATION_TERMINATE_CONFERENCE, _, _))
            .Times(0);
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_CHECK_CONNECTED, _))
            .Times(1);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::NOTIFY_RECEIVED);
}

TEST_F(ConferenceControllerTest, OnSubscriptionStateNotifyReceivedWithoutUsers)
{
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(1));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize(_)).WillByDefault(Return(0));

    EXPECT_CALL(*pMockNotifier, NotifyConferenceInfo(_)).Times(1);
    EXPECT_CALL(*pMockNotifier, NotifyUsersInfo(_)).Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPutWithReason(CONTROL_OPERATION_TERMINATE_CONFERENCE, _, _))
            .Times(1);
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_CHECK_CONNECTED, _))
            .Times(1);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::NOTIFY_RECEIVED);
}

TEST_F(ConferenceControllerTest, OnInviteReferenceStarted)
{
    // TODO: ConferenceConfigurationHelper
}

TEST_F(ConferenceControllerTest, OnByeReferenceStarted)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_BYE));
    ConfUser objUser;
    ON_CALL(*pMockParticipantList, GetConfUser(&objReference)).WillByDefault(Return(&objUser));

    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_REFER_BYE, &objUser))
            .Times(1);

    pController->OnReferenceStarted(&objReference);

    EXPECT_EQ(objUser.eStatus, STATUS_DISCONNECTED);
    EXPECT_EQ(objUser.eStatusCode, SipStatusCode::SC_200);
}

TEST_F(ConferenceControllerTest, OnByeReferenceStartFailed)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_BYE));

    EXPECT_CALL(*pMockNotifier, NotifyDropFailed(_, _)).Times(1);

    pController->OnReferenceStartFailed(&objReference);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_IDLE);
}

TEST_F(ConferenceControllerTest, OnInviteReferenceUpdatedSipFrag200)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ConfUser objUser;
    ON_CALL(*pMockParticipantList, GetConfUser(&objReference)).WillByDefault(Return(&objUser));

    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_REFER_INVITE, &objUser))
            .Times(1);

    pController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_200, ReferSubscriptionState::ACTIVE);

    EXPECT_EQ(objUser.eStatus, STATUS_CONNECTED);
    EXPECT_EQ(objUser.eStatusCode, SipStatusCode::SC_200);
}

TEST_F(ConferenceControllerTest, OnInviteReferenceUpdatedSipFrag100)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ConfUser objUser;
    ON_CALL(*pMockParticipantList, GetConfUser(&objReference)).WillByDefault(Return(&objUser));

    pController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_100, ReferSubscriptionState::ACTIVE);

    EXPECT_EQ(objUser.eStatus, STATUS_DIALING_OUT);
    EXPECT_EQ(objUser.eStatusCode, SipStatusCode::SC_100);
}

TEST_F(ConferenceControllerTest, OnInviteReferenceUpdatedSipFragError)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ConfUser objUser;
    ON_CALL(*pMockParticipantList, GetConfUser(&objReference)).WillByDefault(Return(&objUser));

    pController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_486, ReferSubscriptionState::ACTIVE);

    EXPECT_EQ(objUser.eStatus, STATUS_IDLE);
    EXPECT_EQ(objUser.eStatusCode, SipStatusCode::SC_486);
}

TEST_F(ConferenceControllerTest, OnOperationReadyWhenNextIsCreateConferenceCall)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_CREATE_CONFERENCE_CALL, 0);

    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    // ConferenceController(base class) does nothing. Check this in MergeController
    EXPECT_CALL(*piMockConferenceCall, StartConference(_, _, _)).Times(0);

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWhenNextIsSubscribe)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_SUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    MockIConferenceSubscriptionListener objSubsListener;
    MockConferenceSubscription* pSubscription = CreateSubscription(objSubsListener);

    // TODO: ConferenceConfigurationHelper
    EXPECT_CALL(*pSubscription, Subscribe(_)).Times(1);

    pController->OnOperationReady();
    delete pOperation;
    delete pMockHolderListener;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWhenNextIsUnsubscribe)
{
    // to create subscription, SUBSCRIBE should be done first.
    ConferenceOperationQueue::ConferenceOperation* pSubscribeOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_SUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pSubscribeOperation));

    MockIConferenceSubscriptionListener objSubsListener;
    MockConferenceSubscription* pSubscription = CreateSubscription(objSubsListener);

    pController->OnOperationReady();
    delete pSubscribeOperation;

    ConferenceOperationQueue::ConferenceOperation* pUnsubscribeOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_UNSUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pUnsubscribeOperation));

    EXPECT_CALL(*pSubscription, UnSubscribe).Times(1);

    pController->OnOperationReady();
    delete pUnsubscribeOperation;

    delete pMockHolderListener;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWhenNextIsReferInvite)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_REFER_INVITE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    ConfUser objUser;
    pOperation->SetConfUser(&objUser);

    ON_CALL(*pMockParticipantList, SetReference).WillByDefault(Return());

    MockIConferenceReference* pReference = new MockIConferenceReference();
    ON_CALL(*pMockFactory, CreateReference(CONFERENCE_CALL_KEY, &objUser, _))
            .WillByDefault(Return(pReference));

    EXPECT_CALL(*pReference, SendInvite(_, _)).Times(1);

    pController->OnOperationReady();
    delete pOperation;
    // pReference is deleted in ~ConferenceController
}

TEST_F(ConferenceControllerTest, OnOperationReadyWhenNextIsReferBye)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_REFER_BYE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    ConfUser objUser;
    pOperation->SetConfUser(&objUser);

    ON_CALL(*pMockParticipantList, SetReference).WillByDefault(Return());
    ON_CALL(*pMockParticipantList, SetReferInviteUri).WillByDefault(Return());

    MockIConferenceReference* pReference = new MockIConferenceReference();
    ON_CALL(*pMockFactory, CreateReference(CONFERENCE_CALL_KEY, &objUser, _))
            .WillByDefault(Return(pReference));

    EXPECT_CALL(*pReference, SendBye(_)).Times(1);

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWhenNextCheckConnected)
{
    ConfUser objUser;
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_CHECK_CONNECTED, 0);
    pOperation->SetConfUser(&objUser);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    EXPECT_CALL(*pMockParticipantList, IsConnectedUser(&objUser, _)).WillOnce(Return(IMS_FALSE));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest,
        OnOperationReadyWhenNextCheckConnectedDoesNothingIfSubscribeNotifyReferFlowOn)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_CHECK_CONNECTED, 0);
    ON_CALL(*pMockConfigurationManager, GetConferenceSipFlowOrder).WillByDefault(Return(2));
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, SetListenerAndListenerCalledWhenClosed)
{
    ON_CALL(*pMockQueue, HasPendingOperation).WillByDefault(Return(IMS_FALSE));
    MockIConferenceControllerListener* pListener = new MockIConferenceControllerListener();
    pController->SetListener(pListener);
    EXPECT_CALL(*pListener, OnClosed(_)).Times(1);

    pController->OnCallStateChanged(
            CONFERENCE_CALL_KEY, IMtcCall::State::TERMINATING, CallType::VOIP, IMS_TRUE, 0);
}

TEST_F(ConferenceControllerTest, ProcessJoinCommand)
{
    ConfUser* pUser = new ConfUser();
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser);

    ON_CALL(*pMockConfigurationManager, GetConferenceInvitingReferType).WillByDefault(Return(1));
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(1));

    EXPECT_CALL(*pMockParticipantList, AddUser(pUser)).Times(1);

    pController->ProcessCommand(IConferenceController::ADD, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_JOINING);
}

TEST_F(ConferenceControllerTest, ProcessDropCommand)
{
    ConfUser* pUser = new ConfUser();
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser);

    ON_CALL(*pMockConfigurationManager, GetConferenceInvitingReferType).WillByDefault(Return(1));
    ON_CALL(*pMockParticipantList, FindParticipant(_)).WillByDefault(Return(0));
    ON_CALL(*pMockParticipantList, GetConfUsers(_)).WillByDefault(Return(objUsers));

    EXPECT_CALL(*pMockQueue, CreateNPutWithUser(CONTROL_OPERATION_REFER_BYE, _, _)).Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, _)).Times(1);

    pController->ProcessCommand(IConferenceController::REMOVE, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_DROPPING);
}

TEST_F(ConferenceControllerTest, ProcessJoinedCommand)
{
    ON_CALL(*pMockConfigurationManager, IsEnableConferenceSubscribeByParticipant)
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_SUBSCRIBE, IMS_TRUE)).Times(1);

    ImsList<ConfUser*> objUsers;
    pController->ProcessCommand(IConferenceController::JOINED, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_CREATED);
}

}  // namespace android
