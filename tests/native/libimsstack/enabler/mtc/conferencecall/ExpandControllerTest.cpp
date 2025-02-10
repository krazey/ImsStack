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
#include "ImsList.h"
#include "MockICoreService.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/ParticipantInfo.h"
#include "conferencecall/ConferenceController.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/ExpandController.h"
#include "conferencecall/IConferenceController.h"
#include "conferencecall/IConferenceReference.h"
#include "conferencecall/MockConferenceEventNotifier.h"
#include "conferencecall/MockConferenceFactory.h"
#include "conferencecall/MockConferenceOperationQueue.h"
#include "conferencecall/MockConferenceParticipantList.h"
#include "conferencecall/MockIConferenceControllerListener.h"
#include "conferencecall/MockIConferenceReference.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/MtcSupplementaryService.h"
#include <gtest/gtest.h>
#include <memory>

using ::testing::_;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

namespace android
{

LOCAL const CallKey CONFERENCE_CALL_KEY = 100;
LOCAL const CallKey INDIVIDUAL_CALL_KEY = CONFERENCE_CALL_KEY + 1;

// SipMethod
MATCHER_P(IsSameTargetUser, user, "")
{
    return arg->strTarget.Equals(user->strTarget);
}

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
            objConfigurationProxy(),
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
    MockMtcConfigurationProxy objConfigurationProxy;

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
        pNotifier = new MockConferenceEventNotifier(
                objCallManager, CONFERENCE_CALL_KEY, *pConnectionIdManager);
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
                    INDIVIDUAL_CALL_KEY, IMtcCall::State::TERMINATING, CallType::VOIP, IMS_TRUE,
       0);
        }
    */
};

TEST_F(ExpandControllerTest, OnReferenceStartedNotiesExpandedIfExpandingAndNoReferSubRequired)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ON_CALL(objConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_SUPPORT_CONFERENCE_REFER_SUBSCRIBE_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pNotifier, NotifyExpanded);

    pExpandController->OnReferenceStarted(&objReference);
}

TEST_F(ExpandControllerTest, OnReferenceStartedDoesNothingIfNotExpandingState)
{
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ON_CALL(objConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_SUPPORT_CONFERENCE_REFER_SUBSCRIBE_BOOL))
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

TEST_F(ExpandControllerTest,
        OnReferenceStartFailedDoesNotRecoverIfOperationIsCreateConferenceCallAndStateIsNotExpanding)
{
    // Tests RecoverOnCreating().

    pExpandController->SetStateForTest(ConferenceController::STATE_JOINING);

    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ON_CALL(*pOperationQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_CREATE_CONFERENCE_CALL));

    EXPECT_CALL(*pNotifier, NotifyExpandFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR, -1)))
            .Times(0);
    EXPECT_CALL(*pOperationQueue, Clear).Times(0);

    pExpandController->OnReferenceStartFailed(&objReference);

    EXPECT_EQ(pExpandController->GetState(), ConferenceController::STATE_JOINING);
}

TEST_F(ExpandControllerTest,
        OnReferenceStartFailedRecoversByResuming1To1SessionIfOperationIsCreateConferenceCall)
{
    // Tests RecoverOnCreating().

    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);

    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ON_CALL(*pOperationQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_CREATE_CONFERENCE_CALL));

    EXPECT_CALL(*pNotifier, NotifyExpandFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR, -1)));
    EXPECT_CALL(*pOperationQueue, Clear);
    // TODO: Check that Resume1to1Session() is called after it's implemented.

    pExpandController->OnReferenceStartFailed(&objReference);

    EXPECT_EQ(pExpandController->GetState(), ConferenceController::STATE_IDLE);
}

TEST_F(ExpandControllerTest,
        OnReferenceStartFailedRecoversByTerminatingConferenceCallIfOperationIsReferInvite)
{
    // Tests RecoverOnReferring().

    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);

    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ON_CALL(*pOperationQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_INVITE));
    ON_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_INVITE_REFER_SINGLE));

    EXPECT_CALL(*pNotifier, NotifyExpandFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR, -1)));
    EXPECT_CALL(*pOperationQueue, Clear);
    // TODO: Check that Resume1to1Session() is called after it's implemented.
    EXPECT_CALL(objConfCall, Terminate(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR, -1)));

    pExpandController->OnReferenceStartFailed(&objReference);

    EXPECT_EQ(pExpandController->GetState(), ConferenceController::STATE_IDLE);
}

