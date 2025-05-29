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

#include "CarrierConfig.h"
#include "ISipHeader.h"
#include "ImsMap.h"
#include "MockICoreService.h"
#include "MockIMessage.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MtcDef.h"
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCall.h"
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
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
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockICallStateProxy.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSubscriptionInterfaceHolder.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>
#include <vector>

using ::testing::_;
using ::testing::AnyOf;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

namespace android
{

// To avoid ambiguous input param error.
LOCAL const IMS_UINT32 EXPLICIT_INT_0 = 0;
LOCAL CallKey CONFERENCE_CALL_KEY = 100;

class TestConferenceController : public ConferenceController
{
public:
    TestConferenceController(IN CallKey nConfCallKey, IMtcContext& objContext,
            IN CallConnectionIdManager& objConnectionIdManager, IN ConferenceFactory& objFactory) :
            ConferenceController(nConfCallKey, objContext, objConnectionIdManager, objFactory)
    {
    }
    virtual ~TestConferenceController() {}

    void SetStateForTest(IN IMS_SINT32 nState) { SetState(nState); }
};

class ConferenceControllerTest : public ::testing::Test
{
public:
    MockIMtcContext objMockContext;
    MockIMtcCallManager objMockCallManager;
    MockICallStateProxy objMockCallStateProxy;
    MockIMtcService objMtcService;
    MockICoreService objCoreService;
    MockCallConnectionIdManager* pMockIdManager;
    MockConferenceFactory* pMockFactory;
    MockConferenceOperationQueue* pMockQueue;             // Deleted internally.
    MockConferenceParticipantList* pMockParticipantList;  // Deleted internally.
    MockConferenceEventNotifier* pMockNotifier;           // Deleted Interanlly.
    MockIMtcSession objMtcSession;

    MockIMtcCall* piMockConferenceCall;
    MockIMtcCallContext objMockCallContext;
    TestConferenceController* pController;
    MockMtcConfigurationProxy* pConfigurationProxy;

    MockIMtcSipInterfaceFactory* pMockInterfaceFactory;
    MockSubscriptionInterfaceHolder* pMockSubsHolder;
    MockIInterfaceHolderListener* pMockHolderListener;

protected:
    virtual void SetUp() override
    {
        pMockIdManager = IMS_NULL;
        pMockInterfaceFactory = IMS_NULL;
        pMockSubsHolder = IMS_NULL;
        pMockHolderListener = IMS_NULL;

        ON_CALL(objMockContext, GetCallManager).WillByDefault(ReturnRef(objMockCallManager));
        ON_CALL(objMockContext, GetCallStateProxy).WillByDefault(ReturnRef(objMockCallStateProxy));

        ON_CALL(objMtcService, GetICoreService).WillByDefault(Return(&objCoreService));

        pMockFactory = new MockConferenceFactory(objMockContext);
        pMockQueue = new MockConferenceOperationQueue();
        pMockParticipantList = new MockConferenceParticipantList();

        ON_CALL(*pMockFactory, CreateOperationQueue).WillByDefault(Return(pMockQueue));
        ON_CALL(*pMockFactory, CreateParticipantList).WillByDefault(Return(pMockParticipantList));

        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objMockContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        piMockConferenceCall = CreateMockIMtcCall(CONFERENCE_CALL_KEY);
        pController = CreateController(piMockConferenceCall);
    }

    virtual void TearDown() override
    {
        delete pMockIdManager;
        delete pMockFactory;
        delete piMockConferenceCall;
        delete pController;
        delete pMockInterfaceFactory;
        delete pMockSubsHolder;
        delete pMockHolderListener;
    }

    MockIMtcCall* CreateMockIMtcCall(IN CallKey nKey)
    {
        MockIMtcCall* pCall = new MockIMtcCall();
        ON_CALL(*pCall, GetKey()).WillByDefault(Return(nKey));
        return pCall;
    }

