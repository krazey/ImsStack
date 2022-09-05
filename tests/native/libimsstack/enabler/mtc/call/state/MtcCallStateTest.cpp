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

#include <gtest/gtest.h>
#include "ImsList.h"
#include "ImsMap.h"
#include "MtcDef.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/IMtcCall.h"
#include "call/state/MtcCallState.h"
#include "call/state/IMtcCallState.h"
#include "call/MockIMtcCallContext.h"
#include "conferencecall/ConferenceDef.h"
#include "core/IReference.h"
#include "core/MockISession.h"
#include "precondition/QosDef.h"
#include "sipcore/ISipClientConnection.h"
#include "sipcore/ISipConnection.h"
#include "sipcore/ISipServerConnection.h"
#include "sipcore/MockISipClientConnection.h"

LOCAL CallStateName INITIAL_CALL_STATE = CallStateName::IDLE;
LOCAL CallType ANY_CALL_TYPE = CallType::VOIP;

using ::testing::Return;

class MtcCallStateTest : public ::testing::Test
{
public:
    MtcCallState* pState;
    MockIMtcCallContext objContext;
    MockISession objISession;
    MediaInfo objMediaInfo;
    CallReasonInfo* pReason;

protected:
    virtual void SetUp() override
    {
        pReason = new CallReasonInfo(CODE_UNSPECIFIED);
        pState = new MtcCallState(INITIAL_CALL_STATE, objContext);
    }

    virtual void TearDown() override
    {
        delete pReason;
        delete pState;
    }
};

TEST_F(MtcCallStateTest, OnEnterDoesNothing)
{
    pState->OnEnter();
    EXPECT_EQ(INITIAL_CALL_STATE, pState->GetStateName());
}

TEST_F(MtcCallStateTest, OnExitDoesNothing)
{
    pState->OnExit();
    EXPECT_EQ(INITIAL_CALL_STATE, pState->GetStateName());
}

TEST_F(MtcCallStateTest, StartDoesNothing)
{
    ImsMap<SuppType, SuppService*> objSuppServices;
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->Start(ANY_CALL_TYPE, "anyTarget", &objMediaInfo, objSuppServices));
}

TEST_F(MtcCallStateTest, StartConference1DoesNothing)
{
    ImsMap<SuppType, SuppService*> objSuppServices;
    ImsList<ConfUser*> lstUsers;
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->StartConference(
                    ANY_CALL_TYPE, "anyTarget", &objMediaInfo, objSuppServices, lstUsers));
}

TEST_F(MtcCallStateTest, StartConference2DoesNothing)
{
    ImsList<ConfUser*> lstUsers;
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->StartConference(ANY_CALL_TYPE, "anyTarget", lstUsers));
}

TEST_F(MtcCallStateTest, HandleIncomingDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->HandleIncoming(&objISession));
}

TEST_F(MtcCallStateTest, HandleUserAlertDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->HandleUserAlert());
}

TEST_F(MtcCallStateTest, AcceptDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->Accept(ANY_CALL_TYPE, &objMediaInfo));
}

TEST_F(MtcCallStateTest, RejectDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->Reject(*pReason));
}

TEST_F(MtcCallStateTest, HoldDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->Hold(&objMediaInfo));
}

TEST_F(MtcCallStateTest, ResumeDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->Resume(&objMediaInfo));
}

TEST_F(MtcCallStateTest, AcceptResumeDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->AcceptResume(ANY_CALL_TYPE, &objMediaInfo));
}

TEST_F(MtcCallStateTest, RejectResumeDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->RejectResume(*pReason));
}

TEST_F(MtcCallStateTest, UpdateDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->Update(ANY_CALL_TYPE, &objMediaInfo));
}

TEST_F(MtcCallStateTest, AcceptUpdateDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->AcceptUpdate(ANY_CALL_TYPE, &objMediaInfo));
}

TEST_F(MtcCallStateTest, RejectUpdateDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->RejectUpdate(*pReason));
}

TEST_F(MtcCallStateTest, CancelUpdateDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->CancelUpdate(*pReason));
}

TEST_F(MtcCallStateTest, TerminateDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->Terminate(*pReason));
}

