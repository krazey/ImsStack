/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "MediaDef.h"
#include "config/ImsCodec.h"
#include "video/VideoProfile.h"
#include "video/VideoProfileGenerator.h"

#include "MockICoreService.h"
#include "MockMediaProfileFactory.h"
#include "config/MockCodecAvcConfig.h"
#include "config/MockCodecHevcConfig.h"
#include "config/MockVideoConfiguration.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

/**
 * @brief Test fixture for the VideoProfileGenerator class.
 *
 * Sets up a common environment for testing the VideoProfileGenerator, including
 * the generator itself and mocks for its dependencies.
 */
class VideoProfileGeneratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_pGenerator = std::make_unique<VideoProfileGenerator>();
        m_pProfile = std::make_shared<VideoProfile>();
        m_pConfig = std::make_unique<NiceMock<MockVideoConfiguration>>();
        m_pIService = std::make_unique<NiceMock<MockICoreService>>();

        MockMediaProfileFactory::SetInstance(&m_objMediaProfileFactory);

        ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pProfile));

        // --- Set up default mock expectations for MediaProfileGenerator part ---
        ON_CALL(*m_pConfig, GetCodecConfigs()).WillByDefault(ReturnRef(m_lstCodecConfig));
        ON_CALL(*m_pConfig, GetAsBandwidthKbps()).WillByDefault(Return(1024));
        ON_CALL(*m_pConfig, GetRsBandwidthBps()).WillByDefault(Return(8000));
        ON_CALL(*m_pConfig, GetRrBandwidthBps()).WillByDefault(Return(6000));
        ON_CALL(*m_pConfig, GetRtcpIntervalOnActive()).WillByDefault(Return(5));
        ON_CALL(*m_pConfig, GetRtcpIntervalOnHold()).WillByDefault(Return(50));

        // --- Set up default mock expectations for VideoProfileGenerator part ---
        ON_CALL(*m_pConfig, GetCvoId()).WillByDefault(Return(1));
        ON_CALL(*m_pConfig, IsVideoAvpfEnabled()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(*m_pConfig, IsAvpfCapabilityNegotiationEnabled()).WillByDefault(Return(IMS_FALSE));
        ON_CALL(*m_pConfig, GetSdpOfferCapNegoForAvpf())
                .WillByDefault(Return(MediaConfiguration::CAPNEG_OFFER_NONE));

        m_objIpAddr = IpAddress(LOCAL_IP);
        ON_CALL(*m_pIService.get(), GetIpAddress()).WillByDefault(ReturnRef(m_objIpAddr));
    }

    virtual void TearDown() override { MockMediaProfileFactory::SetInstance(IMS_NULL); }

    MockMediaProfileFactory m_objMediaProfileFactory;
    IpAddress m_objIpAddr;
    ImsList<CodecConfig*> m_lstCodecConfig;
    std::shared_ptr<VideoProfile> m_pProfile;
    std::unique_ptr<VideoProfileGenerator> m_pGenerator;
    std::unique_ptr<NiceMock<MockVideoConfiguration>> m_pConfig;
    std::unique_ptr<NiceMock<MockICoreService>> m_pIService;

    const IMS_SINT32 SLOT_ID = 0;
    const AString LOCAL_IP = "127.0.0.1";
};

/**
 * @brief Verifies that a basic video profile is generated correctly when AVPF is disabled.
 */
TEST_F(VideoProfileGeneratorTest, SetProfileBasic)
{
    // Arrange
    EXPECT_CALL(*m_pConfig, IsVideoAvpfEnabled()).WillRepeatedly(Return(IMS_FALSE));

    // Act
    auto pBaseProfile = m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(),
            m_pConfig.get(), VideoProfileGeneratorTest::SLOT_ID);
    auto pProfile = std::static_pointer_cast<VideoProfile>(pBaseProfile);

    // Assert
    EXPECT_EQ(pProfile->GetBandwidthAs(), 1024);
    EXPECT_EQ(pProfile->GetBandwidthRs(), 8000);
    EXPECT_EQ(pProfile->GetBandwidthRr(), 6000);
    EXPECT_EQ(pProfile->GetRtcpInterval(), 50);
    EXPECT_EQ(pProfile->GetCvoId(), 1);
    EXPECT_EQ(pProfile->GetTransportType(), "RTP/AVP");
    EXPECT_FALSE(pProfile->IsAvpfSupported());
    EXPECT_FALSE(pProfile->IsCapaNegoForAvpfSupported());
}

/**
 * @brief Verifies that the transport type is correctly set to RTP/AVPF when AVPF is enabled.
 */
