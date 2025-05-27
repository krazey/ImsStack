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

#include "CarrierConfig.h"
#include "GeolocationHelper.h"
#include "IPageMessage.h"
#include "ImsTypeDef.h"
#include "IuMtsService.h"
#include "JniEnablerConnector.h"
#include "MockICoreService.h"
#include "MockIJniEnabler.h"
#include "MockIJniMtsServiceThread.h"
#include "MockIMessage.h"
#include "MockIMessageBodyPart.h"
#include "MockIMtsContext.h"
#include "MockIMtsService.h"
#include "MockIMtsServiceState.h"
#include "MockIPageMessage.h"
#include "MockISipMessage.h"
#include "MockISipMessageBodyPart.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "SipHeaderName.h"
#include "TestConfigService.h"
#include "TestPhoneInfoService.h"
#include "TestTimerService.h"
#include "message/MtsMessageController.h"
#include "utility/MtsDynamicLoader.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

const LOCAL IMS_SINT32 MESSAGE_RESPONSE_WAIT_TIMER = 8000;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;
const IMS_SINT32 SEQ_ID = 1;

class TestMtsMessageController : public MtsMessageController
{
public:
    TestMtsMessageController(IN IMtsContext& objContext) :
            MtsMessageController(objContext)
    {
    }
    virtual ~TestMtsMessageController() {}

    IMS_UINT32 GetMessageCount() const { return m_objMsgList.GetSize(); }
    IMS_BOOL SendMessage(IN ImsMessage& objMsg) { return OnMessage(objMsg); }
};

class MtsMessageControllerTest : public ::testing::Test
{
public:
    inline MtsMessageControllerTest() :
            pMtsMessageController(IMS_NULL),
            pTimerService(new TestTimerService()),
            objTimer(pTimerService->GetMockTimer())
    {
        objPhoneInfoService.SetLocationInfo(&objMockILocationInfo);
        SetUpTestLocationProperties();
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);
        GeolocationHelper::GetInstance()->CreatePidfCreator(SLOT_ID);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, pTimerService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);
    }
    inline virtual ~MtsMessageControllerTest()
    {
        delete pTimerService;
        GeolocationHelper::GetInstance()->DestroyPidfCreator(SLOT_ID);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
    }

    TestMtsMessageController* pMtsMessageController;

    JniEnablerConnector* pConnector;
    MockICoreService objMockCoreService;
    MockIJniEnabler objMockJniEnabler;
    MockIJniMtsServiceThread objJniMtsServiceThread;
    MockILocationInfo objMockILocationInfo;
    MockILocationProperties objMockILocationProperties;
    MockIMessage objMockMessage;
    MockIMessageBodyPart objMockMessageBodyPart;
    MockIMtsContext objContext;
    MockIMtsService objMockMtsService;
    MockIMtsServiceState objMockMtsServiceState;
    MockIPageMessage objMockPageMessage;
    MockISipMessage objMockSipMessage;
    MtsDynamicLoader* pMtsDynamicLoader;
    TestConfigService objConfigService;
    TestPhoneInfoService objPhoneInfoService;
    TestTimerService* pTimerService;
    MockITimer& objTimer;

    AString strLocationProperties = AString("LocationProperties");

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objMockMtsService));
        pMtsMessageController = new TestMtsMessageController(objContext);

        pMtsDynamicLoader = new MtsDynamicLoader(objContext);
        ON_CALL(objContext, GetDynamicLoader).WillByDefault(ReturnRef(*pMtsDynamicLoader));
        ON_CALL(objMockMtsService, GetJniServiceThread)
                .WillByDefault(Return(&objJniMtsServiceThread));

        pConnector = &JniEnablerConnector::GetInstance();
        pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTS_SERVICE, &objMockJniEnabler);
        ON_CALL(objMockJniEnabler, GetJniThread).WillByDefault(Return(&objJniMtsServiceThread));
    }

    virtual void TearDown() override
    {
        delete pMtsDynamicLoader;
        delete pMtsMessageController;
        delete pConnector;
    }

    void SetUpTestLocationProperties()
    {
        ON_CALL(objMockILocationInfo, GetLocationProperties(_))
                .WillByDefault(Return(&objMockILocationProperties));
        ON_CALL(objMockILocationProperties, GetLatitude())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetLongitude())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetRadius())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetShape())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetConfidence())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetCurrentTime())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetMethod())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetCountry())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetState())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetCity())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetPostal())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetAltitude())
                .WillByDefault(ReturnRef(strLocationProperties));
        ON_CALL(objMockILocationProperties, GetVerticalAccuracy())
                .WillByDefault(ReturnRef(strLocationProperties));
    }
};

