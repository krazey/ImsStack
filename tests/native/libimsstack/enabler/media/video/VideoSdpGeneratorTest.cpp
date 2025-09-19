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
#include "video/VideoProfile.h"

#include "core/media/MockIMediaDescriptor.h"

using ::testing::_;

const AString STR_PROFILE_LEVEL_ID = "profile-level-id=42C00C";
const AString STR_SPROP_PRAMSET = "sprop-parameter-sets=Z0LAFukDwKMg,aM4G4g==";

const AString PROFILE_LEVEL_ID = "42C00C";
const AString SPROP_PRAMSET = "Z0LAFukDwKMg,aM4G4g==";
const IMS_SINT32 CVO_ID = 1;

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
    std::shared_ptr<VideoProfile::AvcFmtp> m_pAvcFmtpFull;
    std::shared_ptr<VideoProfile::AvcFmtp> m_pAvcFmtpEmpty;

protected:
    virtual void SetUp() override
    {
        m_pAvcFmtpFull = std::make_shared<VideoProfile::AvcFmtp>();
        m_pAvcFmtpEmpty = std::make_shared<VideoProfile::AvcFmtp>();

        m_pAvcFmtpFull->SetProfileLevelId(PROFILE_LEVEL_ID);
        m_pAvcFmtpFull->SetPacketizationMode(PACKETIZATION_MODE);
        m_pAvcFmtpFull->SetSpropParam(SPROP_PRAMSET);

        m_pAvcFmtpFull->SetVisibleProfileLevelId(IMS_TRUE);
        m_pAvcFmtpFull->SetVisiblePacketizationMode(IMS_TRUE);
        m_pAvcFmtpFull->SetVisibleSpropParam(IMS_TRUE);
    }

    virtual void TearDown() override {}
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
    std::shared_ptr<VideoProfile::HevcFmtp> m_pHevcFmtpFull;
    std::shared_ptr<VideoProfile::HevcFmtp> m_pHevcFmtpEmpty;

protected:
    virtual void SetUp() override
    {
        m_pHevcFmtpFull = std::make_shared<VideoProfile::HevcFmtp>();
        m_pHevcFmtpEmpty = std::make_shared<VideoProfile::HevcFmtp>();

        m_pHevcFmtpFull->SetProfile(PROFILE_ID);
        m_pHevcFmtpFull->SetLevel(LEVEL_ID);
        m_pHevcFmtpFull->SetPacketizationMode(PACKETIZATION_MODE);
        m_pHevcFmtpFull->SetSpropParam(SPROP_PARAMSET);

        m_pHevcFmtpFull->SetVisibleProfile(IMS_TRUE);
        m_pHevcFmtpFull->SetVisibleLevel(IMS_TRUE);
        m_pHevcFmtpFull->SetVisiblePacketizationMode(IMS_TRUE);
        m_pHevcFmtpFull->SetVisibleSpropParam(IMS_TRUE);
    }

    virtual void TearDown() override {}
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

class VideoSdpGeneratorTest : public VideoSdpGenerator, public ::testing::Test
{
public:
    std::unique_ptr<VideoProfile> m_pProfile;
    std::unique_ptr<MockIMediaDescriptor> m_pDescriptor;

protected:
    virtual void SetUp() override
    {
        m_pProfile = std::make_unique<VideoProfile>();
        m_pDescriptor = std::make_unique<MockIMediaDescriptor>();
    }

    virtual void TearDown() override {}
};

TEST_F(VideoSdpGeneratorTest, TestGenerateFrameRateAttributeAddedForPositive)
{
    const IMS_SINT32 kFrameRate = 30;
    m_pProfile->SetFrameRate(kFrameRate);
    EXPECT_CALL(*m_pDescriptor, AddAttributeInt(SdpAttribute::FRAMERATE, kFrameRate, _)).Times(1);
    GenerateFrameRate(m_pDescriptor.get(), m_pProfile.get());
}

TEST_F(VideoSdpGeneratorTest, TestGenerateFrameRateAttributeNotAddedForZero)
{
    m_pProfile->SetFrameRate(0);
    EXPECT_CALL(*m_pDescriptor, AddAttributeInt(_, _, _)).Times(0);
    GenerateFrameRate(m_pDescriptor.get(), m_pProfile.get());
}

TEST_F(VideoSdpGeneratorTest, TestGenerateFrameRateAttributeNotAddedForNegative)
{
    m_pProfile->SetFrameRate(-15);
    EXPECT_CALL(*m_pDescriptor, AddAttributeInt(_, _, _)).Times(0);
    GenerateFrameRate(m_pDescriptor.get(), m_pProfile.get());
}

TEST_F(VideoSdpGeneratorTest, TestGenerateCvoAttributeAddedForPositive)
{
    const IMS_SINT32 kCvoId = CVO_ID;
    m_pProfile->SetCvoId(kCvoId);
    AString expected_attr;
    expected_attr.Sprintf("%d urn:3gpp:video-orientation", kCvoId);

    EXPECT_CALL(*m_pDescriptor,
            AddAttribute(SdpAttribute::ATTRIBUTE_OTHER, expected_attr, AString("extmap")))
            .Times(1);

    GenerateCvo(m_pDescriptor.get(), m_pProfile.get());
}

TEST_F(VideoSdpGeneratorTest, TestGenerateCvoAttributeNotAddedForZero)
{
    m_pProfile->SetCvoId(0);
    EXPECT_CALL(*m_pDescriptor, AddAttribute(_, _, _)).Times(0);
    GenerateCvo(m_pDescriptor.get(), m_pProfile.get());
}

TEST_F(VideoSdpGeneratorTest, TestGenerateCvoAttributeNotAddedForNegative)
{
    m_pProfile->SetCvoId(-1);
    EXPECT_CALL(*m_pDescriptor, AddAttribute(_, _, _)).Times(0);
    GenerateCvo(m_pDescriptor.get(), m_pProfile.get());
}

TEST_F(VideoSdpGeneratorTest, TestGenerateRtpMapNoPayload)
{
    AString rtpMap, payloadNum;
    MediaBaseProfile::RtpMap rtpMapObj;
    // No payload set, so it should be empty/zero
    EXPECT_FALSE(GenerateRtpMap(rtpMap, payloadNum, rtpMapObj));
}

TEST_F(VideoSdpGeneratorTest, TestGenerateRtpMapInvalidPayload)
{
    AString rtpMap, payloadNum;
    MediaBaseProfile::RtpMap rtpMapObj;

    // Empty payload type
    rtpMapObj.SetPayloadNumber(99);
    rtpMapObj.SetPayloadType("");
    rtpMapObj.SetSamplingRate(90000);
    EXPECT_FALSE(GenerateRtpMap(rtpMap, payloadNum, rtpMapObj));

    // Null payload type
    rtpMapObj.SetPayloadType(AString::ConstNull());
    rtpMapObj.SetPayloadNumber(99);
    rtpMapObj.SetSamplingRate(90000);
    EXPECT_FALSE(GenerateRtpMap(rtpMap, payloadNum, rtpMapObj));
}
