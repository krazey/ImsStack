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

#include "ImsVector.h"

#include "SdpBandwidth.h"

namespace android
{

class TestBandwidth
{
public:
    inline TestBandwidth() :
            m_strSdpFullLine(AString::ConstNull()),
            m_strSdpLine(AString::ConstNull()),
            m_nBwType(SdpBandwidth::TYPE_OTHER),
            m_strBwTypeName(AString::ConstNull()),
            m_nBwValue(0)
    {
    }
    inline TestBandwidth(const AString& strSdpFullLine, const AString& strSdpLine,
            IMS_SINT32 nBwType, const AString& strBwTypeName, IMS_SINT32 nBwValue) :
            m_strSdpFullLine(strSdpFullLine),
            m_strSdpLine(strSdpLine),
            m_nBwType(nBwType),
            m_strBwTypeName(strBwTypeName),
            m_nBwValue(nBwValue)
    {
    }
    inline TestBandwidth(const TestBandwidth& other) :
            m_strSdpFullLine(other.m_strSdpFullLine),
            m_strSdpLine(other.m_strSdpLine),
            m_nBwType(other.m_nBwType),
            m_strBwTypeName(other.m_strBwTypeName),
            m_nBwValue(other.m_nBwValue)
    {
    }
    inline ~TestBandwidth() {}

