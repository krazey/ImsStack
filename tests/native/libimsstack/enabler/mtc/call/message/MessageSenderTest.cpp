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
#include "MockICoreService.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "SipStatusCode.h"
#include "call/MockIMtcCallContext.h"
#include "call/message/MessageSender.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "utility/MessageUtils.h"
#include <gtest/gtest.h>

LOCAL IMS_SINT32 SLOT_ID = 0;

using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class MessageSenderTest : public ::testing::Test
{
public:
    MessageSender* pSender;

    CallInfo objCallInfo;
    MockIMessage objMessage;
    MockISipMessage objSipMessage;
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockICoreService objCoreService;
    MockISession objSession;
    MtcConfigurationProxy* pConfigurationProxy;
    MtcSupplementaryService* pSupplementaryService;
    MessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MtcConfigurationProxy(new MockIMtcConfigurationManager());
        pSupplementaryService = new MtcSupplementaryService(objContext, *pConfigurationProxy);

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));
        ON_CALL(objSession, GetNextResponse).WillByDefault(Return(&objMessage));
        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
        ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));

        pSender = new MessageSender(objContext, objSession);
    }

    virtual void TearDown() override
    {
        delete pSender;
        delete pConfigurationProxy;
        delete pSupplementaryService;
    }
};

TEST_F(MessageSenderTest, CreateSenderWithNormalFormatter)
{
    MessageSender* pSenderWithNormalFormatter = new MessageSender(objContext, objSession);

    EXPECT_NE(pSenderWithNormalFormatter, nullptr);

    delete pSenderWithNormalFormatter;
}

TEST_F(MessageSenderTest, CreateSenderWithEmergencyFormatter)
{
    CallInfo objEmergencyCallInfo;
    objEmergencyCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;

    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objEmergencyCallInfo));

    MessageSender* pSenderWithEmergencyFormatter = new MessageSender(objContext, objSession);

    EXPECT_NE(pSenderWithEmergencyFormatter, nullptr);

    delete pSenderWithEmergencyFormatter;
}

TEST_F(MessageSenderTest, StartNormalCase)
{
    IMS_RESULT nResult = pSender->Start(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, StartFormFailure)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pSender->Start(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, SendProvisionalResponseWith183)
{
    IMS_RESULT nResult =
            pSender->SendProvisionalResponse(SipStatusCode::SC_183, IMS_TRUE, IMS_TRUE, IMS_TRUE);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, SendProvisionalResponseWith180)
{
    IMS_RESULT nResult =
            pSender->SendProvisionalResponse(SipStatusCode::SC_180, IMS_FALSE, IMS_FALSE, IMS_TRUE);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, SendProvisionalResponseFormFailure)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    IMS_RESULT nResult =
            pSender->SendProvisionalResponse(SipStatusCode::SC_180, IMS_FALSE, IMS_FALSE, IMS_TRUE);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, SendPrackNormalCase)
{
    IMS_RESULT nResult = pSender->SendPrack();

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, SendPrackFormFailure)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pSender->SendPrack();

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, RespondToPrackWith200)
{
    IMS_RESULT nResult = pSender->RespondToPrack(SipStatusCode::SC_200);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, RespondToPrackFormFailure)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pSender->RespondToPrack(SipStatusCode::SC_200);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, SendEarlyUpdateNormalCase)
{
    IMS_RESULT nResult = pSender->SendEarlyUpdate(UpdateType::NORMAL);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, SendEarlyUpdateFormFailure)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pSender->SendEarlyUpdate(UpdateType::NORMAL);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, RespondToEarlyUpdateWith200)
{
    IMS_RESULT nResult = pSender->RespondToEarlyUpdate(SipStatusCode::SC_200);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, RespondToEarlyUpdateFormFailure)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pSender->RespondToEarlyUpdate(SipStatusCode::SC_200);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, AcceptNormalCase)
{
    IMS_RESULT nResult = pSender->Accept();

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, AcceptFormFailure)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pSender->Accept();

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, RejectWithCodeUserDecline)
{
    CallReasonInfo objReasonInfo(CODE_USER_DECLINE);
    IMS_RESULT nResult = pSender->Reject(objReasonInfo);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, RejectFormFailure)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    CallReasonInfo objReasonInfo(CODE_USER_DECLINE);
    IMS_RESULT nResult = pSender->Reject(objReasonInfo);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, SendAckNormalCase)
{
    IMS_RESULT nResult = pSender->SendAck();

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, SendAckFormFailure)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pSender->SendAck();

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, UpdateNormalCase)
{
    IMS_RESULT nResult =
            pSender->Update(UpdateType::NORMAL, IMS_TRUE, SipMethod::INVITE, IMS_FALSE);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, UpdateFormFailure)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult =
            pSender->Update(UpdateType::NORMAL, IMS_TRUE, SipMethod::INVITE, IMS_FALSE);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, AcceptUpdateNormalCase)
{
    IMS_RESULT nResult = pSender->AcceptUpdate();

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, AcceptUpdateFormFailure)
{
    ON_CALL(objSession, GetNextResponse).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pSender->AcceptUpdate();

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, CancelUpdateWithCodeNone)
{
    CallReasonInfo objReasonInfo(CODE_NONE);
    IMS_RESULT nResult = pSender->CancelUpdate(objReasonInfo);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, CancelUpdateFormFailure)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    CallReasonInfo objReasonInfo(CODE_NONE);
    IMS_RESULT nResult = pSender->CancelUpdate(objReasonInfo);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(MessageSenderTest, TerminateWithCodeUserTerminated)
{
    CallReasonInfo objReasonInfo(CODE_USER_TERMINATED);
    IMS_RESULT nResult = pSender->Terminate(IMS_TRUE, objReasonInfo);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, CancelWithCodeUserTerminated)
{
    CallReasonInfo objReasonInfo(CODE_USER_TERMINATED);
    IMS_RESULT nResult = pSender->Terminate(IMS_FALSE, objReasonInfo);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(MessageSenderTest, TerminateFormFailure)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    CallReasonInfo objReasonInfo(CODE_USER_TERMINATED);
    IMS_RESULT nResult = pSender->Terminate(IMS_TRUE, objReasonInfo);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

}  // namespace android
