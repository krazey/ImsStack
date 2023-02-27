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
#include <gtest/gtest.h>

#include "NatHelper.h"

namespace android
{

class NatHelperTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(NatHelperTest, SetPublicAddress)
{
    NatHelper* pNatHelper = NatHelper::GetInstance();
    IpAddress objPrivateIp("192.168.0.101");
    IpAddress objPublicIp("122.156.139.233");

    pNatHelper->SetPublicAddress(IMS_SLOT_0, 1234, objPrivateIp, objPublicIp);
    EXPECT_EQ(pNatHelper->GetPublicAddress(IMS_SLOT_0, objPrivateIp), objPublicIp);

    EXPECT_EQ(pNatHelper->GetPrivateAddress(IMS_SLOT_0, objPublicIp), objPrivateIp);

    EXPECT_TRUE(pNatHelper->IsBehindNat(IMS_SLOT_0, objPrivateIp));
    IpAddress objPublicIp2("122.156.139.211");
    pNatHelper->SetPublicAddress(IMS_SLOT_0, 1234, objPrivateIp, objPublicIp2);
    pNatHelper->Clear(IMS_SLOT_0);
    EXPECT_EQ(pNatHelper->GetPublicAddress(IMS_SLOT_0, objPrivateIp), IpAddress::NONE);
}

TEST_F(NatHelperTest, RemovePublicAddress)
{
    NatHelper* pNatHelper = NatHelper::GetInstance();
    IpAddress objPrivateIp("192.168.0.101");
    IpAddress objPublicIp("122.156.139.233");

    pNatHelper->SetPublicAddress(IMS_SLOT_0, 1234, objPrivateIp, objPublicIp);

    IpAddress objPvtIp("192.168.0.102");
    IpAddress objPubIp("122.156.139.234");

    pNatHelper->SetPublicAddress(IMS_SLOT_0, 1235, objPvtIp, objPubIp);

    EXPECT_EQ(pNatHelper->GetPublicAddress(IMS_SLOT_0, objPvtIp), objPubIp);

    EXPECT_EQ(pNatHelper->GetPrivateAddress(IMS_SLOT_0, objPublicIp), objPrivateIp);

    EXPECT_TRUE(pNatHelper->IsBehindNat(IMS_SLOT_0, objPvtIp));
    pNatHelper->RemovePublicAddress(IMS_SLOT_0, 1234);
    EXPECT_EQ(pNatHelper->GetPublicAddress(IMS_SLOT_0, objPrivateIp), IpAddress::NONE);

    pNatHelper->RemovePublicAddress(IMS_SLOT_0, objPvtIp);
    EXPECT_EQ(pNatHelper->GetPublicAddress(IMS_SLOT_0, objPvtIp), IpAddress::NONE);
    pNatHelper->RemovePublicAddress(IMS_SLOT_1, objPvtIp);
    pNatHelper->RemovePublicAddress(IMS_SLOT_1, 1234);
}

TEST_F(NatHelperTest, IsBehindNat)
{
    NatHelper* pNatHelper = NatHelper::GetInstance();
    IpAddress objPrivateIp("122.156.139.233");
    IpAddress objPublicIp;

    pNatHelper->SetPublicAddress(IMS_SLOT_0, 1234, objPrivateIp, objPublicIp);

    EXPECT_FALSE(pNatHelper->IsBehindNat(IMS_SLOT_0, objPrivateIp));
    pNatHelper->RemovePublicAddress(IMS_SLOT_0, 1234);
    pNatHelper->RemovePublicAddress(IMS_SLOT_0, 1234);

    EXPECT_EQ(pNatHelper->GetPublicAddress(IMS_SLOT_0, objPrivateIp), IpAddress::NONE);

    EXPECT_EQ(pNatHelper->GetPublicAddress(IMS_SLOT_1, objPrivateIp), IpAddress::NONE);
    EXPECT_EQ(pNatHelper->GetPrivateAddress(IMS_SLOT_1, objPublicIp), IpAddress::NONE);
    EXPECT_FALSE(pNatHelper->IsBehindNat(IMS_SLOT_1, objPrivateIp));
}

}  // namespace android
