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
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCall.h"
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceFactory.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "conferencecall/IConferenceSubscriptionListener.h"
#include "conferencecall/MergeController.h"
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
#include "helper/MockMtcTimerWrapper.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSubscriptionInterfaceHolder.h"
#include "sipcore/SipStatusCode.h"
#include <gtest/gtest.h>
#include <memory>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::AnyOf;
using ::testing::Invoke;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

namespace android
{

// To avoid ambiguous input param error.
LOCAL const IMS_UINT32 EXPLICIT_INT_0 = 0;
LOCAL const CallKey CONFERENCE_CALL_KEY = 100;

class MergeControllerTest : public ::testing::Test
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
    std::unique_ptr<MockMtcTimerWrapper> pMockTimerWrapper;

    MockIMtcCall* piMockConferenceCall;
    MockIMtcCallContext objMockCallContext;
    MergeController* pController;
    MtcConfigurationProxy* pConfigurationProxy;

protected:
    virtual void SetUp() override
    {
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
        delete pConfigurationProxy;
    }

    MockIMtcCall* CreateMockIMtcCall(IN CallKey nKey)
    {
        MockIMtcCall* pCall = new MockIMtcCall();
        ON_CALL(*pCall, GetKey()).WillByDefault(Return(nKey));
        return pCall;
    }

    MergeController* CreateController(IN MockIMtcCall* piConferenceCall)
    {
        pMockIdManager = new MockCallConnectionIdManager(objMockContext);

        pMockNotifier = new MockConferenceEventNotifier(
                objMockCallManager, CONFERENCE_CALL_KEY, *pMockIdManager);
        ON_CALL(*pMockFactory, CreateEventNotifier(_, _)).WillByDefault(Return(pMockNotifier));
        ON_CALL(*piConferenceCall, GetCallContext()).WillByDefault(ReturnRef(objMockCallContext));
        ON_CALL(objMockCallContext, GetService()).WillByDefault(ReturnRef(objMtcService));
        IMtcSession* pMtcSession = IMS_NULL;
        ON_CALL(objMockCallContext, GetSession()).WillByDefault(Return(pMtcSession));
        ON_CALL(objMockCallManager, GetCallByCallKey(piConferenceCall->GetKey()))
                .WillByDefault(Return(piConferenceCall));

        return new MergeController(
                piConferenceCall->GetKey(), objMockContext, *pMockIdManager, *pMockFactory);
    }
};

TEST_F(MergeControllerTest, ProcessMergeCommandWithSubscribeAndRefer)
{
    ConfUser* pUser1 = new ConfUser();  // Deleted by ClearListForConfUsers().
    ConfUser* pUser2 = new ConfUser();  // Deleted by ClearListForConfUsers().
    pUser1->nConnectionId = 0;
    pUser2->nConnectionId = 1;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);

    // Sets up CallType.
    // TODO: Check the value of m_eStartCallType.
    MockIMtcCall objCall1;
    ON_CALL(objCall1, GetCallType).WillByDefault(Return(CallType::VT));
    MockIMtcCall objCall2;
    ON_CALL(objCall2, GetCallType).WillByDefault(Return(CallType::VT));
    EXPECT_CALL(objMockCallManager, GetCallByCallKey(_))
            .WillOnce(Return(&objCall1))
            .WillOnce(Return(&objCall2));

    ImsList<ConfUser*> objUsersCopied;
    ConfUser objUserCopied1(*pUser1);
    ConfUser objUserCopied2(*pUser2);
    objUsersCopied.Append(&objUserCopied1);
    objUsersCopied.Append(&objUserCopied1);

    ON_CALL(*pMockConfigurationManager, GetConferenceInvitingReferType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::CONFERENCE_INVITE_REFER_SINGLE));
    ON_CALL(*pMockConfigurationManager, GetConferenceSubscribeType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::CONFERENCE_SUBSCRIBE_TYPE_IN_DIALOG));
    ON_CALL(*pMockConfigurationManager, GetConferenceSipFlowOrder)
            .WillByDefault(
                    Return(CarrierConfig::ImsVoice::CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_REFER));

    EXPECT_CALL(*pMockParticipantList, GetSize)
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));

    ON_CALL(*pMockParticipantList, GetConfUsers).WillByDefault(Return(objUsersCopied));

    EXPECT_CALL(*pMockParticipantList, AddUser(pUser1)).Times(1);
    EXPECT_CALL(*pMockParticipantList, AddUser(pUser2)).Times(1);

    EXPECT_CALL(*pMockQueue,
            CreateNPutWithUsers(CONTROL_OPERATION_CREATE_CONFERENCE_CALL, _, IMS_FALSE))
            .Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_SUBSCRIBE, _)).Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPutWithUser(CONTROL_OPERATION_REFER_INVITE, _, IMS_FALSE))
            .Times(2);
    EXPECT_CALL(*pMockQueue, CreateNPutWithId(CONTROL_OPERATION_TERMINATE_1TO1_CALL, _, _))
            .Times(2);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, _)).Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_MTCCALL, _)).Times(1);

    pController->ProcessCommand(IConferenceController::MERGE, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_MERGING);
}

