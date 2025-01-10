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
#include "IConfiguration.h"
#include "IImsRadio.h"
#include "IIpcan.h"
#include "IPageMessage.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "ImsServiceConfig.h"
#include "ImsServiceConfigTypeDef.h"
#include "IuMtsService.h"
#include "MockICarrierConfig.h"
#include "MockIImsAos.h"
#include "MockIImsAosInfo.h"
#include "MockIMtsContext.h"
#include "MockIMtsMessageController.h"
#include "MockIReference.h"
#include "MtsDef.h"
#include "MtsService.h"
#include "MtsServiceState.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "TestConnector.h"
#include "TestImsRadioService.h"
#include "TestPhoneInfoService.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;
const IMS_SINT32 SEQ_ID_1 = 1;
const IMS_SINT32 SEQ_ID_2 = 2;
const IMS_UINTP FAKE_ADDRESS = 1;

class MtsServiceTest : public ::testing::Test
{
public:
    MockIImsAos objMockIImsAos;
    MockIImsAos objMockIImsEmergencyAos;
    MockIImsAosInfo objMockIImsAosInfo;
    MockIMtsContext objContext;
    MockIMtsMessageController objMessageController;
    MockICoreService objEmergencyCoreService;
    MtsService* pMtsService;

    TestConfigService objConfigService;
    TestImsRadioService objImsRadioService;
    TestPhoneInfoService objPhoneInfoService;
    TestConnector objConnector;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetMessageController).WillByDefault(ReturnRef(objMessageController));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &objImsRadioService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);

        ON_CALL(objConfigService.GetMockCarrierConfig(),
                GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL, _))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(objConfigService.GetMockCarrierConfig(),
                GetBoolean(CarrierConfig::ImsSms::KEY_SMS_ALLOW_IMSI_BASED_SIP_URI_BOOL, _))
                .WillByDefault(Return(IMS_FALSE));

        objConnector.SetCoreService(ImsServiceConfig::GetServiceName(ImsServiceId::MTS_EMERGENCY),
                &objEmergencyCoreService);

        pMtsService = new MtsService(objContext);
        pMtsService->SetIImsAos(&objMockIImsAos);
        pMtsService->SetIImsEmergencyAos(&objMockIImsEmergencyAos);
        pMtsService->InitMtsServiceState();

        ON_CALL(objMockIImsAos, GetAosInfo()).WillByDefault(Return(&objMockIImsAosInfo));
        ON_CALL(objMockIImsEmergencyAos, GetAosInfo()).WillByDefault(Return(&objMockIImsAosInfo));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        delete pMtsService;
    }
};

TEST_F(MtsServiceTest, GetICoreServiceReturnsNotNull)
{
    EXPECT_NE(pMtsService->GetICoreService(IMS_FALSE), nullptr);
}

TEST_F(MtsServiceTest, CoreServicePageMessageReceived)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    EXPECT_CALL(objMessageController, NotifyMtSms(piMessage)).Times(1);

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);

    ByteArray objRpData((IMS_BYTE)0x01);  // message type indicator(RP-MT-DATA)
    objRpData.Append((IMS_BYTE)0x03);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    AString strContent;
    strContent.Attach(
            reinterpret_cast<const IMS_CHAR*>(objRpData.GetData()), objRpData.GetLength());
    ByteArray objContent = strContent.ToBase64();

    pMtsService->ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, objContent);
}

TEST_F(MtsServiceTest, CoreServicePageMessageReceivedWithInvalidParameter)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    EXPECT_CALL(objMessageController, NotifyMtSms(piMessage)).Times(0);

    pMtsService->CoreService_PageMessageReceived(piCoreService, IMS_NULL);
    pMtsService->CoreService_PageMessageReceived(IMS_NULL, piMessage);
}

TEST_F(MtsServiceTest, CoreServiceReferenceReceivedDoesNothing)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IReference* piReference = reinterpret_cast<IReference*>(FAKE_ADDRESS);
    pMtsService->CoreService_ReferenceReceived(piCoreService, piReference);
}

TEST_F(MtsServiceTest, CoreServiceServiceClosedDoesNothing)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IReasonInfo* piReasonInfo = reinterpret_cast<IReasonInfo*>(FAKE_ADDRESS);
    pMtsService->CoreService_ServiceClosed(piCoreService, piReasonInfo);
}

