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
#include "INetworkWatcher.h"
#include "IPageMessage.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "ImsServiceConfig.h"
#include "ImsServiceConfigTypeDef.h"
#include "IuMtsApp.h"
#include "JniEnablerConnector.h"
#include "MockICarrierConfig.h"
#include "MockICoreService.h"
#include "MockIImsAos.h"
#include "MockIImsAosInfo.h"
#include "MockIJniEnabler.h"
#include "MockIJniMtsAppThread.h"
#include "MockIMtsContext.h"
#include "MockIMtsMessageController.h"
#include "MockIMtsNetworkTracker.h"
#include "MockIMtsServiceState.h"
#include "MockIReference.h"
#include "MtsDef.h"
#include "MtsService.h"
#include "MtsServiceState.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "TestImsRadioService.h"
#include <gtest/gtest.h>
#include "utility/MtsDynamicLoader.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;
const IMS_SINT32 SEQ_ID_1 = 1;
const IMS_SINT32 SEQ_ID_2 = 2;
const IMS_UINT32 RETRY_COUNT = 0;
const IMS_UINTP FAKE_ADDRESS = 1;

class TestMtsService : public MtsService
{
public:
    inline TestMtsService(IN IMtsContext& objContext, IN MtsServiceType eServiceType) :
            MtsService(objContext, eServiceType)
    {
    }

    inline void ReplaceCoreService(IN ICoreService* pCoreService)
    {
        m_piCoreService = pCoreService;
    }

    inline void ReplaceIImsAos(IN IImsAos* piImsAos) { m_piImsAos = piImsAos; }

    inline void ReplaceServiceState(IN IMtsServiceState* piMtsServiceState)
    {
        m_piMtsServiceState = piMtsServiceState;
    }
};

class MtsServiceTest : public ::testing::Test
{
public:
    JniEnablerConnector* pConnector;
    MockICoreService objMockCoreService;
    MockIImsAos objMockIImsAos;
    MockIImsAosInfo objMockIImsAosInfo;
    MockIJniEnabler objMockJniEnabler;
    MockIJniMtsAppThread objMockJniAppThread;
    MockIMtsContext objMockContext;
    MockIMtsMessageController objMockMessageController;
    MockIMtsNetworkTracker objMockNetworkTracker;
    MockIMtsServiceState* pMockNormalServiceState;
    MockIMtsServiceState* pMockEmergencyServiceState;
    TestMtsService* pNormalService;
    TestMtsService* pEmergencyService;
    MtsDynamicLoader* pMtsDynamicLoader;

    TestConfigService objConfigService;
    TestImsRadioService objImsRadioService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objMockContext, GetMessageController)
                .WillByDefault(ReturnRef(objMockMessageController));
        ON_CALL(objMockContext, GetNetworkTracker).WillByDefault(ReturnRef(objMockNetworkTracker));
        ON_CALL(objMockContext, GetJniAppThread).WillByDefault(Return(&objMockJniAppThread));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &objImsRadioService);

        ON_CALL(objConfigService.GetMockCarrierConfig(),
                GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL, _))
                .WillByDefault(Return(IMS_TRUE));
        ON_CALL(objConfigService.GetMockCarrierConfig(),
                GetBoolean(CarrierConfig::ImsSms::KEY_SMS_ALLOW_IMSI_BASED_SIP_URI_BOOL, _))
                .WillByDefault(Return(IMS_FALSE));

        pMockNormalServiceState = new MockIMtsServiceState();
        pMockEmergencyServiceState = new MockIMtsServiceState();
        pMtsDynamicLoader = new MtsDynamicLoader(objMockContext);

        pNormalService = new TestMtsService(objMockContext, MtsServiceType::NORMAL);
        pNormalService->Init();
        pNormalService->ReplaceCoreService(&objMockCoreService);
        pNormalService->ReplaceIImsAos(&objMockIImsAos);
        pNormalService->ReplaceServiceState(pMockNormalServiceState);

        pEmergencyService = new TestMtsService(objMockContext, MtsServiceType::EMERGENCY);
        pEmergencyService->Init();
        pEmergencyService->ReplaceCoreService(&objMockCoreService);
        pEmergencyService->ReplaceIImsAos(&objMockIImsAos);
        pEmergencyService->ReplaceServiceState(pMockEmergencyServiceState);

        ON_CALL(objMockIImsAos, GetAosInfo()).WillByDefault(Return(&objMockIImsAosInfo));

        pConnector = &JniEnablerConnector::GetInstance();
        pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTS, &objMockJniEnabler);
        ON_CALL(objMockJniEnabler, GetJniThread).WillByDefault(Return(&objMockJniAppThread));
        ON_CALL(objMockContext, GetDynamicLoader).WillByDefault(ReturnRef(*pMtsDynamicLoader));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        delete pNormalService;
        delete pEmergencyService;
        delete pMtsDynamicLoader;
    }
};