TEST_F(MergeControllerTest, ProcessMergeCommandWithReferAndSubscribeFlow)
{
    ConfUser* pUser1 = new ConfUser();
    ConfUser* pUser2 = new ConfUser();
    pUser1->nConnectionId = 0;
    pUser2->nConnectionId = 1;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);

    // Sets up CallType.
    // TODO: Check the value of m_eStartCallType.
    ON_CALL(*pMockConfigurationManager, GetCallTypeAfterAudioAndVideoCallMerged)
            .WillByDefault(Return(1));  // VOIP
    MockIMtcCall objCall1;
    ON_CALL(objCall1, GetCallType).WillByDefault(Return(CallType::VT));
    MockIMtcCall objCall2;
    ON_CALL(objCall2, GetCallType).WillByDefault(Return(CallType::VOIP));
    EXPECT_CALL(objMockCallManager, GetCallByCallKey(_))
            .WillOnce(Return(&objCall1))
            .WillOnce(Return(&objCall2));

    ImsList<ConfUser*> objUsersCopied;
    ConfUser objUserCopied1(*pUser1);
    ConfUser objUserCopied2(*pUser2);
    objUsersCopied.Append(&objUserCopied1);
    objUsersCopied.Append(&objUserCopied1);

    ON_CALL(*pMockConfigurationManager, GetConferenceInvitingReferType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::CONFERENCE_INVITE_REFER_SINGLE));
    ON_CALL(*pMockConfigurationManager, GetConferenceSubscribeType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::CONFERENCE_SUBSCRIBE_TYPE_IN_DIALOG));
    ON_CALL(*pMockConfigurationManager, GetConferenceSipFlowOrder)
            .WillByDefault(
                    Return(CarrierConfig::ImsVoice::CONFERENCE_SIP_FLOW_REFER_AND_SUBSCRIBE));

    EXPECT_CALL(*pMockParticipantList, GetSize)
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));

    ON_CALL(*pMockParticipantList, GetConfUsers).WillByDefault(Return(objUsersCopied));

    EXPECT_CALL(*pMockParticipantList, AddUser(pUser1)).Times(1);
    EXPECT_CALL(*pMockParticipantList, AddUser(pUser2)).Times(1);

    EXPECT_CALL(*pMockQueue,
            CreateNPutWithUsers(CONTROL_OPERATION_CREATE_CONFERENCE_CALL, _, IMS_FALSE))
            .Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_SUBSCRIBE, _)).Times(1);

    EXPECT_CALL(*pMockQueue, CreateNPutWithUser(CONTROL_OPERATION_REFER_INVITE, _, IMS_FALSE))
            .Times(2);
    EXPECT_CALL(*pMockQueue, CreateNPutWithId(CONTROL_OPERATION_TERMINATE_1TO1_CALL, _, _))
            .Times(2);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, _)).Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_MTCCALL, _)).Times(1);

    // Because config is CONFERENCE_SIP_FLOW_REFER_AND_SUBSCRIBE.
    EXPECT_CALL(*pMockQueue, CreateNPutWithUser(CONTROL_OPERATION_CHECK_CONNECTED, _, IMS_FALSE))
            .Times(0);

    pController->ProcessCommand(IConferenceController::MERGE, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_MERGING);
}