TEST_F(VideoProfileGeneratorTest, SetProfileAvpfEnabled)
{
    // Arrange
    EXPECT_CALL(*m_pConfig, IsVideoAvpfEnabled()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*m_pConfig, IsAvpfCapabilityNegotiationEnabled()).WillRepeatedly(Return(IMS_FALSE));

    // Act
    auto pBaseProfile = m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(),
            m_pConfig.get(), VideoProfileGeneratorTest::SLOT_ID);
    auto pProfile = std::static_pointer_cast<VideoProfile>(pBaseProfile);

    // Assert
    EXPECT_TRUE(pProfile->IsAvpfSupported());
    EXPECT_EQ(pProfile->GetTransportType(), "RTP/AVPF");
    EXPECT_FALSE(pProfile->IsCapaNegoForAvpfSupported());
}

/**
 * @brief Verifies capability negotiation attributes are correctly generated for AVPF
 * when the 'without acap' option is used.
 */
TEST_F(VideoProfileGeneratorTest, SetProfileAvpfWithCapaNegoNoAcap)
{
    // Arrange
    EXPECT_CALL(*m_pConfig, IsVideoAvpfEnabled()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*m_pConfig, IsAvpfCapabilityNegotiationEnabled()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*m_pConfig, GetSdpOfferCapNegoForAvpf())
            .WillRepeatedly(Return(MediaConfiguration::CAPNEG_OFFER_WITHOUT_ACAP));

    // Act
    auto pBaseProfile = m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(),
            m_pConfig.get(), VideoProfileGeneratorTest::SLOT_ID);
    auto pProfile = std::static_pointer_cast<VideoProfile>(pBaseProfile);

    // Assert
    EXPECT_TRUE(pProfile->IsAvpfSupported());
    EXPECT_EQ(pProfile->GetTransportType(), "RTP/AVP");  // Base transport is AVP
    EXPECT_TRUE(pProfile->IsCapaNegoForAvpfSupported());
    EXPECT_EQ(pProfile->GetCapaNego().GetMapTcap().GetSize(), 1);
    EXPECT_EQ(pProfile->GetCapaNego().GetMapTcap().GetValue(1), "RTP/AVPF");
    EXPECT_EQ(pProfile->GetCapaNego().GetListPcfg().GetSize(), 1);
    EXPECT_EQ(pProfile->GetCapaNego().GetListPcfg().GetAt(0), "t=1");
    EXPECT_FALSE(pProfile->GetCapaNego().IsAttCapaInPcfg());
}

/**
 * @brief Verifies capability negotiation attributes are correctly generated for AVPF
 * when the 'with acap' option is used, including feedback mechanisms.
 */
TEST_F(VideoProfileGeneratorTest, SetProfileAvpfWithCapaNegoWithAcap)
{
    // Arrange
    EXPECT_CALL(*m_pConfig, IsVideoAvpfEnabled()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*m_pConfig, IsAvpfCapabilityNegotiationEnabled()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*m_pConfig, GetSdpOfferCapNegoForAvpf())
            .WillRepeatedly(Return(MediaConfiguration::CAPNEG_OFFER_WITH_ACAP));

    // Enable some feedback mechanisms to generate acap attributes
    EXPECT_CALL(*m_pConfig, IsVideoAvpfNackEnabled()).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(*m_pConfig, IsVideoAvpfPliEnabled()).WillRepeatedly(Return(IMS_TRUE));

    // Act
    auto pBaseProfile = m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(),
            m_pConfig.get(), VideoProfileGeneratorTest::SLOT_ID);
    auto pProfile = std::static_pointer_cast<VideoProfile>(pBaseProfile);

    // Assert
    EXPECT_TRUE(pProfile->IsAvpfSupported());
    EXPECT_EQ(pProfile->GetTransportType(), "RTP/AVP");  // Base transport is AVP
    EXPECT_TRUE(pProfile->IsCapaNegoForAvpfSupported());

    // Check tcap
    EXPECT_EQ(pProfile->GetCapaNego().GetMapTcap().GetSize(), 1);
    EXPECT_EQ(pProfile->GetCapaNego().GetMapTcap().GetValue(1), "RTP/AVPF");

    // Check acap
    auto& acapMap = pProfile->GetCapaNego().GetMapAcap();
    EXPECT_EQ(acapMap.GetSize(), 2);
    EXPECT_EQ(acapMap.GetValue(1), "rtcp-fb:* nack");
    EXPECT_EQ(acapMap.GetValue(2), "rtcp-fb:* nack pli");

    // Check pcfg
    EXPECT_EQ(pProfile->GetCapaNego().GetListPcfg().GetSize(), 1);
    EXPECT_EQ(pProfile->GetCapaNego().GetListPcfg().GetAt(0), "t=1 a=1,2");
    EXPECT_TRUE(pProfile->GetCapaNego().IsAttCapaInPcfg());
}

