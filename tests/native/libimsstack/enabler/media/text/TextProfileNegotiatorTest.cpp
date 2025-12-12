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

#include "text/TextProfile.h"
#include "text/TextProfileNegotiator.h"

#include "config/MockTextConfiguration.h"

using testing::Return;

class TextProfileNegotiatorTest : public ::testing::Test
{
protected:
    void SetUp() override;

    TextProfile::Payload* CreateT140Payload(IMS_UINT32 nPayloadNum);
    TextProfile::Payload* CreateRedPayload(
            IMS_UINT32 nRedPayloadNum, IMS_UINT32 nT140PayloadNum, IMS_UINT32 nRedLevel);

    std::unique_ptr<TextProfileNegotiator> m_pNegotiator;
    std::unique_ptr<TextProfile> m_pLocalProfile;
    std::unique_ptr<TextProfile> m_pPeerProfile;
    std::unique_ptr<TextProfile> m_pNegotiatedProfile;
    MockTextConfiguration m_objMockConfig;

    const IMS_UINT32 kLocalT140Payload = 98;
    const IMS_UINT32 kPeerT140Payload = 100;
    const IMS_UINT32 kLocalRedPayload = 99;
    const IMS_UINT32 kPeerRedPayload = 101;
    const IMS_UINT32 kLocalDataPort = 7000;
    const IMS_UINT32 kPeerDataPort = 7002;
};

void TextProfileNegotiatorTest::SetUp()
{
    m_pNegotiator = std::make_unique<TextProfileNegotiator>();
    m_pLocalProfile = std::make_unique<TextProfile>();
    m_pPeerProfile = std::make_unique<TextProfile>();
    m_pNegotiatedProfile = std::make_unique<TextProfile>();

    // Default mock config values (adjust as needed for Text)
    ON_CALL(m_objMockConfig, GetRtcpIntervalOnActive()).WillByDefault(Return(5));
    ON_CALL(m_objMockConfig, GetRtcpIntervalOnHold()).WillByDefault(Return(3));
    ON_CALL(m_objMockConfig, GetAsBandwidthKbps()).WillByDefault(Return(2));
    ON_CALL(m_objMockConfig, GetRsBandwidthBps()).WillByDefault(Return(200));
    ON_CALL(m_objMockConfig, GetRrBandwidthBps()).WillByDefault(Return(200));

    // Set default local profile values
    m_pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pLocalProfile->SetDataPort(kLocalDataPort);
    m_pLocalProfile->SetBandwidthAs(2);
    m_pLocalProfile->SetBandwidthRs(200);
    m_pLocalProfile->SetBandwidthRr(200);

    // set default peer profile values
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(kPeerDataPort);
    m_pPeerProfile->SetBandwidthAs(2);
    m_pPeerProfile->SetBandwidthRs(200);
    m_pPeerProfile->SetBandwidthRr(200);
}

TextProfile::Payload* TextProfileNegotiatorTest::CreateT140Payload(IMS_UINT32 nPayloadNum)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    pPayload->GetRtpMap().SetPayloadType("t140");
    pPayload->GetRtpMap().SetSamplingRate(1000);  // Standard clock rate for t140
    pPayload->GetRtpMap().SetPayloadNumber(nPayloadNum);
    return pPayload;
}

TextProfile::Payload* TextProfileNegotiatorTest::CreateRedPayload(
        IMS_UINT32 nRedPayloadNum, IMS_UINT32 nT140PayloadNum, IMS_UINT32 nRedLevel)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    pPayload->GetRtpMap().SetPayloadType("red");
    pPayload->GetRtpMap().SetSamplingRate(1000);  // RED uses the clock rate of the primary codec
    pPayload->GetRtpMap().SetPayloadNumber(nRedPayloadNum);

    auto pFmtp = std::make_shared<TextProfile::RedFmtp>(nRedLevel, nT140PayloadNum);
    pPayload->SetFmtp(pFmtp);
    return pPayload;
}

// --- Test Cases ---

TEST_F(TextProfileNegotiatorTest, NegotiateNullInputsReturnsFalse)
{
    EXPECT_FALSE(m_pNegotiator->Negotiate(nullptr, m_pPeerProfile.get(), IMS_FALSE,
            m_pNegotiatedProfile.get(), &m_objMockConfig));
    EXPECT_FALSE(m_pNegotiator->Negotiate(m_pLocalProfile.get(), nullptr, IMS_FALSE,
            m_pNegotiatedProfile.get(), &m_objMockConfig));
    EXPECT_FALSE(m_pNegotiator->Negotiate(
            m_pLocalProfile.get(), m_pPeerProfile.get(), IMS_FALSE, nullptr, &m_objMockConfig));
    EXPECT_FALSE(m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(), IMS_FALSE,
            m_pNegotiatedProfile.get(), nullptr));
}

