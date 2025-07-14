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

#include "SipDefNetworkUtil.h"
#include "SipUtil.h"

namespace android
{

class SipUtilTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipUtilTest, UtilityTest)
{
    SipUtil* pUtil = SipUtil::GetInstance();
    ASSERT_TRUE(pUtil != nullptr);

    /* Calling again to verify new object is not created */
    ASSERT_TRUE(pUtil == SipUtil::GetInstance());

    EXPECT_TRUE(pUtil->GetLogger() != nullptr);
    EXPECT_TRUE(pUtil->GetNetwork() != nullptr);
    EXPECT_TRUE(pUtil->GetTransactionCallback() == nullptr);

    ISipNetworkUtil* pNetworkUtil = new SipDefNetworkUtil();
    pUtil->SetNetwork(pNetworkUtil);
    EXPECT_TRUE(pNetworkUtil == pUtil->GetNetwork());

    SipUtil::DestroyInstance();
}

}  // namespace android
