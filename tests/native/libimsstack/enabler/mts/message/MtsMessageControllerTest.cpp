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
#include "IuMtsService.h"
#include "MockIMtsService.h"
#include "MockIMtsServiceState.h"
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
    MockIMtsService* pMockMtsService;

protected:
    virtual void SetUp() override
    {
        pMockMtsService = new MockIMtsService();
        pMtsDynamicLoader = new MtsDynamicLoader(SLOT_ID);
        pMtsDynamicLoader->Initialize();
        pMtsMessageController =
                new MtsMessageController(SLOT_ID, pMockMtsService, pMtsDynamicLoader);
    }

    virtual void TearDown() override
    {
        delete pMockMtsService;
        delete pMtsDynamicLoader;
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
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessage*> objMessages;
    objMessages.Append(pMockMessage);

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
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

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockPageMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWithEmergencyFlag)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIPageMessage* pMockPageMessage = new MockIPageMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessage*> objMessages;
    objMessages.Append(pMockMessage);

    IMS_BOOL bEmergency = IMS_TRUE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
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

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockPageMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, SendMoSmsAndReceiveAck)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIPageMessage* pMockPageMessage = new MockIPageMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessage*> objMessages;
    objMessages.Append(pMockMessage);

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(pMockMessageBodyPart);

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ByteArray objRpAck((IMS_BYTE)0x03);  // message type indicator(RP-ACK)
    objRpData.Append((IMS_BYTE)0x02);    // message reference
    objRpData.Append((IMS_BYTE)0x0F);    // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, GetPreviousResponses(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(objMessages));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(pMockPageMessage);

    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMessageBodyPart;
    delete pMockPageMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, ReceiveMtSmsAndSendAck)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIPageMessage* pMockPageMessage = new MockIPageMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessage*> objMessages;
    objMessages.Append(pMockMessage);

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(pMockMessageBodyPart);

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ByteArray objRpAck((IMS_BYTE)0x02);  // message type indicator(RP-ACK)
    objRpAck.Append((IMS_BYTE)0x03);     // message reference
    objRpAck.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, GetPreviousResponses(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(objMessages));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));

    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpAck, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMessageBodyPart;
    delete pMockPageMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, ReceiveMtSmsAndSendAck3gpp2)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIPageMessage* pMockPageMessage = new MockIPageMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessage*> objMessages;
    objMessages.Append(pMockMessage);

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp2.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(pMockMessageBodyPart);

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ByteArray objRpAck((IMS_BYTE)0x02);  // message type indicator(RP-ACK)
    objRpAck.Append((IMS_BYTE)0x03);     // message reference
    objRpAck.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, GetPreviousResponses(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(objMessages));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));

    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP2, _)).Times(1);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP2, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP2, objRpAck, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMessageBodyPart;
    delete pMockPageMessage;
    delete pMockMtsServiceState;
}

}  // namespace android
