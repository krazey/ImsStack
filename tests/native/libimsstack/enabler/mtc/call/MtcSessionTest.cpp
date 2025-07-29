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
#include "ISipHeader.h"
#include "ImsAosParameter.h"
#include "MediaDef.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MtcSession.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/message/MockIMessageSender.h"
#include "configuration/ConfigDef.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSessionInterfaceHolder.h"
#include "media/IMtcMediaManager.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "utility/MessageUtil.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL const CallKey CALL_KEY = 1;

class MtcSessionTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcCallManager objCallManager;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MockIMtcPreconditionManager objPreconditionManager;
    MockIMtcMediaManager objMediaManager;
    MockISession objSession;
    MockIMessage objMessage;
    MockIMessageSender* pMessageSender;
    MockSessionInterfaceHolder* pSessionInterfaceHolder;
    MockIMtcSipInterfaceFactory objSipInterfaceFactory;
    MockIMtcService objMtcService;
    MockIMessageUtils objMessageUtils;
    MockIMtcAosConnector objAosConnector;
    MockIMtcCall objThisCall;
    CallInfo objCallInfo;
    MtcSession* pMtcSession;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objPreconditionManager));
        ON_CALL(objContext, GetCallKey).WillByDefault(Return(CALL_KEY));

        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        // To increase coverage
        ON_CALL(*pConfigurationProxy,
                GetBoolean(ConfigVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(*pConfigurationProxy,
                GetInt(ConfigVoice::KEY_SESSION_REFRESH_TRIGGER_INTERVAL_SEC_INT))
                .WillByDefault(Return(100));
        ON_CALL(*pConfigurationProxy,
                GetBoolean(
                        ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
                .WillByDefault(Return(IMS_TRUE));

        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        pSessionInterfaceHolder = new MockSessionInterfaceHolder();
        ON_CALL(objSipInterfaceFactory, GetISessionHolder)
                .WillByDefault(ReturnRef(*pSessionInterfaceHolder));
        ON_CALL(objContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(objSipInterfaceFactory));

        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objMtcService));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objMessageUtils, AddValueIfNotExists(&objMessage, _, _, _))
                .WillByDefault(Return(IMS_SUCCESS));

        ON_CALL(objContext, GetAosConnector(_)).WillByDefault(Return(&objAosConnector));

        ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));
        ON_CALL(objSession, GetNextResponse).WillByDefault(Return(&objMessage));

        ON_CALL(objThisCall, GetState).WillByDefault(Return(IMtcCall::State::IDLE));
        ON_CALL(objContext, GetCall).WillByDefault(ReturnRef(objThisCall));

        pMessageSender = new MockIMessageSender();
        pMtcSession = IMS_NULL;
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pMtcSession;
        delete pSessionInterfaceHolder;
    }

    void CreateMtcSession()
    {
        CreateMtcSession(CallType::VOIP, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    }

    void CreateMtcSession(IN CallType eCallType, IN PeerType ePeerType, IN IMS_BOOL bRegAudio,
            IN IMS_BOOL bRegVideo, IN IMS_BOOL bRegText)
    {
        IMS_UINT32 nFeatures = 0x00000000;
        nFeatures |= bRegAudio ? ImsAosFeature::MMTEL : 0;
        nFeatures |= bRegVideo ? ImsAosFeature::VIDEO : 0;
        nFeatures |= bRegText ? ImsAosFeature::TEXT : 0;
        ON_CALL(objAosConnector, GetFeatures()).WillByDefault(Return(nFeatures));

        objCallInfo.ePeerType = ePeerType;
        pMtcSession = new MtcSession(objContext, objSession, eCallType, pMessageSender);
        ON_CALL(objContext, GetSession()).WillByDefault(Return(pMtcSession));
    }

    void SetUpForSetSdp(IN NegotiationState eNegoState, IN IMS_RESULT eFormResult)
    {
        ON_CALL(objMediaManager, GetNegotiationState(_)).WillByDefault(Return(eNegoState));
        ON_CALL(objMediaManager, FormSdp(&objSession, _, _)).WillByDefault(Return(eFormResult));
        ON_CALL(objPreconditionManager, FormPreconditionSdp(&objSession, IMS_FALSE))
                .WillByDefault(Return());
    }
};

TEST_F(MtcSessionTest, CreateMtSessionaInvokesAddISessionInSessionHolder)
{
    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(CALL_KEY, &objSession));
    CreateMtcSession(CallType::VOIP, PeerType::MT, IMS_TRUE, IMS_TRUE, IMS_TRUE);
}