TEST_F(ExpandControllerTest,
        OnReferenceStartFailedNotifiesUsersInfoWithStatusFailIfStateIsExpanding)
{
    // Tests RecoverOnReferring().

    pExpandController->SetStateForTest(ConferenceController::STATE_JOINING);

    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ON_CALL(*pOperationQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_INVITE));

    ImsList<ConfUser*> objUsers;
    ConfUser objUser;
    objUser.eStatus = STATUS_CONNECTED;
    objUsers.Append(&objUser);
    ON_CALL(*pOperationQueue, GetUsersOfCurrentOperation).WillByDefault(ReturnRef(objUsers));
    ON_CALL(*pParticipantList, GetConfUsers).WillByDefault(Return(objUsers));

    // Cannot check that pParticipantList has the ConfUser with `STATUS_FAIL`
    // since it's reflecting by ConferenceEventNotifier.
    EXPECT_CALL(*pNotifier, NotifyUsersInfo(Ref(*pParticipantList)));

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

    EXPECT_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillOnce(Return(ConfigVoice::CONFERENCE_INVITE_REFER_SINGLE));
    // TODO: Implementation required for ExpandController::StopMedia1to1Session().
    EXPECT_CALL(
            *pOperationQueue, CompleteCurrentOperation(CONTROL_OPERATION_REFER_INVITE, IMS_NULL));

    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_202, ReferSubscriptionState::ACTIVE);
}

TEST_F(ExpandControllerTest, StopMedia1to1SessionDoesNothingIfConfigIsReferMultiple)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));

    EXPECT_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillOnce(Return(ConfigVoice::CONFERENCE_INVITE_REFER_MULTIPLE));
    // TODO: Implementation required for ExpandController::StopMedia1to1Session().

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

TEST_F(ExpandControllerTest, OnReferenceUpdatedWithErrorCodeChangesUserStatus)
{
    // Tests UpdateUserStatusByReferResult().

    pExpandController->SetStateForTest(ConferenceController::STATE_JOINING);
    MockIConferenceReference objReference;
    ON_CALL(objReference, GetType).WillByDefault(Return(REFERENCE_TYPE_INVITE));
    ON_CALL(objConfigurationProxy,
            GetBoolean(ConfigVoice::KEY_SUPPORT_CONFERENCE_REFER_SUBSCRIBE_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    ConfUser objUser;
    ON_CALL(*pParticipantList, GetConfUser(&objReference)).WillByDefault(Return(&objUser));

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_400, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_SERVERERROR);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_503, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_SERVERERROR);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_403, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_FORBIDDEN);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_404, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_NOTSUPPORTED);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_415, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_NOTSUPPORTED);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_408, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_NOANSWER);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_480, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_LOWBATTERY);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_486, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_BUSY);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_499, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_NOTREACHABLE);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_500, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_INTSERVERERROR);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_603, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_REJECT);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_606, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_NOTACCEPTABLE);

    objUser.eStatus = STATUS_IDLE;
    pExpandController->OnReferenceUpdated(
            &objReference, SipStatusCode::SC_380, ReferSubscriptionState::TERMINATED);
    EXPECT_EQ(objUser.eStatus, STATUS_FAIL);
}

TEST_F(ExpandControllerTest, ProcessExpandNotifiesFailureIfNotCreatedState)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);

    EXPECT_CALL(*pNotifier, NotifyExpandFailed(CallReasonInfo(CODE_LOCAL_ILLEGAL_STATE, -1)));

    ImsList<ConfUser*> objUsers;
    pExpandController->ProcessCommand(IConferenceController::EXPAND, objUsers);
}

TEST_F(ExpandControllerTest,
        ProcessExpandCreatesConferenceCallAndDoOtherOperationsIfConfigurationIsReferSingle)
{
    ConfUser* pUser1 = new ConfUser();  // Deleted by ClearListForConfUsers().
    ConfUser* pUser2 = new ConfUser();  // Deleted by ClearListForConfUsers().
    pUser1->nConnectionId = 0;
    pUser2->nConnectionId = 1;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);

    ImsList<ConfUser*> objUsersCopied;
    ConfUser objUserCopied1(*pUser1);
    ConfUser objUserCopied2(*pUser2);
    objUsersCopied.Append(&objUserCopied1);
    objUsersCopied.Append(&objUserCopied1);
    ON_CALL(*pParticipantList, GetConfUsers).WillByDefault(Return(objUsersCopied));

    EXPECT_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillOnce(Return(ConfigVoice::CONFERENCE_INVITE_REFER_SINGLE));

    {
        InSequence s;

        EXPECT_CALL(*pOperationQueue,
                CreateNPutWithUsers(
                        CONTROL_OPERATION_CREATE_CONFERENCE_CALL, Ref(objUsers), IMS_FALSE));
        EXPECT_CALL(*pOperationQueue, CreateNPut(CONTROL_OPERATION_SUBSCRIBE, IMS_FALSE));
        EXPECT_CALL(
                *pOperationQueue, CreateNPutWithUser(CONTROL_OPERATION_REFER_INVITE, _, IMS_FALSE));
        EXPECT_CALL(*pOperationQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_FALSE));
        EXPECT_CALL(*pOperationQueue, SetAddingOperationSetCompleted);
    }

    pExpandController->ProcessCommand(IConferenceController::EXPAND, objUsers);
    EXPECT_EQ(pExpandController->GetState(), ConferenceController::STATE_EXPANDING);
}