TEST_F(TextProfileNegotiatorTest, NegotiateBasicT140OfferReceivedSuccess)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 1);
    TextProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "t140");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerT140Payload);  // Takes peer PT
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
    EXPECT_GT(m_pNegotiatedProfile->GetDataPort(), 0);
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthAs(), 2);    // Default AS from mock
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRs(), 200);  // Default RS from mock
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRr(), 200);  // Default RR from mock
    EXPECT_EQ(m_pNegotiatedProfile->GetRtcpInterval(), 5);   // Active interval
}

TEST_F(TextProfileNegotiatorTest, NegotiateBasicT140OfferSentSuccess)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));  // Peer Answer
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);

    // Act: Offer Sent (bIsOfferReceived = false)
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_FALSE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 1);
    TextProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "t140");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerT140Payload);  // Takes peer PT
    EXPECT_EQ(
            m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);  // Uses local dir
    EXPECT_GT(m_pNegotiatedProfile->GetDataPort(), 0);
    // Bandwidth should take peer's values in MO case if available
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthAs(), m_pPeerProfile->GetBandwidthAs());
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRs(), m_pPeerProfile->GetBandwidthRs());
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRr(), m_pPeerProfile->GetBandwidthRr());
}

TEST_F(TextProfileNegotiatorTest, NegotiateRedOfferReceivedSuccess)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pLocalProfile->AddPayload(CreateRedPayload(
            kLocalRedPayload, kLocalT140Payload, 1));  // Local RED PT 99, T140 PT 98
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->AddPayload(CreateRedPayload(
            kPeerRedPayload, kPeerT140Payload, 1));  // Peer RED PT 101, T140 PT 100
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(7004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    // Should negotiate both T140 and RED
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 2);

    // Check T140
    TextProfile::Payload* pNegoT140 = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoT140->GetRtpMap().GetPayloadType(), "t140");
    EXPECT_EQ(pNegoT140->GetRtpMap().GetPayloadNumber(), kPeerT140Payload);

    // Check RED
    TextProfile::Payload* pNegoRed = m_pNegotiatedProfile->GetPayloadAt(1);
    EXPECT_EQ(pNegoRed->GetRtpMap().GetPayloadType(), "red");
    EXPECT_EQ(pNegoRed->GetRtpMap().GetPayloadNumber(), kPeerRedPayload);

    ASSERT_NE(pNegoRed->GetFmtp(), nullptr);
    auto pNegoFmtp = std::static_pointer_cast<TextProfile::RedFmtp>(pNegoRed->GetFmtp());
    // Check if the FMTP string reflects the peer's primary payload type
    EXPECT_EQ(pNegoFmtp->GetRedPayload(), kPeerT140Payload);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(TextProfileNegotiatorTest, NegotiateBandwidthOfferSentPeerNoBw)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetBandwidthAs(0);  // Peer has no AS bandwidth
    m_pPeerProfile->SetBandwidthRs(0);  // Peer has no RS bandwidth
    m_pPeerProfile->SetBandwidthRr(0);  // Peer has no RR bandwidth

    // Act: Offer Sent
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_FALSE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);

    // AS bandwidth should fall back to local profile's values
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthAs(), m_pLocalProfile->GetBandwidthAs());
    // RS/RR bandwidth should fall back to peer profile's values
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRs(), m_pPeerProfile->GetBandwidthRs());
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRr(), m_pPeerProfile->GetBandwidthRr());
}

TEST_F(TextProfileNegotiatorTest, NegotiateBandwidthOfferReceivedLocalNoBw)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pLocalProfile->SetBandwidthAs(0);
    m_pLocalProfile->SetBandwidthRs(0);
    m_pLocalProfile->SetBandwidthRs(0);
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);

    // Act: Offer Received
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);

    // Bandwidth should be local profile's values except the RS
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthAs(), m_pLocalProfile->GetBandwidthAs());
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRs(), m_pPeerProfile->GetBandwidthRs());
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRr(), m_pLocalProfile->GetBandwidthRr());
}

TEST_F(TextProfileNegotiatorTest, NegotiateRedWithMissingT140InPeer)
{
    // Arrange: Local has T140, Peer has RED but is missing the T140 payload in its list.
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pPeerProfile->AddPayload(CreateRedPayload(kPeerRedPayload, kPeerT140Payload, 1));

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert: Should still succeed by finding the T140 in the local profile.
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 1);
}

