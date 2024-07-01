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
#include "IConfiguration.h"
#include "IImsAosInfo.h"
#include "ImsAosReason.h"
#include "MockICarrierConfig.h"
#include "MtsServiceState.h"
#include "MtsDef.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "TestConnector.h"
#include "../../interface/aos/MockIImsAos.h"
#include "../../interface/aos/MockIImsAosInfo.h"

using ::testing::_;
using ::testing::Return;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;

class MtsServiceStateTest : public ::testing::Test
{
public:
    MockIImsAos objMockIImsAos;
    MockIImsAosInfo objMockIImsAosInfo;
    MtsServiceState* pMtsServiceState;
    TestConfigService* pConfigService;
    TestConnector objConnector;

protected:
    virtual void SetUp() override
    {
        pConfigService = new TestConfigService();
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, pConfigService);

        pMtsServiceState = new MtsServiceState(SLOT_ID);

        ON_CALL(objMockIImsAos, GetAosInfo()).WillByDefault(Return(&objMockIImsAosInfo));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);

        delete pMtsServiceState;
        delete pConfigService;
    }
};

TEST_F(MtsServiceStateTest, Constructor)
{
    ASSERT_NE(pMtsServiceState, nullptr);
}

TEST_F(MtsServiceStateTest, BlockMoSmsByImsiBasedSipUri)
{
    ON_CALL(objMockIImsAosInfo, GetRegistrationMode())
            .WillByDefault(Return(IImsAosInfo::REG_MODE_ADMIN));

    MockICarrierConfig& objCarrierConfig = pConfigService->GetMockCarrierConfig();

    ON_CALL(objCarrierConfig, GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_SMS_ALLOW_IMSI_BASED_SIP_URI_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    pMtsServiceState->Init(&objMockIImsAos);

    pMtsServiceState->OnImsConnected();

    EXPECT_TRUE(pMtsServiceState->IsMoServiceBlocked());
}

TEST_F(MtsServiceStateTest, AllowMoSmsByImsiBasedSipUri)
{
    ON_CALL(objMockIImsAosInfo, GetRegistrationMode())
            .WillByDefault(Return(IImsAosInfo::REG_MODE_ADMIN));

    MockICarrierConfig& objCarrierConfig = pConfigService->GetMockCarrierConfig();

    ON_CALL(objCarrierConfig, GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_SMS_ALLOW_IMSI_BASED_SIP_URI_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    pMtsServiceState->Init(&objMockIImsAos);

    pMtsServiceState->OnImsConnected();

    EXPECT_FALSE(pMtsServiceState->IsMoServiceBlocked());
}

TEST_F(MtsServiceStateTest, CheckServiceStateByImsConnection)
{
    ON_CALL(objMockIImsAosInfo, GetRegistrationMode())
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));

    MockICarrierConfig& objCarrierConfig = pConfigService->GetMockCarrierConfig();

    ON_CALL(objCarrierConfig, GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL, _))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_SMS_ALLOW_IMSI_BASED_SIP_URI_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    pMtsServiceState->Init(&objMockIImsAos);

    pMtsServiceState->OnImsConnected();
    EXPECT_EQ(pMtsServiceState->GetState(), STATE_READY);

    pMtsServiceState->OnImsSuspended(ImsAosReason::SUSPEND_OUT_OF_SERVICE);
    EXPECT_EQ(pMtsServiceState->GetState(), STATE_LIMITED);

    pMtsServiceState->OnImsResumed();
    EXPECT_EQ(pMtsServiceState->GetState(), STATE_READY);

    pMtsServiceState->OnImsDisconnecting(ImsAosReason::REG_TERMINATED);
    EXPECT_EQ(pMtsServiceState->GetState(), STATE_READY);

    pMtsServiceState->OnImsDisconnected(ImsAosReason::REG_TERMINATED);
    EXPECT_EQ(pMtsServiceState->GetState(), STATE_NOTREADY);
}

TEST_F(MtsServiceStateTest, LimitedStateBySmsOverIpConfiguration)
{
    ON_CALL(objMockIImsAosInfo, GetRegistrationMode())
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));

    MockICarrierConfig& objCarrierConfig = pConfigService->GetMockCarrierConfig();

    ON_CALL(objCarrierConfig, GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objCarrierConfig,
            GetBoolean(CarrierConfig::Assets::KEY_SMS_ALLOW_IMSI_BASED_SIP_URI_BOOL, _))
            .WillByDefault(Return(IMS_FALSE));
    pMtsServiceState->Init(&objMockIImsAos);

    pMtsServiceState->OnImsConnected();
    EXPECT_EQ(pMtsServiceState->GetState(), STATE_LIMITED);
}

}  // namespace android
