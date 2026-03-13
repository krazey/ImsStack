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
#include "video/VideoDef.h"
#include "video/VideoProfileNegotiator.h"
#include "video/VideoProfile.h"
#include "video/VideoProfileUtil.h"

// mocking
#include "config/MockVideoConfiguration.h"

using ::testing::_;
using ::testing::NiceMock;
using ::testing::Return;

const AString kAvcProfileID = "42e01f";
const IMS_UINT32 kLocalPayload = 97;
const IMS_UINT32 kPeerPayload = 100;

/**
 * @brief Test fixture for the VideoProfileNegotiator class.
 *
 * This class sets up a common environment for testing video profile negotiation logic.
 * It initializes local, peer, and negotiated VideoProfile objects, along with a
 * mock VideoConfiguration. Helper methods are provided to easily create and add
 * AVC and HEVC payloads to the profiles for various test scenarios.
 */
class VideoProfileNegotiatorTest : public ::testing::Test
{
protected:
    // SetUp is called before each test case
    void SetUp() override
    {
        // Initialize objects before each test
        m_pLocalProfile = std::make_unique<VideoProfile>();
        m_pPeerProfile = std::make_unique<VideoProfile>();
        m_pNegotiatedProfile = std::make_unique<VideoProfile>();
        // Use the MockVideoConfiguration, wrapped in NiceMock to avoid warnings
        // for uninteresting calls.
        m_pConfig = std::make_unique<NiceMock<MockVideoConfiguration>>();
        m_pNegotiator = std::make_unique<VideoProfileNegotiator>();

        // --- Basic Configuration Setup (Customize as needed for tests) ---

        // Example: Set default directions
        m_pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
        m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);

        // Example: Set default ports (non-zero to avoid immediate invalidation)
        m_pLocalProfile->SetDataPort(5004);
        m_pLocalProfile->SetControlPort(5005);
        m_pPeerProfile->SetDataPort(6004);
        m_pPeerProfile->SetControlPort(6005);

        // Example: Set default bandwidths
        m_pLocalProfile->SetBandwidthAs(1000);
        m_pLocalProfile->SetBandwidthRs(150);
        m_pLocalProfile->SetBandwidthRr(150);
        m_pPeerProfile->SetBandwidthAs(1200);
        m_pPeerProfile->SetBandwidthRs(200);
        m_pPeerProfile->SetBandwidthRr(200);

        // --- Set up default mock expectations ---
        // The negotiator will *get* values from the config.
        // Use WillRepeatedly if the value might be checked multiple times.
        ON_CALL(*m_pConfig, GetAsBandwidthKbps())
                .WillByDefault(Return(1000));  // Example default config value
        ON_CALL(*m_pConfig, GetRsBandwidthBps()).WillByDefault(Return(150));
        ON_CALL(*m_pConfig, GetRrBandwidthBps()).WillByDefault(Return(150));
        ON_CALL(*m_pConfig, GetRtcpIntervalOnActive()).WillByDefault(Return(5));
        ON_CALL(*m_pConfig, GetRtcpIntervalOnHold()).WillByDefault(Return(5));