TEST_F(MergeControllerTest, ProcessMergeCommandWithSubscribeNotifyReferFlow)
{
    ConfUser* pUser1 = new ConfUser();
    ConfUser* pUser2 = new ConfUser();
    pUser1->nConnectionId = 0;
    pUser2->nConnectionId = 1;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);

    // Sets up CallType.
    // TODO: Check the value of m_eStartCallType.
    ON_CALL(*pMockConfigurationManager, GetCallTypeAfterAudioAndVideoCallMerged)
            .WillByDefault(Return(1));  // CallType::VOIP
    MockIMtcCall objCall1;
    ON_CALL(objCall1, GetCallType).WillByDefault(Return(CallType::VT));
    MockIMtcCall objCall2;
    ON_CALL(objCall2, GetCallType).WillByDefault(Return(CallType::VOIP));
    EXPECT_CALL(objMockCallManager, GetCallByCallKey(_))
            .WillOnce(Return(&objCall1))
            .WillOnce(Return(&objCall2));

    ImsList<ConfUser*> objUsersCopied;
    ConfUser objUserCopied1(*pUser1);
    ConfUser objUserCopied2(*pUser2);
    objUsersCopied.Append(&objUserCopied1);
    objUsersCopied.Append(&objUserCopied1);

    ON_CALL(*pMockConfigurationManager, GetConferenceInvitingReferType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::CONFERENCE_INVITE_REFER_SINGLE));
    ON_CALL(*pMockConfigurationManager, GetConferenceSubscribeType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::CONFERENCE_SUBSCRIBE_TYPE_IN_DIALOG));
    ON_CALL(*pMockConfigurationManager, GetConferenceSipFlowOrder)
            .WillByDefault(Return(
                    CarrierConfig::ImsVoice::CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_NOTIFY_REFER));

    EXPECT_CALL(*pMockParticipantList, GetSize)
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));

    ON_CALL(*pMockParticipantList, GetConfUsers).WillByDefault(Return(objUsersCopied));

    EXPECT_CALL(*pMockParticipantList, AddUser(pUser1)).Times(1);
    EXPECT_CALL(*pMockParticipantList, AddUser(pUser2)).Times(1);

    EXPECT_CALL(*pMockQueue,
            CreateNPutWithUsers(CONTROL_OPERATION_CREATE_CONFERENCE_CALL, _, IMS_FALSE))
            .Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_SUBSCRIBE, _)).Times(1);
    EXPECT_CALL(*pMockQueue,
            CreateNPutWithUser(
                    AnyOf(CONTROL_OPERATION_REFER_INVITE, CONTROL_OPERATION_CHECK_CONNECTED), _,
                    IMS_FALSE))
            .Times(4);
    EXPECT_CALL(*pMockQueue, CreateNPutWithId(CONTROL_OPERATION_TERMINATE_1TO1_CALL, _, _))
            .Times(2);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, _)).Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_MTCCALL, _)).Times(1);

    pController->ProcessCommand(IConferenceController::MERGE, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_MERGING);
}

TEST_F(MergeControllerTest, ProcessMergeCommandWithoutRefer)
{
    ConfUser* pUser1 = new ConfUser();
    ConfUser* pUser2 = new ConfUser();
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);

    // Sets up CallType.
    // TODO: Check the value of m_eStartCallType.
    MockIMtcCall objCall1;
    ON_CALL(objCall1, GetCallType).WillByDefault(Return(CallType::VOIP));
    MockIMtcCall objCall2;
    ON_CALL(objCall2, GetCallType).WillByDefault(Return(CallType::VOIP));
    EXPECT_CALL(objMockCallManager, GetCallByCallKey(_))
            .WillOnce(Return(&objCall1))
            .WillOnce(Return(&objCall2));

    ON_CALL(*pMockConfigurationManager, GetConferenceInvitingReferType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::CONFERENCE_INVITE_COPYCONTROL));

    EXPECT_CALL(*pMockParticipantList, GetSize)
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));

    EXPECT_CALL(*pMockParticipantList, AddUser(pUser1)).Times(1);
    EXPECT_CALL(*pMockParticipantList, AddUser(pUser2)).Times(1);

    EXPECT_CALL(*pMockQueue, CreateNPutWithUsers(CONTROL_OPERATION_CREATE_CONFERENCE_CALL, _, _))
            .Times(1);
    EXPECT_CALL(*pMockQueue, CreateNPut(CONTROL_OPERATION_NOTIFY_RESULT_TO_UI, _)).Times(1);

    pController->ProcessCommand(IConferenceController::MERGE, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_MERGING);
}