TEST_F(MtsMessageControllerTest, Constructor)
{
    ASSERT_NE(pMtsMessageController, nullptr);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithoutEmergencyFlag)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objCallIdHeaders;
    objCallIdHeaders.Append("0057f183b-245fdcb9@192.168.45.139");
    const AString strCallId(SipHeaderName::CALL_ID);

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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithEmergencyFlag)
{
    IMS_BOOL bEmergency = IMS_TRUE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objCallIdHeaders;
    objCallIdHeaders.Append("0057f183b-245fdcb9@192.168.45.139");
    const AString strCallId(SipHeaderName::CALL_ID);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_GEOLOCATION_PIDF_FOR_EMERGENCY_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithoutTargetAddress)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress("");

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithoutRPDU)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";

    ByteArray* pContent = new ByteArray(ByteArray::ConstNull());

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWhenMoServiceBlocked)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToGetICoreService)
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

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToGetUri)
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

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToCreatePageMessage)
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

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithContentTransferEncoding)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objCallIdHeaders;
    objCallIdHeaders.Append("0057f183b-245fdcb9@192.168.45.139");
    const AString strCallId(SipHeaderName::CALL_ID);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_SUPPORT_CONTENT_TRANSFER_ENCODING_HEADER_BOOL,
                    _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::REQUEST_DISPOSITION), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::CONTENT_TRANSFER_ENCODING), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));

    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithoutContentTransferEncoding)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objCallIdHeaders;
    objCallIdHeaders.Append("0057f183b-245fdcb9@192.168.45.139");
    const AString strCallId(SipHeaderName::CALL_ID);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_SUPPORT_CONTENT_TRANSFER_ENCODING_HEADER_BOOL,
                    _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::REQUEST_DISPOSITION), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::CONTENT_TRANSFER_ENCODING), _))
            .Times(0)
            .WillOnce(Return(IMS_SUCCESS));

    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToGetNextRequest)
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(nullptr));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToAddHeader)
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToSendPageMessage)
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndPageMessageDeliveryFailed)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objCallIdHeaders;
    objCallIdHeaders.Append("0057f183b-245fdcb9@192.168.45.139");
    const AString strCallId(SipHeaderName::CALL_ID);

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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_400));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
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
    ImsList<AString> objCallIdHeaders;
    objCallIdHeaders.Append("0057f183b-245fdcb9@192.168.45.139");
    const AString strCallId(SipHeaderName::CALL_ID);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);
    AString strInReplyTo = "0057f183b-245fdcb9@192.168.45.139";

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
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_IN_REPLY_TO_VALIDATION_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessage, GetMessage()).WillByDefault(Return(&objMockSipMessage));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpAck));
    ON_CALL(objMockSipMessage, GetHeader(_, _, _)).WillByDefault(Return(strInReplyTo));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);

    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, NormalMoSmsAndInReplyToMismatched)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objCallIdHeaders;
    objCallIdHeaders.Append("0057f183b-245fdcb9@192.168.45.139");
    const AString strCallId(SipHeaderName::CALL_ID);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);
    AString strInReplyTo = "0057f183b-245fdcb9@192.168.45.1";

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
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_IN_REPLY_TO_VALIDATION_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessage, GetMessage()).WillByDefault(Return(&objMockSipMessage));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpAck));
    ON_CALL(objMockSipMessage, GetHeader(_, _, _)).WillByDefault(Return(strInReplyTo));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
    EXPECT_CALL(objMockPageMessage, Reject(SipStatusCode::SC_488, _)).Times(1);
    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
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
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_IN_REPLY_TO_VALIDATION_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
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
    ON_CALL(objMockSipMessage, GetHeader(_, _, _)).WillByDefault(Return(strCallId));

    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);
    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, SendMoSmsWithGeoLocationInformation)
{
    IMS_BOOL bEmergency = IMS_TRUE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    MockISipMessageBodyPart objMockISipMessageBodyPart;

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_GEOLOCATION_PIDF_FOR_EMERGENCY_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetMessage()).WillByDefault(Return(&objMockSipMessage));
    ON_CALL(objMockSipMessage, CreateBodyPart()).WillByDefault(Return(&objMockISipMessageBodyPart));

    EXPECT_CALL(objMockISipMessageBodyPart, SetContent(_)).Times(1);
    EXPECT_CALL(objMockISipMessageBodyPart, SetHeader(_, _, _)).Times(3);
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::REQUEST_DISPOSITION), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::GEOLOCATION), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::GEOLOCATION_ROUTING), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
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

    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ReceiveMtSmsAndSendAckFailed)
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_407));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));

    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
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

    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP2, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP2, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP2, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ProcessMtSmsWhenMtServiceBlocked)
{
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    EXPECT_CALL(objMockPageMessage, Reject(SipStatusCode::SC_480, _)).Times(1);
    EXPECT_CALL(objMockPageMessage, Destroy()).Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ProcessMtSmsAndFailToGetICoreService)
{
    IMS_BOOL bEmergency = IMS_FALSE;

    ON_CALL(objMockMtsService, GetICoreService(bEmergency)).WillByDefault(Return(nullptr));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    EXPECT_CALL(objMockPageMessage, Reject(SipStatusCode::SC_480, _)).Times(1);
    EXPECT_CALL(objMockPageMessage, Destroy()).Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
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
    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
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
    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
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
    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
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
    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
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

    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(objJniMtsServiceThread, ReportMoStatus(_, _, _, SLOT_ID)).Times(1);
    pMtsMessageController->ProcessMoSms(
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));

    EXPECT_CALL(objJniMtsServiceThread, ReportMoStatus(_, _, _, SLOT_ID)).Times(1);
    pMtsMessageController->ProcessMoSms(
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, Destroy()).WillByDefault(Return());

    EXPECT_CALL(objJniMtsServiceThread, ReportMoStatus(_, _, _, SLOT_ID)).Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(IMS_NULL);
}

