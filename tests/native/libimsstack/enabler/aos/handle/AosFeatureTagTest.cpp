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
#include <gmock/gmock.h>

#include "handle/AosFeatureTag.h"

class AosFeatureTagTest : public ::testing::Test
{
public:
    AosFeatureTagList m_objAosFeatureTagList;

protected:
    void SetUp() override {}
    void TearDown() override {}

    void AppendFeatureTag(IN AosFeatureTag* objFeatureTag)
    {
        m_objAosFeatureTagList.m_objFeatureTagList.Append(objFeatureTag);
    }
};

TEST_F(AosFeatureTagTest, SetFeatureTag_Test)
{
    // Expectation: The name and value are changed to the new value.

    AosFeatureTag objFeatureTag = AosFeatureTag("+test");
    objFeatureTag.SetFeatureTag("+test2", "AosFeatureTagTest");

    EXPECT_STREQ(objFeatureTag.GetName().GetStr(), "+test2");
    EXPECT_STREQ(objFeatureTag.GetValue().GetStr(), "AosFeatureTagTest");
}

TEST_F(AosFeatureTagTest, Equals1_Test)
{
    // Expectation: Return true if target has same name and value.

    AosFeatureTag objFeatureTag = AosFeatureTag("+test1");
    AosFeatureTag objFeatureTagDiffName = AosFeatureTag("+test2");
    AosFeatureTag objFeatureTagDiffValue = AosFeatureTag("+test1", "AosFeatureTagTest");
    AosFeatureTag objFeatureTagEqual = AosFeatureTag("+test1");

    EXPECT_FALSE(objFeatureTag.Equals(&objFeatureTagDiffName));
    EXPECT_FALSE(objFeatureTag.Equals(&objFeatureTagDiffValue));
    EXPECT_TRUE(objFeatureTag.Equals(&objFeatureTagEqual));
}

TEST_F(AosFeatureTagTest, Equals2_Test)
{
    // Expectation: Return true if both name and value are same.

    AosFeatureTag objFeatureTag = AosFeatureTag("+test1", "AosFeatureTagTest");

    EXPECT_FALSE(objFeatureTag.Equals("+test1"));
    EXPECT_TRUE(objFeatureTag.Equals("+test1", "AosFeatureTagTest"));
    EXPECT_FALSE(objFeatureTag.Equals("+test2"));
}

TEST_F(AosFeatureTagTest, GetName_GetValue_GetType_GetOption_Test)
{
    // Expectation: Return each value as right.

    AosFeatureTag objFeatureTag = AosFeatureTag("+test1", "AosFeatureTagTest");

    EXPECT_STREQ(objFeatureTag.GetName().GetStr(), "+test1");
    EXPECT_STREQ(objFeatureTag.GetValue().GetStr(), "AosFeatureTagTest");
    EXPECT_EQ(objFeatureTag.GetType(), 0);
    EXPECT_EQ(objFeatureTag.GetOption(), AosFeatureTag::OPTION_HEADER_PARAMETER);
}

TEST_F(AosFeatureTagTest, AddFeatureTag_RemoveFeatureTag_Test)
{
    // Expectation: Return true if Add/Remove feature tag properly.

    EXPECT_TRUE(m_objAosFeatureTagList.AddFeatureTag("+test"));
    EXPECT_TRUE(m_objAosFeatureTagList.AddFeatureTag("+test2"));
    EXPECT_TRUE(m_objAosFeatureTagList.HasFeatureTag("+test"));
    EXPECT_TRUE(m_objAosFeatureTagList.HasFeatureTag("+test2"));

    EXPECT_FALSE(m_objAosFeatureTagList.AddFeatureTag("+test2"));

    EXPECT_TRUE(m_objAosFeatureTagList.RemoveFeatureTag("+test2"));
    EXPECT_FALSE(m_objAosFeatureTagList.HasFeatureTag("+test2"));

    EXPECT_FALSE(m_objAosFeatureTagList.RemoveFeatureTag("+test2"));
}