TEST_F(MtcSessionTest, DestructorDestroysMediaProfileAndQosInfo)
{
    CreateMtcSession(CallType::VOIP, PeerType::MT, IMS_TRUE, IMS_TRUE, IMS_TRUE);

    EXPECT_CALL(objMediaManager, DestroyMediaProfile(_));
    EXPECT_CALL(objPreconditionManager, DestroyQos(_));
}

TEST_F(MtcSessionTest, StartInvokesStartInMessageSender)
{
    IMS_RESULT eResult = IMS_SUCCESS;
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);
    EXPECT_CALL(objPreconditionManager, OnSdpSent(&objSession, IMS_TRUE)).Times(1);
    EXPECT_CALL(*pMessageSender, Start(CallType::VOIP)).WillOnce(Return(eResult));
    CreateMtcSession();
    EXPECT_EQ(pMtcSession->Start(), eResult);
}

TEST_F(MtcSessionTest, StartFailsIfSetSdpFails)
{
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_FAILURE);
    EXPECT_CALL(objPreconditionManager, OnSdpSent(&objSession, IMS_TRUE)).Times(0);
    EXPECT_CALL(*pMessageSender, Start(CallType::VOIP)).Times(0);
    CreateMtcSession();
    EXPECT_EQ(pMtcSession->Start(), IMS_FAILURE);
}

TEST_F(MtcSessionTest, SendProvisionalResponseSends183ReliablyWithoutSdpIfNoSdpIsNeeded)
{
    CreateMtcSession();

    SetUpForSetSdp(NegotiationState::STATE_OFFER_SENT, IMS_SUCCESS);
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCalls));

    EXPECT_CALL(*pMessageSender,
            SendProvisionalResponse(SipStatusCode::SC_183, IMS_TRUE, IMS_FALSE, IMS_FALSE))
            .Times(1);

    pMtcSession->SendProvisionalResponse(IMS_FALSE, IMS_TRUE);
}

TEST_F(MtcSessionTest, SendProvisionalResponseSends183ReliablyWithSdp)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCalls));

    EXPECT_CALL(objPreconditionManager, OnSdpSent(&objSession, IMS_FALSE)).Times(1);
    EXPECT_CALL(*pMessageSender,
            SendProvisionalResponse(SipStatusCode::SC_183, IMS_TRUE, IMS_TRUE, IMS_FALSE))
            .Times(1);

    pMtcSession->SendProvisionalResponse(IMS_FALSE, IMS_TRUE);
}

TEST_F(MtcSessionTest, SendProvisionalResponseSends183WithAlertInfoIfUpdatingSessionExists)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);
    ImsList<IMtcCall*> objCalls;
    MockIMtcCall objOtherCall1;
    ON_CALL(objOtherCall1, GetState).WillByDefault(Return(IMtcCall::State::INCOMING));
    objCalls.Append(&objOtherCall1);
    MockIMtcCall objOtherCall2;
    ON_CALL(objOtherCall2, GetState).WillByDefault(Return(IMtcCall::State::UPDATING));
    objCalls.Append(&objOtherCall2);
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCalls));

    EXPECT_CALL(*pMessageSender,
            SendProvisionalResponse(SipStatusCode::SC_183, IMS_TRUE, IMS_TRUE, IMS_TRUE))
            .Times(1);

    pMtcSession->SendProvisionalResponse(IMS_FALSE, IMS_TRUE);
}

TEST_F(MtcSessionTest, SendProvisionalResponseSends183WithoutAlertInfoIfItIsConfirmedDialog)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);
    ImsList<IMtcCall*> objCalls;
    MockIMtcCall objOtherCall;
    ON_CALL(objOtherCall, GetState).WillByDefault(Return(IMtcCall::State::UPDATING));
    objCalls.Append(&objOtherCall);
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCalls));

    ON_CALL(objThisCall, GetState).WillByDefault(Return(IMtcCall::State::UPDATING));

    EXPECT_CALL(objPreconditionManager, OnSdpSent(&objSession, IMS_FALSE)).Times(1);
    EXPECT_CALL(*pMessageSender,
            SendProvisionalResponse(SipStatusCode::SC_183, IMS_TRUE, IMS_TRUE, IMS_FALSE))
            .Times(1);

    pMtcSession->SendProvisionalResponse(IMS_FALSE, IMS_TRUE);
}

