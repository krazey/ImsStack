/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "AString.h"
#include "CarrierConfig.h"
#include "ImsTypeDef.h"
#include "MockIConfigurable.h"
#include "MockISipConfig.h"
#include "MockISipConfigV.h"
#include "MtsDef.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "helper/MtsTransactionTimerUpdateHelper.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

const LOCAL AString MESSAGE_RESPONSE_WAIT_TIMER_STR = "8000";
const LOCAL AString TIMER_F_STR = "128000";
const LOCAL IMS_SINT32 MESSAGE_RESPONSE_WAIT_TIMER = 8000;
const LOCAL IMS_SINT32 TIMER_F = 128000;
const LOCAL IMS_SINT32 INVALID_PARAMETER_INPUT = -1;

namespace android
{

class MtsTransactionTimerUpdateHelperTest : public ::testing::Test
{
public:
    MockISipConfig objSipConfig;
    MockISipConfigV objSipConfigV;
    MockIConfigurable objConfigurable;
    TestConfigService objConfigService;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);

        ON_CALL(objSipConfig, GetSipConfigV).WillByDefault(Return(&objSipConfigV));
        ON_CALL(objSipConfigV, GetConfigurable).WillByDefault(Return(&objConfigurable));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
    }
};

TEST_F(MtsTransactionTimerUpdateHelperTest, SetMessageTransactionTimerWhenNoUpdateNeeded)
{
    MtsTransactionTimerUpdateHelper objMtsUpdateHelper = MtsTransactionTimerUpdateHelper(
            &objConfigService.GetMockCarrierConfig(), &objSipConfig);
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT, _))
            .WillByDefault(Return(MESSAGE_RESPONSE_WAIT_TIMER));

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);

    objMtsUpdateHelper.SetMessageTransactionTimer(SMS_3GPP_MTI_RP_ACK_FROM_MS);
}

TEST_F(MtsTransactionTimerUpdateHelperTest, SetMessageTransactionTimerWhenUpdateNeeded)
{
    MtsTransactionTimerUpdateHelper objMtsUpdateHelper = MtsTransactionTimerUpdateHelper(
            &objConfigService.GetMockCarrierConfig(), &objSipConfig);
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT, _))
            .WillByDefault(Return(MESSAGE_RESPONSE_WAIT_TIMER));

    EXPECT_CALL(
            objConfigurable, Update(IConfigurable::CP_I_TIMER_F, MESSAGE_RESPONSE_WAIT_TIMER_STR))
            .Times(1);

    objMtsUpdateHelper.SetMessageTransactionTimer(SMS_3GPP_MTI_RP_DATA_FROM_MS);
}

TEST_F(MtsTransactionTimerUpdateHelperTest, ResetMessageTransactionTimerWhenNoUpdateNeeded)
{
    MtsTransactionTimerUpdateHelper objMtsUpdateHelper = MtsTransactionTimerUpdateHelper(
            &objConfigService.GetMockCarrierConfig(), &objSipConfig);
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT, _))
            .WillByDefault(Return(MESSAGE_RESPONSE_WAIT_TIMER));

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);

    objMtsUpdateHelper.ResetMessageTransactionTimer(SMS_3GPP_MTI_RP_ERROR_FROM_MS);
}

TEST_F(MtsTransactionTimerUpdateHelperTest, ResetMessageTransactionTimerWhenUpdateNeeded)
{
    MtsTransactionTimerUpdateHelper objMtsUpdateHelper = MtsTransactionTimerUpdateHelper(
            &objConfigService.GetMockCarrierConfig(), &objSipConfig);
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT, _))
            .WillByDefault(Return(MESSAGE_RESPONSE_WAIT_TIMER));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return(TIMER_F));

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_F, TIMER_F_STR)).Times(1);

    objMtsUpdateHelper.ResetMessageTransactionTimer(SMS_3GPP_MTI_RP_DATA_FROM_MS);
}

TEST_F(MtsTransactionTimerUpdateHelperTest, UpdateTimerDoesNothingIfInvalidParameter)
{
    MtsTransactionTimerUpdateHelper objMtsUpdateHelper = MtsTransactionTimerUpdateHelper(
            &objConfigService.GetMockCarrierConfig(), &objSipConfig);
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT, _))
            .WillByDefault(Return(MESSAGE_RESPONSE_WAIT_TIMER));
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::Ims::KEY_SIP_TIMER_F_MILLIS_INT, _))
            .WillByDefault(Return(INVALID_PARAMETER_INPUT));

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);

    objMtsUpdateHelper.ResetMessageTransactionTimer(SMS_3GPP_MTI_RP_DATA_FROM_MS);
}

TEST_F(MtsTransactionTimerUpdateHelperTest, UpdateTimerDoesNothingIfConfigIsNull)
{
    MtsTransactionTimerUpdateHelper objMtsUpdateHelper =
            MtsTransactionTimerUpdateHelper(&objConfigService.GetMockCarrierConfig(), IMS_NULL);
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT, _))
            .WillByDefault(Return(MESSAGE_RESPONSE_WAIT_TIMER));

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);

    objMtsUpdateHelper.SetMessageTransactionTimer(SMS_3GPP_MTI_RP_DATA_FROM_MS);
}

TEST_F(MtsTransactionTimerUpdateHelperTest, UpdateTimerDoesNothingIfConfigVIsNull)
{
    MtsTransactionTimerUpdateHelper objMtsUpdateHelper = MtsTransactionTimerUpdateHelper(
            &objConfigService.GetMockCarrierConfig(), &objSipConfig);
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT, _))
            .WillByDefault(Return(MESSAGE_RESPONSE_WAIT_TIMER));
    ON_CALL(objSipConfig, GetSipConfigV).WillByDefault(Return(nullptr));

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);

    objMtsUpdateHelper.SetMessageTransactionTimer(SMS_3GPP_MTI_RP_DATA_FROM_MS);
}

TEST_F(MtsTransactionTimerUpdateHelperTest, UpdateTimerDoesNothingIfConfigurableIsNull)
{
    MtsTransactionTimerUpdateHelper objMtsUpdateHelper = MtsTransactionTimerUpdateHelper(
            &objConfigService.GetMockCarrierConfig(), &objSipConfig);
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT, _))
            .WillByDefault(Return(MESSAGE_RESPONSE_WAIT_TIMER));
    ON_CALL(objSipConfigV, GetConfigurable).WillByDefault(Return(nullptr));

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);

    objMtsUpdateHelper.SetMessageTransactionTimer(SMS_3GPP_MTI_RP_DATA_FROM_MS);
}

TEST_F(MtsTransactionTimerUpdateHelperTest, UpdateTimerDoesNothingIfUpdateFailed)
{
    MtsTransactionTimerUpdateHelper objMtsUpdateHelper = MtsTransactionTimerUpdateHelper(
            &objConfigService.GetMockCarrierConfig(), &objSipConfig);
    ON_CALL(objConfigService.GetMockCarrierConfig(),
            GetInt(CarrierConfig::ImsSms::KEY_SMS_MESSAGE_RESPONSE_WAIT_TIMER_MILLIS_INT, _))
            .WillByDefault(Return(MESSAGE_RESPONSE_WAIT_TIMER));
    ON_CALL(objConfigurable, Update).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(
            objConfigurable, Update(IConfigurable::CP_I_TIMER_F, MESSAGE_RESPONSE_WAIT_TIMER_STR))
            .Times(1);

    objMtsUpdateHelper.SetMessageTransactionTimer(SMS_3GPP_MTI_RP_DATA_FROM_MS);
}

}  // namespace android