TEST_F(MtcCallStateTest, SendDtmfDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SendDtmf("anySignal", 1));
}

TEST_F(MtcCallStateTest, HandleIpcanChangedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->HandleIpcanChanged());
}

TEST_F(MtcCallStateTest, HandleIncomingUssiDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->HandleIncomingUssi(&objISession));
}

TEST_F(MtcCallStateTest, OnUssiAttachedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->OnUssiAttached());
}

TEST_F(MtcCallStateTest, AcceptUssiDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->AcceptUssi(ANY_CALL_TYPE, &objMediaInfo));
}

TEST_F(MtcCallStateTest, UssiStartedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->UssiStarted(&objISession));
}

TEST_F(MtcCallStateTest, TerminateUssiDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->TerminateUssi(*pReason));
}

TEST_F(MtcCallStateTest, UssiTerminatedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->UssiTerminated(&objISession));
}

TEST_F(MtcCallStateTest, SendUssdDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SendUssd("anyUssdData"));
}

TEST_F(MtcCallStateTest, UssiInfoReceivedDoesNothing)
{
    ISipServerConnection* piFakeConnection = reinterpret_cast<ISipServerConnection*>(0x1);
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->UssiInfoReceived(&objISession, piFakeConnection));
}

TEST_F(MtcCallStateTest, NotifyResponseToUssiInfoDoesNothing)
{
    ISipClientConnection* piFakeConnection = reinterpret_cast<ISipClientConnection*>(0x1);
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->NotifyResponseToUssiInfo(piFakeConnection, piFakeConnection));
}

TEST_F(MtcCallStateTest, NotifyErrorToUssiInfoDoesNothing)
{
    ISipConnection* piFakeConnection = reinterpret_cast<ISipConnection*>(0x1);
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->NotifyErrorToUssiInfo(piFakeConnection, 0, "anyMessage"));
}

TEST_F(MtcCallStateTest, SessionAlertingDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionAlerting(&objISession));
}

TEST_F(MtcCallStateTest, SessionReferenceReceivedDoesNothing)
{
    IReference* piFakeReference = reinterpret_cast<IReference*>(0x1);
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionReferenceReceived(&objISession, piFakeReference));
}

TEST_F(MtcCallStateTest, SessionStartedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionStarted(&objISession));
}

TEST_F(MtcCallStateTest, SessionStartFailedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionStartFailed(&objISession));
}

TEST_F(MtcCallStateTest, SessionTerminatedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionTerminated(&objISession));
}

TEST_F(MtcCallStateTest, SessionUpdatedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionUpdated(&objISession));
}

TEST_F(MtcCallStateTest, SessionUpdateFailedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionUpdateFailed(&objISession));
}

TEST_F(MtcCallStateTest, SessionUpdateReceivedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionUpdateReceived(&objISession));
}

TEST_F(MtcCallStateTest, SessionCancelDeliveredDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionCancelDelivered(&objISession));
}

TEST_F(MtcCallStateTest, SessionCancelDeliveryFailedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionCancelDeliveryFailed(&objISession));
}

TEST_F(MtcCallStateTest, SessionEarlyMediaUpdatedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionEarlyMediaUpdated(&objISession));
}

TEST_F(MtcCallStateTest, SessionEarlyMediaUpdateFailedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionEarlyMediaUpdateFailed(&objISession));
}

TEST_F(MtcCallStateTest, SessionEarlyMediaUpdateReceivedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionEarlyMediaUpdateReceived(&objISession));
}

TEST_F(MtcCallStateTest, SessionForkedResponseReceivedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionForkedResponseReceived(&objISession, &objISession));
}

TEST_F(MtcCallStateTest, SessionPRAckDeliveredDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionPRAckDelivered(&objISession));
}

TEST_F(MtcCallStateTest, SessionPRAckDeliveryFailedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionPRAckDeliveryFailed(&objISession));
}

TEST_F(MtcCallStateTest, SessionPRAckReceivedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionPRAckReceived(&objISession));
}

TEST_F(MtcCallStateTest, SessionProvisionalResponseReceivedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionProvisionalResponseReceived(&objISession, 0));
}