TEST_F(MtcSessionTest, SendProvisionalResponseFailsIfResultSetSdpIsFailure)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_FAILURE);

    EXPECT_CALL(*pMessageSender, SendProvisionalResponse(_, _, _, _)).Times(0);

    pMtcSession->SendProvisionalResponse(IMS_FALSE, IMS_TRUE);
}

TEST_F(MtcSessionTest, SendProvisionalResponseSends180WithOutSdpIfUserAlertAndRprNotSupported)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_OFFER_SENT, IMS_SUCCESS);
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCalls));

    EXPECT_CALL(objMediaManager, FormSdp(_, _, _)).Times(0);
    EXPECT_CALL(*pMessageSender,
            SendProvisionalResponse(SipStatusCode::SC_180, IMS_FALSE, IMS_FALSE, IMS_FALSE))
            .Times(1);

    pMtcSession->SendProvisionalResponse(IMS_TRUE, IMS_FALSE);
}

TEST_F(MtcSessionTest,
        SendProvisionalResponseSends180WithOutSdpIfUserAlertAndRprSupportedAndNoSdpIsNeeded)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_OFFER_SENT, IMS_SUCCESS);
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCalls));

    EXPECT_CALL(objMediaManager, FormSdp(_, _, _)).Times(0);
    EXPECT_CALL(*pMessageSender,
            SendProvisionalResponse(SipStatusCode::SC_180, IMS_TRUE, IMS_FALSE, IMS_FALSE))
            .Times(1);

    pMtcSession->SendProvisionalResponse(IMS_TRUE, IMS_TRUE);
}

TEST_F(MtcSessionTest, SendProvisionalResponseSends180WithSdpIfUserAlertAndRprSupported)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCalls));

    EXPECT_CALL(*pMessageSender,
            SendProvisionalResponse(SipStatusCode::SC_180, IMS_TRUE, IMS_TRUE, IMS_FALSE))
            .Times(1);

    pMtcSession->SendProvisionalResponse(IMS_TRUE, IMS_TRUE);
}

TEST_F(MtcSessionTest, SendPrackSendsPrack)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);
    EXPECT_CALL(*pMessageSender, SendPrack).Times(1);

    pMtcSession->SendPrack(IMS_FALSE);
}

TEST_F(MtcSessionTest, SendPrackFailsIfSetSdpFails)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_FAILURE);

    EXPECT_CALL(*pMessageSender, SendPrack).Times(0);

    pMtcSession->SendPrack(IMS_FALSE);
}

TEST_F(MtcSessionTest, SendPrackSendsPrackWithoutReOfferIfSdpOfferIsSent)
{
    CreateMtcSession();
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_ALLOW_SDP_IN_PRACK_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    EXPECT_CALL(objMediaManager, FormSdp(_, _, _)).Times(0);
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp(_, _)).Times(0);
    EXPECT_CALL(*pMessageSender, SendPrack).Times(1);

    pMtcSession->SendPrack(IMS_TRUE);
}

TEST_F(MtcSessionTest, SendPrackSendsPrackWithReOffer)
{
    CreateMtcSession();
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_ALLOW_SDP_IN_PRACK_BOOL))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMediaManager, FormSdp(&objSession, CallType::VOIP, IMS_FALSE))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp(&objSession, IMS_FALSE)).Times(1);
    EXPECT_CALL(*pMessageSender, SendPrack).Times(1);

    pMtcSession->SendPrack(IMS_TRUE);
}

TEST_F(MtcSessionTest, SendPrackSendsPrackWithoutReOfferEvenIfReOfferIsRequiredIfSdpIsNotAllowed)
{
    CreateMtcSession();
    ON_CALL(*pConfigurationProxy, GetBoolean(ConfigVoice::KEY_ALLOW_SDP_IN_PRACK_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMediaManager, FormSdp(_, _, _)).Times(0);
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp(_, _)).Times(0);
    EXPECT_CALL(*pMessageSender, SendPrack).Times(1);

    pMtcSession->SendPrack(IMS_TRUE);
}

TEST_F(MtcSessionTest, SendPrackSendsPrackWithoutReOffer)
{
    CreateMtcSession();
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMediaManager, FormSdp(&objSession, CallType::VOIP, IMS_FALSE)).Times(0);
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp(&objSession, IMS_FALSE)).Times(0);
    EXPECT_CALL(*pMessageSender, SendPrack).Times(1);

    pMtcSession->SendPrack(IMS_FALSE);
}