/**
 * @brief Verifies that an AVC codec payload is correctly generated and added to the profile.
 */
TEST_F(VideoProfileGeneratorTest, CreateAvcPayload)
{
    // Arrange
    const IMS_SINT32 kPayloadNum = 97;
    const IMS_SINT32 kFrameRate = 30;
    const IMS_SINT32 kBitRate = 512000;
    const AString kProfileLevelId("42e01f");  // Baseline, Level 3.1
    const AString kStrSpropParameterSets("Z0LAFtoHgUaagQEBA8UKqA==,aM4NiA==");
    const AString kImageAttr("send [x=640,y=480] recv [x=640,y=480]");
    const AString kFrameSize("640x480");

    const auto pAvcConfig =
            std::make_unique<NiceMock<MockCodecAvcConfig>>(ImsCodec::VIDEO_AVC, kPayloadNum);
    ON_CALL(*pAvcConfig, GetCodec()).WillByDefault(Return(ImsCodec::VIDEO_AVC));
    ON_CALL(*pAvcConfig, GetPayloadType()).WillByDefault(Return(kPayloadNum));
    ON_CALL(*pAvcConfig, GetFramerate()).WillByDefault(Return(kFrameRate));
    ON_CALL(*pAvcConfig, GetResolutionWidth()).WillByDefault(Return(640));
    ON_CALL(*pAvcConfig, GetResolutionHeight()).WillByDefault(Return(480));
    ON_CALL(*pAvcConfig, GetBitrate()).WillByDefault(Return(kBitRate));
    ON_CALL(*pAvcConfig, GetProfileLevelId()).WillByDefault(testing::ReturnRef(kProfileLevelId));
    ON_CALL(*pAvcConfig, GetPacketizationMode()).WillByDefault(Return(1));
    ON_CALL(*pAvcConfig, GetIncludeSpropParameterSets()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pAvcConfig, GetSpropParameterSets()).WillByDefault(ReturnRef(kStrSpropParameterSets));
    ON_CALL(*pAvcConfig, GetImageAttr()).WillByDefault(ReturnRef(kImageAttr));
    ON_CALL(*pAvcConfig, GetFrameSize()).WillByDefault(ReturnRef(kFrameSize));

    m_lstCodecConfig.Append(pAvcConfig.get());
    ON_CALL(*m_pConfig, GetCodecConfigs()).WillByDefault(testing::ReturnRef(m_lstCodecConfig));
    EXPECT_CALL(*m_pConfig, GetVideoSamplingRate()).WillRepeatedly(Return(90000));

    // Act
    auto pBaseProfile = m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(),
            m_pConfig.get(), VideoProfileGeneratorTest::SLOT_ID);
    auto pProfile = std::static_pointer_cast<VideoProfile>(pBaseProfile);

    // Assert
    ASSERT_EQ(pProfile->GetPayloadListSize(), 1);
    VideoProfile::Payload* pPayload = pProfile->GetPayloadAt(0);
    ASSERT_NE(pPayload, nullptr);

    // Check RtpMap
    EXPECT_EQ(pPayload->GetRtpMap().GetPayloadNumber(), kPayloadNum);
    EXPECT_TRUE(pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"));
    EXPECT_EQ(pPayload->GetRtpMap().GetSamplingRate(), 90000);
    EXPECT_EQ(pPayload->GetImageAttr(), kImageAttr);
    EXPECT_TRUE(pPayload->IsImageAttrIncluded());
    EXPECT_FALSE(pPayload->IsFrameSizeIncluded());  // false if there is an image attributes

    // Check Fmtp
    auto pFmtp = std::static_pointer_cast<VideoProfile::AvcFmtp>(pPayload->GetFmtp());
    ASSERT_NE(pFmtp, nullptr);
    EXPECT_EQ(pFmtp->GetFramerate(), kFrameRate);
    EXPECT_EQ(pFmtp->GetResolution(), VIDEO_RESOLUTION_VGA_LS);
    EXPECT_EQ(pFmtp->GetBitrate(), kBitRate);
    EXPECT_EQ(pFmtp->GetProfileLevelId(), "42e01f");
    EXPECT_EQ(pFmtp->GetPacketizationMode(), 1);
    EXPECT_EQ(pFmtp->GetProfile(), AVC_PROFILE_CB);
    EXPECT_EQ(pFmtp->GetLevel(), 31);
    EXPECT_EQ(pFmtp->GetSpropParam(), kStrSpropParameterSets);
    EXPECT_TRUE(pFmtp->IsSpropParamVisible());

    // Check max framerate on profile
    EXPECT_EQ(pProfile->GetFrameRate(), kFrameRate);

    m_lstCodecConfig.Clear();
}

/**
 * @brief Verifies that an HEVC codec payload is correctly generated and added to the profile.
 */
