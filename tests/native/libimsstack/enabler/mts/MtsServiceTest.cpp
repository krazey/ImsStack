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
#include "Configuration.h"
#include "IImsRadio.h"
#include "IIpcan.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "ImsServiceConfig.h"
#include "ImsServiceConfigTypeDef.h"
#include "IuMtsService.h"
#include "MtsService.h"
#include "MtsServiceState.h"
#include "MtsDef.h"
#include "PlatformContext.h"
#include "TestImsRadioService.h"
#include "TestPhoneInfoService.h"
#include "core/MockIReference.h"
#include "core/IPageMessage.h"
#include "../include/mts/MockIMtsServiceListener.h"
#include "../../interface/aos/MockIImsAos.h"
#include "../../interface/aos/MockIImsAosInfo.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;

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
    MockIMtsServiceListener objMtsServiceListener;
    MtsService* pMtsService;

    TestImsRadioService objImsRadioService;
    TestPhoneInfoService objPhoneInfoService;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &objImsRadioService);
        /*
         * To make Connector::Open() return valid IConnector even though
         * MtsApp is not created during the test.
         */
        Configuration::GetInstance()->SetAppConfig(
                ImsServiceConfig::GetAppName(ImsAppId::MTS), SLOT_ID);

        pMtsService = new MtsService(SLOT_ID);
        pMtsService->SetIImsAos(&objMockIImsAos);
        pMtsService->SetIImsEmergencyAos(&objMockIImsEmergencyAos);
        pMtsService->SetListener(&objMtsServiceListener);

        EXPECT_CALL(objMockIImsAos, GetAosInfo())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&objMockIImsAosInfo));
        EXPECT_CALL(objMockIImsEmergencyAos, GetAosInfo())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&objMockIImsAosInfo));
        EXPECT_CALL(objMockIImsAosInfo, GetIpcanType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IIpcan::CATEGORY_MOBILE));
        EXPECT_CALL(objMockIImsEmergencyAos, Control(_)).Times(AnyNumber());
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, IMS_NULL);

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

    EXPECT_CALL(objMtsServiceListener, NotifyMtSms(piMessage)).Times(1);

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
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

TEST_F(MtsServiceTest, ImsAosMonitorConnected)
{
    pMtsService->ImsAosMonitor_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    EXPECT_TRUE(pMtsService->GetIMtsServiceState()->IsServiceConnected(ImsAosFeature::SMSIP));
}

TEST_F(MtsServiceTest, GetServiceStateReturnsReadyAfterAosConnected)
{
    pMtsService->SetIImsEmergencyAos(&objMockIImsEmergencyAos);
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_READY);
}

TEST_F(MtsServiceTest, GetServiceStateReturnsNotreadyAfterAosConnecting)
{
    pMtsService->SetIImsEmergencyAos(&objMockIImsEmergencyAos);
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Disconnected(ImsAosReason::NONE);
    pMtsService->ImsAos_Connecting();
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_NOTREADY);
}

TEST_F(MtsServiceTest, GetServiceStateReturnsNotreadyAfterAosDisconnected)
{
    pMtsService->SetIImsEmergencyAos(&objMockIImsEmergencyAos);
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Disconnected(ImsAosReason::NONE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_NOTREADY);
}

TEST_F(MtsServiceTest, GetServiceStateReturnsReadyAfterAosDisconnecting)
{
    pMtsService->SetIImsEmergencyAos(&objMockIImsEmergencyAos);
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Disconnecting(ImsAosReason::NONE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_READY);
}

TEST_F(MtsServiceTest, GetServiceStateReturnsLimitedAfterAosSuspended)
{
    pMtsService->SetIImsEmergencyAos(&objMockIImsEmergencyAos);
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Suspended(ImsAosReason::NONE);
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_LIMITED);
}

TEST_F(MtsServiceTest, GetServiceStateReturnsReadyAfterAosResumed)
{
    pMtsService->SetIImsEmergencyAos(&objMockIImsEmergencyAos);
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_FALSE));

    pMtsService->GetIMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->ImsAos_Suspended(ImsAosReason::NONE);
    pMtsService->ImsAos_Resumed();
    EXPECT_EQ(pMtsService->GetIMtsServiceState()->GetServiceState(), STATE_READY);
}

TEST_F(MtsServiceTest, ForwardScbmNotification)
{
    pMtsService->SetIImsEmergencyAos(&objMockIImsEmergencyAos);

    ON_CALL(objMockIImsEmergencyAos, GetAosInfo()).WillByDefault(Return(&objMockIImsAosInfo));
    ON_CALL(objMockIImsAosInfo, NotifyScbmState(_)).WillByDefault(Return());
    EXPECT_CALL(objMockIImsAosInfo, NotifyScbmState(NOTIFY_SCBM_STARTED)).Times(1);

    pMtsService->SendScbmNotification(NOTIFY_SCBM_STARTED);
}

TEST_F(MtsServiceTest, SendNormalMoSmsWhenTrafficIsNotAllowed)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements
    EXPECT_CALL(objMtsServiceListener, NotifyMoSms(_, _, _, _, _)).Times(0);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    pMtsService->SendMoSms(SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID_1);
}

TEST_F(MtsServiceTest, SendNormalMoSmsWhenTrafficIsAllowed)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements
    EXPECT_CALL(objMtsServiceListener, NotifyMoSms(_, _, _, _, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);

    pMtsService->SendMoSms(SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID_2);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendNormalMoSmsAndGuardTimerIsActived)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements
    EXPECT_CALL(objMtsServiceListener, NotifyMoSms(_, _, _, _, _)).Times(2);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);

    pMtsService->SendMoSms(SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID_1);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);

    pMtsService->SendMoSms(SmsFormatType::SMSFORMAT_3GPP, objRpData, strTargetAddress, SEQ_ID_2);
}

TEST_F(MtsServiceTest, SendE911MoSmsWhenTrafficIsAllowed)
{
    AString strDialedNumber = "911";
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements
    EXPECT_CALL(objMtsServiceListener, NotifyMoSms(_, _, _, _, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPhoneInfoService.GetMockCallInfo(), IsEmergencyNumber(strDialedNumber))
            .WillByDefault(Return(IMS_TRUE));

    pMtsService->SendMoSms(SmsFormatType::SMSFORMAT_3GPP, objRpData, strDialedNumber, SEQ_ID_1);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendE911MoSmsAndGuardTimerIsActived)
{
    AString strDialedNumber = "911";
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements
    EXPECT_CALL(objMtsServiceListener, NotifyMoSms(_, _, _, _, _)).Times(2);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);
    ON_CALL(objMockIImsEmergencyAos, IsImsConnected()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPhoneInfoService.GetMockCallInfo(), IsEmergencyNumber(strDialedNumber))
            .WillByDefault(Return(IMS_TRUE));

    pMtsService->SendMoSms(SmsFormatType::SMSFORMAT_3GPP, objRpData, strDialedNumber, SEQ_ID_1);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MO);

    pMtsService->SendMoSms(SmsFormatType::SMSFORMAT_3GPP, objRpData, strDialedNumber, SEQ_ID_2);
    pMtsService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsWhenTrafficIsAllowed)
{
    ICoreService* piCoreService = pMtsService->GetICoreService(IMS_FALSE);
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);
    EXPECT_CALL(objMtsServiceListener, NotifyMtSms(_)).Times(2);

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
    EXPECT_CALL(objMtsServiceListener, NotifyMtSms(_)).Times(2);

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MT);

    pMtsService->CoreService_PageMessageReceived(piCoreService, piMessage);
    pMtsService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MT);
}

}  // namespace android
