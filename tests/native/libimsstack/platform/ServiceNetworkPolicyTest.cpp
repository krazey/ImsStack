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

#include "PlatformContext.h"
#include "ServiceNetworkPolicy.h"

namespace android
{

static const AString strProfileName1("mobile_test_1");
static const AString strProfileName2("wifi_test_2");

class NetworkServicePolicyTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(NetworkServicePolicyTest, AddPolicy)
{
    NetworkServicePolicy* pNetworkServicePolicy = NetworkServicePolicy::GetInstance();
    NetworkPolicy objNetworkPolicy(IMS_TRUE, strProfileName1, NetworkPolicy::APN_WIFI_MAX);

    pNetworkServicePolicy->AddPolicy(strProfileName1, objNetworkPolicy, IMS_SLOT_0);

    const NetworkPolicy* pNetworkPolicy =
            pNetworkServicePolicy->GetPolicy(strProfileName1, IMS_SLOT_0);

    EXPECT_EQ(pNetworkPolicy->GetName(), objNetworkPolicy.GetName());
    EXPECT_EQ(pNetworkPolicy->GetApnType(), objNetworkPolicy.GetApnType());

    pNetworkServicePolicy->RemovePolicy(strProfileName1, IMS_SLOT_0);

    EXPECT_TRUE(pNetworkServicePolicy->GetPolicy(strProfileName1, IMS_SLOT_0) == IMS_NULL);
}

TEST_F(NetworkServicePolicyTest, GetPolicy_ApnType)
{
    NetworkServicePolicy* pNetworkServicePolicy = NetworkServicePolicy::GetInstance();
    NetworkPolicy objNetworkPolicy(IMS_TRUE, strProfileName1, NetworkPolicy::APN_WIFI_MAX);

    pNetworkServicePolicy->AddPolicy(strProfileName1, objNetworkPolicy, IMS_SLOT_0);

    const NetworkPolicy* pNetworkPolicy =
            pNetworkServicePolicy->GetPolicy(NetworkPolicy::APN_WIFI_MAX, IMS_SLOT_0);

    EXPECT_EQ(pNetworkPolicy->GetName(), objNetworkPolicy.GetName());
    EXPECT_EQ(pNetworkPolicy->GetApnType(), objNetworkPolicy.GetApnType());

    pNetworkServicePolicy->RemovePolicy(strProfileName1, IMS_SLOT_0);

    EXPECT_TRUE(
            pNetworkServicePolicy->GetPolicy(NetworkPolicy::APN_WIFI_MAX, IMS_SLOT_0) == IMS_NULL);
}

TEST_F(NetworkServicePolicyTest, RemoveAllPolicies)
{
    NetworkServicePolicy* pNetworkServicePolicy = NetworkServicePolicy::GetInstance();
    NetworkPolicy objNetworkPolicy1(IMS_TRUE, strProfileName1, NetworkPolicy::APN_WIFI_MAX);
    NetworkPolicy objNetworkPolicy2(IMS_TRUE, strProfileName2, NetworkPolicy::APN_MOBILE_MAX);

    pNetworkServicePolicy->AddPolicy(strProfileName1, objNetworkPolicy1, IMS_SLOT_0);
    pNetworkServicePolicy->AddPolicy(strProfileName2, objNetworkPolicy2, IMS_SLOT_0);

    const NetworkPolicy* pNetworkPolicy =
            pNetworkServicePolicy->GetPolicy(NetworkPolicy::APN_WIFI_MAX, IMS_SLOT_0);

    EXPECT_EQ(pNetworkPolicy->GetName(), objNetworkPolicy1.GetName());
    EXPECT_EQ(pNetworkPolicy->GetApnType(), objNetworkPolicy1.GetApnType());

    pNetworkServicePolicy->RemoveAllPolicies(IMS_SLOT_0);

    EXPECT_TRUE(
            pNetworkServicePolicy->GetPolicy(NetworkPolicy::APN_WIFI_MAX, IMS_SLOT_0) == IMS_NULL);

    EXPECT_TRUE(
            pNetworkServicePolicy->GetPolicy(NetworkPolicy::APN_WIFI_MAX, IMS_SLOT_0) == IMS_NULL);
    EXPECT_TRUE(pNetworkServicePolicy->GetPolicy(NetworkPolicy::APN_MOBILE_MAX, IMS_SLOT_0) ==
            IMS_NULL);

    // Adding back default policies
    // "ims"
    NetworkPolicy objImsNetworkPolicy(IMS_TRUE, "mobile_ims", NetworkPolicy::APN_IMS);
    pNetworkServicePolicy->AddPolicy(AString("mobile_ims"), objImsNetworkPolicy, IMS_SLOT_0);

    // "emergency"
    NetworkPolicy objEmergencyNetworkPolicy(
            IMS_FALSE, "mobile_emergency", NetworkPolicy::APN_EMERGENCY);
    pNetworkServicePolicy->AddPolicy(
            AString("mobile_emergency"), objEmergencyNetworkPolicy, IMS_SLOT_0);

    // "internet"
    NetworkPolicy objInternetNetworkPolicy(
            IMS_FALSE, "mobile_internet", NetworkPolicy::APN_INTERNET);
    pNetworkServicePolicy->AddPolicy(
            AString("mobile_internet"), objInternetNetworkPolicy, IMS_SLOT_0);

    // "wifi"
    NetworkPolicy objWifiNetworkPolicy(IMS_FALSE, "wifi", NetworkPolicy::APN_WIFI);
    pNetworkServicePolicy->AddPolicy(AString("wifi"), objWifiNetworkPolicy, IMS_SLOT_0);
}

TEST_F(NetworkServicePolicyTest, IsMobileOrWiFiPolicy)
{
    EXPECT_TRUE(NetworkPolicy::IsMobilePolicy(strProfileName1));
    EXPECT_TRUE(NetworkPolicy::IsWiFiPolicy(strProfileName2));

    EXPECT_TRUE(NetworkPolicy::IsMobilePolicy(NetworkPolicy::APN_IMS));
    EXPECT_TRUE(NetworkPolicy::IsMobilePolicy(NetworkPolicy::APN_EMERGENCY));
    EXPECT_TRUE(NetworkPolicy::IsMobilePolicy(NetworkPolicy::APN_INTERNET));
    EXPECT_FALSE(NetworkPolicy::IsMobilePolicy(NetworkPolicy::APN_WIFI_MAX));

    EXPECT_TRUE(NetworkPolicy::IsWiFiPolicy(NetworkPolicy::APN_WIFI));
    EXPECT_FALSE(NetworkPolicy::IsWiFiPolicy(NetworkPolicy::APN_WIFI_MAX));
}

}  // namespace android