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
#include "ImsAosParameter.h"
#include "MediaDef.h"
#include "MockIMtcService.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MtcSession.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/message/MockIMessageSender.h"
#include "configuration/ConfigDef.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSessionInterfaceHolder.h"
#include "media/IMtcMediaManager.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"
#include "sipcore/ISipHeader.h"
#include "sipcore/SipHeaderName.h"
#include "utility/MessageUtil.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class MtcSessionTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcCallManager objCallManager;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockIMtcPreconditionManager objPreconditionManager;
    MockIMtcMediaManager objMediaManager;
    MockISession objSession;
    MockIMessage objMessage;
    MockIMessageSender* pMessageSender;
    MockSessionInterfaceHolder* pSessionInterfaceHolder;
    MockIMtcSipInterfaceFactory objSipInterfaceFactory;
    MockIInterfaceHolderListener objInterfaceHolderListener;
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

        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        pSessionInterfaceHolder = new MockSessionInterfaceHolder(objInterfaceHolderListener);
        ON_CALL(objSipInterfaceFactory, GetISessionHolder)
                .WillByDefault(Return(pSessionInterfaceHolder));
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
    EXPECT_CALL(*pSessionInterfaceHolder, AddISession(&objSession));
    CreateMtcSession(CallType::VOIP, PeerType::MT, IMS_TRUE, IMS_TRUE, IMS_TRUE);
}

TEST_F(MtcSessionTest, StartInvokesStartInMessageSender)
{
    IMS_RESULT eResult = IMS_SUCCESS;
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);
    EXPECT_CALL(*pMessageSender, Start(CallType::VOIP)).WillOnce(Return(eResult));
    CreateMtcSession();
    EXPECT_EQ(pMtcSession->Start(), eResult);
}

TEST_F(MtcSessionTest, StartFailsIfSetSdpFails)
{
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_FAILURE);
    EXPECT_CALL(*pMessageSender, Start(CallType::VOIP)).Times(0);
    CreateMtcSession();
    EXPECT_EQ(pMtcSession->Start(), IMS_FAILURE);
}

TEST_F(MtcSessionTest, SendProvisionalResponseSends183NotReliablyWithoutSdp)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_OFFER_SENT, IMS_SUCCESS);
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCalls));

    EXPECT_CALL(*pMessageSender,
            SendProvisionalResponse(SipStatusCode::SC_183, IMS_FALSE, IMS_FALSE, IMS_FALSE))
            .Times(1);

    pMtcSession->SendProvisionalResponse(IMS_FALSE);
}

TEST_F(MtcSessionTest, SendProvisionalResponseSends183NotReliablyWithSdp)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);
    ImsList<IMtcCall*> objCalls;
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCalls));

    EXPECT_CALL(*pMessageSender,
            SendProvisionalResponse(SipStatusCode::SC_183, IMS_FALSE, IMS_TRUE, IMS_FALSE))
            .Times(1);

    pMtcSession->SendProvisionalResponse(IMS_FALSE);
}

TEST_F(MtcSessionTest, SendProvisionalResponseSends183WithAlertInfoIfUpdatingSessionExists)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_SUCCESS);
    ImsList<IMtcCall*> objCalls;
    MockIMtcCall objOtherCall;
    ON_CALL(objOtherCall, GetState).WillByDefault(Return(IMtcCall::State::UPDATING));
    objCalls.Append(&objOtherCall);
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCalls));

    EXPECT_CALL(*pMessageSender,
            SendProvisionalResponse(SipStatusCode::SC_183, IMS_FALSE, IMS_TRUE, IMS_TRUE))
            .Times(1);

    pMtcSession->SendProvisionalResponse(IMS_FALSE);
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

    EXPECT_CALL(*pMessageSender,
            SendProvisionalResponse(SipStatusCode::SC_183, IMS_FALSE, IMS_TRUE, IMS_FALSE))
            .Times(1);

    pMtcSession->SendProvisionalResponse(IMS_FALSE);
}

TEST_F(MtcSessionTest, SendProvisionalResponseFailsIfResultSetSdpIsFailure)
{
    CreateMtcSession();
    SetUpForSetSdp(NegotiationState::STATE_IDLE, IMS_FAILURE);

    EXPECT_CALL(*pMessageSender, SendProvisionalResponse(_, _, _, _)).Times(0);

    pMtcSession->SendProvisionalResponse(IMS_FALSE);
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
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_OFFER_SENT));
    EXPECT_CALL(objMediaManager, FormSdp(&objSession, CallType::VOIP, IMS_FALSE)).Times(0);
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp(&objSession, IMS_FALSE)).Times(0);
    EXPECT_CALL(*pMessageSender, SendPrack).Times(1);

    pMtcSession->SendPrack(IMS_TRUE);
}

TEST_F(MtcSessionTest, SendPrackSendsPrackWithReOffer)
{
    CreateMtcSession();
    ON_CALL(objMediaManager, GetNegotiationState(&objSession))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));

    EXPECT_CALL(objMediaManager, FormSdp(&objSession, CallType::VOIP, IMS_FALSE))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp(&objSession, IMS_FALSE)).Times(1);
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

    EXPECT_CALL(*pMessageSender, Update(_, _, _, _)).Times(0);

    pMtcSession->Update(eAnyType, bAlertInfo, eMethod);
}