TEST_F(MtsServiceTest, GetICoreServiceReturnsNotNull)
{
    EXPECT_EQ(pNormalService->GetICoreService(), &objMockCoreService);
    EXPECT_EQ(pEmergencyService->GetICoreService(), &objMockCoreService);
}

TEST_F(MtsServiceTest, CoreServicePageMessageReceivedOnNormalService)
{
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockNetworkTracker, GetNetworkType)
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_LTE));
    ON_CALL(objMockIImsAosInfo, GetIpcanType).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));

    EXPECT_CALL(
            objImsRadioService.GetMockImsRadio(), StartImsTraffic(_, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(objMockMessageController, ProcessMtSms(piMessage, MtsServiceType::NORMAL)).Times(1);

    pNormalService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
}

TEST_F(MtsServiceTest, CoreServicePageMessageReceivedOnEmergencyService)
{
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockNetworkTracker, GetNetworkType)
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_LTE));
    ON_CALL(objMockIImsAosInfo, GetIpcanType).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));

    EXPECT_CALL(
            objImsRadioService.GetMockImsRadio(), StartImsTraffic(_, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(objMockMessageController, ProcessMtSms(piMessage, MtsServiceType::EMERGENCY))
            .Times(1);

    pEmergencyService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
}

TEST_F(MtsServiceTest, CoreServicePageMessageReceivedWithInvalidParameter)
{
    EXPECT_CALL(objMockMessageController, ProcessMtSms(_, MtsServiceType::NORMAL)).Times(0);

    pNormalService->CoreService_PageMessageReceived(&objMockCoreService, IMS_NULL);
}

TEST_F(MtsServiceTest, CoreServiceReferenceReceivedDoesNothing)
{
    IReference* piReference = reinterpret_cast<IReference*>(FAKE_ADDRESS);
    pNormalService->CoreService_ReferenceReceived(&objMockCoreService, piReference);
}

TEST_F(MtsServiceTest, CoreServiceServiceClosedDoesNothing)
{
    IReasonInfo* piReasonInfo = reinterpret_cast<IReasonInfo*>(FAKE_ADDRESS);
    pNormalService->CoreService_ServiceClosed(&objMockCoreService, piReasonInfo);
}

TEST_F(MtsServiceTest, CoreServiceSessionInvitationReceivedDoesNothing)
{
    ISession* piSession = reinterpret_cast<ISession*>(FAKE_ADDRESS);
    pNormalService->CoreService_SessionInvitationReceived(&objMockCoreService, piSession);
}

TEST_F(MtsServiceTest, CoreServiceUnsolicitedNotifyReceivedDoesNothing)
{
    IMessage* piNotify = reinterpret_cast<IMessage*>(FAKE_ADDRESS);
    pNormalService->CoreService_UnsolicitedNotifyReceived(&objMockCoreService, piNotify);
}

TEST_F(MtsServiceTest, CoreServiceCapabilityQueryReceivedDoesNothing)
{
    ICapabilities* piCapabilities = reinterpret_cast<ICapabilities*>(FAKE_ADDRESS);
    pNormalService->CoreService_CapabilityQueryReceived(&objMockCoreService, piCapabilities);
}

TEST_F(MtsServiceTest, GetStateReturnsReadyAfterAosConnected)
{
    EXPECT_CALL(*pMockNormalServiceState, OnImsConnected()).Times(1);

    pNormalService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtsServiceTest, GetStateReturnsNotreadyAfterAosConnecting)
{
    EXPECT_CALL(*pMockNormalServiceState, OnImsConnected()).Times(1);
    EXPECT_CALL(*pMockNormalServiceState, OnImsDisconnected(ImsAosReason::NONE)).Times(1);
    EXPECT_CALL(objMockMessageController, ClearAllMessages()).Times(1);
    IMS_SINT32 nDataFailureReason = 0;

    pNormalService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pNormalService->ImsAos_Disconnected(ImsAosReason::NONE, nDataFailureReason);
    pNormalService->ImsAos_Connecting();
}

TEST_F(MtsServiceTest, GetStateReturnsNotreadyAfterAosDisconnected)
{
    EXPECT_CALL(*pMockNormalServiceState, OnImsConnected()).Times(1);
    EXPECT_CALL(*pMockNormalServiceState, OnImsDisconnected(ImsAosReason::NONE)).Times(1);
    EXPECT_CALL(objMockMessageController, ClearAllMessages()).Times(1);
    IMS_SINT32 nDataFailureReason = 0;

    pNormalService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pNormalService->ImsAos_Disconnected(ImsAosReason::NONE, nDataFailureReason);
}

TEST_F(MtsServiceTest, GetStateReturnsReadyAfterAosDisconnecting)
{
    EXPECT_CALL(*pMockNormalServiceState, OnImsConnected()).Times(1);
    EXPECT_CALL(*pMockNormalServiceState, OnImsDisconnecting(ImsAosReason::NONE)).Times(1);

    pNormalService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pNormalService->ImsAos_Disconnecting(ImsAosReason::NONE);
}

TEST_F(MtsServiceTest, GetStateReturnsLimitedAfterAosSuspended)
{
    EXPECT_CALL(*pMockNormalServiceState, OnImsConnected()).Times(1);
    EXPECT_CALL(*pMockNormalServiceState, OnImsSuspended(ImsAosReason::NONE)).Times(1);
    EXPECT_CALL(objMockMessageController, ClearAllMessages()).Times(1);

    pNormalService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pNormalService->ImsAos_Suspended(ImsAosReason::NONE);
}

TEST_F(MtsServiceTest, GetStateReturnsReadyAfterAosResumed)
{
    EXPECT_CALL(*pMockNormalServiceState, OnImsConnected()).Times(1);
    EXPECT_CALL(*pMockNormalServiceState, OnImsSuspended(ImsAosReason::NONE)).Times(1);
    EXPECT_CALL(*pMockNormalServiceState, OnImsResumed()).Times(1);
    EXPECT_CALL(objMockMessageController, ClearAllMessages()).Times(1);

    pNormalService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pNormalService->ImsAos_Suspended(ImsAosReason::NONE);
    pNormalService->ImsAos_Resumed();
}

TEST_F(MtsServiceTest, RequestRegistrationRecoveryToAos)
{
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::PCSCF_NEXT)).Times(1);
    pNormalService->RequestRegistrationRecovery(MtsRegRecoveryPolicy::PCSCF_NEXT);
}