        // Assume AVPF is generally supported by config unless overridden in a test
        ON_CALL(*m_pConfig, IsVideoAvpfEnabled()).WillByDefault(Return(IMS_TRUE));
        // Assume specific FB types are supported by config unless overridden
    }

    // TearDown is called after each test case
    void TearDown() override
    {
        // Clean up resources if necessary (smart pointers handle memory)
    }

    // Helper function to add a basic AVC pPayload (customize as needed)
    VideoProfile::Payload* AddAvcPayload(VideoProfile* pProfile, IMS_UINT32 nPayloadNum,
            VIDEO_RESOLUTION eResolution = VIDEO_RESOLUTION_VGA_LS, IMS_UINT32 nLevel = 31,
            const AString& strProfileLevelId = kAvcProfileID)
    {
        if (!pProfile)
            return nullptr;

        auto* pPayload = new VideoProfile::Payload();
        pPayload->SetRtpMap(nPayloadNum, "H264", 90000);

        auto pFmtp = std::make_shared<VideoProfile::AvcFmtp>();
        pFmtp->SetResolution(eResolution);
        pFmtp->SetLevel(nLevel);
        pFmtp->SetProfileLevelId(strProfileLevelId);
        pFmtp->SetProfile(VideoProfileUtil::GetAvcProfileFromProfileLevelId(strProfileLevelId));
        // Set other pFmtp properties as needed (bitrate, framerate, packetization, sprop)
        pFmtp->SetPacketizationMode(1);
        pFmtp->SetVisiblePacketizationMode(IMS_TRUE);
        pFmtp->SetVisibleProfileLevelId(IMS_TRUE);

        pPayload->SetFmtp(pFmtp);
        pProfile->AddPayload(pPayload);
        return pPayload;
    }

    // Helper function to add a basic HEVC pPayload (customize as needed)
    VideoProfile::Payload* AddHevcPayload(VideoProfile* pProfile, IMS_UINT32 nPayloadNum,
            VIDEO_RESOLUTION eResolution = VIDEO_RESOLUTION_VGA_LS,
            IMS_UINT32 nLevel = 30,  // nLevel-id is typically 30*3 = 90 in SDP
            VIDEO_PROFILE_HEVC eHevcProfile = HEVC_PROFILE_MAIN)
    {
        if (!pProfile)
            return nullptr;

        auto* pPayload = new VideoProfile::Payload();
        pPayload->SetRtpMap(nPayloadNum, "H265", 90000);

        auto pFmtp = std::make_shared<VideoProfile::HevcFmtp>();
        pFmtp->SetResolution(eResolution);
        pFmtp->SetLevel(nLevel * 3);  // Store the SDP nLevel-id value
        pFmtp->SetProfile(eHevcProfile);
        // Set other pFmtp properties as needed (bitrate, framerate, packetization, sprop)
        pFmtp->SetPacketizationMode(1);
        pFmtp->SetVisiblePacketizationMode(IMS_TRUE);
        pFmtp->SetVisibleProfile(IMS_TRUE);
        pFmtp->SetVisibleLevel(IMS_TRUE);

        pPayload->SetFmtp(pFmtp);
        pProfile->AddPayload(pPayload);
        return pPayload;
    }

    // Member variables accessible by tests
    std::unique_ptr<VideoProfileNegotiator> m_pNegotiator;
    std::unique_ptr<VideoProfile> m_pLocalProfile;
    std::unique_ptr<VideoProfile> m_pPeerProfile;
    std::unique_ptr<VideoProfile> m_pNegotiatedProfile;
    // Use the mock configuration type
    std::unique_ptr<NiceMock<MockVideoConfiguration>> m_pConfig;
};

// --- Test Cases ---

TEST_F(VideoProfileNegotiatorTest, NegotiateBasicSuccess)
{
    // Arrange: Add compatible payloads
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 31,
            "42e01f");  // Level 3.1
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_VGA_LS, 31,
            "42e01f");  // Level 3.1

    // Act: Perform negotiation (Offer received scenario)
    // Pass the mock config object using .get()
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE /* bIsOfferReceived */, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_NE(m_pNegotiatedProfile->GetDataPort(), 0);  // Port should be non-zero
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(),
            MEDIA_DIRECTION_SEND_RECEIVE);  // Example expected direction

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(),
            kPeerPayload);  // Should take peer's pPayload number in offer scenario
    EXPECT_TRUE(pNegoPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"));

    auto pNegoFmtp = std::static_pointer_cast<VideoProfile::AvcFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    EXPECT_EQ(pNegoFmtp->GetResolution(), VIDEO_RESOLUTION_VGA_LS);
    EXPECT_EQ(pNegoFmtp->GetLevel(), 31);
    EXPECT_EQ(pNegoFmtp->GetProfileLevelId(), kAvcProfileID);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateNoCommonCodec)
{
    // Arrange: Add incompatible payloads
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddHevcPayload(m_pPeerProfile.get(), kPeerPayload);  // Different codec

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);  // Peer payload is copied
    EXPECT_EQ(m_pNegotiatedProfile->GetIpAddress(), m_pLocalProfile->GetIpAddress());
    EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), 0);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
    EXPECT_NE(m_pNegotiatedProfile->GetRtcpInterval(), 0);
    EXPECT_TRUE(m_pNegotiatedProfile->IsOmitAttributes());
}