    TestConferenceController* CreateController(IN MockIMtcCall* piConferenceCall)
    {
        pMockIdManager = new MockCallConnectionIdManager(objMockContext);

        pMockNotifier = new MockConferenceEventNotifier(
                objMockCallManager, CONFERENCE_CALL_KEY, *pMockIdManager);
        ON_CALL(*pMockFactory, CreateEventNotifier(_, _)).WillByDefault(Return(pMockNotifier));
        ON_CALL(*piConferenceCall, GetCallContext()).WillByDefault(ReturnRef(objMockCallContext));
        ON_CALL(objMockCallContext, GetService()).WillByDefault(ReturnRef(objMtcService));
        ON_CALL(objMockCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMockCallManager, GetCallByCallKey(piConferenceCall->GetKey()))
                .WillByDefault(Return(piConferenceCall));

        return new TestConferenceController(
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

TEST_F(ConferenceControllerTest, GetCallStatusInConferenceReturnsHostForConferenceCall)
{
    EXPECT_EQ(
            pController->GetCallStatusInConference(CONFERENCE_CALL_KEY), IndividualCallState::HOST);
}

TEST_F(ConferenceControllerTest, GetCallStatusInConferenceReturnsIdleForIndividualCall)
{
    const CallKey IDLE_CALL_KEY = 1;
    EXPECT_EQ(pController->GetCallStatusInConference(IDLE_CALL_KEY), IndividualCallState::IDLE);
}

TEST_F(ConferenceControllerTest, GetCallStatusInConferenceReturnsIdleForDeletedConferenceCall)
{
    const CallKey IDLE_CALL_KEY = 1;
    ON_CALL(*piMockConferenceCall, GetKey()).WillByDefault(Return(IMtcCall::CALL_KEY_INVALID));
    EXPECT_EQ(pController->GetCallStatusInConference(IDLE_CALL_KEY), IndividualCallState::IDLE);
}

TEST_F(ConferenceControllerTest, GetCallStatusInConferenceReturnsJoiningForReferInvitingCall)
{
    const CallKey JOINING_CALL_KEY = 1000;

    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_INVITE));
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(0, 0);
    ConfUser objUser;
    pOperation->SetConfUser(&objUser);
    ON_CALL(*pMockQueue, GetCurrentOperation).WillByDefault(Return(pOperation));

    ON_CALL(*pMockIdManager, GetCallKey(_)).WillByDefault(Return(JOINING_CALL_KEY));

    EXPECT_EQ(
            pController->GetCallStatusInConference(JOINING_CALL_KEY), IndividualCallState::JOINING);

    delete pOperation;
}

TEST_F(ConferenceControllerTest, GetCallStatusInConferenceReturnsJoinedForTerminating1To1Call)
{
    const CallKey JOINED_CALL_KEY = 1000;

    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_TERMINATE_1TO1_CALL));
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(0, 0);
    ConfUser objUser;
    pOperation->SetConfUser(&objUser);
    ON_CALL(*pMockQueue, GetCurrentOperation).WillByDefault(Return(pOperation));

    ON_CALL(*pMockIdManager, GetCallKey(_)).WillByDefault(Return(JOINED_CALL_KEY));

    EXPECT_EQ(pController->GetCallStatusInConference(JOINED_CALL_KEY), IndividualCallState::JOINED);

    delete pOperation;
}

TEST_F(ConferenceControllerTest, GetCallStatusInConferenceReturnsInvitedForAlreadyConnectedCall)
{
    const CallKey INVITED_CALL_KEY = 1000;

    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation).WillByDefault(Return(CONTROL_OPERATION_NONE));
    ConfUser objUser1;
    objUser1.eStatus = STATUS_DISCONNECTED;  // To increse coverage.
    ConfUser objUser2;
    objUser2.eStatus = STATUS_CONNECTED;
    const IMS_UINT32 nConnectionId = 1000;
    objUser2.nConnectionId = nConnectionId;
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(2));
    ON_CALL(*pMockParticipantList, GetConfUser(EXPLICIT_INT_0)).WillByDefault(Return(&objUser1));
    ON_CALL(*pMockParticipantList, GetConfUser(1)).WillByDefault(Return(&objUser2));
    ON_CALL(*pMockIdManager, GetCallKey(nConnectionId)).WillByDefault(Return(INVITED_CALL_KEY));

    EXPECT_EQ(
            pController->GetCallStatusInConference(INVITED_CALL_KEY), IndividualCallState::INVITED);
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
    MockIConferenceControllerListener objListener;
    pController->SetListener(&objListener);
    EXPECT_CALL(objListener, OnClosed(_)).Times(1);

    pController->OnCallStateChanged(
            CONFERENCE_CALL_KEY, IMtcCall::State::TERMINATING, CallType::VOIP, IMS_TRUE, 0);
}

TEST_F(ConferenceControllerTest,
        OnConferenceCallStateTerminatingPutsUnsubscribeOperationIfSubscriptionExists)
{
    // Sets m_pSubscription exists.
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_SUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    MockIConferenceSubscriptionListener objSubsListener;
    MockConferenceSubscription* pSubscription = CreateSubscription(objSubsListener);
    ON_CALL(*pSubscription, Subscribe(_)).WillByDefault(Return(IMS_FAILURE));
    // Sets GetFocusAddress() not to have exception.
    ON_CALL(objMockCallContext, GetSession()).WillByDefault(ReturnNull());

    pController->OnOperationReady();
    delete pOperation;

    // Tests.
    ON_CALL(*pSubscription, GetState()).WillByDefault(Return(SubscriptionState::ACTIVE));
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_UNSUBSCRIBE, IMS_TRUE));

    ON_CALL(*pMockQueue, HasPendingOperation).WillByDefault(Return(IMS_FALSE));
    MockIConferenceControllerListener objListener;
    pController->SetListener(&objListener);
    EXPECT_CALL(objListener, OnClosed(_)).Times(1);

    pController->OnCallStateChanged(
            CONFERENCE_CALL_KEY, IMtcCall::State::TERMINATING, CallType::VOIP, IMS_TRUE, 0);
}

