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

#include "audio/AudioProfileNegotiator.h"

#include "config/CodecAudioConfig.h"
#include "config/MockAudioConfiguration.h"

using testing::Return;

const IMS_UINT32 kLocalPayload = 99;
const IMS_UINT32 kPeerPayload = 100;

class AudioProfileNegotiatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_pNegotiator = std::make_unique<AudioProfileNegotiator>();
        m_pLocalProfile = std::make_unique<AudioProfile>();
        m_pPeerProfile = std::make_unique<AudioProfile>();
        m_pNegotiatedProfile = std::make_unique<AudioProfile>();

        // Default mock config values
        ON_CALL(m_objMockConfig, GetBandwidthNegoOption())
                .WillByDefault(Return(MediaConfiguration::BW_OPTION_LOCAL_VALUE));
        ON_CALL(m_objMockConfig, GetRtcpIntervalOnActive()).WillByDefault(Return(5));
        ON_CALL(m_objMockConfig, GetRtcpIntervalOnHold()).WillByDefault(Return(3));
        ON_CALL(m_objMockConfig, IsAmrPayloadFormatRelaxedMatching())
                .WillByDefault(Return(IMS_FALSE));
        // Set default local profile values often needed
        m_pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
        m_pLocalProfile->SetDataPort(5004);
        m_pLocalProfile->SetBandwidthRs(300);
        m_pLocalProfile->SetBandwidthRr(300);
        m_pLocalProfile->SetBandwidthAs(32);  // Example AS
    }

    void TearDown() override {}

    // Helper to create a basic AMR-WB payload
    AudioProfile::Payload* CreateAmrWbPayload(IMS_UINT32 nPayloadNum, IMS_UINT32 nModeSet)
    {
        auto pPayload = new AudioProfile::Payload();
        pPayload->GetRtpMap().SetPayloadType("AMR-WB");
        pPayload->GetRtpMap().SetSamplingRate(16000);
        pPayload->GetRtpMap().SetPayloadNumber(nPayloadNum);
        auto pFmtp = std::make_shared<AudioProfile::AmrFmtp>();
        pFmtp->SetModeSetList(nModeSet);
        pPayload->SetFmtp(pFmtp);
        return pPayload;
    }

    // Helper to create a basic AMR-WB payload with octet-align
    AudioProfile::Payload* CreateAmrWbPayload(
            IMS_UINT32 nPayloadNum, IMS_UINT32 nModeSet, IMS_SINT32 octetAlign)
    {
        auto pPayload = CreateAmrWbPayload(nPayloadNum, nModeSet);
        std::static_pointer_cast<AudioProfile::AmrFmtp>(pPayload->GetFmtp())
                ->SetOctetAlign(octetAlign);
        return pPayload;
    }

    // Helper to create a basic AMR payload
    AudioProfile::Payload* CreateAmrPayload(IMS_UINT32 nPayloadNum, IMS_UINT32 nModeSet)
    {
        auto pPayload = new AudioProfile::Payload();
        pPayload->GetRtpMap().SetPayloadType("AMR");
        pPayload->GetRtpMap().SetSamplingRate(8000);
        pPayload->GetRtpMap().SetPayloadNumber(nPayloadNum);
        auto pFmtp = std::make_shared<AudioProfile::AmrFmtp>();
        pFmtp->SetModeSetList(nModeSet);
        pPayload->SetFmtp(pFmtp);
        return pPayload;
    }

    // Helper to create a basic AMR payload with octet-align
    AudioProfile::Payload* CreateAmrPayload(
            IMS_UINT32 nPayloadNum, IMS_UINT32 nModeSet, IMS_SINT32 octetAlign)
    {
        auto pPayload = CreateAmrPayload(nPayloadNum, nModeSet);
        std::static_pointer_cast<AudioProfile::AmrFmtp>(pPayload->GetFmtp())
                ->SetOctetAlign(octetAlign);
        return pPayload;
    }

    // Helper to create a basic EVS payload
    AudioProfile::Payload* CreateEvsPayload(
            IMS_UINT32 nPayloadNum, IMS_UINT32 bwList, IMS_UINT32 brList)
    {
        auto pPayload = new AudioProfile::Payload();
        pPayload->GetRtpMap().SetPayloadType("EVS");
        pPayload->GetRtpMap().SetSamplingRate(16000);  // EVS uses 16k base rate in RTPMAP
        pPayload->GetRtpMap().SetPayloadNumber(nPayloadNum);
        auto pFmtp = std::make_shared<AudioProfile::EvsFmtp>();
        pFmtp->SetBwList(bwList);
        pFmtp->SetBrList(brList);
        // Add other necessary EVS params if needed for specific tests
        pPayload->SetFmtp(pFmtp);
        return pPayload;
    }

    // Helper to create a basic PCMU payload
    AudioProfile::Payload* CreatePcmuPayload(IMS_UINT32 nPayloadNum)
    {
        auto pPayload = new AudioProfile::Payload();
        pPayload->GetRtpMap().SetPayloadType("PCMU");
        pPayload->GetRtpMap().SetSamplingRate(8000);
        pPayload->GetRtpMap().SetPayloadNumber(nPayloadNum);
        // PCMU typically doesn't have FMTP
        return pPayload;
    }

    // Helper to create a telephone-event payload
    AudioProfile::Payload* CreateTelephoneEventPayload(
            IMS_UINT32 nPayloadNum, IMS_SINT32 samplingRate)
    {
        auto pPayload = new AudioProfile::Payload();
        pPayload->GetRtpMap().SetPayloadType("telephone-event");
        pPayload->GetRtpMap().SetSamplingRate(samplingRate);
        pPayload->GetRtpMap().SetPayloadNumber(nPayloadNum);
        std::shared_ptr<AudioProfile::TelephoneEventFmtp> pFmtp =
                std::make_shared<AudioProfile::TelephoneEventFmtp>();
        pFmtp->SetEvents("0-15");  // Example event list
        pPayload->SetFmtp(pFmtp);
        return pPayload;
    }

    std::unique_ptr<AudioProfileNegotiator> m_pNegotiator;
    std::unique_ptr<AudioProfile> m_pLocalProfile;
    std::unique_ptr<AudioProfile> m_pPeerProfile;
    std::unique_ptr<AudioProfile> m_pNegotiatedProfile;
    testing::NiceMock<MockAudioConfiguration> m_objMockConfig;
};

TEST_F(AudioProfileNegotiatorTest, NegotiateNullInputsReturnsFalse)
{
    EXPECT_FALSE(m_pNegotiator->Negotiate(IMS_NULL, m_pPeerProfile.get(), IMS_FALSE,
            m_pNegotiatedProfile.get(), &m_objMockConfig));
    EXPECT_FALSE(m_pNegotiator->Negotiate(m_pLocalProfile.get(), IMS_NULL, IMS_FALSE,
            m_pNegotiatedProfile.get(), &m_objMockConfig));
    EXPECT_FALSE(m_pNegotiator->Negotiate(
            m_pLocalProfile.get(), m_pPeerProfile.get(), IMS_FALSE, IMS_NULL, &m_objMockConfig));
    EXPECT_FALSE(m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(), IMS_FALSE,
            m_pNegotiatedProfile.get(), IMS_NULL));
}

