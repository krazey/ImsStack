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

#include <memory>
#include <gtest/gtest.h>
#include "CallReasonInfo.h"
#include "JniMtcCallThread.h"
#include "JniMtcServiceThread.h"
#include "MockIMtcContext.h"
#include "MockIMtcImsEventReceiver.h"
#include "MockIMtcService.h"
#include "MtcDef.h"
#include "../../interface/mtc/MockIMtcCallController.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallManager.h"
#include "call/MtcCall.h"
#include "call/NullCall.h"
#include "call/state/MockIMtcCallState.h"
#include "call/state/MockMtcCallStateMachine.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/MockIConferenceManager.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "core/MockIReference.h"
#include "core/MockISession.h"
#include "dialingplan/MockIMtcDialingPlan.h"
#include "helper/MockICallStateProxy.h"
#include "helper/OperationAsyncRunner.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSessionInterfaceHolder.h"
#include "media/JniMediaSessionThread.h"
#include "sipcore/ISipHeader.h"
#include "sipcore/MockISipClientConnection.h"
#include "sipcore/MockISipConnection.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/MockISipServerConnection.h"
#include "sipcore/SipStatusCode.h"
#include "ussi/UssiConstants.h"
#include "vonr/MockIMtcVonrManager.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Unused;

class MtcCallTest : public ::testing::Test
{
public:
    MockIMtcService objService;
    MockIMtcContext objContext;
    CallInfo objCallInfo;
    MockICallStateProxy objCallStateProxy;
    MockIInterfaceHolderListener objInterfaceHolderListener;
    MockIMtcSipInterfaceFactory objSipInterfaceFactory;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockSessionInterfaceHolder* pSessionInterfaceHolder;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallStateProxy)
                .WillByDefault(ReturnRef(objCallStateProxy));

        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objCallStateProxy, UpdateCallState(_, _, _, _, _))
                .WillByDefault(Return());

        pSessionInterfaceHolder = new MockSessionInterfaceHolder(objInterfaceHolderListener);
        ON_CALL(objSipInterfaceFactory, GetISessionHolder)
                .WillByDefault(Return(pSessionInterfaceHolder));
        ON_CALL(objContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(objSipInterfaceFactory));
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pSessionInterfaceHolder;
    }

    std::unique_ptr<MockIMtcCallStateFactory> CreateStateFactory(IN IMtcCallState* pState)
    {
        std::unique_ptr<MockIMtcCallStateFactory> pStateFactory =
                std::make_unique<MockIMtcCallStateFactory>();
        ON_CALL(*pStateFactory, CreateState)
                .WillByDefault(Invoke([pState](Unused, Unused) { return pState; }));
        return pStateFactory;
    }

    std::unique_ptr<MockIMtcCallStateFactory> CreateStateFactory()
    {
        return CreateStateFactory(new MockIMtcCallState());
    }
};

TEST_F(MtcCallTest, CallKeyIsValidInitially)
{
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory()));
    NullCall objNullCall;

    EXPECT_NE(objNullCall.GetKey(), objCall.GetKey());
}

TEST_F(MtcCallTest, CallKeysAreNotOverlapped)
{
    MtcCall objCall1(objContext, objService, objCallInfo, std::move(CreateStateFactory()));
    MtcCall objCall2(objContext, objService, objCallInfo, std::move(CreateStateFactory()));

    EXPECT_NE(objCall1.GetKey(), objCall2.GetKey());
}

TEST_F(MtcCallTest, HandleIncomingCallsState)
{
    MockISession objSession;
    JniMtcServiceThread* pServiceThread = reinterpret_cast<JniMtcServiceThread*>(0x1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIncoming(&objSession, pServiceThread))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.HandleIncoming(&objSession, pServiceThread);
}

TEST_F(MtcCallTest, HandleIncomingCallsStateForUssi)
{
    MockISipMessage objUssiSipMessage;
    AString strRecvInfo(UssiConstants::HEADER_RECVINFO);
    ImsList<AString> lstRecvInfoHeaders;
    lstRecvInfoHeaders.Append(UssiConstants::HEADER_USSD_PACKAGE);
    ON_CALL(objUssiSipMessage, GetHeaders(ISipHeader::UNKNOWN, strRecvInfo))
            .WillByDefault(Return(lstRecvInfoHeaders));
    ImsList<AString> lstAcceptHeaders;
    lstAcceptHeaders.Append(UssiConstants::HEADER_APPLICATION_USSDXML);
    ON_CALL(objUssiSipMessage, GetHeaders(ISipHeader::ACCEPT, _))
            .WillByDefault(Return(lstAcceptHeaders));

    MockIMessage objUssiMessage;
    ON_CALL(objUssiMessage, GetMessage)
            .WillByDefault(Return(&objUssiSipMessage));

    MockISession objSession;
    ON_CALL(objSession, GetPreviousRequest(_))
            .WillByDefault(Return(&objUssiMessage));

    JniMtcServiceThread* pServiceThread = reinterpret_cast<JniMtcServiceThread*>(0x1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIncomingUssi(&objSession, pServiceThread))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.HandleIncoming(&objSession, pServiceThread);
}

