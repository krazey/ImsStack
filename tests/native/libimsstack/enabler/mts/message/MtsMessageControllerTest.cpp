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

class TestMtsMessageController : public MtsMessageController
{
public:
    TestMtsMessageController(IN IMS_SINT32 nSlotId, IN IMtsService* piMtsService,
            IN MtsDynamicLoader* pMtsDynamicLoader) :
            MtsMessageController(nSlotId, piMtsService, pMtsDynamicLoader)
    {
    }
    virtual ~TestMtsMessageController() {}

    IMS_UINT32 GetMessageCount() const { return m_objMsgList.GetSize(); }
};

class MtsMessageControllerTest : public ::testing::Test
{
public:
    TestMtsMessageController* pMtsMessageController;
    MtsDynamicLoader* pMtsDynamicLoader;
    MockIMtsService* pMockMtsService;
    MockIPageMessage* pMockPageMessage;

protected:
    virtual void SetUp() override
    {
        pMockMtsService = new MockIMtsService();
        pMockPageMessage = new MockIPageMessage();
        pMtsDynamicLoader = new MtsDynamicLoader(SLOT_ID);
        pMtsDynamicLoader->Initialize();
        pMtsMessageController =
                new TestMtsMessageController(SLOT_ID, pMockMtsService, pMtsDynamicLoader);
    }

    virtual void TearDown() override
    {
        delete pMockMtsService;
        delete pMtsDynamicLoader;
        delete pMtsMessageController;
        delete pMockPageMessage;
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
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

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
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(*pMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWithEmergencyFlag)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

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
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(*pMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWithoutTargetAddress)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress;

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);

