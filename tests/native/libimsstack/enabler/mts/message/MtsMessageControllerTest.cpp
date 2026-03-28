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
#include "IIpcan.h"
#include "IPageMessage.h"
#include "ImsTypeDef.h"
#include "message/IMtsMessage.h"
#include "MockIImsAos.h"
#include "MockIImsAosInfo.h"
#include "IuMtsApp.h"
#include "JniEnablerConnector.h"
#include "MockICoreService.h"
#include "MockIJniEnabler.h"
#include "MockIJniMtsAppThread.h"
#include "MockIMessage.h"
#include "MockIMessageBodyPart.h"
#include "MockIMtsContext.h"
#include "MockIMtsService.h"
#include "MockIMtsServiceState.h"
#include "MockIPageMessage.h"
#include "MockISipHeader.h"
#include "MockISipMessage.h"
#include "MockISipMessageBodyPart.h"
#include "MockITimer.h"
#include "MtsDef.h"
#include "PlatformContext.h"
#include "SipAddress.h"
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

namespace android
{

const IMS_SINT32 SLOT_ID = 0;
const IMS_SINT32 SEQ_ID = 1;
const IMS_UINT32 RETRY_COUNT = 0;

class TestMtsMessageController : public MtsMessageController
{
public:
    explicit TestMtsMessageController(IN IMtsContext& objContext) :
            MtsMessageController(objContext)
    {
    }
    virtual ~TestMtsMessageController() override {}

    IMS_UINT32 GetMessageCount() const { return m_objMsgList.GetSize(); }
    IMS_BOOL SendMessage(IN ImsMessage& objMsg) { return OnMessage(objMsg); }
    IMtsMessage* GetMessageAt(IN IMS_UINT32 nIndex) const
    {
        if (nIndex >= m_objMsgList.GetSize())
        {
            return IMS_NULL;
        }
        return m_objMsgList.GetAt(nIndex);
    }
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
    MockIJniMtsAppThread objJniMtsAppThread;
    MockILocationInfo objMockILocationInfo;
    MockILocationProperties objMockILocationProperties;
    MockIMessage objMockMessage;
    MockIMessageBodyPart objMockMessageBodyPart;
    MockIMtsContext objContext;
    MockIMtsService objMockMtsService;
    MockIMtsService objMockEmergencyMtsService;
    MockIMtsServiceState objMockMtsServiceState;
    MockIPageMessage objMockPageMessage;
    MockISipHeader objMockSipHeader;
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
        ON_CALL(objContext, GetService(MtsServiceType::NORMAL))
                .WillByDefault(ReturnRef(objMockMtsService));
        ON_CALL(objContext, GetService(MtsServiceType::EMERGENCY))
                .WillByDefault(ReturnRef(objMockEmergencyMtsService));
        pMtsMessageController = new TestMtsMessageController(objContext);

        pMtsDynamicLoader = new MtsDynamicLoader(objContext);
        ON_CALL(objContext, GetDynamicLoader).WillByDefault(ReturnRef(*pMtsDynamicLoader));
        ON_CALL(objContext, GetJniAppThread).WillByDefault(Return(&objJniMtsAppThread));

