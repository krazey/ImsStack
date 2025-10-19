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

#include "CallReasonInfo.h"
#include "IIpcan.h"
#include "INetworkWatcher.h"
#include "ISipHeader.h"
#include "ImsList.h"
#include "MockIMessage.h"
#include "MockIMtcCallController.h"
#include "MockIMtcContext.h"
#include "MockIMtcImsEventReceiver.h"
#include "MockIMtcService.h"
#include "MockIReference.h"
#include "MockISession.h"
#include "MockISipClientConnection.h"
#include "MockISipConnection.h"
#include "MockISipMessage.h"
#include "MockISipServerConnection.h"
#include "MtcDef.h"
#include "SipStatusCode.h"
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcCall.h"
#include "call/IMtcSession.h"
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/MtcCall.h"
#include "call/MtcCallStringUtils.h"
#include "call/MtcPendingOperationHolder.h"
#include "call/NullCall.h"
#include "call/UpdatingInfo.h"
#include "call/radio/MockIMtcRadioChecker.h"
#include "call/state/MockIMtcCallState.h"
#include "call/state/MockMtcCallStateMachine.h"
#include "call/state/MtcCallState.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/MockIConferenceManager.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MockIMtcDialingPlan.h"
#include "ect/MockIEctManager.h"
#include "emergency/CurrentLocationDiscoveryController.h"
#include "emergency/MockIMtcEmergencyServiceManager.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/ISrvccStateListener.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockILastComeFirstServedHelper.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "helper/MockMtcTimerWrapper.h"
#include "helper/OperationAsyncRunner.h"
#include "helper/UdpKeepAliveSender.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSessionInterfaceHolder.h"
#include "ussi/UssiConstants.h"
#include "utility/MessageUtils.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>
#include <memory>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Unused;

// SipMethod
MATCHER_P(IsEqualSipMethod, method, "")
{
    return arg.Equals(method);
}

class MtcCallTest : public ::testing::Test
{
public:
    MockIMtcService objService;
    MockIMtcContext objContext;
    CallInfo objCallInfo;
    MockICallStateProxy objCallStateProxy;
    MockIMtcSipInterfaceFactory objSipInterfaceFactory;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MockIMtcRadioChecker objRadioChecker;
    MockSessionInterfaceHolder* pSessionInterfaceHolder;
    MessageUtils objMessageUtils;
    MockIMtcCallManager objCallManager;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));

        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objCallStateProxy, UpdateCallState(_, _, _, _, _)).WillByDefault(Return());

        pSessionInterfaceHolder = new MockSessionInterfaceHolder();
        ON_CALL(objSipInterfaceFactory, GetISessionHolder)
                .WillByDefault(ReturnRef(*pSessionInterfaceHolder));
        ON_CALL(objContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(objSipInterfaceFactory));

        ON_CALL(objContext, CreateTimer)
                .WillByDefault(Invoke(
                        []()
                        {
                            return std::make_unique<MockMtcTimerWrapper>();
                        }));
        ON_CALL(objContext, GetRadioChecker).WillByDefault(ReturnRef(objRadioChecker));

        ON_CALL(objCallManager, GetCallsExcluding(_)).WillByDefault(Return(ImsList<IMtcCall*>()));
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
                .WillByDefault(Invoke(
                        [pState](Unused, Unused)
                        {
                            return pState;
                        }));
        return pStateFactory;
    }

    std::unique_ptr<MockIMtcCallStateFactory> CreateStateFactory()
    {
        return CreateStateFactory(new MockIMtcCallState());
    }

    void SetIsCsfbAvailable(IN IMS_BOOL bIfEpsOnlyAttachBlocked, IN IMS_BOOL bEpsOnlyAttach,
            IN IMS_BOOL bInNrBlocked, IN IMS_BOOL bNr, IN IMS_BOOL bInWifiBlocked,
            IN IMS_BOOL bWlanIpCanType, IN IMS_BOOL bInRoamingBlocked, IN IMS_BOOL bRoaming,
            IN IMS_BOOL bInHomeBlocked)
    {
        ON_CALL(*pConfigurationProxy,
                Contains(ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                        ConfigVoice::CSFB_BLOCK_CONDITION_IF_EPS_ONLY_ATTACH))
                .WillByDefault(Return(bIfEpsOnlyAttachBlocked));
        ON_CALL(*pConfigurationProxy,
                Contains(ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                        ConfigVoice::CSFB_BLOCK_CONDITION_IN_NR))
                .WillByDefault(Return(bInNrBlocked));
        ON_CALL(*pConfigurationProxy,
                Contains(ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                        ConfigVoice::CSFB_BLOCK_CONDITION_IN_WIFI))
                .WillByDefault(Return(bInWifiBlocked));
        ON_CALL(*pConfigurationProxy,
                Contains(ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                        ConfigVoice::CSFB_BLOCK_CONDITION_IN_ROAMING))
                .WillByDefault(Return(bInRoamingBlocked));
        ON_CALL(*pConfigurationProxy,
                Contains(ConfigVoice::KEY_CSFB_BLOCK_CONDITION_INT_ARRAY,
                        ConfigVoice::CSFB_BLOCK_CONDITION_IN_HOME))
                .WillByDefault(Return(bInHomeBlocked));

        ON_CALL(objService, IsEpsOnlyAttach).WillByDefault(Return(bEpsOnlyAttach));
        ON_CALL(objService, IsNr).WillByDefault(Return(bNr));
        ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(bWlanIpCanType));
        ON_CALL(objService, IsRoaming).WillByDefault(Return(bRoaming));
    }
};

TEST_F(MtcCallTest, ConstructorAddsListeners)
{
    EXPECT_CALL(objService, AddAosStateListener(_));
    EXPECT_CALL(objService, AddSrvccStateListener(_));
    EXPECT_CALL(objService, AddNetworkWatcherListener(_));
    EXPECT_CALL(objRadioChecker, AddTrafficCheckerListener(_));

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));
}

TEST_F(MtcCallTest, DestructorRemovesListeners)
{
    EXPECT_CALL(objService, RemoveAosStateListener(_));
    EXPECT_CALL(objService, RemoveSrvccStateListener(_));
    EXPECT_CALL(objService, RemoveNetworkWatcherListener(_));
    EXPECT_CALL(objRadioChecker, RemoveTrafficCheckerListener(_));

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));
}

TEST_F(MtcCallTest, AttachNotCallsStateForMo)
{
    objCallInfo.ePeerType = PeerType::MO;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnAttached).Times(0);
    EXPECT_CALL(*pState, OnUssiAttached).Times(0);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Attach();
}

TEST_F(MtcCallTest, AttachCallsStateForMt)
{
    objCallInfo.ePeerType = PeerType::MT;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnAttached).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Attach();
}

TEST_F(MtcCallTest, AttachCallsStateForMtUssi)
{
    objCallInfo.ePeerType = PeerType::MT;
    objCallInfo.bUssi = IMS_TRUE;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnUssiAttached).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Attach();
}