TEST_F(MtcSessionTest, RespondToPrackRespondsToPrack)
{
    IMS_SINT32 eAnyStatusCode = 200;
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);

    EXPECT_CALL(*pMessageSender, RespondToPrack(eAnyStatusCode)).Times(1);

    pMtcSession->RespondToPrack(eAnyStatusCode);
}

TEST_F(MtcSessionTest, RespondToPrackFailsIfSetSdpFails)
{
    IMS_SINT32 eAnyStatusCode = 200;
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_FAILURE);

    EXPECT_CALL(*pMessageSender, RespondToPrack(eAnyStatusCode)).Times(0);

    pMtcSession->RespondToPrack(eAnyStatusCode);
}

TEST_F(MtcSessionTest, SendEarlyUpdateSendsEarlyUpdate)
{
    UpdateType eAnyType = UpdateType::SESSION;
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);

    EXPECT_CALL(*pMessageSender, SendEarlyUpdate(eAnyType)).Times(1);

    pMtcSession->SendEarlyUpdate(eAnyType);
}

TEST_F(MtcSessionTest, SendEarlyUpdateFailsIfSetSdpFails)
{
    UpdateType eAnyType = UpdateType::SESSION;
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_FAILURE);

    EXPECT_CALL(*pMessageSender, SendEarlyUpdate(eAnyType)).Times(0);

    pMtcSession->SendEarlyUpdate(eAnyType);
}

TEST_F(MtcSessionTest, GetOngoingUpdateTypeInitiallyReturnsNone)
{
    CreateMtcSession();
    EXPECT_EQ(pMtcSession->GetOngoingUpdateType(), UpdateType::NONE);
}

TEST_F(MtcSessionTest, GetOngoingUpdateTypeReturnsTypeOfSendEarlyUpdate)
{
    UpdateType eSomeType = UpdateType::SESSION;
    CreateMtcSession();

    // If SetSdpToSend() is failed, UpdateType is not updated.
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);

    pMtcSession->SendEarlyUpdate(eSomeType);

    EXPECT_EQ(pMtcSession->GetOngoingUpdateType(), eSomeType);
}

TEST_F(MtcSessionTest, RespondToEarlyUpdateRespondsToEarlyUpdate)
{
    IMS_SINT32 eAnyStatusCode = 200;
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);

    EXPECT_CALL(*pMessageSender, RespondToEarlyUpdate(eAnyStatusCode)).Times(1);

    pMtcSession->RespondToEarlyUpdate(eAnyStatusCode);
}

TEST_F(MtcSessionTest, RespondToEarlyUpdateFailsIfSetSdpFails)
{
    IMS_SINT32 eAnyStatusCode = 200;
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_FAILURE);

    EXPECT_CALL(*pMessageSender, RespondToEarlyUpdate(eAnyStatusCode)).Times(0);

    pMtcSession->RespondToEarlyUpdate(eAnyStatusCode);
}

TEST_F(MtcSessionTest, SendAckSendsAck)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_NEGOTIATED, IMS_SUCCESS);

    EXPECT_CALL(*pMessageSender, SendAck).Times(1);

    pMtcSession->SendAck();
}

TEST_F(MtcSessionTest, SendAckFailsIfSetSdpFails)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_OFFER_RECEIVED, IMS_FAILURE);

    EXPECT_CALL(*pMessageSender, SendAck).Times(0);

    pMtcSession->SendAck();
}

TEST_F(MtcSessionTest, AcceptAccepts)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_NEGOTIATED, IMS_SUCCESS);

    EXPECT_CALL(*pMessageSender, Accept).Times(1);

    pMtcSession->Accept();
}

TEST_F(MtcSessionTest, AcceptFailsIfSetSdpFails)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_OFFER_RECEIVED, IMS_FAILURE);

    EXPECT_CALL(*pMessageSender, Accept).Times(0);

    pMtcSession->Accept();
}

TEST_F(MtcSessionTest, RejectRejects)
{
    const CallReasonInfo objReason(CODE_LOCAL_CALL_BUSY);
    CreateMtcSession();

    EXPECT_CALL(*pMessageSender, Reject(objReason)).Times(1);

    pMtcSession->Reject(objReason);
}