TEST_F(MergeControllerTest, ProcessMergeInvokesMergeFailedIfStateIsNotReady)
{
    // Sets State to Merging
    ConfUser* pUser1 = new ConfUser();
    ConfUser* pUser2 = new ConfUser();
    pUser1->nConnectionId = 0;
    pUser2->nConnectionId = 1;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);

    MockIMtcCall objCall1;
    ON_CALL(objCall1, GetCallType).WillByDefault(Return(CallType::VOIP));
    MockIMtcCall objCall2;
    ON_CALL(objCall2, GetCallType).WillByDefault(Return(CallType::VOIP));
    EXPECT_CALL(objMockCallManager, GetCallByCallKey(_))
            .WillOnce(Return(&objCall1))
            .WillOnce(Return(&objCall2));

    ImsList<ConfUser*> objUsersCopied;
    ConfUser objUserCopied1(*pUser1);
    ConfUser objUserCopied2(*pUser2);
    objUsersCopied.Append(&objUserCopied1);
    objUsersCopied.Append(&objUserCopied1);

    ON_CALL(*pMockConfigurationManager, GetConferenceInvitingReferType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::CONFERENCE_INVITE_REFER_SINGLE));
    ON_CALL(*pMockConfigurationManager, GetConferenceSubscribeType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::CONFERENCE_SUBSCRIBE_TYPE_IN_DIALOG));
    ON_CALL(*pMockConfigurationManager, GetConferenceSipFlowOrder)
            .WillByDefault(
                    Return(CarrierConfig::ImsVoice::CONFERENCE_SIP_FLOW_SUBSCRIBE_AND_REFER));

    EXPECT_CALL(*pMockParticipantList, GetSize)
            .Times(AnyNumber())
            .WillOnce(Return(0))
            .WillRepeatedly(Return(2));

    ON_CALL(*pMockParticipantList, GetConfUsers).WillByDefault(Return(objUsersCopied));
    pController->ProcessCommand(IConferenceController::MERGE, objUsers);

    EXPECT_EQ(pController->GetState(), ConferenceController::STATE_MERGING);

    // Tests the case
    EXPECT_CALL(*pMockNotifier, NotifyMergeFailed(CallReasonInfo(CODE_NONE)));
    pController->ProcessCommand(IConferenceController::MERGE, objUsers);
}

TEST_F(MergeControllerTest, ProcessMergeInvokesMergeFailedIfIfUserSizeIsNotTwo)
{
    ConfUser* pUser1 = new ConfUser();
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);

    MockIConferenceControllerListener objListener;
    pController->SetListener(&objListener);

    EXPECT_CALL(*pMockNotifier, NotifyMergeFailed(CallReasonInfo(CODE_NONE)));
    EXPECT_CALL(*pMockQueue, HasPendingOperation).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objListener, OnClosed(pController));

    pController->ProcessCommand(IConferenceController::MERGE, objUsers);
}

TEST_F(MergeControllerTest, OnOperationReadyWhenNextCreateConference)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_CREATE_CONFERENCE_CALL, 0);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    EXPECT_CALL(*piMockConferenceCall, StartConference(_, _, _)).Times(1);

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(MergeControllerTest, OnIndividualCallTerminatedStartsSipFragWaitTimer)
{
    // Tests that the timer is started when the invited call is terminated.
    pMockTimerWrapper = std::make_unique<MockMtcTimerWrapper>();
    // Refer to ConferenceController.h
    // TIMER_FINAL_SIPFRAG_WAIT = 0;
    // TIME_FINAL_SIPFRAG_WAIT = 3000;
    EXPECT_CALL(*pMockTimerWrapper, Start(0, 3000));

    ON_CALL(objMockContext, CreateTimer)
            .WillByDefault(Invoke(
                    [&]()
                    {
                        return std::move(pMockTimerWrapper);
                    }));

    const CallKey nNonConfCallKey = 1;
    const CallType eAnyType = CallType::VOIP;
    const IMS_BOOL bAnyEmergency = IMS_FALSE;
    const IMS_UINT32 nAnyReason = 0;
    pController->OnCallStateChanged(
            nNonConfCallKey, IMtcCall::State::TERMINATING, eAnyType, bAnyEmergency, nAnyReason);
}

