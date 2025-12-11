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

#include "config/ImsCodec.h"
#include "gmock/gmock.h"
#include "media/audio/AudioProfile.h"
#include "media/audio/AudioProfileGenerator.h"

#include "MockICoreService.h"
#include "MockMediaProfileFactory.h"
#include "config/MockAudioConfiguration.h"
#include "config/MockCodecAmrConfig.h"
#include "config/MockCodecConfig.h"
#include "config/MockCodecEvsConfig.h"
#include "config/MockCodecTelephoneEventConfig.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

const AString LOCAL_IP = "127.0.0.1";

class AudioProfileGeneratorTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        m_pGenerator = std::make_unique<AudioProfileGenerator>();
        m_pProfile = std::make_shared<AudioProfile>();
        m_pConfig = std::make_unique<NiceMock<MockAudioConfiguration>>();
        m_pIService = std::make_unique<NiceMock<MockICoreService>>();

        MockMediaProfileFactory::SetInstance(&m_objMediaProfileFactory);

        ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pProfile));
        ON_CALL(*m_pConfig, GetCodecConfigs()).WillByDefault(ReturnRef(m_lstCodecConfig));
        ON_CALL(*m_pConfig, GetAudioCandidateAttribute())
                .WillByDefault(ReturnRef(m_objCandidateAttr));

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

protected:
    MockMediaProfileFactory m_objMediaProfileFactory;
    IpAddress m_objIpAddr;
    ImsList<CodecConfig*> m_lstCodecConfig;
    ImsVector<AString> m_objCandidateAttr;
    std::unique_ptr<AudioProfileGenerator> m_pGenerator;
    std::shared_ptr<AudioProfile> m_pProfile;
    std::unique_ptr<NiceMock<MockAudioConfiguration>> m_pConfig;
    std::unique_ptr<NiceMock<MockICoreService>> m_pIService;
};

/**
 * @brief Verifies that a basic video profile is generated correctly
 */
TEST_F(AudioProfileGeneratorTest, SetProfileBasic)
{
    // Act
    auto pBaseProfile =
            m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(), m_pConfig.get(), 0);
    auto pProfile = std::static_pointer_cast<AudioProfile>(pBaseProfile);

    // Assert
    EXPECT_EQ(pProfile->GetBandwidthAs(), 10);
    EXPECT_EQ(pProfile->GetBandwidthRs(), 1000);
    EXPECT_EQ(pProfile->GetBandwidthRr(), 1000);
    EXPECT_EQ(pProfile->GetRtcpInterval(), 50);
}

TEST_F(AudioProfileGeneratorTest, CreateAmrPayloadTest)
{
    // Create a mock CodecAmrConfig locally for this test
    auto pAmrConfig = std::make_unique<NiceMock<MockCodecAmrConfig>>(ImsCodec::AUDIO_AMR, 97);
    ON_CALL(*pAmrConfig, GetCodec()).WillByDefault(Return(ImsCodec::AUDIO_AMR));
    ON_CALL(*pAmrConfig, GetOctetAlign()).WillByDefault(Return(1));  // Octet aligned
    ON_CALL(*pAmrConfig, GetModeSetList()).WillByDefault(Return(0b10101010));
    ON_CALL(*pAmrConfig, GetDtx()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pAmrConfig, GetPayloadType()).WillByDefault(Return(97));
    ON_CALL(*pAmrConfig, GetSamplingRate()).WillByDefault(Return(8000));
    ON_CALL(*pAmrConfig, GetChannel()).WillByDefault(Return(1));

    m_lstCodecConfig.Append(pAmrConfig.get());
    ON_CALL(*m_pConfig, GetCodecConfigs()).WillByDefault(ReturnRef(m_lstCodecConfig));
    ON_CALL(*m_pConfig, GetMaxRed()).WillByDefault(Return(100));

    // Act: Generate the full profile. The generator will internally call CreateAmrPayload.
    auto pBaseProfile =
            m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(), m_pConfig.get(), 0);
    auto pProfile = std::static_pointer_cast<AudioProfile>(pBaseProfile);

    // Assert
    ASSERT_EQ(pProfile->GetPayloadListSize(), 1);
    AudioProfile::Payload* pPayload = pProfile->GetPayloadAt(0);
    ASSERT_NE(pPayload, nullptr);
    // Verify RtpMap
    EXPECT_EQ(pPayload->GetRtpMap().GetPayloadNumber(), 97);
    EXPECT_TRUE(pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR"));
    EXPECT_EQ(pPayload->GetRtpMap().GetSamplingRate(), 8000);
    EXPECT_EQ(pPayload->GetRtpMap().GetChannel(), 1);

    // Verify Fmtp
    auto pFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(pPayload->GetFmtp());
    ASSERT_NE(pFmtp, nullptr);
    EXPECT_EQ(pFmtp->GetOctetAlign(), 1);
    EXPECT_EQ(pFmtp->GetModeSetList(), 0b10101010);
    EXPECT_TRUE(pFmtp->IsDtxEnabled());
    EXPECT_EQ(pFmtp->GetMaxRed(), 100);

    m_lstCodecConfig.Clear();
}

