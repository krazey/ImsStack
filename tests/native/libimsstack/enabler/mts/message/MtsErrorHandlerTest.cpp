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
#include "ImsAosParameter.h"
#include "IuMtsService.h"
#include "MockICarrierConfig.h"
#include "MockIMtsService.h"
#include "MtsDef.h"
#include "PlatformContext.h"
#include "SipStatusCode.h"
#include "TestConfigService.h"
#include "core/MockIMessage.h"
#include "message/MtsErrorHandler.h"
#include "utility/MtsDynamicLoader.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;

class MtsErrorHandlerTest : public ::testing::Test
{
public:
    MockIMessage objMockMessage;
    MockIMtsService objMockMtsService;
    MtsDynamicLoader* pMtsDynamicLoader;
    MtsErrorHandler* pMtsErrorHandler;
    TestConfigService objConfigService;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);

        pMtsDynamicLoader = new MtsDynamicLoader(SLOT_ID);
        pMtsDynamicLoader->Initialize();
        pMtsErrorHandler = new MtsErrorHandler(&objConfigService.GetMockCarrierConfig());
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
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_403));

    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_400);
    objArray.Push(SipStatusCode::SC_403);
    objArray.Push(SipStatusCode::SC_404);
    objArray.Push(SipStatusCode::SC_406);
    objArray.Push(SipStatusCode::SC_408);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_403_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::REGISTER_REFRESH));

    EXPECT_CALL(objMockMtsService, RequestRegistrationRecovery(ImsAosControl::REGISTER_REFRESH))
            .Times(1);

    IMS_SINT32 nResult =
            pMtsErrorHandler->Handle(&objMockMtsService, pMtsDynamicLoader, &objMockMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle404Error)
{
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_404));

    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_400);
    objArray.Push(SipStatusCode::SC_403);
    objArray.Push(SipStatusCode::SC_404);
    objArray.Push(SipStatusCode::SC_406);
    objArray.Push(SipStatusCode::SC_408);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_404_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::PCSCF_NEXT));

    EXPECT_CALL(objMockMtsService, RequestRegistrationRecovery(ImsAosControl::PCSCF_NEXT)).Times(1);

    IMS_SINT32 nResult =
            pMtsErrorHandler->Handle(&objMockMtsService, pMtsDynamicLoader, &objMockMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle406Error)
{
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_406));

    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_400);
    objArray.Push(SipStatusCode::SC_403);
    objArray.Push(SipStatusCode::SC_404);
    objArray.Push(SipStatusCode::SC_406);
    objArray.Push(SipStatusCode::SC_408);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_406_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::IPSEC_DISABLED));

    EXPECT_CALL(objMockMtsService, RequestRegistrationRecovery(ImsAosControl::IPSEC_DISABLED))
            .Times(1);

    IMS_SINT32 nResult =
            pMtsErrorHandler->Handle(&objMockMtsService, pMtsDynamicLoader, &objMockMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle408Error)
{
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_408));

    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_400);
    objArray.Push(SipStatusCode::SC_403);
    objArray.Push(SipStatusCode::SC_404);
    objArray.Push(SipStatusCode::SC_406);
    objArray.Push(SipStatusCode::SC_408);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_408_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult =
            pMtsErrorHandler->Handle(&objMockMtsService, pMtsDynamicLoader, &objMockMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle500Error)
{
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_500));

    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_500);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_504);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_500_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult =
            pMtsErrorHandler->Handle(&objMockMtsService, pMtsDynamicLoader, &objMockMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle503Error)
{
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));

    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_500);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_504);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_503_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult =
            pMtsErrorHandler->Handle(&objMockMtsService, pMtsDynamicLoader, &objMockMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle503ErrorWithReasonHeader)
{
    AString strReasonPhrase = "OUTAGE";
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(objMockMessage, GetReasonPhrase()).WillByDefault(ReturnRef(strReasonPhrase));

    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_500);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_504);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_503_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::REGISTER_REINITIATE));

    EXPECT_CALL(objMockMtsService, RequestRegistrationRecovery(ImsAosControl::REGISTER_REINITIATE))
            .Times(1);

    IMS_SINT32 nResult =
            pMtsErrorHandler->Handle(&objMockMtsService, pMtsDynamicLoader, &objMockMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle503ErrorWithoutReasonHeader)
{
    AString strReasonPhrase = AString::ConstEmpty();
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_503));
    ON_CALL(objMockMessage, GetReasonPhrase()).WillByDefault(ReturnRef(strReasonPhrase));

    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_500);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_504);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_503_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::REGISTER_REINITIATE));

    EXPECT_CALL(objMockMtsService, RequestRegistrationRecovery(_)).Times(0);

    IMS_SINT32 nResult =
            pMtsErrorHandler->Handle(&objMockMtsService, pMtsDynamicLoader, &objMockMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, Handle504Error)
{
    ON_CALL(objMockMessage, GetStatusCode()).WillByDefault(Return(SipStatusCode::SC_504));

    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_500);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_504);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_504_RESPONSE_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult =
            pMtsErrorHandler->Handle(&objMockMtsService, pMtsDynamicLoader, &objMockMessage);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, HandleTimerFExpiredAndReportGenericError)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_406);
    objArray.Push(SipStatusCode::SC_503);
    objArray.Push(SipStatusCode::SC_INVALID);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_EXPIRY_TIMER_F_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(&objMockMtsService, pMtsDynamicLoader);
    EXPECT_EQ(nResult, MO_ERROR_GENERIC);
}

TEST_F(MtsErrorHandlerTest, HandleTimerFExpiredAndReportErrorRetry)
{
    ImsVector<IMS_SINT32> objArray;
    objArray.Push(SipStatusCode::SC_406);
    objArray.Push(SipStatusCode::SC_503);

    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetIntArray(CarrierConfig::Assets::KEY_SMS_GENERIC_ERROR_CODES_INT_ARRAY))
            .WillByDefault(Return(objArray));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Assets::KEY_SMS_POLICY_FOR_EXPIRY_TIMER_F_INT,
                    MTS_REG_RECOVERY_POLICY_NONE))
            .WillByDefault(Return(ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION));

    EXPECT_CALL(objMockMtsService,
            RequestRegistrationRecovery(
                    ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION))
            .Times(1);

    IMS_SINT32 nResult = pMtsErrorHandler->Handle(&objMockMtsService, pMtsDynamicLoader);
    EXPECT_EQ(nResult, MO_ERROR_RETRY);
}

}  // namespace android