TEST_F(ConferenceControllerTest, OnCallStateChangedWithUninterestingStateDoesNothing)
{
    MockIConferenceControllerListener objListener;
    pController->SetListener(&objListener);
    EXPECT_CALL(objListener, OnClosed(_)).Times(0);
    EXPECT_CALL(*pMockQueue, CreateNPut(_, _)).Times(0);
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(_, _)).Times(0);

    // clang-format off
    std::vector<IMtcCall::State> objCallStates{
            IMtcCall::State::IDLE, IMtcCall::State::ALERTING,
            IMtcCall::State::INCOMING, IMtcCall::State::OUTGOING,
            IMtcCall::State::UPDATING};
    // clang-format on
    for (IMtcCallStateListener::State eCallState : objCallStates)
    {
        pController->OnCallStateChanged(
                CONFERENCE_CALL_KEY, eCallState, CallType::VOIP, IMS_TRUE, 0);
    }

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_CREATED);
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

TEST_F(ConferenceControllerTest, UnknownTimerExpiresDoesNothing)
{
    const static IMS_UINT32 TIMER_UNKNOWN = 100;
    pController->OnTimerExpired(TIMER_UNKNOWN);
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

    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnSubscriptionStateSucceededCompletesSubscribeOperation)
{
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_SUBSCRIBE, _)).Times(1);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::SUCCEEDED);
}

TEST_F(ConferenceControllerTest, OnSubscriptionStateSucceededDoesNotCompleteOperation)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_SIP_FLOW_ORDER_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_NOTIFY_REFER));
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

TEST_F(ConferenceControllerTest, OnSubscriptionStateNotifyReceivedWithoutParticipantDoesNothing)
{
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(0));

    EXPECT_CALL(*pMockQueue, CreateNPutWithReason(CONTROL_OPERATION_TERMINATE_CONFERENCE, _, _))
            .Times(0);

    pController->OnSubscriptionUpdated(SubscriptionUpdateType::NOTIFY_RECEIVED);
}

TEST_F(ConferenceControllerTest,
        OnSubscriptionStateNotifyReceivedCompletesSubscribeOperationIfSubscribeNotifyReferFlowOn)
{
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(1));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize(_)).WillByDefault(Return(1));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_SIP_FLOW_ORDER_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_NOTIFY_REFER));

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

TEST_F(ConferenceControllerTest, OnInviteReferenceStartedCompletesReferInviteOperation)
{
    // Sets state to Joining.
    pController->SetStateForTest(ConferenceController::STATE_JOINING);

    // Tests.
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_SUPPORT_CONFERENCE_REFER_SUBSCRIBE_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    ConfUser objUser;
    ON_CALL(*pMockParticipantList, GetConfUser(&objReference)).WillByDefault(Return(&objUser));
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_REFER_INVITE, &objUser))
            .Times(1);

    pController->OnReferenceStarted(&objReference);
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

TEST_F(ConferenceControllerTest, OnByeReferenceStartFailedSetsStateIdle)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_BYE));

    EXPECT_CALL(*pMockNotifier, NotifyDropFailed(_, _)).Times(1);

    pController->OnReferenceStartFailed(&objReference);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_IDLE);
}

TEST_F(ConferenceControllerTest, OnInviteReferenceStartFailedRemovesUserFromParticipant)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));

    ConfUser objUser;
    ON_CALL(*pMockParticipantList, GetConfUser(&objReference)).WillByDefault(Return(&objUser));
    EXPECT_CALL(*pMockParticipantList, RemoveUser(&objUser));

    pController->OnReferenceStartFailed(&objReference);
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

TEST_F(ConferenceControllerTest, OnByeReferenceUpdatedCompletesReferByeOperation)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_BYE));
    ON_CALL(*pMockParticipantList, GetConfUser(&objReference)).WillByDefault(ReturnNull());

    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_REFER_BYE, IMS_NULL));

    pController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_486, ReferSubscriptionState::ACTIVE);
}

TEST_F(ConferenceControllerTest, OnReferenceUpdatedRemovesReferenceIfReferSubTerminated)
{
    // Sets m_objIConfReferences by pReference1.
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_REFER_BYE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    ConfUser objUser1;
    pOperation->SetConfUser(&objUser1);
    // Deleted by the destructor.
    MockIConferenceReference* pReference1 = new MockIConferenceReference();
    ON_CALL(*pMockFactory, CreateReference(CONFERENCE_CALL_KEY, &objUser1, _))
            .WillByDefault(Return(pReference1));
    pController->OnOperationReady();
    delete pOperation;

    // Sets m_objIConfReferences by pReference2. To increase coverage of for statement.
    pOperation = new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_REFER_BYE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    ConfUser objUser2;
    pOperation->SetConfUser(&objUser2);
    // Deleted by RemoveReference().
    MockIConferenceReference* pReference2 = new MockIConferenceReference();
    ON_CALL(*pMockFactory, CreateReference(CONFERENCE_CALL_KEY, &objUser2, _))
            .WillByDefault(Return(pReference2));
    pController->OnOperationReady();
    delete pOperation;

    ON_CALL(*pReference2, GetType).WillByDefault(Return(REFERENCE_TYPE_BYE));
    ON_CALL(*pMockParticipantList, GetConfUser(pReference2)).WillByDefault(ReturnNull());

    EXPECT_CALL(*pMockParticipantList, ResetReference(pReference2));

    pController->OnReferenceUpdated(
            pReference2, SipStatusCode::SC_486, ReferSubscriptionState::TERMINATED);
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithCreateConferenceCallInvokesStartConference)
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

