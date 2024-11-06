/*
 * Copyright (C) 2024 The Android Open Source Project
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
#include "video/VideoNegoAvc.h"

const AString SEMICOLON = ";";
const AString COMMA = ",";

const AString STR_PROFILE_LEVEL_ID = "profile-level-id=42C00C";
const AString STR_PACKETIZATION_MODE = "packetization-mode=1";
const AString STR_SPROP_PRAMSET = "sprop-parameter-sets=Z0LAFukDwKMg,aM4G4g==";

const AString PROFILE_LEVEL_ID = "42C00C";
const IMS_UINT32 PACKETIZATION_MODE = 1;
const AString SPROP_PRAMSET = "Z0LAFukDwKMg,aM4G4g==";

class VideoNegoAvcTest : public ::testing::Test
{
public:
    VideoProfile::AvcFmtp* m_pAvcFmtpFull;
    VideoProfile::AvcFmtp* m_pAvcFmtpEmpty;

protected:
    virtual void SetUp() override
    {
        m_pAvcFmtpFull = new VideoProfile::AvcFmtp();
        m_pAvcFmtpEmpty = new VideoProfile::AvcFmtp();

        m_pAvcFmtpFull->SetProfileLevelId(PROFILE_LEVEL_ID);
        m_pAvcFmtpFull->SetPacketizationMode(PACKETIZATION_MODE);
        m_pAvcFmtpFull->SetSpropParam(SPROP_PRAMSET);

        m_pAvcFmtpFull->SetShowProfileLevelId(IMS_TRUE);
        m_pAvcFmtpFull->SetShowPacketizationMode(IMS_TRUE);
        m_pAvcFmtpFull->SetShowSpropParam(IMS_TRUE);
    }

    virtual void TearDown() override
    {
        delete m_pAvcFmtpFull;
        delete m_pAvcFmtpEmpty;
    }
};

TEST_F(VideoNegoAvcTest, TestSetSdpFmtpFromAvcFmtp)
{
    AString strFmtp = AString::ConstNull();

    strFmtp = VideoNegoAvc::SetSdpFmtpFromAvcFmtp(m_pAvcFmtpEmpty);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = VideoNegoAvc::SetSdpFmtpFromAvcFmtp(m_pAvcFmtpFull);

    EXPECT_EQ(strFmtp.Contains(STR_PROFILE_LEVEL_ID), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_PACKETIZATION_MODE), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_SPROP_PRAMSET), IMS_TRUE);

    AString strResult = STR_PROFILE_LEVEL_ID;
    VideoNegoAvc::AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_PACKETIZATION_MODE);
    VideoNegoAvc::AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_SPROP_PRAMSET);

    EXPECT_EQ(strFmtp, strResult);
}

TEST_F(VideoNegoAvcTest, TestAddProfileLevelIdToFmtp)
{
    AString strFmtp = AString::ConstNull();

    VideoNegoAvc::AddProfileLevelIdToFmtp(m_pAvcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    VideoNegoAvc::AddProfileLevelIdToFmtp(m_pAvcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_PROFILE_LEVEL_ID);
}

TEST_F(VideoNegoAvcTest, TestAddPacketizationModeToFmtp)
{
    AString strFmtp = AString::ConstNull();

    VideoNegoAvc::AddPacketizationModeToFmtp(m_pAvcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    VideoNegoAvc::AddPacketizationModeToFmtp(m_pAvcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_PACKETIZATION_MODE);
}

TEST_F(VideoNegoAvcTest, TestAddSpropParameterSetsToFmtp)
{
    AString strFmtp = AString::ConstNull();

    VideoNegoAvc::AddSpropParameterSetsToFmtp(m_pAvcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    VideoNegoAvc::AddSpropParameterSetsToFmtp(m_pAvcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_SPROP_PRAMSET);
}