TEST_F(AudioProfileGeneratorTest, CreateEvsPayloadTest)
{
    // Create a mock CodecEvsConfig locally for this test
    auto pEvsConfig = std::make_unique<NiceMock<MockCodecEvsConfig>>(ImsCodec::AUDIO_EVS, 98);
    ON_CALL(*pEvsConfig, GetCodec()).WillByDefault(Return(ImsCodec::AUDIO_EVS));
    ON_CALL(*pEvsConfig, GetModeSetList()).WillByDefault(Return(0b10101010));
    ON_CALL(*pEvsConfig, GetBrList())
            .WillByDefault(Return(0b000000000001));  // EVS_PRIMARY_MODE_BITRATE_5_9_KBPS
    ON_CALL(*pEvsConfig, GetBwList()).WillByDefault(Return(0b000000000011));  // NB | WB
    ON_CALL(*pEvsConfig, GetChAwareRecv()).WillByDefault(Return(2));
    ON_CALL(*pEvsConfig, GetDtx()).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pEvsConfig, GetPayloadType()).WillByDefault(Return(98));
    ON_CALL(*pEvsConfig, GetChannel()).WillByDefault(Return(1));
    ON_CALL(*pEvsConfig, GetEvsModeSwitch()).WillByDefault(Return(0));  // Primary mode

    m_lstCodecConfig.Append(pEvsConfig.get());
    ON_CALL(*m_pConfig, GetCodecConfigs()).WillByDefault(ReturnRef(m_lstCodecConfig));
    ON_CALL(*m_pConfig, GetMaxRed()).WillByDefault(Return(100));

    // Act: Generate the full profile. The generator will internally call CreateEvsPayload.
    auto pBaseProfile =
            m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(), m_pConfig.get(), 0);
    auto pProfile = std::static_pointer_cast<AudioProfile>(pBaseProfile);

    // Assert
    ASSERT_EQ(pProfile->GetPayloadListSize(), 1);
    AudioProfile::Payload* pPayload = pProfile->GetPayloadAt(0);
    // Verify RtpMap
    EXPECT_EQ(pPayload->GetRtpMap().GetPayloadNumber(), 98);
    EXPECT_TRUE(pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"));
    EXPECT_EQ(pPayload->GetRtpMap().GetSamplingRate(), 16000);
    EXPECT_EQ(pPayload->GetRtpMap().GetChannel(), 1);

    // Verify Fmtp
    auto pFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pPayload->GetFmtp());
    ASSERT_NE(pFmtp, nullptr);
    EXPECT_EQ(pFmtp->GetModeSetList(), 0b10101010);
    EXPECT_EQ(pFmtp->GetBrList(), 0b000000000001);
    EXPECT_EQ(pFmtp->GetBwList(), 0b000000000011);
    EXPECT_EQ(pFmtp->GetChAwRecv(), 2);
    EXPECT_TRUE(pFmtp->IsDtxEnabled());
    EXPECT_EQ(pFmtp->GetMaxRed(), 100);

    m_lstCodecConfig.Clear();
}