        pConnector = &JniEnablerConnector::GetInstance();
        pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTS, &objMockJniEnabler);
        ON_CALL(objMockJniEnabler, GetJniThread).WillByDefault(Return(&objJniMtsAppThread));
        ON_CALL(objMockMessage, GetMessage()).WillByDefault(Return(&objMockSipMessage));
        ON_CALL(objMockMtsService, GetICoreService()).WillByDefault(Return(&objMockCoreService));
        ON_CALL(objMockEmergencyMtsService, GetICoreService())
                .WillByDefault(Return(&objMockCoreService));
        ON_CALL(objMockMtsService, GetIMtsServiceState())
                .WillByDefault(Return(&objMockMtsServiceState));
        ON_CALL(objMockEmergencyMtsService, GetIMtsServiceState())
                .WillByDefault(Return(&objMockMtsServiceState));
        ON_CALL(objMockCoreService, CreatePageMessage(_, _))
                .WillByDefault(Return(&objMockPageMessage));
    }

    virtual void TearDown() override
    {
        delete pMtsDynamicLoader;
        delete pMtsMessageController;
        delete pConnector;
    }

    void SetUpForEmergencyMoSms(MtsServiceType eServiceType, const SipAddress& objSipAddress,
            const ImsVector<IMS_SINT32>& objArray)
    {
        if (eServiceType == MtsServiceType::EMERGENCY)
        {
            ON_CALL(objMockEmergencyMtsService, GetICoreService())
                    .WillByDefault(Return(&objMockCoreService));
        }
        else
        {
            ON_CALL(objMockMtsService, GetICoreService()).WillByDefault(Return(&objMockCoreService));
        }
        ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
        ON_CALL(objMockCoreService, CreatePageMessage(_, _))
                .WillByDefault(Return(&objMockPageMessage));
        ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
        ON_CALL(objMockMessage, GetMessage()).WillByDefault(Return(&objMockSipMessage));
        ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
        ON_CALL(objConfigService.GetMockCarrierConfig(),
                GetIntArray(CarrierConfig::ImsSms::
                                    KEY_SMS_GEOLOCATION_PIDF_IN_SIP_MESSAGE_SUPPORT_INT_ARRAY,
                        _))
                .WillByDefault(Return(objArray));
        ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
        ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
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

    void SetUpForReceivingRpMessage(ByteArray& objRpData)
    {
        AString strContentType = "application/vnd.3gpp.sms";
        ImsList<IMessageBodyPart*> objMessageBodies;
        objMessageBodies.Append(&objMockMessageBodyPart);

        ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
                .WillByDefault(Return(&objMockMessage));
        ImsList<AString> objToHeaders;
        objToHeaders.Append("sip:user@example.com");
        ON_CALL(objMockMessage, GetHeaders(AString("To"))).WillByDefault(Return(objToHeaders));
        ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
        ON_CALL(objMockMessageBodyPart, GetHeader(AString("Content-Type")))
                .WillByDefault(Return(strContentType));
        ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));
        ImsList<AString> objFromHeaders;
        objFromHeaders.Append("sip:smsgw@example.com");
        ON_CALL(objMockPageMessage, GetRemoteUserId()).WillByDefault(Return(ImsList<AString>()));
        ON_CALL(objMockMessage, GetHeaders(AString(SipHeaderName::FROM)))
                .WillByDefault(Return(objFromHeaders));
        ON_CALL(objConfigService.GetMockCarrierConfig(),
                GetBoolean(CarrierConfig::ImsSms::KEY_SMS_IN_REPLY_TO_VALIDATION_BOOL, _))
                .WillByDefault(Return(IMS_FALSE));
    }

    void SetUpForSendingMoSms()
    {
        ON_CALL(objMockCoreService, CreatePageMessage(_, _))
                .WillByDefault(Return(&objMockPageMessage));
        ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
        ON_CALL(objMockMessage, GetMessage()).WillByDefault(Return(&objMockSipMessage));
        ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
        ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
        ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    }

    ByteArray CreateSmsPdu(IMS_SINT32 messageType, IMS_BYTE messageReference)
    {
        ByteArray pdu;
        pdu.Append(static_cast<IMS_BYTE>(messageType));
        pdu.Append(messageReference);
        pdu.Append((IMS_BYTE)0x0F);
        return pdu;
    }
};

TEST_F(MtsMessageControllerTest, Constructor)
{
    ASSERT_NE(pMtsMessageController, nullptr);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithoutEmergencyFlag)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithEmergencyFlag)
{
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    MtsServiceType eServiceType = MtsServiceType::EMERGENCY;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objCallIdHeaders;
    objCallIdHeaders.Append("0057f183b-245fdcb9@192.168.45.139");
    const AString strCallId(SipHeaderName::CALL_ID);
    ImsVector<IMS_SINT32> objArray;

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::
                                KEY_SMS_GEOLOCATION_PIDF_IN_SIP_MESSAGE_SUPPORT_INT_ARRAY,
                    _))
            .WillByDefault(Return(objArray));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithoutTargetAddress)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress("");

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithoutRPDU)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "tel:+12345678901";

    ByteArray* pContent = new ByteArray(ByteArray::ConstNull());

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToGetICoreService)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "tel:+12345678901";

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsService, GetICoreService()).WillByDefault(Return(nullptr));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToGetUri)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToCreatePageMessage)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockCoreService, CreatePageMessage(_, _)).WillByDefault(Return(nullptr));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithContentTransferEncoding)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::REQUEST_DISPOSITION), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::CONTENT_TRANSFER_ENCODING), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithoutContentTransferEncoding)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::REQUEST_DISPOSITION), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::CONTENT_TRANSFER_ENCODING), _))
            .Times(0)
            .WillOnce(Return(IMS_SUCCESS));

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToGetNextRequest)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(nullptr));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToAddHeader)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndFailToSendPageMessage)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsAndPageMessageDeliveryFailed)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_400));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, SendMoSmsAndReceiveAck)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
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

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, NormalMoSmsAndInReplyToMismatched)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
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

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
    EXPECT_CALL(objMockPageMessage, Reject(SipStatusCode::SC_488, _)).Times(1);
    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, SendMoSmsAndReceiveAckWithout202Accepted)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
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
    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);
    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
}