TEST_F(MtcCallTest, HandleIncomingFailsIfSessionIsNull)
{
    JniMtcServiceThread* pServiceThread = reinterpret_cast<JniMtcServiceThread*>(0x1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIncoming(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.HandleIncoming(IMS_NULL, pServiceThread);
}

TEST_F(MtcCallTest, HandleIncomingFailsIfServiceThreadIsNull)
{
    MockISession objSession;
    EXPECT_CALL(objSession, Reject(SipStatusCode::SC_488))
            .Times(1);

    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(&objSession))
            .Times(1);
    EXPECT_CALL(*pSessionInterfaceHolder, ReleaseISession(&objSession, _))
            .Times(1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIncoming(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.HandleIncoming(&objSession, IMS_NULL);
}

TEST_F(MtcCallTest, AttachSetUiNotifierThreads)
{
    MockIMtcCallState* pState = new MockIMtcCallState();

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));
    JniMtcCallThread* pJniMtcCallThread = reinterpret_cast<JniMtcCallThread*>(0x1);
    JniMediaSessionThread* pJniMediaSessionThread = reinterpret_cast<JniMediaSessionThread*>(0x2);

    objCall.Attach(pJniMtcCallThread, pJniMediaSessionThread);

    EXPECT_EQ(pJniMtcCallThread, objCall.GetUiNotifier().GetJniCallThread());
    EXPECT_EQ(pJniMediaSessionThread, objCall.GetUiNotifier().GetJniMediaThread());
}

TEST_F(MtcCallTest, AttachNotCallsStateForMo)
{
    objCallInfo.ePeerType = PeerType::MO;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnAttached)
            .Times(0);
    EXPECT_CALL(*pState, OnUssiAttached)
            .Times(0);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));
    JniMtcCallThread* pJniMtcCallThread = reinterpret_cast<JniMtcCallThread*>(0x1);
    JniMediaSessionThread* pJniMediaSessionThread = reinterpret_cast<JniMediaSessionThread*>(0x2);

    objCall.Attach(pJniMtcCallThread, pJniMediaSessionThread);
}

TEST_F(MtcCallTest, AttachCallsStateForMt)
{
    objCallInfo.ePeerType = PeerType::MT;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnAttached)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));
    JniMtcCallThread* pJniMtcCallThread = reinterpret_cast<JniMtcCallThread*>(0x1);
    JniMediaSessionThread* pJniMediaSessionThread = reinterpret_cast<JniMediaSessionThread*>(0x2);

    objCall.Attach(pJniMtcCallThread, pJniMediaSessionThread);
}

TEST_F(MtcCallTest, AttachCallsStateForMtUssi)
{
    objCallInfo.ePeerType = PeerType::MT;
    objCallInfo.bUssi = IMS_TRUE;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnUssiAttached)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));
    JniMtcCallThread* pJniMtcCallThread = reinterpret_cast<JniMtcCallThread*>(0x1);
    JniMediaSessionThread* pJniMediaSessionThread = reinterpret_cast<JniMediaSessionThread*>(0x2);

    objCall.Attach(pJniMtcCallThread, pJniMediaSessionThread);
}

TEST_F(MtcCallTest, AttachFailsIfJniMtcCallThreadIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));
    JniMediaSessionThread* pJniMediaSessionThread = reinterpret_cast<JniMediaSessionThread*>(0x2);

    objCall.Attach(IMS_NULL, pJniMediaSessionThread);
}

TEST_F(MtcCallTest, DetachUnsetUiNotifierCallThread)
{
    MockIMtcCallState* pState = new MockIMtcCallState();

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));
    JniMtcCallThread* pJniMtcCallThread = reinterpret_cast<JniMtcCallThread*>(0x1);
    JniMediaSessionThread* pJniMediaSessionThread = reinterpret_cast<JniMediaSessionThread*>(0x2);
    objCall.Attach(pJniMtcCallThread, pJniMediaSessionThread);

    objCall.Detach();

    EXPECT_EQ(IMS_NULL, objCall.GetUiNotifier().GetJniCallThread());
    EXPECT_EQ(pJniMediaSessionThread, objCall.GetUiNotifier().GetJniMediaThread());
}

TEST_F(MtcCallTest, StartCallsState)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo* pMediaInfo = new MediaInfo();
    ImsMap<SuppType, SuppService*> objSuppServices;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Start(eCallType, strTarget, pMediaInfo, Ref(objSuppServices)))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Start(eCallType, strTarget, pMediaInfo, objSuppServices);
}

