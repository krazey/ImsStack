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
#include "video/VideoSdpGenerator.h"

const AString STR_PROFILE_LEVEL_ID = "profile-level-id=42C00C";
const AString STR_SPROP_PRAMSET = "sprop-parameter-sets=Z0LAFukDwKMg,aM4G4g==";

const AString PROFILE_LEVEL_ID = "42C00C";
const AString SPROP_PRAMSET = "Z0LAFukDwKMg,aM4G4g==";

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

class VideoSdpGeneratorAvcTest : public VideoSdpGenerator, public ::testing::Test
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

TEST_F(VideoSdpGeneratorAvcTest, TestGenerateAvcFmtp)
{
    AString strFmtp = AString::ConstNull();

    strFmtp = GenerateAvcFmtp(m_pAvcFmtpEmpty);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = GenerateAvcFmtp(m_pAvcFmtpFull);

    EXPECT_EQ(strFmtp.Contains(STR_PROFILE_LEVEL_ID), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_PACKETIZATION_MODE), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_SPROP_PRAMSET), IMS_TRUE);

    AString strResult = STR_PROFILE_LEVEL_ID;
    AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_PACKETIZATION_MODE);
    AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_SPROP_PRAMSET);

    EXPECT_EQ(strFmtp, strResult);
}

TEST_F(VideoSdpGeneratorAvcTest, TestAddProfileLevelIdToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddProfileLevelIdToFmtp(m_pAvcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddProfileLevelIdToFmtp(m_pAvcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_PROFILE_LEVEL_ID);
}

TEST_F(VideoSdpGeneratorAvcTest, TestAddPacketizationModeToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddPacketizationModeToFmtp(m_pAvcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddPacketizationModeToFmtp(m_pAvcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_PACKETIZATION_MODE);
}

TEST_F(VideoSdpGeneratorAvcTest, TestAddSpropParameterSetsToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddSpropParameterSetsToFmtp(m_pAvcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddSpropParameterSetsToFmtp(m_pAvcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_SPROP_PRAMSET);
}

class VideoSdpGeneratorHevcTest : public VideoSdpGenerator, public ::testing::Test
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

TEST_F(VideoSdpGeneratorHevcTest, TestGenerateHevcFmtp)
{
    AString strFmtp = AString::ConstNull();

    strFmtp = GenerateHevcFmtp(m_pHevcFmtpEmpty);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = GenerateHevcFmtp(m_pHevcFmtpFull);

    EXPECT_EQ(strFmtp.Contains(STR_PROFILE_ID), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_LEVEL_ID), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_SPROP_PARAMSET), IMS_TRUE);

    AString strResult = STR_PROFILE_ID;
    AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_LEVEL_ID);
    AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_SPROP_PARAMSET);

    EXPECT_EQ(strFmtp, strResult);
}

TEST_F(VideoSdpGeneratorHevcTest, TestAddProfilelIdToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddProfileIdToFmtp(m_pHevcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddProfileIdToFmtp(m_pHevcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_PROFILE_ID);
}

TEST_F(VideoSdpGeneratorHevcTest, TestAddLevelIdToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddLevelIdToFmtp(m_pHevcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddLevelIdToFmtp(m_pHevcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_LEVEL_ID);
}

TEST_F(VideoSdpGeneratorHevcTest, TestAddPacketizationModeToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddPacketizationModeToFmtp(m_pHevcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddPacketizationModeToFmtp(m_pHevcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_PACKETIZATION_MODE);
}

TEST_F(VideoSdpGeneratorHevcTest, TestAddSpropParamsToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddSpropParamsToFmtp(m_pHevcFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddSpropParamsToFmtp(m_pHevcFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_SPROP_PARAMSET);
}