TEST_F(VideoProfileNegotiatorTest, NegotiateNoPeerPayloads)
{
    // Arrange: Add incompatible payloads
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    // When no peer payloads, ResetNegotiatedProfile copies the local profile.
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 0);
    EXPECT_EQ(m_pNegotiatedProfile->GetIpAddress(), m_pLocalProfile->GetIpAddress());
    EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), 0);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
    EXPECT_NE(m_pNegotiatedProfile->GetRtcpInterval(), 0);
    EXPECT_TRUE(m_pNegotiatedProfile->IsOmitAttributes());
}

TEST_F(VideoProfileNegotiatorTest, NegotiatePeerPortZero)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pPeerProfile->SetDataPort(0);  // Peer port is zero

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);  // Negotiation should still "succeed" but bResult in port 0
    // When IP/Port negotiation fails, ResetNegotiatedProfile is called.
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);  // Local payload is copied
    EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), 0);         // Port is reset
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
    EXPECT_FALSE(m_pNegotiatedProfile->IsOmitAttributes());
}

TEST_F(VideoProfileNegotiatorTest, NegotiateAvpf)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);

    m_pLocalProfile->SetSupportAvpf(IMS_TRUE);
    m_pLocalProfile->SetTransportType("RTP/AVPF");
    m_pPeerProfile->SetSupportAvpf(IMS_TRUE);
    m_pPeerProfile->SetTransportType("RTP/AVPF");

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(m_pNegotiatedProfile->IsAvpfSupported());
    EXPECT_EQ(m_pNegotiatedProfile->GetTransportType(), "RTP/AVPF");
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    // Add more checks for RTCP-FB attributes if needed
}

TEST_F(VideoProfileNegotiatorTest, NegotiateNullInputsReturnsFalse)
{
    // Arrange (Payloads don't matter here)
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);

    // Act & Assert
    EXPECT_FALSE(m_pNegotiator->Negotiate(
            nullptr, m_pPeerProfile.get(), IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get()));
    EXPECT_FALSE(m_pNegotiator->Negotiate(
            m_pLocalProfile.get(), nullptr, IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get()));
    EXPECT_FALSE(m_pNegotiator->Negotiate(
            m_pLocalProfile.get(), m_pPeerProfile.get(), IMS_TRUE, nullptr, m_pConfig.get()));
    EXPECT_FALSE(m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(), IMS_TRUE,
            m_pNegotiatedProfile.get(), nullptr));
}

