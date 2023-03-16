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

#include "Feature.h"

namespace android
{

class FeatureTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(FeatureTest, Constructor)
{
    Feature objFeature("feature-tag", "value");
    EXPECT_EQ(objFeature.GetTag(), AString("feature-tag"));
    EXPECT_EQ(objFeature.GetValue(), AString("value"));
    EXPECT_FALSE(objFeature.IsTagOnly());

    Feature objTagOnlyFeature("feature-tag", AString::ConstNull());
    EXPECT_TRUE(objTagOnlyFeature.IsSameTag(&objFeature));
    EXPECT_FALSE(objTagOnlyFeature.IsSameTag(IMS_NULL));
}

TEST_F(FeatureTest, BaseTag)
{
    AString strBaseTag("video");
    Feature objFeature(Feature::GetFeatureTag(Feature::BASE_VIDEO), AString::ConstNull());
    EXPECT_EQ(objFeature.GetTag(), strBaseTag);
    EXPECT_TRUE(objFeature.GetValue().IsNull());
    EXPECT_TRUE(objFeature.IsTagOnly());
    EXPECT_EQ(objFeature.ToString(), strBaseTag);

    Feature objVideoFeature("tag", "true");
    EXPECT_FALSE(objVideoFeature.IsSameTag(IMS_NULL));

    Feature objVideoFeature2("tag", "true");
    EXPECT_TRUE(objVideoFeature.Equals(&objVideoFeature2));

    Feature objIcsiFeature(
            Feature::OTHER_G_3GPP_ICSI_REF, "urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel");
    EXPECT_EQ(objIcsiFeature.GetTag(), AString(Feature::OTHER_G_3GPP_ICSI_REF));
    EXPECT_EQ(objIcsiFeature.ToString(),
            AString("+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\""));
    EXPECT_EQ(objIcsiFeature.GetValue(), AString("urn:urn-7:3gpp-service.ims.icsi.mmtel"));

    Feature objCopyFeature = objIcsiFeature;
    EXPECT_TRUE(objIcsiFeature.Equals(&objCopyFeature));
    EXPECT_FALSE(objCopyFeature.Equals(&objFeature));
    EXPECT_FALSE(objIcsiFeature.Equals(IMS_NULL));
}

TEST_F(FeatureTest, IsFeatureTag)
{
    EXPECT_TRUE(Feature::IsFeatureTag("audio"));
    EXPECT_FALSE(Feature::IsFeatureTag("sip.tag"));
    EXPECT_FALSE(Feature::IsFeatureTag("sip.instance"));
    EXPECT_FALSE(Feature::IsFeatureTag("+"));
    EXPECT_FALSE(Feature::IsFeatureTag("+126"));
    EXPECT_FALSE(Feature::IsFeatureTag("+g.!**"));
    EXPECT_TRUE(Feature::IsFeatureTag("+g.smsip"));
}

TEST_F(FeatureTest, FeatureSet)
{
    FeatureSet objFeatureSet("feature-tag", "\"value\"");
    EXPECT_EQ(objFeatureSet.ToString(), AString("feature-tag=\"value\""));
    EXPECT_EQ(objFeatureSet.Add(AString::ConstNull()), FeatureSet::OP_FAIL);
    EXPECT_EQ(objFeatureSet.Add("methods"), FeatureSet::OP_FAIL);
    EXPECT_EQ(objFeatureSet.Add("feature-tag", "+gtemp"), FeatureSet::OP_ADD);
    EXPECT_EQ(objFeatureSet.Add("feature-tag", "<string>"), FeatureSet::OP_ADD);
    EXPECT_EQ(objFeatureSet.Add("feature-tag"), FeatureSet::OP_ADD);
    EXPECT_EQ(objFeatureSet.Add("feature-tag"), FeatureSet::OP_ADD_REF);

    EXPECT_TRUE(objFeatureSet.Contains("+gtemp"));
    EXPECT_EQ(objFeatureSet.ToString(), AString("feature-tag=\"value,+gtemp,<string>,\""));

    EXPECT_EQ(objFeatureSet.Remove("feature-tag", "value"), FeatureSet::OP_REMOVE);
    EXPECT_EQ(objFeatureSet.Remove("feature-tag"), FeatureSet::OP_REMOVE_REF);
    EXPECT_EQ(objFeatureSet.Remove("feature-tags"), FeatureSet::OP_FAIL);
    EXPECT_EQ(objFeatureSet.Remove(AString::ConstNull()), FeatureSet::OP_FAIL);
}

}  // namespace android