TEST_F(MtcSessionTest, UpdateUpdates)
{
    UpdateType eAnyType = UpdateType::SESSION;
    IMS_BOOL bAlertInfo = IMS_TRUE;
    IMS_SINT32 eMethod = SipMethod::INVITE;

    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_NEGOTIATED, IMS_SUCCESS);

    EXPECT_CALL(objMediaManager, FormSdp(_, _, _)).Times(1);
    EXPECT_CALL(*pMessageSender, Update(eAnyType, bAlertInfo, eMethod, IMS_FALSE)).Times(1);

    pMtcSession->Update(eAnyType, bAlertInfo, eMethod);
}

TEST_F(MtcSessionTest, UpdateUpdatesForRefresh)
{
    UpdateType eAnyType = UpdateType::REFRESH;
    IMS_BOOL bAlertInfo = IMS_FALSE;
    IMS_SINT32 eMethod = SipMethod::UPDATE;

    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_NEGOTIATED, IMS_SUCCESS);

    EXPECT_CALL(objMediaManager, FormSdp(_, _, _)).Times(1);
    EXPECT_CALL(*pMessageSender, Update(eAnyType, bAlertInfo, eMethod, IMS_TRUE)).Times(1);

    pMtcSession->Update(eAnyType, bAlertInfo, eMethod);
}

TEST_F(MtcSessionTest, UpdateFailsIfSetSdpFails)
{
    UpdateType eAnyType = UpdateType::REFRESH;
    IMS_BOOL bAlertInfo = IMS_FALSE;
    IMS_SINT32 eMethod = SipMethod::UPDATE;

    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_OFFER_RECEIVED, IMS_FAILURE);

    EXPECT_CALL(objMediaManager, FormSdp(_, _, _)).Times(1);
    EXPECT_CALL(*pMessageSender, Update(_, _, _, _)).Times(0);

    pMtcSession->Update(eAnyType, bAlertInfo, eMethod);
}

TEST_F(MtcSessionTest, UpdateUpdatesForLocation)
{
    UpdateType eAnyType = UpdateType::LOCATION;
    IMS_BOOL bAlertInfo = IMS_FALSE;
    IMS_SINT32 eMethod = SipMethod::UPDATE;

    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_NEGOTIATED, IMS_SUCCESS);

    EXPECT_CALL(objMediaManager, FormSdp(_, _, _)).Times(0);
    EXPECT_CALL(*pMessageSender, Update(_, _, _, _)).Times(1);

    pMtcSession->Update(eAnyType, bAlertInfo, eMethod);
}

TEST_F(MtcSessionTest, AcceptUpdateAcceptsUpdate)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_NEGOTIATED, IMS_SUCCESS);

    SipMethod objSipMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(objSession, GetPreviousRequest).WillByDefault(Return(&objMessage));

    EXPECT_CALL(objMediaManager, FormSdp(_, _, _));
    EXPECT_CALL(*pMessageSender, AcceptUpdate).Times(1);

    pMtcSession->AcceptUpdate();
}

TEST_F(MtcSessionTest, AcceptUpdateDoesNotSetSdpIfMethodIsUpdate)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_NEGOTIATED, IMS_SUCCESS);

    SipMethod objSipMethod(SipMethod::UPDATE);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(objSession, GetPreviousRequest).WillByDefault(Return(&objMessage));

    EXPECT_CALL(objMediaManager, FormSdp(_, _, _)).Times(0);
    EXPECT_CALL(*pMessageSender, AcceptUpdate).Times(1);

    pMtcSession->AcceptUpdate();
}

TEST_F(MtcSessionTest, AcceptUpdateReturnsFailureIfSetSdpFails)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_NEGOTIATED, IMS_FAILURE);

    SipMethod objSipMethod(SipMethod::INVITE);
    ON_CALL(objMessage, GetMethod()).WillByDefault(ReturnRef(objSipMethod));
    ON_CALL(objSession, GetPreviousRequest).WillByDefault(Return(&objMessage));

    EXPECT_CALL(*pMessageSender, AcceptUpdate).Times(0);

    EXPECT_EQ(pMtcSession->AcceptUpdate(), IMS_FAILURE);
}

TEST_F(MtcSessionTest, CancelUpdateCancelsUpdate)
{
    const CallReasonInfo objReason(CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE);
    CreateMtcSession();

    EXPECT_CALL(*pMessageSender, CancelUpdate(objReason)).Times(1);

    pMtcSession->CancelUpdate(objReason);
}

TEST_F(MtcSessionTest, TerminateWithReasonVccDoesNotInvokeTerminate)
{
    CreateMtcSession();
    const CallReasonInfo objReason(CODE_LOCAL_CALL_VCC_ON_PROGRESSING);
    EXPECT_CALL(*pMessageSender, Terminate(_, _)).Times(0);

    pMtcSession->Terminate(IMS_TRUE, objReason);
}