TEST_F(MtcCallTest, StartCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo* pMediaInfo = new MediaInfo();
    ImsMap<SuppType, SuppService*> objSuppServices;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Start(eCallType, strTarget, pMediaInfo, Ref(objSuppServices)))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Start(eCallType, strTarget, pMediaInfo, objSuppServices);
}

TEST_F(MtcCallTest, StartFailsIfMediaInfoIsNull)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo* pMediaInfo = IMS_NULL;
    ImsMap<SuppType, SuppService*> objSuppServices;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Start(_, _, _, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Start(eCallType, strTarget, pMediaInfo, objSuppServices);
}

TEST_F(MtcCallTest, StartConference5CallsState)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo* pMediaInfo = new MediaInfo();
    ImsMap<SuppType, SuppService*> objSuppServices;
    ImsList<ConfUser*> lstUsers;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState,
            StartConference(eCallType, strTarget, pMediaInfo, Ref(objSuppServices), Ref(lstUsers)))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.StartConference(eCallType, strTarget, pMediaInfo, objSuppServices, lstUsers);
}

TEST_F(MtcCallTest, StartConference5FailsIfMediaInfoIsNull)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo* pMediaInfo = IMS_NULL;
    ImsMap<SuppType, SuppService*> objSuppServices;
    ImsList<ConfUser*> lstUsers;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, StartConference(_, _, _, _, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.StartConference(eCallType, strTarget, pMediaInfo, objSuppServices, lstUsers);
}

TEST_F(MtcCallTest, StartConference3CallsState)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, StartConference(eCallType, strTarget, Ref(lstUsers)))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.StartConference(eCallType, strTarget, lstUsers);
}

TEST_F(MtcCallTest, HandleUserAlertCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleUserAlert)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.HandleUserAlert();
}

TEST_F(MtcCallTest, HandleUserAlertDoNothingForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleUserAlert)
            .Times(0);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.HandleUserAlert();
}

TEST_F(MtcCallTest, AcceptCallsState)
{
    CallType eCallType = CallType::VOIP;
    MediaInfo* pMediaInfo = new MediaInfo();

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Accept(eCallType, pMediaInfo))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Accept(eCallType, pMediaInfo);
}

TEST_F(MtcCallTest, AcceptCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    CallType eCallType = CallType::VOIP;
    MediaInfo* pMediaInfo = new MediaInfo();

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, AcceptUssi(eCallType, pMediaInfo))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Accept(eCallType, pMediaInfo);
}

TEST_F(MtcCallTest, RejectCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Reject(objReason))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Reject(objReason);
}

TEST_F(MtcCallTest, HoldCallsState)
{
    MediaInfo* pMediaInfo = new MediaInfo();

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Hold(pMediaInfo))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Hold(pMediaInfo);
}

TEST_F(MtcCallTest, HoldFailsIfMediaInfoIsNull)
{
    MediaInfo* pMediaInfo = IMS_NULL;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Hold(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Hold(pMediaInfo);
}

TEST_F(MtcCallTest, HoldIsPendedInUpdatingState)
{
    MediaInfo* pMediaInfo = new MediaInfo();

    MockIMtcCallState* pState = new MockIMtcCallState();
    ON_CALL(*pState, GetStateName).WillByDefault(Return(CallStateName::UPDATING));
    EXPECT_CALL(*pState, Hold(_))
            .Times(0);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Hold(pMediaInfo);
}

TEST_F(MtcCallTest, HoldIsNotPendedInEstablishedState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    ON_CALL(*pState, GetStateName)
            .WillByDefault(Return(CallStateName::ESTABLISHED));

    EXPECT_CALL(*pState, Hold(_))
            .Times(1)
            .WillRepeatedly(Return(CallStateName::ESTABLISHED));

    MediaInfo objMediaInfo;
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));
    objCall.Hold(&objMediaInfo);
}

TEST_F(MtcCallTest, HoldIsPendedIfSessionIsRefreshing)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    ON_CALL(*pState, GetStateName)
            .WillByDefault(Return(CallStateName::ESTABLISHED));

    MockISession objSession;
    ON_CALL(objSession, IsSessionRefreshInProgress).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(*pState, Hold(_))
            .Times(0);

    MediaInfo objMediaInfo;
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));
    objCall.CreateSession(&objSession);
    objCall.Hold(&objMediaInfo);
}