TEST_F(MtcCallTest, StartCallsState)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo objMediaInfo;
    ImsList<SuppService*> objSuppServices;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Start(eCallType, strTarget, objMediaInfo, Ref(objSuppServices))).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Start(eCallType, strTarget, objMediaInfo, objSuppServices);
}

TEST_F(MtcCallTest, StartCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo objMediaInfo;
    ImsList<SuppService*> objSuppServices;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Start(eCallType, strTarget, objMediaInfo, Ref(objSuppServices))).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Start(eCallType, strTarget, objMediaInfo, objSuppServices);
}

TEST_F(MtcCallTest, StartConference5CallsState)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    MediaInfo objMediaInfo;
    ImsList<SuppService*> objSuppServices;
    ImsList<ConfUser*> lstUsers;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState,
            StartConference(
                    eCallType, strTarget, objMediaInfo, Ref(objSuppServices), Ref(lstUsers)))
            .Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.StartConference(eCallType, strTarget, objMediaInfo, objSuppServices, lstUsers);
}

TEST_F(MtcCallTest, StartConference3CallsState)
{
    CallType eCallType = CallType::VOIP;
    AString strTarget("some_target");
    ImsList<ConfUser*> lstUsers;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, StartConference(eCallType, strTarget, Ref(lstUsers))).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.StartConference(eCallType, strTarget, lstUsers);
}

TEST_F(MtcCallTest, HandleIncomingCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIncoming(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.HandleIncoming(&objSession);
}

TEST_F(MtcCallTest, HandleIncomingCallsStateForUssi)
{
    MockISipMessage objUssiSipMessage;
    ImsList<AString> lstRecvInfoHeaders;
    lstRecvInfoHeaders.Append(UssiConstants::HEADER_USSD_PACKAGE);
    ON_CALL(objUssiSipMessage, GetHeaders(ISipHeader::RECV_INFO, _))
            .WillByDefault(Return(lstRecvInfoHeaders));
    ImsList<AString> lstAcceptHeaders;
    lstAcceptHeaders.Append(UssiConstants::HEADER_APPLICATION_USSDXML);
    ON_CALL(objUssiSipMessage, GetHeaders(ISipHeader::ACCEPT, _))
            .WillByDefault(Return(lstAcceptHeaders));

    MockIMessage objUssiMessage;
    ON_CALL(objUssiMessage, GetMessage).WillByDefault(Return(&objUssiSipMessage));

    MockISession objSession;
    ON_CALL(objSession, GetPreviousRequest(_)).WillByDefault(Return(&objUssiMessage));

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIncomingUssi(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.HandleIncoming(&objSession);
}

TEST_F(MtcCallTest, HandleIncomingFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIncoming(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.HandleIncoming(IMS_NULL);
}

TEST_F(MtcCallTest, HandleUserAlertCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleUserAlert).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.HandleUserAlert();
}

TEST_F(MtcCallTest, HandleUserAlertDoesNothingForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleUserAlert).Times(0);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.HandleUserAlert();
}

TEST_F(MtcCallTest, AcceptCallsState)
{
    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Accept(eCallType, objMediaInfo)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Accept(eCallType, objMediaInfo);
}

TEST_F(MtcCallTest, AcceptCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, AcceptUssi(eCallType, objMediaInfo)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Accept(eCallType, objMediaInfo);
}

TEST_F(MtcCallTest, RejectCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Reject(objReason)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Reject(objReason);
}

TEST_F(MtcCallTest, HoldCallsState)
{
    MediaInfo objMediaInfo;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Hold(objMediaInfo)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Hold(objMediaInfo);
}

TEST_F(MtcCallTest, ResumeCallsState)
{
    MediaInfo objMediaInfo;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Resume(objMediaInfo)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Resume(objMediaInfo);
}

TEST_F(MtcCallTest, AcceptResumeCallsState)
{
    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, AcceptResume(eCallType, objMediaInfo)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.AcceptResume(eCallType, objMediaInfo);
}

TEST_F(MtcCallTest, RejectResumeCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, RejectResume(objReason)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.RejectResume(objReason);
}

TEST_F(MtcCallTest, UpdateCallsState)
{
    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Update(eCallType, objMediaInfo)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Update(eCallType, objMediaInfo);
}

TEST_F(MtcCallTest, AcceptUpdateCallsState)
{
    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, AcceptUpdate(eCallType, objMediaInfo)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.AcceptUpdate(eCallType, objMediaInfo);
}

TEST_F(MtcCallTest, RejectUpdateCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, RejectUpdate(objReason)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.RejectUpdate(objReason);
}

TEST_F(MtcCallTest, CancelUpdateCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, CancelUpdate(objReason)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.CancelUpdate(objReason);
}

TEST_F(MtcCallTest, TerminateCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Terminate(objReason)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Terminate(objReason);
}

TEST_F(MtcCallTest, TerminateCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, TerminateUssi(objReason)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Terminate(objReason);
}

TEST_F(MtcCallTest, SendUssdCallsState)
{
    AString strUssd("some_ussd");

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SendUssd(strUssd)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SendUssd(strUssd);
}

TEST_F(MtcCallTest, GetKeyReturnsValidKeyInitially)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    NullCall objNullCall;

    EXPECT_NE(objNullCall.GetKey(), objCall.GetKey());
}

TEST_F(MtcCallTest, GetKeyReturnsNotOverlappingKey)
{
    MtcCall objCall1(objContext, objService, objCallInfo, CreateStateFactory());
    MtcCall objCall2(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_NE(objCall1.GetKey(), objCall2.GetKey());
}

TEST_F(MtcCallTest, GetCallTypeReturnsInitialCallTypeWhenNoSessionExists)
{
    objCallInfo.eInitialCallType = CallType::VIDEO_RTT;
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(objCallInfo.eInitialCallType, objCall.GetCallType());
}

TEST_F(MtcCallTest, GetCallTypeReturnsCallTypeOfLastSession)
{
    MockISession objSession1;
    MockISession objSession2;

    objCallInfo.eInitialCallType = CallType::VOIP;
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    IMtcSession* pMtcSession1 = objCall.CreateSession(&objSession1);
    IMtcSession* pMtcSession2 = objCall.CreateSession(&objSession2);
    pMtcSession1->SetCallType(CallType::RTT);
    pMtcSession2->SetCallType(CallType::VIDEO_RTT);

    EXPECT_EQ(pMtcSession2->GetCallType(), objCall.GetCallType());
}

TEST_F(MtcCallTest, GetStateReturnsIdleInitially)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(CallStateName::IDLE, objCall.GetState());
}

TEST_F(MtcCallTest, GetStateReturnsChangedState)
{
    const CallStateName eChangedState = CallStateName::ESTABLISHED;

    MockIMtcCallState* pState = new MockIMtcCallState();
    ON_CALL(*pState, GetStateName).WillByDefault(Return(eChangedState));
    ON_CALL(*pState, HandleUserAlert).WillByDefault(Return(eChangedState));

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));
    objCall.HandleUserAlert();

    EXPECT_EQ(eChangedState, objCall.GetState());
}

TEST_F(MtcCallTest, GetCallContextReturnsThis)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(&objCall, &objCall.GetCallContext());
}