TEST_F(AudioProfileNegotiatorTest, NegotiateBasicAmrOfferReceivedReturnsTrue)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadType(), "AMR-WB");
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(),
            kPeerPayload);  // Should take peer's PT
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
    EXPECT_GT(m_pNegotiatedProfile->GetDataPort(), 0);
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);  // Local index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // Peer index
    // Check if local PT was updated (MT case)
    EXPECT_EQ(m_pLocalProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), kPeerPayload);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateBasicAmrOfferSentReturnsTrue)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));  // Local PT kLocalPayload
    m_pPeerProfile->AddPayload(
            CreateAmrWbPayload(kPeerPayload, 0xFF));  // Peer PT kPeerPayload (in Answer)
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act: Call Negotiate with bIsOfferReceived = IMS_FALSE (MO scenario)
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_FALSE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "AMR-WB");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
    EXPECT_GT(m_pNegotiatedProfile->GetDataPort(), 0);
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);  // Local index
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);   // Peer index
    // Check that local PT was NOT updated (MO case)
    EXPECT_EQ(m_pLocalProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), kLocalPayload);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsPeerNoFmtpSamePayloadNumber)
{
    // Arrange
    // Local profile has a complete EVS payload
    m_pLocalProfile->AddPayload(CreateEvsPayload(98, 0x0F, 0xFFF));

    // Peer profile has an EVS payload but its fmtp is null
    AudioProfile::Payload* pPeerEvsPayload = new AudioProfile::Payload();
    pPeerEvsPayload->GetRtpMap().SetPayloadType("EVS");
    pPeerEvsPayload->GetRtpMap().SetSamplingRate(16000);
    pPeerEvsPayload->GetRtpMap().SetPayloadNumber(98);
    pPeerEvsPayload->SetFmtp(nullptr);  // No fmtp
    m_pPeerProfile->AddPayload(pPeerEvsPayload);

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6018);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");

    // When peer fmtp is null, the local fmtp should be copied to the negotiated payload.
    EXPECT_EQ(*pNegoPayload->GetFmtp(), *m_pLocalProfile->GetPayloadAt(0)->GetFmtp());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsPeerNoFmtpDifferentPayloadNumber)
{
    // Arrange
    // Local profile has a complete EVS payload
    m_pLocalProfile->AddPayload(CreateEvsPayload(98, 0x0F, 0xFFF));

    // Peer profile has an EVS payload but its fmtp is null and a different payload number
    AudioProfile::Payload* pPeerEvsPayload = new AudioProfile::Payload();
    pPeerEvsPayload->GetRtpMap().SetPayloadType("EVS");
    pPeerEvsPayload->GetRtpMap().SetSamplingRate(16000);
    pPeerEvsPayload->GetRtpMap().SetPayloadNumber(100);  // Different payload number
    pPeerEvsPayload->SetFmtp(nullptr);                   // No fmtp
    m_pPeerProfile->AddPayload(pPeerEvsPayload);

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6018);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    // The negotiated payload should have the peer's payload number but the local's fmtp.
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), 100);
    EXPECT_EQ(*pNegoPayload->GetFmtp(), *m_pLocalProfile->GetPayloadAt(0)->GetFmtp());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAmrFmtpVisibility)
{
    // Arrange
    // Local profile supports octet-align=1 and has a specific mode-set
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0x0F, 1));  // octet-align=1
    auto localFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            m_pLocalProfile->GetPayloadAt(0)->GetFmtp());
    localFmtp->SetVisibleOctetAlign(IMS_TRUE);
    localFmtp->SetVisibleModeSet(IMS_TRUE);

    // Peer profile supports octet-align=1 and has an overlapping mode-set
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0x3F, 1));  // octet-align=1
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // Check negotiated mode-set (intersection of 0x0F and 0x3F is 0x3F)
    EXPECT_EQ(pNegoFmtp->GetModeSetList(), 0x3F);
    // Since negotiated mode-set is not 0, visibility should be true.
    EXPECT_TRUE(pNegoFmtp->IsModeSetVisible());

    // Check negotiated octet-align (should take local's value)
    EXPECT_EQ(pNegoFmtp->GetOctetAlign(), 1);
    // Since local octet-align is 1, visibility should be true.
    EXPECT_TRUE(pNegoFmtp->IsOctetAlignVisible());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAmrFmtpPeerModeSetNotVisible)
{
    // Arrange
    // Local profile has a specific mode-set and octet-align=0 (not visible)
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0x0F, 0));
    auto localFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            m_pLocalProfile->GetPayloadAt(0)->GetFmtp());
    localFmtp->SetVisibleModeSet(IMS_TRUE);
    localFmtp->SetVisibleOctetAlign(IMS_FALSE);

    // Peer profile has no mode-set (mode-set=0) and it's not visible.
    // octet-align is also 0.
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0, 0));
    auto peerFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            m_pPeerProfile->GetPayloadAt(0)->GetFmtp());
    peerFmtp->SetVisibleModeSet(IMS_FALSE);
    peerFmtp->SetVisibleOctetAlign(IMS_FALSE);
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // Check negotiated mode-set 0x0F
    EXPECT_EQ(pNegoFmtp->GetModeSetList(), 0x0F);
    // Since negotiated mode-set is 0, visibility should be inherited from the peer (false).
    EXPECT_TRUE(pNegoFmtp->IsModeSetVisible());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAmrWbFmtpFullModeSetShouldBeNotVisible)
{
    // Arrange
    // Local profile has a Full mode-set
    m_pLocalProfile->AddPayload(
            CreateAmrWbPayload(kLocalPayload, CodecAudioConfig::FULL_MODESET_AMRWB, 0));
    auto localFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            m_pLocalProfile->GetPayloadAt(0)->GetFmtp());
    // Force to set mode-set visible
    localFmtp->SetVisibleModeSet(IMS_TRUE);

    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0x000, 0));
    auto peerFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            m_pPeerProfile->GetPayloadAt(0)->GetFmtp());
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // Check negotiated mode-set FULL_MODESET_AMRWB
    EXPECT_EQ(pNegoFmtp->GetModeSetList(), CodecAudioConfig::FULL_MODESET_AMRWB);
    // Since negotiated mode-set is full modeset, The mode-set should NOT be visible.
    EXPECT_FALSE(pNegoFmtp->IsModeSetVisible());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAmrNbFmtpFullModeSetShouldBeNotVisible)
{
    // Arrange
    // Local profile has a Full mode-set for AMR-NB
    m_pLocalProfile->AddPayload(
            CreateAmrPayload(kLocalPayload, CodecAudioConfig::FULL_MODESET_AMRNB, 0));
    auto localFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            m_pLocalProfile->GetPayloadAt(0)->GetFmtp());
    // Force to set mode-set visible
    localFmtp->SetVisibleModeSet(IMS_TRUE);

    m_pPeerProfile->AddPayload(CreateAmrPayload(kPeerPayload, 0x000, 0));
    auto peerFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            m_pPeerProfile->GetPayloadAt(0)->GetFmtp());
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // Check negotiated mode-set is FULL_MODESET_AMRNB
    EXPECT_EQ(pNegoFmtp->GetModeSetList(), CodecAudioConfig::FULL_MODESET_AMRNB);
    // Since negotiated mode-set is full modeset, The mode-set should NOT be visible.
    EXPECT_FALSE(pNegoFmtp->IsModeSetVisible());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAmrWbFmtpFullModeSetPeerVisibleShouldBeVisible)
{
    // Arrange
    // Local profile has a full mode-set
    m_pLocalProfile->AddPayload(
            CreateAmrWbPayload(kLocalPayload, CodecAudioConfig::FULL_MODESET_AMRWB, 0));
    auto localFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            m_pLocalProfile->GetPayloadAt(0)->GetFmtp());
    // Force to set mode-set visible
    localFmtp->SetVisibleModeSet(IMS_TRUE);

    // Peer profile also has a full mode-set, and it is visible
    m_pPeerProfile->AddPayload(
            CreateAmrWbPayload(kPeerPayload, CodecAudioConfig::FULL_MODESET_AMRWB, 0));
    auto peerFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            m_pPeerProfile->GetPayloadAt(0)->GetFmtp());
    // Force to set peer's mode-set visible to test the 'else' branch
    peerFmtp->SetVisibleModeSet(IMS_TRUE);
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // Check negotiated mode-set is FULL_MODESET_AMRWB
    EXPECT_EQ(pNegoFmtp->GetModeSetList(), CodecAudioConfig::FULL_MODESET_AMRWB);
    // Since peer's full modeset was visible, the negotiated mode-set should also be visible.
    EXPECT_TRUE(pNegoFmtp->IsModeSetVisible());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAmrNbFmtpFullModeSetPeerVisibleShouldBeVisible)
{
    // Arrange
    // Local profile has a Full mode-set for AMR-NB
    m_pLocalProfile->AddPayload(
            CreateAmrPayload(kLocalPayload, CodecAudioConfig::FULL_MODESET_AMRNB, 0));
    auto localFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            m_pLocalProfile->GetPayloadAt(0)->GetFmtp());
    // Force to set mode-set visible
    localFmtp->SetVisibleModeSet(IMS_TRUE);

    // Peer profile also has a Full mode-set, and it is visible
    m_pPeerProfile->AddPayload(
            CreateAmrPayload(kPeerPayload, CodecAudioConfig::FULL_MODESET_AMRNB, 0));
    auto peerFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(
            m_pPeerProfile->GetPayloadAt(0)->GetFmtp());
    // Force to set peer's mode-set visible to test the 'else' branch
    peerFmtp->SetVisibleModeSet(IMS_TRUE);
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // Check negotiated mode-set is FULL_MODESET_AMRNB
    EXPECT_EQ(pNegoFmtp->GetModeSetList(), CodecAudioConfig::FULL_MODESET_AMRNB);
    // Since peer's full modeset was visible, the negotiated mode-set should also be visible.
    EXPECT_TRUE(pNegoFmtp->IsModeSetVisible());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsOfferReceivedReturnsTrue)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateEvsPayload(98, 0x0F, 0xFFF));  // Supports all BW/BR
    m_pPeerProfile->AddPayload(
            CreateEvsPayload(101, 0x07, 0x0FF));  // Supports NB/WB/SWB, lower BR set
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6006);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), 101);  // Should take peer's PT
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
    EXPECT_GT(m_pNegotiatedProfile->GetDataPort(), 0);
    ASSERT_NE(pNegoPayload->GetFmtp(), IMS_NULL);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    // Check negotiated BW/BR (intersection)
    EXPECT_EQ(pNegoFmtp->GetBwList(), 0x07);   // NB & WB & SWB
    EXPECT_EQ(pNegoFmtp->GetBrList(), 0x0FF);  // Lower BR set
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);
    EXPECT_EQ(m_pLocalProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(),
            101);  // Local PT updated
}

