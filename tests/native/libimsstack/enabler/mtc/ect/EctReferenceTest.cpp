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

#include "IMtcService.h"
#include "MockICoreService.h"
#include "MockIMessage.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockIReference.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "MtcDef.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/ParticipantInfo.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MockIMtcDialingPlan.h"
#include "ect/EctReference.h"
#include "ect/MockIEctReferenceListener.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockReferenceInterfaceHolder.h"
#include "utility/MessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

namespace android
{

LOCAL CallKey ANY_TRANSFEREE_CALL_KEY = 100;
LOCAL CallKey ANY_TRANSFER_TARGET_CALL_KEY = 200;
LOCAL AString ANY_TRANSFER_TARGET = "1111";

class EctReferenceTest : public ::testing::Test
{
public:
    MockIMtcContext objMockContext;
    MockIMtcService objMtcService;
    MockICoreService objCoreService;
    MockIEctReferenceListener objMockListener;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MockIMtcSipInterfaceFactory objMockInterfaceFactory;
    MockReferenceInterfaceHolder* pMockReferenceInterfaceHolder;
    MockIInterfaceHolderListener objMockHolderListener;
    MockIMtcCallManager objMockCallManager;
    MockIMtcDialingPlan objDialingPlan;

    MockIMtcCall objMockTransfereeCall;
    MockIMtcCallContext objMockTransfereeContext;
    MockIMtcSession objMockTransfereeMtcSession;
    MockISession objMockTransfereeSession;

    MockIMtcCall objMockTargetCall;
    MockIMtcCallContext objMockTargetContext;
    MockIMtcSession objMockTargetMtcSession;
    MockISession objMockTargetSession;
    CallInfo objTargetInfo;
    ParticipantInfo* pParticipantInfo;

    MockIReference objMockReference;
    MockIMessage objMockMessage;
    MockISipMessage objMockSipMessage;
    EctReference* pEctReference;
    MessageUtils objMessageUtils;
    MtcSupplementaryService* pSupplementaryService;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objMockContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objMockContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(&objMtcService));
        ON_CALL(objMtcService, GetICoreService).WillByDefault(Return(&objCoreService));

        ON_CALL(objMockContext, GetCallManager).WillByDefault(ReturnRef(objMockCallManager));

        ON_CALL(objMockContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(objMockInterfaceFactory));

        ON_CALL(objMockContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objMockContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));

        pMockReferenceInterfaceHolder = new MockReferenceInterfaceHolder(objMockHolderListener);
        ON_CALL(objMockInterfaceFactory, GetIReferenceHolder)
                .WillByDefault(Return(pMockReferenceInterfaceHolder));
        ON_CALL(*pMockReferenceInterfaceHolder, GetIReference(_, _, _))
                .WillByDefault(Return(&objMockReference));

        pSupplementaryService =
                new MtcSupplementaryService(objMockTargetContext, *pConfigurationProxy);
        ON_CALL(objMockTargetContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));

        pParticipantInfo = IMS_NULL;
        SetUpCalls();
        pEctReference = new EctReference(objMockContext, ANY_TRANSFEREE_CALL_KEY, objMockListener);
    }

    virtual void TearDown() override
    {
        delete pEctReference;
        delete pConfigurationProxy;
        delete pMockReferenceInterfaceHolder;
        delete pParticipantInfo;
    }

    void SetUpCalls()
    {
        ON_CALL(objMockCallManager, GetCallByCallKey(ANY_TRANSFEREE_CALL_KEY))
                .WillByDefault(Return(&objMockTransfereeCall));
        ON_CALL(objMockTransfereeCall, GetCallContext)
                .WillByDefault(ReturnRef(objMockTransfereeContext));
        ON_CALL(objMockTransfereeContext, GetSession())
                .WillByDefault(Return(&objMockTransfereeMtcSession));
        ON_CALL(objMockTransfereeMtcSession, GetISession())
                .WillByDefault(ReturnRef(objMockTransfereeSession));

        ON_CALL(objMockCallManager, GetCallByCallKey(ANY_TRANSFER_TARGET_CALL_KEY))
                .WillByDefault(Return(&objMockTargetCall));
        ON_CALL(objMockTargetCall, GetCallContext).WillByDefault(ReturnRef(objMockTargetContext));
        ON_CALL(objMockTargetContext, GetSession()).WillByDefault(Return(&objMockTargetMtcSession));
        ON_CALL(objMockTargetMtcSession, GetISession())
                .WillByDefault(ReturnRef(objMockTargetSession));
        ON_CALL(objMockTargetContext, GetConfigurationProxy)
                .WillByDefault(ReturnRef(*pConfigurationProxy));
    }

    void SetUpUriFormatter(IN const AString& strUri)
    {
        ON_CALL(objMtcService, IsEmergency).WillByDefault(Return(IMS_FALSE));
        ON_CALL(objMockTargetContext, GetService).WillByDefault(ReturnRef(objMtcService));

        ON_CALL(objMockTargetContext, GetCallInfo).WillByDefault(ReturnRef(objTargetInfo));
        objTargetInfo.bConference = IMS_TRUE;  // to get Remote Uri from ParticipantInfo easily

        ON_CALL(objMockTargetContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));

        delete pParticipantInfo;
        pParticipantInfo = new ParticipantInfo(objMockTargetContext);
        ON_CALL(objMockTargetContext, GetParticipantInfo)
                .WillByDefault(ReturnRef(*pParticipantInfo));

        ON_CALL(*pConfigurationProxy,
                GetBoolean(ConfigVoice::KEY_CONFERENCE_REFER_TO_URI_SOURCE_PAID_BOOL))
                .WillByDefault(Return(IMS_FALSE));

        ON_CALL(objDialingPlan, GetToUri(_, _, Scheme::SIP)).WillByDefault(Return(strUri));
        ON_CALL(objDialingPlan, GetToUri(_, _, Scheme::UNKNOWN)).WillByDefault(Return(strUri));
        pParticipantInfo->UpdateFromRemoteNumber("123");
    }

    void SetUpForSuccessfulReferenceOperation(IN const AString& strSessionId)
    {
        SetUpUriFormatter("sip:anyuri");
        ON_CALL(objMockTransfereeCall, GetState)
                .WillByDefault(Return(IMtcCall::State::ESTABLISHED));
        ON_CALL(objMockTransfereeCall, GetKey).WillByDefault(Return(ANY_TRANSFEREE_CALL_KEY));
        ON_CALL(objMockTargetSession, GetSessionId).WillByDefault(ReturnRef(strSessionId));
        ON_CALL(objCoreService, GetLocalUserId).WillByDefault(Return("anyUserId"));
        ON_CALL(objMockReference, GetNextRequest).WillByDefault(Return(&objMockMessage));
        ON_CALL(objMockMessage, GetMessage).WillByDefault(Return(&objMockSipMessage));
        ON_CALL(objMockSipMessage, AddHeader(_, _, _)).WillByDefault(Return(IMS_SUCCESS));
    }
};