TEST_F(MtcCallTest, GetCallKeyReturnsKey)
{
    MtcCall objCall1(objContext, objService, objCallInfo, CreateStateFactory());
    MtcCall objCall2(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(objCall1.GetKey(), objCall1.GetCallKey());
    EXPECT_EQ(objCall2.GetKey(), objCall2.GetCallKey());
}

TEST_F(MtcCallTest, IsEstablishedReturnsTrueAfterEstablishedState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_FALSE(objCall.IsEstablished());

    objCall.OnStateTransition(CallStateName::IDLE);
    EXPECT_FALSE(objCall.IsEstablished());

    objCall.OnStateTransition(CallStateName::OUTGOING);
    EXPECT_FALSE(objCall.IsEstablished());

    objCall.OnStateTransition(CallStateName::INCOMING);
    EXPECT_FALSE(objCall.IsEstablished());

    objCall.OnStateTransition(CallStateName::ALERTING);
    EXPECT_FALSE(objCall.IsEstablished());

    objCall.OnStateTransition(CallStateName::TERMINATING);
    EXPECT_FALSE(objCall.IsEstablished());

    objCall.OnStateTransition(CallStateName::ESTABLISHED);
    EXPECT_TRUE(objCall.IsEstablished());

    objCall.OnStateTransition(CallStateName::UPDATING);
    EXPECT_TRUE(objCall.IsEstablished());

    objCall.OnStateTransition(CallStateName::TERMINATING);
    EXPECT_TRUE(objCall.IsEstablished());
}

TEST_F(MtcCallTest, IsHeldByMeInitiallyReturnsFalse)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(IMS_FALSE, objCall.IsHeldByMe());
}

TEST_F(MtcCallTest, IsUssiReturnsFalseIfNotUssi)
{
    objCallInfo.bUssi = IMS_FALSE;
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(IMS_FALSE, objCall.IsUssi());
}

TEST_F(MtcCallTest, IsUssiReturnsTrueIfUssi)
{
    objCallInfo.bUssi = IMS_TRUE;
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(IMS_TRUE, objCall.IsUssi());
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsFalseIfOtherCallExists)
{
    MtcCall objExistingCall(objContext, objService, objCallInfo, CreateStateFactory());
    ImsList<IMtcCall*> lstOtherCalls;
    lstOtherCalls.Append(&objExistingCall);
    ON_CALL(objCallManager, GetCallsExcluding(_)).WillByDefault(Return(lstOtherCalls));

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_FALSE, IMS_TRUE, IMS_FALSE, IMS_TRUE, IMS_FALSE, IMS_TRUE, IMS_FALSE,
            IMS_TRUE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_FALSE);
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsFalseIfEpsOnlyAttachBlocked)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_TRUE, IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_FALSE);
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsFalseIfInNrBlocked)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_FALSE);
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsFalseIfInWifiBlocked)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_TRUE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_FALSE);
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsFalseIfInRoamingBlocked)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE,
            IMS_TRUE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_FALSE);
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsFalseIfInHomeBlocked)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_TRUE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_FALSE);
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsTrueIfNoBlockConditions)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_TRUE);
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsTrueIfEpsOnlyAttachNotBlocked)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_TRUE);

    SetIsCsfbAvailable(IMS_FALSE, IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_TRUE);
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsTrueIfInNrNotBlocked)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_TRUE);

    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_TRUE);
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsTrueIfInWifiNotBlocked)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_FALSE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_TRUE);

    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE, IMS_FALSE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_TRUE);
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsTrueIfInRoamingNotBlocked)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_TRUE,
            IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_TRUE);

    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_TRUE, IMS_FALSE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_TRUE);
}

TEST_F(MtcCallTest, IsCsfbAvailableReturnsTrueIfInHomeNotBlocked)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    SetIsCsfbAvailable(IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE, IMS_FALSE,
            IMS_TRUE, IMS_TRUE);
    EXPECT_EQ(objCall.IsCsfbAvailable(), IMS_TRUE);
}

TEST_F(MtcCallTest, GetCallInfoReturnsCallInfo)
{
    objCallInfo.bConference = !objCallInfo.bConference;
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(objCallInfo, objCall.GetCallInfo());
}

TEST_F(MtcCallTest, GetParticipantInfoReturnsNotNull)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_NE(nullptr, &objCall.GetParticipantInfo());
}

TEST_F(MtcCallTest, GetSessionWithISessionInitiallyReturnsNull)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    MockISession objSession;

    EXPECT_EQ(nullptr, objCall.GetSession(&objSession));
}

TEST_F(MtcCallTest, GetSessionWithISessionReturnsMatchingSession)
{
    MockISession objSession1;
    MockISession objSession2;

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    const IMtcSession* pMtcSession1 = objCall.CreateSession(&objSession1);
    const IMtcSession* pMtcSession2 = objCall.CreateSession(&objSession2);

    EXPECT_EQ(pMtcSession1, objCall.GetSession(&objSession1));
    EXPECT_EQ(pMtcSession2, objCall.GetSession(&objSession2));
}

TEST_F(MtcCallTest, GetSessionInitiallyReturnsNull)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(nullptr, objCall.GetSession());
}

TEST_F(MtcCallTest, GetSessionReturnsLastSession)
{
    MockISession objSession1;
    MockISession objSession2;

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    const IMtcSession* pMtcSession1 = objCall.CreateSession(&objSession1);
    const IMtcSession* pMtcSession2 = objCall.CreateSession(&objSession2);

    EXPECT_NE(pMtcSession1, objCall.GetSession());
    EXPECT_EQ(pMtcSession2, objCall.GetSession());
}

TEST_F(MtcCallTest, GetSessionsReturnsAllSessions)
{
    MockISession objSession1;
    MockISession objSession2;

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    const IMtcSession* pMtcSession1 = objCall.CreateSession(&objSession1);
    const IMtcSession* pMtcSession2 = objCall.CreateSession(&objSession2);

    ImsList<IMtcSession*> objSessions = objCall.GetSessions();

    EXPECT_EQ(objSessions.GetAt(0), pMtcSession1);
    EXPECT_EQ(objSessions.GetAt(1), pMtcSession2);
}

TEST_F(MtcCallTest, GetServiceReturnsGivenService)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(&objService, &objCall.GetService());
}

TEST_F(MtcCallTest, GetUiNotifierReturnsNotNull)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_NE(nullptr, &objCall.GetUiNotifier());
}

TEST_F(MtcCallTest, GetMediaManagerReturnsNotNull)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_NE(nullptr, &objCall.GetMediaManager());
}

TEST_F(MtcCallTest, GetPreconditionManagerReturnsNotNull)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_NE(nullptr, &objCall.GetPreconditionManager());
}

TEST_F(MtcCallTest, GetUssiControllerInitiallyReturnsNull)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(nullptr, objCall.GetUssiController());
}

TEST_F(MtcCallTest, GetPendingOperationHolderReturnsNotNull)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_NE(nullptr, &objCall.GetPendingOperationHolder());
}

