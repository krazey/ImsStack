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
#include "text/TextProfile.h"
#include "text/TextProfileGenerator.h"

#include "MockICoreService.h"
#include "MockMediaProfileFactory.h"
#include "config/MockTextConfiguration.h"
#include "config/MockCodecT140Config.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

/**
 * @brief Test fixture for the TextProfileGenerator class.
 *
 * Sets up a common environment for testing the TextProfileGenerator, including
 * the generator itself and mocks for its dependencies.
 */
class TextProfileGeneratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_pGenerator = std::make_unique<TextProfileGenerator>();
        m_pConfig = std::make_unique<NiceMock<MockTextConfiguration>>();
        m_pProfile = std::make_shared<TextProfile>();
        m_pIService = std::make_unique<NiceMock<MockICoreService>>();
        ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pProfile));
        ON_CALL(*m_pConfig, GetCodecConfigs()).WillByDefault(ReturnRef(m_lstCodecConfig));
        MockMediaProfileFactory::SetInstance(&m_objMediaProfileFactory);

        // --- Set up default mock expectations for MediaProfileGenerator part ---
        ON_CALL(*m_pConfig, GetAsBandwidthKbps()).WillByDefault(Return(10));
        ON_CALL(*m_pConfig, GetRsBandwidthBps()).WillByDefault(Return(1000));
        ON_CALL(*m_pConfig, GetRrBandwidthBps()).WillByDefault(Return(1000));
        ON_CALL(*m_pConfig, GetRtcpIntervalOnActive()).WillByDefault(Return(5));
        ON_CALL(*m_pConfig, GetRtcpIntervalOnHold()).WillByDefault(Return(50));

        m_objIpAddr = IpAddress(LOCAL_IP);
        ON_CALL(*m_pIService.get(), GetIpAddress()).WillByDefault(ReturnRef(m_objIpAddr));
    }

    virtual void TearDown() override { MockMediaProfileFactory::SetInstance(IMS_NULL); }

    MockMediaProfileFactory m_objMediaProfileFactory;
    IpAddress m_objIpAddr;
    ImsList<CodecConfig*> m_lstCodecConfig;
    std::unique_ptr<TextProfileGenerator> m_pGenerator;
    std::shared_ptr<TextProfile> m_pProfile;
    std::unique_ptr<NiceMock<MockTextConfiguration>> m_pConfig;
    std::unique_ptr<NiceMock<MockICoreService>> m_pIService;
    const IMS_SINT32 SLOT_ID = 0;
    const AString LOCAL_IP = "127.0.0.1";
};

/**
 * @brief Verifies that a basic text profile is generated with correct common attributes.
 */
TEST_F(TextProfileGeneratorTest, SetProfileBasic)
{
    // Act
    auto pBaseProfile = m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(),
            m_pConfig.get(), TextProfileGeneratorTest::SLOT_ID);
    auto pProfile = std::static_pointer_cast<TextProfile>(pBaseProfile);

    // Assert
    EXPECT_EQ(pProfile->GetBandwidthAs(), 10);
    EXPECT_EQ(pProfile->GetBandwidthRs(), 1000);
    EXPECT_EQ(pProfile->GetBandwidthRr(), 1000);
    EXPECT_EQ(pProfile->GetRtcpInterval(), 50);
    EXPECT_EQ(pProfile->GetTransportType(), "RTP/AVP");
}

/**
 * @brief Verifies that a T.140 codec payload is correctly generated and added to the profile.
 */
