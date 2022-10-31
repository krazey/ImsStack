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

#include "configuration/ConfigCache.h"
#include "configuration/ConfigDef.h"
#include <gtest/gtest.h>

namespace android
{

class ConfigCacheTest : public ::testing::Test
{
public:
    ConfigCache objConfigCache;

protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(ConfigCacheTest, PutAndGetBoolCacheAndReset)
{
    Feature eAnyFeature = Feature::SUPPORT_GEOLOCATION_PIDF_IN_SIP_INVITE;
    IMS_BOOL bAnyBooleanValue = IMS_TRUE;
    objConfigCache.PutCache(eAnyFeature, bAnyBooleanValue);
    EXPECT_EQ(bAnyBooleanValue, objConfigCache.GetBooleanCache(eAnyFeature));

    EXPECT_FALSE(objConfigCache.IsEmpty());
    EXPECT_TRUE(objConfigCache.HasBooleanCache(eAnyFeature));
    objConfigCache.ResetCache(eAnyFeature);
    EXPECT_TRUE(objConfigCache.IsEmpty());
    EXPECT_FALSE(objConfigCache.HasBooleanCache(eAnyFeature));
}

TEST_F(ConfigCacheTest, PutAndGetIntegerCacheAndReset)
{
    Feature eAnyFeature = Feature::REQUEST_URI_TYPE;
    IMS_SINT32 nAnyIntegerValue = 12345;
    objConfigCache.PutCache(eAnyFeature, nAnyIntegerValue);
    EXPECT_EQ(nAnyIntegerValue, objConfigCache.GetIntegerCache(eAnyFeature));

    EXPECT_FALSE(objConfigCache.IsEmpty());
    EXPECT_TRUE(objConfigCache.HasIntegerCache(eAnyFeature));
    objConfigCache.ResetCache(eAnyFeature);
    EXPECT_TRUE(objConfigCache.IsEmpty());
    EXPECT_FALSE(objConfigCache.HasIntegerCache(eAnyFeature));
}

TEST_F(ConfigCacheTest, PutAndGetStringCacheAndReset)
{
    Feature eAnyFeature = Feature::CONFERENCE_FACTORY_URI;
    const AString& strAnyStringValue = "12345";
    objConfigCache.PutCache(eAnyFeature, strAnyStringValue);
    EXPECT_STREQ(strAnyStringValue.GetStr(), objConfigCache.GetStringCache(eAnyFeature).GetStr());

    EXPECT_FALSE(objConfigCache.IsEmpty());
    EXPECT_TRUE(objConfigCache.HasStringCache(eAnyFeature));
    objConfigCache.ResetCache(eAnyFeature);
    EXPECT_TRUE(objConfigCache.IsEmpty());
    EXPECT_FALSE(objConfigCache.HasStringCache(eAnyFeature));
}

}  // namespace android
