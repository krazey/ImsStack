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

#include "CarrierConfig.h"
#include "IIpcan.h"
#include "utility/MtsGeolocationUtils.h"
#include <gtest/gtest.h>

namespace android
{

class MtsGeolocationUtilsTest : public ::testing::Test
{
public:
    MtsGeolocationUtils* pMtsGeolocationUtils;

protected:
    virtual void SetUp() override { pMtsGeolocationUtils = new MtsGeolocationUtils(); }

    virtual void TearDown() override { delete pMtsGeolocationUtils; }
};

TEST_F(MtsGeolocationUtilsTest, Constructor)
{
    ASSERT_NE(pMtsGeolocationUtils, nullptr);
}

TEST_F(MtsGeolocationUtilsTest, GetGeolocationPidfAllowedType)
{
    EXPECT_EQ(pMtsGeolocationUtils->GetGeolocationPidfAllowedType(IIpcan::CATEGORY_WLAN, IMS_FALSE),
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI);
    EXPECT_EQ(
            pMtsGeolocationUtils->GetGeolocationPidfAllowedType(IIpcan::CATEGORY_MOBILE, IMS_FALSE),
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR);
    EXPECT_EQ(pMtsGeolocationUtils->GetGeolocationPidfAllowedType(IIpcan::CATEGORY_WLAN, IMS_TRUE),
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI);
    EXPECT_EQ(
            pMtsGeolocationUtils->GetGeolocationPidfAllowedType(IIpcan::CATEGORY_MOBILE, IMS_TRUE),
            CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR);
}

}  // namespace android
