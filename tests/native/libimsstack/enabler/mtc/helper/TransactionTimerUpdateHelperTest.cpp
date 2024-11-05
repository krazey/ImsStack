/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "MockIMtcService.h"
#include "MockISipConfig.h"
#include "MockISipConfigV.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "configuration/ConfigDef.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "helper/TransactionTimerUpdateHelper.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

const LOCAL IMS_SINT32 TIMER_VALUE = 12000;
const LOCAL AString TIMER_VALUE_STR = "12000";

namespace android
{

class TransactionTimerUpdateHelperTest : public ::testing::Test
{
public:
    inline TransactionTimerUpdateHelperTest() :
            objContext(),
            pConfigurationManager(new MockIMtcConfigurationManager()),
            objConfigurationProxy(pConfigurationManager),
            objService()
    {
    }

    inline ~TransactionTimerUpdateHelperTest() {}

protected:
    MockIMtcCallContext objContext;
    MockIMtcConfigurationManager* pConfigurationManager;
    MockMtcConfigurationProxy objConfigurationProxy;
    MockISipConfig objSipConfig;
    MockISipConfigV objSipConfigV;
    MockIConfigurable objConfigurable;
    MockIMtcService objService;
    CallInfo objCallInfo;

    inline void SetUp() override
    {
        objCallInfo.eEmergencyType = EmergencyType::NONE;

        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(IMS_SLOT_0));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

        ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

        ON_CALL(objSipConfig, GetSipConfigV).WillByDefault(Return(&objSipConfigV));
        ON_CALL(objSipConfigV, GetConfigurable).WillByDefault(Return(&objConfigurable));
    }
};

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithEcallDoesNothingIfEcallTimeoutPolicyIsWait)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteEmergencyCall)
            .WillByDefault(Return(
                    CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);
    objUpdateHelper.SetInviteTransactionTimer();
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithEcallUpdatesTimerIfEcallTimeoutPolicyIsNotWait)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteEmergencyCall)
            .WillByDefault(Return(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB));
    ON_CALL(*pConfigurationManager, GetEmergencyTCallTimer).WillByDefault(Return(TIMER_VALUE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, TIMER_VALUE_STR));
    objUpdateHelper.SetInviteTransactionTimer();

    // EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, 0));
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithNormalCallDoesNothingIfTimeoutPolicyIsWait)
{
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteCall)
            .WillByDefault(Return(
                    CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);
    objUpdateHelper.SetInviteTransactionTimer();
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithNormalCallUpdatesTimerIfTimeoutPolicyIsNotWait)
{
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteCall)
            .WillByDefault(Return(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB));
    ON_CALL(*pConfigurationManager, GetMoCallRequestTimeout).WillByDefault(Return(TIMER_VALUE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, TIMER_VALUE_STR));
    objUpdateHelper.SetInviteTransactionTimer();

    // EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, 0));
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithNormalWifiCallDoesNothingIfTimeoutPolicyIsWait)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVowifiCall)
            .WillByDefault(Return(
                    CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);
    objUpdateHelper.SetInviteTransactionTimer();
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithNormalWifiCallUpdatesTimerIfTimeoutPolicyIsNotWait)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVowifiCall)
            .WillByDefault(Return(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB));
    ON_CALL(*pConfigurationManager, GetMoCallRequestTimeout).WillByDefault(Return(TIMER_VALUE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, TIMER_VALUE_STR));
    objUpdateHelper.SetInviteTransactionTimer();

    // EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, 0));
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest, SetNonInviteTransactionTimerUpdatesTimer)
{
    ON_CALL(*pConfigurationManager, GetPrackUpdateResponseWaitTimer)
            .WillByDefault(Return(TIMER_VALUE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_F, TIMER_VALUE_STR));
    objUpdateHelper.SetNonInviteTransactionTimer();

    // EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_F, 0));
    objUpdateHelper.ResetNonInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest, UpdateTimerDoesNothingIfConfigIsNull)
{
    ON_CALL(*pConfigurationManager, GetPrackUpdateResponseWaitTimer)
            .WillByDefault(Return(TIMER_VALUE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);
    TransactionTimerUpdateHelper objUpdateHelperWithNullConfig =
            TransactionTimerUpdateHelper(objContext, IMS_NULL);

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);

    objUpdateHelperWithNullConfig.SetNonInviteTransactionTimer();

    ON_CALL(objSipConfigV, GetConfigurable).WillByDefault(Return(nullptr));
    objUpdateHelper.SetNonInviteTransactionTimer();

    ON_CALL(objSipConfig, GetSipConfigV).WillByDefault(Return(nullptr));
    objUpdateHelper.SetNonInviteTransactionTimer();
}

}  // namespace android