TEST_F(MtcSessionTest, TerminateWithReasonLocalServiceUnavailableInvokesTerminate)
{
    CreateMtcSession();
    const CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    EXPECT_CALL(*pMessageSender, Terminate(IMS_TRUE, objReason)).Times(1);
    pMtcSession->Terminate(IMS_TRUE, objReason);
}

TEST_F(MtcSessionTest, TerminateWithReasonUserTerminatedInvokesTerminate)
{
    CreateMtcSession();
    const CallReasonInfo objReason(CODE_USER_TERMINATED);
    EXPECT_CALL(*pMessageSender, Terminate(IMS_TRUE, objReason)).Times(1);

    pMtcSession->Terminate(IMS_TRUE, objReason);
}

TEST_F(MtcSessionTest, SecondTerminateReturnsFailure)
{
    CreateMtcSession();
    const CallReasonInfo objReason(CODE_USER_TERMINATED);
    EXPECT_CALL(*pMessageSender, Terminate(_, _)).Times(1);

    EXPECT_EQ(IMS_SUCCESS, pMtcSession->Terminate(IMS_TRUE, objReason));
    EXPECT_EQ(IMS_FAILURE, pMtcSession->Terminate(IMS_TRUE, objReason));
}

TEST_F(MtcSessionTest, GetExtensionSetReturnsMember)
{
    CreateMtcSession();
    const MtcExtensionSet& objExtensionSet = pMtcSession->GetExtensionSet();
    EXPECT_NE(&objExtensionSet, nullptr);
}

TEST_F(MtcSessionTest,
        HandleStartRequestUpdatesCallTypeAndCapabilityIfVideoTextInRegAndVideoTextInMessage)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::UNKNOWN, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
    EXPECT_TRUE(pMtcSession->IsVideoCapable());
    EXPECT_TRUE(pMtcSession->IsRttCapable());
}

TEST_F(MtcSessionTest,
        HandleStartRequestUpdatesCallTypeAndCapabilityIfVideoTextInRegAndVideoTextNotInMessage)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::UNKNOWN, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
    EXPECT_FALSE(pMtcSession->IsVideoCapable());
    EXPECT_FALSE(pMtcSession->IsRttCapable());
}

TEST_F(MtcSessionTest,
        HandleStartRequestUpdatesCallTypeAndAndCapabilityIfOnlyVideoInRegAndOnlyTextInMessage)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_FALSE, IMS_TRUE, IMS_FALSE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::UNKNOWN, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
    EXPECT_FALSE(pMtcSession->IsVideoCapable());
    EXPECT_FALSE(pMtcSession->IsRttCapable());
}

TEST_F(MtcSessionTest, HandleStartRequestDoesNotUpdatesCapabilityTrueIfInvalidContactHeader)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_TRUE, IMS_FALSE, IMS_FALSE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage))
            .WillByDefault(Return(std::nullopt));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage))
            .WillByDefault(Return(std::nullopt));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_FALSE(pMtcSession->IsVideoCapable());
    EXPECT_FALSE(pMtcSession->IsRttCapable());
}

TEST_F(MtcSessionTest, HandleStartRequestDoesNotUpdatesCapabilityFalseIfInvalidContactHeader)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage))
            .WillByDefault(Return(std::nullopt));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage))
            .WillByDefault(Return(std::nullopt));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_TRUE(pMtcSession->IsVideoCapable());
    EXPECT_TRUE(pMtcSession->IsRttCapable());
}

