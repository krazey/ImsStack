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

#include "SdpVersion.h"

namespace android
{

class SdpVersionTest : public ::testing::Test
{
protected:
    enum
    {
        TEST_SDP_VERSION = 1
    };
};

TEST_F(SdpVersionTest, Constructor)
{
    SdpVersion objVersion;
    EXPECT_EQ(objVersion.GetVersion(), SdpVersion::SDP_VERSION);
}

TEST_F(SdpVersionTest, CopyConstructor)
{
    SdpVersion objVersion;
    objVersion.SetVersion(TEST_SDP_VERSION);

    SdpVersion objNewVersion(objVersion);
    EXPECT_EQ(objNewVersion.GetVersion(), objVersion.GetVersion());
}

TEST_F(SdpVersionTest, OperatorAssignment)
{
    SdpVersion objVersion;
    objVersion.SetVersion(TEST_SDP_VERSION);

    SdpVersion objNewVersion;
    objNewVersion = objVersion;
    EXPECT_EQ(objNewVersion.GetVersion(), TEST_SDP_VERSION);
}

TEST_F(SdpVersionTest, Decode)
{
    SdpVersion objVersion;
    AString strSdpLine;
    strSdpLine.SetNumber(SdpVersion::SDP_VERSION);
    ASSERT_TRUE(objVersion.Decode(strSdpLine));
    EXPECT_EQ(objVersion.GetVersion(), SdpVersion::SDP_VERSION);

    strSdpLine.SetNumber(TEST_SDP_VERSION);
    ASSERT_TRUE(objVersion.Decode(strSdpLine));
    EXPECT_EQ(objVersion.GetVersion(), TEST_SDP_VERSION);

    EXPECT_FALSE(objVersion.Decode(AString::ConstNull()));
    EXPECT_FALSE(objVersion.Decode(AString::ConstEmpty()));
    EXPECT_FALSE(objVersion.Decode("a"));
}

TEST_F(SdpVersionTest, Encode)
{
    SdpVersion objVersion;
    AString strExpected;
    AString strEncoded;

    strExpected.Sprintf("v=%d\r\n", SdpVersion::SDP_VERSION);
    strEncoded = objVersion.Encode();
    EXPECT_EQ(strEncoded, strExpected);

    strExpected.Sprintf("v=%d\r\n", TEST_SDP_VERSION);
    objVersion.SetVersion(TEST_SDP_VERSION);
    strEncoded = objVersion.Encode();
    EXPECT_EQ(strEncoded, strExpected);
}

TEST_F(SdpVersionTest, SetVersion)
{
    SdpVersion objVersion;
    EXPECT_EQ(objVersion.GetVersion(), SdpVersion::SDP_VERSION);

    objVersion.SetVersion(TEST_SDP_VERSION);
    EXPECT_EQ(objVersion.GetVersion(), TEST_SDP_VERSION);

    objVersion.SetVersion();
    EXPECT_EQ(objVersion.GetVersion(), SdpVersion::SDP_VERSION);
}

}  // namespace android
