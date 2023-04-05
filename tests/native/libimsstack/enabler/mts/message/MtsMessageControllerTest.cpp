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
#include "CarrierConfig.h"
#include "ImsTypeDef.h"
#include "IuMtsService.h"
#include "MockIMtsService.h"
#include "MockIMtsServiceState.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "TestTimerService.h"
#include "core/IPageMessage.h"
#include "core/MockICoreService.h"
#include "core/MockIMessage.h"
#include "core/MockIMessageBodyPart.h"
#include "core/MockIPageMessage.h"
#include "message/MtsMessageController.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipHeaderName.h"
#include "utility/MtsDynamicLoader.h"

using ::testing::_;
using ::testing::AnyNumber;
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
    inline MtsMessageControllerTest() :
            pMtsDynamicLoader(IMS_NULL),
            pTimerService(new TestTimerService()),
            pMtsMessageController(IMS_NULL),
            objTimer(pTimerService->GetMockTimer())
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, pTimerService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);
    }
    inline virtual ~MtsMessageControllerTest()
    {
        delete pTimerService;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
    }

    MockICoreService objMockCoreService;
    MockIMessage objMockMessage;
    MockIMessageBodyPart objMockMessageBodyPart;
    MockISipMessage objMockSipMessage;
    MockIMtsService objMockMtsService;
    MockIMtsServiceState objMockMtsServiceState;
    MockIPageMessage objMockPageMessage;
    MtsDynamicLoader* pMtsDynamicLoader;
    TestConfigService objConfigService;
    TestTimerService* pTimerService;
    TestMtsMessageController* pMtsMessageController;
    MockITimer& objTimer;

protected:
    virtual void SetUp() override
    {
        pMtsDynamicLoader = new MtsDynamicLoader(SLOT_ID);
        pMtsDynamicLoader->Initialize();
        pMtsMessageController =
                new TestMtsMessageController(SLOT_ID, &objMockMtsService, pMtsDynamicLoader);
    }

    virtual void TearDown() override
    {
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
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWithEmergencyFlag)
{
    IMS_BOOL bEmergency = IMS_TRUE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::Assets::KEY_SMS_GEOLOCATION_PIDF_FOR_EMERGENCY_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWithoutTargetAddress)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress("");

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWithoutRPDU)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";

    ByteArray* pContent = new ByteArray(ByteArray::ConstNull());

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWhenMoServiceBlocked)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToGetICoreService)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(nullptr));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToGetUri)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToCreatePageMessage)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(nullptr));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToGetNextRequest)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(nullptr));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToAddHeader)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndFailToSendPageMessage)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsAndPageMessageDeliveryFailed)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_400));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, SendMoSmsAndReceiveAck)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ByteArray objRpAck((IMS_BYTE)0x03);  // message type indicator(RP-ACK)
    objRpAck.Append((IMS_BYTE)0x02);     // message reference
    objRpAck.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpAck));
    ON_CALL(objMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SUCCESS));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);

    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, SendMoSmsAndReceiveAckWithout202Accepted)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    AString strCallId = "0037811b8-3dbafe86@fc03:abab:cdcd:efe0:e9c3:3db4:69ec:66c8";

    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ByteArray objRpAck((IMS_BYTE)0x03);  // message type indicator(RP-ACK)
    objRpAck.Append((IMS_BYTE)0x02);     // message reference
    objRpAck.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetMessage()).WillByDefault(Return(&objMockSipMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpAck));
    ON_CALL(objMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SUCCESS));
    ON_CALL(objMockSipMessage, GetHeader(_, _, _)).WillByDefault(Return(strCallId));

    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(0);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);
    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
}

TEST_F(MtsMessageControllerTest, SendMoSmsWithSMMAAndFailToFormDestination)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x06);  // message type indicator(RP-SMMA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ReceiveMtSmsAndSendAck)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x02);  // message type indicator(RP-ACK)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(objMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SUCCESS));

    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ReceiveMtSmsAndSendAck3gpp2)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp2.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x02);  // message type indicator(RP-ACK)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(objMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SUCCESS));

    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP2, _)).Times(1);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP2, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP2, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, NotifyMtSmsWhenMtServiceBlocked)
{
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    EXPECT_CALL(objMockPageMessage, Reject(SipStatusCode::SC_480, _)).Times(1);
    EXPECT_CALL(objMockPageMessage, Destroy()).Times(1);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, NotifyMtSmsAndFailToGetICoreService)
{
    IMS_BOOL bEmergency = IMS_FALSE;

    ON_CALL(objMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(nullptr));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    EXPECT_CALL(objMockPageMessage, Reject(SipStatusCode::SC_480, _)).Times(1);
    EXPECT_CALL(objMockPageMessage, Destroy()).Times(1);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageIfIMessageIsNull)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));

    EXPECT_CALL(objMockPageMessage, Reject(400, _)).Times(1);
    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageBecauseOfNoToHeader)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objHeaders;

    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));

    EXPECT_CALL(objMockPageMessage, Reject(400, _)).Times(1);
    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageBecauseOfNoMessageBody)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();

    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));

    EXPECT_CALL(objMockPageMessage, Reject(400, _)).Times(1);
    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageBecauseOfInvalidSmsFormat)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "unknown";
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));

    EXPECT_CALL(objMockPageMessage, Reject(415, _)).Times(1);
    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ReceivedTooLargeRpdu)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "application/vnd.3gpp.sms";
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Resize(256);                // other required information elements

    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));

    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(0);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, FailInRespondReceivedMessageWithFormatFailure)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "application/vnd.3gpp.sms";
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(objMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SMS_FORMAT_FAILURE));

    EXPECT_CALL(objMockPageMessage, Reject(415, _)).Times(1);
    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, FailInRespondReceivedMessageWithNodataFailure)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "application/vnd.3gpp.sms";
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(objMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SMS_NODATA_FAILURE));

    EXPECT_CALL(objMockPageMessage, Reject(400, _)).Times(1);
    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _));
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, FailInRespondReceivedMessageWithFailure)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "application/vnd.3gpp.sms";
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(objMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_FAILURE));

    EXPECT_CALL(objMockPageMessage, Reject(480, _)).Times(1);
    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, FailInRespondReceivedMessageWithUnknownError)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "application/vnd.3gpp.sms";
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
    ON_CALL(objMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_INVALID));

    EXPECT_CALL(objMockPageMessage, Reject(500, _)).Times(1);
    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, CannotFindMatchedMtsMessageInPageMessageDelivered)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(objMockMtsService, ReportMoStatus(_, _, _, _)).Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(IMS_NULL);
}

