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
#include "Engine.h"
#include "IConfiguration.h"
#include "IIpcan.h"
#include "INetworkWatcher.h"
#include "ImsAosParameter.h"
#include "ImsEventDef.h"
#include "IuMtsApp.h"
#include "MockICarrierConfig.h"
#include "MockIMessage.h"
#include "MockIMtsContext.h"
#include "MockIMtsMessage.h"
#include "MockIMtsNetworkTracker.h"
#include "MockIMtsService.h"
#include "MockIPageMessage.h"
#include "MtsDef.h"
#include "PlatformContext.h"
#include "SipHeaderName.h"
#include "SipStatusCode.h"
#include "TestConfigService.h"
#include "message/MtsErrorHandler.h"
#include "utility/MtsDynamicLoader.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;
const IMS_UINT32 RETRY_COUNT = 0;
const IMS_UINT32 RETRY_COUNT_FOR_2ND_ATTEMPT = 1;

class MtsErrorHandlerTest : public ::testing::Test
{
public:
    MockIMessage objMockMessage;
    MockIMtsContext objContext;
    MockIMtsMessage objMockMtsMessage;
    MockIMtsNetworkTracker objMockMtsNetworkTracker;
    MockIMtsService objMockMtsService;
    MockIPageMessage objMockPageMessage;
    MtsDynamicLoader* pMtsDynamicLoader;
    MtsErrorHandler* pMtsErrorHandler;
    TestConfigService objConfigService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objMockMtsService));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetNetworkTracker).WillByDefault(ReturnRef(objMockMtsNetworkTracker));
        ON_CALL(objMockMtsMessage, GetPageMessage).WillByDefault(Return(&objMockPageMessage));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);

        pMtsErrorHandler = new MtsErrorHandler(objContext);

        pMtsDynamicLoader = new MtsDynamicLoader(objContext);
        ON_CALL(objContext, GetDynamicLoader).WillByDefault(ReturnRef(*pMtsDynamicLoader));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);

        delete pMtsDynamicLoader;
        delete pMtsErrorHandler;
    }
};

TEST_F(MtsErrorHandlerTest, Constructor)
{
    ASSERT_NE(pMtsErrorHandler, nullptr);
}