    delete pMockCoreService;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWithoutRPDU)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";

    ByteArray objRpData;

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);

    delete pMockCoreService;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWhenMoServiceBlocked)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);

    delete pMockCoreService;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWhenTemporaryServiceBlocked)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);

    delete pMockCoreService;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToGetICoreService)
{
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(nullptr));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);

    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToGetUri)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);

    delete pMockCoreService;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToCreatePageMessage)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

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
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(nullptr));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);

    delete pMockCoreService;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToGetNextRequest)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

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
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(nullptr));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);

    delete pMockCoreService;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToAddHeader)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

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
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToSendPageMessage)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

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
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndPageMessageDeliveryFailed)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

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
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(*pMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_400));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, SendMoSmsAndReceiveAck)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

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
    objRpAck.Append((IMS_BYTE)0x02);     // message reference
    objRpAck.Append((IMS_BYTE)0x0F);     // other required information elements

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
    ON_CALL(*pMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(*pMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SUCCESS));

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
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, SendMoSmsWithSMMAAndFailToFormDestination)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray objRpData((IMS_BYTE)0x06);  // message type indicator(RP-SMMA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ByteArray objRpAck((IMS_BYTE)0x03);  // message type indicator(RP-ACK)
    objRpAck.Append((IMS_BYTE)0x02);     // message reference
    objRpAck.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    ON_CALL(*pMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(*pMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, ReceiveMtSmsAndSendAck)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

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
    ON_CALL(*pMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(*pMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SUCCESS));

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
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, ReceiveMtSmsAndSendAck3gpp2)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

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
    ON_CALL(*pMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(*pMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SUCCESS));

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
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMtSmsWhenMtServiceBlocked)
{
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    EXPECT_CALL(*pMockPageMessage, Reject(SipStatusCode::SC_480, _)).Times(1);
    EXPECT_CALL(*pMockPageMessage, Destroy()).Times(1);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NotifyMtSmsAndFailToGetICoreService)
{
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    IMS_BOOL bEmergency = IMS_FALSE;

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(nullptr));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    EXPECT_CALL(*pMockPageMessage, Reject(SipStatusCode::SC_480, _)).Times(1);
    EXPECT_CALL(*pMockPageMessage, Destroy()).Times(1);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageIfIMessageIsNull)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));

    EXPECT_CALL(*pMockPageMessage, Reject(400, _)).Times(1);
    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageBecauseOfNoToHeader)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objHeaders;

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));

    EXPECT_CALL(*pMockPageMessage, Reject(400, _)).Times(1);
    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageBecauseOfNoMessageBody)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));

    EXPECT_CALL(*pMockPageMessage, Reject(400, _)).Times(1);
    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageBecauseOfInvalidSmsFormat)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(pMockMessageBodyPart);

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "unknown";

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));

    EXPECT_CALL(*pMockPageMessage, Reject(415, _)).Times(1);
    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMessageBodyPart;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, ReceivedTooLargeRpdu)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(pMockMessageBodyPart);

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "application/vnd.3gpp.sms";

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Resize(256);                // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));

    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMessageBodyPart;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, FailInRespondReceivedMessageWithFormatFailure)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(pMockMessageBodyPart);

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "application/vnd.3gpp.sms";

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(*pMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SMS_FORMAT_FAILURE));

    EXPECT_CALL(*pMockPageMessage, Reject(415, _)).Times(1);
    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMessageBodyPart;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, FailInRespondReceivedMessageWithNodataFailure)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(pMockMessageBodyPart);

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "application/vnd.3gpp.sms";

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(*pMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SMS_NODATA_FAILURE));

    EXPECT_CALL(*pMockPageMessage, Reject(400, _)).Times(1);
    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMessageBodyPart;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, FailInRespondReceivedMessageWithFailure)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(pMockMessageBodyPart);

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "application/vnd.3gpp.sms";

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(*pMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_FAILURE));

    EXPECT_CALL(*pMockPageMessage, Reject(480, _)).Times(1);
    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMessageBodyPart;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, FailInRespondReceivedMessageWithUnknownError)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMessageBodyPart* pMockMessageBodyPart = new MockIMessageBodyPart();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(pMockMessageBodyPart);

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "application/vnd.3gpp.sms";

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(*pMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(*pMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(*pMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(*pMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_INVALID));

    EXPECT_CALL(*pMockPageMessage, Reject(500, _)).Times(1);
    EXPECT_CALL(*pMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMessageBodyPart;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, CannotFindMatchedMtsMessageInPageMessageDelivered)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(*pMockMtsService, ReportMoStatus(_, _, _, _)).Times(0);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(IMS_NULL);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NoReceivedResponsesInPageMessageDelivered)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();
    ImsList<IMessage*> objMessages;

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    ON_CALL(*pMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));

    EXPECT_CALL(*pMockMtsService, ReportMoStatus(_, _, _, _)).Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, CannotFindMatchedMtsMessageInPageMessageDeliveryFailed)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockPageMessage, Destroy()).WillByDefault(Return());

    EXPECT_CALL(*pMockMtsService, ReportMoStatus(_, _, _, _)).Times(0);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(IMS_NULL);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, NoReceivedResponsesInPageMessageDeliveryFailed)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    ON_CALL(*pMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));

    EXPECT_CALL(*pMockMtsService, ReportMoStatus(_, _, _, _)).Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(pMockPageMessage);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, OnServiceDisconnectedThenRemoveAllMessages)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    IMS_UINT32 nMessageCount = pMtsMessageController->GetMessageCount();
    EXPECT_EQ(nMessageCount, 1);

    pMtsMessageController->OnServiceDisconnected();
    nMessageCount = pMtsMessageController->GetMessageCount();
    EXPECT_EQ(nMessageCount, 0);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, OnServiceSuspendedThenRemoveAllMessages)
{
    MockICoreService* pMockCoreService = new MockICoreService();
    MockIMessage* pMockMessage = new MockIMessage();
    MockIMtsServiceState* pMockMtsServiceState = new MockIMtsServiceState();

    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(*pMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(pMockCoreService));
    ON_CALL(*pMockMtsService, GetIMtsServiceState()).WillByDefault(Return(pMockMtsServiceState));
    ON_CALL(*pMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockMtsServiceState, IsTemporaryServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(*pMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(pMockPageMessage));
    ON_CALL(*pMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(*pMockPageMessage, GetNextRequest()).WillByDefault(Return(pMockMessage));
    ON_CALL(*pMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(*pMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID, bEmergency);
    IMS_UINT32 nMessageCount = pMtsMessageController->GetMessageCount();
    EXPECT_EQ(nMessageCount, 1);

    pMtsMessageController->OnServiceSuspended();
    nMessageCount = pMtsMessageController->GetMessageCount();
    EXPECT_EQ(nMessageCount, 0);

    delete pMockCoreService;
    delete pMockMessage;
    delete pMockMtsServiceState;
}

TEST_F(MtsMessageControllerTest, TerminateAllMessagesDoesNothingWhenThereIsNoMessage)
{
    IMS_UINT32 nMessageCount = pMtsMessageController->GetMessageCount();
    EXPECT_EQ(nMessageCount, 0);

    pMtsMessageController->OnServiceSuspended();
    nMessageCount = pMtsMessageController->GetMessageCount();
    EXPECT_EQ(nMessageCount, 0);
}

TEST_F(MtsMessageControllerTest, AosControlForRegistrationRecovery)
{
    EXPECT_CALL(*pMockMtsService, RequestRegistrationRecovery(MTS_REG_RECOVERY_POLICY_NONE))
            .Times(1);

    pMtsMessageController->NotifyControlAos(MTS_REG_RECOVERY_POLICY_NONE);
}

}  // namespace android