TEST_F(MtsMessageControllerTest, SendMoSmsWithSMMAAndFailToFormDestination)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_GENERIC, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, SendMoSmsWithGeoLocationInformation)
{
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    MtsServiceType eServiceType = MtsServiceType::EMERGENCY;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    MockISipMessageBodyPart objMockISipMessageBodyPart;
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::
                                KEY_SMS_GEOLOCATION_PIDF_IN_SIP_MESSAGE_SUPPORT_INT_ARRAY,
                    _))
            .WillByDefault(Return(objArray));
    ON_CALL(objMockMtsService, IsWlan()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
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
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, SendMoSmsWithOutGeoLocationInformation)
{
    IMS_BOOL bEmergency = IMS_TRUE;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    MtsServiceType eServiceType = MtsServiceType::EMERGENCY;
    objSipAddress.Create(strTargetAddress);
    MockISipMessageBodyPart objMockISipMessageBodyPart;
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR);
    objArray.Push(CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR);
    objArray.Push(CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::
                                KEY_SMS_GEOLOCATION_PIDF_IN_SIP_MESSAGE_SUPPORT_INT_ARRAY,
                    _))
            .WillByDefault(Return(objArray));
    ON_CALL(objMockEmergencyMtsService, IsWlan()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetMessage()).WillByDefault(Return(&objMockSipMessage));
    ON_CALL(objMockSipMessage, CreateBodyPart()).WillByDefault(Return(&objMockISipMessageBodyPart));

    EXPECT_CALL(objMockISipMessageBodyPart, SetContent(_)).Times(0);
    EXPECT_CALL(objMockISipMessageBodyPart, SetHeader(_, _, _)).Times(0);
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::REQUEST_DISPOSITION), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::GEOLOCATION), _))
            .Times(0)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::GEOLOCATION_ROUTING), _))
            .Times(0)
            .WillOnce(Return(IMS_SUCCESS));

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergency, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, ReceiveMtSmsAndSendAck)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
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

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ReceiveMtSmsAndSendAckFailed)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
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

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ReceiveMtSmsAndSendAck3gpp2)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
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

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP2, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP2, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP2, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ProcessMtSmsWhenMtServiceBlocked)
{
    MtsServiceType eServiceType = MtsServiceType::NORMAL;

    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_ERROR_CODE_WHEN_MT_SMS_BLOCKED_INT, _))
            .WillByDefault(Return(SipStatusCode::SC_480));

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    EXPECT_CALL(objMockPageMessage, Reject(SipStatusCode::SC_480, _)).Times(1);
    EXPECT_CALL(objMockPageMessage, Destroy()).Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, ProcessMtSmsAndRespondSpecifiedErrorWhenMtServiceBlocked)
{
    MtsServiceType eServiceType = MtsServiceType::NORMAL;

    ON_CALL(objMockMtsService, GetIMtsServiceState())
            .WillByDefault(Return(&objMockMtsServiceState));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_ERROR_CODE_WHEN_MT_SMS_BLOCKED_INT, _))
            .WillByDefault(Return(SipStatusCode::SC_415));

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    EXPECT_CALL(objMockPageMessage, Reject(SipStatusCode::SC_415, _)).Times(1);
    EXPECT_CALL(objMockPageMessage, Destroy()).Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, ProcessMtSmsAndFailToGetICoreService)
{
    MtsServiceType eServiceType = MtsServiceType::NORMAL;

    ON_CALL(objMockMtsService, GetICoreService()).WillByDefault(Return(nullptr));
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    EXPECT_CALL(objMockPageMessage, Reject(SipStatusCode::SC_480, _)).Times(1);
    EXPECT_CALL(objMockPageMessage, Destroy()).Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageIfIMessageIsNull)
{
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));

    EXPECT_CALL(objMockPageMessage, Reject(400, _)).Times(1);
    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageBecauseOfNoToHeader)
{
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsList<AString> objHeaders;

    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));

    EXPECT_CALL(objMockPageMessage, Reject(400, _)).Times(1);
    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageBecauseOfNoMessageBody)
{
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();

    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));

    EXPECT_CALL(objMockPageMessage, Reject(400, _)).Times(1);
    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, FailInProcessReceivedMessageBecauseOfInvalidSmsFormat)
{
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    AString strContentType = "unknown";
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));

    EXPECT_CALL(objMockPageMessage, Reject(415, _)).Times(1);
    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, ReceivedTooLargeRpdu)
{
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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

    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, CannotFindMatchedMtsMessageInPageMessageDelivered)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x02);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(objJniMtsAppThread, ReportMoStatus(_, _, _, SLOT_ID)).Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(IMS_NULL);
}

TEST_F(MtsMessageControllerTest, NoReceivedResponsesInPageMessageDelivered)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));

    EXPECT_CALL(objJniMtsAppThread, ReportMoStatus(_, _, _, SLOT_ID)).Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, CannotFindMatchedMtsMessageInPageMessageDeliveryFailed)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, Destroy()).WillByDefault(Return());

    EXPECT_CALL(objJniMtsAppThread, ReportMoStatus(_, _, _, SLOT_ID)).Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDeliveryFailed(IMS_NULL);
}

