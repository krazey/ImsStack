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
#include "MtsServiceState.h"

namespace android
{

const IMS_SINT32 SLOT_ID = 0;

class MtsServiceStateTest : public ::testing::Test
{
public:
    MtsServiceState* pMtsServiceState;

protected:
    virtual void SetUp() override { pMtsServiceState = new MtsServiceState(SLOT_ID); }

    virtual void TearDown() override { delete pMtsServiceState; }
};

TEST_F(MtsServiceStateTest, Constructor)
{
    ASSERT_NE(pMtsServiceState, nullptr);
}

TEST_F(MtsServiceStateTest, ServiceStateAfterAosConnected)
{
    pMtsServiceState->SetSmsOverIpState(IMS_TRUE);
    pMtsServiceState->OnImsConnected();
    pMtsServiceState->UpdateServiceState();

    EXPECT_TRUE(pMtsServiceState->GetImsRegState());
    EXPECT_EQ(pMtsServiceState->GetServiceState(), MtsMessageController::STATE_READY);
}

TEST_F(MtsServiceStateTest, IsServiceBlocked)
{
    pMtsServiceState->SetSmsOverIpState(IMS_TRUE);
    pMtsServiceState->OnImsConnected();
    pMtsServiceState->UpdateServiceState();

    EXPECT_FALSE(pMtsServiceState->IsMoServiceBlocked());
    EXPECT_FALSE(pMtsServiceState->IsMtServiceBlocked());
}

}  // namespace android
