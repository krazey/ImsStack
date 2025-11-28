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
#include "SdpTime.h"

namespace android
{

class SdpTimeTest : public ::testing::Test
{
public:
    SdpTimeTest();

protected:
    AString m_strDefaultTime;
    AString m_strTestTime;
    IMS_UINT32 m_nTestStartTime;
    IMS_UINT32 m_nTestStopTime;
};

SdpTimeTest::SdpTimeTest() :
        m_strDefaultTime("0 0"),
        m_strTestTime("2873397496 2873404696"),
        m_nTestStartTime(2873397496),
        m_nTestStopTime(2873404696)
{
}

TEST_F(SdpTimeTest, Constructor)
{
    SdpTime objTime;
    EXPECT_EQ(objTime.GetStartTime(), 0);
    EXPECT_EQ(objTime.GetStopTime(), 0);
}

TEST_F(SdpTimeTest, CopyConstructor)
{
    SdpTime objTime;
    ASSERT_TRUE(objTime.SetValue(m_nTestStartTime, m_nTestStopTime));

    SdpTime objNewTime(objTime);
    EXPECT_EQ(objNewTime.GetStartTime(), objTime.GetStartTime());
    EXPECT_EQ(objNewTime.GetStopTime(), objTime.GetStopTime());
}

TEST_F(SdpTimeTest, OperatorAssignment)
{
    SdpTime objTime;
    ASSERT_TRUE(objTime.SetValue(m_nTestStartTime, m_nTestStopTime));

    SdpTime objNewTime;
    objNewTime = objTime;
    EXPECT_EQ(objNewTime.GetStartTime(), m_nTestStartTime);
    EXPECT_EQ(objNewTime.GetStopTime(), m_nTestStopTime);
}

TEST_F(SdpTimeTest, Decode)
{
    SdpTime objTime;

    ASSERT_TRUE(objTime.Decode(m_strTestTime));
    EXPECT_EQ(objTime.GetStartTime(), m_nTestStartTime);
    EXPECT_EQ(objTime.GetStopTime(), m_nTestStopTime);

    EXPECT_FALSE(objTime.Decode(AString::ConstNull()));
    EXPECT_FALSE(objTime.Decode(AString::ConstEmpty()));
    EXPECT_FALSE(objTime.Decode("00"));
    EXPECT_FALSE(objTime.Decode("100a 3600"));
    EXPECT_FALSE(objTime.Decode("100 3600a"));
}

TEST_F(SdpTimeTest, Encode)
{
    SdpTime objTime;
    AString strExpected;
    AString strEncoded;

    strExpected.Sprintf("%c=%s\r\n", Sdp::LINE_T, m_strDefaultTime.GetStr());
    strEncoded = objTime.Encode();
    EXPECT_EQ(strEncoded, strExpected);

    strExpected.Sprintf("%c=%u %u\r\n", Sdp::LINE_T, m_nTestStartTime, m_nTestStopTime);
    ASSERT_TRUE(objTime.SetValue(m_nTestStartTime, m_nTestStopTime));
    strEncoded = objTime.Encode();
    EXPECT_EQ(strEncoded, strExpected);
}

TEST_F(SdpTimeTest, SetValue)
{
    SdpTime objTime;

    ASSERT_TRUE(objTime.SetValue(m_nTestStartTime, m_nTestStopTime));
    EXPECT_EQ(objTime.GetValue(), m_strTestTime);
    EXPECT_EQ(objTime.GetStartTime(), m_nTestStartTime);
    EXPECT_EQ(objTime.GetStopTime(), m_nTestStopTime);
}

}  // namespace android