TEST_F(MtcCallTest, GetOtherCallsReturnsExcludingMe)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    EXPECT_CALL(objCallManager, GetCallsExcluding(objCall.GetKey())).Times(1);
    objCall.GetOtherCalls();
}

TEST_F(MtcCallTest, GetUpdatingInfoReturnsSameNotNullInstance)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    const UpdatingInfo& objUpdatingInfo = objCall.GetUpdatingInfo();

    EXPECT_NE(nullptr, &objUpdatingInfo);
    EXPECT_EQ(&objUpdatingInfo, &objCall.GetUpdatingInfo());
}

TEST_F(MtcCallTest, GetEpsFallbackTriggerReturnsSameNotNullInstance)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    const EpsFallbackTrigger& objEpsFbTrigger = objCall.GetEpsFallbackTrigger();

    EXPECT_NE(nullptr, &objEpsFbTrigger);
    EXPECT_EQ(&objEpsFbTrigger, &objCall.GetEpsFallbackTrigger());
}

TEST_F(MtcCallTest, GetCurrentLocationDiscoveryControllerCreatesInstance)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    const CurrentLocationDiscoveryController* pCurrentLocationDiscoveryController =
            &objCall.GetCurrentLocationDiscoveryController();

    EXPECT_NE(nullptr, pCurrentLocationDiscoveryController);
}

TEST_F(MtcCallTest, CreateSessionDoesNothingIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_EQ(nullptr, objCall.CreateSession(IMS_NULL));
}

TEST_F(MtcCallTest, CreateSessionDoesNothingIfMtcSessionExists)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.CreateSession(&objSession);

    EXPECT_EQ(nullptr, objCall.CreateSession(&objSession));
}

TEST_F(MtcCallTest, CreateSessionSetListenersOfSession)
{
    MockISession objSession;
    EXPECT_CALL(objSession, SetListener(_)).Times(1);
    // 2 times: when init and in MtcSession::~MtcSession
    EXPECT_CALL(objSession, SetMessageMediator(_)).Times(2);
    EXPECT_CALL(objSession, SetRefreshListener(_)).Times(2);

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_NE(nullptr, objCall.CreateSession(&objSession));
}

TEST_F(MtcCallTest, CreateSessionCreatesMtcSession)
{
    MockISession objSession;
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_NE(nullptr, objCall.CreateSession(&objSession));
}

TEST_F(MtcCallTest, CreateSessionGetsISessionFromInterfaceFactory)
{
    EXPECT_CALL(*pSessionInterfaceHolder, GetISession(_, _, _, _));

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.CreateSession();
}

TEST_F(MtcCallTest, CreateBlockCheckerReturnsNotNull)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    ImsList<IMtcBlockRule*> lstSomeRules;

    EXPECT_NE(nullptr, objCall.CreateBlockChecker(lstSomeRules));
}

TEST_F(MtcCallTest, CreateJniCallInfoReturnsMatchingWithCallInfo)
{
    objCallInfo.eInitialCallType = CallType::VT;
    objCallInfo.eEmergencyType = EmergencyType::NORMAL_ROUTING;
    objCallInfo.bOffline = !objCallInfo.bOffline;
    objCallInfo.bUssi = !objCallInfo.bUssi;
    objCallInfo.bConference = !objCallInfo.bConference;

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    JniCallInfo objJniCallInfo = objCall.CreateJniCallInfo();

    EXPECT_EQ(objService.GetServiceType(), objJniCallInfo.eServiceType);
    EXPECT_EQ(objCall.GetCallType(), objJniCallInfo.eCallType);
    EXPECT_EQ(objCallInfo.eEmergencyType, objJniCallInfo.eEmergencyType);
    EXPECT_EQ(objCallInfo.bOffline, objJniCallInfo.bOffline);
    EXPECT_EQ(objCallInfo.bUssi, objJniCallInfo.bUssi);
    EXPECT_EQ(objCallInfo.bConference, objJniCallInfo.bConference);
}

TEST_F(MtcCallTest, CreateJniCallInfoReturnsConferenceSubscribeFalse)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_SUBSCRIBE_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_SUBSCRIBE_NOT_SUPPORT));

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    JniCallInfo objJniCallInfo = objCall.CreateJniCallInfo();

    EXPECT_EQ(IMS_FALSE, objJniCallInfo.bConferenceSubscriptionRequired);
}

TEST_F(MtcCallTest, CreateJniCallInfoReturnsConferenceSubscribeTrueIfInDialog)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_SUBSCRIBE_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_SUBSCRIBE_TYPE_IN_DIALOG));

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    JniCallInfo objJniCallInfo = objCall.CreateJniCallInfo();

    EXPECT_EQ(IMS_TRUE, objJniCallInfo.bConferenceSubscriptionRequired);
}

TEST_F(MtcCallTest, CreateJniCallInfoReturnsConferenceSubscribeTrueIfOutDialog)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_CONFERENCE_SUBSCRIBE_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::CONFERENCE_SUBSCRIBE_TYPE_OUT_OF_DIALOG));

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    JniCallInfo objJniCallInfo = objCall.CreateJniCallInfo();

    EXPECT_EQ(IMS_TRUE, objJniCallInfo.bConferenceSubscriptionRequired);
}

TEST_F(MtcCallTest, CreateJniCallInfoInitiallyReturnsFalseCapability)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    JniCallInfo objJniCallInfo = objCall.CreateJniCallInfo();

    EXPECT_EQ(IMS_FALSE, objJniCallInfo.bRttCapable);
    EXPECT_EQ(IMS_FALSE, objJniCallInfo.bVideoCapable);
}

TEST_F(MtcCallTest, CreateJniCallInfoReturnsCapabilityOfSession)
{
    MockISession objSession;
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    const IMtcSession* pSession = objCall.CreateSession(&objSession);
    JniCallInfo objJniCallInfo = objCall.CreateJniCallInfo();

    EXPECT_EQ(pSession->IsRttCapable(), objJniCallInfo.bRttCapable);
    EXPECT_EQ(pSession->IsVideoCapable(), objJniCallInfo.bVideoCapable);
}

TEST_F(MtcCallTest, CreateJniCallInfoReturnsMatchingWithCrossSim)
{
    ON_CALL(objService, IsCrossSimConnected).WillByDefault(Return(IMS_TRUE));

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    JniCallInfo objJniCallInfo = objCall.CreateJniCallInfo();

    EXPECT_EQ(objService.IsCrossSimConnected(), objJniCallInfo.bCrossSim);

    ON_CALL(objService, IsCrossSimConnected).WillByDefault(Return(IMS_FALSE));
    objJniCallInfo = objCall.CreateJniCallInfo();

    EXPECT_EQ(objService.IsCrossSimConnected(), objJniCallInfo.bCrossSim);
}

TEST_F(MtcCallTest, CreateClientConnectionReturnsNullIfNoSessionExists)
{
    const SipMethod eAnyMethod = SipMethod::INVITE;

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_EQ(nullptr, objCall.CreateClientConnection(eAnyMethod));
}