TEST_F(VideoProfileNegotiatorTest, NegotiateLocalPortZero)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pLocalProfile->SetDataPort(0);  // Local port is zero

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);  // Negotiation "succeeds" but bResults in port 0
    // When IP/Port negotiation fails, ResetNegotiatedProfile is called.
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);  // Local payload is copied
    EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), 0);         // Port is reset
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateAvcMultipleOffersSuccess)
{
    const AString kTargetAvcProfileID = "42C01F";
    // Arrange: Add compatible payloads
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 15, "640C1F");
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload + 1, VIDEO_RESOLUTION_VGA_LS, 15,
            kTargetAvcProfileID);
    AddAvcPayload(
            m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_VGA_LS, 15, kTargetAvcProfileID);

    // Act: Perform negotiation (Offer received scenario)
    // Pass the mock config object using .get()
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE /* bIsOfferReceived */, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_NE(m_pNegotiatedProfile->GetDataPort(), 0);  // Port should be non-zero
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(),
            MEDIA_DIRECTION_SEND_RECEIVE);  // Example expected direction

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);
    EXPECT_TRUE(pNegoPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"));

    auto pNegoFmtp = std::static_pointer_cast<VideoProfile::AvcFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    EXPECT_NE(pNegoFmtp, nullptr);
    EXPECT_EQ(pNegoFmtp->GetResolution(), VIDEO_RESOLUTION_VGA_LS);
    EXPECT_EQ(pNegoFmtp->GetLevel(), 15);
    EXPECT_EQ(pNegoFmtp->GetProfileLevelId(), kTargetAvcProfileID);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateAvcLevelMatchedInMultipleItems)
{
    const IMS_SINT32 kLocalPayload1 = kLocalPayload;
    const IMS_SINT32 kLocalPayload2 = kLocalPayload + 1;
    // Arrange: Multiple local payloads and single peer payload, the 2nd local payload is matched
    // with the peer payload.
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload1, VIDEO_RESOLUTION_VGA_LS, 30,
            "42e01e");  // Level 3.0
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload2, VIDEO_RESOLUTION_VGA_LS, 12,
            "42e00C");  // Level 1.2
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_VGA_LS, 12,
            "42e00C");  // Level 1.2

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);
    std::shared_ptr<VideoProfile::VideoFmtp> pNegoFmtp = pNegoPayload->GetFmtp();

    // Check the negotiated level and the index, direction
    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    EXPECT_EQ(pNegoFmtp->GetLevel(), 12);
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 1);  // 2nd index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // 1st index
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateAvcLevelMismatchLocalLower)
{
    // Arrange: Local nLevel is lower than peer's
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 30,
            "42e01e");  // Level 3.0
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_VGA_LS, 31,
            "42e01f");  // Level 3.1

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);
    std::shared_ptr<VideoProfile::VideoFmtp> pNegoFmtp = pNegoPayload->GetFmtp();

    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    EXPECT_EQ(pNegoFmtp->GetLevel(), 30);                        // lower Level
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);  // 1st index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // 1st index
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateAvcLevelMismatchPeerLower)
{
    // Arrange: Local nLevel is lower than peer's
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 31,
            "42e01f");  // Level 3.1
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_VGA_LS, 30,
            "42e01e");  // Level 3.0

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);
    auto pNegoFmtp = pNegoPayload->GetFmtp();

    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    EXPECT_EQ(pNegoFmtp->GetLevel(), 30);                        // lower Level
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);  // 1st index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // 1st index
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateMultipleAvcLevelPeerPickMiddle)
{
    // Arrange: Local nLevel is lower than peer's
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 31,
            "640C1F");  // Level 3.1
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload + 1, VIDEO_RESOLUTION_VGA_LS, 31,
            "42e01f");  // Level 3.1
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload + 2, VIDEO_RESOLUTION_QVGA_LS, 12,
            "42C00C");  // Level 1.2
    AddAvcPayload(m_pPeerProfile.get(), kLocalPayload + 1, VIDEO_RESOLUTION_VGA_LS, 12,
            "42C00C");  // Level 1.2

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kLocalPayload + 1);
    auto pNegoFmtp = pNegoPayload->GetFmtp();

    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    EXPECT_EQ(pNegoFmtp->GetLevel(), 12);                        // level 1.2
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 1);  // 2nd index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // 1st index
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateMultipleAvcLevelPeerPickLast)
{
    // Arrange: Local nLevel is lower than peer's
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 31,
            "640C1F");  // Level 3.1
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload + 1, VIDEO_RESOLUTION_VGA_LS, 31,
            "42e01f");  // Level 3.1
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload + 2, VIDEO_RESOLUTION_QVGA_LS, 12,
            "42C00C");  // Level 1.2
    AddAvcPayload(m_pPeerProfile.get(), kLocalPayload + 1, VIDEO_RESOLUTION_QVGA_LS, 12,
            "42C00C");  // Level 1.2

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kLocalPayload + 1);
    auto pNegoFmtp = pNegoPayload->GetFmtp();

    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    EXPECT_EQ(pNegoFmtp->GetLevel(), 12);                        // level 1.2
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 2);  // 3rd index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // 1st index
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateAvcPeerResolutionNotSpecified)
{
    // Arrange: No exact eResolution match, but levels match. Local has VGA+.
    // Expect negotiation to succeed using the closest available local eResolution (VGA).
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 31, "42e01f");
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_NOT_USED, 31, "42e01f");

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);  // Should succeed by choosing the closest (highest available local)
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);  // Peer's PT
    auto pNegoFmtp = pNegoPayload->GetFmtp();
    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";

    // VGA_LS
    EXPECT_EQ(pNegoFmtp->GetResolution(), VIDEO_RESOLUTION_VGA_LS);
    EXPECT_EQ(pNegoFmtp->GetLevel(), 31);
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);  // 1st index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // 1st index
}