TEST_F(MtsServiceTest, CoreServiceSessionInvitationReceivedDoesNothing)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    ISession* piSession = reinterpret_cast<ISession*>(FAKE_ADDRESS);
    pMtsService->CoreService_SessionInvitationReceived(piCoreService, piSession);
}

TEST_F(MtsServiceTest, CoreServiceUnsolicitedNotifyReceivedDoesNothing)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IMessage* piNotify = reinterpret_cast<IMessage*>(FAKE_ADDRESS);
    pMtsService->CoreService_UnsolicitedNotifyReceived(piCoreService, piNotify);
}

TEST_F(MtsServiceTest, CoreServiceCapabilityQueryReceivedDoesNothing)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    ICapabilities* piCapabilities = reinterpret_cast<ICapabilities*>(FAKE_ADDRESS);
    pMtsService->CoreService_CapabilityQueryReceived(piCoreService, piCapabilities);
}

TEST_F(MtsServiceTest, GetStateReturnsReadyAfterAosConnected)
{
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetState(), STATE_READY);
}

TEST_F(MtsServiceTest, GetStateReturnsNotreadyAfterAosConnecting)
{
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMessageController, ClearAllMessages()).Times(1);

    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Disconnected(ImsAosReason::NONE);
    pMtsService->ImsAos_Connecting();
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetState(), STATE_NOTREADY);
}

TEST_F(MtsServiceTest, GetStateReturnsNotreadyAfterAosDisconnected)
{
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMessageController, ClearAllMessages()).Times(1);

    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Disconnected(ImsAosReason::NONE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetState(), STATE_NOTREADY);
}

TEST_F(MtsServiceTest, GetStateReturnsReadyAfterAosDisconnecting)
{
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Disconnecting(ImsAosReason::NONE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetState(), STATE_READY);
}

TEST_F(MtsServiceTest, GetStateReturnsLimitedAfterAosSuspended)
{
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMessageController, ClearAllMessages()).Times(1);

    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Suspended(ImsAosReason::NONE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetState(), STATE_LIMITED);
}

TEST_F(MtsServiceTest, GetStateReturnsReadyAfterAosResumed)
{
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMessageController, ClearAllMessages()).Times(1);

    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Suspended(ImsAosReason::NONE);
    pMtsService->ImsAos_Resumed();
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetState(), STATE_READY);
}

TEST_F(MtsServiceTest, RequestRegistrationRecoveryToAos)
{
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::PCSCF_NEXT)).Times(1);
    pMtsService->RequestRegistrationRecovery(ImsAosControl::PCSCF_NEXT);
}

TEST_F(MtsServiceTest, RequestRegistrationRecoveryWhenAosIsDetached)
{
    pMtsService->SetIImsAos(IMS_NULL);
    EXPECT_CALL(objMockIImsAos, Control(_)).Times(0);
    pMtsService->RequestRegistrationRecovery(ImsAosControl::PCSCF_NEXT);
}

TEST_F(MtsServiceTest, RequestRegisterWithNextPcscfToAos)
{
    IMS_UINT32 nRetryAfterValue = 32;
    EXPECT_CALL(objMockIImsAos, RegisterWithNextPcscf(nRetryAfterValue)).Times(1);
    pMtsService->RequestRegisterWithNextPcscf(nRetryAfterValue);
}

TEST_F(MtsServiceTest, RequestRegisterWithNextPcscfWhenAosIsDetached)
{
    IMS_UINT32 nRetryAfterValue = 32;
    pMtsService->SetIImsAos(IMS_NULL);
    EXPECT_CALL(objMockIImsAos, RegisterWithNextPcscf(nRetryAfterValue)).Times(0);
    pMtsService->RequestRegisterWithNextPcscf(nRetryAfterValue);
}

TEST_F(MtsServiceTest, SendNormalMoSmsWhenTrafficIsNotAllowed)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergency = IMS_FALSE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMessageController, NotifyMoSms(_, _, _, _, _)).Times(0);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    pMtsService->SendMoSms(eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_1, bEmergency);
}

TEST_F(MtsServiceTest, SendNormalMoSmsWhenTrafficIsAllowed)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergency = IMS_FALSE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMessageController, NotifyMoSms(_, _, _, _, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);

    pMtsService->SendMoSms(eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_2, bEmergency);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendNormalMoSmsAndTrafficConnectionFailed)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergency = IMS_FALSE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMessageController, NotifyMoSms(_, _, _, _, _)).Times(0);
    EXPECT_CALL(objMessageController, ClearAllMessages()).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);

    pMtsService->SendMoSms(eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_2, bEmergency);
    pMtsService->Traffic_OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO,
            IImsRadio::REASON_ACCESS_DENIED, 400, 2000);
}