TEST_F(MtsMessageControllerTest, PageMessageDeliveryFailsAndReportsUserFailure)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(nullptr));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objArray));

    EXPECT_CALL(objJniMtsAppThread, ReportMoStatus(MO_ERROR_GENERIC, _, _, SLOT_ID)).Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, PageMessageDeliveryFailsAndReportsFallback)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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

    EXPECT_CALL(objJniMtsAppThread, ReportMoStatus(MO_ERROR_FALLBACK, _, _, SLOT_ID)).Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, SmmaPageMessageDeliveryFailsAndReportsError)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpData));

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);

    ImsVector<IMS_SINT32> objSmmaGenericErrorCodes;
    objSmmaGenericErrorCodes.Push(SipStatusCode::SC_406);
    pContent = new ByteArray((IMS_BYTE)0x06);  // message type indicator(RP-SMMA)
    pContent->Append((IMS_BYTE)0x02);          // message reference

    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_SMMA_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objSmmaGenericErrorCodes));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_406));

    EXPECT_CALL(objJniMtsAppThread, ReportMoStatus(MO_ERROR_GENERIC, _, _, SLOT_ID)).Times(1);

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, DataConnectionLostThenRemoveAllMessages)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
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
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
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
            .Times(7)
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_407))
            .WillOnce(Return(SipStatusCode::SC_202));
    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);

    // Timer_TimerExpired() with nullptr should not impact `m_piRetryAfterTimer`
    pMtsMessageController->Timer_TimerExpired(nullptr);
    pMtsMessageController->Timer_TimerExpired(&objTimer);

    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, ReachRetryAfterMaxDuration)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
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

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ReachRetryAfterMaxCount)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
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

    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_ERROR_RETRY, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
    pMtsMessageController->Timer_TimerExpired(&objTimer);

    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
    pMtsMessageController->Timer_TimerExpired(&objTimer);

    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
    pMtsMessageController->Timer_TimerExpired(&objTimer);
}

TEST_F(MtsMessageControllerTest, ProcessPendingRpDataFromNetwork)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
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
            .WillRepeatedly(ReturnRef(objFirstRpData));
    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(2);
    EXPECT_CALL(objJniMtsAppThread,
            ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, SLOT_ID))
            .Times(2);

    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pFirstRpAck,
            strTargetAddress, SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
    EXPECT_CALL(objMockMessageBodyPart, GetContent())
            .WillRepeatedly(ReturnRef(objSecondRpData));
    ImsMessage objMsg(0, 0, 0);
    EXPECT_TRUE(pMtsMessageController->SendMessage(objMsg));
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pSecondRpAck,
            strTargetAddress, SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, CheckMoSmsPendingStateWhenNoResponse)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);

    EXPECT_EQ(pMtsMessageController->HasPendingMoSms(), IMS_TRUE);
}

TEST_F(MtsMessageControllerTest, CheckMoSmsPendingStateWhenPageMessageDelivered)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
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
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockMessage, GetHeaders(strCallId)).WillByDefault(Return(objCallIdHeaders));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, SetListener(_)).WillByDefault(Return());
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);

    EXPECT_EQ(pMtsMessageController->HasPendingMoSms(), IMS_FALSE);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithEmergencyAndContactHeader)
{
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    MtsServiceType eServiceType = MtsServiceType::EMERGENCY;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strContactHeaderValue("sip:contact");

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);
    pContent->Append((IMS_BYTE)0x03);
    pContent->Append((IMS_BYTE)0x0F);

    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockCoreService, GetContactHeader(_, _, _)).WillByDefault(Return(&objMockSipHeader));
    ON_CALL(objMockSipHeader, ToStringWithoutName()).WillByDefault(Return(strContactHeaderValue));
    ON_CALL(objMockMessage, GetMessage()).WillByDefault(Return(&objMockSipMessage));
    ON_CALL(objMockSipHeader, Destroy()).WillByDefault(Return());

    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::REQUEST_DISPOSITION), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockSipMessage, SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeaderValue, _))
            .Times(1)
            .WillOnce(Return(IMS_FAILURE));

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsWithEmergencyAndNoEPdn)
{
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);
    pContent->Append((IMS_BYTE)0x03);
    pContent->Append((IMS_BYTE)0x0F);

    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockPageMessage, Send(_, _)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::REQUEST_DISPOSITION), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockSipMessage, SetHeader(ISipHeader::CONTACT_NORMAL, _, _)).Times(0);

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, ProcessMoSmsFailToGetContactHeader)
{
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    MtsServiceType eServiceType = MtsServiceType::EMERGENCY;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);
    pContent->Append((IMS_BYTE)0x03);
    pContent->Append((IMS_BYTE)0x0F);

    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetNextRequest()).WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, AddHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMockCoreService, GetContactHeader(_, _, _)).WillByDefault(Return(IMS_NULL));

    EXPECT_CALL(objMockMessage, AddHeader(AString(SipHeaderName::REQUEST_DISPOSITION), _))
            .Times(1)
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMockSipMessage, SetHeader(ISipHeader::CONTACT_NORMAL, _, _)).Times(0);

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
}

