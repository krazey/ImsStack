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

#include "SdpRepeatTime.h"

namespace android
{

class TestRepeatTime
{
public:
    inline TestRepeatTime() :
            m_strSdpFullLine(AString::ConstNull()),
            m_strSdpLine(AString::ConstNull()),
            m_nInterval(0),
            m_nActiveDuration(0),
            m_nFirstOffset(0)
    {
    }
    inline TestRepeatTime(const AString& strSdpFullLine, const AString& strSdpLine,
            IMS_UINT32 nInterval, IMS_UINT32 nActiveDuration, IMS_UINT32 nFirstOffset,
            const ImsList<IMS_UINT32>& objAdditionalOffsets) :
            m_strSdpFullLine(strSdpFullLine),
            m_strSdpLine(strSdpLine),
            m_nInterval(nInterval),
            m_nActiveDuration(nActiveDuration),
            m_nFirstOffset(nFirstOffset),
            m_objAdditionalOffsets(objAdditionalOffsets)
    {
    }
    inline TestRepeatTime(const TestRepeatTime& other) :
            m_strSdpFullLine(other.m_strSdpFullLine),
            m_strSdpLine(other.m_strSdpLine),
            m_nInterval(other.m_nInterval),
            m_nActiveDuration(other.m_nActiveDuration),
            m_nFirstOffset(other.m_nFirstOffset),
            m_objAdditionalOffsets(other.m_objAdditionalOffsets)
    {
    }
    inline ~TestRepeatTime() {}