TEST_F(MtcCallTest, RunPendingOperationByRefreshCompleted)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    ON_CALL(*pState, GetStateName)
            .WillByDefault(Return(CallStateName::ESTABLISHED));
    ON_CALL(*pState, Refresh_NotifyCompleted(_))
            .WillByDefault(Return(CallStateName::ESTABLISHED));

    MockISession objSession;
    EXPECT_CALL(objSession, IsSessionRefreshInProgress)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(*pState, Hold(_))
            .Times(1)
            .WillRepeatedly(Return(CallStateName::ESTABLISHED));

    MediaInfo objMediaInfo;
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    std::function<void()> objOperation = [&]()
            {
                return objCall.RunPendingOperation();
            };
    OperationAsyncRunner* pRunner = new OperationAsyncRunner(objOperation);
    ON_CALL(objContext, GetAsyncRunner(_))
            .WillByDefault(Return(pRunner));

    objCall.CreateSession(&objSession);
    objCall.Hold(&objMediaInfo);
    objCall.Refresh_NotifyCompleted(reinterpret_cast<ISipClientConnection*>(0x1));
    ImsMessage objMessage(0, 0, 0);
    pRunner->OnMessage(objMessage);
}

TEST_F(MtcCallTest, ResumeCallsState)
{
    MediaInfo* pMediaInfo = new MediaInfo();

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Resume(pMediaInfo))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Resume(pMediaInfo);
}

TEST_F(MtcCallTest, ResumeFailsIfMediaInfoIsNull)
{
    MediaInfo* pMediaInfo = IMS_NULL;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Resume(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Resume(pMediaInfo);
}

TEST_F(MtcCallTest, AcceptResumeCallsState)
{
    CallType eCallType = CallType::VOIP;
    MediaInfo* pMediaInfo = new MediaInfo();

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, AcceptResume(eCallType, pMediaInfo))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.AcceptResume(eCallType, pMediaInfo);
}

TEST_F(MtcCallTest, AcceptResumeFailsIfMediaInfoIsNull)
{
    CallType eCallType = CallType::VOIP;
    MediaInfo* pMediaInfo = IMS_NULL;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, AcceptResume(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.AcceptResume(eCallType, pMediaInfo);
}

TEST_F(MtcCallTest, RejectResumeCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, RejectResume(objReason))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.RejectResume(objReason);
}

TEST_F(MtcCallTest, UpdateCallsState)
{
    CallType eCallType = CallType::VOIP;
    MediaInfo* pMediaInfo = new MediaInfo();

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Update(eCallType, pMediaInfo))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Update(eCallType, pMediaInfo);
}

TEST_F(MtcCallTest, UpdateFailsIfMediaInfoIsNull)
{
    CallType eCallType = CallType::VOIP;
    MediaInfo* pMediaInfo = IMS_NULL;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Update(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Update(eCallType, pMediaInfo);
}

TEST_F(MtcCallTest, AcceptUpdateCallsState)
{
    CallType eCallType = CallType::VOIP;
    MediaInfo* pMediaInfo = new MediaInfo();

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, AcceptUpdate(eCallType, pMediaInfo))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.AcceptUpdate(eCallType, pMediaInfo);
}

TEST_F(MtcCallTest, AcceptUpdateFailsIfMediaInfoIsNull)
{
    CallType eCallType = CallType::VOIP;
    MediaInfo* pMediaInfo = IMS_NULL;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, AcceptUpdate(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.AcceptUpdate(eCallType, pMediaInfo);
}

TEST_F(MtcCallTest, RejectUpdateCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, RejectUpdate(objReason))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.RejectUpdate(objReason);
}

TEST_F(MtcCallTest, CancelUpdateCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, CancelUpdate(objReason))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.CancelUpdate(objReason);
}

TEST_F(MtcCallTest, TerminateCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Terminate(objReason))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Terminate(objReason);
}

TEST_F(MtcCallTest, TerminateCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, TerminateUssi(objReason))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Terminate(objReason);
}

TEST_F(MtcCallTest, SendDtmfCallsState)
{
    AString strSignal("some_signal");
    IMS_SINT32 nDuration = 1;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SendDtmf(strSignal, nDuration))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SendDtmf(strSignal, nDuration);
}

TEST_F(MtcCallTest, SendUssdCallsState)
{
    AString strUssd("some_ussd");

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SendUssd(strUssd))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SendUssd(strUssd);
}

TEST_F(MtcCallTest, HandleSrvccSuccessCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleSrvccSuccess)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.HandleSrvccSuccess();
}

TEST_F(MtcCallTest, HandleSrvccFailureCallsState)
{
    UpdateType eUpdateType = UpdateType::NORMAL;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleSrvccFailure(eUpdateType))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.HandleSrvccFailure(eUpdateType);
}

TEST_F(MtcCallTest, HandleIpcanChangedCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIpcanChanged)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.HandleIpcanChanged();
}

// TODO: IMtcCallContext methods