TEST_F(MtsServiceTest, RequestRegistrationRecoveryWhenAosIsDetached)
{
    pNormalService->ReplaceIImsAos(IMS_NULL);
    EXPECT_CALL(objMockIImsAos, Control(_)).Times(0);
    pNormalService->RequestRegistrationRecovery(MtsRegRecoveryPolicy::PCSCF_NEXT);
}

TEST_F(MtsServiceTest, RequestRegisterWithNextPcscfToAos)
{
    IMS_UINT32 nRetryAfterValue = 32;
    EXPECT_CALL(objMockIImsAos, RegisterWithNextPcscf(nRetryAfterValue)).Times(1);
    pNormalService->RequestRegisterWithNextPcscf(nRetryAfterValue);
}

TEST_F(MtsServiceTest, RequestRegisterWithNextPcscfWhenAosIsDetached)
{
    IMS_UINT32 nRetryAfterValue = 32;
    pNormalService->ReplaceIImsAos(IMS_NULL);
    EXPECT_CALL(objMockIImsAos, RegisterWithNextPcscf(nRetryAfterValue)).Times(0);
    pNormalService->RequestRegisterWithNextPcscf(nRetryAfterValue);
}

TEST_F(MtsServiceTest, SendNormalMoSmsWhenTrafficIsNotAllowed)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    EXPECT_CALL(objMockMessageController, ProcessMoSms(_, _, _, _, _, _, _)).Times(0);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockJniAppThread, ReportMoStatus(MO_ERROR_RETRY, eSmsFormat, SEQ_ID_1, SLOT_ID))
            .Times(1);

    pNormalService->SendMoSms(
            eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_1, bEmergencyNumber, RETRY_COUNT);
}

