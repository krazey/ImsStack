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

#include "SdpTimezone.h"

namespace android
{

class TestTimezone
{
public:
    inline TestTimezone() :
            m_strSdpFullLine(AString::ConstNull()),
            m_strSdpLine(AString::ConstNull()),
            m_objAdjustments(ImsList<SdpTimezone::ZoneAdjustment>())
    {
    }
    inline TestTimezone(const AString& strSdpFullLine, const AString& strSdpLine,
            const ImsList<SdpTimezone::ZoneAdjustment>& objAdjustments) :
            m_strSdpFullLine(strSdpFullLine),
            m_strSdpLine(strSdpLine),
            m_objAdjustments(objAdjustments)
    {
    }
    inline TestTimezone(const TestTimezone& other) :
            m_strSdpFullLine(other.m_strSdpFullLine),
            m_strSdpLine(other.m_strSdpLine),
            m_objAdjustments(other.m_objAdjustments)
    {
    }
    inline ~TestTimezone() {}

    inline TestTimezone& operator=(const TestTimezone& other)
    {
        if (this != &other)
        {
            m_strSdpFullLine = other.m_strSdpFullLine;
            m_strSdpLine = other.m_strSdpLine;
            m_objAdjustments = other.m_objAdjustments;
        }

        return *this;
    }

public:
    AString m_strSdpFullLine;
    AString m_strSdpLine;
    ImsList<SdpTimezone::ZoneAdjustment> m_objAdjustments;
};

class SdpTimezoneTest : public ::testing::Test
{
public:
    SdpTimezoneTest();

protected:
    TestTimezone m_objTestTimezone;
    TestTimezone m_objTestTimezoneCompact;
};

SdpTimezoneTest::SdpTimezoneTest()
{
    // clang-format off
    ImsList<SdpTimezone::ZoneAdjustment> objAdjustments;

    objAdjustments.Append(SdpTimezone::ZoneAdjustment(2882844526, -3600));
    objAdjustments.Append(SdpTimezone::ZoneAdjustment(2898848070, 0));
    m_objTestTimezone = TestTimezone(
            "z=2882844526 -3600 2898848070 0\r\n",
            "2882844526 -3600 2898848070 0", objAdjustments);
    objAdjustments.Clear();

    objAdjustments.Append(SdpTimezone::ZoneAdjustment(2882844527, -3600));
    objAdjustments.Append(SdpTimezone::ZoneAdjustment(2898848071, 1));
    m_objTestTimezoneCompact = TestTimezone(
            "z=2882844527 -1h 2898848071 1\r\n",
            "2882844527 -1h 2898848071 1", objAdjustments);
    objAdjustments.Clear();
    // clang-format on
}

TEST_F(SdpTimezoneTest, Constructor)
{
    SdpTimezone objTimezone;
    EXPECT_EQ(objTimezone.GetAdjustments().GetSize(), 0);
    EXPECT_EQ(objTimezone.GetValue(), AString::ConstNull());
}

TEST_F(SdpTimezoneTest, CopyConstructor)
{
    SdpTimezone objTimezone;
    IMS_UINT32 nAdjustmentCount = m_objTestTimezone.m_objAdjustments.GetSize();

    for (IMS_UINT32 i = 0; i < nAdjustmentCount; ++i)
    {
        const SdpTimezone::ZoneAdjustment& objZa = m_objTestTimezone.m_objAdjustments.GetAt(i);
        ASSERT_TRUE(objTimezone.AddAdjustment(objZa.GetAdjustmentTime(), objZa.GetOffset()));
    }

    SdpTimezone objNewTimezone(objTimezone);
    ASSERT_EQ(objNewTimezone.GetAdjustments().GetSize(), nAdjustmentCount);
    ASSERT_EQ(objTimezone.GetAdjustments().GetSize(), nAdjustmentCount);

    for (IMS_UINT32 i = 0; i < nAdjustmentCount; ++i)
    {
        const SdpTimezone::ZoneAdjustment& objZa1 = objNewTimezone.GetAdjustments().GetAt(i);
        const SdpTimezone::ZoneAdjustment& objZa2 = objTimezone.GetAdjustments().GetAt(i);
        EXPECT_EQ(objZa1, objZa2);
    }
}

TEST_F(SdpTimezoneTest, OperatorAssignment)
{
    SdpTimezone objTimezone;
    IMS_UINT32 nAdjustmentCount = m_objTestTimezone.m_objAdjustments.GetSize();

    for (IMS_UINT32 i = 0; i < nAdjustmentCount; ++i)
    {
        const SdpTimezone::ZoneAdjustment& objZa = m_objTestTimezone.m_objAdjustments.GetAt(i);
        ASSERT_TRUE(objTimezone.AddAdjustment(objZa.GetAdjustmentTime(), objZa.GetOffset()));
    }

    SdpTimezone objNewTimezone;
    objNewTimezone = objTimezone;
    ASSERT_EQ(objNewTimezone.GetAdjustments().GetSize(), nAdjustmentCount);
    ASSERT_EQ(objTimezone.GetAdjustments().GetSize(), nAdjustmentCount);

    for (IMS_UINT32 i = 0; i < nAdjustmentCount; ++i)
    {
        const SdpTimezone::ZoneAdjustment& objZa1 = objNewTimezone.GetAdjustments().GetAt(i);
        const SdpTimezone::ZoneAdjustment& objZa2 = objTimezone.GetAdjustments().GetAt(i);
        // cppcheck-suppress knownConditionTrueFalse
        EXPECT_EQ(objZa1, objZa2);
    }
}

TEST_F(SdpTimezoneTest, Decode)
{
    SdpTimezone objTimezone;

    ASSERT_TRUE(objTimezone.Decode(m_objTestTimezone.m_strSdpLine));
    EXPECT_EQ(objTimezone.GetAdjustments().GetSize(), m_objTestTimezone.m_objAdjustments.GetSize());

    IMS_UINT32 nAdjustmentCount = objTimezone.GetAdjustments().GetSize();

    for (IMS_UINT32 i = 0; i < nAdjustmentCount; ++i)
    {
        const SdpTimezone::ZoneAdjustment& objZa1 = objTimezone.GetAdjustments().GetAt(i);
        const SdpTimezone::ZoneAdjustment& objZa2 = m_objTestTimezone.m_objAdjustments.GetAt(i);
        EXPECT_EQ(objZa1, objZa2);
    }

    ASSERT_TRUE(objTimezone.Decode(m_objTestTimezoneCompact.m_strSdpLine));
    EXPECT_EQ(objTimezone.GetAdjustments().GetSize(),
            m_objTestTimezoneCompact.m_objAdjustments.GetSize());

    nAdjustmentCount = objTimezone.GetAdjustments().GetSize();

    for (IMS_UINT32 i = 0; i < nAdjustmentCount; ++i)
    {
        const SdpTimezone::ZoneAdjustment& objZa1 = objTimezone.GetAdjustments().GetAt(i);
        const SdpTimezone::ZoneAdjustment& objZa2 =
                m_objTestTimezoneCompact.m_objAdjustments.GetAt(i);
        EXPECT_EQ(objZa1, objZa2);
    }

    EXPECT_FALSE(objTimezone.Decode(AString::ConstNull()));
    EXPECT_FALSE(objTimezone.Decode(AString::ConstEmpty()));
    EXPECT_FALSE(objTimezone.Decode("2882844527"));
    EXPECT_FALSE(objTimezone.Decode("2882844527 0 2882844528"));
    EXPECT_FALSE(objTimezone.Decode("3600 0"));
    EXPECT_FALSE(objTimezone.Decode("288284452a 0"));
    EXPECT_FALSE(objTimezone.Decode("2882844527 1a"));
}

TEST_F(SdpTimezoneTest, Encode)
{
    SdpTimezone objTimezone;
    AString strEncoded = objTimezone.Encode();
    EXPECT_EQ(strEncoded, AString::ConstNull());

    IMS_UINT32 nAdjustmentCount = m_objTestTimezone.m_objAdjustments.GetSize();

    for (IMS_UINT32 i = 0; i < nAdjustmentCount; ++i)
    {
        const SdpTimezone::ZoneAdjustment& objZa = m_objTestTimezone.m_objAdjustments.GetAt(i);
        ASSERT_TRUE(objTimezone.AddAdjustment(objZa.GetAdjustmentTime(), objZa.GetOffset()));
    }
    strEncoded = objTimezone.Encode();
    EXPECT_EQ(strEncoded, m_objTestTimezone.m_strSdpFullLine);
}

TEST_F(SdpTimezoneTest, AddAdjustment)
{
    SdpTimezone objTimezone;
    IMS_UINT32 nAdjustmentCount = m_objTestTimezone.m_objAdjustments.GetSize();

    for (IMS_UINT32 i = 0; i < nAdjustmentCount; ++i)
    {
        const SdpTimezone::ZoneAdjustment& objZa = m_objTestTimezone.m_objAdjustments.GetAt(i);
        ASSERT_TRUE(objTimezone.AddAdjustment(objZa.GetAdjustmentTime(), objZa.GetOffset()));
    }

    for (IMS_UINT32 i = 0; i < nAdjustmentCount; ++i)
    {
        const SdpTimezone::ZoneAdjustment& objZa = objTimezone.GetAdjustments().GetAt(i);
        const SdpTimezone::ZoneAdjustment& objOtherZa = m_objTestTimezone.m_objAdjustments.GetAt(i);
        EXPECT_EQ(objZa, objOtherZa);
    }

    EXPECT_FALSE(objTimezone.AddAdjustment(0, 3600));
    EXPECT_FALSE(objTimezone.AddAdjustment(123123123, 3600));
}

}  // namespace android