TEST_F(MtcCallTest, CreateSessionDoNothingIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    EXPECT_EQ(nullptr, objCall.CreateSession(IMS_NULL));
}

TEST_F(MtcCallTest, CreateSessionDoNothingIfMtcSessionExists)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.CreateSession(&objSession);

    EXPECT_EQ(nullptr, objCall.CreateSession(&objSession));
}

TEST_F(MtcCallTest, CreateSessionSetListenersOfSession)
{
    MockISession objSession;
    EXPECT_CALL(objSession, SetListener(_))
            .Times(1);
    // 2 times: when init and in MtcSession::~MtcSession
    EXPECT_CALL(objSession, SetMessageMediator(_))
            .Times(2);
    EXPECT_CALL(objSession, SetRefreshListener(_))
            .Times(2);

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    EXPECT_NE(nullptr, objCall.CreateSession(&objSession));
}

TEST_F(MtcCallTest, CreateSessionCreatesMtcSession)
{
    MockISession objSession;
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    EXPECT_NE(nullptr, objCall.CreateSession(&objSession));
}

TEST_F(MtcCallTest, GetSlotIdCallsMtcContext)
{
    ON_CALL(objContext, GetCallStateProxy)
            .WillByDefault(ReturnRef(objCallStateProxy));
    ON_CALL(objContext, GetConfigurationProxy)
            .WillByDefault(ReturnRef(*pConfigurationProxy));

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    EXPECT_CALL(objContext, GetSlotId)
            .Times(2);  // MtcMediaManager::DestroyMediaSession also calls it when it's destroyed
    objCall.GetSlotId();
}

TEST_F(MtcCallTest, GetDialingPlanCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    MockIMtcDialingPlan objDialingPlan;
    EXPECT_CALL(objContext, GetDialingPlan)
            .Times(1)
            .WillRepeatedly(ReturnRef(objDialingPlan));

    objCall.GetDialingPlan();
}

TEST_F(MtcCallTest, GetServiceByTypeCallsMtcContext)
{
    ServiceType eServiceType = ServiceType::NORMAL;

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    EXPECT_CALL(objContext, GetServiceByType(eServiceType))
            .Times(1);

    objCall.GetServiceByType(eServiceType);
}

TEST_F(MtcCallTest, GetCallManagerCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    MockIMtcCallManager objCallManager;
    EXPECT_CALL(objContext, GetCallManager)
            .Times(1)
            .WillRepeatedly(ReturnRef(objCallManager));

    objCall.GetCallManager();
}

TEST_F(MtcCallTest, GetCallControllerCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    MockIMtcCallController objCallController;
    EXPECT_CALL(objContext, GetCallController)
            .Times(1)
            .WillRepeatedly(ReturnRef(objCallController));

    objCall.GetCallController();
}

TEST_F(MtcCallTest, GetVonrManagerCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    MockIMtcVonrManager objVonrManager;
    EXPECT_CALL(objContext, GetVonrManager)
            .Times(1)
            .WillRepeatedly(ReturnRef(objVonrManager));

    objCall.GetVonrManager();
}

TEST_F(MtcCallTest, GetConfigurationProxyCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    EXPECT_CALL(objContext, GetConfigurationProxy)
            .Times(1);

    objCall.GetConfigurationProxy();
}

TEST_F(MtcCallTest, GetCallStateProxyCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    EXPECT_CALL(objContext, GetCallStateProxy)
            .Times(1);

    objCall.GetCallStateProxy();
}

TEST_F(MtcCallTest, GetImsEventReceiverCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    MockIMtcImsEventReceiver objImsEventReceiver;
    EXPECT_CALL(objContext, GetImsEventReceiver)
            .Times(1)
            .WillRepeatedly(ReturnRef(objImsEventReceiver));

    objCall.GetImsEventReceiver();
}

TEST_F(MtcCallTest, GetAosConnectorCallsMtcContext)
{
    ServiceType eServiceType = ServiceType::NORMAL;

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    EXPECT_CALL(objContext, GetAosConnector(eServiceType))
            .Times(1);

    objCall.GetAosConnector(eServiceType);
}

TEST_F(MtcCallTest, GetSipInterfaceFactoryCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    MockIMtcSipInterfaceFactory objSipInterfaceFactory;
    EXPECT_CALL(objContext, GetSipInterfaceFactory)
            .Times(1)
            .WillRepeatedly(ReturnRef(objSipInterfaceFactory));

    objCall.GetSipInterfaceFactory();
}

TEST_F(MtcCallTest, GetConferenceManagerCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    MockIConferenceManager objConferenceManager;
    EXPECT_CALL(objContext, GetConferenceManager)
            .Times(1)
            .WillRepeatedly(ReturnRef(objConferenceManager));

    objCall.GetConferenceManager();
}