    inline TestBandwidth& operator=(const TestBandwidth& other)
    {
        if (this != &other)
        {
            m_strSdpFullLine = other.m_strSdpFullLine;
            m_strSdpLine = other.m_strSdpLine;
            m_nBwType = other.m_nBwType;
            m_strBwTypeName = other.m_strBwTypeName;
            m_nBwValue = other.m_nBwValue;
        }

        return *this;
    }

public:
    AString m_strSdpFullLine;  // including "b="
    AString m_strSdpLine;      // excluding "b="
    IMS_SINT32 m_nBwType;      // bandwidth-name as enum
    AString m_strBwTypeName;   // bandwidth-name as string
    IMS_SINT32 m_nBwValue;     // bandwidth-value
};

class SdpBandwidthTest : public ::testing::Test
{
public:
    SdpBandwidthTest();

protected:
    TestBandwidth m_objTestBw;
    ImsVector<TestBandwidth> m_objNormalTestBandwidths;
    ImsVector<TestBandwidth> m_objAbnormalTestBandwidths;
    ImsVector<TestBandwidth> m_objNotEqualsTestBandwidths1;
    ImsVector<TestBandwidth> m_objNotEqualsTestBandwidths2;
};

SdpBandwidthTest::SdpBandwidthTest() :
        m_objTestBw(TestBandwidth(
                "b=AS:100\r\n", "AS:100", SdpBandwidth::TYPE_AS, SdpBandwidth::TOKEN_AS, 100))
{
    // clang-format off
    m_objNormalTestBandwidths.Add(TestBandwidth(
            "b=AS:100\r\n", "AS:100", SdpBandwidth::TYPE_AS, SdpBandwidth::TOKEN_AS, 100));
    m_objNormalTestBandwidths.Add(TestBandwidth(
            "b=CT:101\r\n", "CT:101", SdpBandwidth::TYPE_CT, SdpBandwidth::TOKEN_CT, 101));
    m_objNormalTestBandwidths.Add(TestBandwidth(
            "b=RR:102\r\n", "RR:102", SdpBandwidth::TYPE_RR, SdpBandwidth::TOKEN_RR, 102));
    m_objNormalTestBandwidths.Add(TestBandwidth(
            "b=RS:103\r\n", "RS:103", SdpBandwidth::TYPE_RS, SdpBandwidth::TOKEN_RS, 103));
    m_objNormalTestBandwidths.Add(TestBandwidth(
            "b=TIAS:104\r\n", "TIAS:104", SdpBandwidth::TYPE_TIAS, SdpBandwidth::TOKEN_TIAS, 104));
    m_objNormalTestBandwidths.Add(TestBandwidth(
            "b=BW:105\r\n", "BW:105", SdpBandwidth::TYPE_OTHER, "BW", 105));

    m_objAbnormalTestBandwidths.Add(TestBandwidth(
            "b=AS\r\n", "AS", SdpBandwidth::TYPE_AS, SdpBandwidth::TOKEN_AS,
            SdpBandwidth::INVALID_BANDWIDTH));
    m_objAbnormalTestBandwidths.Add(TestBandwidth(
            "b=TI AS:100\r\n", "TI AS:100", SdpBandwidth::TYPE_OTHER, "TI AS", 100));
    m_objAbnormalTestBandwidths.Add(TestBandwidth(
            "b=AS:\r\n", "AS:", SdpBandwidth::TYPE_AS, SdpBandwidth::TOKEN_AS,
            SdpBandwidth::INVALID_BANDWIDTH));
    m_objAbnormalTestBandwidths.Add(TestBandwidth(
            "b=AS:10a\r\n", "AS:10a", SdpBandwidth::TYPE_AS, SdpBandwidth::TOKEN_AS,
            SdpBandwidth::INVALID_BANDWIDTH));

    m_objNotEqualsTestBandwidths1.Add(TestBandwidth(
            "b=AS:100\r\n", "AS:100", SdpBandwidth::TYPE_AS, SdpBandwidth::TOKEN_AS, 100));
    m_objNotEqualsTestBandwidths1.Add(TestBandwidth(
            "b=BW:102\r\n", "BW:102", SdpBandwidth::TYPE_OTHER, "BW", 102));

    m_objNotEqualsTestBandwidths2.Add(TestBandwidth(
            "b=RR:101\r\n", "RR:101", SdpBandwidth::TYPE_RR, SdpBandwidth::TOKEN_RR, 101));
    m_objNotEqualsTestBandwidths2.Add(TestBandwidth(
            "b=RS:102\r\n", "RS:102", SdpBandwidth::TYPE_RS, SdpBandwidth::TOKEN_RS, 102));
    // clang-format on
}

TEST_F(SdpBandwidthTest, Constructor)
{
    SdpBandwidth objBw;
    EXPECT_EQ(objBw.GetType(), SdpBandwidth::TYPE_OTHER);
    EXPECT_EQ(objBw.GetTypeName(), AString::ConstNull());
    EXPECT_EQ(objBw.GetBandwidth(), SdpBandwidth::INVALID_BANDWIDTH);
}

TEST_F(SdpBandwidthTest, CopyConstructor)
{
    SdpBandwidth objBw;
    ASSERT_TRUE(objBw.SetValue(
            m_objTestBw.m_nBwType, m_objTestBw.m_nBwValue, m_objTestBw.m_strBwTypeName));

    SdpBandwidth objNewBw(objBw);
    EXPECT_TRUE(objNewBw.Equals(&objBw));
}

TEST_F(SdpBandwidthTest, OperatorAssignment)
{
    SdpBandwidth objBw;
    ASSERT_TRUE(objBw.SetValue(
            m_objTestBw.m_nBwType, m_objTestBw.m_nBwValue, m_objTestBw.m_strBwTypeName));

    SdpBandwidth objNewBw;
    objNewBw = objBw;
    EXPECT_TRUE(objNewBw.Equals(&objBw));
}

TEST_F(SdpBandwidthTest, Decode)
{
    SdpBandwidth objBw;
    IMS_UINT32 nTestCount = m_objNormalTestBandwidths.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestBandwidth& objTestBw = m_objNormalTestBandwidths.GetAt(i);

        ASSERT_TRUE(objBw.Decode(objTestBw.m_strSdpLine));
        EXPECT_EQ(objBw.GetType(), objTestBw.m_nBwType);
        EXPECT_EQ(objBw.GetTypeName(), objTestBw.m_strBwTypeName);
        EXPECT_EQ(objBw.GetBandwidth(), objTestBw.m_nBwValue);
    }

    nTestCount = m_objAbnormalTestBandwidths.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestBandwidth& objTestBw = m_objAbnormalTestBandwidths.GetAt(i);

