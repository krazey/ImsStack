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

#include "IMtcCallStateListener.h"
#include "MockIMtcContext.h"
#include "call/CallConnectionIdManager.h"
#include "call/IMtcCall.h"
#include "conferencecall/IConferenceController.h"
#include "conferencecall/MockIConferenceController.h"
#include "helper/MockICallStateProxy.h"
#include <gtest/gtest.h>

namespace android
{

using ::testing::Return;
using ::testing::ReturnRef;

LOCAL CallKey CALL_KEY_1TO1_A = 10;
LOCAL CallKey CALL_KEY_1TO1_B = 20;
LOCAL CallKey CALL_KEY_1TO1_C = 30;
LOCAL CallKey CALL_KEY_CONFERENCE = 100;

class CallConnectionIdManagerTest : public ::testing::Test
{
public:
    CallConnectionIdManager* pIdManager;

    MockIMtcContext objContext;
    MockICallStateProxy objCallStateProxy;
    MockIConferenceController objConferenceController;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        pIdManager = new CallConnectionIdManager(objContext);
    }

    virtual void TearDown() override { delete pIdManager; }
};

TEST_F(CallConnectionIdManagerTest, IdleStateChangedDoesNothing)
{
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::IDLE,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);
}

TEST_F(CallConnectionIdManagerTest, TotalCallStateOtherThanIdleChangedDoesNothing)
{
    pIdManager->OnTotalCallStateChanged(IMtcCallStateListener::State::IDLE);
}

TEST_F(CallConnectionIdManagerTest, OutgoingStateChangedAddsId)
{
    EXPECT_EQ(-1, pIdManager->GetIndex(CALL_KEY_1TO1_A));
    EXPECT_EQ(0, pIdManager->GetCallKey(1));

    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(1, pIdManager->GetIndex(CALL_KEY_1TO1_A));
    EXPECT_EQ(CALL_KEY_1TO1_A, pIdManager->GetCallKey(1));
}

TEST_F(CallConnectionIdManagerTest, IncomingStateChangedAddsId)
{
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::INCOMING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(CALL_KEY_1TO1_A, pIdManager->GetCallKey(1));
}

TEST_F(CallConnectionIdManagerTest, AletingStateChangedAddsId)
{
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::ALERTING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(CALL_KEY_1TO1_A, pIdManager->GetCallKey(1));
}

TEST_F(CallConnectionIdManagerTest, AddAndAddAndRemoveAndAdd)
{
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_B, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(CALL_KEY_1TO1_A, pIdManager->GetCallKey(1));
    EXPECT_EQ(CALL_KEY_1TO1_B, pIdManager->GetCallKey(2));

    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::TERMINATING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(0, pIdManager->GetCallKey(1));
    EXPECT_EQ(CALL_KEY_1TO1_B, pIdManager->GetCallKey(2));

    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_C, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(CALL_KEY_1TO1_C, pIdManager->GetCallKey(1));
}

TEST_F(CallConnectionIdManagerTest, 3CallsAndIdleTotalCallState)
{
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_B, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_C, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    pIdManager->OnTotalCallStateChanged(IMtcCallStateListener::State::IDLE);

    EXPECT_EQ(0, pIdManager->GetCallKey(1));
    EXPECT_EQ(0, pIdManager->GetCallKey(2));
    EXPECT_EQ(0, pIdManager->GetCallKey(3));
}

TEST_F(CallConnectionIdManagerTest, ConferenceCallDoesNotAddId)
{
    ON_CALL(objConferenceController, GetCallStatusInConference(CALL_KEY_CONFERENCE))
            .WillByDefault(Return(IndividualCallState::HOST));
    pIdManager->OnConferenceCallStarted(&objConferenceController, IMS_TRUE);
    pIdManager->OnCallStateChanged(CALL_KEY_CONFERENCE, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(0, pIdManager->GetCallKey(1));

    pIdManager->OnConferenceCallStarted(&objConferenceController, IMS_FALSE);
}

TEST_F(CallConnectionIdManagerTest, ConferenceParticipantTerminatedDoesNotRemoveId)
{
    ON_CALL(objConferenceController, GetCallStatusInConference(CALL_KEY_CONFERENCE))
            .WillByDefault(Return(IndividualCallState::HOST));
    ON_CALL(objConferenceController, GetCallStatusInConference(CALL_KEY_1TO1_A))
            .WillByDefault(Return(IndividualCallState::JOINED));
    pIdManager->OnConferenceCallStarted(&objConferenceController, IMS_TRUE);
    pIdManager->OnCallStateChanged(CALL_KEY_CONFERENCE, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::TERMINATING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(CALL_KEY_1TO1_A, pIdManager->GetCallKey(1));
}

TEST_F(CallConnectionIdManagerTest, ConferenceCallTerminatedRemovesParticipantId)
{
    ON_CALL(objConferenceController, GetCallStatusInConference(CALL_KEY_CONFERENCE))
            .WillByDefault(Return(IndividualCallState::HOST));
    ON_CALL(objConferenceController, GetCallStatusInConference(CALL_KEY_1TO1_A))
            .WillByDefault(Return(IndividualCallState::JOINED));
    pIdManager->OnConferenceCallStarted(&objConferenceController, IMS_TRUE);
    pIdManager->OnCallStateChanged(CALL_KEY_CONFERENCE, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::TERMINATING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    pIdManager->OnCallStateChanged(CALL_KEY_CONFERENCE, IMtcCallStateListener::State::TERMINATING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(0, pIdManager->GetCallKey(1));
}

TEST_F(CallConnectionIdManagerTest, OnConferenceParticipantDisconnectedRemovesParticipantId)
{
    ON_CALL(objConferenceController, GetCallStatusInConference(CALL_KEY_CONFERENCE))
            .WillByDefault(Return(IndividualCallState::HOST));
    ON_CALL(objConferenceController, GetCallStatusInConference(CALL_KEY_1TO1_A))
            .WillByDefault(Return(IndividualCallState::JOINED));
    pIdManager->OnConferenceCallStarted(&objConferenceController, IMS_TRUE);
    pIdManager->OnCallStateChanged(CALL_KEY_CONFERENCE, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    pIdManager->OnConferenceParticipantDisconnected(1);

    EXPECT_EQ(0, pIdManager->GetCallKey(1));
}

TEST_F(CallConnectionIdManagerTest, DoubleOutgoingStateChangedDoesNotAddId)
{
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(CALL_KEY_1TO1_A, pIdManager->GetCallKey(1));

    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(0, pIdManager->GetCallKey(2));
}

TEST_F(CallConnectionIdManagerTest, DoubleTerminatingStateChangedDoesNotRemoveId)
{
    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(CALL_KEY_1TO1_A, pIdManager->GetCallKey(1));

    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::TERMINATING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(0, pIdManager->GetCallKey(1));

    pIdManager->OnCallStateChanged(CALL_KEY_1TO1_A, IMtcCallStateListener::State::TERMINATING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(0, pIdManager->GetCallKey(1));  // Nothing to check
}

}  // namespace android