    inline TestRepeatTime& operator=(const TestRepeatTime& other)
    {
        if (this != &other)
        {
            m_strSdpFullLine = other.m_strSdpFullLine;
            m_strSdpLine = other.m_strSdpLine;
            m_nInterval = other.m_nInterval;
            m_nActiveDuration = other.m_nActiveDuration;
            m_nFirstOffset = other.m_nFirstOffset;
            m_objAdditionalOffsets = other.m_objAdditionalOffsets;
        }

        return *this;
    }

public:
    AString m_strSdpFullLine;
    AString m_strSdpLine;
    IMS_UINT32 m_nInterval;
    IMS_UINT32 m_nActiveDuration;
    IMS_UINT32 m_nFirstOffset;
    ImsList<IMS_UINT32> m_objAdditionalOffsets;
};

class SdpRepeatTimeTest : public ::testing::Test
{
public:
    SdpRepeatTimeTest();

protected:
    TestRepeatTime m_objTestRepeatTime;
    TestRepeatTime m_objTestRepeatTimeCompact;
    TestRepeatTime m_objTestRepeatTimeOneOffset;
    TestRepeatTime m_objTestRepeatTimeExtraOffsets;
};

SdpRepeatTimeTest::SdpRepeatTimeTest()
{
    // clang-format off
    // If a session is active at 10am on Monday and 11am on Tuesday for one hour
    // each week for three months, then the <start-time> in the corresponding "t=" field
    // would be the NTP representation of 10am on the first Monday,
    // the <repeat interval> would be 1 week, the <active duration> would be 1 hour,
    // and the offsets would be zero and 25 hours.
    ImsList<IMS_UINT32> objOffsets;

    objOffsets.Append(90000);
    m_objTestRepeatTime = TestRepeatTime(
            "r=604800 3600 0 90000\r\n",
            "604800 3600 0 90000", 604800, 3600, 0, objOffsets);
    objOffsets.Clear();

    objOffsets.Append(25 * 3600);
    m_objTestRepeatTimeCompact = TestRepeatTime(
            "r=7d 1h 0 25h\r\n",
            "7d 1h 0 25h", 604800, 3600, 0, objOffsets);
    objOffsets.Clear();

    m_objTestRepeatTimeOneOffset = TestRepeatTime(
            "r=604800 3600 0\r\n",
            "604800 3600 0", 604800, 3600, 0, objOffsets);

    objOffsets.Append(90000);
    objOffsets.Append(180000);
    objOffsets.Append(270000);
    m_objTestRepeatTimeExtraOffsets = TestRepeatTime(
            "r=604800 3600 0 90000 180000 270000\r\n",
            "604800 3600 0 90000 180000 270000", 604800, 3600, 0, objOffsets);
    objOffsets.Clear();
    // clang-format on
}

TEST_F(SdpRepeatTimeTest, Constructor)
{
    SdpRepeatTime objRepeatTime;
    EXPECT_EQ(objRepeatTime.GetRepeatInterval(), 0);
    EXPECT_EQ(objRepeatTime.GetActiveDuration(), 0);
    EXPECT_EQ(objRepeatTime.GetFirstOffset(), 0);
    EXPECT_EQ(objRepeatTime.GetAdditionalOffsets().GetSize(), 0);
    EXPECT_EQ(objRepeatTime.GetValue(), AString::ConstNull());
}

TEST_F(SdpRepeatTimeTest, CopyConstructor)
{
    SdpRepeatTime objRepeatTime;
    ASSERT_TRUE(objRepeatTime.SetValue(m_objTestRepeatTime.m_nInterval,
            m_objTestRepeatTime.m_nActiveDuration, m_objTestRepeatTime.m_nFirstOffset,
            m_objTestRepeatTime.m_objAdditionalOffsets));

    SdpRepeatTime objNewRepeatTime(objRepeatTime);
    EXPECT_EQ(objNewRepeatTime.GetRepeatInterval(), objRepeatTime.GetRepeatInterval());
    EXPECT_EQ(objNewRepeatTime.GetActiveDuration(), objRepeatTime.GetActiveDuration());
    EXPECT_EQ(objNewRepeatTime.GetFirstOffset(), objRepeatTime.GetFirstOffset());
    EXPECT_EQ(objNewRepeatTime.GetAdditionalOffsets().GetSize(),
            objRepeatTime.GetAdditionalOffsets().GetSize());

    for (IMS_UINT32 i = 0; i < objRepeatTime.GetAdditionalOffsets().GetSize(); ++i)
    {
        EXPECT_EQ(objNewRepeatTime.GetAdditionalOffsets().GetAt(i),
                objRepeatTime.GetAdditionalOffsets().GetAt(i));
    }
}

TEST_F(SdpRepeatTimeTest, OperatorAssignment)
{
    SdpRepeatTime objRepeatTime;
    ASSERT_TRUE(objRepeatTime.SetValue(m_objTestRepeatTime.m_nInterval,
            m_objTestRepeatTime.m_nActiveDuration, m_objTestRepeatTime.m_nFirstOffset,
            m_objTestRepeatTime.m_objAdditionalOffsets));

    SdpRepeatTime objNewRepeatTime;
    objNewRepeatTime = objRepeatTime;
    EXPECT_EQ(objNewRepeatTime.GetRepeatInterval(), m_objTestRepeatTime.m_nInterval);
    EXPECT_EQ(objNewRepeatTime.GetActiveDuration(), m_objTestRepeatTime.m_nActiveDuration);
    EXPECT_EQ(objNewRepeatTime.GetFirstOffset(), m_objTestRepeatTime.m_nFirstOffset);
    EXPECT_EQ(objNewRepeatTime.GetAdditionalOffsets().GetSize(),
            m_objTestRepeatTime.m_objAdditionalOffsets.GetSize());

    for (IMS_UINT32 i = 0; i < m_objTestRepeatTime.m_objAdditionalOffsets.GetSize(); ++i)
    {
        EXPECT_EQ(objNewRepeatTime.GetAdditionalOffsets().GetAt(i),
                m_objTestRepeatTime.m_objAdditionalOffsets.GetAt(i));
    }
}

TEST_F(SdpRepeatTimeTest, Decode)
{
    SdpRepeatTime objRepeatTime;

    ASSERT_TRUE(objRepeatTime.Decode(m_objTestRepeatTime.m_strSdpLine));
    EXPECT_EQ(objRepeatTime.GetRepeatInterval(), m_objTestRepeatTime.m_nInterval);
    EXPECT_EQ(objRepeatTime.GetActiveDuration(), m_objTestRepeatTime.m_nActiveDuration);
    EXPECT_EQ(objRepeatTime.GetFirstOffset(), m_objTestRepeatTime.m_nFirstOffset);
    EXPECT_EQ(objRepeatTime.GetAdditionalOffsets().GetSize(),
            m_objTestRepeatTime.m_objAdditionalOffsets.GetSize());

    for (IMS_UINT32 i = 0; i < m_objTestRepeatTime.m_objAdditionalOffsets.GetSize(); ++i)
    {
        EXPECT_EQ(objRepeatTime.GetAdditionalOffsets().GetAt(i),
                m_objTestRepeatTime.m_objAdditionalOffsets.GetAt(i));
    }

    ASSERT_TRUE(objRepeatTime.Decode(m_objTestRepeatTimeCompact.m_strSdpLine));
    EXPECT_EQ(objRepeatTime.GetRepeatInterval(), m_objTestRepeatTimeCompact.m_nInterval);
    EXPECT_EQ(objRepeatTime.GetActiveDuration(), m_objTestRepeatTimeCompact.m_nActiveDuration);
    EXPECT_EQ(objRepeatTime.GetFirstOffset(), m_objTestRepeatTimeCompact.m_nFirstOffset);
    EXPECT_EQ(objRepeatTime.GetAdditionalOffsets().GetSize(),
            m_objTestRepeatTimeCompact.m_objAdditionalOffsets.GetSize());

    for (IMS_UINT32 i = 0; i < m_objTestRepeatTimeCompact.m_objAdditionalOffsets.GetSize(); ++i)
    {
        EXPECT_EQ(objRepeatTime.GetAdditionalOffsets().GetAt(i),
                m_objTestRepeatTimeCompact.m_objAdditionalOffsets.GetAt(i));
    }

    ASSERT_TRUE(objRepeatTime.Decode(m_objTestRepeatTimeOneOffset.m_strSdpLine));
    EXPECT_EQ(objRepeatTime.GetRepeatInterval(), m_objTestRepeatTimeOneOffset.m_nInterval);
    EXPECT_EQ(objRepeatTime.GetActiveDuration(), m_objTestRepeatTimeOneOffset.m_nActiveDuration);
    EXPECT_EQ(objRepeatTime.GetFirstOffset(), m_objTestRepeatTimeOneOffset.m_nFirstOffset);
    EXPECT_EQ(objRepeatTime.GetAdditionalOffsets().GetSize(),
            m_objTestRepeatTimeOneOffset.m_objAdditionalOffsets.GetSize());

    ASSERT_TRUE(objRepeatTime.Decode(m_objTestRepeatTimeExtraOffsets.m_strSdpLine));
    EXPECT_EQ(objRepeatTime.GetRepeatInterval(), m_objTestRepeatTimeExtraOffsets.m_nInterval);
    EXPECT_EQ(objRepeatTime.GetActiveDuration(), m_objTestRepeatTimeExtraOffsets.m_nActiveDuration);
    EXPECT_EQ(objRepeatTime.GetFirstOffset(), m_objTestRepeatTimeExtraOffsets.m_nFirstOffset);
    EXPECT_EQ(objRepeatTime.GetAdditionalOffsets().GetSize(),
            m_objTestRepeatTimeExtraOffsets.m_objAdditionalOffsets.GetSize());

    IMS_UINT32 nExtraOffsetCount = m_objTestRepeatTimeExtraOffsets.m_objAdditionalOffsets.GetSize();

    for (IMS_UINT32 i = 0; i < nExtraOffsetCount; ++i)
    {
        EXPECT_EQ(objRepeatTime.GetAdditionalOffsets().GetAt(i),
                m_objTestRepeatTimeExtraOffsets.m_objAdditionalOffsets.GetAt(i));
    }

    EXPECT_FALSE(objRepeatTime.Decode(AString::ConstNull()));
    EXPECT_FALSE(objRepeatTime.Decode(AString::ConstEmpty()));
    EXPECT_FALSE(objRepeatTime.Decode("3600 600"));
    EXPECT_FALSE(objRepeatTime.Decode("03600 600 0"));
    EXPECT_FALSE(objRepeatTime.Decode("3600a 600 0"));
    EXPECT_FALSE(objRepeatTime.Decode("3600 600b 0"));
    EXPECT_FALSE(objRepeatTime.Decode("3600 600 0c"));
    EXPECT_FALSE(objRepeatTime.Decode("3600 600 0 1200c"));
}

TEST_F(SdpRepeatTimeTest, Encode)
{
    SdpRepeatTime objRepeatTime;
    AString strEncoded = objRepeatTime.Encode();
    EXPECT_EQ(strEncoded, AString::ConstNull());

    ASSERT_TRUE(objRepeatTime.SetValue(m_objTestRepeatTime.m_nInterval,
            m_objTestRepeatTime.m_nActiveDuration, m_objTestRepeatTime.m_nFirstOffset,
            m_objTestRepeatTime.m_objAdditionalOffsets));
    strEncoded = objRepeatTime.Encode();
    EXPECT_EQ(strEncoded, m_objTestRepeatTime.m_strSdpFullLine);
}

TEST_F(SdpRepeatTimeTest, SetValue)
{
    SdpRepeatTime objRepeatTime;

    ASSERT_TRUE(objRepeatTime.SetValue(m_objTestRepeatTime.m_nInterval,
            m_objTestRepeatTime.m_nActiveDuration, m_objTestRepeatTime.m_nFirstOffset,
            m_objTestRepeatTime.m_objAdditionalOffsets));
    EXPECT_EQ(objRepeatTime.GetRepeatInterval(), m_objTestRepeatTime.m_nInterval);
    EXPECT_EQ(objRepeatTime.GetActiveDuration(), m_objTestRepeatTime.m_nActiveDuration);
    EXPECT_EQ(objRepeatTime.GetFirstOffset(), m_objTestRepeatTime.m_nFirstOffset);
    EXPECT_EQ(objRepeatTime.GetAdditionalOffsets().GetSize(),
            m_objTestRepeatTime.m_objAdditionalOffsets.GetSize());

    for (IMS_UINT32 i = 0; i < m_objTestRepeatTime.m_objAdditionalOffsets.GetSize(); ++i)
    {
        EXPECT_EQ(objRepeatTime.GetAdditionalOffsets().GetAt(i),
                m_objTestRepeatTime.m_objAdditionalOffsets.GetAt(i));
    }

    ASSERT_FALSE(objRepeatTime.SetValue(0, m_objTestRepeatTime.m_nActiveDuration,
            m_objTestRepeatTime.m_nFirstOffset, m_objTestRepeatTime.m_objAdditionalOffsets));
}

}  // namespace android