        EXPECT_FALSE(objBw.Decode(objTestBw.m_strSdpLine));
    }

    EXPECT_FALSE(objBw.Decode(AString::ConstNull()));
    EXPECT_FALSE(objBw.Decode(AString::ConstEmpty()));
}

TEST_F(SdpBandwidthTest, Encode)
{
    SdpBandwidth objBw;
    AString strEncoded = objBw.Encode();
    EXPECT_EQ(strEncoded, AString::ConstNull());

    IMS_UINT32 nTestCount = m_objNormalTestBandwidths.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestBandwidth& objTestBw = m_objNormalTestBandwidths.GetAt(i);

        ASSERT_TRUE(objBw.SetValue(
                objTestBw.m_nBwType, objTestBw.m_nBwValue, objTestBw.m_strBwTypeName));
        strEncoded = objBw.Encode();
        EXPECT_EQ(objTestBw.m_strSdpFullLine, strEncoded);
    }
}

TEST_F(SdpBandwidthTest, Equals)
{
    SdpBandwidth objBw1;
    SdpBandwidth objBw2;
    IMS_UINT32 nTestCount = m_objNormalTestBandwidths.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestBandwidth& objTestBw = m_objNormalTestBandwidths.GetAt(i);

        ASSERT_TRUE(objBw1.Decode(objTestBw.m_strSdpLine));
        ASSERT_TRUE(objBw2.Decode(objTestBw.m_strSdpLine));
        EXPECT_TRUE(objBw1.Equals(&objBw2));

        SdpBandwidth objBw3(objBw1);
        SdpBandwidth objBw4 = objBw2;
        EXPECT_TRUE(objBw3.Equals(&objBw4));
    }

    ASSERT_TRUE(objBw1.Decode(m_objTestBw.m_strSdpLine));
    ASSERT_TRUE(objBw2.SetValue(
            m_objTestBw.m_nBwType, m_objTestBw.m_nBwValue, m_objTestBw.m_strBwTypeName));
    EXPECT_TRUE(objBw1.Equals(&objBw2));

    ASSERT_EQ(m_objNotEqualsTestBandwidths1.GetSize(), m_objNotEqualsTestBandwidths2.GetSize());
    nTestCount = m_objNotEqualsTestBandwidths1.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestBandwidth& objTestBw1 = m_objNotEqualsTestBandwidths1.GetAt(i);
        const TestBandwidth& objTestBw2 = m_objNotEqualsTestBandwidths2.GetAt(i);

        ASSERT_TRUE(objBw1.Decode(objTestBw1.m_strSdpLine));
        ASSERT_TRUE(objBw2.Decode(objTestBw2.m_strSdpLine));
        EXPECT_FALSE(objBw1.Equals(&objBw2));

        SdpBandwidth objBw3(objBw1);
        SdpBandwidth objBw4 = objBw2;
        EXPECT_FALSE(objBw3.Equals(&objBw4));
    }
}

TEST_F(SdpBandwidthTest, SetValue)
{
    SdpBandwidth objBw;
    IMS_UINT32 nTestCount = m_objNormalTestBandwidths.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestBandwidth& objTestBw = m_objNormalTestBandwidths.GetAt(i);

        ASSERT_TRUE(objBw.SetValue(
                objTestBw.m_nBwType, objTestBw.m_nBwValue, objTestBw.m_strBwTypeName));
        EXPECT_EQ(objBw.GetType(), objTestBw.m_nBwType);
        EXPECT_EQ(objBw.GetTypeName(), objTestBw.m_strBwTypeName);
        EXPECT_EQ(objBw.GetBandwidth(), objTestBw.m_nBwValue);
    }

    nTestCount = m_objAbnormalTestBandwidths.GetSize();

    for (IMS_UINT32 i = 0; i < nTestCount; ++i)
    {
        const TestBandwidth& objTestBw = m_objAbnormalTestBandwidths.GetAt(i);

        ASSERT_FALSE(objBw.SetValue(
                objTestBw.m_nBwType, objTestBw.m_nBwValue, objTestBw.m_strBwTypeName));
    }
}

}  // namespace android