TEST_F(TextProfileGeneratorTest, CreateT140Payload)
{
    // Arrange
    const IMS_SINT32 kT140PayloadType = 106;
    auto pT140Config =
            std::make_unique<NiceMock<MockCodecT140Config>>(ImsCodec::TEXT_T140, kT140PayloadType);
    ON_CALL(*pT140Config, GetCodec()).WillByDefault(Return(ImsCodec::TEXT_T140));
    ON_CALL(*pT140Config, GetPayloadType()).WillByDefault(Return(kT140PayloadType));
    ON_CALL(*pT140Config, GetSamplingRate()).WillByDefault(Return(1000));
    ON_CALL(*pT140Config, GetRedLevel()).WillByDefault(Return(0));

    ImsList<CodecConfig*> codecConfigs;
    codecConfigs.Append(pT140Config.get());
    ON_CALL(*m_pConfig, GetCodecConfigs()).WillByDefault(testing::ReturnRef(codecConfigs));

    // Act
    auto pBaseProfile = m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(),
            m_pConfig.get(), TextProfileGeneratorTest::SLOT_ID);
    auto pProfile = std::static_pointer_cast<TextProfile>(pBaseProfile);

    // Assert
    ASSERT_EQ(pProfile->GetPayloadListSize(), 1);
    TextProfile::Payload* pPayload = pProfile->GetPayloadAt(0);
    ASSERT_NE(pPayload, nullptr);

    // Check RtpMap
    EXPECT_EQ(pPayload->GetRtpMap().GetPayloadNumber(), kT140PayloadType);
    EXPECT_TRUE(pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140"));
    EXPECT_EQ(pPayload->GetRtpMap().GetSamplingRate(), 1000);

    auto pFmtp = std::static_pointer_cast<TextProfile::T140Fmtp>(pPayload->GetFmtp());
    // Check Fmtp
    ASSERT_NE(pFmtp, nullptr);
    EXPECT_EQ(pFmtp->GetCps(), 30);
}

/**
 * @brief Verifies that a T.140 payload with redundancy (RED) is correctly generated.
 */
TEST_F(TextProfileGeneratorTest, CreateRedPayload)
{
    // Arrange
    const IMS_SINT32 kT140PayloadType = 106;
    const IMS_SINT32 kRedPayloadType = 107;
    const IMS_SINT32 kRedLevel = 2;

    auto pRedConfig =
            std::make_unique<NiceMock<MockCodecT140Config>>(ImsCodec::TEXT_RED, kRedPayloadType);
    ON_CALL(*pRedConfig, GetCodec()).WillByDefault(Return(ImsCodec::TEXT_RED));
    ON_CALL(*pRedConfig, GetPayloadType()).WillByDefault(Return(kRedPayloadType));
    ON_CALL(*pRedConfig, GetSamplingRate()).WillByDefault(Return(1000));
    ON_CALL(*pRedConfig, GetRedLevel()).WillByDefault(Return(kRedLevel));

    ON_CALL(*m_pConfig, GetT140PayloadType()).WillByDefault(Return(kT140PayloadType));

    ImsList<CodecConfig*> codecConfigs;
    codecConfigs.Append(pRedConfig.get());
    ON_CALL(*m_pConfig, GetCodecConfigs()).WillByDefault(testing::ReturnRef(codecConfigs));

    // Act
    auto pBaseProfile = m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(),
            m_pConfig.get(), TextProfileGeneratorTest::SLOT_ID);
    auto pProfile = std::static_pointer_cast<TextProfile>(pBaseProfile);

    // Assert
    ASSERT_EQ(pProfile->GetPayloadListSize(), 1);
    TextProfile::Payload* pPayload = pProfile->GetPayloadAt(0);
    ASSERT_NE(pPayload, nullptr);

    // Check RtpMap for RED payload
    EXPECT_EQ(pPayload->GetRtpMap().GetPayloadNumber(), kRedPayloadType);
    EXPECT_TRUE(pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"));

    // Check Fmtp for RED payload
    auto pRedFmtp = std::static_pointer_cast<TextProfile::RedFmtp>(pPayload->GetFmtp());
    ASSERT_NE(pRedFmtp, nullptr);
    EXPECT_EQ(pRedFmtp->GetRedLevel(), kRedLevel);
    EXPECT_EQ(pRedFmtp->GetRedPayload(), kT140PayloadType);
}