TEST_F(MtsErrorHandlerTest, Handle403Error)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_403));

    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_400);
    objErrorCodes.Push(SipStatusCode::SC_403);
    objErrorCodes.Push(SipStatusCode::SC_404);
    objErrorCodes.Push(SipStatusCode::SC_406);
    objErrorCodes.Push(SipStatusCode::SC_408);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_403_RESPONSE_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::REGISTER_REFRESH));

    EXPECT_CALL(objMockMtsService, RequestRegistrationRecovery(ImsAosControl::REGISTER_REFRESH))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle404Error)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_404));

    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_400);
    objErrorCodes.Push(SipStatusCode::SC_403);
    objErrorCodes.Push(SipStatusCode::SC_404);
    objErrorCodes.Push(SipStatusCode::SC_406);
    objErrorCodes.Push(SipStatusCode::SC_408);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_404_RESPONSE_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::PCSCF_NEXT));

    EXPECT_CALL(objMockMtsService, RequestRegistrationRecovery(ImsAosControl::PCSCF_NEXT)).Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle406Error)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_406));

    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_400);
    objErrorCodes.Push(SipStatusCode::SC_403);
    objErrorCodes.Push(SipStatusCode::SC_404);
    objErrorCodes.Push(SipStatusCode::SC_406);
    objErrorCodes.Push(SipStatusCode::SC_408);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_406_RESPONSE_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::IPSEC_DISABLED));

    EXPECT_CALL(objMockMtsService, RequestRegistrationRecovery(ImsAosControl::IPSEC_DISABLED))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle408Error)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_408));

    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_400);
    objErrorCodes.Push(SipStatusCode::SC_403);
    objErrorCodes.Push(SipStatusCode::SC_404);
    objErrorCodes.Push(SipStatusCode::SC_406);
    objErrorCodes.Push(SipStatusCode::SC_408);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_408_RESPONSE_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle500Error)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_500));

    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_500);
    objErrorCodes.Push(SipStatusCode::SC_503);
    objErrorCodes.Push(SipStatusCode::SC_504);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_500_RESPONSE_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle503Error)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));

    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_500);
    objErrorCodes.Push(SipStatusCode::SC_503);
    objErrorCodes.Push(SipStatusCode::SC_504);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_503_RESPONSE_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle503ErrorWithReasonHeader)
{
    AString strReasonPhrase = "OUTAGE";
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(objMockMessage, GetReasonPhrase()).WillByDefault(ReturnRef(strReasonPhrase));

    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_500);
    objErrorCodes.Push(SipStatusCode::SC_503);
    objErrorCodes.Push(SipStatusCode::SC_504);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_503_RESPONSE_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::REGISTER_REINITIATE));

    EXPECT_CALL(objMockMtsService, RequestRegistrationRecovery(ImsAosControl::REGISTER_REINITIATE))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle503ErrorWithoutReasonHeader)
{
    AString strReasonPhrase = AString::ConstEmpty();
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(objMockMessage, GetReasonPhrase()).WillByDefault(ReturnRef(strReasonPhrase));

    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_500);
    objErrorCodes.Push(SipStatusCode::SC_503);
    objErrorCodes.Push(SipStatusCode::SC_504);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_503_RESPONSE_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::REGISTER_REINITIATE));

    EXPECT_CALL(objMockMtsService, RequestRegistrationRecovery(_)).Times(0);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle503ErrorWithNextPcscfRequired)
{
    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_500);
    objErrorCodes.Push(SipStatusCode::SC_504);
    const AString strRetryAfter(SipHeaderName::RETRY_AFTER);
    ImsList<AString> objRetryAfterHeaders;
    objRetryAfterHeaders.Append("130");

    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetHeaders(strRetryAfter)).WillByDefault(Return(objRetryAfterHeaders));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_503_RESPONSE_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE));
    Engine::GetConfiguration()->RefreshConfigs(SLOT_ID);

    EXPECT_CALL(objMockMtsService,
            RequestRegisterWithNextPcscf(objRetryAfterHeaders.GetAt(0).ToInt32()))
            .Times(1);
    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle504Error)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_504));

    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_500);
    objErrorCodes.Push(SipStatusCode::SC_503);
    objErrorCodes.Push(SipStatusCode::SC_504);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_504_RESPONSE_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, HandleTimerFExpiredAndReportGenericError)
{
    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_406);
    objErrorCodes.Push(SipStatusCode::SC_503);
    objErrorCodes.Push(SipStatusCode::SC_INVALID);

    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(IMS_NULL));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_EXPIRY_TIMER_F_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, HandleTimerFExpiredAndReportErrorRetry)
{
    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_406);
    objErrorCodes.Push(SipStatusCode::SC_503);

    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(IMS_NULL));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_POLICY_FOR_EXPIRY_TIMER_F_INT, _))
            .WillByDefault(Return(MO_ERROR_RETRY));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_EXPIRY_TIMER_F_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_RETRY);
}

TEST_F(MtsErrorHandlerTest, HandleTimerFExpiredAndReportFallback)
{
    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_406);
    objErrorCodes.Push(SipStatusCode::SC_503);

    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(IMS_NULL));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_POLICY_FOR_EXPIRY_TIMER_F_INT, _))
            .WillByDefault(Return(MO_ERROR_FALLBACK));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_REG_POLICY_FOR_EXPIRY_TIMER_F_INT,
                    MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_FALLBACK);
}