TEST_F(ConferenceControllerTest, OnOperationReadyWithSubscribeInvokesSubscribeForMoCall)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_SUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    MockIConferenceSubscriptionListener objSubsListener;
    MockConferenceSubscription* pSubscription = CreateSubscription(objSubsListener);

    const AString strTo("anyto");
    EXPECT_CALL(*pSubscription, Subscribe(strTo)).WillOnce(Return(IMS_FAILURE));

    // Sets to cover GetFocusAddress().
    MockISession objISession;
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objISession));
    CallInfo objCallInfo;
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objMockCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
    MockIMessage objMessage;
    EXPECT_CALL(objISession, GetPreviousResponse(IMessage::SESSION_START))
            .WillOnce(Return(&objMessage));
    MockIMessageUtils objMessageUtils;
    ON_CALL(objMockContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
    EXPECT_CALL(objMessageUtils,
            GetUri(&objMessage, IMS_TRUE, ISipHeader::CONTACT_NORMAL, AString::ConstNull()))
            .WillOnce(Return(strTo));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithSubscribeInvokesSubscribeForMtCall)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_SUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    MockIConferenceSubscriptionListener objSubsListener;
    MockConferenceSubscription* pSubscription = CreateSubscription(objSubsListener);

    const AString strTo("anyto");
    EXPECT_CALL(*pSubscription, Subscribe(strTo)).WillOnce(Return(IMS_FAILURE));

    // Sets to cover GetFocusAddress().
    MockISession objISession;
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objISession));
    CallInfo objCallInfo;
    objCallInfo.ePeerType = PeerType::MT;
    ON_CALL(objMockCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
    MockIMessage objMessage;
    EXPECT_CALL(objISession, GetPreviousRequest(IMessage::SESSION_START))
            .WillOnce(Return(&objMessage));
    MockIMessageUtils objMessageUtils;
    ON_CALL(objMockContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
    EXPECT_CALL(objMessageUtils,
            GetUri(&objMessage, IMS_TRUE, ISipHeader::CONTACT_NORMAL, AString::ConstNull()))
            .WillOnce(Return(strTo));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithSubscribeDoesNotInvokeSubscribeIfAlreadyExists)
{
    // Sets m_pSubscription exists.
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_SUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    MockIConferenceSubscriptionListener objSubsListener;
    MockConferenceSubscription* pSubscription = CreateSubscription(objSubsListener);
    // Sets GetFocusAddress() not to have exception.
    ON_CALL(objMockCallContext, GetSession()).WillByDefault(ReturnNull());

    EXPECT_CALL(*pSubscription, Subscribe(_)).Times(1);
    pController->OnOperationReady();

    // Tests.
    EXPECT_CALL(*pSubscription, Subscribe(_)).Times(0);
    pController->OnOperationReady();

    delete pOperation;
}

TEST_F(ConferenceControllerTest,
        OnOperationReadyWithSubscribeDoesNotInvokeSubscribeIfCreatSubscriptionFails)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_SUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    MockIConferenceSubscriptionListener objSubsListener;
    MockConferenceSubscription* pSubscription = CreateSubscription(objSubsListener);
    ON_CALL(*pMockFactory, CreateSubscription(_, _, _)).WillByDefault(ReturnNull());

    EXPECT_CALL(*pSubscription, Subscribe(_)).Times(0);
    pController->OnOperationReady();

    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithUnsubscribeInvokesUnsubscribe)
{
    // to create subscription, SUBSCRIBE should be done first.
    ConferenceOperationQueue::ConferenceOperation* pSubscribeOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_SUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pSubscribeOperation));

    MockIConferenceSubscriptionListener objSubsListener;
    MockConferenceSubscription* pSubscription = CreateSubscription(objSubsListener);

    // Sets GetFocusAddress() not to have exception.
    ON_CALL(objMockCallContext, GetSession()).WillByDefault(ReturnNull());

    pController->OnOperationReady();
    delete pSubscribeOperation;

    ConferenceOperationQueue::ConferenceOperation* pUnsubscribeOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_UNSUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pUnsubscribeOperation));

    EXPECT_CALL(*pSubscription, UnSubscribe).Times(1);

    pController->OnOperationReady();
    delete pUnsubscribeOperation;
}

