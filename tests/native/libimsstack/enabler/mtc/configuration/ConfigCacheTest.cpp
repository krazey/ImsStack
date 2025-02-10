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

#include "ImsTypeDef.h"
#include "configuration/ConfigCache.h"
#include "configuration/ConfigDef.h"
#include <gtest/gtest.h>
#include <variant>

TEST(ConfigCacheTest, GetCacheReturnsNull)
{
    ConfigCache objCache;

    const IMS_CHAR* pszAnyKey = "any_key_int";
    auto result = objCache.GetCache(pszAnyKey);
    EXPECT_FALSE(result.has_value());
}

TEST(ConfigCacheTest, PutAndGetCacheReturnsCachingValue)
{
    ConfigCache objCache;

    const IMS_CHAR* pszAnyKey = "any_key_int";
    IMS_SINT32 nValue = 1000;
    objCache.PutCache(pszAnyKey, nValue);

    auto result = objCache.GetCache(pszAnyKey);
    EXPECT_TRUE(result.has_value());

    EXPECT_EQ(std::get<IMS_SINT32>(result.value()), nValue);
}