TEST_F(AudioProfileNegotiatorTest, NegotiatePcmOfferReceivedReturnsTrue)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreatePcmuPayload(0));
    m_pPeerProfile->AddPayload(CreatePcmuPayload(0));  // PCMU often uses PT 0
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6008);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadType(), "PCMU");
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), 0);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
    EXPECT_GT(m_pNegotiatedProfile->GetDataPort(), 0);
    EXPECT_EQ(m_pLocalProfile->GetNegotiatedPayloadIndex(), 0);
    EXPECT_EQ(m_pPeerProfile->GetNegotiatedPayloadIndex(), 0);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateTelephoneEventMatchingRateAppended)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));  // AMR-WB (16k)
    m_pLocalProfile->AddPayload(CreateTelephoneEventPayload(101, 16000));

    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));   // Peer AMR-WB (16k)
    m_pPeerProfile->AddPayload(CreateTelephoneEventPayload(106, 16000));  // Peer TE (16k)
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6010);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    // Expecting AMR-WB and matching telephone-event
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 2);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadType(), "AMR-WB");
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), kPeerPayload);
    EXPECT_EQ(
            m_pNegotiatedProfile->GetPayloadAt(1)->GetRtpMap().GetPayloadType(), "telephone-event");
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(1)->GetRtpMap().GetPayloadNumber(), 106);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(1)->GetRtpMap().GetSamplingRate(), 16000);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateTelephoneEvent8kAcceptedAppended)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));  // AMR-WB (16k)
    m_pLocalProfile->AddPayload(CreateTelephoneEventPayload(101, 8000));   // Local supports 8k TE

    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));  // Peer AMR-WB (16k)
    m_pPeerProfile->AddPayload(CreateTelephoneEventPayload(106, 8000));  // Peer offers 8k TE
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6012);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    // Expecting AMR-WB and the 8k telephone-event (as fallback)
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 2);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadType(), "AMR-WB");
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), kPeerPayload);
    EXPECT_EQ(
            m_pNegotiatedProfile->GetPayloadAt(1)->GetRtpMap().GetPayloadType(), "telephone-event");
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(1)->GetRtpMap().GetPayloadNumber(), 106);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(1)->GetRtpMap().GetSamplingRate(),
            8000);  // Should accept 8k
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateNoMatchingAudioPayloadReturnsFalse)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));  // Local only AMR-WB
    m_pPeerProfile->AddPayload(CreateEvsPayload(101, 0x07, 0x0FF));        // Peer only EVS
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6014);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_FALSE(bResult);  // Should fail as no common audio codec found
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), m_pLocalProfile->GetPayloadListSize());
    EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), 0);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateNoMatchingPayloadResetsProfile)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateEvsPayload(101, 0x07, 0x0FF));

    // --- Case 1: Peer port is non-zero ---
    {
        m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
        m_pPeerProfile->SetDataPort(6014);

        // Act
        IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
                IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

        // Assert
        EXPECT_FALSE(bResult);  // Should fail as no common audio codec found.
        // Profile should be reset based on local (bPeerPreferred = false)
        EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
        EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadType(), "AMR-WB");
        EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), 0);
        EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
    }

    // --- Case 2: Peer port is zero ---
    {
        m_pPeerProfile->SetDirection(MEDIA_DIRECTION_INACTIVE);
        m_pPeerProfile->SetDataPort(0);

        // Act
        IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
                IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

        // Assert
        EXPECT_TRUE(bResult);  // Should succeed by resetting.
        // Profile should be reset based on peer (bPeerPreferred = true)
        EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
        EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadType(), "EVS");
        EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), 0);
        EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
    }
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAmrPayloadFormatRelaxedMatching)
{
    // Arrange
    // Local and Peer profiles have different octet-align values but compatible mode-sets.
    // Local prefers octet-align=0, Peer prefers octet-align=1.
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, (1 << 8), 0));  // mode 8, oa=0
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, (1 << 8), 1));    // mode 8, oa=1
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // --- Test Case 1: bAmrPayloadFormatRelaxedMatching = false ---
    // Negotiation should fail because octet-align values do not match and preference is false.
    ON_CALL(m_objMockConfig, IsAmrPayloadFormatRelaxedMatching()).WillByDefault(Return(IMS_FALSE));

    // Act 1
    IMS_BOOL bResult1 = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert 1
    EXPECT_FALSE(bResult1);  // Expect failure
    // The negotiated profile should be reset (inactive)
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);

    // --- Test Case 2: bAmrPayloadFormatRelaxedMatching = true ---
    // Negotiation should succeed. The negotiated octet-align should match the peer's preference,
    // and the mode-set should be the common mode.
    ON_CALL(m_objMockConfig, IsAmrPayloadFormatRelaxedMatching()).WillByDefault(Return(IMS_TRUE));
    // Reset negotiated profile for the second run
    m_pNegotiatedProfile = std::make_unique<AudioProfile>();
    m_pLocalProfile->SetNegotiatedPayloadIndex(-1);
    m_pPeerProfile->SetNegotiatedPayloadIndex(-1);

    // Act 2
    IMS_BOOL bResult2 = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert 2
    EXPECT_TRUE(bResult2);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload2 = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload2, nullptr);
    auto pNegoFmtp2 = std::static_pointer_cast<AudioProfile::AmrFmtp>(pNegoPayload2->GetFmtp());
    ASSERT_NE(pNegoFmtp2, nullptr);
    EXPECT_EQ(pNegoFmtp2->GetOctetAlign(), 1);     // Should keep peer's value (1)
    EXPECT_EQ(pNegoFmtp2->GetModeSetList(), 256);  // Should be the common mode 8 (1 << 8)
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEmptyPeerPayloadListCopiesLocal)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pLocalProfile->AddPayload(CreateTelephoneEventPayload(101, 16000));
    // m_pPeerProfile has empty payload list (default from SetUp)
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);  // Direction might still be set
    m_pPeerProfile->SetDataPort(6016);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);  // Negotiation succeeds by copying local
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), m_pLocalProfile->GetPayloadListSize());
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadType(), "AMR-WB");
    EXPECT_EQ(
            m_pNegotiatedProfile->GetPayloadAt(1)->GetRtpMap().GetPayloadType(), "telephone-event");
    EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), 0);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsOfferReceivedCompareEvsBwBrModeReturnsTrue)
{
    // Arrange: Setup compatible EVS profiles (e.g., both Category A)
    // This should succeed on the first pass using CompareEvsBwBrMode.
    m_pLocalProfile->AddPayload(
            CreateEvsPayload(98, 0x0F, 0xFFF));  // Local: EVS NB/WB/SWB/FB, Full BR
    m_pPeerProfile->AddPayload(
            CreateEvsPayload(101, 0x07, 0x0FF));  // Peer: EVS NB/WB/SWB, Lower BR set
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6020);

    // Act: Negotiate with offer received (triggers IR.92 rel15 logic path first)
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);  // Expect success
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), 101);
    ASSERT_NE(pNegoPayload->GetFmtp(), IMS_NULL);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    // Check negotiated BW/BR (intersection from CompareEvsBwBrMode)
    EXPECT_EQ(pNegoFmtp->GetBwList(), 0x07);   // NB & WB & SWB (0x0F & 0x07)
    EXPECT_EQ(pNegoFmtp->GetBrList(), 0x0FF);  // Lower BR set (0xFFF & 0x0FF)
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsOfferReceivedCompareEvsBwBrModeLegacyReturnsTrue)
{
    // Arrange: Setup EVS profiles where CompareEvsBwBrMode fails but Legacy succeeds.
    // Example: Local is Cat B (SWB only), Peer offers Cat A (NB/WB/SWB/FB).
    // CompareEvsBwBrMode (IR.92 rel15) should reject this in MT case initially.
    m_pLocalProfile->AddPayload(
            CreateEvsPayload(99, 0x04, 0x01F));  // Local: EVS SWB only (Cat B1/B2), ~13.2k BR
    m_pPeerProfile->AddPayload(
            CreateEvsPayload(102, 0x0F, 0xFFF));  // Peer: EVS NB/WB/SWB/FB (Cat A), Full BR
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6022);

    // Act: Negotiate with offer received.
    // Expect CompareEvsBwBrMode to fail (nNegoEntry=0), then CompareEvsBwBrModeLegacy to succeed
    // (nNegoEntry=1).
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);  // Expect success via legacy path
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), 102);
    ASSERT_NE(pNegoPayload->GetFmtp(), IMS_NULL);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    // Check negotiated BW/BR (intersection from CompareEvsBwBrModeLegacy)
    EXPECT_EQ(pNegoFmtp->GetBwList(), 0x04);   // SWB only (0x04 & 0x0F)
    EXPECT_EQ(pNegoFmtp->GetBrList(), 0x01F);  // Lower BR set (0x01F & 0xFFF)
}