TEST_F(MtsErrorHandlerTest, Handle380FallbackErrorCode)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_380));

    ImsVector<IMS_SINT32> objGenericErrorCodes;
    objGenericErrorCodes.Push(SipStatusCode::SC_503);
    objGenericErrorCodes.Push(SipStatusCode::SC_504);

    ImsVector<IMS_SINT32> objFallbackErrorCodes;
    objFallbackErrorCodes.Push(SipStatusCode::SC_380);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objGenericErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_FALLBACK_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objFallbackErrorCodes));

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_FALLBACK);
}

TEST_F(MtsErrorHandlerTest, HandleSmma400GenericErrorCode)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_SMMA));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_400));

    ImsVector<IMS_SINT32> objGenericErrorCodes;

    ImsVector<IMS_SINT32> objFallbackErrorCodes;
    objFallbackErrorCodes.Push(SipStatusCode::SC_380);

    ImsVector<IMS_SINT32> objSmmaGenericErrorCodes;
    objSmmaGenericErrorCodes.Push(SipStatusCode::SC_400);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objGenericErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_FALLBACK_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objFallbackErrorCodes));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_SMMA_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objSmmaGenericErrorCodes));
    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, ResetRetryAfterConditionIfRetryAfterValueDoesNotExist)
{
    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_500);
    objErrorCodes.Push(SipStatusCode::SC_504);
    const AString strRetryAfter(SipHeaderName::RETRY_AFTER);
    ImsList<AString> objRetryAfterHeaders;
    objRetryAfterHeaders.Append("10");
    ImsList<AString> objNoHeaders;

    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_COUNT_INT, _))
            .WillByDefault(Return(5));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_RETRY_AFTER_MAX_TIME_SEC_INT, _))
            .WillByDefault(Return(45));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(CarrierConfig::ImsSms::
                               KEY_SMS_REPORT_GENERIC_ERROR_WHEN_RETRY_AFTER_NOT_POSSIBLE_BOOL,
                    _))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMockMessage, GetHeaders(strRetryAfter))
            .WillOnce(Return(objRetryAfterHeaders))
            .WillRepeatedly(Return(objNoHeaders));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_603));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::ImsSms::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY, _))
            .WillByDefault(Return(objErrorCodes));

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_BY_RETRY_AFTER);
    EXPECT_EQ(pMtsErrorHandler->GetRetryAfterValue(), 10);

    nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
    EXPECT_EQ(pMtsErrorHandler->GetRetryAfterValue(), 0);
}

TEST_F(MtsErrorHandlerTest, EvaluateNetworkStatusBefore3rdAttemptWhenLteEpsOnly)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT_FOR_2ND_ATTEMPT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(
                    CarrierConfig::ImsSms::KEY_SMS_EVALUATE_RADIO_STATUS_FOR_3RD_ATTEMPT_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockMtsNetworkTracker, GetNetworkType)
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_LTE));
    ON_CALL(objMockMtsNetworkTracker, GetLteAttachState)
            .WillByDefault(Return(IMS_LTE_INFO_EPS_ONLY_ATTACHED));

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, EvaluateNetworkStatusBefore3rdAttemptWhenLteCombined)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT_FOR_2ND_ATTEMPT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(
                    CarrierConfig::ImsSms::KEY_SMS_EVALUATE_RADIO_STATUS_FOR_3RD_ATTEMPT_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockMtsNetworkTracker, GetNetworkType)
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_LTE_CA));
    ON_CALL(objMockMtsNetworkTracker, GetLteAttachState)
            .WillByDefault(Return(IMS_LTE_INFO_COMBINED_ATTACHED));

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_FALLBACK);
}

