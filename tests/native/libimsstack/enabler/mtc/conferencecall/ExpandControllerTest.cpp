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
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "call/IMtcCall.h"
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "conferencecall/ConferenceController.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/ExpandController.h"
#include "conferencecall/IConferenceReference.h"
#include "conferencecall/MockConferenceEventNotifier.h"
#include "conferencecall/MockConferenceFactory.h"
#include "conferencecall/MockConferenceOperationQueue.h"
#include "conferencecall/MockConferenceParticipantList.h"
#include "conferencecall/MockIConferenceControllerListener.h"
#include "conferencecall/MockIConferenceReference.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockICoreService.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockMtcTimerWrapper.h"
#include "sipcore/SipStatusCode.h"
#include <gtest/gtest.h>
#include <memory>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

LOCAL const CallKey CONFERENCE_CALL_KEY = 100;
LOCAL const CallKey INDIVIDUAL_CALL_KEY = CONFERENCE_CALL_KEY + 1;

class TestExpandController : public ExpandController
{
public:
    TestExpandController(IN CallKey nConfCallKey, IMtcContext& objContext,
            IN CallConnectionIdManager& objConnectionIdManager, IN ConferenceFactory& objFactory) :
            ExpandController(nConfCallKey, objContext, objConnectionIdManager, objFactory)
    {
    }
    virtual ~TestExpandController() {}

    void SetStateForTest(IN IMS_SINT32 nState) { SetState(nState); }
};

class ExpandControllerTest : public ::testing::Test
{
public:
    ExpandControllerTest() :
            objContext(),
            objConfCall(),
            objConfCallContext(),
            objCallManager(),
            objService(),
            objCoreService(),
            objFactory(objContext),
            pConnectionIdManager(nullptr),
            pOperationQueue(IMS_NULL),
            pParticipantList(IMS_NULL),
            pNotifier(IMS_NULL),
            objMockCallStateProxy(),
            pConfigurationManager(new MockIMtcConfigurationManager()),
            objConfigurationProxy(pConfigurationManager),
            pExpandController(nullptr)
    {
    }

protected:
    MockIMtcContext objContext;
    MockIMtcCall objConfCall;
    MockIMtcCallContext objConfCallContext;
    MockIMtcCallManager objCallManager;
    MockIMtcService objService;
    MockICoreService objCoreService;
    MockConferenceFactory objFactory;
    std::unique_ptr<MockCallConnectionIdManager> pConnectionIdManager;
    MockConferenceOperationQueue* pOperationQueue;    // Deleted internally.
    MockConferenceParticipantList* pParticipantList;  // Deleted internally.
    MockConferenceEventNotifier* pNotifier;           // Deleted internally.
    MockICallStateProxy objMockCallStateProxy;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy objConfigurationProxy;

    std::unique_ptr<TestExpandController> pExpandController;