TEST_F(AudioProfileNegotiatorTest, NegotiateDirectionOfferPeerSendReturnsReceive)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND);  // Peer offers to send only
    m_pPeerProfile->SetDataPort(6030);

    // Act: Negotiate with offer received
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);  // Ensure payload was negotiated
    // If peer offers SEND, we should negotiate to RECEIVE
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_RECEIVE);
    EXPECT_GT(m_pNegotiatedProfile->GetDataPort(), 0);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateDirectionOfferPeerReceiveReturnsSend)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_RECEIVE);  // Peer offers to receive only
    m_pPeerProfile->SetDataPort(6032);

    // Act: Negotiate with offer received
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);  // Ensure payload was negotiated
    // If peer offers RECEIVE, we should negotiate to SEND
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND);
    EXPECT_GT(m_pNegotiatedProfile->GetDataPort(), 0);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateDirectionInactiveReturnsTrue)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_INACTIVE);
    m_pPeerProfile->SetDataPort(0);  // Inactive often has port 0

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    // Payload might still be negotiated even if direction is inactive
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadType(), "AMR-WB");
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
    EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), 0);  // Port should remain 0 for inactive
}

TEST_F(AudioProfileNegotiatorTest, NegotiateBandwidthRemoteOption)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0x02));
    m_pLocalProfile->SetBandwidthRs(300);
    m_pLocalProfile->SetBandwidthRr(300);
    m_pLocalProfile->SetBandwidthAs(32);

    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0x02));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6040);
    m_pPeerProfile->SetBandwidthRs(500);  // Different from local
    m_pPeerProfile->SetBandwidthRr(600);  // Different from local
    m_pPeerProfile->SetBandwidthAs(40);   // Different from local

    // Configure mock to use remote bandwidth values
    EXPECT_CALL(m_objMockConfig, GetBandwidthNegoOption())
            .WillRepeatedly(Return(MediaConfiguration::BW_OPTION_REMOTE_VALUE));

    // Act: Negotiate with offer received
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRs(), 500);
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRr(), 600);
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthAs(), 34);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateBandwidthLocalOption)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0x02));
    m_pLocalProfile->SetBandwidthRs(350);  // Use distinct local values
    m_pLocalProfile->SetBandwidthRr(450);
    m_pLocalProfile->SetBandwidthAs(32);

    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0x02));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6042);
    m_pPeerProfile->SetBandwidthRs(500);
    m_pPeerProfile->SetBandwidthRr(600);
    m_pPeerProfile->SetBandwidthAs(40);

    // Configure mock to use local bandwidth values (default, but explicit here)
    EXPECT_CALL(m_objMockConfig, GetBandwidthNegoOption())
            .WillRepeatedly(Return(MediaConfiguration::BW_OPTION_LOCAL_VALUE));

    // Act: Negotiate with offer received
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    // Check that negotiated bandwidth matches the local's values due to config option
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRs(), 350);
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthRr(), 450);
    // AS should be the calculated codec AS when BW_OPTION_LOCAL_VALUE is used in MT case.
    EXPECT_EQ(m_pNegotiatedProfile->GetBandwidthAs(), 34);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateRtcpIntervalActive)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);  // Active direction
    m_pPeerProfile->SetDataPort(6044);

    // Act: Negotiate with offer received
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
    // Should use the active interval because direction is SEND_RECEIVE
    EXPECT_EQ(m_pNegotiatedProfile->GetRtcpInterval(), 5);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateRtcpIntervalHold)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(
            MEDIA_DIRECTION_SEND);  // Not SEND_RECEIVE, triggers hold interval logic
    m_pPeerProfile->SetDataPort(6046);

    // Act: Negotiate with offer received
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_RECEIVE);  // Should be RECEIVE
    // Should use the hold interval because direction is not SEND_RECEIVE
    EXPECT_EQ(m_pNegotiatedProfile->GetRtcpInterval(), 3);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateRtcpIntervalDisabledWhenRsRrZero)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pLocalProfile->SetBandwidthRs(300);  // Local has non-zero RS/RR
    m_pLocalProfile->SetBandwidthRr(300);

    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6048);
    m_pPeerProfile->SetBandwidthRs(0);  // Peer has zero RS/RR
    m_pPeerProfile->SetBandwidthRr(0);

    // Configure mock to use remote bandwidth values, resulting in negotiated RS/RR = 0
    EXPECT_CALL(m_objMockConfig, GetBandwidthNegoOption())
            .WillRepeatedly(Return(MediaConfiguration::BW_OPTION_REMOTE_VALUE));

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

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsCompareEvsBwBrModeLegacyPath)
{
    // Arrange: Create a scenario where CompareEvsBwBrMode (IR.92 rel15) fails,
    // forcing a fallback to CompareEvsBwBrModeLegacy.
    // Example: Local supports only SWB, Peer offers NB/WB/SWB/FB.
    // The initial CompareEvsBw (in CompareEvsBwBrMode) will fail because of the strict
    // BW_OPTION_REMOTE_VALUE logic in MT case for B-type codecs.
    // Then CompareEvsBwBrModeLegacy should be called.

    // Local: EVS SWB only (0x04), Full BR (0xFFF)
    m_pLocalProfile->AddPayload(CreateEvsPayload(kLocalPayload, 0x04, 0xFFF));
    // Peer: EVS NB/WB/SWB/FB (0x0F), Full BR (0xFFF)
    m_pPeerProfile->AddPayload(CreateEvsPayload(kPeerPayload, 0x0F, 0xFFF));

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6050);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), kPeerPayload);

    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);
    // In this specific scenario, the legacy mode should negotiate the intersection of BW/BR.
    // Local BW: 0x04 (SWB), Peer BW: 0x0F (NB|WB|SWB|FB) -> Intersection: 0x04 (SWB)
    EXPECT_EQ(pNegoFmtp->GetBwList(), 0x04);
    EXPECT_EQ(pNegoFmtp->GetBrList(), 0xFFF);  // Full BR list
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsCompareEvsModeAmrIoMode)
{
    // Arrange: Test CompareEvsMode for AMR IO Mode
    // Local: EVS AMR IO Mode, mode-set=0x03 (0,1)
    auto pLocalEvs = CreateEvsPayload(kLocalPayload, 0, 0);
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pLocalEvs->GetFmtp())->SetEvsModeSwitch(1);
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pLocalEvs->GetFmtp())->SetModeSetList(0x03);
    m_pLocalProfile->AddPayload(pLocalEvs);

    // Peer: EVS AMR IO Mode, mode-set=0x05 (0,2)
    auto pPeerEvs = CreateEvsPayload(kPeerPayload, 0, 0);
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerEvs->GetFmtp())->SetEvsModeSwitch(1);
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerEvs->GetFmtp())->SetModeSetList(0x05);
    m_pPeerProfile->AddPayload(pPeerEvs);

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6052);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);
    // Intersection of 0x03 and 0x05 is 0x01 (mode 0)
    EXPECT_EQ(pNegoFmtp->GetModeSetList(), 0x01);
    EXPECT_EQ(pNegoFmtp->GetEvsModeSwitch(), 1);  // Should remain AMR IO mode
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsCompareEvsModeAmrIoModeNoIntersection)
{
    // Arrange: Test CompareEvsMode for AMR IO Mode with no intersection
    // Local: EVS AMR IO Mode, mode-set=0x02 (1)
    auto pLocalEvs = CreateEvsPayload(kLocalPayload, 0, 0);
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pLocalEvs->GetFmtp())->SetEvsModeSwitch(1);
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pLocalEvs->GetFmtp())->SetModeSetList(0x02);
    m_pLocalProfile->AddPayload(pLocalEvs);

    // Peer: EVS AMR IO Mode, mode-set=0x04 (2)
    auto pPeerEvs = CreateEvsPayload(kPeerPayload, 0, 0);
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerEvs->GetFmtp())->SetEvsModeSwitch(1);
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerEvs->GetFmtp())->SetModeSetList(0x04);
    m_pPeerProfile->AddPayload(pPeerEvs);

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6054);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_FALSE(bResult);  // Should fail as no common mode-set
    EXPECT_EQ(
            m_pNegotiatedProfile->GetPayloadListSize(), 1);  // ResetNegotiatedProfile copies local
    EXPECT_EQ(m_pNegotiatedProfile->GetDataPort(), 0);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsUniDirectionBrBw)
{
    // Arrange: Test NegotiateUniDirectionBrBw
    // Local: EVS, full BW/BR lists
    auto pLocalEvs = CreateEvsPayload(kLocalPayload, 0x0F, 0xFFF);
    m_pLocalProfile->AddPayload(pLocalEvs);

    // Peer: EVS, with specific br-send and bw-recv
    auto pPeerEvs = CreateEvsPayload(kPeerPayload, 0x0F, 0xFFF);
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerEvs->GetFmtp())
            ->SetBrSend(0x03);  // 5.9, 7.2
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerEvs->GetFmtp())->SetBwRecv(0x01);  // NB
    m_pPeerProfile->AddPayload(pPeerEvs);

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6056);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // Negotiated br-send should be intersection with negotiated br-list
    EXPECT_EQ(pNegoFmtp->GetBrSend(), 15);
    // Negotiated bw-recv should be intersection with negotiated bw-list
    EXPECT_EQ(pNegoFmtp->GetBwRecv(), 4095);
    // Other unidirectional parameters not set by peer should remain 0
    EXPECT_EQ(pNegoFmtp->GetBrRecv(), 0);
    EXPECT_EQ(pNegoFmtp->GetBwSend(), 0);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsCmrAndModeSet)
{
    // Arrange: Test NegotiateCmr and NegotiateModeSet
    // Local: EVS, SendCmrEnabled=true, EvsModeSwitch=0
    auto pLocalEvs = CreateEvsPayload(kLocalPayload, 0x0F, 0x01F);  // BR 5.9-13.2 (5 modes)
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pLocalEvs->GetFmtp())->SetSendCmr(IMS_TRUE);
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pLocalEvs->GetFmtp())->SetEvsModeSwitch(0);
    m_pLocalProfile->AddPayload(pLocalEvs);

    // Peer: EVS, EvsModeSwitch=0
    auto pPeerEvs = CreateEvsPayload(kPeerPayload, 0x0F, 0x01F);  // BR 5.9-13.2 (5 modes)
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerEvs->GetFmtp())->SetEvsModeSwitch(0);
    m_pPeerProfile->AddPayload(pPeerEvs);

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6058);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // NegotiateCmr: Should be true as local is true and BR count > 1
    EXPECT_TRUE(pNegoFmtp->IsSendCmrEnabled());
    // NegotiateModeSet: Should add mode-set=0,1,2 because max BR is 13.2kbps (0x10)
    EXPECT_EQ(pNegoFmtp->GetModeSetList(), 0x07);
    EXPECT_TRUE(pNegoFmtp->IsModeSetVisible());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsCmrWithModeSwitchOne)
{
    // Arrange: Test NegotiateCmr when evs-mode-switch is 1
    // Local: EVS, br=5.9-24.4, evs-mode-switch=1, cmr=-1
    // 0x0F: bw=nb-swb, 0x01FF: br=5.9-24.4
    auto pLocalEvs = CreateEvsPayload(kLocalPayload, 0x0F, 0x01FF);
    auto pLocalFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pLocalEvs->GetFmtp());
    pLocalFmtp->SetEvsModeSwitch(1);
    pLocalFmtp->SetCmr(-1);
    pLocalFmtp->SetShowCmr(IMS_TRUE);
    m_pLocalProfile->AddPayload(pLocalEvs);

    // Peer: EVS, br=5.9-24.4, evs-mode-switch=1, cmr=-1
    auto pPeerEvs = CreateEvsPayload(kPeerPayload, 0x0F, 0x01FF);
    auto pPeerFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerEvs->GetFmtp());
    pPeerFmtp->SetEvsModeSwitch(1);
    pPeerFmtp->SetCmr(-1);
    pPeerFmtp->SetShowCmr(IMS_TRUE);
    m_pPeerProfile->AddPayload(pPeerEvs);

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6058);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // Verify: Even with evs-mode-switch=1, cmr should be negotiated and visible
    EXPECT_EQ(pNegoFmtp->GetEvsModeSwitch(), 1);
    EXPECT_EQ(pNegoFmtp->GetCmr(), -1);
    EXPECT_TRUE(pNegoFmtp->IsCmrVisible());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsCmrDisabledWhenBrCountIsOne)
{
    // Arrange: Test NegotiateCmr when bitrate count is 1
    // Local: EVS, SendCmrEnabled=true, EvsModeSwitch=0
    auto pLocalEvs = CreateEvsPayload(kLocalPayload, 0x0F, 0x01);  // BR 5.9kbps (1 mode)
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pLocalEvs->GetFmtp())->SetSendCmr(IMS_TRUE);
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pLocalEvs->GetFmtp())->SetEvsModeSwitch(0);
    m_pLocalProfile->AddPayload(pLocalEvs);

    // Peer: EVS, EvsModeSwitch=0
    auto pPeerEvs = CreateEvsPayload(kPeerPayload, 0x0F, 0x01);  // BR 5.9kbps (1 mode)
    std::static_pointer_cast<AudioProfile::EvsFmtp>(pPeerEvs->GetFmtp())->SetEvsModeSwitch(0);
    m_pPeerProfile->AddPayload(pPeerEvs);

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6060);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // NegotiateCmr: Should be false as BR count is 1
    EXPECT_FALSE(pNegoFmtp->IsSendCmrEnabled());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateRtcpXr)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6062);

    // Enable RTCP-XR on local profile
    m_pLocalProfile->SetSupportRtcpXr(IMS_TRUE);
    m_pLocalProfile->GetRtcpXrAttr().SetSupportVoipMetrics(IMS_TRUE);
    m_pLocalProfile->GetRtcpXrAttr().SetSupportStatisticMetrics(IMS_TRUE);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(m_pNegotiatedProfile->IsRtcpXrSupported());
    EXPECT_TRUE(m_pNegotiatedProfile->GetRtcpXrAttr().IsVoipMetricsSupported());
    EXPECT_TRUE(m_pNegotiatedProfile->GetRtcpXrAttr().IsStatisticMetricsSupported());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateRtcpXrNotSupported)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6064);

    // RTCP-XR not enabled on local profile (default)

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_FALSE(m_pNegotiatedProfile->IsRtcpXrSupported());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateRtcpXrDirectionNotSendReceive)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND);  // Not SEND_RECEIVE
    m_pPeerProfile->SetDataPort(6066);

    // Enable RTCP-XR on local profile
    m_pLocalProfile->SetSupportRtcpXr(IMS_TRUE);
    m_pLocalProfile->GetRtcpXrAttr().SetSupportVoipMetrics(IMS_TRUE);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_FALSE(m_pNegotiatedProfile->IsRtcpXrSupported());  // Should be false due to direction
}