TEST_F(TextProfileNegotiatorTest, NegotiateRedOnlyInPeer)
{
    // Arrange
    // Local: T140, RED
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pLocalProfile->AddPayload(CreateRedPayload(kLocalRedPayload, kLocalT140Payload, 1));

    // Peer: RED only, with a T140 subtype that doesn't exist in the peer's own payload list
    m_pPeerProfile->AddPayload(CreateRedPayload(kPeerRedPayload, kPeerT140Payload, 1));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(7018);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 2);

    // Check RED
    TextProfile::Payload* pNegoRed = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoRed->GetFmtp(), nullptr);
    EXPECT_EQ(pNegoRed->GetRtpMap().GetPayloadType(), "red");
    EXPECT_EQ(pNegoRed->GetRtpMap().GetPayloadNumber(), kPeerRedPayload);

    // Check T140
    TextProfile::Payload* pNegoT140 = m_pNegotiatedProfile->GetPayloadAt(1);
    EXPECT_EQ(pNegoT140->GetRtpMap().GetPayloadType(), "t140");
    EXPECT_EQ(pNegoT140->GetRtpMap().GetPayloadNumber(), kPeerT140Payload);

    // Check direction
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(TextProfileNegotiatorTest, NegotiateRedSuccessEvenIfNullFmtpInLocalPayload)
{
    // Arrange
    // Local profile has a valid T140, but its RED payload has a null fmtp.
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    TextProfile::Payload* pLocalRed = CreateRedPayload(kLocalRedPayload, kLocalT140Payload, 1);
    pLocalRed->SetFmtp(IMS_NULL);
    m_pLocalProfile->AddPayload(pLocalRed);

    // Peer profile has valid T140 and RED payloads.
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->AddPayload(CreateRedPayload(kPeerRedPayload, kPeerT140Payload, 1));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(7004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    // The overall negotiation should succeed because T140 is a valid fallback.
    EXPECT_TRUE(bResult);
    // Only T140 should be negotiated. RED negotiation should fail due to the null fmtp.
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 2);
    TextProfile::Payload* pNegoPayload1 = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload1->GetRtpMap().GetPayloadType(), "t140");
    EXPECT_EQ(pNegoPayload1->GetRtpMap().GetPayloadNumber(), kPeerT140Payload);
    TextProfile::Payload* pNegoPayload2 = m_pNegotiatedProfile->GetPayloadAt(1);
    EXPECT_EQ(pNegoPayload2->GetRtpMap().GetPayloadType(), "red");
    EXPECT_EQ(pNegoPayload2->GetRtpMap().GetPayloadNumber(), kPeerRedPayload);
    EXPECT_NE(pNegoPayload2->GetFmtp(), IMS_NULL);
}

TEST_F(TextProfileNegotiatorTest, NegotiateRedSuccessEvenIfNullFmtpInPeerPayload)
{
    // Arrange
    // Local profile has a valid T140, but its RED payload has a null fmtp.
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pLocalProfile->AddPayload(CreateRedPayload(kLocalRedPayload, kLocalT140Payload, 1));

    // Peer profile has valid T140 and RED payloads.
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    TextProfile::Payload* pPeerRed = CreateRedPayload(kPeerRedPayload, kPeerT140Payload, 1);
    pPeerRed->SetFmtp(IMS_NULL);
    m_pPeerProfile->AddPayload(pPeerRed);
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(7004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    // The overall negotiation should succeed because T140 is a valid fallback.
    EXPECT_TRUE(bResult);
    // Only T140 should be negotiated. RED negotiation should fail due to the null fmtp.
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 2);
    TextProfile::Payload* pNegoPayload1 = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload1->GetRtpMap().GetPayloadType(), "t140");
    EXPECT_EQ(pNegoPayload1->GetRtpMap().GetPayloadNumber(), kPeerT140Payload);
    TextProfile::Payload* pNegoPayload2 = m_pNegotiatedProfile->GetPayloadAt(1);
    EXPECT_EQ(pNegoPayload2->GetRtpMap().GetPayloadType(), "red");
    EXPECT_EQ(pNegoPayload2->GetRtpMap().GetPayloadNumber(), kPeerRedPayload);
    EXPECT_NE(pNegoPayload2->GetFmtp(), IMS_NULL);
}

TEST_F(TextProfileNegotiatorTest, NegotiateNoMatchingPayloadReturnsFalse)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    // Peer profile has no payloads (or incompatible ones)
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(7006);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    // Negotiation "succeeds" but results in port 0 / invalid direction because no payload match
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 0);
    EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), 0);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
}