TEST_F(EctReferenceTest, DestructorReleaseInterface)
{
    EXPECT_CALL(*pMockReferenceInterfaceHolder, ReleaseIReference(_, _)).Times(1);
}

TEST_F(EctReferenceTest, ReferenceDeliveredCallsListener)
{
    ON_CALL(objMockReference, GetPreviousResponse(_)).WillByDefault(Return(nullptr));
    EXPECT_CALL(objMockListener, OnReferenceStarted).Times(1);
    pEctReference->ReferenceDelivered(&objMockReference);

    ON_CALL(objMockReference, GetPreviousResponse(_)).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_202));
    EXPECT_CALL(objMockListener, OnReferenceStarted).Times(1);
    pEctReference->ReferenceDelivered(&objMockReference);

    ON_CALL(objMockMessage, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_405));
    EXPECT_CALL(objMockListener, OnReferenceStartFailed).Times(1);
    pEctReference->ReferenceDelivered(&objMockReference);
}

TEST_F(EctReferenceTest, ReferenceDeliveryFailedCallsListener)
{
    EXPECT_CALL(objMockListener, OnReferenceStartFailed).Times(1);
    pEctReference->ReferenceDeliveryFailed(&objMockReference);
}

TEST_F(EctReferenceTest, ReferenceNotifyInvokesOnReferenceUpdated)
{
    MockIMessage objMockNotifyMessage;
    ON_CALL(objMockNotifyMessage, GetMessage).WillByDefault(ReturnNull());

    EXPECT_CALL(objMockListener, OnReferenceUpdated(_)).Times(1);

    pEctReference->ReferenceNotify(&objMockReference, &objMockMessage);
}

TEST_F(EctReferenceTest, ReferenceTerminatedDoesNothing)
{
    EXPECT_CALL(objMockListener, OnReferenceStarted).Times(0);
    EXPECT_CALL(objMockListener, OnReferenceStartFailed).Times(0);
    pEctReference->ReferenceTerminated(&objMockReference);
}

TEST_F(EctReferenceTest, SendInviteWithKeyReturnsFailure)
{
    ON_CALL(objMockTransfereeCall, GetState).WillByDefault(Return(IMtcCall::State::TERMINATING));
    EXPECT_EQ(IMS_FAILURE, pEctReference->SendInvite(ANY_TRANSFER_TARGET_CALL_KEY));

    ON_CALL(objMockTransfereeCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));

    SetUpUriFormatter("");
    EXPECT_EQ(IMS_FAILURE, pEctReference->SendInvite(ANY_TRANSFER_TARGET_CALL_KEY));

    SetUpUriFormatter("sip:anyuri");
    ON_CALL(objMockTransfereeCall, GetKey).WillByDefault(Return(IMtcCall::CALL_KEY_INVALID));
    EXPECT_EQ(IMS_FAILURE, pEctReference->SendInvite(ANY_TRANSFER_TARGET_CALL_KEY));
}