TEST_F(AudioProfileNegotiatorTest, NegotiatePtimeMaxPtime)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6068);

    // Set local ptime and maxptime
    m_pLocalProfile->SetPtime(10);  // Less than 20
    m_pLocalProfile->SetMaxPtime(100);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPtime(), 20);  // Should be clamped to 20
    EXPECT_EQ(m_pNegotiatedProfile->GetMaxPtime(), 100);
}

TEST_F(AudioProfileNegotiatorTest, NegotiatePtimeMaxPtimeDefaults)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6070);

    // Local ptime and maxptime are 0 (default)

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetPtime(), 20);      // Default to 20
    EXPECT_EQ(m_pNegotiatedProfile->GetMaxPtime(), 240);  // Default to 240
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAnbrSupported)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6072);

    // Both local and peer support ANBR
    m_pLocalProfile->SetAnbr(IMS_TRUE);
    m_pPeerProfile->SetAnbr(IMS_TRUE);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_TRUE(m_pNegotiatedProfile->IsAnbrSupported());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAnbrNotSupportedByLocal)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6074);

    // Local does not support ANBR, peer does
    m_pLocalProfile->SetAnbr(IMS_FALSE);
    m_pPeerProfile->SetAnbr(IMS_TRUE);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_FALSE(m_pNegotiatedProfile->IsAnbrSupported());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAnbrNotSupportedByPeer)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6076);

    // Local supports ANBR, peer does not
    m_pLocalProfile->SetAnbr(IMS_TRUE);
    m_pPeerProfile->SetAnbr(IMS_FALSE);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_FALSE(m_pNegotiatedProfile->IsAnbrSupported());
}