TEST_F(ConferenceControllerTest,
        OnOperationReadyWithUnsubscribeDoesNotInvokeUnsubscribeIfCreateSubscriptionFails)
{
    ConferenceOperationQueue::ConferenceOperation* pUnsubscribeOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_UNSUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pUnsubscribeOperation));

    MockIConferenceSubscriptionListener objSubsListener;
    MockConferenceSubscription* pSubscription = CreateSubscription(objSubsListener);

    ON_CALL(*pMockFactory, CreateSubscription(_, _, _)).WillByDefault(ReturnNull());

    EXPECT_CALL(*pSubscription, UnSubscribe).Times(0);

    pController->OnOperationReady();
    delete pUnsubscribeOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithReferInviteInvokesSendInvite)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_REFER_INVITE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_INVITE_REFER_SINGLE));

    ConfUser objUser;
    pOperation->SetConfUser(&objUser);

    ON_CALL(*pMockParticipantList, SetReference).WillByDefault(Return());

    // Deleted in ~ConferenceController.
    MockIConferenceReference* pReference = new MockIConferenceReference();
    EXPECT_CALL(*pMockFactory, CreateReference(CONFERENCE_CALL_KEY, &objUser, _))
            .WillOnce(Return(pReference));

    EXPECT_CALL(*pReference, SendInvite(_, _)).Times(1);

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest,
        OnOperationReadyWithReferInviteInvokesCreateReferenceWithMultipleUser)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_REFER_INVITE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_INVITE_REFER_MULTIPLE));

    ConfUser objUser1;
    ConfUser objUser2;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(&objUser1);
    objUsers.Append(&objUser2);
    pOperation->SetConfUsers(objUsers);

    ON_CALL(*pMockParticipantList, SetReference).WillByDefault(Return());

    // Deleted in ~ConferenceController.
    MockIConferenceReference* pReference = new MockIConferenceReference();
    EXPECT_CALL(*pMockFactory, CreateReference(CONFERENCE_CALL_KEY, objUsers, _))
            .WillOnce(Return(pReference));
    EXPECT_CALL(*pReference, SendInvite(_, _)).WillOnce(Return(IMS_SUCCESS));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest,
        OnOperationReadyWithReferInviteInvokesSendClosedIfReferFailedWhileExpanding)
{
    // Sets State to Expanding.
    pController->SetStateForTest(ConferenceController::STATE_EXPANDING);

    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_REFER_INVITE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_INVITE_REFER_MULTIPLE));

    ConfUser objUser1;
    ConfUser objUser2;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(&objUser1);
    objUsers.Append(&objUser2);
    pOperation->SetConfUsers(objUsers);

    ON_CALL(*pMockParticipantList, SetReference).WillByDefault(Return());

    // Deleted in ~ConferenceController.
    MockIConferenceReference* pReference = new MockIConferenceReference();
    EXPECT_CALL(*pMockFactory, CreateReference(CONFERENCE_CALL_KEY, objUsers, _))
            .WillOnce(Return(pReference));
    EXPECT_CALL(*pReference, SendInvite(_, _)).WillOnce(Return(IMS_FAILURE));
    MockIConferenceControllerListener* pListener = new MockIConferenceControllerListener();
    pController->SetListener(pListener);
    EXPECT_CALL(*pListener, OnClosed(_)).Times(1);

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithReferByeInvokesSendBye)
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

TEST_F(ConferenceControllerTest, OnOperationReadyWithReferInviteClearsOngoingReferences)
{
    // Sets 1st ongoing references.
    ConferenceOperationQueue::ConferenceOperation* pOperation1 =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_REFER_BYE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation1));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_INVITE_REFER_SINGLE));
    ConfUser objUser1;
    pOperation1->SetConfUser(&objUser1);
    ON_CALL(*pMockParticipantList, SetReference).WillByDefault(Return());
    ON_CALL(*pMockParticipantList, SetReferInviteUri).WillByDefault(Return());
    // Deleted in ~ConferenceController.
    MockIConferenceReference* pReference1 = new MockIConferenceReference();
    ON_CALL(*pReference1, GetType).WillByDefault(Return(REFERENCE_TYPE_BYE));
    ON_CALL(*pReference1, GetResponseCode).WillByDefault(Return(SipStatusCode::SC_403));
    ON_CALL(*pMockFactory, CreateReference(CONFERENCE_CALL_KEY, &objUser1, _))
            .WillByDefault(Return(pReference1));
    EXPECT_CALL(*pMockParticipantList, ResetReference(pReference1)).Times(0);
    pController->OnOperationReady();
    delete pOperation1;

    // Sets 2nd ongoing references.
    ConferenceOperationQueue::ConferenceOperation* pOperation2 =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_REFER_BYE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation2));
    ConfUser objUser2;
    pOperation2->SetConfUser(&objUser2);
    ON_CALL(*pMockParticipantList, SetReference).WillByDefault(Return());
    ON_CALL(*pMockParticipantList, SetReferInviteUri).WillByDefault(Return());
    // Deleted by ClearOngoingReferences.
    MockIConferenceReference* pReference2 = new MockIConferenceReference();
    ON_CALL(*pReference2, GetType).WillByDefault(Return(REFERENCE_TYPE_BYE));
    ON_CALL(*pReference2, GetResponseCode).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(*pMockFactory, CreateReference(CONFERENCE_CALL_KEY, &objUser2, _))
            .WillByDefault(Return(pReference2));
    pController->OnOperationReady();
    delete pOperation2;

    // Tests.
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_REFER_INVITE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    ConfUser objUser;
    pOperation->SetConfUser(&objUser);

    ON_CALL(*pMockParticipantList, SetReference).WillByDefault(Return());

    // Deleted in ~ConferenceController.
    MockIConferenceReference* pReference = new MockIConferenceReference();
    ON_CALL(*pMockFactory, CreateReference(CONFERENCE_CALL_KEY, &objUser, _))
            .WillByDefault(Return(pReference));

    EXPECT_CALL(*pMockParticipantList, ResetReference(pReference2));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest,
        OnOperationReadyWithCheckConnectedInvokesIsConnectedUserAndCompletesOperation)
{
    ConfUser objUser;
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_CHECK_CONNECTED, 0);
    pOperation->SetConfUser(&objUser);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    EXPECT_CALL(*pMockParticipantList, IsConnectedUser(&objUser, _)).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_CHECK_CONNECTED, IMS_NULL));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest,
        OnOperationReadyWithCheckConnectedDoesNothingIfSubscribeNotifyReferFlowOnAndSubscribing)
{
    // Sets m_pSubscription exists.
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_SUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    MockIConferenceSubscriptionListener objSubsListener;
    MockConferenceSubscription* pSubscription = CreateSubscription(objSubsListener);
    // Sets GetFocusAddress() not to have exception.
    ON_CALL(objMockCallContext, GetSession()).WillByDefault(ReturnNull());

    pController->OnOperationReady();
    delete pOperation;

    // Tests
    IMS_SINT32 nPreviousState = pController->GetState();
    pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_CHECK_CONNECTED, 0);
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_SIP_FLOW_ORDER_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_NOTIFY_REFER));
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    pController->OnOperationReady();
    EXPECT_EQ(pController->GetState(), nPreviousState);
    delete pOperation;
}