TEST_F(MtcCallTest, CreateClientConnectionReturnsNullIfISessionReturnsNull)
{
    const SipMethod eAnyMethod = SipMethod::INVITE;

    MockISession objSession;
    EXPECT_CALL(objSession, CreateTransaction(IsEqualSipMethod(eAnyMethod)))
            .Times(1)
            .WillOnce(Return(nullptr));

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    objCall.CreateSession(&objSession);

    EXPECT_EQ(nullptr, objCall.CreateClientConnection(eAnyMethod));
}

TEST_F(MtcCallTest, CreateClientConnectionReturnsConnectionFromISession)
{
    const SipMethod eAnyMethod = SipMethod::INVITE;

    MockISipClientConnection objClientConnection;
    MockISession objSession;
    EXPECT_CALL(objSession, CreateTransaction(IsEqualSipMethod(eAnyMethod)))
            .Times(1)
            .WillOnce(Return(&objClientConnection));

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    objCall.CreateSession(&objSession);

    EXPECT_EQ(&objClientConnection, objCall.CreateClientConnection(eAnyMethod));
}

TEST_F(MtcCallTest, CreateUdpKeepAliveSenderReturnsInstance)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    UdpKeepAliveSender* pUdpKeepAliveSender = objCall.CreateUdpKeepAliveSender();
    EXPECT_NE(nullptr, pUdpKeepAliveSender);

    delete pUdpKeepAliveSender;
}

TEST_F(MtcCallTest, RemoveSessionRemovesMatchingSession)
{
    MockISession objSession1;
    MockISession objSession2;

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    objCall.CreateSession(&objSession1);
    IMtcSession* piMtcSession2 = objCall.CreateSession(&objSession2);
    objCall.RemoveSession(*piMtcSession2);

    EXPECT_NE(nullptr, objCall.GetSession(&objSession1));
    EXPECT_EQ(nullptr, objCall.GetSession(&objSession2));
}

TEST_F(MtcCallTest, RemoveAllSessionsRemovesAllSessions)
{
    MockISession objSession1;
    MockISession objSession2;

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    objCall.CreateSession(&objSession1);
    objCall.CreateSession(&objSession2);
    objCall.RemoveAllSessions();

    EXPECT_EQ(nullptr, objCall.GetSession(&objSession1));
    EXPECT_EQ(nullptr, objCall.GetSession(&objSession2));
    EXPECT_EQ(nullptr, objCall.GetSession());
}

TEST_F(MtcCallTest, DeleteUpdatingInfoDeletesPreviousOne)
{
    const CallType eChangedCallType = CallType::VT;

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    objCall.GetUpdatingInfo().SetTargetCallType(eChangedCallType);

    objCall.DeleteUpdatingInfo();

    EXPECT_NE(eChangedCallType, objCall.GetUpdatingInfo().GetTargetCallType());
}

TEST_F(MtcCallTest, RunPendingOperationIfPossibledRunsAllPendingOperationIfEstablished)
{
    std::function<IMtcCall::State(IMtcCallState*)> objPendingOperationReturnEstablished =
            [](IMtcCallState* /* pState */)
    {
        return CallStateName::ESTABLISHED;
    };

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    ON_CALL(*pState, GetStateName).WillByDefault(Return(CallStateName::ESTABLISHED));

    objCall.GetPendingOperationHolder().PushPendingOperation(objPendingOperationReturnEstablished);
    objCall.GetPendingOperationHolder().PushPendingOperation(objPendingOperationReturnEstablished);
    objCall.GetPendingOperationHolder().PushPendingOperation(objPendingOperationReturnEstablished);

    objCall.RunPendingOperationIfPossible();
    EXPECT_FALSE(objCall.GetPendingOperationHolder().HasPendingOperation());
}

TEST_F(MtcCallTest, RunPendingOperationIfPossibledNotRunPendingOperationIfInDelay)
{
    std::function<IMtcCall::State(IMtcCallState*)> objPendingOperationReturnEstablished =
            [](IMtcCallState* /* pState */)
    {
        return CallStateName::ESTABLISHED;
    };

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    ON_CALL(*pState, GetStateName).WillByDefault(Return(CallStateName::ESTABLISHED));

    objCall.GetPendingOperationHolder().PushPendingOperation(objPendingOperationReturnEstablished);

    EXPECT_CALL(static_cast<MockMtcTimerWrapper&>(objCall.GetTimer()),
            IsActive(MtcCallState::TimerType::TIMER_DELAY_UPDATE_AFTER_CONNECTED))
            .WillRepeatedly(Return(IMS_TRUE));

    objCall.RunPendingOperationIfPossible();
    EXPECT_TRUE(objCall.GetPendingOperationHolder().HasPendingOperation());
}

TEST_F(MtcCallTest, RunPendingOperationIfPossibledNotRunsAllPendingOperationIfUpdating)
{
    std::function<IMtcCall::State(IMtcCallState*)> objPendingOperationReturnEstablished =
            [](IMtcCallState* /* pState */)
    {
        return CallStateName::ESTABLISHED;
    };

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    ON_CALL(*pState, GetStateName).WillByDefault(Return(CallStateName::ESTABLISHED));
    objCall.GetPendingOperationHolder().PushPendingOperation(objPendingOperationReturnEstablished);
    objCall.GetPendingOperationHolder().PushPendingOperation(objPendingOperationReturnEstablished);
    objCall.GetPendingOperationHolder().PushPendingOperation(objPendingOperationReturnEstablished);

    ON_CALL(*pState, GetStateName).WillByDefault(Return(CallStateName::UPDATING));
    objCall.RunPendingOperationIfPossible();
    EXPECT_TRUE(objCall.GetPendingOperationHolder().HasPendingOperation());
}

TEST_F(MtcCallTest, GetTimerReturnsMember)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    const MtcTimerWrapper* pTimer = &objCall.GetTimer();

    EXPECT_NE(pTimer, nullptr);
}

TEST_F(MtcCallTest, GetSupplementaryServiceReturnsMember)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    const MtcSupplementaryService* pSuppService = &objCall.GetSupplementaryService();

    EXPECT_NE(pSuppService, nullptr);
}

TEST_F(MtcCallTest, GetSlotIdCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_CALL(objContext, GetSlotId).Times(1);
    objCall.GetSlotId();
}

TEST_F(MtcCallTest, GetSubscriberConfigCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_CALL(objContext, GetSubscriberConfig).Times(1);
    objCall.GetSubscriberConfig();
}

TEST_F(MtcCallTest, GetDialingPlanCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    MockIMtcDialingPlan objDialingPlan;
    EXPECT_CALL(objContext, GetDialingPlan).Times(1).WillRepeatedly(ReturnRef(objDialingPlan));

    objCall.GetDialingPlan();
}

TEST_F(MtcCallTest, GetServiceByTypeCallsMtcContext)
{
    ServiceType eServiceType = ServiceType::NORMAL;

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_CALL(objContext, GetServiceByType(eServiceType)).Times(1);

    objCall.GetServiceByType(eServiceType);
}