TEST_F(MtsMessageControllerTest, NotifyEmergencySmsStateToAos)
{
    // Part 1: Send an emergency SMS and verify NotifyEmergencySmsStateToAos(true) is called.
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    MtsServiceType eServiceType = MtsServiceType::EMERGENCY;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsVector<IMS_SINT32> objArray;  // for geolocation pidf config

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    SetUpForEmergencyMoSms(eServiceType, objSipAddress, objArray);

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_TRUE)).Times(1);

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);

    // Part 2: Receive an ACK for the emergency SMS and verify NotifyEmergencySmsStateToAos(false)
    MtsServiceType eNormalServiceType = MtsServiceType::NORMAL;
    AString strContentType = "application/vnd.3gpp.sms";
    ImsList<IMessageBodyPart*> objMessageBodies;
    MockIMessageBodyPart objMtMessageBodyPart;
    objMessageBodies.Append(&objMtMessageBodyPart);

    ByteArray objRpAck((IMS_BYTE)0x03);  // message type indicator(RP-ACK from network)
    objRpAck.Append((IMS_BYTE)0x03);     // message reference
    objRpAck.Append((IMS_BYTE)0x0F);

    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));

    ImsList<AString> objToHeaders;
    objToHeaders.Append("sip:user@example.com");
    ON_CALL(objMockMessage, GetHeaders(AString("To"))).WillByDefault(Return(objToHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMtMessageBodyPart, GetHeader(AString("Content-Type")))
            .WillByDefault(Return(strContentType));
    ON_CALL(objMtMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpAck));

    // For GetSmsgwFromReceivedMessage
    ImsList<AString> objFromHeaders;
    objFromHeaders.Append("sip:smsgw@example.com");
    ON_CALL(objMockPageMessage, GetRemoteUserId()).WillByDefault(Return(ImsList<AString>()));
    ON_CALL(objMockMessage, GetHeaders(AString(SipHeaderName::FROM)))
            .WillByDefault(Return(objFromHeaders));

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_FALSE)).Times(1);

    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eNormalServiceType);
}

TEST_F(MtsMessageControllerTest, NotifyEmergencySmsStateToAosOverNormalService)
{
    // Part 1: Send an emergency SMS and verify NotifyEmergencySmsStateToAos(true) is called.
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsVector<IMS_SINT32> objArray;  // for geolocation pidf config

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);
    pContent->Append((IMS_BYTE)0x03);
    pContent->Append((IMS_BYTE)0x0F);

    SetUpForEmergencyMoSms(eServiceType, objSipAddress, objArray);

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_TRUE)).Times(1);

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);

    // Part 2: Receive an ACK for the emergency SMS and verify NotifyEmergencySmsStateToAos(false)
    MtsServiceType eNormalServiceType = MtsServiceType::NORMAL;
    AString strContentType = "application/vnd.3gpp.sms";
    ImsList<IMessageBodyPart*> objMessageBodies;
    MockIMessageBodyPart objMtMessageBodyPart;
    objMessageBodies.Append(&objMtMessageBodyPart);

    ByteArray objRpAck((IMS_BYTE)0x03);
    objRpAck.Append((IMS_BYTE)0x03);
    objRpAck.Append((IMS_BYTE)0x0F);

    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));

    ImsList<AString> objToHeaders;
    objToHeaders.Append("sip:user@example.com");
    ON_CALL(objMockMessage, GetHeaders(AString("To"))).WillByDefault(Return(objToHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMtMessageBodyPart, GetHeader(AString("Content-Type")))
            .WillByDefault(Return(strContentType));
    ON_CALL(objMtMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpAck));

    ImsList<AString> objFromHeaders;
    objFromHeaders.Append("sip:smsgw@example.com");
    ON_CALL(objMockPageMessage, GetRemoteUserId()).WillByDefault(Return(ImsList<AString>()));
    ON_CALL(objMockMessage, GetHeaders(AString(SipHeaderName::FROM)))
            .WillByDefault(Return(objFromHeaders));

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_FALSE)).Times(1);

    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eNormalServiceType);
}

TEST_F(MtsMessageControllerTest, NotifyEmergencySmsStateToAosOnRpError)
{
    // Part 1: Send an emergency SMS
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    MtsServiceType eServiceType = MtsServiceType::EMERGENCY;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsVector<IMS_SINT32> objArray;  // for geolocation pidf config

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    SetUpForEmergencyMoSms(eServiceType, objSipAddress, objArray);

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_TRUE)).Times(1);

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);

    // Part 2: Receive an RP-ERROR for the emergency SMS and verify
    // NotifyEmergencySmsStateToAos(false)
    AString strContentType = "application/vnd.3gpp.sms";
    ImsList<IMessageBodyPart*> objMessageBodies;
    MockIMessageBodyPart objMtMessageBodyPart;
    objMessageBodies.Append(&objMtMessageBodyPart);

    ByteArray objRpError;
    objRpError.Append(static_cast<IMS_BYTE>(SMS_3GPP_MTI_RP_ERROR_FROM_N));
    objRpError.Append((IMS_BYTE)0x03);  // message reference
    objRpError.Append((IMS_BYTE)0x0F);

    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));

    ImsList<AString> objToHeaders;
    objToHeaders.Append("sip:user@example.com");
    ON_CALL(objMockMessage, GetHeaders(AString("To"))).WillByDefault(Return(objToHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMtMessageBodyPart, GetHeader(AString("Content-Type")))
            .WillByDefault(Return(strContentType));
    ON_CALL(objMtMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpError));

    // For GetSmsgwFromReceivedMessage
    ImsList<AString> objFromHeaders;
    objFromHeaders.Append("sip:smsgw@example.com");
    ON_CALL(objMockPageMessage, GetRemoteUserId()).WillByDefault(Return(ImsList<AString>()));
    ON_CALL(objMockMessage, GetHeaders(AString(SipHeaderName::FROM)))
            .WillByDefault(Return(objFromHeaders));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_IN_REPLY_TO_VALIDATION_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_FALSE)).Times(1);

    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, NotifyEmergencySmsStateToAosOnSipError)
{
    // Part 1: Send an emergency SMS
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    MtsServiceType eServiceType = MtsServiceType::EMERGENCY;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsVector<IMS_SINT32> objArray;  // for geolocation pidf config

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);
    pContent->Append((IMS_BYTE)0x03);
    pContent->Append((IMS_BYTE)0x0F);

    SetUpForEmergencyMoSms(eServiceType, objSipAddress, objArray);

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_TRUE)).Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);

    // Part 2: receive a SIP error.
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_404));
    ImsVector<IMS_SINT32> objErrorArray;
    objErrorArray.Push(SipStatusCode::SC_404);
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorArray));

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_FALSE)).Times(1);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}