TEST_F(MtcCallStateTest, SessionRPRDeliveryFailedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionRPRDeliveryFailed(&objISession));
}

TEST_F(MtcCallStateTest, SessionRPRReceivedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionRPRReceived(&objISession, 0));
}

TEST_F(MtcCallStateTest, SessionTransactionReceivedDoesNothing)
{
    ISipServerConnection* piFakeConnection = reinterpret_cast<ISipServerConnection*>(0x1);
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->SessionTransactionReceived(&objISession, piFakeConnection));
}

TEST_F(MtcCallStateTest, Refresh_NotifyCompletedDoesNothing)
{
    ISipClientConnection* piFakeConnection = reinterpret_cast<ISipClientConnection*>(0x1);
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->Refresh_NotifyCompleted(piFakeConnection));
}

TEST_F(MtcCallStateTest, Refresh_NotifyTerminatedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->Refresh_NotifyTerminated());
}

TEST_F(MtcCallStateTest, Refresh_NotifyTimerExpiredDoesNothing)
{
    IMS_BOOL bAnyValue = IMS_FALSE;
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->Refresh_NotifyTimerExpired(bAnyValue));
}

TEST_F(MtcCallStateTest, OnTimerExpiredDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->OnTimerExpired(0));
}

TEST_F(MtcCallStateTest, OnBlockCheckedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->OnBlockChecked(IMtcBlockChecker::Result::Status::BLOCKED));
}

TEST_F(MtcCallStateTest, QosReservedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->QosReserved(&objISession, 0));
}

TEST_F(MtcCallStateTest, QosReserveFailedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->QosReserveFailed(&objISession, QosLossPolicy::MAINTAIN));
}

TEST_F(MtcCallStateTest, OnInternalFailureDoesNothingButReturnsTerminating)
{
    EXPECT_EQ(CallStateName::TERMINATING,
            pState->OnInternalFailure());
}

TEST_F(MtcCallStateTest, OnAttachedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->OnAttached());
}

TEST_F(MtcCallStateTest, ClientConnectionNotifyResponseInvokesReceive)
{
    ISipClientConnection* piFakeConnection = reinterpret_cast<ISipClientConnection*>(0x1);
    MockISipClientConnection objClientConnection;
    EXPECT_CALL(objClientConnection, Receive)
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));

    EXPECT_CALL(objClientConnection, GetStatusCode)
            .Times(1)
            .WillOnce(Return(SipStatusCode::SC_200));

    EXPECT_CALL(objClientConnection, Close)
            .Times(1);

    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->ClientConnection_NotifyResponse(&objClientConnection, piFakeConnection));
}

TEST_F(MtcCallStateTest, ClientConnectionNotifyResponseInvokesReceiveAndFails)
{
    ISipClientConnection* piFakeConnection = reinterpret_cast<ISipClientConnection*>(0x1);
    MockISipClientConnection objClientConnection;
    EXPECT_CALL(objClientConnection, Receive)
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));

    EXPECT_CALL(objClientConnection, GetStatusCode)
            .Times(0);

    EXPECT_CALL(objClientConnection, Close)
            .Times(0);

    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->ClientConnection_NotifyResponse(&objClientConnection, piFakeConnection));
}

TEST_F(MtcCallStateTest, ErrorNotifyErrorInvokesClose)
{
    MockISipClientConnection objClientConnection;
    EXPECT_CALL(objClientConnection, Close)
            .Times(1);

    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->Error_NotifyError(&objClientConnection, SipStatusCode::SC_420, "anyMessage"));
}

TEST_F(MtcCallStateTest, OnReceivingMediaDataFailedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->OnReceivingMediaDataFailed(0, 0));
}

TEST_F(MtcCallStateTest, OnVideoLowestBitRateDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->OnVideoLowestBitRate());
}

TEST_F(MtcCallStateTest, OnReceivingNetworkToneStartedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->OnReceivingNetworkToneStarted());
}

TEST_F(MtcCallStateTest, OnReceivingNetworkToneFailedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->OnReceivingNetworkToneFailed());
}

TEST_F(MtcCallStateTest, OnMediaFailedDoesNothing)
{
    EXPECT_EQ(INITIAL_CALL_STATE,
            pState->OnMediaFailed(*pReason));
}