TEST_F(MergeControllerTest, OnIndividualCallTerminatedUpdatesUserStateIfCopyControlMode)
{
    pMockTimerWrapper = std::make_unique<MockMtcTimerWrapper>();
    EXPECT_CALL(*pMockTimerWrapper, Stop(_)).Times(0);  // Because m_pTimer is null.
    EXPECT_CALL(*pMockTimerWrapper, Start(0, 3000));

    ON_CALL(objMockContext, CreateTimer)
            .WillByDefault(Invoke(
                    [&]()
                    {
                        return std::move(pMockTimerWrapper);
                    }));

    ON_CALL(*pMockConfigurationManager, GetConferenceInvitingReferType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::CONFERENCE_INVITE_COPYCONTROL));

    ConfUser objUser1;
    objUser1.eStatus = STATUS_IDLE;
    const IMS_UINT32 nAnyConnectionId1 = 100;
    objUser1.nConnectionId = nAnyConnectionId1;
    ConfUser objUser2;
    objUser2.eStatus = STATUS_IDLE;
    const IMS_UINT32 nAnyConnectionId2 = 200;
    objUser2.nConnectionId = nAnyConnectionId2;
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(2));
    // 0 is ambiguous due to another GetConfUser(IConferenceReference*).
    ON_CALL(*pMockParticipantList, GetConfUser(EXPLICIT_INT_0)).WillByDefault(Return(&objUser1));
    ON_CALL(*pMockParticipantList, GetConfUser(1)).WillByDefault(Return(&objUser2));

    const CallKey n1to1Key1 = 10;
    ON_CALL(*pMockIdManager, GetCallKey(nAnyConnectionId1)).WillByDefault(Return(n1to1Key1));
    const CallKey n1to1Key2 = 20;
    ON_CALL(*pMockIdManager, GetCallKey(nAnyConnectionId2)).WillByDefault(Return(n1to1Key2));

    EXPECT_CALL(*pMockNotifier, NotifyUsersInfo(Ref(*pMockParticipantList))).Times(1);

    const CallType eAnyType = CallType::VOIP;
    const IMS_BOOL bAnyEmergency = IMS_FALSE;
    const IMS_UINT32 nAnyReason = 0;

    // To skip Call 1 for increasing the coverage, use n1to1Key2 only.
    pController->OnCallStateChanged(
            n1to1Key2, IMtcCall::State::TERMINATING, eAnyType, bAnyEmergency, nAnyReason);

    EXPECT_EQ(objUser1.eStatus, STATUS_IDLE);
    EXPECT_EQ(objUser2.eStatus, STATUS_DISCONNECTED);
}

TEST_F(MergeControllerTest, StartFailedNotifiesFailureAndClearsQueueIfInCreatingConferenceCall)
{
    // Tests that the failure is reported when the creation of conference call is failed.
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_CREATE_CONFERENCE_CALL));

    MockIConferenceControllerListener objListener;
    pController->SetListener(&objListener);

    EXPECT_CALL(
            *pMockNotifier, NotifyMergeFailed(CallReasonInfo(CODE_USER_TERMINATED_BY_REMOTE, -1)));
    EXPECT_CALL(*pMockQueue, Clear());
    EXPECT_CALL(objListener, OnClosed(pController));

    const CallType eAnyType = CallType::VOIP;
    const IMS_BOOL bAnyEmergency = IMS_FALSE;
    const IMS_UINT32 nAnyReason = 0;
    pController->OnCallStateChanged(
            CONFERENCE_CALL_KEY, IMtcCall::State::TERMINATING, eAnyType, bAnyEmergency, nAnyReason);
}

TEST_F(MergeControllerTest,
        OnTimerExpiredCompletesReferOperationIfInTerminating1To1CallAndAlreadyConnected)
{
    // Tests that the abnormal case "no final sipfrag received but C-NOTIFY with connected state" is
    // handled.
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_TERMINATE_1TO1_CALL));

    ImsList<ConfUser*> objUsers;
    ConfUser objUser;
    objUser.eStatus = STATUS_CONNECTED;
    objUsers.Append(&objUser);
    ON_CALL(*pMockQueue, GetUsersOfCurrentOperation).WillByDefault(ReturnRef(objUsers));
    // Makes RecoverOnConferenceCallFailed() returns false
    ON_CALL(*piMockConferenceCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));

    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_REFER_INVITE, &objUser))
            .WillOnce(Return(IMS_TRUE));
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(ReturnNull());

    pController->OnTimerExpired(/* TIMER_FINAL_SIPFRAG_WAIT */ 0);
}

TEST_F(MergeControllerTest, OnTimerExpiredNotifiesFailureIfNoParticipantJoined)
{
    // Tests that the failure is reported when timer is expired but no member is joined.
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_INVITE));

    ImsList<ConfUser*> objUsers;
    ConfUser objUser;
    objUser.eStatus = STATUS_IDLE;
    objUsers.Append(&objUser);
    ON_CALL(*pMockQueue, GetUsersOfCurrentOperation).WillByDefault(ReturnRef(objUsers));
    // Makes RecoverOnConferenceCallFailed() returns false
    ON_CALL(*piMockConferenceCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize).WillByDefault(Return(0));

    // Sets up ClearIndividualCallOnMergeFailed to be test.
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(1));
    ON_CALL(*pMockParticipantList, GetConfUser(EXPLICIT_INT_0)).WillByDefault(Return(&objUser));
    ConferenceParticipantList::ConferenceParticipant* pParticipant =
            new ConferenceParticipantList::ConferenceParticipant();
    pParticipant->SetInfoUpdated(IMS_TRUE);
    ON_CALL(*pMockParticipantList, GetAt(0)).WillByDefault(Return(pParticipant));

    EXPECT_CALL(*pMockNotifier, NotifyMergeFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR, -1)));
    EXPECT_CALL(*pMockQueue, Clear());

    pController->OnTimerExpired(/* TIMER_FINAL_SIPFRAG_WAIT */ 0);

    delete pParticipant;
}