TEST_F(MtsMessageControllerTest, NotifyEmergencySmsStateToAosOnRpErrorOverNormalService)
{
    // Part 1: Send an emergency SMS
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsVector<IMS_SINT32> objArray;  // for geolocation pidf config

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    pContent->Append((IMS_BYTE)0x03);                     // message reference
    pContent->Append((IMS_BYTE)0x0F);                     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    SetUpForEmergencyMoSms(eServiceType, objSipAddress, objArray);

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_TRUE)).Times(1);

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);

    // Part 2: Receive an RP-ERROR for the emergency SMS
    AString strContentType = "application/vnd.3gpp.sms";
    ImsList<IMessageBodyPart*> objMessageBodies;
    MockIMessageBodyPart objMtMessageBodyPart;
    objMessageBodies.Append(&objMtMessageBodyPart);

    ByteArray objRpError;
    objRpError.Append(static_cast<IMS_BYTE>(SMS_3GPP_MTI_RP_ERROR_FROM_N));
    objRpError.Append((IMS_BYTE)0x03);  // message reference
    objRpError.Append((IMS_BYTE)0x0F);

    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));

    ImsList<AString> objToHeaders;
    objToHeaders.Append("sip:user@example.com");
    ON_CALL(objMockMessage, GetHeaders(AString("To"))).WillByDefault(Return(objToHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMtMessageBodyPart, GetHeader(AString("Content-Type")))
            .WillByDefault(Return(strContentType));
    ON_CALL(objMtMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpError));

    // For GetSmsgwFromReceivedMessage
    ImsList<AString> objFromHeaders;
    objFromHeaders.Append("sip:smsgw@example.com");
    ON_CALL(objMockPageMessage, GetRemoteUserId()).WillByDefault(Return(ImsList<AString>()));
    ON_CALL(objMockMessage, GetHeaders(AString(SipHeaderName::FROM)))
            .WillByDefault(Return(objFromHeaders));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::ImsSms::KEY_SMS_IN_REPLY_TO_VALIDATION_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_FALSE)).Times(1);

    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
}

TEST_F(MtsMessageControllerTest, NotifyEmergencySmsStateToAosOnSipErrorOverNormalService)
{
    // Part 1: Send an emergency SMS
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    ImsVector<IMS_SINT32> objArray;  // for geolocation pidf config

    ByteArray* pContent = new ByteArray((IMS_BYTE)0x00);
    pContent->Append((IMS_BYTE)0x03);
    pContent->Append((IMS_BYTE)0x0F);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    SetUpForEmergencyMoSms(eServiceType, objSipAddress, objArray);

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_TRUE)).Times(1);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);

    // Part 2: receive a SIP error.
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_404));
    ImsVector<IMS_SINT32> objErrorArray;
    objErrorArray.Push(SipStatusCode::SC_404);
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorArray));

    EXPECT_CALL(objMockEmergencyMtsService, NotifyEmergencySmsStateToAos(IMS_FALSE)).Times(1);
    pMtsMessageController->PageMessageDeliveryFailed(&objMockPageMessage);
}