TEST_F(MtsErrorHandlerTest, EvaluateNetworkStatusBefore3rdAttemptWhenNr)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT_FOR_2ND_ATTEMPT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(
                    CarrierConfig::ImsSms::KEY_SMS_EVALUATE_RADIO_STATUS_FOR_3RD_ATTEMPT_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockMtsNetworkTracker, GetNetworkType)
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_NR));
    ON_CALL(objMockMtsNetworkTracker, IsInRoamingState).WillByDefault(Return(IMS_FALSE));

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, EvaluateNetworkStatusBefore3rdAttemptWhenNrRoaming)
{
    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT_FOR_2ND_ATTEMPT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetBoolean(
                    CarrierConfig::ImsSms::KEY_SMS_EVALUATE_RADIO_STATUS_FOR_3RD_ATTEMPT_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockMtsNetworkTracker, GetNetworkType)
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_NR));
    ON_CALL(objMockMtsNetworkTracker, IsInRoamingState).WillByDefault(Return(IMS_TRUE));

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_FALLBACK);
}

TEST_F(MtsErrorHandlerTest, EvaluateNetworkStatusWhen500ErrorReceivedInCellularNetwork)
{
    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_500);

    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(
                    CarrierConfig::ImsSms::KEY_SMS_EVALUATE_RADIO_STATUS_FOR_ERROR_CODES_INT_ARRAY,
                    _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_500));
    ON_CALL(objMockMtsService, IsWlan()).WillByDefault(Return(IIpcan::CATEGORY_MOBILE));
    ON_CALL(objMockMtsNetworkTracker, GetCellularServiceState)
            .WillByDefault(Return(INetworkWatcher::STATE_IN_SERVICE));

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_FALLBACK);
}

TEST_F(MtsErrorHandlerTest, EvaluateNetworkStatusWhen503ErrorReceivedOverWlanInHome)
{
    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_503);

    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(
                    CarrierConfig::ImsSms::KEY_SMS_EVALUATE_RADIO_STATUS_FOR_ERROR_CODES_INT_ARRAY,
                    _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(objMockMtsService, IsWlan()).WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    ON_CALL(objMockMtsNetworkTracker, IsInRoamingState).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsNetworkTracker, GetCellularServiceState)
            .WillByDefault(Return(INetworkWatcher::STATE_IN_SERVICE));

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_FALLBACK);
}

TEST_F(MtsErrorHandlerTest, EvaluateNetworkStatusWhen503ErrorReceivedOverWlanInRoaming)
{
    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_503);

    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(
                    CarrierConfig::ImsSms::KEY_SMS_EVALUATE_RADIO_STATUS_FOR_ERROR_CODES_INT_ARRAY,
                    _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(objMockMtsService, IsWlan()).WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    ON_CALL(objMockMtsNetworkTracker, IsInRoamingState).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objMockMtsNetworkTracker, GetCellularServiceState)
            .WillByDefault(Return(INetworkWatcher::STATE_IN_SERVICE));

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, EvaluateNetworkStatusWhen503ErrorReceivedOverWlanDuringRadioPowerOff)
{
    ImsVector<IMS_SINT32> objErrorCodes;
    objErrorCodes.Push(SipStatusCode::SC_503);

    ON_CALL(objMockMtsMessage, GetRetryCount).WillByDefault(Return(RETRY_COUNT));
    ON_CALL(objMockMtsMessage, GetMti).WillByDefault(Return(SMS_3GPP_MTI_RP_DATA_FROM_MS));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(
                    CarrierConfig::ImsSms::KEY_SMS_EVALUATE_RADIO_STATUS_FOR_ERROR_CODES_INT_ARRAY,
                    _))
            .WillByDefault(Return(objErrorCodes));
    ON_CALL(objMockPageMessage, GetPreviousResponse(IMessage::PAGEMESSAGE_SEND))
            .WillByDefault(Return(&objMockMessage));
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(objMockMtsService, IsWlan()).WillByDefault(Return(IIpcan::CATEGORY_WLAN));
    ON_CALL(objMockMtsNetworkTracker, IsInRoamingState).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objMockMtsNetworkTracker, GetCellularServiceState)
            .WillByDefault(Return(INetworkWatcher::STATE_POWER_OFF));

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(objMockMtsService, &objMockMtsMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

}  // namespace android
