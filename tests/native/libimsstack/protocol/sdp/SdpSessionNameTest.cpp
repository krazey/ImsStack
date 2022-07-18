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

#include "TextParser.h"

#include "Sdp.h"
#include "SdpSessionName.h"

namespace android
{

class SdpSessionNameTest : public ::testing::Test
{
public:
    SdpSessionNameTest();

protected:
    AString m_strDefaultSessionName;
    AString m_strTestSessionName;
};

SdpSessionNameTest::SdpSessionNameTest() :
        m_strDefaultSessionName(&TextParser::CHAR_HYPHEN, 1),
        m_strTestSessionName("SDP Seminar")
{
}

TEST_F(SdpSessionNameTest, Constructor)
{
    SdpSessionName objSessionName;
    EXPECT_EQ(objSessionName.GetValue(), m_strDefaultSessionName);
}

TEST_F(SdpSessionNameTest, CopyConstructor)
{
    SdpSessionName objSessionName;
    ASSERT_TRUE(objSessionName.Decode(m_strTestSessionName));

    SdpSessionName objNewSessionName(objSessionName);
    EXPECT_EQ(objNewSessionName.GetValue(), objSessionName.GetValue());
}

TEST_F(SdpSessionNameTest, OperatorAssignment)
{
    SdpSessionName objSessionName;
    ASSERT_TRUE(objSessionName.Decode(m_strTestSessionName));

    SdpSessionName objNewSessionName;
    objNewSessionName = objSessionName;
    EXPECT_EQ(objNewSessionName.GetValue(), objSessionName.GetValue());
}

TEST_F(SdpSessionNameTest, Decode)
{
    SdpSessionName objSessionName;

    ASSERT_TRUE(objSessionName.Decode(m_strDefaultSessionName));
    EXPECT_EQ(objSessionName.GetValue(), m_strDefaultSessionName);

    ASSERT_TRUE(objSessionName.Decode(m_strTestSessionName));
    EXPECT_EQ(objSessionName.GetValue(), m_strTestSessionName);

    EXPECT_FALSE(objSessionName.Decode(AString::ConstNull()));
    EXPECT_FALSE(objSessionName.Decode(AString::ConstEmpty()));
    EXPECT_FALSE(objSessionName.Decode("SDP\r Seminar"));
}

TEST_F(SdpSessionNameTest, Encode)
{
    SdpSessionName objSessionName;
    AString strExpected;
    AString strEncoded;

    strExpected.Sprintf("%c=%s\r\n", Sdp::LINE_S, m_strDefaultSessionName.GetStr());
    strEncoded = objSessionName.Encode();
    EXPECT_EQ(strEncoded, strExpected);

    strExpected.Sprintf("%c=%s\r\n", Sdp::LINE_S, m_strTestSessionName.GetStr());
    ASSERT_TRUE(objSessionName.Decode(m_strTestSessionName));
    strEncoded = objSessionName.Encode();
    EXPECT_EQ(strEncoded, strExpected);
}

}  // namespace android
