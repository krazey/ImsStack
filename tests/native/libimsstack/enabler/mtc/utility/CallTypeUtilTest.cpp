/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "ImsAosParameter.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "utility/CallTypeUtil.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::Return;

TEST(CallTypeUtilTest, RestrictCallTypeByRegisteredFeatureChangesCallTypeWhenAosIsNotConnected)
{
    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::VOIP, IMS_NULL));
    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::VT, IMS_NULL));
    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::RTT, IMS_NULL));
    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::VIDEO_RTT, IMS_NULL));
}

TEST(CallTypeUtilTest, RestrictCallTypeByRegisteredFeatureChangesCallTypeWhenAllFeaturesEnabled)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objAosConnector, GetFeatures)
            .WillByDefault(Return(ImsAosFeature::VIDEO | ImsAosFeature::TEXT));

    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::VOIP, &objAosConnector));
    EXPECT_EQ(CallType::VT,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::VT, &objAosConnector));
    EXPECT_EQ(CallType::RTT,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::RTT, &objAosConnector));
    EXPECT_EQ(CallType::VIDEO_RTT,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(
                    CallType::VIDEO_RTT, &objAosConnector));
}

TEST(CallTypeUtilTest, RestrictCallTypeByRegisteredFeatureChangesCallTypeWhenVideoFeatureDisabled)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::TEXT));

    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::VOIP, &objAosConnector));
    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::VT, &objAosConnector));
    EXPECT_EQ(CallType::RTT,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::RTT, &objAosConnector));
    EXPECT_EQ(CallType::RTT,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(
                    CallType::VIDEO_RTT, &objAosConnector));
}

TEST(CallTypeUtilTest, RestrictCallTypeByRegisteredFeatureChangesCallTypeWhenTextFeatureDisabled)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::VIDEO));

    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::VOIP, &objAosConnector));
    EXPECT_EQ(CallType::VT,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::VT, &objAosConnector));
    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(CallType::RTT, &objAosConnector));
    EXPECT_EQ(CallType::VT,
            CallTypeUtil::RestrictCallTypeByRegisteredFeature(
                    CallType::VIDEO_RTT, &objAosConnector));
}

TEST(CallTypeUtilTest, RestrictCallTypeByCapabilityChangesCallType)
{
    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::VOIP, IMS_TRUE, IMS_TRUE));
    EXPECT_EQ(CallType::VT,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::VT, IMS_TRUE, IMS_TRUE));
    EXPECT_EQ(CallType::RTT,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::RTT, IMS_TRUE, IMS_TRUE));
    EXPECT_EQ(CallType::VIDEO_RTT,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::VIDEO_RTT, IMS_TRUE, IMS_TRUE));

    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::VOIP, IMS_FALSE, IMS_TRUE));
    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::VT, IMS_FALSE, IMS_TRUE));
    EXPECT_EQ(CallType::RTT,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::RTT, IMS_FALSE, IMS_TRUE));
    EXPECT_EQ(CallType::RTT,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::VIDEO_RTT, IMS_FALSE, IMS_TRUE));

    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::VOIP, IMS_TRUE, IMS_FALSE));
    EXPECT_EQ(CallType::VT,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::VT, IMS_TRUE, IMS_FALSE));
    EXPECT_EQ(CallType::VOIP,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::RTT, IMS_TRUE, IMS_FALSE));
    EXPECT_EQ(CallType::VT,
            CallTypeUtil::RestrictCallTypeByCapability(CallType::VIDEO_RTT, IMS_TRUE, IMS_FALSE));
}

TEST(CallTypeUtilTest, GetCallTypeByRegisteredFeatureReturnsCallTypeByConfigIfAosIsNotConnected)
{
    MockMtcConfigurationProxy objConfig;
    EXPECT_EQ(CallType::VOIP, CallTypeUtil::GetCallTypeByRegisteredFeature(IMS_NULL, objConfig));
}

TEST(CallTypeUtilTest, GetCallTypeByRegisteredFeatureReturnsCallTypeByConfigIfAllFeaturesEnabled)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objAosConnector, GetFeatures)
            .WillByDefault(Return(ImsAosFeature::VIDEO | ImsAosFeature::TEXT));
    MockMtcConfigurationProxy objConfig;

    ON_CALL(objConfig, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_NOT_ALLOWED));
    EXPECT_EQ(CallType::VT,
            CallTypeUtil::GetCallTypeByRegisteredFeature(&objAosConnector, objConfig));

    ON_CALL(objConfig, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_NOT_ALLOWED_IF_ACTIVE));
    EXPECT_EQ(CallType::VT,
            CallTypeUtil::GetCallTypeByRegisteredFeature(&objAosConnector, objConfig));

    ON_CALL(objConfig, GetInt(ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT))
            .WillByDefault(Return(ConfigVt::TEXT_VIDEO_ALLOWED));
    EXPECT_EQ(CallType::VIDEO_RTT,
            CallTypeUtil::GetCallTypeByRegisteredFeature(&objAosConnector, objConfig));
}

TEST(CallTypeUtilTest, GetCallTypeByRegisteredFeatureReturnsCallTypeByConfigIfVideoFeatureDisabled)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::TEXT));
    MockMtcConfigurationProxy objConfig;

    EXPECT_EQ(CallType::RTT,
            CallTypeUtil::GetCallTypeByRegisteredFeature(&objAosConnector, objConfig));
}

TEST(CallTypeUtilTest, GetCallTypeByRegisteredFeatureReturnsCallTypeByConfigIfTextFeatureDisabled)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objAosConnector, GetFeatures).WillByDefault(Return(ImsAosFeature::VIDEO));
    MockMtcConfigurationProxy objConfig;

    EXPECT_EQ(CallType::VT,
            CallTypeUtil::GetCallTypeByRegisteredFeature(&objAosConnector, objConfig));
}

TEST(CallTypeUtilTest, IsVideoCallReturnsTrueIfCallTypeContainsVideo)
{
    EXPECT_FALSE(CallTypeUtil::IsVideoCall(CallType::VOIP));
    EXPECT_TRUE(CallTypeUtil::IsVideoCall(CallType::VT));
    EXPECT_FALSE(CallTypeUtil::IsVideoCall(CallType::RTT));
    EXPECT_TRUE(CallTypeUtil::IsVideoCall(CallType::VIDEO_RTT));
}

TEST(CallTypeUtilTest, IsRttCallReturnsTrueIfCallTypeContainsRtt)
{
    EXPECT_FALSE(CallTypeUtil::IsRttCall(CallType::VOIP));
    EXPECT_FALSE(CallTypeUtil::IsRttCall(CallType::VT));
    EXPECT_TRUE(CallTypeUtil::IsRttCall(CallType::RTT));
    EXPECT_TRUE(CallTypeUtil::IsRttCall(CallType::VIDEO_RTT));
}
