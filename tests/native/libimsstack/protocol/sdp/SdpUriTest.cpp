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

#include "AString.h"

#include "Sdp.h"
#include "SdpUri.h"

namespace android
{

class SdpUriTest : public ::testing::Test
{
public:
    SdpUriTest();

protected:
    AString m_strTestUri;
};

SdpUriTest::SdpUriTest() :
        m_strTestUri("http://www.example.com/seminars/sdp.pdf")
{
}

TEST_F(SdpUriTest, Constructor)
{
    SdpUri objUri;
    EXPECT_EQ(objUri.GetValue(), AString::ConstNull());
}

TEST_F(SdpUriTest, CopyConstructor)
{
    SdpUri objUri;
    ASSERT_TRUE(objUri.Decode(m_strTestUri));

    SdpUri objNewUri(objUri);
    EXPECT_EQ(objNewUri.GetValue(), objUri.GetValue());
}

TEST_F(SdpUriTest, OperatorAssignment)
{
    SdpUri objUri;
    ASSERT_TRUE(objUri.Decode(m_strTestUri));

    SdpUri objNewUri;
    objNewUri = objUri;
    // cppcheck-suppress knownConditionTrueFalse
    EXPECT_EQ(objNewUri.GetValue(), objUri.GetValue());
}

TEST_F(SdpUriTest, Decode)
{
    SdpUri objUri;

    ASSERT_TRUE(objUri.Decode(m_strTestUri));
    EXPECT_EQ(objUri.GetValue(), m_strTestUri);

    EXPECT_FALSE(objUri.Decode(AString::ConstNull()));
    EXPECT_FALSE(objUri.Decode(AString::ConstEmpty()));
}

TEST_F(SdpUriTest, Encode)
{
    SdpUri objUri;
    AString strEncoded = objUri.Encode();
    EXPECT_EQ(strEncoded, AString::ConstNull());

    AString strExpected;
    strExpected.Sprintf("%c=%s\r\n", Sdp::LINE_U, m_strTestUri.GetStr());
    ASSERT_TRUE(objUri.Decode(m_strTestUri));
    strEncoded = objUri.Encode();
    EXPECT_EQ(strEncoded, strExpected);
}

}  // namespace android
