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

#include "Sdp.h"
#include "SdpInformation.h"

namespace android
{

class SdpInformationTest : public ::testing::Test
{
public:
    SdpInformationTest();

protected:
    AString m_strTestInformation;
};

SdpInformationTest::SdpInformationTest() :
        m_strTestInformation("A Seminar on the session description protocol")
{
}

TEST_F(SdpInformationTest, Constructor)
{
    SdpInformation objInformation;
    EXPECT_EQ(objInformation.GetValue(), AString::ConstNull());
}

TEST_F(SdpInformationTest, CopyConstructor)
{
    SdpInformation objInformation;
    ASSERT_TRUE(objInformation.Decode(m_strTestInformation));

    SdpInformation objNewInformation(objInformation);
    EXPECT_EQ(objNewInformation.GetValue(), objInformation.GetValue());
}

TEST_F(SdpInformationTest, OperatorAssignment)
{
    SdpInformation objInformation;
    ASSERT_TRUE(objInformation.Decode(m_strTestInformation));

    SdpInformation objNewInformation;
    objNewInformation = objInformation;
    EXPECT_EQ(objNewInformation.GetValue(), objInformation.GetValue());
}

TEST_F(SdpInformationTest, Decode)
{
    SdpInformation objInformation;

    ASSERT_TRUE(objInformation.Decode(m_strTestInformation));
    EXPECT_EQ(objInformation.GetValue(), m_strTestInformation);

    EXPECT_FALSE(objInformation.Decode(AString::ConstNull()));
    EXPECT_FALSE(objInformation.Decode(AString::ConstEmpty()));
    EXPECT_FALSE(objInformation.Decode("test\nsession"));
}

TEST_F(SdpInformationTest, Encode)
{
    SdpInformation objInformation;
    AString strEncoded = objInformation.Encode();
    EXPECT_EQ(strEncoded, AString::ConstNull());

    AString strExpected;
    strExpected.Sprintf("%c=%s\r\n", Sdp::LINE_I, m_strTestInformation.GetStr());
    ASSERT_TRUE(objInformation.Decode(m_strTestInformation));
    strEncoded = objInformation.Encode();
    EXPECT_EQ(strEncoded, strExpected);
}

}  // namespace android