TEST_F(MtcSessionTest, AcceptUpdateAcceptsUpdate)
{
    CreateMtcSession();

    EXPECT_CALL(*pMessageSender, AcceptUpdate).Times(1);

    pMtcSession->AcceptUpdate();
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
    // ON_CALL(*pMessageSender, Terminate(_, _)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_EQ(IMS_SUCCESS, pMtcSession->Terminate(IMS_TRUE, objReason));
    EXPECT_EQ(IMS_FAILURE, pMtcSession->Terminate(IMS_TRUE, objReason));
}

TEST_F(MtcSessionTest, GetExtensionSetReturnsMember)
{
    CreateMtcSession();
    MtcExtensionSet& objExtensionSet = pMtcSession->GetExtensionSet();
    EXPECT_NE(&objExtensionSet, nullptr);
}

TEST_F(MtcSessionTest, HandleStartRequestUpdatesCallType)
{
    CreateMtcSession();
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationManager, IsSupportVideoCallUpgradeRegardlessOfFeatureTags)
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strHeader))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VOIP, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleStartRequestUpdatesCallTypeToVoIpIfMessageDoesNotIncludeVideo)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationManager, IsSupportVideoCallUpgradeRegardlessOfFeatureTags)
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strHeader))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VOIP));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VOIP, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleStartRequestDoesNotChangeCallTypeIfVideoIsOnlyOneRegFeature)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_FALSE, IMS_TRUE, IMS_FALSE);
    RequestType eType = RequestType::START;

    ON_CALL(*pConfigurationManager, IsSupportVideoCallUpgradeRegardlessOfFeatureTags)
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strHeader))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_FALSE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleRequestUpdatesVideoCapabilityByAvchange)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::EARLY_UPDATE;

    ON_CALL(*pConfigurationManager, IsSupportVideoCallUpgradeRegardlessOfFeatureTags)
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strHeader))
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

TEST_F(MtcSessionTest, HandleUpdateRequestInvokesSetCallTypeIfSameCallType)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::UPDATE;

    ON_CALL(*pConfigurationManager, IsSupportVideoCallUpgradeRegardlessOfFeatureTags)
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strHeader))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
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

TEST_F(MtcSessionTest, HandleEarlyUpdateRequestDoesNotInvokeSetCallTypeIfSameCallType)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::EARLY_UPDATE;

    ON_CALL(*pConfigurationManager, IsSupportVideoCallUpgradeRegardlessOfFeatureTags)
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strHeader))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
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
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::START;

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(*pConfigurationManager, GetPolicyForTextWithVideo)
            .WillByDefault(Return(CarrierConfig::ImsVt::TEXT_VIDEO_NOT_ALLOWED_IF_ACTIVE));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleRequestInvokesSetCallTypeByRegisteredFeatureAndReturnsVideoRtt)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    RequestType eType = RequestType::START;

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::UNKNOWN));
    ON_CALL(*pConfigurationManager, GetPolicyForTextWithVideo)
            .WillByDefault(Return(CarrierConfig::ImsVt::TEXT_VIDEO_ALLOWED));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VIDEO_RTT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleRequestInvokesSetCallTypeByHistory)
{
    CreateMtcSession(CallType::VT, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    pMtcSession->SetCallType(CallType::VIDEO_RTT);
    RequestType eType = RequestType::UPDATE;

    ON_CALL(objMessageUtils, IsVideoFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, IsTextFeatureIncluded(&objMessage)).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::UNKNOWN));

    pMtcSession->HandleRequest(eType, objMessage);

    EXPECT_EQ(CallType::VIDEO_RTT, pMtcSession->GetCallType());
}

TEST_F(MtcSessionTest, HandleResponseInvokesSetCallTypeIfDifferentCallType)
{
    CreateMtcSession(CallType::VOIP, PeerType::MO, IMS_TRUE, IMS_TRUE, IMS_TRUE);
    ResponseType eType = ResponseType::PROVISIONAL_RESPONSE;

    ON_CALL(*pConfigurationManager, IsSupportVideoCallUpgradeRegardlessOfFeatureTags)
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strHeader))
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

    ON_CALL(*pConfigurationManager, IsSupportVideoCallUpgradeRegardlessOfFeatureTags)
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strHeader))
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

    ON_CALL(*pConfigurationManager, IsSupportVideoCallUpgradeRegardlessOfFeatureTags)
            .WillByDefault(Return(IMS_FALSE));
    AString strHeader(MessageUtil::STR_P_TTA_VOLTE_INFO);
    ON_CALL(*pConfigurationManager, IsCarrierSpecificSipHeader(strHeader))
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objMessageUtils, GetCallType(&objMessage, &objSession, IMS_TRUE))
            .WillByDefault(Return(CallType::VT));
    ON_CALL(objMessageUtils, IsFocusConf(&objMessage)).WillByDefault(Return(IMS_TRUE));

    EXPECT_FALSE(objCallInfo.bConference);

    pMtcSession->HandleResponse(eType, objMessage);

    EXPECT_TRUE(objCallInfo.bConference);
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