TEST_F(ExpandControllerTest,
        ProcessExpandDoesReferAndSubscribeOperationsOnlyIfConfigurationIsReferMultiple)
{
    ConfUser* pUser1 = new ConfUser();  // Deleted by ClearListForConfUsers().
    ConfUser* pUser2 = new ConfUser();  // Deleted by ClearListForConfUsers().
    pUser1->nConnectionId = 0;
    pUser2->nConnectionId = 1;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);

    ImsList<ConfUser*> objUsersCopied;
    ConfUser objUserCopied1(*pUser1);
    ConfUser objUserCopied2(*pUser2);
    objUsersCopied.Append(&objUserCopied1);
    objUsersCopied.Append(&objUserCopied1);
    ON_CALL(*pParticipantList, GetConfUsers).WillByDefault(Return(objUsersCopied));

    EXPECT_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillOnce(Return(ConfigVoice::CONFERENCE_INVITE_REFER_MULTIPLE));

    {
        InSequence s;

        EXPECT_CALL(*pOperationQueue,
                CreateNPutWithUsers(CONTROL_OPERATION_REFER_INVITE, objUsersCopied, IMS_FALSE));
        EXPECT_CALL(*pOperationQueue, CreateNPut(CONTROL_OPERATION_SUBSCRIBE, IMS_FALSE));
        EXPECT_CALL(*pOperationQueue, SetAddingOperationSetCompleted);
    }

    pExpandController->ProcessCommand(IConferenceController::EXPAND, objUsers);
    EXPECT_EQ(pExpandController->GetState(), ConferenceController::STATE_EXPANDING);
}

TEST_F(ExpandControllerTest, StartConferenceCallInvokesStartConferenceWithUsers)
{
    ConferenceOperationQueue::ConferenceOperation objOperation(
            CONTROL_OPERATION_CREATE_CONFERENCE_CALL, 0);
    ConfUser objUser;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(&objUser);

    objOperation.SetConfUsers(objUsers);
    ON_CALL(*pOperationQueue, GetNextOperation).WillByDefault(Return(&objOperation));

    EXPECT_CALL(objConfCall, StartConference(CallType::VOIP, AString::ConstNull(), objUsers));
    pExpandController->OnOperationReady();
}

TEST_F(ExpandControllerTest, OnCallStateChangedForNonConfCallDoesNothing)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);

    EXPECT_CALL(*pParticipantList, AddUser(_)).Times(0);
    EXPECT_CALL(*pOperationQueue, CompleteCurrentOperation(_, _)).Times(0);

    const IMtcCall::State eAnyState = IMtcCall::State::ESTABLISHED;
    pExpandController->OnCallStateChanged(
            INDIVIDUAL_CALL_KEY, eAnyState, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(ExpandControllerTest, OnCallStateChangedDoesNothingIfConfigIsSingleRefer)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);

    EXPECT_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillOnce(Return(ConfigVoice::CONFERENCE_INVITE_REFER_SINGLE));

    EXPECT_CALL(*pParticipantList, AddUser(_)).Times(0);
    EXPECT_CALL(*pOperationQueue, CompleteCurrentOperation(_, _)).Times(0);

    const IMtcCall::State eAnyState = IMtcCall::State::ESTABLISHED;
    pExpandController->OnCallStateChanged(
            CONFERENCE_CALL_KEY, eAnyState, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(ExpandControllerTest, OnCallStateChangedDoesNothingIfStateIsNotExpanding)
{
    EXPECT_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillOnce(Return(ConfigVoice::CONFERENCE_INVITE_REFER_MULTIPLE));

    EXPECT_CALL(*pParticipantList, AddUser(_)).Times(0);
    EXPECT_CALL(*pOperationQueue, CompleteCurrentOperation(_, _)).Times(0);

    const IMtcCall::State eAnyState = IMtcCall::State::ESTABLISHED;
    pExpandController->OnCallStateChanged(
            CONFERENCE_CALL_KEY, eAnyState, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(ExpandControllerTest, OnCallStateChangedAddsUserToParticipantAndCompletesInviteOperation)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);

    EXPECT_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_INVITING_REFER_TYPE_INT))
            .WillOnce(Return(ConfigVoice::CONFERENCE_INVITE_REFER_MULTIPLE));

    ParticipantInfo objParticipantInfo(objConfCallContext);
    ON_CALL(objConfCallContext, GetParticipantInfo).WillByDefault(ReturnRef(objParticipantInfo));
    MtcSupplementaryService objSuppService(objConfCallContext, objConfigurationProxy);
    const AString strTargetUriWithScheme("sip:anyUri@domain.com");
    const AString strTargetUri("anyUri");
    objSuppService.Add(SuppType::TARGET_URI, strTargetUriWithScheme);
    ON_CALL(objConfCallContext, GetSupplementaryService).WillByDefault(ReturnRef(objSuppService));

    ConfUser objUser;
    objUser.strTarget = strTargetUri;
    EXPECT_CALL(*pParticipantList, AddUser(IsSameTargetUser(&objUser)));
    EXPECT_CALL(
            *pOperationQueue, CompleteCurrentOperation(CONTROL_OPERATION_REFER_INVITE, IMS_NULL));

    const IMtcCall::State eAnyState = IMtcCall::State::ESTABLISHED;
    pExpandController->OnCallStateChanged(
            CONFERENCE_CALL_KEY, eAnyState, CallType::VOIP, IMS_FALSE, 0);

    EXPECT_EQ(pExpandController->GetState(), ConferenceController::STATE_IDLE);
}

