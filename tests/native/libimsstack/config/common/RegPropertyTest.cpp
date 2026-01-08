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

#include "RegProperty.h"

namespace android
{
static const AString SERVICE_ID("ims.app.test");
static const AString HEADER_1("Supported: path");
static const AString HEADER_2("Supported: eventlist");

class RegPropertyTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(RegPropertyTest, CopyConstructor)
{
    RegProperty objRegProperty(SERVICE_ID);

    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    RegProperty objCopiedRegProperty(objRegProperty);

    const ImsProperty* pImsProperty = &objCopiedRegProperty;

    EXPECT_TRUE(pImsProperty->Equals(objRegProperty));
}

TEST_F(RegPropertyTest, AssignmentOperator)
{
    RegProperty objRegProperty(SERVICE_ID);
    RegProperty objAssignedRegProperty(AString("ServiceId"));

    objAssignedRegProperty = objRegProperty;

    const ImsProperty* pImsProperty = &objAssignedRegProperty;

    EXPECT_TRUE(pImsProperty->Equals(objRegProperty));
}

TEST_F(RegPropertyTest, AddValueAndGetValues)
{
    RegProperty objRegProperty(SERVICE_ID);

    EXPECT_TRUE(objRegProperty.GetValues().GetSize() == 0);

    EXPECT_TRUE(objRegProperty.AddValue(HEADER_1));

    EXPECT_TRUE(objRegProperty.GetValues().GetSize() == 1);

    EXPECT_TRUE(objRegProperty.AddValue(HEADER_2));

    EXPECT_TRUE(objRegProperty.GetValues().GetSize() == 2);
}

}  // namespace android
