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
#include "MockIMtsService.h"
#include "core/IPageMessage.h"
#include "core/MockICoreService.h"
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
    MockICoreService objMockCoreService;

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

TEST_F(MtsMessageControllerTest, GetICoreServiceReturnsNotNull)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    EXPECT_CALL(*pMtsService, GetICoreService(bEmergency)).WillOnce(Return(&objMockCoreService));
    EXPECT_NE(pMtsMessageController->GetICoreService(bEmergency), nullptr);

    bEmergency = IMS_TRUE;
    EXPECT_CALL(*pMtsService, GetICoreService(bEmergency)).WillOnce(Return(&objMockCoreService));
    EXPECT_NE(pMtsMessageController->GetICoreService(bEmergency), nullptr);
}

TEST_F(MtsMessageControllerTest, GetMtsUtilsReturnsNotNull)
{
    EXPECT_NE(pMtsMessageController->GetMtsUtils(), nullptr);
}

TEST_F(MtsMessageControllerTest, ReportMoStatus)
{
    EXPECT_CALL(*pMtsService, ReportMoStatus(MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, _, SEQ_ID))
            .Times(1);
    pMtsMessageController->ReportMoStatus(
            MO_SUCCESS, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID);
}

TEST_F(MtsMessageControllerTest, ReportMtSms)
{
    AString strData = "Received SMS PDU";
    ByteArray objSms = strData.ToBase64();
    EXPECT_CALL(*pMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->ReportMtSms(
            SmsFormatType::SMSFORMAT_3GPP, SEQ_ID, (const IMS_BYTE*)objSms.GetData());
}

TEST_F(MtsMessageControllerTest, ReportTransmissionResult)
{
    EXPECT_CALL(*pMtsService,
            ReportMoStatus(MO_IMS_TEMP_FAILURE, SmsFormatType::SMSFORMAT_3GPP, _, SEQ_ID))
            .Times(1);
    pMtsMessageController->ReportTransmissionResult(
            SipStatusCode::SC_404, SmsFormatType::SMSFORMAT_3GPP, SEQ_ID);
}

TEST_F(MtsMessageControllerTest, ReportTransmissionFailureWithRetryTime)
{
    EXPECT_CALL(*pMtsService,
            ReportMoStatus(
                    MO_IMS_TEMP_FAILURE, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->ReportTransmissionFailureWithRetryTime(
            SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID);
}

TEST_F(MtsMessageControllerTest, NotifyMoSmsWithEmergencyFlag)
{
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);

    ByteArray objSms((IMS_BYTE)0x00); // RP-DATA
    IMS_BOOL bEmergency = IMS_FALSE;
    pMtsDynamicLoader->GetMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsDynamicLoader->GetMtsServiceState()->OnImsConnected();
    pMtsDynamicLoader->GetMtsServiceState()->UpdateServiceState();

    EXPECT_CALL(*pMtsService, GetICoreService(bEmergency)).WillOnce(Return(&objMockCoreService));
    EXPECT_CALL(objMockCoreService, GetAuthorizedUserId()).WillOnce(ReturnRef(objSipAddress));
    /*
     * TODO: MO_IMS_PERM_FAILURE is the expected result because IPageMessage is null
     *       MockIPageMessage is needed to received MO_SUCCESS result
     */
    EXPECT_CALL(*pMtsService,
            ReportMoStatus(
                    MO_IMS_PERM_FAILURE, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objSms, strTargetAddress, SEQ_ID, bEmergency);

    bEmergency = IMS_TRUE;
    EXPECT_CALL(*pMtsService, GetICoreService(bEmergency)).WillOnce(Return(&objMockCoreService));
    EXPECT_CALL(objMockCoreService, GetAuthorizedUserId()).WillOnce(ReturnRef(objSipAddress));
    /*
     * TODO: MO_IMS_PERM_FAILURE is the expected result because IPageMessage is null
     *       MockIPageMessage is needed to received MO_SUCCESS result
     */
    EXPECT_CALL(*pMtsService,
            ReportMoStatus(
                    MO_IMS_PERM_FAILURE, SmsFormatType::SMSFORMAT_3GPP, RETRY_AFTER, SEQ_ID))
            .Times(1);
    pMtsMessageController->NotifyMoSms(
            SmsFormatType::SMSFORMAT_3GPP, objSms, strTargetAddress, SEQ_ID, bEmergency);
}

TEST_F(MtsMessageControllerTest, NotifyMtSms)
{
    IMS_BOOL bEmergency = IMS_FALSE;
    AString strTargetAddress = "tel:+12345678901";
    SipAddress objSipAddress;
    objSipAddress.Create(strTargetAddress);
    pMtsDynamicLoader->GetMtsServiceState()->SetSmsOverIpState(IMS_TRUE);
    pMtsDynamicLoader->GetMtsServiceState()->OnImsConnected();
    pMtsDynamicLoader->GetMtsServiceState()->UpdateServiceState();

    EXPECT_CALL(*pMtsService, GetICoreService(bEmergency)).WillOnce(Return(&objMockCoreService));
    EXPECT_CALL(objMockCoreService, GetAuthorizedUserId()).WillOnce(ReturnRef(objSipAddress));
    // TODO: MockIPageMessage is needed to received MockIMtsService::ReportMtSms()
    // EXPECT_CALL(*pMtsService, ReportMtSms(SmsFormatType::SMSFORMAT_3GPP, _)).Times(1);
    pMtsMessageController->NotifyMtSms(IMS_NULL);
}

}  // namespace android