TEST_F(ConferenceControllerTest,
        OnOperationReadyWithCheckConnectedCompletesOperationIfSubscribeNotifyReferFlowOnAndSubscriptionFailed)
{
    IMS_SINT32 nPreviousState = pController->GetState();
    ConfUser objUser;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(&objUser);
    EXPECT_CALL(*pMockParticipantList, IsConnectedUser(&objUser, _)).WillOnce(Return(IMS_TRUE));

    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_CHECK_CONNECTED, 0);
    pOperation->SetConfUsers(objUsers);
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_SIP_FLOW_ORDER_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_NOTIFY_REFER));
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_CHECK_CONNECTED, IMS_NULL));

    pController->OnOperationReady();
    EXPECT_EQ(pController->GetState(), nPreviousState);
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithNotifyResultCompletesOperationInCreatedState)
{
    pController->SetStateForTest(ConferenceController::STATE_CREATED);
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    EXPECT_CALL(
            *pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_NULL))
            .WillOnce(Return(IMS_FALSE));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest,
        OnOperationReadyWithNotifyResultNotifiesGroupCallStartedInGroupCallState)
{
    pController->SetStateForTest(ConferenceController::STATE_GROUPCALLING);
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    EXPECT_CALL(*pMockNotifier, NotifyGroupCallStarted);
    EXPECT_CALL(
            *pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_NULL))
            .WillOnce(Return(IMS_FALSE));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithNotifyResultNotifiesExpandedInExpandingState)
{
    pController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    EXPECT_CALL(*pMockNotifier, NotifyExpanded);
    EXPECT_CALL(*pMockNotifier, NotifyUsersInfo(Ref(*pMockParticipantList)));
    EXPECT_CALL(
            *pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_NULL))
            .WillOnce(Return(IMS_FALSE));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithNotifyResultNotifiesMergedInMergingState)
{
    pController->SetStateForTest(ConferenceController::STATE_MERGING);
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    EXPECT_CALL(*pMockNotifier, NotifyMerged(Ref(*pMockParticipantList), IMS_FALSE));
    EXPECT_CALL(
            *pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_NULL))
            .WillOnce(Return(IMS_FALSE));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest,
        OnOperationReadyWithNotifyResultNotifiesMergedInMergingStateWhenSubscriptionExists)
{
    // Sets m_pSubscription exists.
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_SUBSCRIBE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    MockIConferenceSubscriptionListener objSubsListener;
    MockConferenceSubscription* pSubscription = CreateSubscription(objSubsListener);
    // Sets GetFocusAddress() not to have exception.
    ON_CALL(objMockCallContext, GetSession()).WillByDefault(ReturnNull());

    EXPECT_CALL(*pSubscription, Subscribe(_)).Times(1);
    pController->OnOperationReady();

    pController->SetStateForTest(ConferenceController::STATE_MERGING);
    pOperation = new ConferenceOperationQueue::ConferenceOperation(
            CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    EXPECT_CALL(*pMockNotifier, NotifyMerged(Ref(*pMockParticipantList), IMS_TRUE));
    EXPECT_CALL(
            *pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_NULL))
            .WillOnce(Return(IMS_FALSE));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithNotifyResultNotifiesJoinedInJoiningState)
{
    pController->SetStateForTest(ConferenceController::STATE_JOINING);
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    EXPECT_CALL(*pMockNotifier, NotifyJoined(Ref(*pMockParticipantList)));
    EXPECT_CALL(
            *pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_NULL))
            .WillOnce(Return(IMS_FALSE));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithNotifyResultNotifiesDroppedInDroppingState)
{
    pController->SetStateForTest(ConferenceController::STATE_DROPPING);
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    EXPECT_CALL(*pMockNotifier, NotifyDropped(Ref(*pMockParticipantList)));
    EXPECT_CALL(
            *pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_NULL))
            .WillOnce(Return(IMS_FALSE));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithTerminate1To1CallInvokesTerminate)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_TERMINATE_1TO1_CALL, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));
    const IMS_UINT32 nConnectionId = 1000;
    pOperation->SetConnectionId(nConnectionId);
    const CallKey nCallKey = 100;
    ON_CALL(*pMockIdManager, GetCallKey(nConnectionId)).WillByDefault(Return(nCallKey));

    MockIMtcCall objCall;
    ON_CALL(objMockCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));
    EXPECT_CALL(objCall, Terminate(CallReasonInfo(CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE, -1)));

    EXPECT_CALL(
            *pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_TERMINATE_1TO1_CALL, IMS_NULL))
            .WillOnce(Return(IMS_FALSE));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithTerminateConferenceInvokesTerminate)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_TERMINATE_CONFERENCE, 0);
    pOperation->SetTerminateReason(CODE_USER_TERMINATED);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    EXPECT_CALL(*piMockConferenceCall, Terminate(CallReasonInfo(CODE_USER_TERMINATED, -1)));

    EXPECT_CALL(
            *pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_TERMINATE_CONFERENCE, IMS_NULL))
            .WillOnce(Return(IMS_FALSE));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithDestroyControllerInvokesOnClosed)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_DESTROY_CONTROLLER, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    MockIConferenceControllerListener* pListener = new MockIConferenceControllerListener();
    pController->SetListener(pListener);
    EXPECT_CALL(*pListener, OnClosed(_)).Times(1);

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithNotifyMtCallCompletesOperation)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_NOTIFY_RESULT_TO_MTCCALL, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    EXPECT_CALL(*pMockQueue,
            CompleteCurrentOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_MTCCALL, IMS_NULL))
            .WillOnce(Return(IMS_FALSE));

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(ConferenceControllerTest, OnOperationReadyWithNoneOperationDoesNothing)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(CONTROL_OPERATION_NONE, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(_, _)).Times(0);

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

