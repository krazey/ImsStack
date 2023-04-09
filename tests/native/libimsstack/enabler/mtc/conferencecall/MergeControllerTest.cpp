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
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "conferencecall/CallConnectionIdManager.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceFactory.h"
#include "conferencecall/ConferenceOperationQueue.h"
#include "conferencecall/IConferenceSubscriptionListener.h"
#include "conferencecall/MergeController.h"
#include "conferencecall/MockCallConnectionIdManager.h"
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
using ::testing::AnyNumber;
using ::testing::AnyOf;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

LOCAL CallKey CONFERENCE_CALL_KEY = 100;

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

        pMockNotifier = new MockConferenceEventNotifier(objMockCallContext, *pMockIdManager);
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

TEST_F(MergeControllerTest, ProcessMergeCommand)
{
    ConfUser* pUser1 = new ConfUser();
    ConfUser* pUser2 = new ConfUser();
    pUser1->nConnectionId = 0;
    pUser2->nConnectionId = 1;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);

    ImsList<ConfUser*> objUsersCopied;
    objUsersCopied.Append(new ConfUser(*pUser1));
    objUsersCopied.Append(new ConfUser(*pUser2));

    ON_CALL(*pMockConfigurationManager, GetConferenceInvitingReferType).WillByDefault(Return(1));
    ON_CALL(*pMockConfigurationManager, GetConferenceSubscribeType).WillByDefault(Return(0));
    ON_CALL(*pMockConfigurationManager, GetConferenceSipFlowOrder).WillByDefault(Return(0));

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

TEST_F(MergeControllerTest, ProcessMergeCommandWithSubscriptionNotifyReferFlow)
{
    ConfUser* pUser1 = new ConfUser();
    ConfUser* pUser2 = new ConfUser();
    pUser1->nConnectionId = 0;
    pUser2->nConnectionId = 1;
    ImsList<ConfUser*> objUsers;
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);

    ImsList<ConfUser*> objUsersCopied;
    objUsersCopied.Append(new ConfUser(*pUser1));
    objUsersCopied.Append(new ConfUser(*pUser2));

    ON_CALL(*pMockConfigurationManager, GetConferenceInvitingReferType).WillByDefault(Return(1));
    ON_CALL(*pMockConfigurationManager, GetConferenceSubscribeType).WillByDefault(Return(0));
    ON_CALL(*pMockConfigurationManager, GetConferenceSipFlowOrder).WillByDefault(Return(2));

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

    ON_CALL(*pMockConfigurationManager, GetConferenceInvitingReferType).WillByDefault(Return(0));

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

TEST_F(MergeControllerTest, OnOperationReadyWhenNextCreateConferenceWithTwoVtCalls)
{
    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_CREATE_CONFERENCE_CALL, 0);
    ImsList<ConfUser*> objUsers;
    ConfUser* pUser1 = new ConfUser();
    ConfUser* pUser2 = new ConfUser();
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);
    pOperation->SetConfUsers(objUsers);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    MockIMtcCall objCall1;
    ON_CALL(objCall1, GetCallType).WillByDefault(Return(CallType::VT));
    MockIMtcCall objCall2;
    ON_CALL(objCall2, GetCallType).WillByDefault(Return(CallType::VT));

    EXPECT_CALL(objMockCallManager, GetCallByCallKey(_))
            .WillOnce(Return(&objCall1))
            .WillOnce(Return(&objCall2))
            .WillOnce(Return(piMockConferenceCall));

    EXPECT_CALL(*piMockConferenceCall, StartConference(CallType::VT, _, _)).Times(1);

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(MergeControllerTest, OnOperationReadyWhenNextCreateConferenceWithConfigVoip)
{
    ON_CALL(*pMockConfigurationManager, GetCallTypeAfterAudioAndVideoCallMerged)
            .WillByDefault(Return(1));  // VOIP

    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_CREATE_CONFERENCE_CALL, 0);
    ImsList<ConfUser*> objUsers;
    ConfUser* pUser1 = new ConfUser();
    ConfUser* pUser2 = new ConfUser();
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);
    pOperation->SetConfUsers(objUsers);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    MockIMtcCall objCall1;
    ON_CALL(objCall1, GetCallType).WillByDefault(Return(CallType::VT));
    MockIMtcCall objCall2;
    ON_CALL(objCall2, GetCallType).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(objMockCallManager, GetCallByCallKey(_))
            .WillOnce(Return(&objCall1))
            .WillOnce(Return(&objCall2))
            .WillOnce(Return(piMockConferenceCall));

    EXPECT_CALL(*piMockConferenceCall, StartConference(CallType::VOIP, _, _)).Times(1);

    pController->OnOperationReady();
    delete pOperation;
}

TEST_F(MergeControllerTest, OnOperationReadyWhenNextCreateConferenceWithConfigVt)
{
    ON_CALL(*pMockConfigurationManager, GetCallTypeAfterAudioAndVideoCallMerged)
            .WillByDefault(Return(2));  // VT

    ConferenceOperationQueue::ConferenceOperation* pOperation =
            new ConferenceOperationQueue::ConferenceOperation(
                    CONTROL_OPERATION_CREATE_CONFERENCE_CALL, 0);
    ImsList<ConfUser*> objUsers;
    ConfUser* pUser1 = new ConfUser();
    ConfUser* pUser2 = new ConfUser();
    objUsers.Append(pUser1);
    objUsers.Append(pUser2);
    pOperation->SetConfUsers(objUsers);
    ON_CALL(*pMockQueue, GetNextOperation).WillByDefault(Return(pOperation));

    MockIMtcCall objCall1;
    ON_CALL(objCall1, GetCallType).WillByDefault(Return(CallType::VT));
    MockIMtcCall objCall2;
    ON_CALL(objCall2, GetCallType).WillByDefault(Return(CallType::VOIP));

    EXPECT_CALL(objMockCallManager, GetCallByCallKey(_))
            .WillOnce(Return(&objCall1))
            .WillOnce(Return(&objCall2))
            .WillOnce(Return(piMockConferenceCall));

    EXPECT_CALL(*piMockConferenceCall, StartConference(CallType::VT, _, _)).Times(1);

    pController->OnOperationReady();
    delete pOperation;
}

}  // namespace android
