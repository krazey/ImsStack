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

#include "CarrierConfig.h"
#include "ImsTypeDef.h"
#include "MockIMtcService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "configuration/ConfigDef.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "helper/TransactionTimerUpdateHelper.h"
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;

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
    MockIMtcService objService;
    CallInfo objCallInfo;

    inline void SetUp() override
    {
        objCallInfo.bEmergency = IMS_FALSE;

        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(IMS_SLOT_0));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

        ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));
    }
};

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithEcallDoesNothingIfEcallTimeoutPolicyIsWait)
{
    objCallInfo.bEmergency = IMS_TRUE;
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteEmergencyCall)
            .WillByDefault(Return(
                    CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE));
    EXPECT_CALL(*pConfigurationManager, GetEmergencyTCallTimer).Times(0);

    TransactionTimerUpdateHelper(objContext).SetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithEcallUpdateTimerIfEcallTimeoutPolicyIsNotWait)
{
    objCallInfo.bEmergency = IMS_TRUE;
    ON_CALL(*pConfigurationManager, GetPolicyForTcallTimerExpiryOfVolteEmergencyCall)
            .WillByDefault(Return(CarrierConfig::ImsVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB));
    const IMS_SINT32 ANY_TIME = 12000;
    EXPECT_CALL(*pConfigurationManager, GetEmergencyTCallTimer).WillOnce(Return(ANY_TIME));

    TransactionTimerUpdateHelper(objContext).SetInviteTransactionTimer();
}

}  // namespace android