TEST_F(ExpandControllerTest, OnOperationReadyWithNotifyResultNotifiesJoinFailedIfNotReadyState)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    ConferenceOperationQueue::ConferenceOperation objOperation(
            CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, 0);
    ON_CALL(*pOperationQueue, GetNextOperation).WillByDefault(Return(&objOperation));

    // To keep the state as EXPANDING.
    ON_CALL(*pOperationQueue,
            CompleteCurrentOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_NULL))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pNotifier, NotifyExpanded);
    EXPECT_CALL(*pNotifier, NotifyUsersInfo(Ref(*pParticipantList)));
    EXPECT_CALL(*pNotifier,
            NotifyJoinFailed(CallReasonInfo(CODE_LOCAL_ILLEGAL_STATE, -1), Ref(*pParticipantList)));

    pExpandController->OnOperationReady();
}

TEST_F(ExpandControllerTest,
        OnOperationReadyWithNotifyResultDoesNotNotifyJoinFailedIfCurrentOperationIsNotifyResult)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    ConferenceOperationQueue::ConferenceOperation objOperation(
            CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, 0);
    EXPECT_CALL(*pOperationQueue, GetNextOperation)
            .WillOnce(Return(&objOperation))
            .WillRepeatedly(ReturnNull());  // To avoid infinitely invoking DoNextOperation().

    // To change the state to IDLE.
    ON_CALL(*pOperationQueue,
            CompleteCurrentOperation(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, IMS_NULL))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pNotifier, NotifyJoinFailed(_, _)).Times(0);

    pExpandController->OnOperationReady();
}

TEST_F(ExpandControllerTest, OnOperationReadyWithNotifyResultChangesStateToJoining)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    ConferenceOperationQueue::ConferenceOperation objOperation(
            CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, 0);
    ON_CALL(*pOperationQueue, GetNextOperation).WillByDefault(Return(&objOperation));

    pExpandController->OnOperationReady();
    EXPECT_EQ(pExpandController->GetState(), ConferenceController::STATE_JOINING);
}

TEST_F(ExpandControllerTest, OnOperationReadyWithNotifyPusReferInviteOperation)
{
    pExpandController->SetStateForTest(ConferenceController::STATE_EXPANDING);
    ConferenceOperationQueue::ConferenceOperation objOperation(
            CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, 0);
    ON_CALL(*pOperationQueue, GetNextOperation).WillByDefault(Return(&objOperation));

    ConfUser objUser1;
    objUser1.eStatus = STATUS_CONNECTED;  // Already joined user.
    ConfUser objUser2;
    objUser2.eStatus = STATUS_IDLE;  // To be invited user.
    ImsList<ConfUser*> objUsers;
    objUsers.Append(&objUser1);
    objUsers.Append(&objUser2);
    ON_CALL(*pParticipantList, GetConfUsers).WillByDefault(Return(objUsers));
    ON_CALL(*pParticipantList, GetSize).WillByDefault(Return(2));

    EXPECT_CALL(*pOperationQueue,
            CreateNPutWithUser(CONTROL_OPERATION_REFER_INVITE, &objUser2, IMS_FALSE));

    pExpandController->OnOperationReady();
}

}  // namespace android