TEST_F(MergeControllerTest, OnTimerExpiredDoesNothingForIndividualCallIfReferIsNotSent)
{
    // Tests that "individual call terminated" is not reported if REFER transaction is not completed
    // when the merge operation is failed.
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_INVITE));

    ImsList<ConfUser*> objUsers;
    ConfUser objUser;
    objUser.eStatus = STATUS_IDLE;
    objUsers.Append(&objUser);
    ON_CALL(*pMockQueue, GetUsersOfCurrentOperation).WillByDefault(ReturnRef(objUsers));
    // Makes RecoverOnConferenceCallFailed() returns false
    ON_CALL(*piMockConferenceCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize).WillByDefault(Return(0));

    // Sets up ClearIndividualCallOnMergeFailed to be test.
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(2));
    ON_CALL(*pMockParticipantList, GetConfUser(EXPLICIT_INT_0)).WillByDefault(Return(&objUser));
    ON_CALL(*pMockParticipantList, GetConfUser(1)).WillByDefault(Return(&objUser));
    ConferenceParticipantList::ConferenceParticipant* pParticipant1 =
            new ConferenceParticipantList::ConferenceParticipant();
    pParticipant1->SetInfoUpdated(IMS_TRUE);
    ConferenceParticipantList::ConferenceParticipant* pParticipant2 =
            new ConferenceParticipantList::ConferenceParticipant();
    pParticipant2->SetInfoUpdated(IMS_FALSE);
    ON_CALL(*pMockParticipantList, GetAt(0)).WillByDefault(Return(pParticipant1));
    ON_CALL(*pMockParticipantList, GetAt(1)).WillByDefault(Return(pParticipant2));
    const CallKey n1to1Key = 10;
    const IMS_UINT32 nAnyConnectionId = 100;
    objUser.nConnectionId = nAnyConnectionId;
    ON_CALL(*pMockIdManager, GetCallKey(nAnyConnectionId)).WillByDefault(Return(n1to1Key));

    // Sets `REFER is not sent or not responded yet`.
    MockIConferenceReference objReference;
    pParticipant2->SetReference(&objReference);
    EXPECT_CALL(objReference, GetResponseCode).WillOnce(Return(SipStatusCode::SC_INVALID));

    EXPECT_CALL(*pMockNotifier, NotifyIndividualCallTerminated(_)).Times(0);

    pController->OnTimerExpired(/* TIMER_FINAL_SIPFRAG_WAIT */ 0);

    delete pParticipant1;
    delete pParticipant2;
}

TEST_F(MergeControllerTest, OnTimerExpiredInvokesNotifyIndividualCallTerminated)
{
    // Tests that "individual call terminated" is reported if REFER transaction gets succeeded when
    // the merge operation is failed.
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_INVITE));

    ImsList<ConfUser*> objUsers;
    ConfUser objUser;
    objUser.eStatus = STATUS_IDLE;
    objUsers.Append(&objUser);
    ON_CALL(*pMockQueue, GetUsersOfCurrentOperation).WillByDefault(ReturnRef(objUsers));
    // Makes RecoverOnConferenceCallFailed() returns false
    ON_CALL(*piMockConferenceCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize).WillByDefault(Return(0));

    // Sets up ClearIndividualCallOnMergeFailed to be test.
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(2));
    ON_CALL(*pMockParticipantList, GetConfUser(EXPLICIT_INT_0)).WillByDefault(Return(&objUser));
    ON_CALL(*pMockParticipantList, GetConfUser(1)).WillByDefault(Return(&objUser));
    ConferenceParticipantList::ConferenceParticipant* pParticipant1 =
            new ConferenceParticipantList::ConferenceParticipant();
    pParticipant1->SetInfoUpdated(IMS_TRUE);
    ConferenceParticipantList::ConferenceParticipant* pParticipant2 =
            new ConferenceParticipantList::ConferenceParticipant();
    pParticipant2->SetInfoUpdated(IMS_FALSE);
    ON_CALL(*pMockParticipantList, GetAt(0)).WillByDefault(Return(pParticipant1));
    ON_CALL(*pMockParticipantList, GetAt(1)).WillByDefault(Return(pParticipant2));
    const CallKey n1to1Key = 10;
    const IMS_UINT32 nAnyConnectionId = 100;
    objUser.nConnectionId = nAnyConnectionId;
    ON_CALL(*pMockIdManager, GetCallKey(nAnyConnectionId)).WillByDefault(Return(n1to1Key));

    MockIConferenceReference objReference;
    pParticipant2->SetReference(&objReference);
    ON_CALL(objReference, GetResponseCode).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(*pMockNotifier, NotifyIndividualCallTerminated(n1to1Key)).Times(1);

    pController->OnTimerExpired(/* TIMER_FINAL_SIPFRAG_WAIT */ 0);

    delete pParticipant1;
    delete pParticipant2;
}