TEST_F(AudioProfileNegotiatorTest, NegotiateDirectionInvalidPeerDirection)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(static_cast<MEDIA_DIRECTION>(-1));  // Invalid direction
    m_pPeerProfile->SetDataPort(6078);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_FALSE(bResult);  // Negotiation should fail due to invalid direction
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_INACTIVE);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateDirectionMoSendMtReceive)
{
    // Arrange: MO case, local wants SEND, peer offers RECEIVE.
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND);

    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_RECEIVE);
    m_pPeerProfile->SetDataPort(6080);

    // Act: MO scenario (bIsOfferReceived = false)
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_FALSE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);  // Should fail due to strict direction check
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateDirectionMoStrictCheckSuccess)
{
    // Arrange: MO case, local wants SEND_RECEIVE, peer offers SEND_RECEIVE. This should pass.
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));
    m_pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);

    m_pPeerProfile->AddPayload(CreateAmrWbPayload(kPeerPayload, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6082);

    // Act: MO scenario (bIsOfferReceived = false)
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_FALSE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    EXPECT_EQ(m_pNegotiatedProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
}
TEST_F(AudioProfileNegotiatorTest, NegotiateEvsOfferB0ReceivedA1ReturnA1)
{
    m_pLocalProfile->AddPayload(
            CreateEvsPayload(98, 0x04, 0x10));  // Local: EVS B0+A1
    m_pLocalProfile->AddPayload(
            CreateEvsPayload(99, 0x07, 0x01F));
    m_pPeerProfile->AddPayload(
            CreateEvsPayload(98, 0x07, 0x01F));  // Peer: EVS A1
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6020);

    // Act: Negotiate with offer received (triggers IR.92 Table mapping)
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);  // Expect success
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), 98);
    ASSERT_NE(pNegoPayload->GetFmtp(), IMS_NULL);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    // Check negotiated BW/BR (intersection from CompareEvsBwBrMode)
    EXPECT_EQ(pNegoFmtp->GetBwList(), 0x07);   // EVS A1
    EXPECT_EQ(pNegoFmtp->GetBrList(), 0x01F);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsOfferB0ReceivedA2ReturnB1)
{
    m_pLocalProfile->AddPayload(
            CreateEvsPayload(98, 0x04, 0x10));  // Local: EVS B0+A1
    m_pLocalProfile->AddPayload(
            CreateEvsPayload(99, 0x07, 0x01F));
    m_pPeerProfile->AddPayload(
            CreateEvsPayload(98, 0x07, 0x07F));  // Peer: EVS A2
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6020);

    // Act: Negotiate with offer received (triggers IR.92 Table mapping)
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);  // Expect success
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), 98);
    ASSERT_NE(pNegoPayload->GetFmtp(), IMS_NULL);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    // Check negotiated BW/BR (intersection from CompareEvsBwBrMode)
    EXPECT_EQ(pNegoFmtp->GetBwList(), 0x07);   // EVS A1
    EXPECT_EQ(pNegoFmtp->GetBrList(), 0x01F);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsOfferB2ReceivedA2ReturnA2)
{
    // Arrange: Setup compatible EVS profiles (e.g., EVS B2+A2)
    // This should succeed on the return of non-subset bitrate list but follow to IR92 table.
    m_pLocalProfile->AddPayload(
            CreateEvsPayload(98, 0x04, 0x78));  // Local: EVS B2+A2
    m_pLocalProfile->AddPayload(
            CreateEvsPayload(99, 0x07, 0x07F));
    m_pPeerProfile->AddPayload(
            CreateEvsPayload(98, 0x07, 0x07F));  // Peer: EVS A2
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6020);

    // Act: Negotiate with offer received (triggers IR.92 Table mapping)
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);  // Expect success
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), 98);
    ASSERT_NE(pNegoPayload->GetFmtp(), IMS_NULL);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    // Check negotiated BW/BR (intersection from CompareEvsBwBrMode)
    EXPECT_EQ(pNegoFmtp->GetBwList(), 0x07);   // EVS A2
    EXPECT_EQ(pNegoFmtp->GetBrList(), 0x07F);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsOfferA2ReceivedB1OnlyReturnSubset)
{
    // Arrange: Setup compatible EVS profiles (e.g., EVS B2+A2)
    // This should succeed on the return of non-subset bitrate list but follow to IR92 table.
    m_pLocalProfile->AddPayload(
            CreateEvsPayload(98, 0x07, 0x07F));  // Local: EVS A2
    m_pPeerProfile->AddPayload(
            CreateEvsPayload(98, 0x04, 0x18));  // Peer: EVS B1 only
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6020);

    // Act: Negotiate with offer received (triggers IR.92 Table mapping)
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);  // Expect success
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), 98);
    ASSERT_NE(pNegoPayload->GetFmtp(), IMS_NULL);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    // Check negotiated BW/BR (intersection from CompareEvsBwBrMode)
    EXPECT_EQ(pNegoFmtp->GetBwList(), 0x04);   // Subset of B1+A2
    EXPECT_EQ(pNegoFmtp->GetBrList(), 0x018);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsOfferA2ReceivedNonIR92ReturnSubset)
{
    // Arrange: Setup compatible EVS profiles (e.g., EVS B2+A2)
    // This should succeed on the return of non-subset bitrate list but follow to IR92 table.
    m_pLocalProfile->AddPayload(
            CreateEvsPayload(98, 0x07, 0x07F));  // Local: EVS B1
    m_pPeerProfile->AddPayload(
            CreateEvsPayload(98, 0x04, 0x01F));  // Peer: Non IR92 config
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6020);

    // Act: Negotiate with offer received (triggers IR.92 Table mapping)
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);  // Expect success
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), 98);
    ASSERT_NE(pNegoPayload->GetFmtp(), IMS_NULL);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    // Check negotiated BW/BR (intersection from CompareEvsBwBrMode)
    EXPECT_EQ(pNegoFmtp->GetBwList(), 0x04);   // Subset of offer+answer
    EXPECT_EQ(pNegoFmtp->GetBrList(), 0x01F);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsWithMismatchedBandwidthForSamePayload)
{
    // Arrange
    // Local has EVS PT 127 for SWB and PT 126 for WB.
    m_pLocalProfile->AddPayload(CreateEvsPayload(127, EVS_BW_SWB, 0xFFF));
    m_pLocalProfile->AddPayload(CreateEvsPayload(126, EVS_BW_WB, 0xFFF));

    // Peer offers EVS with PT 127 but for WB.
    m_pPeerProfile->AddPayload(CreateEvsPayload(127, EVS_BW_WB, 0xFFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    // Negotiation should succeed by matching the peer's WB offer with the local PT 126 WB config.
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    // The negotiated payload number should be the one from the peer's offer.
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), 127);

    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);
    // The negotiated bandwidth should be WB.
    EXPECT_EQ(pNegoFmtp->GetBwList(), EVS_BW_WB);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsWithBandwidthSubset)
{
    // Arrange
    // Local has EVS PT 127 supporting a range of bandwidths (NB-SWB).
    m_pLocalProfile->AddPayload(CreateEvsPayload(127, EVS_BW_NB | EVS_BW_WB | EVS_BW_SWB, 0xFFF));
    // Also add another EVS config to ensure the correct one is chosen.
    m_pLocalProfile->AddPayload(CreateEvsPayload(126, EVS_BW_WB, 0xFFF));

    // Peer offers EVS with PT 127 but only for WB, which is a subset of the local offer.
    m_pPeerProfile->AddPayload(CreateEvsPayload(127, EVS_BW_WB, 0xFFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    // Negotiation should succeed with the intersection of bandwidths.
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    auto pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
    // The negotiated payload number should be the one from the peer's offer.
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadNumber(), 127);

    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);
    // The negotiated bandwidth should be the intersection, which is WB.
    EXPECT_EQ(pNegoFmtp->GetBwList(), EVS_BW_WB);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsPeerFmtpNullReturnsTrue)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateEvsPayload(kLocalPayload, 0x0F, 0xFFF));

    auto pPeerPayload = new AudioProfile::Payload();
    pPeerPayload->GetRtpMap().SetPayloadType("EVS");
    pPeerPayload->GetRtpMap().SetSamplingRate(16000);
    pPeerPayload->GetRtpMap().SetPayloadNumber(kPeerPayload);
    pPeerPayload->SetFmtp(IMS_NULL);  // Peer FMTP is null
    m_pPeerProfile->AddPayload(pPeerPayload);

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadType(), "EVS");
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), kPeerPayload);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAmrWbPeerFmtpNullReturnsTrue)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));

    auto pPeerPayload = new AudioProfile::Payload();
    pPeerPayload->GetRtpMap().SetPayloadType("AMR-WB");
    pPeerPayload->GetRtpMap().SetSamplingRate(16000);
    pPeerPayload->GetRtpMap().SetPayloadNumber(kPeerPayload);
    pPeerPayload->SetFmtp(IMS_NULL);  // Peer FMTP is null
    m_pPeerProfile->AddPayload(pPeerPayload);

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadType(), "AMR-WB");
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), kPeerPayload);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateAmrWbLocalAmrPeerFmtpNullReturnsFalse)
{
    // Arrange
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(kLocalPayload, 0xFF));  // AMR-WB (16k)

    auto pPeerPayload = new AudioProfile::Payload();
    pPeerPayload->GetRtpMap().SetPayloadType("AMR");  // AMR (8k)
    pPeerPayload->GetRtpMap().SetSamplingRate(8000);
    pPeerPayload->GetRtpMap().SetPayloadNumber(kPeerPayload);
    pPeerPayload->SetFmtp(IMS_NULL);  // Peer FMTP is null
    m_pPeerProfile->AddPayload(pPeerPayload);

    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    // Should fail because AMR-WB (16k) does not match AMR (8k)
    EXPECT_FALSE(bResult);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateWithPayloadNumberCollision)
{
    // Arrange
    // Local has AMR-WB on PT 98 and telephone-event on PT 96.
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(99, 0xFF));
    m_pLocalProfile->AddPayload(CreateAmrPayload(98, 0xFF));
    m_pLocalProfile->AddPayload(CreateTelephoneEventPayload(96, 8000));

    // Peer offers AMR-WB with PT 96. This will be negotiated.
    m_pPeerProfile->AddPayload(CreateAmrPayload(96, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act: Negotiate in an offer-received (MT) scenario.
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), 96);

    // Verify the local profile was updated correctly.
    AudioProfile::Payload* pLocalAmr = nullptr;
    AudioProfile::Payload* pLocalTelEvent = nullptr;

    for (IMS_UINT32 i = 0; i < m_pLocalProfile->GetPayloadListSize(); ++i)
    {
        auto* p = m_pLocalProfile->GetPayloadAt(i);
        if (p->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR"))
            pLocalAmr = p;
        if (p->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
            pLocalTelEvent = p;
    }

    ASSERT_NE(pLocalAmr, nullptr);
    ASSERT_NE(pLocalTelEvent, nullptr);

    // The local AMR-WB should now have the negotiated payload number.
    EXPECT_EQ(pLocalAmr->GetRtpMap().GetPayloadNumber(), 96);
    // The local telephone-event should have been re-numbered to avoid the collision.
    EXPECT_NE(pLocalTelEvent->GetRtpMap().GetPayloadNumber(), 96);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateEvsWithPayloadNumberCollision)
{
    // Local has EVS on PT 98 and telephone-event on PT 96.
    m_pLocalProfile->AddPayload(CreateEvsPayload(98, EVS_BW_SWB, 0xFFF));
    m_pLocalProfile->AddPayload(CreateAmrWbPayload(99, 0xFF));
    m_pLocalProfile->AddPayload(CreateAmrPayload(97, 0xFF));
    m_pLocalProfile->AddPayload(CreateTelephoneEventPayload(96, 8000));

    // Peer offers EVS with PT 96. This will be negotiated.
    m_pPeerProfile->AddPayload(CreateEvsPayload(96, EVS_BW_SWB, 0xFFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act: Negotiate in an offer-received (MT) scenario.
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert
    EXPECT_TRUE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), 96);
    EXPECT_EQ(m_pNegotiatedProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadType(), "EVS");

    // Verify the local profile was updated correctly.
    AudioProfile::Payload* pLocalEvs = nullptr;
    AudioProfile::Payload* pLocalTelEvent = nullptr;

    for (IMS_UINT32 i = 0; i < m_pLocalProfile->GetPayloadListSize(); ++i)
    {
        auto* p = m_pLocalProfile->GetPayloadAt(i);
        if (p->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
            pLocalEvs = p;
        if (p->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
            pLocalTelEvent = p;
    }

    ASSERT_NE(pLocalEvs, nullptr);
    ASSERT_NE(pLocalTelEvent, nullptr);

    // The local EVS codec should now have the negotiated payload number.
    EXPECT_EQ(pLocalEvs->GetRtpMap().GetPayloadNumber(), 96);
    // The local telephone-event should have been re-numbered to avoid the collision.
    EXPECT_NE(pLocalTelEvent->GetRtpMap().GetPayloadNumber(), 96);
}

TEST_F(AudioProfileNegotiatorTest, NegotiateWithPayloadNumberCollisionExhausted)
{
    // Arrange: Exhaust all dynamic payload numbers (96-127) in the local profile.
    for (IMS_UINT32 pt = 96; pt <= 127; ++pt)
    {
        m_pLocalProfile->AddPayload(CreateAmrPayload(pt, 0xFF));
    }

    // Peer offers a new payload with a number that is already used locally.
    // Since all 96-127 are used, re-numbering should fail.
    m_pPeerProfile->AddPayload(CreateAmrWbPayload(96, 0xFF));
    m_pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
    m_pPeerProfile->SetDataPort(6004);

    // Act: Negotiate in an offer-received (MT) scenario.
    IMS_BOOL bResult = m_pNegotiator->Negotiate(m_pLocalProfile.get(), m_pPeerProfile.get(),
            IMS_TRUE, m_pNegotiatedProfile.get(), &m_objMockConfig);

    // Assert: Negotiation should fail because no PT is available for re-numbering.
    EXPECT_FALSE(bResult);
}
