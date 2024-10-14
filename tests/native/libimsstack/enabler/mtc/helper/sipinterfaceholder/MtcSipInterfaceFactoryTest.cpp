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

#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/ReferenceInterfaceHolder.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "helper/sipinterfaceholder/SubscriptionInterfaceHolder.h"
#include <gtest/gtest.h>

namespace android
{

class MtcSipInterfaceFactoryTest : public ::testing::Test
{
public:
    MtcSipInterfaceFactory* pFactory;

protected:
    virtual void SetUp() override { pFactory = new MtcSipInterfaceFactory(); }

    virtual void TearDown() override { delete pFactory; }
};

TEST_F(MtcSipInterfaceFactoryTest, ReferenceInterfaceHolderIsReturnedForTheFirstInvoking)
{
    ASSERT_NE(pFactory->GetIReferenceHolder(), nullptr);
}

TEST_F(MtcSipInterfaceFactoryTest, SubscriptionInterfaceHolderIsReturnedForTheFirstInvoking)
{
    ASSERT_NE(pFactory->GetISubscriptionHolder(), nullptr);
}

TEST_F(MtcSipInterfaceFactoryTest, SameReferenceInterfaceHolderIsReturnedForTheSecondInvoking)
{
    ReferenceInterfaceHolder* pHolder1 = pFactory->GetIReferenceHolder();
    ASSERT_NE(pHolder1, nullptr);
    ReferenceInterfaceHolder* pHolder2 = pFactory->GetIReferenceHolder();
    EXPECT_EQ(pHolder1, pHolder2);
}

TEST_F(MtcSipInterfaceFactoryTest, SameSubscriptionInterfaceHolderIsReturnedForTheSecondInvoking)
{
    SubscriptionInterfaceHolder* pHolder1 = pFactory->GetISubscriptionHolder();
    ASSERT_NE(pHolder1, nullptr);
    SubscriptionInterfaceHolder* pHolder2 = pFactory->GetISubscriptionHolder();
    EXPECT_EQ(pHolder1, pHolder2);
}

TEST_F(MtcSipInterfaceFactoryTest,
        SameSessionInterfaceHolderIsReturnedForTheSecondInvokingAfterReleased)
{
    SessionInterfaceHolder* pHolder1 = &pFactory->GetISessionHolder();
    pFactory->OnSessionInterfaceReleased(0);
    SessionInterfaceHolder* pHolder2 = &pFactory->GetISessionHolder();
    EXPECT_EQ(pHolder1, pHolder2);
}

TEST_F(MtcSipInterfaceFactoryTest,
        NewReferenceInterfaceHolderIsReturnedForTheSecondInvokingAfterCleared)
{
    ReferenceInterfaceHolder* pHolder1 = pFactory->GetIReferenceHolder();
    ASSERT_NE(pHolder1, nullptr);
    EXPECT_TRUE(pFactory->IsReferenceHolderExist());
    pFactory->OnReferenceInterfaceCleared();
    EXPECT_FALSE(pFactory->IsReferenceHolderExist());
    ReferenceInterfaceHolder* pHolder2 = pFactory->GetIReferenceHolder();
    ASSERT_NE(pHolder2, nullptr);
}

TEST_F(MtcSipInterfaceFactoryTest,
        NewSubscriptionInterfaceHolderIsReturnedForTheSecondInvokingAfterCleared)
{
    SubscriptionInterfaceHolder* pHolder1 = pFactory->GetISubscriptionHolder();
    ASSERT_NE(pHolder1, nullptr);
    EXPECT_TRUE(pFactory->IsSubscriptionHolderExist());
    pFactory->OnSubscriptionInterfaceCleared();
    EXPECT_FALSE(pFactory->IsSubscriptionHolderExist());
    SubscriptionInterfaceHolder* pHolder2 = pFactory->GetISubscriptionHolder();
    ASSERT_NE(pHolder2, nullptr);
}

}  // namespace android