TEST_F(MtcCallTest, GetEctManagerCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    EXPECT_CALL(objContext, GetEctManager)
            .Times(1);

    objCall.GetEctManager();
}

TEST_F(MtcCallTest, GetEmergencyServiceManagerCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    EXPECT_CALL(objContext, GetEmergencyServiceManager)
            .Times(1);

    objCall.GetEmergencyServiceManager();
}

TEST_F(MtcCallTest, SetHeldByMeSetsHeldByMe)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SetHeldByMe(IMS_TRUE);
    EXPECT_EQ(IMS_TRUE, objCall.IsHeldByMe());

    objCall.SetHeldByMe(IMS_FALSE);
    EXPECT_EQ(IMS_FALSE, objCall.IsHeldByMe());
}

TEST_F(MtcCallTest, SessionAlertingCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionAlerting(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionAlerting(&objSession);
}

TEST_F(MtcCallTest, SessionAlertingFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionAlerting(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionAlerting(IMS_NULL);
}

TEST_F(MtcCallTest, SessionReferenceReceivedCallsState)
{
    MockISession objSession;
    MockIReference objReference;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionReferenceReceived(&objSession, &objReference))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionReferenceReceived(&objSession, &objReference);
}

TEST_F(MtcCallTest, SessionReferenceReceivedFailsIfSessionIsNull)
{
    MockIReference objReference;
    EXPECT_CALL(objReference, Reject)
            .Times(1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionReferenceReceived(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionReferenceReceived(IMS_NULL, &objReference);
}

TEST_F(MtcCallTest, SessionReferenceReceivedFailsIfReferenceIsNull)
{
    MockISession objSession;
    EXPECT_CALL(objSession, Terminate)
            .Times(1);

    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(&objSession))
            .Times(1);
    EXPECT_CALL(*pSessionInterfaceHolder, ReleaseISession(&objSession, _))
            .Times(1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionReferenceReceived(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionReferenceReceived(&objSession, IMS_NULL);
}

TEST_F(MtcCallTest, SessionStartedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionStarted(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionStarted(&objSession);
}

TEST_F(MtcCallTest, SessionStartedCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, UssiStarted(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionStarted(&objSession);
}

TEST_F(MtcCallTest, SessionStartedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionStarted(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionStarted(IMS_NULL);
}

TEST_F(MtcCallTest, SessionStartFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionStartFailed(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionStartFailed(&objSession);
}

TEST_F(MtcCallTest, SessionStartFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionStartFailed(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionStartFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionTerminatedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionTerminated(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionTerminated(&objSession);
}

TEST_F(MtcCallTest, SessionTerminatedCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, UssiTerminated(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionTerminated(&objSession);
}

TEST_F(MtcCallTest, SessionTerminatedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionTerminated(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionTerminated(IMS_NULL);
}

TEST_F(MtcCallTest, SessionUpdatedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdated(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionUpdated(&objSession);
}

TEST_F(MtcCallTest, SessionUpdatedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdated(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionUpdated(IMS_NULL);
}

TEST_F(MtcCallTest, SessionUpdateFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdateFailed(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionUpdateFailed(&objSession);
}

TEST_F(MtcCallTest, SessionUpdateFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdateFailed(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionUpdateFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionUpdateReceivedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdateReceived(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionUpdateReceived(&objSession);
}

TEST_F(MtcCallTest, SessionUpdateReceivedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdateReceived(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionUpdateReceived(IMS_NULL);
}

TEST_F(MtcCallTest, SessionCancelDeliveredCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionCancelDelivered(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionCancelDelivered(&objSession);
}

TEST_F(MtcCallTest, SessionCancelDeliveredFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionCancelDelivered(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionCancelDelivered(IMS_NULL);
}

TEST_F(MtcCallTest, SessionCancelDeliveryFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionCancelDeliveryFailed(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionCancelDeliveryFailed(&objSession);
}

TEST_F(MtcCallTest, SessionCancelDeliveryFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionCancelDeliveryFailed(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionCancelDeliveryFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdatedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdated(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionEarlyMediaUpdated(&objSession);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdatedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdated(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionEarlyMediaUpdated(IMS_NULL);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdateFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdateFailed(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionEarlyMediaUpdateFailed(&objSession);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdateFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdateFailed(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionEarlyMediaUpdateFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdateReceivedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdateReceived(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionEarlyMediaUpdateReceived(&objSession);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdateReceivedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdateReceived(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionEarlyMediaUpdateReceived(IMS_NULL);
}

TEST_F(MtcCallTest, SessionForkedResponseReceivedCallsState)
{
    MockISession objSession;
    MockISession objForkedSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionForkedResponseReceived(&objSession, &objForkedSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionForkedResponseReceived(&objSession, &objForkedSession);
}

TEST_F(MtcCallTest, SessionForkedResponseReceivedFailsIfSessionIsNull)
{
    MockISession objForkedSession;
    EXPECT_CALL(objForkedSession, Terminate)
            .Times(1);

    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(&objForkedSession))
            .Times(1);
    EXPECT_CALL(*pSessionInterfaceHolder, ReleaseISession(&objForkedSession, _))
            .Times(1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIncoming(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionForkedResponseReceived(IMS_NULL, &objForkedSession);
}

TEST_F(MtcCallTest, SessionForkedResponseReceivedFailsIfForkedSessionIsNull)
{
    MockISession objSession;
    EXPECT_CALL(objSession, Reject())
            .Times(1);

    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(&objSession))
            .Times(1);
    EXPECT_CALL(*pSessionInterfaceHolder, ReleaseISession(&objSession, _))
            .Times(1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIncoming(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionForkedResponseReceived(&objSession, IMS_NULL);
}

TEST_F(MtcCallTest, SessionPrackDeliveredCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPRAckDelivered(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionPrackDelivered(&objSession);
}

TEST_F(MtcCallTest, SessionPrackDeliveredFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPRAckDelivered(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionPrackDelivered(IMS_NULL);
}

TEST_F(MtcCallTest, SessionPrackDeliveryFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPRAckDeliveryFailed(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionPrackDeliveryFailed(&objSession);
}

TEST_F(MtcCallTest, SessionPrackDeliveryFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPRAckDeliveryFailed(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionPrackDeliveryFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionPrackReceivedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPRAckReceived(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionPrackReceived(&objSession);
}

TEST_F(MtcCallTest, SessionPrackReceivedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPRAckReceived(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionPrackReceived(IMS_NULL);
}

TEST_F(MtcCallTest, SessionProvisionalResponseReceivedCallsState)
{
    MockISession objSession;
    IMS_UINT32 nIndex = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionProvisionalResponseReceived(&objSession, nIndex))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionProvisionalResponseReceived(&objSession, nIndex);
}

TEST_F(MtcCallTest, SessionProvisionalResponseReceivedFailsIfSessionIsNull)
{
    IMS_UINT32 nIndex = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionProvisionalResponseReceived(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionProvisionalResponseReceived(IMS_NULL, nIndex);
}

TEST_F(MtcCallTest, SessionRprDeliveryFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionRPRDeliveryFailed(&objSession))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionRprDeliveryFailed(&objSession);
}

TEST_F(MtcCallTest, SessionRprDeliveryFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionRPRDeliveryFailed(_))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionRprDeliveryFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionRprReceivedCallsState)
{
    MockISession objSession;
    IMS_UINT32 nIndex = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionRPRReceived(&objSession, nIndex))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionRprReceived(&objSession, nIndex);
}

TEST_F(MtcCallTest, SessionRprReceivedFailsIfSessionIsNull)
{
    IMS_UINT32 nIndex = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionRPRReceived(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionRprReceived(IMS_NULL, nIndex);
}

TEST_F(MtcCallTest, SessionTransactionReceivedCallsState)
{
    MockISession objSession;
    MockISipServerConnection objServerConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionTransactionReceived(&objSession, &objServerConnection))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionTransactionReceived(&objSession, &objServerConnection);
}

TEST_F(MtcCallTest, SessionTransactionReceivedCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockISession objSession;
    MockISipServerConnection objServerConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, UssiInfoReceived(&objSession, &objServerConnection))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionTransactionReceived(&objSession, &objServerConnection);
}

TEST_F(MtcCallTest, SessionTransactionReceivedFailsIfSessionIsNull)
{
    MockISipServerConnection objServerConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionTransactionReceived(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.SessionTransactionReceived(IMS_NULL, &objServerConnection);
}

TEST_F(MtcCallTest, OnRefreshCompletedCallsState)
{
    MockISipClientConnection objConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Refresh_NotifyCompleted(&objConnection))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Refresh_NotifyCompleted(&objConnection);
}

TEST_F(MtcCallTest, OnRefreshTerminatedCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Refresh_NotifyTerminated())
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Refresh_NotifyTerminated();
}

TEST_F(MtcCallTest, OnRefreshTimerExpiredCallsState)
{
    IMS_BOOL bDoImplicitRefresh = IMS_FALSE;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Refresh_NotifyTimerExpired(bDoImplicitRefresh))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Refresh_NotifyTimerExpired(bDoImplicitRefresh);
}

TEST_F(MtcCallTest, OnTimerExpiredCallsState)
{
    IMS_SINT32 nType = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnTimerExpired(nType))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.OnTimerExpired(nType);
}

TEST_F(MtcCallTest, OnBlockCheckedCallsState)
{
    IMtcBlockChecker::Result objResult(IMtcBlockChecker::Result::Status::UNBLOCKED);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnBlockChecked(objResult))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.OnBlockChecked(objResult);
}

TEST_F(MtcCallTest, QosReservedCallsState)
{
    MockISession objSession;
    IMS_UINT32 eMediaType = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, QosReserved(&objSession, eMediaType))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.QosReserved(&objSession, eMediaType);
}

TEST_F(MtcCallTest, QosReservedFailsIfSessionIsNull)
{
    IMS_UINT32 eMediaType = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, QosReserved(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.QosReserved(IMS_NULL, eMediaType);
}

TEST_F(MtcCallTest, QosReserveFailedCallsState)
{
    MockISession objSession;
    QosLossPolicy eNextAction = QosLossPolicy::MAINTAIN;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, QosReserveFailed(&objSession, eNextAction))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.QosReserveFailed(&objSession, eNextAction);
}

TEST_F(MtcCallTest, QosReserveFailedFailsIfSessionIsNull)
{
    QosLossPolicy eNextAction = QosLossPolicy::MAINTAIN;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, QosReserveFailed(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.QosReserveFailed(IMS_NULL, eNextAction);
}

TEST_F(MtcCallTest, OnStateTransitionUpdatesCallStateProxy)
{
    CallStateName eState = CallStateName::INCOMING;

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    EXPECT_CALL(objCallStateProxy, UpdateCallState(objCall.GetKey(), eState, objCall.GetCallType(),
            objCallInfo.bEmergency, _))
            .Times(1);

    objCall.OnStateTransition(eState);
}

TEST_F(MtcCallTest, ClientConnection_NotifyResponseCallsState)
{
    MockISipClientConnection objConnection;
    MockISipClientConnection objForkedConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, ClientConnection_NotifyResponse(&objConnection, &objForkedConnection))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.ClientConnection_NotifyResponse(&objConnection, &objForkedConnection);
}

TEST_F(MtcCallTest, ClientConnection_NotifyResponseCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockISipClientConnection objConnection;
    MockISipClientConnection objForkedConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, NotifyResponseToUssiInfo(&objConnection, &objForkedConnection))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.ClientConnection_NotifyResponse(&objConnection, &objForkedConnection);
}

TEST_F(MtcCallTest, ClientConnection_NotifyResponseFailsIfConnectionIsNull)
{
    MockISipClientConnection objForkedConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, ClientConnection_NotifyResponse(_, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.ClientConnection_NotifyResponse(IMS_NULL, &objForkedConnection);
}

TEST_F(MtcCallTest, Error_NotifyErrorCallsState)
{
    MockISipConnection objConnection;
    IMS_SINT32 nCode = 1;
    AString strMessage("some_message");

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Error_NotifyError(&objConnection, nCode, strMessage))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Error_NotifyError(&objConnection, nCode, strMessage);
}

TEST_F(MtcCallTest, Error_NotifyErrorCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockISipConnection objConnection;
    IMS_SINT32 nCode = 1;
    AString strMessage("some_message");

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, NotifyErrorToUssiInfo(&objConnection, nCode, strMessage))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Error_NotifyError(&objConnection, nCode, strMessage);
}

TEST_F(MtcCallTest, Error_NotifyErrorFailsIfConnectionIsNull)
{
    IMS_SINT32 nCode = 1;
    AString strMessage("some_message");

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Error_NotifyError(_, _, _))
            .Times(0);
    EXPECT_CALL(*pState, OnInternalFailure)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.Error_NotifyError(IMS_NULL, nCode, strMessage);
}

TEST_F(MtcCallTest, OnReceivingMediaDataFailedCallsState)
{
    IMS_UINT32 eMediaType = 1;
    IMS_UINT32 eProtocolType = 2;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnReceivingMediaDataFailed(eMediaType, eProtocolType))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.OnReceivingMediaDataFailed(eMediaType, eProtocolType);
}

TEST_F(MtcCallTest, OnVideoLowestBitRateCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnVideoLowestBitRate)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.OnVideoLowestBitRate();
}

TEST_F(MtcCallTest, OnReceivingNetworkToneStartedCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnReceivingNetworkToneStarted)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.OnReceivingNetworkToneStarted();
}

TEST_F(MtcCallTest, OnReceivingNetworkToneFailedCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnReceivingNetworkToneFailed)
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.OnReceivingNetworkToneFailed();
}

TEST_F(MtcCallTest, OnMediaFailedCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnMediaFailed(objReason))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, std::move(CreateStateFactory(pState)));

    objCall.OnMediaFailed(objReason);
}