TEST_F(MergeControllerTest, OnTimerExpiredInvokesNotifyIndividualCallTerminatedIfUserStateIsNotIdle)
{
    // Tests that "individual call terminated" is reported if REFER transaction is not completed but
    // the user state is connected(or progressing) by C-NOTIFY when the merge operation is failed.
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_INVITE));

    ImsList<ConfUser*> objUsers;
    ConfUser objUser;
    objUser.eStatus = STATUS_PROGRESSING;
    objUsers.Append(&objUser);
    ON_CALL(*pMockQueue, GetUsersOfCurrentOperation).WillByDefault(ReturnRef(objUsers));
    // Makes RecoverOnConferenceCallFailed() returns false
    ON_CALL(*piMockConferenceCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize).WillByDefault(Return(0));

    // Sets up ClearIndividualCallOnMergeFailed to be test.
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(1));
    ON_CALL(*pMockParticipantList, GetConfUser(EXPLICIT_INT_0)).WillByDefault(Return(&objUser));
    ConferenceParticipantList::ConferenceParticipant* pParticipant =
            new ConferenceParticipantList::ConferenceParticipant();
    pParticipant->SetInfoUpdated(IMS_FALSE);
    ON_CALL(*pMockParticipantList, GetAt(0)).WillByDefault(Return(pParticipant));
    const CallKey n1to1Key = 10;
    const IMS_UINT32 nAnyConnectionId = 100;
    objUser.nConnectionId = nAnyConnectionId;
    ON_CALL(*pMockIdManager, GetCallKey(nAnyConnectionId)).WillByDefault(Return(n1to1Key));

    EXPECT_CALL(*pMockNotifier, NotifyIndividualCallTerminated(n1to1Key)).Times(1);

    pController->OnTimerExpired(/* TIMER_FINAL_SIPFRAG_WAIT */ 0);

    delete pParticipant;
}

TEST_F(MergeControllerTest,
        OnTimerExpiredInvokesNotifyIndividualCallTerminatedIfCallStateIsTerminating)
{
    // Tests that "individual call terminated" is reported if a call state is terminating when the
    // merge operation is failed.
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_INVITE));

    ImsList<ConfUser*> objUsers;
    ConfUser objUser;
    objUser.eStatus = STATUS_IDLE;
    objUsers.Append(&objUser);
    ON_CALL(*pMockQueue, GetUsersOfCurrentOperation).WillByDefault(ReturnRef(objUsers));
    // Makes RecoverOnConferenceCallFailed() returns false
    ON_CALL(*piMockConferenceCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize).WillByDefault(Return(0));

    // Sets up ClearIndividualCallOnMergeFailed to be test.
    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(1));
    ON_CALL(*pMockParticipantList, GetConfUser(EXPLICIT_INT_0)).WillByDefault(Return(&objUser));
    ConferenceParticipantList::ConferenceParticipant* pParticipant =
            new ConferenceParticipantList::ConferenceParticipant();
    pParticipant->SetInfoUpdated(IMS_FALSE);
    ON_CALL(*pMockParticipantList, GetAt(0)).WillByDefault(Return(pParticipant));
    const CallKey n1to1Key = 10;
    const IMS_UINT32 nAnyConnectionId = 100;
    objUser.nConnectionId = nAnyConnectionId;
    ON_CALL(*pMockIdManager, GetCallKey(nAnyConnectionId)).WillByDefault(Return(n1to1Key));

    MockIMtcCall obj1to1Call;
    ON_CALL(objMockCallManager, GetCallByCallKey(n1to1Key)).WillByDefault(Return(&obj1to1Call));
    ON_CALL(obj1to1Call, GetState).WillByDefault(Return(IMtcCall::State::TERMINATING));

    EXPECT_CALL(*pMockNotifier, NotifyIndividualCallTerminated(n1to1Key)).Times(1);

    pController->OnTimerExpired(/* TIMER_FINAL_SIPFRAG_WAIT */ 0);

    delete pParticipant;
}