    virtual void SetUp() override
    {
        pOperationQueue = new MockConferenceOperationQueue();
        pParticipantList = new MockConferenceParticipantList();

        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objMockCallStateProxy));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));

        ON_CALL(objFactory, CreateOperationQueue).WillByDefault(Return(pOperationQueue));
        ON_CALL(objFactory, CreateParticipantList).WillByDefault(Return(pParticipantList));

        ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
        ON_CALL(objConfCall, GetCallContext()).WillByDefault(ReturnRef(objConfCallContext));
        ON_CALL(objConfCall, GetKey()).WillByDefault(Return(CONFERENCE_CALL_KEY));
        ON_CALL(objConfCallContext, GetService()).WillByDefault(ReturnRef(objService));
        ON_CALL(objCallManager, GetCallByCallKey(objConfCall.GetKey()))
                .WillByDefault(Return(&objConfCall));

        pConnectionIdManager = std::make_unique<MockCallConnectionIdManager>(objContext);
        pNotifier = new MockConferenceEventNotifier(objConfCallContext, *pConnectionIdManager);
        ON_CALL(objFactory, CreateEventNotifier(_, _)).WillByDefault(Return(pNotifier));

        pExpandController = std::make_unique<TestExpandController>(
                CONFERENCE_CALL_KEY, objContext, *pConnectionIdManager, objFactory);
    }

    virtual void TearDown() override {}

    // TODO: Check if it's meaningful in ExpandController.
    // Currently, it doesn't work due to ExpandController#OnCallUpdated.
    /*
        void SetIndividualCallTerminated()
        {
            ON_CALL(*pOperationQueue, GetTypeOfCurrentOperation)
                    .WillByDefault(Return(CONTROL_OPERATION_TERMINATE_1TO1_CALL));

            const IMS_UINT32 nConnectionId = 10;
            ConferenceOperationQueue::ConferenceOperation objOperation(0, 0);
            objOperation.SetConnectionId(nConnectionId);
            ON_CALL(*pOperationQueue, GetCurrentOperation).WillByDefault(Return(&objOperation));
            ON_CALL(*pConnectionIdManager, GetCallKey(nConnectionId))
                    .WillByDefault(Return(INDIVIDUAL_CALL_KEY));
            pExpandController->OnCallStateChanged(
                    INDIVIDUAL_CALL_KEY, IMtcCall::State::TERMINATING, CallType::VOIP, IMS_TRUE, 0);
        }
    */
};

TEST_F(ExpandControllerTest, OnReferenceStartedNotiesExpandedIfExpandingAndNoReferSubRequired)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ON_CALL(*pConfigurationManager, IsSupportConferenceReferSubscribe)
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pNotifier, NotifyExpanded);

    pExpandController->OnReferenceStarted(&objReference);
}

TEST_F(ExpandControllerTest, OnReferenceStartedDoesNothingIfNotExpandingState)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ON_CALL(*pConfigurationManager, IsSupportConferenceReferSubscribe)
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pNotifier, NotifyExpanded).Times(0);

    pExpandController->OnReferenceStarted(&objReference);
}

TEST_F(ExpandControllerTest, OnReferenceStartFailedInvokesNotifyDropFailedIfByeReference)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_BYE));

    EXPECT_CALL(*pNotifier, NotifyDropFailed(CallReasonInfo(CODE_NONE), Ref(*pParticipantList)));
    pExpandController->OnReferenceStartFailed(&objReference);
}

TEST_F(ExpandControllerTest, OnReferenceStartFailedCompletesOperation)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));

    ConfUser objUser;
    ON_CALL(*pParticipantList, GetConfUser(&objReference)).WillByDefault(Return(&objUser));

    EXPECT_CALL(
            *pOperationQueue, CompleteCurrentOperation(CONTROL_OPERATION_REFER_INVITE, &objUser));

    pExpandController->OnReferenceStartFailed(&objReference);

    EXPECT_EQ(objUser.eStatus, STATUS_FAIL);
}

TEST_F(ExpandControllerTest, OnReferenceStartFailedInvokesSendClosedIfExpandingState)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));

    EXPECT_CALL(*pOperationQueue, HasPendingOperation).WillOnce(Return(IMS_FALSE));
    MockIConferenceControllerListener objListener;
    pExpandController->SetListener(&objListener);
    EXPECT_CALL(objListener, OnClosed(pExpandController.get()));

    pExpandController->OnReferenceStartFailed(&objReference);
}

TEST_F(ExpandControllerTest, OnReferenceStartFailedPutsNotifyOperationIfJoiningState)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_JOINING);
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));

    EXPECT_CALL(*pOperationQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_TRUE));

    pExpandController->OnReferenceStartFailed(&objReference);
}

TEST_F(ExpandControllerTest, OnReferenceUpdatedCompletesOperationIfByeReference)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_BYE));

    EXPECT_CALL(*pOperationQueue, CompleteCurrentOperation(CONTROL_OPERATION_REFER_BYE, IMS_NULL));

    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_202, ReferSubscriptionState::ACTIVE);
}