TEST_F(VideoProfileNegotiatorTest, NegotiateAvcResolutionMismatchClosest)
{
    // Arrange: No exact eResolution match, but levels match. Local has VGA+CIF, Peer offers VGA.
    // Expect negotiation to succeed using the closest available local eResolution (VGA).
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 31,
            "42e01f");                                                                // Local VGA
    AddAvcPayload(m_pLocalProfile.get(), 99, VIDEO_RESOLUTION_CIF_LS, 31, "42e01f");  // Local CIF
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_HD_LS, 31,
            "42e01f");  // Peer offers HD (mismatch)

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);  // Should succeed by choosing the closest (highest available local)
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);  // Peer's PT
    auto pNegoFmtp = pNegoPayload->GetFmtp();
    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    // When no exact resolution match is found, the negotiator falls back to the first
    // compatible local payload. The final resolution is the lower of the two.
    // Here, local candidate is VGA_LS and peer is HD_LS, so the result is VGA_LS.
    EXPECT_EQ(pNegoFmtp->GetResolution(), VIDEO_RESOLUTION_VGA_LS);
    EXPECT_EQ(pNegoFmtp->GetLevel(), 31);
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);  // 1st index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // 1st index
}

TEST_F(VideoProfileNegotiatorTest, NegotiateAvcResolutionMismatchClosestCIFAndQVGA)
{
    // Arrange: No exact eResolution match, but levels match. Local has QVGA, Peer offers CIF.
    // Expect negotiation to succeed using the closest available local eResolution (QVGA).
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_QVGA_PR, 12,
            "42000C");  // Local QVGA
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_CIF_PR, 11,
            "42000B");  // Peer offers CIF (mismatch)

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);  // Should succeed by choosing the closest (highest available local)
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);  // Peer's PT
    auto pNegoFmtp = pNegoPayload->GetFmtp();
    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    // When resolutions mismatch, the negotiator uses the first compatible local payload
    // as a candidate. The final resolution is the lower of the two.
    // Here, local is QVGA_PR and peer is CIF_PR, so the result is QVGA_PR.
    EXPECT_EQ(pNegoFmtp->GetResolution(), VIDEO_RESOLUTION_QVGA_PR);
    EXPECT_EQ(pNegoFmtp->GetLevel(), 11);
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);  // 1st index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // 1st index
}

