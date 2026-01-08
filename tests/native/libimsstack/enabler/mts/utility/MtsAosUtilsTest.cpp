/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "utility/MtsAosUtils.h"
#include "ImsAosParameter.h"
#include "MtsDef.h"
#include <gtest/gtest.h>

namespace android
{

class MtsAosUtilsTest : public ::testing::Test
{
public:
    MtsAosUtils* pMtsAosUtils;

protected:
    virtual void SetUp() override { pMtsAosUtils = new MtsAosUtils(); }

    virtual void TearDown() override { delete pMtsAosUtils; }
};

TEST_F(MtsAosUtilsTest, Constructor)
{
    ASSERT_NE(pMtsAosUtils, nullptr);
}

TEST_F(MtsAosUtilsTest, ConvertToAosControl)
{
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(MtsRegRecoveryPolicy::REGISTER_START),
            ImsAosControl::REGISTER_START);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(MtsRegRecoveryPolicy::REGISTER_START_WITH_WLAN),
            ImsAosControl::REGISTER_START_WITH_WLAN);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(MtsRegRecoveryPolicy::REGISTER_REFRESH),
            ImsAosControl::REGISTER_REFRESH);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(MtsRegRecoveryPolicy::REGISTER_STOP),
            ImsAosControl::REGISTER_STOP);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(MtsRegRecoveryPolicy::REGISTER_REINITIATE),
            ImsAosControl::REGISTER_REINITIATE);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(
                      MtsRegRecoveryPolicy::E_REGISTER_FAKE_WITH_NEXT_PCSCF),
            ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(MtsRegRecoveryPolicy::PCSCF_NEXT),
            ImsAosControl::PCSCF_NEXT);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(MtsRegRecoveryPolicy::PCSCF_NEXT_WITH_DISCOVERY),
            ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(MtsRegRecoveryPolicy::IPSEC_DISABLED),
            ImsAosControl::IPSEC_DISABLED);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(MtsRegRecoveryPolicy::RETRY_COUNT_INCREASE),
            ImsAosControl::RETRY_COUNT_INCREASE);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(
                      MtsRegRecoveryPolicy::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION),
            ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(
                      MtsRegRecoveryPolicy::UPDATE_SIP_DELEGATE_REGISTRATION),
            ImsAosControl::UPDATE_SIP_DELEGATE_REGISTRATION);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(
                      MtsRegRecoveryPolicy::TRIGGER_SIP_DELEGATE_DEREGISTRATION),
            ImsAosControl::TRIGGER_SIP_DELEGATE_DEREGISTRATION);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(
                      MtsRegRecoveryPolicy::TRIGGER_FULL_NETWORK_REGISTRATION),
            ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(MtsRegRecoveryPolicy::PLMN_BLOCK_WITH_TIMEOUT),
            ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(
                      MtsRegRecoveryPolicy::E_REGISTER_FAKE_WITH_SAME_PCSCF),
            ImsAosControl::E_REGISTER_FAKE_WITH_SAME_PCSCF);
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE),
            MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE);
    // Test default case with an invalid value
    EXPECT_EQ(pMtsAosUtils->ConvertToAosControl(999),
            MtsRegRecoveryPolicy::MTS_REG_RECOVERY_POLICY_NONE);
}

}  // namespace android