TEST_F(MtsServiceTest, SendNormalMoSmsAndInvalidTrafficTypeIsAllowed)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergency = IMS_FALSE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMessageController, NotifyMoSms(_, _, _, _, _)).Times(0);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);

    pMtsService->SendMoSms(eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_2, bEmergency);
    // Invalid traffic type
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendNormalMoSmsAndGuardTimerIsActived)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergency = IMS_FALSE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMessageController, NotifyMoSms(_, _, _, _, _)).Times(2);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);

    pMtsService->SendMoSms(eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_1, bEmergency);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);

    pMtsService->SendMoSms(eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_2, bEmergency);
}

TEST_F(MtsServiceTest, InvalidGuardTimerExpired)
{
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), StopImsTraffic(_)).Times(0);
    // Invalid traffic type
    pMtsService->Traffic_GuardTimerExpired(IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendE911MoSmsWhenTrafficIsAllowed)
{
    AString strTargetAddress = "911";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergency = IMS_TRUE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMessageController, NotifyMoSms(_, _, _, _, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);
    EXPECT_CALL(objMockIImsEmergencyAos, Control(ImsAosControl::REGISTER_START)).Times(1);
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_TRUE));

    pMtsService->SendMoSms(eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_1, bEmergency);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendE911MoSmsWhenTrafficIsAllowedButDoNotUseEPDN)
{
    AString strTargetAddress = "911";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergency = IMS_TRUE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objMessageController, NotifyMoSms(_, _, _, _, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);
    EXPECT_CALL(objMockIImsEmergencyAos, Control(ImsAosControl::REGISTER_START)).Times(0);

    pMtsService->SendMoSms(eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_1, bEmergency);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendE911MoSmsAndGuardTimerIsActived)
{
    AString strTargetAddress = "911";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergency = IMS_TRUE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::KEY_SUPPORT_EMERGENCY_SMS_OVER_IMS_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMessageController, NotifyMoSms(_, _, _, _, _)).Times(2);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);
    EXPECT_CALL(objMockIImsEmergencyAos, Control(ImsAosControl::REGISTER_START)).Times(2);
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_TRUE));

    pMtsService->SendMoSms(eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_1, bEmergency);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MO);

    pMtsService->SendMoSms(eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_2, bEmergency);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsWhenTrafficIsAllowed)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    EXPECT_CALL(objMessageController, NotifyMtSms(_)).Times(2);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(2);

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedE911MtSmsWhenTrafficIsAllowed)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_TRUE);
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    EXPECT_CALL(objMessageController, NotifyMtSms(_)).Times(2);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(2);

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MT);

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsThroughIWLAN)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockIImsAosInfo, GetIpcanType()).WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    EXPECT_CALL(objMessageController, NotifyMtSms(piMessage)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsThroughHSPA)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockIImsAosInfo, GetIpcanType()).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_CALL(objMessageController, NotifyMtSms(piMessage)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetworkType())
            .WillRepeatedly(Return(INetworkWatcher::RADIOTECH_TYPE_HSPA));

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsThroughLTE)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockIImsAosInfo, GetIpcanType()).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_CALL(objMessageController, NotifyMtSms(piMessage)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetworkType())
            .WillRepeatedly(Return(INetworkWatcher::RADIOTECH_TYPE_LTE));

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsThroughNR)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockIImsAosInfo, GetIpcanType()).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_CALL(objMessageController, NotifyMtSms(piMessage)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetworkType())
            .WillRepeatedly(Return(INetworkWatcher::RADIOTECH_TYPE_NR));

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsThroughUnknownNetworkType)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockIImsAosInfo, GetIpcanType()).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_CALL(objMessageController, NotifyMtSms(piMessage)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetworkType())
            .WillRepeatedly(Return(INetworkWatcher::RADIOTECH_TYPE_UNKNOWN));

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, TrafficGuardTimerExpiredAndStopImsTraffic)
{
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), StopImsTraffic(_)).Times(1);

    pMtsService->Traffic_GuardTimerExpired(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);
}

}  // namespace android