TEST_F(VideoProfileNegotiatorTest, NegotiateHevcLevelMismatchLocalLower)
{
    // Arrange: Local level is lower than peer's
    AddHevcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS,
            30);  // Level 3.0 (SDP 90)
    AddHevcPayload(
            m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_VGA_LS, 31);  // Level 3.1 (SDP 93)

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);
    std::shared_ptr<VideoProfile::VideoFmtp> pNegoFmtp = pNegoPayload->GetFmtp();

    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    EXPECT_EQ(pNegoFmtp->GetLevel(), 90);                        // lower nLevel
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);  // 1st index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // 1st index
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateHevcLevelMismatchPeerLower)
{
    // Arrange: peer level is lower than local's
    AddHevcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS,
            31);  // Level 3.1 (SDP 93)
    AddHevcPayload(
            m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_VGA_LS, 30);  // Level 3.0 (SDP 93)

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);
    auto pNegoFmtp = pNegoPayload->GetFmtp();

    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    EXPECT_EQ(pNegoFmtp->GetLevel(), 90);                        // lower nLevel
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);  // 1st index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // 1st index
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateHevcPeerResolutionNotSpecified)
{
    // Arrange: Local level and profile is the same and different resolution.
    AddHevcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 31);
    AddHevcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_NOT_USED, 31);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1) << "Assert by the no payload";

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);

    auto pNegoFmtp = pNegoPayload->GetFmtp();
    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    EXPECT_EQ(pNegoFmtp->GetResolution(), VIDEO_RESOLUTION_VGA_LS);

    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
    EXPECT_NE(m_pNegotiatedProfile->GetDataPort(), 0);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateHevcResolutionMismatch)
{
    // Arrange: Local level and profile is the same and different resolution.
    AddHevcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 31);
    AddHevcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_HD_LS, 31);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1) << "Assert by the no payload";

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);

    auto pNegoFmtp = pNegoPayload->GetFmtp();
    ASSERT_NE(pNegoFmtp, nullptr) << "Assert by the no fmtp";
    EXPECT_EQ(pNegoFmtp->GetResolution(), VIDEO_RESOLUTION_VGA_LS);

    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
    EXPECT_NE(m_pNegotiatedProfile->GetDataPort(), 0);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateAnswerSentBasicSuccess)
{
    // Arrange: Add compatible payloads
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 31, "42e01f");
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_VGA_LS, 31, "42e01f");

    // Act: Perform negotiation (Answer sent scenario)
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_FALSE /* bIsOfferReceived = false */, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    // Should use local pPayload number in answer scenario
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);
    EXPECT_TRUE(pNegoPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("H264"));
    // Local pPayload number should NOT be modified in MO case
    EXPECT_EQ(m_pLocalProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), kLocalPayload);
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);  // 1st index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // 1st index
}

TEST_F(VideoProfileNegotiatorTest, NegotiateBandwidthRemoteOption)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    // Set config option to use remote bandwidth values
    EXPECT_CALL(*m_pConfig, GetBandwidthNegoOption())
            .WillRepeatedly(Return(MediaConfiguration::BW_OPTION_REMOTE_VALUE));
    // Set distinct peer bandwidths
    m_pPeerProfile->SetBandwidthRs(500);
    m_pPeerProfile->SetBandwidthRr(600);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    // Check that negotiated bandwidth matches the peer's values due to config option
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRs(), 500);
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRr(), 600);
    // AS value depends on codec and potentially remote AS if config option is remote and lower
    // Add specific AS check if needed based on MakeNegotiatedBandwidth logic
}