TEST_F(MtcSessionTest, IncomingRttRequestIsRestrictedByRegisteredFeatureIfTextNotInReg)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_FALSE, IMS_TRUE, IMS_FALSE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::RTT));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::UNKNOWN, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VOIP, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleStartRequestWithoutSdpSetsCallTypeVoipIfConfigSet)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_INVITE_INT))
            .WillByDefault(Return(ConfigVoice::OFFERLESS_INVITE_MEDIA_TYPE_AUDIO));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VOIP, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleStartRequestWithoutSdpSetsCallTypeVoipIfVoiceOnlyRegistered)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_TRUE, IMS_FALSE, IMS_FALSE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_INVITE_INT))
            .WillByDefault(Return(ConfigVoice::OFFERLESS_INVITE_MEDIA_TYPE_FULL_CAPABILITY));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VOIP, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleStartRequestWithoutSdpSetsCallTypeVtIfVideoRegistered)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_TRUE, IMS_TRUE, IMS_FALSE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_INVITE_INT))
            .WillByDefault(Return(ConfigVoice::OFFERLESS_INVITE_MEDIA_TYPE_FULL_CAPABILITY));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleStartRequestWithoutSdpSetsCallTypeRttIfTextRegistered)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_TRUE, IMS_FALSE, IMS_TRUE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_INVITE_INT))
            .WillByDefault(Return(ConfigVoice::OFFERLESS_INVITE_MEDIA_TYPE_FULL_CAPABILITY));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::RTT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleRequestWithPrackUpdatesCallType)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::PRACK;

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VT, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VOIP, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleRequestWithAckUpdatesCallType)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::ACK;

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VT, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VOIP, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleRequestUpdatesVideoCapabilityByAvchange)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::EARLY_UPDATE;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_TRUE));

    // avchange case
    ON_CALL(objMessageUtils, GetHeader(&objMessage, ISipHeader::UNKNOWN, strHeader))
            .WillByDefault(Return(MessageUtil::STR_AVCHANGE));
    pMtcSession->HandleRequest(eType, objMessage);
    EXPECT_TRUE(pMtcSession->IsVideoCapable());

    // no avchange case
    ON_CALL(objMessageUtils, GetHeader(&objMessage, ISipHeader::UNKNOWN, strHeader))
            .WillByDefault(Return(""));
    pMtcSession->HandleRequest(eType, objMessage);
    EXPECT_FALSE(pMtcSession->IsVideoCapable());
}

TEST_F(MtcSessionTest, HandleEarlyUpdateRequestDoesNotInvokeSetCallTypeIfSameCallType)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::EARLY_UPDATE;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(CallType::UNKNOWN, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());

    pMtcSession->HandleRequest(eType, objMessage);

    // previous call type is not changed.
    EXPECT_EQ(CallType::UNKNOWN, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleRequestInvokesSetCallTypeByRegisteredFeatureAndReturnsVt)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::START;

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_NOT_ALLOWED_IF_ACTIVE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleRequestInvokesSetCallTypeByRegisteredFeatureAndReturnsVideoRtt)
{
    CreateMtcSession(CallType::UNKNOWN, PeerType::MT, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::START;

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_ALLOWED));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VIDEO_RTT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleUpdateRequestWithSdpInvokesSetCallTypeIfSameCallType)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::UPDATE;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(CallType::UNKNOWN, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());

    pMtcSession->HandleRequest(eType, objMessage);

    // previous call type is changed.
    EXPECT_EQ(CallType::VT, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleUpdateRequestWithoutSdpInvokesSetCallTypeByRegisteredFeature)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT))
            .WillByDefault(Return(ConfigVoice::OFFERLESS_REINVITE_MEDIA_TYPE_FULL));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_FALSE, IMS_TRUE);
    pMtcSession->SetCallType(CallType::VIDEO_RTT);
    RequestType eType = RequestType::UPDATE;

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::RTT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleUpdateRequestWithoutSdpInvokesSetCallTypeWithVoip)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT))
            .WillByDefault(Return(ConfigVoice::OFFERLESS_REINVITE_MEDIA_TYPE_AUDIO));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    pMtcSession->SetCallType(CallType::VIDEO_RTT);
    RequestType eType = RequestType::UPDATE;

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VOIP, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleUpdateRequestWithoutSdpInvokesSetCallTypeWithCurrentType)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT))
            .WillByDefault(Return(ConfigVoice::OFFERLESS_REINVITE_MEDIA_TYPE_CURRENT));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    pMtcSession->SetCallType(CallType::RTT);
    pMtcSession->SetCallType(CallType::VIDEO_RTT);
    RequestType eType = RequestType::UPDATE;

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VIDEO_RTT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleUpdateRequestWithoutSdpInvokesSetCallTypeByHistory)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT))
            .WillByDefault(Return(ConfigVoice::OFFERLESS_REINVITE_MEDIA_TYPE_BY_HISTORY));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    pMtcSession->SetCallType(CallType::VIDEO_RTT);
    pMtcSession->SetCallType(CallType::RTT);
    pMtcSession->SetCallType(CallType::VT);
    RequestType eType = RequestType::UPDATE;

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VIDEO_RTT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleUpdateRequestWithoutSdpInvokesSetCallTypeWithInitialOfferedType)
{
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT))
            .WillByDefault(Return(ConfigVoice::OFFERLESS_REINVITE_MEDIA_TYPE_INITIALLY_OFFERED));

    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    pMtcSession->SetCallType(CallType::VOIP);
    RequestType eType = RequestType::UPDATE;

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, HasSdp(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleResponseInvokesSetCallTypeIfDifferentCallType)
{
    CreateMtcSession(CallType::VOIP, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    ResponseType eType = ResponseType::PROVISIONAL_RESPONSE;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(CallType::UNKNOWN, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VOIP, pMtcSession->GetCallType());

    pMtcSession->HandleResponse(eType, objMessage);

    // previous call type is not changed.
    EXPECT_EQ(CallType::VOIP, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleResponseDoesNotInvokeSetCallTypeIfSameCallType)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    ResponseType eType = ResponseType::EARLY_UPDATE_RESPONSE;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(CallType::UNKNOWN, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());

    pMtcSession->HandleResponse(eType, objMessage);

    // previous call type is not changed.
    EXPECT_EQ(CallType::UNKNOWN, pMtcSession->GetPreviousCallType());
    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleResponseSetsInConferenceIfIsFocus)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    ResponseType eType = ResponseType::PROVISIONAL_RESPONSE;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_TRUE));

    EXPECT_FALSE(objCallInfo.bConference);

    pMtcSession->HandleResponse(eType, objMessage);

    EXPECT_TRUE(objCallInfo.bConference);

    // To increase coverage
    pMtcSession->HandleResponse(eType, objMessage);
}

