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

#include "private/ImsProperty.h"

namespace android
{
static const IMS_SINT32 PROP_KEY = ImsProperty::PKEY_STREAM;

static const AString PROP_VALUE1("audio video text Video");
static const AString PROP_VALUE2(" Type1 Type2");

class ImsPropertyTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(ImsPropertyTest, CopyConstructor)
{
    ImsProperty objImsProperty(PROP_KEY);

    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    ImsProperty objCopiedImsProperty(objImsProperty);

    EXPECT_TRUE(objCopiedImsProperty.Equals(objImsProperty));
    EXPECT_TRUE(objCopiedImsProperty.Equals(ImsProperty::PKEY_STRING[PROP_KEY]));
}

TEST_F(ImsPropertyTest, AssignmentOperator)
{
    ImsProperty objImsProperty(PROP_KEY);
    ImsProperty objAssignedImsProperty(ImsProperty::PKEY_FRAMED);

    EXPECT_FALSE(objAssignedImsProperty.Equals(objImsProperty));

    objAssignedImsProperty = objImsProperty;

    EXPECT_TRUE(objAssignedImsProperty.Equals(objImsProperty));
    EXPECT_TRUE(objAssignedImsProperty.Equals(ImsProperty::PKEY_STRING[PROP_KEY]));
}

TEST_F(ImsPropertyTest, Decode)
{
    EXPECT_TRUE(ImsProperty::Decode(AString()).GetCount() == 0);

    EXPECT_TRUE(ImsProperty::Decode(PROP_VALUE1).GetCount() == 4);

    AStringArray objPropertyValues = ImsProperty::Decode(PROP_VALUE2);

    EXPECT_TRUE(objPropertyValues.GetCount() == 2);
    EXPECT_EQ(objPropertyValues.GetElementAt(0), AString("Type1"));
    EXPECT_EQ(objPropertyValues.GetElementAt(1), AString("Type2"));
}

TEST_F(ImsPropertyTest, Encode)
{
    AStringArray objPropertyValues;

    objPropertyValues.AddElement("audio");
    objPropertyValues.AddElement("video");
    objPropertyValues.AddElement("text");
    objPropertyValues.AddElement("Video");

    EXPECT_EQ(ImsProperty::Encode(objPropertyValues), PROP_VALUE1);
}

TEST_F(ImsPropertyTest, CheckDuplicate)
{
    AStringArray objPropertyValues;

    objPropertyValues.AddElement("audio");
    objPropertyValues.AddElement("video");
    objPropertyValues.AddElement("text");
    objPropertyValues.AddElement("Video");

    EXPECT_FALSE(ImsProperty::CheckDuplicate(objPropertyValues, IMS_FALSE));
    EXPECT_TRUE(ImsProperty::CheckDuplicate(objPropertyValues, IMS_TRUE));

    objPropertyValues.AddElement("audio");

    EXPECT_FALSE(ImsProperty::CheckDuplicate(objPropertyValues, IMS_TRUE));
}

TEST_F(ImsPropertyTest, KeyToString)
{
    EXPECT_EQ(ImsProperty::KeyToString(PROP_KEY), AString(ImsProperty::PKEY_STRING[PROP_KEY]));
    EXPECT_EQ(ImsProperty::KeyToString(ImsProperty::PKEY_MAX), AString::ConstNull());
}

TEST_F(ImsPropertyTest, StringToKey)
{
    EXPECT_EQ(ImsProperty::StringToKey(AString(ImsProperty::PKEY_STRING[PROP_KEY])), PROP_KEY);
    EXPECT_EQ(ImsProperty::StringToKey(AString::ConstNull()), ImsProperty::PKEY_MAX);
    EXPECT_EQ(ImsProperty::StringToKey(AString("CustomKey")), ImsProperty::PKEY_CUSTOM);
}

TEST_F(ImsPropertyTest, TrimAndCheckProperties)
{
    ImsRegistry objRegistry;
    ImsRegistry objNewRegistry;

    AStringArray objPropertyValues;

    objPropertyValues.AddElement("Stream");
    objPropertyValues.AddElement(" audio");
    objPropertyValues.AddElement("  video  ");

    objRegistry.Add(objPropertyValues);

    EXPECT_TRUE(ImsProperty::TrimAndCheckProperties(objRegistry, objNewRegistry));

    ASSERT_TRUE(objNewRegistry.GetCount() == 1);

    const AStringArray& objTrimmedProperties = objNewRegistry.GetAt(0);

    EXPECT_EQ(objTrimmedProperties.GetElementAt(0), AString("Stream"));
    EXPECT_EQ(objTrimmedProperties.GetElementAt(1), AString("audio"));
    EXPECT_EQ(objTrimmedProperties.GetElementAt(2), AString("video"));

    // Adding duplicate property, fail
    objRegistry.Add(objPropertyValues);

    EXPECT_FALSE(ImsProperty::TrimAndCheckProperties(objRegistry, objNewRegistry));

    // Custom property, fail
    ImsRegistry objCustomRegistry;

    AStringArray objCustomValues;

    objCustomValues.AddElement("Custom");
    objCustomValues.AddElement("test");

    objCustomRegistry.Add(objCustomValues);

    EXPECT_FALSE(ImsProperty::TrimAndCheckProperties(objCustomRegistry, objNewRegistry));

    // Empty property, fail
    ImsRegistry objEmptyRegistry;

    AStringArray objEmptyValues;

    objEmptyRegistry.Add(objEmptyValues);

    EXPECT_FALSE(ImsProperty::TrimAndCheckProperties(objEmptyRegistry, objNewRegistry));
}

}  // namespace android