TEST_F(VideoProfileNegotiatorTest, NegotiateRtcpIntervalHold)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pPeerProfile->SetDirection(
            MEDIA_DIRECTION_SEND);  // Not SEND_RECEIVE, so should use hold interval
    // Set distinct config intervals
    EXPECT_CALL(*m_pConfig, GetRtcpIntervalOnActive()).WillRepeatedly(Return(5));
    EXPECT_CALL(*m_pConfig, GetRtcpIntervalOnHold()).WillRepeatedly(Return(30));

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_RECEIVE);
    // Should use the hold interval because direction is not SEND_RECEIVE
    EXPECT_EQ(m_pNegotiatedProfile->GetRtcpInterval(), 30);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateRtcpFbAllSupported)
{
    // Arrange
    VideoProfile::Payload* pLocalPayload = AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    VideoProfile::Payload* pPeerPayload = AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pLocalProfile->SetSupportAvpf(IMS_TRUE);
    m_pPeerProfile->SetSupportAvpf(IMS_TRUE);

    // Enable all FB types on both local and peer
    pLocalPayload->GetRtcpFbAttr().SetNackSupported(IMS_TRUE);
    pLocalPayload->GetRtcpFbAttr().SetTmmbrSupported(IMS_TRUE);
    pLocalPayload->GetRtcpFbAttr().SetPliSupported(IMS_TRUE);
    pLocalPayload->GetRtcpFbAttr().SetFirSupported(IMS_TRUE);
    pLocalPayload->GetRtcpFbAttr().SetTrrSupported(IMS_TRUE);
    pLocalPayload->GetRtcpFbAttr().SetTrrInt(100);

    pPeerPayload->GetRtcpFbAttr().SetNackSupported(IMS_TRUE);
    pPeerPayload->GetRtcpFbAttr().SetTmmbrSupported(IMS_TRUE);
    pPeerPayload->GetRtcpFbAttr().SetPliSupported(IMS_TRUE);
    pPeerPayload->GetRtcpFbAttr().SetFirSupported(IMS_TRUE);
    pPeerPayload->GetRtcpFbAttr().SetTrrSupported(IMS_TRUE);
    pPeerPayload->GetRtcpFbAttr().SetTrrInt(120);  // Different TrrInt

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_NE(pNegoPayload, nullptr);
    EXPECT_TRUE(m_pNegotiatedProfile->IsAvpfSupported());

    // Check that all common FB types are enabled in negotiated pPayload
    EXPECT_TRUE(pNegoPayload->GetRtcpFbAttr().IsNackSupported());
    EXPECT_TRUE(pNegoPayload->GetRtcpFbAttr().IsTmmbrSupported());
    EXPECT_TRUE(pNegoPayload->GetRtcpFbAttr().IsPliSupported());
    EXPECT_TRUE(pNegoPayload->GetRtcpFbAttr().IsFirSupported());
    EXPECT_TRUE(pNegoPayload->GetRtcpFbAttr().IsTrrSupported());
    // TRR-INT should take the peer's value
    EXPECT_EQ(pNegoPayload->GetRtcpFbAttr().GetTrrInt(), 120);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateCvoEnabled)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pLocalProfile->SetCvoId(1);  // Local supports CVO
    m_pPeerProfile->SetCvoId(1);   // Peer supports CVO

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    // CVO ID should be negotiated (takes peer's ID if both > 0)
    EXPECT_EQ(m_pNegotiatedProfile->GetCvoId(), 1);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateCvoDisabledPeer)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pLocalProfile->SetCvoId(1);  // Local supports CVO
    m_pPeerProfile->SetCvoId(0);   // Peer does NOT support CVO

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    // CVO ID should be 0 if either side doesn't support it
    EXPECT_EQ(m_pNegotiatedProfile->GetCvoId(), 0);
}

TEST_F(VideoProfileNegotiatorTest, NegotiatePacketizationMode)
{
    // Arrange
    // Local payload with packetization-mode = 1
    VideoProfile::Payload* pLocalPayload = AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    auto pLocalFmtp = std::static_pointer_cast<VideoProfile::AvcFmtp>(pLocalPayload->GetFmtp());
    pLocalFmtp->SetPacketizationMode(1);

    // Peer payload with packetization-mode = 0
    VideoProfile::Payload* pPeerPayload = AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    auto pPeerFmtp = std::static_pointer_cast<VideoProfile::AvcFmtp>(pPeerPayload->GetFmtp());
    pPeerFmtp->SetPacketizationMode(0);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE /* bIsOfferReceived */, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);

    VideoProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);

    auto pNegoFmtp = std::static_pointer_cast<VideoProfile::AvcFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // The negotiated packetization-mode should be the peer's mode.
    EXPECT_EQ(pNegoFmtp->GetPacketizationMode(), pPeerFmtp->GetPacketizationMode());
    EXPECT_EQ(pNegoFmtp->GetPacketizationMode(), 0);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateDirectionOfferPeerSend)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND);  // Peer offers to send only

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE /* Offer Received */, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_RECEIVE);  // We should receive
}