TEST_F(MergeControllerTest, OnTimerExpiredDoesNothingIfCurrentOperationIsReferBye)
{
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_BYE));

    EXPECT_CALL(*pMockNotifier, NotifyIndividualCallTerminated(_)).Times(0);
    EXPECT_CALL(*pMockNotifier, NotifyMergeFailed(_)).Times(0);
    EXPECT_CALL(*pMockQueue, Clear()).Times(0);
    pController->OnTimerExpired(/* TIMER_FINAL_SIPFRAG_WAIT */ 0);
}

TEST_F(MergeControllerTest, OnTimerExpiredDoesNothingIfCurrentOperationIsUninterested)
{
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_CHECK_CONNECTED));

    EXPECT_CALL(*pMockNotifier, NotifyIndividualCallTerminated(_)).Times(0);
    EXPECT_CALL(*pMockNotifier, NotifyMergeFailed(_)).Times(0);
    EXPECT_CALL(*pMockQueue, Clear()).Times(0);
    pController->OnTimerExpired(/* TIMER_FINAL_SIPFRAG_WAIT */ 0);
}

TEST_F(MergeControllerTest, OnTimerExpiredCompletesAndDoesNextOperationIfAnotherMemberIsJoined)
{
    // Tests that the failed user's state goes disconnected and gets and does the next operation if
    // the 1st invited user is joined and the 2nd invited user is failed to join.
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_INVITE));

    ImsList<ConfUser*> objUsers;
    ConfUser objUser;
    objUser.eStatus = STATUS_IDLE;
    objUsers.Append(&objUser);
    ON_CALL(*pMockQueue, GetUsersOfCurrentOperation).WillByDefault(ReturnRef(objUsers));
    // Makes RecoverOnConferenceCallFailed() returns false
    ON_CALL(*piMockConferenceCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize).WillByDefault(Return(1));

    EXPECT_CALL(*pMockQueue, CompleteCurrentOperation(CONTROL_OPERATION_REFER_INVITE, &objUser))
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(*pMockQueue, GetNextOperation).WillOnce(ReturnNull());

    pController->OnTimerExpired(/* TIMER_FINAL_SIPFRAG_WAIT */ 0);

    EXPECT_EQ(objUser.eStatus, STATUS_DISCONNECTED);
}

TEST_F(MergeControllerTest, OnTimerExpiredNotifiesFailureInAdditionalMergeCase)
{
    // Tests that the failure is reported when an additional merge operation gets failed.
    ON_CALL(*pMockQueue, GetTypeOfCurrentOperation)
            .WillByDefault(Return(CONTROL_OPERATION_REFER_INVITE));

    ImsList<ConfUser*> objUsers;
    ConfUser* pNullUser = IMS_NULL;
    objUsers.Append(pNullUser);
    objUsers.Append(pNullUser);
    objUsers.Append(pNullUser);
    ON_CALL(*pMockQueue, GetUsersOfCurrentOperation).WillByDefault(ReturnRef(objUsers));
    // Makes RecoverOnConferenceCallFailed() returns false
    ON_CALL(*piMockConferenceCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
    ON_CALL(*pMockParticipantList, GetConnectedParticipantSize).WillByDefault(Return(1));

    // Sets up ClearIndividualCallOnMergeFailed to be test.
    ConferenceParticipantList::ConferenceParticipant* pParticipant =
            new ConferenceParticipantList::ConferenceParticipant();
    pParticipant->SetInfoUpdated(IMS_FALSE);
    ON_CALL(*pMockParticipantList, GetAt(_)).WillByDefault(Return(pParticipant));

    ON_CALL(*pMockParticipantList, GetSize).WillByDefault(Return(3));
    ON_CALL(*pMockParticipantList, GetConfUser(EXPLICIT_INT_0)).WillByDefault(Return(pNullUser));
    ON_CALL(*pMockParticipantList, GetConfUser(1)).WillByDefault(Return(pNullUser));
    ON_CALL(*pMockParticipantList, GetConfUser(2)).WillByDefault(Return(pNullUser));

    EXPECT_CALL(*pMockNotifier, NotifyMergeFailed(CallReasonInfo(CODE_LOCAL_INTERNAL_ERROR, -1)));
    EXPECT_CALL(*pMockQueue, Clear());

    pController->OnTimerExpired(/* TIMER_FINAL_SIPFRAG_WAIT */ 0);
}

}  // namespace android