TEST_F(EctReferenceTest, SendInviteWithKeyReturnsSuccess)
{
    AString strAnySessionId("sessionid");
    SetUpForSuccessfulReferenceOperation(strAnySessionId);

    EXPECT_CALL(objMockReference, SetListener(pEctReference)).Times(1);
    EXPECT_CALL(objMockReference, ReferEx(IMS_TRUE, _)).Times(1);
    EXPECT_CALL(objMockReference, SetReplaces(strAnySessionId)).Times(1);

    EXPECT_EQ(IMS_SUCCESS, pEctReference->SendInvite(ANY_TRANSFER_TARGET_CALL_KEY));
}

TEST_F(EctReferenceTest, SendInviteWithKeyReturnsSuccessAndNotSetReferredByHeaderIfNoLocalUserId)
{
    AString strAnySessionId("sessionid");
    SetUpForSuccessfulReferenceOperation(strAnySessionId);
    ON_CALL(objCoreService, GetLocalUserId).WillByDefault(Return(""));

    EXPECT_CALL(objMockReference, SetListener(pEctReference)).Times(1);
    EXPECT_CALL(objMockReference, SetReplaces(strAnySessionId)).Times(1);
    EXPECT_CALL(objMockSipMessage, AddHeader(_, _, _)).Times(0);
    EXPECT_CALL(objMockReference, ReferEx(IMS_TRUE, _)).Times(1);

    EXPECT_EQ(IMS_SUCCESS, pEctReference->SendInvite(ANY_TRANSFER_TARGET_CALL_KEY));
}

TEST_F(EctReferenceTest, SendInviteWithKeyReturnsSuccessAndNotSetReferredByHeaderIfMessageIsNull)
{
    AString strAnySessionId("sessionid");
    SetUpForSuccessfulReferenceOperation(strAnySessionId);
    ON_CALL(objMockReference, GetNextRequest).WillByDefault(ReturnNull());

    EXPECT_CALL(objMockReference, SetListener(pEctReference)).Times(1);
    EXPECT_CALL(objMockReference, SetReplaces(strAnySessionId)).Times(1);
    EXPECT_CALL(objMockSipMessage, AddHeader(_, _, _)).Times(0);
    EXPECT_CALL(objMockReference, ReferEx(IMS_TRUE, _)).Times(1);

    EXPECT_EQ(IMS_SUCCESS, pEctReference->SendInvite(ANY_TRANSFER_TARGET_CALL_KEY));
}

TEST_F(EctReferenceTest, SendInviteWithNumberReturnsFailureIfCallStatusIsTerminating)
{
    ON_CALL(objMockTransfereeCall, GetState).WillByDefault(Return(IMtcCall::State::TERMINATING));

    EXPECT_EQ(IMS_FAILURE, pEctReference->SendInvite(ANY_TRANSFER_TARGET));

    ON_CALL(objMockTransfereeCall, GetState).WillByDefault(Return(IMtcCall::State::ESTABLISHED));
}

TEST_F(EctReferenceTest, SendInviteWithNumberReturnsFailureIfReferenceIsNull)
{
    AString strAnySessionId("sessionid");
    SetUpForSuccessfulReferenceOperation(strAnySessionId);

    ON_CALL(*pMockReferenceInterfaceHolder, GetIReference(_, _, _)).WillByDefault(ReturnNull());

    EXPECT_CALL(objMockReference, SetListener(pEctReference)).Times(0);

    EXPECT_EQ(IMS_FAILURE, pEctReference->SendInvite(ANY_TRANSFER_TARGET));
}

TEST_F(EctReferenceTest, SendInviteWithNumberReturnsSuccess)
{
    AString strAnySessionId("sessionid");
    SetUpForSuccessfulReferenceOperation(strAnySessionId);

    EXPECT_EQ(IMS_SUCCESS, pEctReference->SendInvite(ANY_TRANSFER_TARGET));
}

TEST_F(EctReferenceTest, GetResponseCode)
{
    AString strAnySessionId("sessionid");
    SetUpForSuccessfulReferenceOperation(strAnySessionId);  // to set m_piReference
    pEctReference->SendInvite(ANY_TRANSFER_TARGET_CALL_KEY);

    ON_CALL(objMockReference, GetPreviousResponse(_)).WillByDefault(Return(nullptr));
    EXPECT_EQ(SipStatusCode::SC_INVALID, pEctReference->GetResponseCode());

    IMS_SINT32 nAnyCode = SipStatusCode::SC_202;
    ON_CALL(objMockReference, GetPreviousResponse(_)).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode).WillByDefault(Return(nAnyCode));
    EXPECT_EQ(nAnyCode, pEctReference->GetResponseCode());
}

}  // namespace android
