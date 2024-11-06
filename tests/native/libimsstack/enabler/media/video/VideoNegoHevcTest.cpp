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
#include "video/VideoNegoHevc.h"

const AString SEMICOLON = ";";
const AString COMMA = ",";

const AString STR_PROFILE_ID = "profile-id=1";
const AString STR_LEVEL_ID = "level-id=93";
const AString STR_PACKETIZATION_MODE = "packetization-mode=1";
const AString STR_SPROP_PARAMSET = "sprop-vps=AAAAAUABDAH//wFgAAADALAAAAMAAAMAWqxZ;"
                                   "sprop-sps=AAAAAUIBAQFgAAADALAAAAMAAAMAWqAPCAKBZa7kyS7gC7QoSg==;"
                                   "sprop-pps=AAAAAUQBwPPAAhA=";

const VIDEO_PROFILE_HEVC PROFILE_ID = HEVC_PROFILE_MAIN;
const IMS_SINT32 LEVEL_ID = 93;
const IMS_SINT32 PACKETIZATION_MODE = 1;
const AString SPROP_PARAMSET = "AAAAAUABDAH//wFgAAADALAAAAMAAAMAWqxZ,"
                               "AAAAAUIBAQFgAAADALAAAAMAAAMAWqAPCAKBZa7kyS7gC7QoSg==,"
                               "AAAAAUQBwPPAAhA=";

class VideoNegoHevcTest : public ::testing::Test
{
public:
    VideoProfile::HevcFmtp* m_pHevcFmtpFull;
    VideoProfile::HevcFmtp* m_pHevcFmtpEmpty;

protected:
    virtual void SetUp() override
    {
        m_pHevcFmtpFull = new VideoProfile::HevcFmtp();
        m_pHevcFmtpEmpty = new VideoProfile::HevcFmtp();

        m_pHevcFmtpFull->SetProfile(PROFILE_ID);
        m_pHevcFmtpFull->SetLevel(LEVEL_ID);
        m_pHevcFmtpFull->SetPacketizationMode(PACKETIZATION_MODE);
        m_pHevcFmtpFull->SetSpropParam(SPROP_PARAMSET);

        m_pHevcFmtpFull->SetShowProfile(IMS_TRUE);
        m_pHevcFmtpFull->SetShowLevel(IMS_TRUE);
        m_pHevcFmtpFull->SetShowPacketizationMode(IMS_TRUE);
        m_pHevcFmtpFull->SetShowSpropParam(IMS_TRUE);
    }

    virtual void TearDown() override
    {
        delete m_pHevcFmtpFull;
        delete m_pHevcFmtpEmpty;
    }
};

TEST_F(VideoNegoHevcTest, TestSetSdpFmtpFromHevcFmtp)
{
    AString strFmtp = AString::ConstNull();

    strFmtp = VideoNegoHevc::SetSdpFmtpFromHevcFmtp(m_pHevcFmtpEmpty);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = VideoNegoHevc::SetSdpFmtpFromHevcFmtp(m_pHevcFmtpFull);

    EXPECT_EQ(strFmtp.Contains(STR_PROFILE_ID), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_LEVEL_ID), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_SPROP_PARAMSET), IMS_TRUE);

    AString strResult = STR_PROFILE_ID;
    VideoNegoHevc::AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_LEVEL_ID);
    VideoNegoHevc::AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_SPROP_PARAMSET);

    EXPECT_EQ(strFmtp, strResult);
}

TEST_F(VideoNegoHevcTest, TestAddProfilelIdToFmtp)
{
    AString strFmtp = AString::ConstNull();

    VideoNegoHevc::AddProfileIdToFmtp(m_pHevcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    VideoNegoHevc::AddProfileIdToFmtp(m_pHevcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_PROFILE_ID);
}

TEST_F(VideoNegoHevcTest, TestAddLevelIdToFmtp)
{
    AString strFmtp = AString::ConstNull();

    VideoNegoHevc::AddLevelIdToFmtp(m_pHevcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    VideoNegoHevc::AddLevelIdToFmtp(m_pHevcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_LEVEL_ID);
}

TEST_F(VideoNegoHevcTest, TestAddPacketizationModeToFmtp)
{
    AString strFmtp = AString::ConstNull();

    VideoNegoHevc::AddPacketizationModeToFmtp(m_pHevcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    VideoNegoHevc::AddPacketizationModeToFmtp(m_pHevcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_PACKETIZATION_MODE);
}

TEST_F(VideoNegoHevcTest, TestAddSpropParamsToFmtp)
{
    AString strFmtp = AString::ConstNull();

    VideoNegoHevc::AddSpropParamsToFmtp(m_pHevcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    VideoNegoHevc::AddSpropParamsToFmtp(m_pHevcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_SPROP_PARAMSET);
}