TEST_F(MtcCallTest, GetCallManagerCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    MockIMtcCallManager objCallManager;
    EXPECT_CALL(objContext, GetCallManager).Times(1).WillRepeatedly(ReturnRef(objCallManager));

    objCall.GetCallManager();
}

TEST_F(MtcCallTest, GetRadioCheckerCallsMtcContext)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());

    EXPECT_CALL(objContext, GetRadioChecker).Times(2).WillRepeatedly(ReturnRef(objRadioChecker));

    objCall.GetRadioChecker();
}

TEST_F(MtcCallTest, GetCallControllerCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    MockIMtcCallController objCallController;
    EXPECT_CALL(objContext, GetCallController)
            .Times(1)
            .WillRepeatedly(ReturnRef(objCallController));

    objCall.GetCallController();
}

TEST_F(MtcCallTest, GetConfigurationProxyCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_CALL(objContext, GetConfigurationProxy).Times(1);

    objCall.GetConfigurationProxy();
}

TEST_F(MtcCallTest, GetCallStateProxyCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_CALL(objContext, GetCallStateProxy).Times(1);

    objCall.GetCallStateProxy();
}

TEST_F(MtcCallTest, GetImsEventReceiverCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

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
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_CALL(objContext, GetAosConnector(eServiceType)).Times(1);

    objCall.GetAosConnector(eServiceType);
}

TEST_F(MtcCallTest, GetSipInterfaceFactoryCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    MockIMtcSipInterfaceFactory objSipInterfaceFactory;
    EXPECT_CALL(objContext, GetSipInterfaceFactory)
            .Times(1)
            .WillRepeatedly(ReturnRef(objSipInterfaceFactory));

    objCall.GetSipInterfaceFactory();
}

TEST_F(MtcCallTest, GetConferenceManagerCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    MockIConferenceManager objConferenceManager;
    EXPECT_CALL(objContext, GetConferenceManager)
            .Times(1)
            .WillRepeatedly(ReturnRef(objConferenceManager));

    objCall.GetConferenceManager();
}

TEST_F(MtcCallTest, GetEctManagerCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    MockIEctManager objEctManager;
    EXPECT_CALL(objContext, GetEctManager).Times(1).WillOnce(ReturnRef(objEctManager));

    objCall.GetEctManager();
}

TEST_F(MtcCallTest, GetEmergencyServiceManagerCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    MockIMtcEmergencyServiceManager objEsm;
    EXPECT_CALL(objContext, GetEmergencyServiceManager).Times(1).WillOnce(ReturnRef(objEsm));

    objCall.GetEmergencyServiceManager();
}

TEST_F(MtcCallTest, RunAsyncOperationCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));
    std::function<void()> objAnyOperation;

    EXPECT_CALL(objContext, RunAsyncOperation(this, _)).Times(1);

    objCall.RunAsyncOperation(this, objAnyOperation);
}

TEST_F(MtcCallTest, GetMessageUtilsCallsMtcContext)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    MockIMessageUtils objMessageUtils;
    EXPECT_CALL(objContext, GetMessageUtils).Times(1).WillRepeatedly(ReturnRef(objMessageUtils));

    objCall.GetMessageUtils();
}

TEST_F(MtcCallTest, GetPassiveTimerHolderCallsMtcContext)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    MockIPassiveTimerHolder objPassiveTimerHolder;
    EXPECT_CALL(objContext, GetPassiveTimerHolder)
            .Times(1)
            .WillRepeatedly(ReturnRef(objPassiveTimerHolder));

    objCall.GetPassiveTimerHolder();
}

TEST_F(MtcCallTest, GetMultiEndpointManagerCallsMtcContext)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    EXPECT_CALL(objContext, GetMultiEndpointManager).Times(1);

    objCall.GetMultiEndpointManager();
}

TEST_F(MtcCallTest, GetLastComeFirstServedHelperCallsMtcContext)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    MockILastComeFirstServedHelper objLastComeFirstServedHelper;
    EXPECT_CALL(objContext, GetLastComeFirstServedHelper)
            .WillOnce(ReturnRef(objLastComeFirstServedHelper));

    objCall.GetLastComeFirstServedHelper();
}

TEST_F(MtcCallTest, GetCallConnectionIdManagerCallsMtcContext)
{
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory());
    MockCallConnectionIdManager objCallConnectionIdManager(objContext);
    EXPECT_CALL(objContext, GetCallConnectionIdManager)
            .Times(1)
            .WillRepeatedly(ReturnRef(objCallConnectionIdManager));

    objCall.GetCallConnectionIdManager();
}

TEST_F(MtcCallTest, GetWifiTestModeCallsMtcContext)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_CALL(objContext, IsWifiTestMode).Times(1);

    objCall.IsWifiTestMode();
}

TEST_F(MtcCallTest, SetHeldByMeSetsHeldByMe)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SetHeldByMe(IMS_TRUE);
    EXPECT_EQ(IMS_TRUE, objCall.IsHeldByMe());

    objCall.SetHeldByMe(IMS_FALSE);
    EXPECT_EQ(IMS_FALSE, objCall.IsHeldByMe());
}

