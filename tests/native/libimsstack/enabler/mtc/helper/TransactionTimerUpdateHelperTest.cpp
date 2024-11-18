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
#include "configuration/MockMtcConfigurationProxy.h"
#include "helper/TransactionTimerUpdateHelper.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

const LOCAL IMS_SINT32 INITIAL_TIMER_VALUE = 64000;
const LOCAL AString INITIAL_TIMER_VALUE_STR = "64000";

const LOCAL IMS_SINT32 TIMER_VALUE = 12000;
const LOCAL AString TIMER_VALUE_STR = "12000";

namespace android
{

class TransactionTimerUpdateHelperTest : public ::testing::Test
{
public:
    inline TransactionTimerUpdateHelperTest() :
            objContext(),
            objConfigurationProxy(),
            objService()
    {
    }

    inline ~TransactionTimerUpdateHelperTest() {}

protected:
    MockIMtcCallContext objContext;
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

        ON_CALL(objConfigurationProxy, GetInt(ConfigIms::KEY_SIP_TIMER_B_MILLIS_INT))
                .WillByDefault(Return(INITIAL_TIMER_VALUE));
        ON_CALL(objConfigurationProxy, GetInt(ConfigIms::KEY_SIP_TIMER_F_MILLIS_INT))
                .WillByDefault(Return(INITIAL_TIMER_VALUE));

        DisableUpdateForEpsFallback();
        DisableUpdateForTcallTimerExpiry();
    }

    void DisableUpdateForEpsFallback()
    {
        ON_CALL(objConfigurationProxy,
                GetInt(ConfigVoice::
                                KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT))
                .WillByDefault(Return(-1));
        ON_CALL(objService, IsNr).WillByDefault(Return(IMS_FALSE));
    }

    void DisableUpdateForTcallTimerExpiry()
    {
        ON_CALL(objConfigurationProxy,
                GetInt(ConfigEmergency::
                                KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL_INT))
                .WillByDefault(
                        Return(ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE));
        ON_CALL(objConfigurationProxy,
                GetInt(ConfigVoice::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT))
                .WillByDefault(
                        Return(ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE));
    }
};

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerDoesNothingForEpsFallbackIfEpsfbTimerIsNotSet)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT))
            .WillByDefault(Return(-1));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);
    objUpdateHelper.SetInviteTransactionTimer();
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerDoesNothingForEpsFallbackIfNotInNr)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT))
            .WillByDefault(Return(TIMER_VALUE));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_FALSE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);
    objUpdateHelper.SetInviteTransactionTimer();
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest, SetInviteTransactionTimerUpdatesForEpsFallback)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_FOR_EPS_FALLBACK_TRIGGER_MILLIS_INT))
            .WillByDefault(Return(TIMER_VALUE));
    ON_CALL(objService, IsNr).WillByDefault(Return(IMS_TRUE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, TIMER_VALUE_STR));
    objUpdateHelper.SetInviteTransactionTimer();

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, INITIAL_TIMER_VALUE_STR));
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithEcallDoesNothingIfEcallTimeoutPolicyIsWait)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigEmergency::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL_INT))
            .WillByDefault(Return(ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE));

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
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigEmergency::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_EMERGENCY_CALL_INT))
            .WillByDefault(Return(ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB));
    ON_CALL(objConfigurationProxy, GetInt(ConfigEmergency::KEY_EMERGENCY_TCALL_TIMER_MILLIS_INT))
            .WillByDefault(Return(TIMER_VALUE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, TIMER_VALUE_STR));
    objUpdateHelper.SetInviteTransactionTimer();

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, INITIAL_TIMER_VALUE_STR));
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithNormalCallDoesNothingIfTimeoutPolicyIsWait)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT))
            .WillByDefault(Return(ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(_, _)).Times(0);
    objUpdateHelper.SetInviteTransactionTimer();
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithNormalCallUpdatesTimerIfTimeoutPolicyIsNotWait)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOLTE_CALL_INT))
            .WillByDefault(Return(ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB));
    ON_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(TIMER_VALUE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, TIMER_VALUE_STR));
    objUpdateHelper.SetInviteTransactionTimer();

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, INITIAL_TIMER_VALUE_STR));
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest,
        SetInviteTransactionTimerWithNormalWifiCallDoesNothingIfTimeoutPolicyIsWait)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigWfc::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL_INT))
            .WillByDefault(Return(ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_WAIT_FOR_RESPONSE));

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
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigWfc::KEY_POLICY_FOR_TCALL_TIMER_EXPIRY_OF_VOWIFI_CALL_INT))
            .WillByDefault(Return(ConfigVoice::MO_CALL_REQUEST_TIMEOUT_POLICY_CSFB));
    ON_CALL(objConfigurationProxy, GetInt(ConfigVoice::KEY_MO_CALL_REQUEST_TIMEOUT_MILLIS_INT))
            .WillByDefault(Return(TIMER_VALUE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, TIMER_VALUE_STR));
    objUpdateHelper.SetInviteTransactionTimer();

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_B, INITIAL_TIMER_VALUE_STR));
    objUpdateHelper.ResetInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest, SetNonInviteTransactionTimerUpdatesTimer)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_PRACK_UPDATE_RESPONSE_WAIT_TIMER_MILLIS_INT))
            .WillByDefault(Return(TIMER_VALUE));

    TransactionTimerUpdateHelper objUpdateHelper =
            TransactionTimerUpdateHelper(objContext, &objSipConfig);

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_F, TIMER_VALUE_STR));
    objUpdateHelper.SetNonInviteTransactionTimer();

    EXPECT_CALL(objConfigurable, Update(IConfigurable::CP_I_TIMER_F, INITIAL_TIMER_VALUE_STR));
    objUpdateHelper.ResetNonInviteTransactionTimer();
}

TEST_F(TransactionTimerUpdateHelperTest, UpdateTimerDoesNothingIfConfigIsNull)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_PRACK_UPDATE_RESPONSE_WAIT_TIMER_MILLIS_INT))
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