TEST_F(MtsMessageControllerTest, ClearStaleMtSmsAndProcessNext_WithQueuedMessage)
{
    // Setup for adding messages
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strContentType = "application/vnd.3gpp.sms";
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    AString strHeaders = "<sip:+12345678901@ims.google.com>,<sip:+12345678902@ims.google.com>";
    ImsList<AString> objHeaders = strHeaders.Split(TextParser::CHAR_COMMA);
    ImsList<IMessageBodyPart*> objMessageBodies = ImsList<IMessageBodyPart*>();
    objMessageBodies.Append(&objMockMessageBodyPart);

    ByteArray objRpData1((IMS_BYTE)0x01);
    objRpData1.Append((IMS_BYTE)0x03);
    objRpData1.Append((IMS_BYTE)0x0F);

    ByteArray objRpData2((IMS_BYTE)0x01);
    objRpData2.Append((IMS_BYTE)0x04);
    objRpData2.Append((IMS_BYTE)0x0F);

    ON_CALL(objMockMtsServiceState, IsMtServiceBlocked()).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));
    ON_CALL(objMockPageMessage, GetPreviousRequest(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(_)).WillByDefault(Return(objHeaders));
    ON_CALL(objMockMessage, GetBodyParts()).WillByDefault(Return(objMessageBodies));
    ON_CALL(objMockMessageBodyPart, GetHeader(_)).WillByDefault(Return(strContentType));

    // First message
    EXPECT_CALL(objMockMessageBodyPart, GetContent())
            .WillRepeatedly(ReturnRef(objRpData1));

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);

    // Process first message. It should be processed immediately.
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);


    EXPECT_CALL(objMockMessageBodyPart, GetContent())
            .WillRepeatedly(ReturnRef(objRpData2));

    // Process second message. It should be queued.
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 2);

    EXPECT_CALL(objMockMessageBodyPart, GetContent())
            .WillRepeatedly(ReturnRef(objRpData1));

    // Call the method to test. Stale message with message reference 3 is removed.
    pMtsMessageController->ClearStaleMtSmsAndProcessNext(3);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);

    EXPECT_CALL(objMockMessageBodyPart, GetContent())
            .WillRepeatedly(ReturnRef(objRpData2));

    IMtsMessage* piRemainingMessage = pMtsMessageController->GetMessageAt(0);
    ASSERT_NE(piRemainingMessage, IMS_NULL);
    EXPECT_EQ(piRemainingMessage->GetMessageReference(), 4);

    // Simulate the message loop processing the posted message.
    // This should trigger processing of the queued message.
    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    ImsMessage objMsg(0, 0, 0);
    pMtsMessageController->SendMessage(objMsg);

    // The message is still in the list until an ACK is sent, but it has been processed.
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);
}

TEST_F(MtsMessageControllerTest, CleanPendingMoOnReceivingRpAck)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pMoContent =
            new ByteArray(CreateSmsPdu(SMS_3GPP_MTI_RP_DATA_FROM_MS, 0x05));
    SetUpForSendingMoSms();
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pMoContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    ASSERT_EQ(pMtsMessageController->GetMessageCount(), 1);

    // Action: receive an RP-ACK with the same message reference.
    ByteArray objRpAck = CreateSmsPdu(SMS_3GPP_MTI_RP_ACK_FROM_N, 0x05);
    SetUpForReceivingRpMessage(objRpAck);

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);

    // Verification: the pending MO message should be cleaned up.
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
}

TEST_F(MtsMessageControllerTest, CleanPendingMoOnReceivingRpError)
{
    // Setup: send a MO SMS first, so there is a pending message.
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pMoContent =
            new ByteArray(CreateSmsPdu(SMS_3GPP_MTI_RP_DATA_FROM_MS, 0x05));
    SetUpForSendingMoSms();
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pMoContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    ASSERT_EQ(pMtsMessageController->GetMessageCount(), 1);

    // Action: receive an RP-ERROR with the same message reference.
    ByteArray objRpError;
    objRpError.Append(static_cast<IMS_BYTE>(SMS_3GPP_MTI_RP_ERROR_FROM_N));
    objRpError.Append((IMS_BYTE)0x05);  // same message reference
    objRpError.Append((IMS_BYTE)0x0F);

    SetUpForReceivingRpMessage(objRpError);

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);

    // Verification: the pending MO message should be cleaned up.
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
}

TEST_F(MtsMessageControllerTest, NoCleanPendingMoOnReceivingRpAckWithMismatchMr)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pMoContent =
            new ByteArray(CreateSmsPdu(SMS_3GPP_MTI_RP_DATA_FROM_MS, 0x05));
    SetUpForSendingMoSms();
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pMoContent, strTargetAddress,
            SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    ASSERT_EQ(pMtsMessageController->GetMessageCount(), 1);

    // Action: receive an RP-ACK with a different message reference.
    ByteArray objRpAck = CreateSmsPdu(SMS_3GPP_MTI_RP_ACK_FROM_N, 0x06);
    SetUpForReceivingRpMessage(objRpAck);

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);

    // Verification: the pending MO message should not be cleaned up.
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);
}

TEST_F(MtsMessageControllerTest, SendSecondMoSmsAfterRpAckForFirst)
{
    // 1. Send Message A
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray* pMoContentA =
            new ByteArray(CreateSmsPdu(SMS_3GPP_MTI_RP_DATA_FROM_MS, 0x05));
    SetUpForSendingMoSms();
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));

    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pMoContentA,
            strTargetAddress, SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    ASSERT_EQ(pMtsMessageController->GetMessageCount(), 1);

    // 2. Receive RP-ACK for Message A
    ByteArray objRpAck = CreateSmsPdu(SMS_3GPP_MTI_RP_ACK_FROM_N, 0x05);
    SetUpForReceivingRpMessage(objRpAck);

    // This call should clean the pending MO message.
    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);

    // 3. Send Message B to the same destination
    ByteArray* pMoContentB =
            new ByteArray(CreateSmsPdu(SMS_3GPP_MTI_RP_DATA_FROM_MS, 0x06));
    EXPECT_CALL(objJniMtsAppThread, ReportMoStatus(MO_ERROR_RETRY, _, _, _)).Times(0);
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pMoContentB,
            strTargetAddress, SEQ_ID + 1, bEmergencyNumber, eServiceType, RETRY_COUNT);

    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);

    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_202));
    EXPECT_CALL(objJniMtsAppThread, ReportMoStatus(MO_SUCCESS, _, _, _)).Times(1);
    pMtsMessageController->PageMessageDelivered(&objMockPageMessage);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
}