TEST_F(VideoProfileGeneratorTest, CreateHevcPayload)
{
    // Arrange
    const IMS_SINT32 kPayloadNum = 98;
    const IMS_SINT32 kFrameRate = 30;
    const IMS_SINT32 kBitRate = 512000;
    const AString kStrSpropParameterSets(
            "AAAAAUABDAH//"
            "wFgAAADALAAAAMAAAMAWqxZ,AAAAAUIBAQFgAAADALAAAAMAAAMAWqAPCAKBZa7kyS7gC7QoSg==,"
            "AAAAAUQBwPPAAhA=");
    const AString kImageAttr("send [x=640,y=480] recv [x=640,y=480]");
    const AString kFrameSize("640x480");

    const auto pHevcConfig =
            std::make_unique<NiceMock<MockCodecHevcConfig>>(ImsCodec::VIDEO_HEVC, kPayloadNum);
    ON_CALL(*pHevcConfig, GetCodec()).WillByDefault(Return(ImsCodec::VIDEO_HEVC));
    ON_CALL(*pHevcConfig, GetPayloadType()).WillByDefault(Return(kPayloadNum));
    ON_CALL(*pHevcConfig, GetFramerate()).WillByDefault(Return(kFrameRate));
    ON_CALL(*pHevcConfig, GetResolutionWidth()).WillByDefault(Return(1280));
    ON_CALL(*pHevcConfig, GetResolutionHeight()).WillByDefault(Return(720));
    ON_CALL(*pHevcConfig, GetBitrate()).WillByDefault(Return(kBitRate));
    ON_CALL(*pHevcConfig, GetHevcProfile()).WillByDefault(Return(HEVC_PROFILE_MAIN));
    ON_CALL(*pHevcConfig, GetHevcLevel()).WillByDefault(Return(120));  // Level 4.0
    ON_CALL(*pHevcConfig, GetPacketizationMode()).WillByDefault(Return(1));
    ON_CALL(*pHevcConfig, GetSpropParameterSets()).WillByDefault(ReturnRef(kStrSpropParameterSets));
    ON_CALL(*pHevcConfig, GetImageAttr()).WillByDefault(ReturnRef(kImageAttr));
    ON_CALL(*pHevcConfig, GetFrameSize()).WillByDefault(ReturnRef(kFrameSize));

    m_lstCodecConfig.Append(pHevcConfig.get());
    ON_CALL(*m_pConfig, GetCodecConfigs()).WillByDefault(testing::ReturnRef(m_lstCodecConfig));
    EXPECT_CALL(*m_pConfig, GetVideoSamplingRate()).WillRepeatedly(Return(90000));

    // Act
    auto pBaseProfile = m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(),
            m_pConfig.get(), VideoProfileGeneratorTest::SLOT_ID);
    auto pProfile = std::static_pointer_cast<VideoProfile>(pBaseProfile);

    // Assert
    ASSERT_EQ(pProfile->GetPayloadListSize(), 1);
    VideoProfile::Payload* pPayload = pProfile->GetPayloadAt(0);
    ASSERT_NE(pPayload, nullptr);

    // Check RtpMap
    EXPECT_EQ(pPayload->GetRtpMap().GetPayloadNumber(), kPayloadNum);
    EXPECT_TRUE(pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H265"));
    EXPECT_EQ(pPayload->GetRtpMap().GetSamplingRate(), 90000);
    EXPECT_EQ(pPayload->GetImageAttr(), kImageAttr);
    EXPECT_TRUE(pPayload->IsImageAttrIncluded());
    EXPECT_FALSE(pPayload->IsFrameSizeIncluded());  // false if there is an image attributes

    // Check Fmtp
    auto pFmtp = std::static_pointer_cast<VideoProfile::HevcFmtp>(pPayload->GetFmtp());
    ASSERT_NE(pFmtp, nullptr);
    EXPECT_EQ(pFmtp->GetFramerate(), kFrameRate);
    EXPECT_EQ(pFmtp->GetResolution(), VIDEO_RESOLUTION_HD_LS);
    EXPECT_EQ(pFmtp->GetBitrate(), kBitRate);
    EXPECT_EQ(pFmtp->GetPacketizationMode(), 1);
    EXPECT_EQ(pFmtp->GetProfile(), HEVC_PROFILE_MAIN);
    EXPECT_EQ(pFmtp->GetLevel(), 120);
    EXPECT_EQ(pFmtp->GetSpropParam(), kStrSpropParameterSets);
    EXPECT_TRUE(pFmtp->IsSpropParamVisible());

    // Check max framerate on profile
    EXPECT_EQ(pProfile->GetFrameRate(), kFrameRate);

    m_lstCodecConfig.Clear();
}