TEST_F(MtcCallTest, SessionAlertingCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionAlerting(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionAlerting(&objSession);
}

TEST_F(MtcCallTest, SessionAlertingFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionAlerting(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionAlerting(IMS_NULL);
}

TEST_F(MtcCallTest, SessionReferenceReceivedCallsState)
{
    MockISession objSession;
    MockIReference objReference;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionReferenceReceived(&objSession, &objReference)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionReferenceReceived(&objSession, &objReference);
}

TEST_F(MtcCallTest, SessionReferenceReceivedFailsIfSessionIsNull)
{
    MockIReference objReference;
    EXPECT_CALL(objReference, Reject).Times(1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionReferenceReceived(_, _)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionReferenceReceived(IMS_NULL, &objReference);
}

TEST_F(MtcCallTest, SessionReferenceReceivedFailsIfReferenceIsNull)
{
    MockISession objSession;
    EXPECT_CALL(objSession, Terminate).Times(1);

    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(_, &objSession)).Times(1);
    EXPECT_CALL(*pSessionInterfaceHolder, ReleaseISession(&objSession)).Times(1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionReferenceReceived(_, _)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionReferenceReceived(&objSession, IMS_NULL);
}

TEST_F(MtcCallTest, SessionStartedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionStarted(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionStarted(&objSession);
}

TEST_F(MtcCallTest, SessionStartedCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, UssiStarted(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionStarted(&objSession);
}

TEST_F(MtcCallTest, SessionStartedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionStarted(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionStarted(IMS_NULL);
}

TEST_F(MtcCallTest, SessionStartFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionStartFailed(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionStartFailed(&objSession);
}

TEST_F(MtcCallTest, SessionStartFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionStartFailed(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionStartFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionTerminatedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionTerminated(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionTerminated(&objSession);
}

TEST_F(MtcCallTest, SessionTerminatedCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, UssiTerminated(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionTerminated(&objSession);
}

TEST_F(MtcCallTest, SessionTerminatedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionTerminated(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionTerminated(IMS_NULL);
}

TEST_F(MtcCallTest, SessionUpdatedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdated(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionUpdated(&objSession);
}

TEST_F(MtcCallTest, SessionUpdatedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdated(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionUpdated(IMS_NULL);
}

TEST_F(MtcCallTest, SessionUpdateFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdateFailed(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionUpdateFailed(&objSession);
}

TEST_F(MtcCallTest, SessionUpdateFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdateFailed(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionUpdateFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionUpdateReceivedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdateReceived(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionUpdateReceived(&objSession);
}

TEST_F(MtcCallTest, SessionUpdateReceivedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionUpdateReceived(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionUpdateReceived(IMS_NULL);
}

TEST_F(MtcCallTest, SessionCancelDeliveredCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionCancelDelivered(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionCancelDelivered(&objSession);
}

TEST_F(MtcCallTest, SessionCancelDeliveredFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionCancelDelivered(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionCancelDelivered(IMS_NULL);
}

TEST_F(MtcCallTest, SessionCancelDeliveryFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionCancelDeliveryFailed(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionCancelDeliveryFailed(&objSession);
}

TEST_F(MtcCallTest, SessionCancelDeliveryFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionCancelDeliveryFailed(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionCancelDeliveryFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdatedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdated(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionEarlyMediaUpdated(&objSession);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdatedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdated(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionEarlyMediaUpdated(IMS_NULL);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdateFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdateFailed(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionEarlyMediaUpdateFailed(&objSession);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdateFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdateFailed(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionEarlyMediaUpdateFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdateReceivedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdateReceived(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionEarlyMediaUpdateReceived(&objSession);
}

TEST_F(MtcCallTest, SessionEarlyMediaUpdateReceivedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionEarlyMediaUpdateReceived(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionEarlyMediaUpdateReceived(IMS_NULL);
}

TEST_F(MtcCallTest, SessionForkedResponseReceivedCallsState)
{
    MockISession objSession;
    MockISession objForkedSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionForkedResponseReceived(&objSession, &objForkedSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionForkedResponseReceived(&objSession, &objForkedSession);
}

TEST_F(MtcCallTest, SessionForkedResponseReceivedFailsIfSessionIsNull)
{
    MockISession objForkedSession;
    EXPECT_CALL(objForkedSession, Terminate).Times(1);

    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(_, &objForkedSession)).Times(1);
    EXPECT_CALL(*pSessionInterfaceHolder, ReleaseISession(&objForkedSession)).Times(1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIncoming(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionForkedResponseReceived(IMS_NULL, &objForkedSession);
}

TEST_F(MtcCallTest, SessionForkedResponseReceivedFailsIfForkedSessionIsNull)
{
    MockISession objSession;
    EXPECT_CALL(objSession, Reject()).Times(1);

    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(_, &objSession)).Times(1);
    EXPECT_CALL(*pSessionInterfaceHolder, ReleaseISession(&objSession)).Times(1);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, HandleIncoming(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionForkedResponseReceived(&objSession, IMS_NULL);
}

TEST_F(MtcCallTest, SessionPrackDeliveredCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPrackDelivered(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionPrackDelivered(&objSession);
}

TEST_F(MtcCallTest, SessionPrackDeliveredFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPrackDelivered(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionPrackDelivered(IMS_NULL);
}

TEST_F(MtcCallTest, SessionPrackDeliveryFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPrackDeliveryFailed(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionPrackDeliveryFailed(&objSession);
}

TEST_F(MtcCallTest, SessionPrackDeliveryFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPrackDeliveryFailed(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionPrackDeliveryFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionPrackReceivedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPrackReceived(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionPrackReceived(&objSession);
}

TEST_F(MtcCallTest, SessionPrackReceivedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionPrackReceived(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionPrackReceived(IMS_NULL);
}

TEST_F(MtcCallTest, SessionProvisionalResponseReceivedCallsState)
{
    MockISession objSession;
    IMS_UINT32 nIndex = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionProvisionalResponseReceived(&objSession, nIndex)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionProvisionalResponseReceived(&objSession, nIndex);
}

TEST_F(MtcCallTest, SessionProvisionalResponseReceivedFailsIfSessionIsNull)
{
    IMS_UINT32 nIndex = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionProvisionalResponseReceived(_, _)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionProvisionalResponseReceived(IMS_NULL, nIndex);
}

TEST_F(MtcCallTest, SessionRprDeliveryFailedCallsState)
{
    MockISession objSession;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionRprDeliveryFailed(&objSession)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionRprDeliveryFailed(&objSession);
}

TEST_F(MtcCallTest, SessionRprDeliveryFailedFailsIfSessionIsNull)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionRprDeliveryFailed(_)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionRprDeliveryFailed(IMS_NULL);
}

TEST_F(MtcCallTest, SessionRprReceivedCallsState)
{
    MockISession objSession;
    IMS_UINT32 nIndex = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionRprReceived(&objSession, nIndex)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionRprReceived(&objSession, nIndex);
}

TEST_F(MtcCallTest, SessionRprReceivedFailsIfSessionIsNull)
{
    IMS_UINT32 nIndex = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionRprReceived(_, _)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionRprReceived(IMS_NULL, nIndex);
}

TEST_F(MtcCallTest, SessionTransactionReceivedCallsState)
{
    MockISession objSession;
    MockISipServerConnection objServerConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionTransactionReceived(&objSession, &objServerConnection)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionTransactionReceived(&objSession, &objServerConnection);
}

TEST_F(MtcCallTest, SessionTransactionReceivedCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockISession objSession;
    MockISipServerConnection objServerConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, UssiInfoReceived(&objSession, &objServerConnection)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionTransactionReceived(&objSession, &objServerConnection);
}

TEST_F(MtcCallTest, SessionTransactionReceivedFailsIfSessionIsNull)
{
    MockISipServerConnection objServerConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, SessionTransactionReceived(_, _)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.SessionTransactionReceived(IMS_NULL, &objServerConnection);
}

TEST_F(MtcCallTest, OnRefreshCompletedCallsState)
{
    MockISipClientConnection objConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Refresh_NotifyCompleted(&objConnection)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Refresh_NotifyCompleted(&objConnection);
}

TEST_F(MtcCallTest, OnRefreshTerminatedCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Refresh_NotifyTerminated()).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Refresh_NotifyTerminated();
}

TEST_F(MtcCallTest, OnRefreshTimerExpiredCallsState)
{
    IMS_BOOL bDoImplicitRefresh = IMS_FALSE;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Refresh_NotifyTimerExpired(bDoImplicitRefresh)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Refresh_NotifyTimerExpired(bDoImplicitRefresh);
}

TEST_F(MtcCallTest, OnTimerExpiredCallsState)
{
    IMS_SINT32 nType = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnTimerExpired(nType)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnTimerExpired(nType);
}

TEST_F(MtcCallTest, OnBlockCheckedCallsState)
{
    IMtcBlockChecker::Result objResult(IMtcBlockChecker::Result::Status::UNBLOCKED);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnBlockChecked(objResult)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnBlockChecked(objResult);
}

TEST_F(MtcCallTest, QosReservedCallsState)
{
    MockISession objSession;
    IMS_UINT32 eMediaType = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, QosReserved(&objSession, eMediaType)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.QosReserved(&objSession, eMediaType);
}

TEST_F(MtcCallTest, QosReservedFailsIfSessionIsNull)
{
    IMS_UINT32 eMediaType = 0;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, QosReserved(_, _)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.QosReserved(IMS_NULL, eMediaType);
}

TEST_F(MtcCallTest, QosReserveFailedCallsState)
{
    MockISession objSession;
    QosLossPolicy eNextAction = QosLossPolicy::MAINTAIN;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, QosReserveFailed(&objSession, eNextAction)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.QosReserveFailed(&objSession, eNextAction);
}

TEST_F(MtcCallTest, QosReserveFailedFailsIfSessionIsNull)
{
    QosLossPolicy eNextAction = QosLossPolicy::MAINTAIN;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, QosReserveFailed(_, _)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.QosReserveFailed(IMS_NULL, eNextAction);
}

TEST_F(MtcCallTest, OnStateTransitionUpdatesCallStateProxy)
{
    CallStateName eState = CallStateName::INCOMING;

    MockIMtcCallState* pState = new MockIMtcCallState();
    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    EXPECT_CALL(objCallStateProxy,
            UpdateCallState(
                    objCall.GetKey(), eState, objCall.GetCallType(), objCallInfo.IsEmergency(), _))
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

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.ClientConnection_NotifyResponse(&objConnection, &objForkedConnection);
}

TEST_F(MtcCallTest, ClientConnection_NotifyResponseCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockISipClientConnection objConnection;
    MockISipClientConnection objForkedConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, NotifyResponseToUssiInfo(&objConnection, &objForkedConnection)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.ClientConnection_NotifyResponse(&objConnection, &objForkedConnection);
}

TEST_F(MtcCallTest, ClientConnection_NotifyResponseFailsIfConnectionIsNull)
{
    MockISipClientConnection objForkedConnection;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, ClientConnection_NotifyResponse(_, _)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.ClientConnection_NotifyResponse(IMS_NULL, &objForkedConnection);
}

TEST_F(MtcCallTest, Error_NotifyErrorCallsState)
{
    MockISipConnection objConnection;
    IMS_SINT32 nCode = 1;
    AString strMessage("some_message");

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Error_NotifyError(&objConnection, nCode, strMessage)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Error_NotifyError(&objConnection, nCode, strMessage);
}

TEST_F(MtcCallTest, Error_NotifyErrorCallsStateForUssi)
{
    objCallInfo.bUssi = IMS_TRUE;

    MockISipConnection objConnection;
    IMS_SINT32 nCode = 1;
    AString strMessage("some_message");

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, NotifyErrorToUssiInfo(&objConnection, nCode, strMessage)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Error_NotifyError(&objConnection, nCode, strMessage);
}

TEST_F(MtcCallTest, Error_NotifyErrorFailsIfConnectionIsNull)
{
    IMS_SINT32 nCode = 1;
    AString strMessage("some_message");

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, Error_NotifyError(_, _, _)).Times(0);
    EXPECT_CALL(*pState, OnInternalFailure).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.Error_NotifyError(IMS_NULL, nCode, strMessage);
}

TEST_F(MtcCallTest, OnReceivingMediaDataStartedCallsState)
{
    IMS_UINT32 eMediaType = 1;
    IMS_UINT32 eProtocolType = 2;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnReceivingMediaDataStarted(eMediaType, eProtocolType)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnReceivingMediaDataStarted(eMediaType, eProtocolType);
}

TEST_F(MtcCallTest, OnReceivingMediaDataFailedCallsState)
{
    IMS_UINT32 eMediaType = 1;
    IMS_UINT32 eProtocolType = 2;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnReceivingMediaDataFailed(eMediaType, eProtocolType)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnReceivingMediaDataFailed(eMediaType, eProtocolType);
}

TEST_F(MtcCallTest, OnVideoLowestBitRateCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnVideoLowestBitRate).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnVideoLowestBitRate();
}

TEST_F(MtcCallTest, OnReceivingNetworkToneStartedCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnReceivingNetworkToneStarted).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnReceivingNetworkToneStarted();
}

TEST_F(MtcCallTest, OnReceivingNetworkToneFailedCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnReceivingNetworkToneFailed).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnReceivingNetworkToneFailed();
}

TEST_F(MtcCallTest, OnMediaFailedCallsState)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnMediaFailed(objReason)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnMediaFailed(objReason);
}

TEST_F(MtcCallTest, OnSrvccStateUpdatedCallsState)
{
    SrvccState eAnyState = SrvccState::STARTED;

    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnSrvccStateUpdated(eAnyState)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnSrvccStateUpdated(eAnyState);
}

TEST_F(MtcCallTest, OnAosStateChangedCallsState)
{
    MtcAosState eAnyState = MtcAosState::CONNECTED;
    IMS_UINT32 eAnyReason = 0;
    IMS_SINT32 nDataFailureReason = 0;
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnAosStateChanged(eAnyState, eAnyReason, nDataFailureReason)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnAosStateChanged(objService, eAnyState, eAnyReason, nDataFailureReason);
}

TEST_F(MtcCallTest, OnRatChangedCallsState)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnIpcanChanged(IIpcan::CATEGORY_WLAN)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnRatChanged(objService.GetServiceType(), INetworkWatcher::RADIOTECH_TYPE_IWLAN,
            INetworkWatcher::RADIOTECH_TYPE_IWLAN);
}

TEST_F(MtcCallTest, OnRatChangedCallsStateOnRatChangedIfNoIpcanChanged)
{
    IMS_SINT32 eOldRat = INetworkWatcher::RADIOTECH_TYPE_NR;
    IMS_SINT32 eNewRat = INetworkWatcher::RADIOTECH_TYPE_LTE;
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnRatChanged(eOldRat, eNewRat)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnRatChanged(objService.GetServiceType(), eOldRat, eNewRat);
}

TEST_F(MtcCallTest, OnConnectionFailedCallsState)
{
    IMS_UINT32 nFailureReason = 0;
    IMS_UINT32 nWaitTimeMillis = 1;
    MockIMtcCallState* pState = new MockIMtcCallState();
    EXPECT_CALL(*pState, OnConnectionFailed(nFailureReason, nWaitTimeMillis)).Times(1);

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    objCall.OnConnectionFailed(nFailureReason, nWaitTimeMillis);
}

TEST_F(MtcCallTest, ToString)
{
    MockIMtcCallState* pState = new MockIMtcCallState();
    ON_CALL(*pState, GetStateName).WillByDefault(Return(CallStateName::TERMINATING));

    MtcCall objCall(objContext, objService, objCallInfo, CreateStateFactory(pState));

    AString strCall;
    strCall.Sprintf("MtcCall[%lu][TERMINATING]", objCall.GetKey());

    EXPECT_EQ(objCall.ToString(), strCall);
}