TEST_F(VideoProfileNegotiatorTest, NegotiateDirectionOfferPeerReceive)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_RECEIVE);  // Peer offers to receive only

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE /* Offer Received */, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    // If peer offers RECEIVE, we should negotiate to SEND
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateBandwidthASPeerLower)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pLocalProfile->SetBandwidthAs(1000);
    m_pPeerProfile->SetBandwidthAs(800);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    // AS should be the lower of the two values
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthAs(), 800);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateBandwidthASPeerNoBw)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pLocalProfile->SetBandwidthAs(1000);
    m_pPeerProfile->SetBandwidthAs(-1); // Peer has no AS bandwidth

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    // AS bandwidth should fall back to local profile's value
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthAs(), 1000);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateFrameRate)
{
    // Arrange
    // Case 1: Negotiated payload framerate is higher than peer's profile framerate.
    m_pPeerProfile->SetFrameRate(15);
    VideoProfile::Payload* pLocalPayload1 =
            AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 31);
    auto pLocalFmtp1 = std::static_pointer_cast<VideoProfile::AvcFmtp>(pLocalPayload1->GetFmtp());
    pLocalFmtp1->SetFramerate(30);

    VideoProfile::Payload* pPeerPayload1 =
            AddAvcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_VGA_LS, 31);
    auto pPeerFmtp1 = std::static_pointer_cast<VideoProfile::AvcFmtp>(pPeerPayload1->GetFmtp());
    pPeerFmtp1->SetFramerate(30);

    // Act
    IMS_BOOL bResult1 = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult1);
    // The final framerate should be the higher one from the negotiated payload.
    EXPECT_EQ(m_pNegotiatedProfile->GetFrameRate(), 30);

    // Arrange
    // Case 2: Peer's profile framerate is higher.
    m_pLocalProfile->DeletePayloads();
    m_pPeerProfile->DeletePayloads();
    m_pNegotiatedProfile = std::make_unique<VideoProfile>();

    m_pPeerProfile->SetFrameRate(25);
    VideoProfile::Payload* pLocalPayload2 =
            AddAvcPayload(m_pLocalProfile.get(), kLocalPayload, VIDEO_RESOLUTION_VGA_LS, 31);
    auto pLocalFmtp2 = std::static_pointer_cast<VideoProfile::AvcFmtp>(pLocalPayload2->GetFmtp());
    pLocalFmtp2->SetFramerate(20);

    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload, VIDEO_RESOLUTION_VGA_LS, 31);

    // Act
    IMS_BOOL bResult2 = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult2);
    // The final framerate should be the higher one from the peer's profile.
    EXPECT_EQ(m_pNegotiatedProfile->GetFrameRate(), 25);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateBandwidthASLocalLower)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pLocalProfile->SetBandwidthAs(800);
    m_pPeerProfile->SetBandwidthAs(1000);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    // AS should be the lower of the two values
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthAs(), 800);
}

TEST_F(VideoProfileNegotiatorTest, NegotiateDirectionAnswerPeerReceive)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_RECEIVE);  // Peer expects to receive only

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_FALSE /* Answer Sent */, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND);  // We should send
}

TEST_F(VideoProfileNegotiatorTest, NegotiateDirectionInactive)
{
    // Arrange
    AddAvcPayload(m_pLocalProfile.get(), kLocalPayload);
    AddAvcPayload(m_pPeerProfile.get(), kPeerPayload);
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_INACTIVE);  // Peer offers inactive

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE /* Offer Received */, m_pNegotiatedProfile.get(), m_pConfig.get());

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
}