TEST_F(MtsMessageControllerTest, NoReceivedResponsesInPageMessageDelivered)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));

    EXPECT_CALL(objMockMtsService, ReportMoStatus(_, _, _, _)).Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, CannotFindMatchedMtsMessageInPageMessageDeliveryFailed)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, Destroy()).WillByDefault(Return());

    EXPECT_CALL(objMockMtsService, ReportMoStatus(_, _, _, _)).Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(IMS_NULL);
}

TEST_F(MtsMessageControllerTest, NoReceivedResponsesInPageMessageDeliveryFailed)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_INVALID);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));

    EXPECT_CALL(objMockMtsService, ReportMoStatus(_, _, _, _)).Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, OnServiceDisconnectedThenRemoveAllMessages)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);

    pMtsMessageController->OnServiceDisconnected();
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
}

TEST_F(MtsMessageControllerTest, OnServiceSuspendedThenRemoveAllMessages)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);

    pMtsMessageController->OnServiceSuspended();
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
}

TEST_F(MtsMessageControllerTest, TerminateAllMessagesDoesNothingWhenThereIsNoMessage)
{
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
    pMtsMessageController->OnServiceSuspended();
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
}

TEST_F(MtsMessageControllerTest, ErrorResponseReceivedWithRetryAfterHeader)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<AString> objRetryAfterHeaders;
    objRetryAfterHeaders.Append("5");
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);
    const AString strCallId(SipHeaderName::CALL_ID);
    const AString strRetryAfter(SipHeaderName::RETRY_AFTER);
    const AString strTo(SipHeaderName::TO);
    const AString strFrom(SipHeaderName::FROM);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ByteArray objRpAck((IMS_BYTE)0x03);  // message type indicator(RP-ACK)
    objRpAck.Append((IMS_BYTE)0x02);     // message reference
    objRpAck.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_RETRY_AFTER_MAX_COUNT_INT, _))
            .WillByDefault(Return(5));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT, _))
            .WillByDefault(Return(45));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetHeaders(strRetryAfter)).WillByDefault(Return(objRetryAfterHeaders));
    ON_CALL(objMockMessage, GetHeaders(strTo)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetHeaders(strFrom)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpAck));
    ON_CALL(objMockMtsService, ReportMtSms(_, _)).WillByDefault(Return(MT_SUCCESS));

    EXPECT_CALL(objMockMessage, GetStatusCode())
            .Times(4)
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_202));
    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);

    pMtsMessageController->Timer_TimerExpired(&objTimer);

    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);

    EXPECT_CALL(objMockMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ReachRetryAfterMaxDuration)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<AString> objRetryAfterHeaders;
    objRetryAfterHeaders.Append("50");
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);
    const AString strCallId(SipHeaderName::CALL_ID);
    const AString strRetryAfter(SipHeaderName::RETRY_AFTER);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_RETRY_AFTER_MAX_COUNT_INT, _))
            .WillByDefault(Return(3));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT, _))
            .WillByDefault(Return(45));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetHeaders(strRetryAfter)).WillByDefault(Return(objRetryAfterHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_407));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ReachRetryAfterMaxCount)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<AString> objRetryAfterHeaders;
    objRetryAfterHeaders.Append("5");
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);
    const AString strCallId(SipHeaderName::CALL_ID);
    const AString strRetryAfter(SipHeaderName::RETRY_AFTER);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_RETRY_AFTER_MAX_COUNT_INT, _))
            .WillByDefault(Return(3));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT, _))
            .WillByDefault(Return(45));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _))
            .WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetHeaders(strRetryAfter)).WillByDefault(Return(objRetryAfterHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_407));

    EXPECT_CALL(objMockMtsService,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
    pMtsMessageController->Timer_TimerExpired(&objTimer);

    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
    pMtsMessageController->Timer_TimerExpired(&objTimer);

    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
    pMtsMessageController->Timer_TimerExpired(&objTimer);
}

}  // namespace android
