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

#include "SdpLine.h"

namespace android
{

class TestSdpLine : public SdpLine
{
public:
    inline TestSdpLine() :
            SdpLine()
    {
    }
    ~TestSdpLine() override = default;
};

class SdpLineTest : public ::testing::Test
{
public:
    SdpLineTest() = default;

protected:
    TestSdpLine m_objSdpLine;
};

TEST_F(SdpLineTest, Constructor)
{
    AString strValue = m_objSdpLine.GetValue();
    EXPECT_EQ(strValue, AString::ConstNull());
}

TEST_F(SdpLineTest, CopyConstructor)
{
    TestSdpLine objSdpLine(m_objSdpLine);
    AString strValue = objSdpLine.GetValue();
    EXPECT_EQ(strValue, AString::ConstNull());
}

TEST_F(SdpLineTest, OperatorAssignment)
{
    TestSdpLine objSdpLine;
    objSdpLine = m_objSdpLine;
    AString strValue = objSdpLine.GetValue();
    EXPECT_EQ(strValue, AString::ConstNull());
}

TEST_F(SdpLineTest, Decode)
{
    EXPECT_FALSE(m_objSdpLine.Decode("sendrecv"));
    EXPECT_FALSE(m_objSdpLine.Decode(AString::ConstNull()));
    EXPECT_FALSE(m_objSdpLine.Decode(AString::ConstEmpty()));
}

TEST_F(SdpLineTest, Encode)
{
    AString strEncoded = m_objSdpLine.Encode();
    EXPECT_EQ(strEncoded, AString::ConstNull());
}

}  // namespace android
