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
#include "audio/AudioProfileNegotiator.h"

// mocking
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
        AudioProfile::Payload* pPayload = new AudioProfile::Payload();
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
        AudioProfile::Payload* pPayload = CreateAmrWbPayload(nPayloadNum, nModeSet);
        std::static_pointer_cast<AudioProfile::AmrFmtp>(pPayload->GetFmtp())
                ->SetOctetAlign(octetAlign);
        return pPayload;
    }

    // Helper to create a basic EVS payload
    AudioProfile::Payload* CreateEvsPayload(
            IMS_UINT32 nPayloadNum, IMS_UINT32 bwList, IMS_UINT32 brList)
    {
        AudioProfile::Payload* pPayload = new AudioProfile::Payload();
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
        AudioProfile::Payload* pPayload = new AudioProfile::Payload();
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
        AudioProfile::Payload* pPayload = new AudioProfile::Payload();
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
    MockAudioConfiguration m_objMockConfig;
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
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
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
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
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
    EXPECT_FALSE(bResult);
    ASSERT_EQ(m_pNegotiatedProfile->GetPayloadListSize(), 1);
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    EXPECT_EQ(pNegoPayload->GetRtpMap().GetPayloadType(), "EVS");
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
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
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
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
    ASSERT_NE(pNegoPayload, nullptr);
    auto pNegoFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(pNegoPayload->GetFmtp());
    ASSERT_NE(pNegoFmtp, nullptr);

    // Check negotiated mode-set 0x0F
    EXPECT_EQ(pNegoFmtp->GetModeSetList(), 0x0F);
    // Since negotiated mode-set is 0, visibility should be inherited from the peer (false).
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
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
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
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
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
    AudioProfile::Payload* pNegoPayload = m_pNegotiatedProfile->GetPayloadAt(0);
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