TEST_F(MtsMessageControllerTest, OutOfOrderRpAckForMoSms)
{
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    SetUpForSendingMoSms();

    // 1. Send Message A (MR=5) to destination 1
    AString strTargetAddress1 = "sip:+11111111111@ims.google.com";
    SipAddress objSipAddress1;
    objSipAddress1.Create(strTargetAddress1);
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress1));

    ByteArray* pMoContentA =
            new ByteArray(CreateSmsPdu(SMS_3GPP_MTI_RP_DATA_FROM_MS, 0x05));
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pMoContentA,
            strTargetAddress1, SEQ_ID, bEmergencyNumber, eServiceType, RETRY_COUNT);
    ASSERT_EQ(pMtsMessageController->GetMessageCount(), 1);

    // 2. Send Message B (MR=6) to destination 2
    AString strTargetAddress2 = "sip:+22222222222@ims.google.com";
    SipAddress objSipAddress2;
    objSipAddress2.Create(strTargetAddress2);
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress2));

    ByteArray* pMoContentB =
            new ByteArray(CreateSmsPdu(SMS_3GPP_MTI_RP_DATA_FROM_MS, 0x06));
    pMtsMessageController->ProcessMoSms(SmsFormatType::SMSFORMAT_3GPP, pMoContentB,
            strTargetAddress2, SEQ_ID + 1, bEmergencyNumber, eServiceType, RETRY_COUNT);
    ASSERT_EQ(pMtsMessageController->GetMessageCount(), 2);

    // Setup for receiving ACKs
    ImsList<IMessageBodyPart*> objMessageBodies;
    objMessageBodies.Append(&objMockMessageBodyPart);

    // 3. Receive RP-ACK for Message B (MR=6)
    ByteArray objRpAckB = CreateSmsPdu(SMS_3GPP_MTI_RP_ACK_FROM_N, 0x06);
    SetUpForReceivingRpMessage(objRpAckB);
    ON_CALL(objMockMessageBodyPart, GetContent()).WillByDefault(ReturnRef(objRpAckB));

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);

    // 4. Verify Message B is cleaned up and Message A remains
    ASSERT_EQ(pMtsMessageController->GetMessageCount(), 1);
    IMtsMessage* piRemainingMessage = pMtsMessageController->GetMessageAt(0);
    ASSERT_NE(piRemainingMessage, nullptr);
    EXPECT_EQ(piRemainingMessage->GetMessageReference(), 5);

    // 5. Receive RP-ACK for Message A (MR=5)
    ByteArray objRpAckA = CreateSmsPdu(SMS_3GPP_MTI_RP_ACK_FROM_N, 0x05);
    SetUpForReceivingRpMessage(objRpAckA);

    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);

    // 6. Verify list is empty
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);
}

TEST_F(MtsMessageControllerTest, ProcessMtSmsAndAcceptFailed)
{
    MtsServiceType eServiceType = MtsServiceType::NORMAL;
    ByteArray objRpData = CreateSmsPdu(SMS_3GPP_MTI_RP_DATA_FROM_N, 0x01);
    SetUpForReceivingRpMessage(objRpData);
    SipAddress objSipAddress;
    objSipAddress.Create("sip:user@example.com");
    ON_CALL(objMockCoreService, GetAuthorizedUserId()).WillByDefault(ReturnRef(objSipAddress));

    // First MT SMS, Accept() fails
    ON_CALL(objMockPageMessage, Accept(_)).WillByDefault(Return(IMS_FAILURE));
    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(_, _, _)).Times(0);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 0);

    // Second MT SMS, Accept() succeeds
    ByteArray objRpData2 = CreateSmsPdu(SMS_3GPP_MTI_RP_DATA_FROM_N, 0x02);
    SetUpForReceivingRpMessage(objRpData2);
    ON_CALL(objMockPageMessage, Accept(_)).WillByDefault(Return(IMS_SUCCESS));
    EXPECT_CALL(objJniMtsAppThread, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _, SLOT_ID))
            .Times(1);
    pMtsMessageController->ProcessMtSms(&objMockPageMessage, eServiceType);
    // After processing, if an ACK is not sent back, the message stays in the list.
    // In this test, we are not sending an ACK back, so it should be in the list.
    EXPECT_EQ(pMtsMessageController->GetMessageCount(), 1);
}
}  // namespace android