TEST_F(ConferenceControllerTest, ProcessJoinPutsOneUserToReferInviteIfConfigIsSingle)
{
    ConfUser* pUser1 = new ConfUser();  // Deleted by ClearListForConfUsers()
    ConfUser* pUser2 = new ConfUser();  // Deleted by ClearListForConfUsers()
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_INVITE_REFER_SINGLE));
    EXPECT_CALL(*pMockParticipantList, GetSize)
            .WillOnce(Return(0))  // To make nStartIndex 0.
            .WillRepeatedly(Return(2));
    ON_CALL(*pMockParticipantList, GetConfUsers).WillByDefault(Return(objUsers));

    EXPECT_CALL(*pMockParticipantList, AddUser(pUser1));
    EXPECT_CALL(*pMockParticipantList, AddUser(pUser2));

    // ConfUser is copied during ConferenceParticipantList#AddUser so the pointer will be different.
    EXPECT_CALL(*pMockQueue, CreateNPutWithUser(CONTROL_OPERATION_REFER_INVITE, _, IMS_FALSE))
            .Times(2);

    pController->ProcessCommand(IConferenceController::ADD, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_JOINING);
}

TEST_F(ConferenceControllerTest, ProcessJoinPutsTwoUsersToReferInviteIfConfigIsMultiple)
{
    ConfUser* pUser1 = new ConfUser();  // Deleted by ClearListForConfUsers()
    ConfUser* pUser2 = new ConfUser();  // Deleted by ClearListForConfUsers()
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_INVITE_REFER_MULTIPLE));
    EXPECT_CALL(*pMockParticipantList, GetSize)
            .WillOnce(Return(0))  // To make nStartIndex 0.
            .WillRepeatedly(Return(2));
    ON_CALL(*pMockParticipantList, GetConfUsers).WillByDefault(Return(objUsers));

    EXPECT_CALL(*pMockParticipantList, AddUser(pUser1));
    EXPECT_CALL(*pMockParticipantList, AddUser(pUser2));
    EXPECT_CALL(
            *pMockQueue, CreateNPutWithUsers(CONTROL_OPERATION_REFER_INVITE, objUsers, IMS_FALSE));

    pController->ProcessCommand(IConferenceController::ADD, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_JOINING);
}