TEST_F(AosFeatureTagTest, AddFeature_RemoveFeature_Test)
{
    // Expectaion: Added/Removed feature properly.

    m_objAosFeatureTagList.AddFeature(ImsAosFeature::MMTEL);
    m_objAosFeatureTagList.AddFeature(ImsAosFeature::VIDEO);
    m_objAosFeatureTagList.AddFeature(ImsAosFeature::TEXT);

    EXPECT_TRUE(m_objAosFeatureTagList.HasFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_objAosFeatureTagList.HasFeature(ImsAosFeature::VIDEO));
    EXPECT_TRUE(m_objAosFeatureTagList.HasFeature(ImsAosFeature::TEXT));

    m_objAosFeatureTagList.RemoveFeature(ImsAosFeature::VIDEO);
    m_objAosFeatureTagList.RemoveFeature(ImsAosFeature::TEXT);

    EXPECT_FALSE(m_objAosFeatureTagList.HasFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_objAosFeatureTagList.HasFeature(ImsAosFeature::TEXT));
}

TEST_F(AosFeatureTagTest, AddUnavailableFeature_RemoveUnavailableFeature_Test)
{
    // Expectation: Added/Removed unavailable feature properly.

    m_objAosFeatureTagList.AddUnavailableFeature(ImsAosFeature::MMTEL);
    m_objAosFeatureTagList.AddUnavailableFeature(ImsAosFeature::VIDEO);
    m_objAosFeatureTagList.AddUnavailableFeature(ImsAosFeature::TEXT);

    EXPECT_TRUE(m_objAosFeatureTagList.HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_objAosFeatureTagList.HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_TRUE(m_objAosFeatureTagList.HasUnavailableFeature(ImsAosFeature::TEXT));

    m_objAosFeatureTagList.RemoveUnavailableFeature(ImsAosFeature::VIDEO);
    m_objAosFeatureTagList.RemoveUnavailableFeature(ImsAosFeature::TEXT);

    EXPECT_FALSE(m_objAosFeatureTagList.HasUnavailableFeature(ImsAosFeature::VIDEO));
    EXPECT_FALSE(m_objAosFeatureTagList.HasUnavailableFeature(ImsAosFeature::TEXT));
}

TEST_F(AosFeatureTagTest, GetFeatures_GetUnavailableFeatures_ClearFeatures_Test)
{
    // Expectation: Return each value as right, No features after clear.

    IMS_UINT32 nFeatures = (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO | ImsAosFeature::TEXT);

    m_objAosFeatureTagList.AddFeature(ImsAosFeature::MMTEL);
    m_objAosFeatureTagList.AddFeature(ImsAosFeature::VIDEO);
    m_objAosFeatureTagList.AddFeature(ImsAosFeature::TEXT);
    EXPECT_EQ(m_objAosFeatureTagList.GetFeatures(), nFeatures);

    m_objAosFeatureTagList.AddUnavailableFeature(ImsAosFeature::MMTEL);
    m_objAosFeatureTagList.AddUnavailableFeature(ImsAosFeature::VIDEO);
    m_objAosFeatureTagList.AddUnavailableFeature(ImsAosFeature::TEXT);
    EXPECT_EQ(m_objAosFeatureTagList.GetUnavailableFeatures(), nFeatures);

    m_objAosFeatureTagList.ClearFeatures();
    EXPECT_EQ(m_objAosFeatureTagList.GetFeatures(), ImsAosFeature::NONE);
    EXPECT_EQ(m_objAosFeatureTagList.GetUnavailableFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosFeatureTagTest, ClearFeatureTags_Test)
{
    // Expectation: Clear feature tag only.

    EXPECT_TRUE(m_objAosFeatureTagList.AddFeatureTag("+test1"));
    EXPECT_TRUE(m_objAosFeatureTagList.AddFeatureTag("+test2"));
    m_objAosFeatureTagList.AddFeature(ImsAosFeature::MMTEL);
    m_objAosFeatureTagList.AddFeature(ImsAosFeature::VIDEO);

    m_objAosFeatureTagList.ClearFeatureTags();
    EXPECT_FALSE(m_objAosFeatureTagList.HasFeatureTag("+test1"));
    EXPECT_FALSE(m_objAosFeatureTagList.HasFeatureTag("+test2"));
    EXPECT_TRUE(m_objAosFeatureTagList.HasFeature(ImsAosFeature::MMTEL));
    EXPECT_TRUE(m_objAosFeatureTagList.HasFeature(ImsAosFeature::VIDEO));
}

TEST_F(AosFeatureTagTest, Clear_Test)
{
    // Expectation: Clear feature tags and features.

    EXPECT_TRUE(m_objAosFeatureTagList.AddFeatureTag("+test1"));
    EXPECT_TRUE(m_objAosFeatureTagList.AddFeatureTag("+test2"));
    m_objAosFeatureTagList.AddFeature(ImsAosFeature::MMTEL);
    m_objAosFeatureTagList.AddFeature(ImsAosFeature::VIDEO);

    m_objAosFeatureTagList.Clear();
    EXPECT_FALSE(m_objAosFeatureTagList.HasFeatureTag("+test1"));
    EXPECT_FALSE(m_objAosFeatureTagList.HasFeatureTag("+test2"));
    EXPECT_FALSE(m_objAosFeatureTagList.HasFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_objAosFeatureTagList.HasFeature(ImsAosFeature::VIDEO));
}

TEST_F(AosFeatureTagTest, Equals3_Test)
{
    // Expectation: Return true if target has same feature tags and features.

    m_objAosFeatureTagList.AddFeature(ImsAosFeature::MMTEL);
    m_objAosFeatureTagList.AddFeature(ImsAosFeature::VIDEO);
    m_objAosFeatureTagList.AddFeatureTag("+test1");
    m_objAosFeatureTagList.AddFeatureTag("+test2");

    AosFeatureTagList objFeatureTagListDiffFeature;
    objFeatureTagListDiffFeature.AddFeature(ImsAosFeature::MMTEL);
    objFeatureTagListDiffFeature.AddFeatureTag("+test1");

    AosFeatureTagList objFeatureTagListDiffSize;
    objFeatureTagListDiffSize.AddFeature(ImsAosFeature::MMTEL);
    objFeatureTagListDiffSize.AddFeature(ImsAosFeature::VIDEO);
    objFeatureTagListDiffSize.AddFeatureTag("+test1");
    objFeatureTagListDiffSize.AddFeatureTag("+test2");
    objFeatureTagListDiffSize.AddFeatureTag("+test3");

    AosFeatureTagList objFeatureTagListDiffFeatureTags;
    objFeatureTagListDiffFeatureTags.AddFeature(ImsAosFeature::MMTEL);
    objFeatureTagListDiffFeatureTags.AddFeature(ImsAosFeature::VIDEO);
    objFeatureTagListDiffFeatureTags.AddFeatureTag("+test1");
    objFeatureTagListDiffFeatureTags.AddFeatureTag("+test3");

    AosFeatureTagList objFeatureTagListEqual;
    objFeatureTagListEqual.AddFeature(ImsAosFeature::MMTEL);
    objFeatureTagListEqual.AddFeature(ImsAosFeature::VIDEO);
    objFeatureTagListEqual.AddFeatureTag("+test1");
    objFeatureTagListEqual.AddFeatureTag("+test2");

    EXPECT_FALSE(m_objAosFeatureTagList.Equals(objFeatureTagListDiffFeature));
    EXPECT_FALSE(m_objAosFeatureTagList.Equals(objFeatureTagListDiffSize));
    EXPECT_FALSE(m_objAosFeatureTagList.Equals(objFeatureTagListDiffFeatureTags));
    EXPECT_TRUE(m_objAosFeatureTagList.Equals(objFeatureTagListEqual));
}

TEST_F(AosFeatureTagTest, Copy_Test)
{
    // Expectation: Target becomes to have same feature tags and features with source.

    AosFeatureTagList objSourceList;
    objSourceList.AddFeature(ImsAosFeature::MMTEL);
    objSourceList.AddFeature(ImsAosFeature::VIDEO);
    objSourceList.AddFeatureTag("+test_source");

    m_objAosFeatureTagList.Copy(objSourceList);

    EXPECT_TRUE(m_objAosFeatureTagList.Equals(objSourceList));
}

TEST_F(AosFeatureTagTest, CopyFeatures_Test)
{
    // Expectation: Target becomes to have same features and unavailable features.

    AosFeatureTagList objSourceList;
    objSourceList.AddFeature(ImsAosFeature::MMTEL);
    objSourceList.AddFeature(ImsAosFeature::VIDEO);

    m_objAosFeatureTagList.CopyFeatures(objSourceList);

    EXPECT_EQ(m_objAosFeatureTagList.GetFeatures(), objSourceList.GetFeatures());
    EXPECT_EQ(m_objAosFeatureTagList.GetUnavailableFeatures(),
            objSourceList.GetUnavailableFeatures());
}

TEST_F(AosFeatureTagTest, CopyFeatureTags_Test)
{
    // Expectation: Target becomes to have same feature tags.

    ImsList<ImsAosFeatureTag*> objFeatureTag;
    objFeatureTag.Append(new ImsAosFeatureTag("+test1", "ImsAosFeatureTag"));
    objFeatureTag.Append(new ImsAosFeatureTag("+test2", "ImsAosFeatureTag2"));

    AosFeatureTagList objExpectedFeatureTagList;
    objExpectedFeatureTagList.AddFeatureTag("+test1", "ImsAosFeatureTag");
    objExpectedFeatureTagList.AddFeatureTag("+test2", "ImsAosFeatureTag2");

    m_objAosFeatureTagList.CopyFeatureTags(objFeatureTag);

    EXPECT_TRUE(m_objAosFeatureTagList.Equals(objExpectedFeatureTagList));
}

TEST_F(AosFeatureTagTest, GetSize_Test)
{
    // Expectation: Return the number of feature tags.

    m_objAosFeatureTagList.AddFeatureTag("+test1");
    EXPECT_EQ(m_objAosFeatureTagList.GetSize(), 1);

    m_objAosFeatureTagList.AddFeatureTag("+test2");
    EXPECT_EQ(m_objAosFeatureTagList.GetSize(), 2);

    m_objAosFeatureTagList.AddFeatureTag("+test3");
    EXPECT_EQ(m_objAosFeatureTagList.GetSize(), 3);
}

TEST_F(AosFeatureTagTest, GetAt_Test)
{
    // Expectation: Return AosFeatureTag at the index

    AosFeatureTag* pFeatureTag1 = new AosFeatureTag("name1", "value1");
    AosFeatureTag* pFeatureTag2 = new AosFeatureTag("name2", "value2");
    AosFeatureTag* pFeatureTag3 = new AosFeatureTag("name3", "value3");

    AppendFeatureTag(pFeatureTag1);
    AppendFeatureTag(pFeatureTag2);
    AppendFeatureTag(pFeatureTag3);

    EXPECT_EQ(m_objAosFeatureTagList.GetAt(0), pFeatureTag1);
    EXPECT_EQ(m_objAosFeatureTagList.GetAt(1), pFeatureTag2);
    EXPECT_EQ(m_objAosFeatureTagList.GetAt(2), pFeatureTag3);
}

TEST_F(AosFeatureTagTest, HasUnavailableFeature_Test)
{
    // Expectation: Return true if the feature is existed

    m_objAosFeatureTagList.AddUnavailableFeature(ImsAosFeature::MMTEL);

    EXPECT_TRUE(m_objAosFeatureTagList.HasUnavailableFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_objAosFeatureTagList.HasUnavailableFeature(ImsAosFeature::VIDEO));
}

TEST_F(AosFeatureTagTest, HasFeature_Test)
{
    // Expectation: Return true if the feature is existed

    m_objAosFeatureTagList.AddFeature(ImsAosFeature::MMTEL);

    EXPECT_TRUE(m_objAosFeatureTagList.HasFeature(ImsAosFeature::MMTEL));
    EXPECT_FALSE(m_objAosFeatureTagList.HasFeature(ImsAosFeature::VIDEO));
}

TEST_F(AosFeatureTagTest, HasFeatureTag_Test)
{
    // Expectation: Return true if the feature tag that has same name and value is existed

    m_objAosFeatureTagList.AddFeatureTag("+test1");
    m_objAosFeatureTagList.AddFeatureTag("+test2", "AosFeatureTagTest");

    EXPECT_TRUE(m_objAosFeatureTagList.HasFeatureTag("+test1"));
    EXPECT_TRUE(m_objAosFeatureTagList.HasFeatureTag("+test2", "AosFeatureTagTest"));
    EXPECT_FALSE(m_objAosFeatureTagList.HasFeatureTag("+test2"));
    EXPECT_FALSE(m_objAosFeatureTagList.HasFeatureTag("+test3"));
}

TEST_F(AosFeatureTagTest, PrintFeatureTagList_Test)
{
    m_objAosFeatureTagList.PrintFeatureTagList();
}