TEST_F(AudioProfileGeneratorTest, CreateTelephoneEventPayloadTest)
{
    const AString kEvents("0-15");

    // Create a mock CodecTelephoneEventConfig locally for this test
    auto pTelEventConfig = std::make_unique<NiceMock<MockCodecTelephoneEventConfig>>(
            ImsCodec::AUDIO_TELEPHONE_EVENT, 101);
    ON_CALL(*pTelEventConfig, GetPayloadType()).WillByDefault(Return(101));
    ON_CALL(*pTelEventConfig, GetCodec()).WillByDefault(Return(ImsCodec::AUDIO_TELEPHONE_EVENT));
    ON_CALL(*pTelEventConfig, GetSamplingRate()).WillByDefault(Return(8000));
    ON_CALL(*pTelEventConfig, GetEvents()).WillByDefault(ReturnRef(kEvents));

    m_lstCodecConfig.Append(pTelEventConfig.get());
    ON_CALL(*m_pConfig, GetCodecConfigs()).WillByDefault(ReturnRef(m_lstCodecConfig));

    // Act: Generate the full profile. The generator will internally call
    // CreateTelephoneEventPayload.
    auto pBaseProfile =
            m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(), m_pConfig.get(), 0);
    auto pProfile = std::static_pointer_cast<AudioProfile>(pBaseProfile);

    // Assert
    ASSERT_EQ(pProfile->GetPayloadListSize(), 1);
    AudioProfile::Payload* pPayload = pProfile->GetPayloadAt(0);

    // Verify RtpMap
    EXPECT_EQ(pPayload->GetRtpMap().GetPayloadNumber(), 101);
    EXPECT_TRUE(pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"));
    EXPECT_EQ(pPayload->GetRtpMap().GetSamplingRate(), 8000);

    // Verify Fmtp
    auto pFmtp = std::static_pointer_cast<AudioProfile::TelephoneEventFmtp>(pPayload->GetFmtp());
    ASSERT_NE(pFmtp, nullptr);
    EXPECT_EQ(pFmtp->GetEvents(), "0-15");

    m_lstCodecConfig.Clear();
}

TEST_F(AudioProfileGeneratorTest, CreatePcmPayloadTest)
{
    // Create a mock CodecConfig for PCM locally for this test
    auto pPcmConfig = std::make_unique<NiceMock<MockCodecConfig>>(ImsCodec::AUDIO_PCMA, 8);
    ON_CALL(*pPcmConfig, GetCodec()).WillByDefault(Return(ImsCodec::AUDIO_PCMA));
    ON_CALL(*pPcmConfig, GetPayloadType()).WillByDefault(Return(8));  // Default PCMA pPayload type

    m_lstCodecConfig.Append(pPcmConfig.get());

    auto pBaseProfile =
            m_pGenerator->Generate(MEDIA_SERVICE_DEFAULT, m_pIService.get(), m_pConfig.get(), 0);
    auto pProfile = std::static_pointer_cast<AudioProfile>(pBaseProfile);

    // Assert
    ASSERT_EQ(pProfile->GetPayloadListSize(), 1);
    AudioProfile::Payload* pPayload = pProfile->GetPayloadAt(0);

    // Verify RtpMap
    EXPECT_EQ(pPayload->GetRtpMap().GetPayloadNumber(), 8);
    EXPECT_TRUE(pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMA"));
    EXPECT_EQ(pPayload->GetRtpMap().GetSamplingRate(), 8000);  // Default for PCMA

    m_lstCodecConfig.Clear();
}