TEST_F(ExpandControllerTest, OnReferenceUpdatedInvokesStopMedia1to1SessionIfExpandingAndSuccess)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));

    EXPECT_CALL(*pConfigurationManager, GetConferenceInvitingReferType)
            .WillOnce(Return(CarrierConfig::ImsVoice::CONFERENCE_INVITE_REFER_SINGLE));
    // TODO: Implementation required.
    EXPECT_CALL(
            *pOperationQueue, CompleteCurrentOperation(CONTROL_OPERATION_REFER_INVITE, IMS_NULL));

    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_202, ReferSubscriptionState::ACTIVE);
}

TEST_F(ExpandControllerTest, OnReferenceUpdatedNotifiesUsersInfoAndResultIfJoiningState)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_JOINING);
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));

    EXPECT_CALL(*pNotifier, NotifyUsersInfo(Ref(*pParticipantList)));
    EXPECT_CALL(*pOperationQueue, CompleteCurrentOperation(_, _)).Times(0);
    EXPECT_CALL(*pOperationQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_TRUE));

    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_202, ReferSubscriptionState::TERMINATED);
}

TEST_F(ExpandControllerTest, OnReferenceUpdatedInvokesSendClosedIfExpandingStateAndFailure)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));

    EXPECT_CALL(*pOperationQueue, HasPendingOperation).WillOnce(Return(IMS_FALSE));
    MockIConferenceControllerListener objListener;
    pExpandController->SetListener(&objListener);
    EXPECT_CALL(objListener, OnClosed(pExpandController.get()));

    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_403, ReferSubscriptionState::TERMINATED);
}

TEST_F(ExpandControllerTest, OnReferenceUpdatedNotifiesUsersInfoIfNotExpandingStateAndFailure)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_DROPPING);
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));

    EXPECT_CALL(*pNotifier, NotifyUsersInfo(Ref(*pParticipantList)));

    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_403, ReferSubscriptionState::TERMINATED);
}

TEST_F(ExpandControllerTest,
        OnReferenceUpdatedDoesNotStartWaitTimerIfProvisionalResponseBefore1To1CallTerminated)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));

    std::unique_ptr<MockMtcTimerWrapper> pTimerWrapper = std::make_unique<MockMtcTimerWrapper>();
    EXPECT_CALL(*pTimerWrapper, Start(_, _)).Times(0);
    ON_CALL(objContext, CreateTimer)
            .WillByDefault(Invoke(
                    [&]()
                    {
                        return std::move(pTimerWrapper);
                    }));

    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_100, ReferSubscriptionState::ACTIVE);
}

// TODO: Check if this is valid case. Otherwise, the implementation must be changed.
/*
TEST_F(ExpandControllerTest,
        OnReferenceUpdatedStartsWaitTimerIfProvisionalResponseAfter1To1CallTerminated)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    SetIndividualCallTerminated();

    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));

    std::unique_ptr<MockMtcTimerWrapper> pTimerWrapper = std::make_unique<MockMtcTimerWrapper>();

    // EXPECT_CALL(*pTimerWrapper, Start(0, 3000));
    ON_CALL(objContext, CreateTimer)
            .WillByDefault(Invoke(
                    [&]()
                    {
                        return std::move(pTimerWrapper);
                    }));

    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_100, ReferSubscriptionState::ACTIVE);
}
*/

TEST_F(ExpandControllerTest, OnCallStateChangedForNonConfCallDoesNothing)
{
    EXPECT_CALL(*pParticipantList, AddUser(_)).Times(0);
    EXPECT_CALL(*pOperationQueue, CompleteCurrentOperation(_, _)).Times(0);

    const IMtcCall::State eAnyState = IMtcCall::State::ESTABLISHED;
    pExpandController->OnCallStateChanged(
            INDIVIDUAL_CALL_KEY, eAnyState, CallType::VOIP, IMS_FALSE, 0);
}

}  // namespace android