TEST_F(MtsServiceTest, SendNormalMoSmsWhenTrafficIsAllowed)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    EXPECT_CALL(objMockMessageController, ProcessMoSms(_, _, _, _, _, _, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);

    pNormalService->SendMoSms(
            eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_2, bEmergencyNumber, RETRY_COUNT);
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendNormalMoSmsAndTrafficConnectionFailed)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    EXPECT_CALL(objMockMessageController, ProcessMoSms(_, _, _, _, _, _, _)).Times(0);
    EXPECT_CALL(objMockMessageController, ClearAllMessages()).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);

    pNormalService->SendMoSms(
            eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_2, bEmergencyNumber, RETRY_COUNT);
    pNormalService->Traffic_OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO,
            IImsRadio::REASON_ACCESS_DENIED, 400, 2000);
}

TEST_F(MtsServiceTest, SendNormalMoSmsAndInvalidTrafficTypeIsAllowed)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    EXPECT_CALL(objMockMessageController, ProcessMoSms(_, _, _, _, _, _, _)).Times(0);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);

    pNormalService->SendMoSms(
            eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_2, bEmergencyNumber, RETRY_COUNT);
    // Invalid traffic type
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendNormalMoSmsAndGuardTimerIsActived)
{
    AString strTargetAddress = "sip:+12345678901@ims.google.com";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergencyNumber = IMS_FALSE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    EXPECT_CALL(objMockMessageController, ProcessMoSms(_, _, _, _, _, _, _)).Times(2);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);

    pNormalService->SendMoSms(
            eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_1, bEmergencyNumber, RETRY_COUNT);
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);

    pNormalService->SendMoSms(
            eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_2, bEmergencyNumber, RETRY_COUNT);
}

TEST_F(MtsServiceTest, InvalidGuardTimerExpired)
{
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), StopImsTraffic(_)).Times(0);
    // Invalid traffic type
    pNormalService->Traffic_GuardTimerExpired(
            IImsRadio::TRAFFIC_TYPE_VOICE, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendE911MoSmsWhenTrafficIsAllowed)
{
    AString strTargetAddress = "911";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    EXPECT_CALL(objMockMessageController, ProcessMoSms(_, _, _, _, _, _, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::REGISTER_START)).Times(1);
    ON_CALL(objMockIImsAosInfo, GetRegistrationMode())
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));

    pEmergencyService->SendMoSms(
            eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_1, bEmergencyNumber, RETRY_COUNT);
    pEmergencyService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pEmergencyService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendE911MoSmsWhenTrafficIsAllowedButDoNotUseEPDN)
{
    AString strTargetAddress = "911";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    EXPECT_CALL(objMockMessageController, ProcessMoSms(_, _, _, _, _, _, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::REGISTER_START)).Times(0);

    pNormalService->SendMoSms(
            eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_1, bEmergencyNumber, RETRY_COUNT);
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);
}