TEST_F(ConferenceControllerTest, ProcessJoinNotifiesMergeFailedIfStateIsNotReady)
{
    // Sets State to Joining.
    pController->SetStateForTest(ConferenceController::STATE_JOINING);

    // Tests
    ConfUser* pUser = new ConfUser();  // Deleted by ClearListForConfUsers()
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser);
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_INVITE_REFER_SINGLE));
    EXPECT_CALL(*pMockNotifier,
            NotifyJoinFailed(
                    CallReasonInfo(CODE_LOCAL_ILLEGAL_STATE, -1), Ref(*pMockParticipantList)));
    pController->ProcessCommand(IConferenceController::ADD, objUsers);
}

TEST_F(ConferenceControllerTest, ProcessDropPutsReferByeOperation)
{
    ConfUser* pUser = new ConfUser();  // Deleted by ClearListForConfUsers()
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser);

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_INVITE_REFER_SINGLE));
    ON_CALL(*pMockParticipantList, FindParticipant(_)).WillByDefault(Return(0));
    ON_CALL(*pMockParticipantList, GetConfUsers(_)).WillByDefault(Return(objUsers));

    EXPECT_CALL(*pMockQueue, CreateNPutWithUser(CONTROL_OPERATION_REFER_BYE, _, _)).Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, _)).Times(1);

    pController->ProcessCommand(IConferenceController::REMOVE, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_DROPPING);
}

TEST_F(ConferenceControllerTest, ProcessDropNotifiesDropFailedIfNoParticipantFound)
{
    ConfUser* pUser = new ConfUser();  // Deleted by ClearListForConfUsers()
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser);

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_INVITE_REFER_SINGLE));
    ON_CALL(*pMockParticipantList, FindParticipant(_)).WillByDefault(Return(-1));
    ON_CALL(*pMockParticipantList, GetConfUsers(_)).WillByDefault(Return(objUsers));

    EXPECT_CALL(*pMockNotifier,
            NotifyDropFailed(CallReasonInfo(CODE_NONE), Ref(*pMockParticipantList)));

    pController->ProcessCommand(IConferenceController::REMOVE, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_CREATED);
}

TEST_F(ConferenceControllerTest, ProcessDropNotifiesDropFailedIfStateIsNotReady)
{
    // Sets State to Dropping.
    pController->SetStateForTest(ConferenceController::STATE_DROPPING);

    // Tests.
    ConfUser* pUser = new ConfUser();  // Deleted by ClearListForConfUsers()
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser);

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_INVITE_REFER_SINGLE));
    ON_CALL(*pMockParticipantList, FindParticipant(_)).WillByDefault(Return(0));
    ON_CALL(*pMockParticipantList, GetConfUsers(_)).WillByDefault(Return(objUsers));

    EXPECT_CALL(*pMockNotifier,
            NotifyDropFailed(CallReasonInfo(CODE_NONE), Ref(*pMockParticipantList)));

    pController->ProcessCommand(IConferenceController::REMOVE, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_DROPPING);
}

TEST_F(ConferenceControllerTest, ProcessSubscribeOnParticipantPutsSubscribeOperation)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_SUBSCRIBE, IMS_TRUE)).Times(1);

    ImsList<ConfUser*> objUsers;
    pController->ProcessCommand(IConferenceController::JOINED, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_CREATED);
}

TEST_F(ConferenceControllerTest, ProcessSubscribeOnParticipantDoesNothingIfConfigNotSupported)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_ENABLE_CONFERENCE_SUBSCRIBE_BY_PARTICIPANT_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pMockQueue, CreateNPut(_, _)).Times(0);

    ImsList<ConfUser*> objUsers;
    pController->ProcessCommand(IConferenceController::JOINED, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_CREATED);
}

TEST_F(ConferenceControllerTest, ProcessGroupCallDoesNothing)
{
    ImsList<ConfUser*> objUsers;
    CallInfo objCallInfo;
    MediaInfo objMediaInfo;
    ImsMap<SuppType, SuppService*> objSuppServices;
    pController->ProcessCommand(
            IuMtcCall::STARTCONF, objUsers, objCallInfo, objMediaInfo, objSuppServices);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_CREATED);
}

TEST_F(ConferenceControllerTest, ProcessCommandWithInvalidTypeDoesNothing)
{
    EXPECT_CALL(*pMockQueue, CreateNPut(_, _)).Times(0);

    ImsList<ConfUser*> objUsers;
    pController->ProcessCommand(IConferenceController::GROUPCALL, objUsers);

    CallInfo objCallInfo;
    MediaInfo objMediaInfo;
    ImsMap<SuppType, SuppService*> objSuppServices;
    pController->ProcessCommand(
            IConferenceController::MERGE, objUsers, objCallInfo, objMediaInfo, objSuppServices);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_CREATED);
}

TEST_F(ConferenceControllerTest, ProcessExpandDoesNothing)
{
    ImsList<ConfUser*> objUsers;
    pController->ProcessCommand(IConferenceController::EXPAND, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_CREATED);
}

TEST_F(ConferenceControllerTest, ProcessMergeDoesNothing)
{
    ImsList<ConfUser*> objUsers;
    pController->ProcessCommand(IConferenceController::MERGE, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_CREATED);
}

}  // namespace android