TEST_F(MtsMessageControllerTest, PageMessageDeliveryFailsAndReportsUserFailure)
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objArray));

    EXPECT_CALL(objJniMtsServiceThread, ReportMoStatus(MO_ERROR_GENERIC, _, _, SLOT_ID)).Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, PageMessageDeliveryFailsAndReportsFallback)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_406);

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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_POLICY_FOR_EXPIRY_TIMER_F_INT, _))
            .WillByDefault(Return(MO_ERROR_FALLBACK));

    EXPECT_CALL(objJniMtsServiceThread, ReportMoStatus(MO_ERROR_FALLBACK, _, _, SLOT_ID)).Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, DataConnectionLostThenRemoveAllMessages)
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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);

    pMtsMessageController->ClearAllMessages();
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
}

TEST_F(MtsMessageControllerTest, TerminateAllMessagesDoesNothingWhenThereIsNoMessage)
{
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
    pMtsMessageController->ClearAllMessages();
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
            GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_COUNT_INT, _))
            .WillByDefault(Return(5));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT, _))
            .WillByDefault(Return(45));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
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

    EXPECT_CALL(objMockMessage, GetStatusCode())
            .Times(6)
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_202));
    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);

    // Timer_TimerExpired() with nullptr should not impact `m_piRetryAfterTimer`
    pMtsMessageController->Timer_TimerExpired(nullptr);
    pMtsMessageController->Timer_TimerExpired(&objTimer);

    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);

    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
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
            GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_COUNT_INT, _))
            .WillByDefault(Return(3));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT, _))
            .WillByDefault(Return(45));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
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

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
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
            GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_COUNT_INT, _))
            .WillByDefault(Return(3));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT, _))
            .WillByDefault(Return(45));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
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

    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
    pMtsMessageController->Timer_TimerExpired(&objTimer);

    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
    pMtsMessageController->Timer_TimerExpired(&objTimer);

    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
    pMtsMessageController->Timer_TimerExpired(&objTimer);
}

TEST_F(MtsMessageControllerTest, ProcessPendingRpDataFromNetwork)
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

    ByteArray objFirstRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objFirstRpData.Append((IMS_BYTE)0x03);     // message reference
    objFirstRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    // The second RP-DATA is queued because the first RP-DATA is still processing.
    ByteArray objSecondRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objSecondRpData.Append((IMS_BYTE)0x04);     // message reference
    objSecondRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ByteArray* pFirstRpAck = new ByteArray((IMS_BYTE)0x02);  // message type indicator(RP-ACK)
    pFirstRpAck->Append((IMS_BYTE)0x03);                     // message reference
    pFirstRpAck->Append((IMS_BYTE)0x0F);                     // other required information elements

    ByteArray* pSecondRpAck = new ByteArray((IMS_BYTE)0x02);  // message type indicator(RP-ACK)
    pSecondRpAck->Append((IMS_BYTE)0x04);                     // message reference
    pSecondRpAck->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService(bEmergency))
            .WillByDefault(Return(&objMockCoreService));
    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMoServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
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

    EXPECT_CALL(objMockMessageBodyPart, GetContent())
            .WillOnce(ReturnRef(objFirstRpData))
            .WillRepeatedly(ReturnRef(objSecondRpData));
    EXPECT_CALL(objJniMtsServiceThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(2);
    EXPECT_CALL(objJniMtsServiceThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(2);

    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage);
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pFirstRpAck, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
    ImsMessage objMsg(0, 0, 0);
    EXPECT_TRUE(pMtsMessageController->SendMessage(objMsg));
    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pSecondRpAck, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, CheckMoSmsPendingStateWhenNoResponse)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objCallIdHeaders;
    objCallIdHeaders.Append("0057f183b-245fdcb9@192.168.45.139");
    const AString strCallId(SipHeaderName::CALL_ID);

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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));

    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);

    EXPECT_EQ(pMtsMessageController->HasPendingMoSms(), IMS_TRUE);
}

TEST_F(MtsMessageControllerTest, CheckMoSmsPendingStateWhenPageMessageDelivered)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objCallIdHeaders;
    objCallIdHeaders.Append("0057f183b-245fdcb9@192.168.45.139");
    const AString strCallId(SipHeaderName::CALL_ID);

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
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(&objMockPageMessage));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    pMtsMessageController->ProcessMoSms(
            SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress, SEQ_ID, bEmergency);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);

    EXPECT_EQ(pMtsMessageController->HasPendingMoSms(), IMS_FALSE);
}

}  // namespace android