TEST_F(MtsServiceTest, SendE911MoSmsAndGuardTimerIsActived)
{
    AString strTargetAddress = "911";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    EXPECT_CALL(objMockMessageController, ProcessMoSms(_, _, _, _, _, _, _)).Times(2);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), IsImsTrafficAllowed(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, _, IImsRadio::DIRECTION_MO, _))
            .Times(1);
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::REGISTER_START)).Times(2);
    ON_CALL(objMockIImsAosInfo, GetRegistrationMode())
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));

    pEmergencyService->SendMoSms(
            eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_1, bEmergencyNumber, RETRY_COUNT);
    pEmergencyService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
    pEmergencyService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MO);

    pEmergencyService->SendMoSms(
            eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_2, bEmergencyNumber, RETRY_COUNT);
    pEmergencyService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtsServiceTest, SendE911MoSmsWhenAdminRegistrationMode)
{
    AString strTargetAddress = "911";
    SmsFormatType eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    IMS_BOOL bEmergencyNumber = IMS_TRUE;
    ByteArray objRpData((IMS_BYTE)0x00);  // message type indicator(RP-MO-DATA)
    objRpData.Append((IMS_BYTE)0x02);     // message reference
    objRpData.Append((IMS_BYTE)0x0F);     // other required information elements

    EXPECT_CALL(objMockMessageController, ProcessMoSms(_, _, _, _, _, _, _)).Times(0);
    EXPECT_CALL(objMockIImsAos, Control(ImsAosControl::REGISTER_START)).Times(1);
    EXPECT_CALL(
            objMockJniAppThread, ReportMoStatus(MO_ERROR_GENERIC, eSmsFormat, SEQ_ID_1, SLOT_ID))
            .Times(1);
    ON_CALL(objMockIImsAosInfo, GetRegistrationMode())
            .WillByDefault(Return(IImsAosInfo::REG_MODE_ADMIN));

    pEmergencyService->SendMoSms(
            eSmsFormat, &objRpData, strTargetAddress, SEQ_ID_1, bEmergencyNumber, RETRY_COUNT);
    pEmergencyService->ImsAos_Connected(ImsAosFeature::SMSIP, IIpcan::CATEGORY_MOBILE);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsWhenTrafficIsAllowed)
{
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    EXPECT_CALL(objMockMessageController, ProcessMtSms(_, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);

    pNormalService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedE911MtSmsWhenTrafficIsAllowed)
{
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    EXPECT_CALL(objMockMessageController, ProcessMtSms(_, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);

    pEmergencyService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
    pEmergencyService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsWhenGuardTimerIsActived)
{
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    EXPECT_CALL(objMockMessageController, ProcessMtSms(_, _)).Times(2);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);

    pNormalService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);

    pNormalService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsThroughIWLAN)
{
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockIImsAosInfo, GetIpcanType()).WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    EXPECT_CALL(objMockMessageController, ProcessMtSms(piMessage, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);

    pNormalService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsThroughHSPA)
{
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockIImsAosInfo, GetIpcanType()).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_CALL(objMockMessageController, ProcessMtSms(piMessage, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(objMockNetworkTracker, GetNetworkType())
            .WillRepeatedly(Return(INetworkWatcher::RADIOTECH_TYPE_HSPA));

    pNormalService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsThroughLTE)
{
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockIImsAosInfo, GetIpcanType()).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_CALL(objMockMessageController, ProcessMtSms(piMessage, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(objMockNetworkTracker, GetNetworkType())
            .WillRepeatedly(Return(INetworkWatcher::RADIOTECH_TYPE_LTE));

    pNormalService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsThroughNR)
{
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockIImsAosInfo, GetIpcanType()).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_CALL(objMockMessageController, ProcessMtSms(piMessage, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(objMockNetworkTracker, GetNetworkType())
            .WillRepeatedly(Return(INetworkWatcher::RADIOTECH_TYPE_NR));

    pNormalService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, ReceivedNormalMtSmsThroughUnknownNetworkType)
{
    IPageMessage* piMessage = reinterpret_cast<IPageMessage*>(FAKE_ADDRESS);

    ON_CALL(objMockIImsAosInfo, GetIpcanType()).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_CALL(objMockMessageController, ProcessMtSms(piMessage, _)).Times(1);
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(),
            StartImsTraffic(IImsRadio::TRAFFIC_TYPE_SMS, _, IImsRadio::DIRECTION_MT, _))
            .Times(1);
    EXPECT_CALL(objMockNetworkTracker, GetNetworkType())
            .WillRepeatedly(Return(INetworkWatcher::RADIOTECH_TYPE_UNKNOWN));

    pNormalService->CoreService_PageMessageReceived(&objMockCoreService, piMessage);
    pNormalService->Traffic_OnConnectionSetupPrepared(
            IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);
}

TEST_F(MtsServiceTest, TrafficGuardTimerExpiredAndStopImsTraffic)
{
    EXPECT_CALL(objImsRadioService.GetMockImsRadio(), StopImsTraffic(_)).Times(1);

    pNormalService->Traffic_GuardTimerExpired(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);
}

}  // namespace android