TEST_F(MtcSessionTest, HandleResponseResetsOngoingUpdateTypeIfEarlyUpdateResponse)
{
    // Update m_eOngoingUpdateType
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);
    pMtcSession->SendEarlyUpdate(UpdateType::SESSION);
    EXPECT_EQ(pMtcSession->GetOngoingUpdateType(), UpdateType::SESSION);

    ResponseType eType = ResponseType::EARLY_UPDATE_RESPONSE;

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_200));

    pMtcSession->HandleResponse(eType, objMessage);

    EXPECT_EQ(pMtcSession->GetOngoingUpdateType(), UpdateType::NONE);
}

TEST_F(MtcSessionTest, HandleResponseSetsTerminatedFlagIf199)
{
    // Update m_eOngoingUpdateType
    CreateMtcSession();
    ResponseType eType = ResponseType::EARLY_UPDATE_RESPONSE;

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_199));

    pMtcSession->HandleResponse(eType, objMessage);

    static const IMS_UINT32 ANY_REASON_CODE = 0;
    const CallReasonInfo objCallReasonInfo(ANY_REASON_CODE);

    // To check that m_bTerminated is set to true. If it's true, Terminate() returns failure.
    EXPECT_EQ(pMtcSession->Terminate(IMS_TRUE, objCallReasonInfo), IMS_FAILURE);
}

TEST_F(MtcSessionTest, SetCallTypeUpdatesCurrentCallTypeAndPreviousCallType)
{
    CreateMtcSession(CallType::VOIP, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    EXPECT_EQ(pMtcSession->GetCallType(), CallType::VOIP);
    EXPECT_EQ(pMtcSession->GetPreviousCallType(), CallType::UNKNOWN);

    pMtcSession->SetCallType(CallType::VT);
    EXPECT_EQ(pMtcSession->GetCallType(), CallType::VT);
    EXPECT_EQ(pMtcSession->GetPreviousCallType(), CallType::VOIP);
}

TEST_F(MtcSessionTest, HandleResponseUpdateCallTypeAndCapabilityFromMessage)
{
    CreateMtcSession(CallType::VOIP, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    ResponseType eType = ResponseType::ACCEPT;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));

    pMtcSession->HandleResponse(eType, objMessage);
    EXPECT_TRUE(pMtcSession->IsVideoCapable());

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleResponse(eType, objMessage);
    EXPECT_FALSE(pMtcSession->IsVideoCapable());
}

TEST_F(MtcSessionTest, HandleResponseSkipUpdateCallTypeAndCapabilityFromMessageIfRejectType)
{
    CreateMtcSession(CallType::VOIP, PeerType::MO, IMS_TRUE, IMS_FALSE, IMS_TRUE);
    ResponseType eType = ResponseType::REJECT;

    ON_CALL(*pConfigurationProxy,
            GetBoolean(ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_TTA_VOLTE_INFO))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));

    pMtcSession->HandleResponse(eType, objMessage);

    EXPECT_FALSE(pMtcSession->IsVideoCapable());
}
