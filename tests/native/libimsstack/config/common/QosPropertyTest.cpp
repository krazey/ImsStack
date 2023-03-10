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

#include "QosProperty.h"

namespace android
{
static const AString CONTENT_TYPE("Content-Type");

class QosPropertyTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(QosPropertyTest, CopyConstructor)
{
    QosProperty objDefaultQosProperty;
    QosProperty objQosProperty(CONTENT_TYPE);

    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    QosProperty objCopiedQosProperty(objQosProperty);

    EXPECT_EQ(objDefaultQosProperty.GetContentType(), AString::ConstNull());
    EXPECT_EQ(objCopiedQosProperty.GetContentType(), CONTENT_TYPE);
}

TEST_F(QosPropertyTest, AssignmentOperator)
{
    QosProperty objDefaultQosProperty;
    QosProperty objQosProperty(CONTENT_TYPE);
    QosProperty objAssignedQosProperty;

    objAssignedQosProperty = objQosProperty;

    EXPECT_EQ(objDefaultQosProperty.GetContentType(), AString::ConstNull());
    EXPECT_EQ(objAssignedQosProperty.GetContentType(), CONTENT_TYPE);
}

TEST_F(QosPropertyTest, SetQosAndGetQosString)
{
    QosProperty objQosProperty;

    EXPECT_EQ(objQosProperty.GetQosString(), AString("0 0 0 0 0 0 0"));

    // Not all values of Qos passed, fail
    EXPECT_FALSE(objQosProperty.SetQos(AString("1")));

    // Passed all 7 values of Qos but invalid values, fail
    EXPECT_FALSE(objQosProperty.SetQos(AString("1 2 3 4 5 6 invalid")));

    AString strValidQos("1 2 3 4 5 6 7");

    // Passed all 7 valid values of Qos, success
    EXPECT_TRUE(objQosProperty.SetQos(strValidQos));

    EXPECT_EQ(objQosProperty.GetQosString(), strValidQos);

    QosProperty::QualityOfService objQualityOfService = objQosProperty.GetQos();

    EXPECT_EQ(objQualityOfService.nAverageRate, 1);
    EXPECT_EQ(objQualityOfService.nBufferSize, 2);
    EXPECT_EQ(objQualityOfService.nPeakRate, 3);
    EXPECT_EQ(objQualityOfService.nDelay, 4);
    EXPECT_EQ(objQualityOfService.nDelayVariance, 5);
    EXPECT_EQ(objQualityOfService.nMaxChunkSize, 6);
    EXPECT_EQ(objQualityOfService.nMinimalPolicedSize, 7);
}

}  // namespace android
