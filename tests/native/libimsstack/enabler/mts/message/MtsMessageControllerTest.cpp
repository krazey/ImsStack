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
#include "ImsTypeDef.h"
#include "../../include/mts/MockIMtsService.h"
#include "core/IPageMessage.h"
#include "core/MockICoreService.h"
#include "core/MockIMessage.h"
#include "core/MockIMessageBodyPart.h"
#include "core/MockIPageMessage.h"
#include "message/MtsMessageController.h"
#include "utility/MtsDynamicLoader.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;
const IMS_SINT32 SEQ_ID = 1;
const IMS_SINT32 RETRY_AFTER = 0;

class MtsMessageControllerTest : public ::testing::Test
{
public:
    MtsMessageController* pMtsMessageController;
    MtsDynamicLoader* pMtsDynamicLoader;
    MockIMtsService* pMtsService;

protected:
    virtual void SetUp() override
    {
        pMtsDynamicLoader = new MtsDynamicLoader(SLOT_ID);
        pMtsDynamicLoader->Initialize();
        pMtsService = new MockIMtsService();
        pMtsMessageController =
                new MtsMessageController(SLOT_ID, pMtsService, pMtsDynamicLoader);
    }

    virtual void TearDown() override
    {
        delete pMtsDynamicLoader;
        delete pMtsService;
        delete pMtsMessageController;
    }
};

TEST_F(MtsMessageControllerTest, Constructor)
{
    ASSERT_NE(pMtsMessageController, nullptr);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWithoutEmergencyFlag)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIPageMessage* pMockPageMessage = new MockIPageMessage();
    IMSList<IMessage*> objMessages = IMSList<IMessage*>();
    objMessages.Append(pMockMessage);

    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray objSms((IMS_BYTE)0x00);  // RP-DATA
    IMS_BOOL bEmergency = IMS_FALSE;
    pMtsDynamicLoader->GetMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsDynamicLoader->GetMtsServiceState()->OnImsConnected();
    pMtsDynamicLoader->GetMtsServiceState()->UpdateServiceState();

    ON_CALL(*pMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, SetListener(_)).WillByDefault(Return());

    ON_CALL(*pMockPageMessage, GetPreviousResponses(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(objMessages));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(*pMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objSms, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockPageMessage;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWithEmergencyFlag)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIPageMessage* pMockPageMessage = new MockIPageMessage();
    IMSList<IMessage*> objMessages = IMSList<IMessage*>();
    objMessages.Append(pMockMessage);

    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray objSms((IMS_BYTE)0x00); // RP-DATA
    IMS_BOOL bEmergency = IMS_TRUE;
    pMtsDynamicLoader->GetMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsDynamicLoader->GetMtsServiceState()->OnImsConnected();
    pMtsDynamicLoader->GetMtsServiceState()->UpdateServiceState();

    ON_CALL(*pMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, SetListener(_)).WillByDefault(Return());

    ON_CALL(*pMockPageMessage, GetPreviousResponses(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(objMessages));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(*pMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objSms, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockPageMessage;
}

TEST_F(MtsMessageControllerTest, NotifyMtSms)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIPageMessage* pMockPageMessage = new MockIPageMessage();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@mts.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@mts.google.com>,<sip:+12345678902@mts.google.com>";
    IMSList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    IMSList<IMessageBodyPart*> objMessageBodies = IMSList<IMessageBodyPart*>();
    objMessageBodies.Append(pMockMessageBodyPart);
    ByteArray objSms((IMS_BYTE)0x00);  // RP-DATA

    pMtsDynamicLoader->GetMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsDynamicLoader->GetMtsServiceState()->OnImsConnected();
    pMtsDynamicLoader->GetMtsServiceState()->UpdateServiceState();

    ON_CALL(*pMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objSms));

    EXPECT_CALL(*pMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMessageBodyPart;
    delete pMockPageMessage;
}

}  // namespace android