TEST_F(TextProfileNegotiatorTest, NegotiateDirectionOfferPeerSendReturnsReceive)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND);  // Peer offers SEND
    m_pPeerProfile->SetDataPort(7008);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_RECEIVE);  // Should be RECEIVE
    EXPECT_EQ(m_pNegotiatedProfile->GetRtcpInterval(), 3);                     // Hold interval
}

TEST_F(TextProfileNegotiatorTest, NegotiateDirectionInvalidPeerDirReturnsInactive)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_INVALID);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    // If peer direction is invalid, negotiated direction should be inactive.
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), kLocalDataPort);
}

TEST_F(TextProfileNegotiatorTest, NegotiateDirectionOfferPeerReceiveReturnsSend)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_RECEIVE);  // Peer offers RECEIVE
    m_pPeerProfile->SetDataPort(7010);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND);  // Should be SEND
    EXPECT_EQ(m_pNegotiatedProfile->GetRtcpInterval(), 3);                  // Hold interval
}

TEST_F(TextProfileNegotiatorTest, NegotiateDirectionInactiveReturnsInactive)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_INACTIVE);  // Peer offers INACTIVE

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 1);  // Payload still negotiated
    EXPECT_EQ(
            m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);  // Should be INACTIVE
    EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), m_pLocalProfile->GetDataPort());
    EXPECT_EQ(m_pNegotiatedProfile->GetRtcpInterval(), 3);  // Hold interval (or 0 if RS/RR are 0)
}

TEST_F(TextProfileNegotiatorTest, NegotiateRtcpIntervalDisabledWhenRsRrZero)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(7012);
    m_pPeerProfile->SetBandwidthRs(0);  // Peer has zero RS/RR
    m_pPeerProfile->SetBandwidthRr(0);

    // Act: Negotiate with offer received
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRs(), 0);
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRr(), 0);
    // RTCP interval should be forced to 0 when RS and RR are 0
    EXPECT_EQ(m_pNegotiatedProfile->GetRtcpInterval(), 0);
}

TEST_F(TextProfileNegotiatorTest, NegotiateMultiplePayloadsPeerRedFirst)
{
    // Arrange
    // Local: T140, RED
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));
    m_pLocalProfile->AddPayload(CreateRedPayload(kLocalRedPayload, kLocalT140Payload, 1));

    // Peer: RED, T140 (RED has higher priority)
    m_pPeerProfile->AddPayload(CreateRedPayload(kPeerRedPayload, kPeerT140Payload, 1));
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(7014);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 2);

    // Check that negotiated order matches peer's priority
    TextProfile::Payload* pNegoRed = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoRed->GetRtpMap().GetPayloadType(), "red");
    EXPECT_EQ(pNegoRed->GetRtpMap().GetPayloadNumber(), kPeerRedPayload);

    TextProfile::Payload* pNegoT140 = m_pNegotiatedProfile->GetPayloadAt(1);
    EXPECT_EQ(pNegoT140->GetRtpMap().GetPayloadType(), "t140");
    EXPECT_EQ(pNegoT140->GetRtpMap().GetPayloadNumber(), kPeerT140Payload);
}

TEST_F(TextProfileNegotiatorTest, NegotiateMultiplePayloadsPeerT140First)
{
    // Arrange
    // Local: RED, T140
    m_pLocalProfile->AddPayload(CreateRedPayload(kLocalRedPayload, kLocalT140Payload, 1));
    m_pLocalProfile->AddPayload(CreateT140Payload(kLocalT140Payload));

    // Peer: T140, RED (T140 has higher priority)
    m_pPeerProfile->AddPayload(CreateT140Payload(kPeerT140Payload));
    m_pPeerProfile->AddPayload(CreateRedPayload(kPeerRedPayload, kPeerT140Payload, 1));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(7016);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadList().GetSize(), 2);

    // Check that negotiated order matches peer's priority
    TextProfile::Payload* pNegoT140 = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoT140->GetRtpMap().GetPayloadType(), "t140");
    EXPECT_EQ(pNegoT140->GetRtpMap().GetPayloadNumber(), kPeerT140Payload);

    TextProfile::Payload* pNegoRed = m_pNegotiatedProfile->GetPayloadAt(1);
    EXPECT_EQ(pNegoRed->GetRtpMap().GetPayloadType(), "red");
    EXPECT_EQ(pNegoRed->GetRtpMap().GetPayloadNumber(), kPeerRedPayload);
}
