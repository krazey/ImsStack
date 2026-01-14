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

#include "SdpAttribute.h"
#include "audio/AudioSdpGenerator.h"

#include "core/MockISessionDescriptor.h"
#include "media/MockIMediaDescriptor.h"
#include "offeranswer/SdpMediaFormat.h"

using ::testing::_;

// for AudioFmtp
const int MODESET_LIST = 7;
const int DEFAULT_RTP_MODESET = 15;
const int MODE_CHANGE_CAPABILITY = 2;
const int MODE_CHANGE_PERIOD = 1;
const int MODE_CHANGE_NEIGHBOR = 1;
const int MAXRED = 220;

const AString STR_MODESET_LIST = "mode-set=0,1,2";
const AString STR_MODE_CHANGE_CAPABILITY = "mode-change-capability=2";
const AString STR_MODE_CHANGE_PERIOD = "mode-change-period=1";
const AString STR_MODE_CHANGE_NEIGHBOR = "mode-change-neighbor=1";
const AString STR_MAXRED = "max-red=220";

// for AmrFmtp
const int OCTET_ALIGN = 1;

const AString STR_OCTETALIGN0 = "octet-align=0";
const AString STR_OCTETALIGN1 = "octet-align=1";

// for EvsFmtp
const int DTX = 1;
const int HFONLY = 1;
const int EVS_MODE_SWITCH = 1;
const int BW_LIST = 7;
const int BRLIST = 31;
const int CMR = 1;
const int CH_AW_RECV = 1;

const AString STR_DTX = "dtx=1";
const AString STR_HFONLY = "hf-only=1";
const AString STR_EVS_MODE_SWITCH = "evs-mode-switch=1";
const AString STR_BW_LIST = "bw=nb-swb";
const AString STR_BR_LIST = "br=5.9-13.2";
const AString STR_CMR = "cmr=1";
const AString STR_CH_AW_RECV = "ch-aw-recv=1";
const AString STR_BW_SEND_LIST = "bw-send=nb-swb";
const AString STR_BW_RECV_LIST = "bw-recv=nb-swb";
const AString STR_BR_SEND_LIST = "br-send=5.9-13.2";
const AString STR_BR_RECV_LIST = "br-recv=5.9-13.2";

class AudioSdpGeneratorAmrTest : public AudioSdpGenerator, public ::testing::Test
{
public:
    std::shared_ptr<AudioProfile::AmrFmtp> m_pAmrFmtpFull;
    std::shared_ptr<AudioProfile::AmrFmtp> m_pAmrFmtpEmpty;
    std::shared_ptr<AudioProfile::AmrFmtp> m_pAmrFmtpNull;

protected:
    virtual void SetUp() override
    {
        m_pAmrFmtpFull = std::make_shared<AudioProfile::AmrFmtp>();
        m_pAmrFmtpEmpty = std::make_shared<AudioProfile::AmrFmtp>();
        m_pAmrFmtpNull = IMS_NULL;

        m_pAmrFmtpFull->SetModeSetList(MODESET_LIST);
        m_pAmrFmtpFull->SetOctetAlign(OCTET_ALIGN);
        m_pAmrFmtpFull->SetModeChangeCapability(MODE_CHANGE_CAPABILITY);
        m_pAmrFmtpFull->SetModeChangePeriod(MODE_CHANGE_PERIOD);
        m_pAmrFmtpFull->SetModeChangeNeighbor(MODE_CHANGE_NEIGHBOR);
        m_pAmrFmtpFull->SetMaxRed(MAXRED);

        m_pAmrFmtpFull->SetVisibleModeSet(IMS_TRUE);
        m_pAmrFmtpFull->SetVisibleOctetAlign(IMS_TRUE);
        m_pAmrFmtpFull->SetVisibleModeChangeCapability(IMS_TRUE);
        m_pAmrFmtpFull->SetVisibleModeChangePeriod(IMS_TRUE);
        m_pAmrFmtpFull->SetVisibleModeChangeNeighbor(IMS_TRUE);
        m_pAmrFmtpFull->SetVisibleMaxRed(IMS_TRUE);
    }

    virtual void TearDown() override {}
};

TEST_F(AudioSdpGeneratorAmrTest, TestGenerateAmrFmtp)
{
    AString strFmtp = AString::ConstNull();

    strFmtp = GenerateAmrFmtp(m_pAmrFmtpNull);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = GenerateAmrFmtp(m_pAmrFmtpEmpty);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = GenerateAmrFmtp(m_pAmrFmtpFull);

    EXPECT_EQ(strFmtp.Contains(STR_MODESET_LIST), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_OCTETALIGN1), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_MODE_CHANGE_CAPABILITY), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_MODE_CHANGE_PERIOD), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_MODE_CHANGE_NEIGHBOR), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(STR_MAXRED), IMS_TRUE);

    AString strResult = STR_MODESET_LIST;
    AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_OCTETALIGN1);
    AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_MODE_CHANGE_CAPABILITY);
    AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_MODE_CHANGE_PERIOD);
    AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_MODE_CHANGE_NEIGHBOR);
    AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(STR_MAXRED);

    EXPECT_EQ(strFmtp, strResult);
}

TEST_F(AudioSdpGeneratorAmrTest, TestAppendSeparatorIfNotEmpty)
{
    AString strFmtp = AString::ConstNull();

    AppendSeparatorIfNotEmpty(strFmtp, COMMA);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = "test";

    AppendSeparatorIfNotEmpty(strFmtp, COMMA);
    EXPECT_EQ(strFmtp, "test,");

    AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);
    EXPECT_EQ(strFmtp, "test,;");
}

TEST_F(AudioSdpGeneratorAmrTest, TestAddModeSetListToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddModeSetListToFmtp(m_pAmrFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeSetListToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeSetListToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODESET_LIST);
}

TEST_F(AudioSdpGeneratorAmrTest, TestAddOctetAlignToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddOctetAlignToFmtp(m_pAmrFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddOctetAlignToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddOctetAlignToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_OCTETALIGN1);
}

TEST_F(AudioSdpGeneratorAmrTest, TestAddModeChangeCapabilityToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddModeChangeCapabilityToFmtp(m_pAmrFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangeCapabilityToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangeCapabilityToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODE_CHANGE_CAPABILITY);
}

TEST_F(AudioSdpGeneratorAmrTest, TestAddModeChangePeriodToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddModeChangePeriodToFmtp(m_pAmrFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangePeriodToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangePeriodToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODE_CHANGE_PERIOD);
}

TEST_F(AudioSdpGeneratorAmrTest, TestAddModeChangeNeighborToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddModeChangeNeighborToFmtp(m_pAmrFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangeNeighborToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangeNeighborToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODE_CHANGE_NEIGHBOR);
}

TEST_F(AudioSdpGeneratorAmrTest, TestAddMaxRedToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddMaxRedToFmtp(m_pAmrFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddMaxRedToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddMaxRedToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MAXRED);
}

class AudioSdpGeneratorFullTest : public AudioSdpGenerator, public ::testing::Test
{
public:
    std::unique_ptr<MockIMediaDescriptor> m_pMockIMediaDescriptor;
    std::unique_ptr<MockISessionDescriptor> m_pMockISessionDescriptor;
    std::unique_ptr<AudioProfile> m_pAudioProfile;

protected:
    virtual void SetUp() override
    {
        m_pMockIMediaDescriptor = std::make_unique<MockIMediaDescriptor>();
        m_pMockISessionDescriptor = std::make_unique<MockISessionDescriptor>();
        m_pAudioProfile = std::make_unique<AudioProfile>();

        // Populate the profile with various attributes to test all generator functions
        m_pAudioProfile->SetPtime(20);
        m_pAudioProfile->SetMaxPtime(100);
        m_pAudioProfile->SetSupportRtcpXr(IMS_TRUE);
        m_pAudioProfile->GetRtcpXrAttr().SetSupportVoipMetrics(IMS_TRUE);
        m_pAudioProfile->GetRtcpXrAttr().SetSupportStatisticMetrics(IMS_TRUE);
        m_pAudioProfile->SetAnbr(IMS_TRUE);

        ImsVector<AString> candidateAttr;
        candidateAttr.Add("1 1 UDP 2122260223 192.168.1.1 8000 typ host");
        m_pAudioProfile->SetCandidateAttr(candidateAttr);

        // Add a payload to test payload generation
        auto pPayload = new AudioProfile::Payload();
        pPayload->SetRtpMap(97, "AMR-WB", 16000);
        auto pFmtp = std::make_shared<AudioProfile::AmrFmtp>();
        pFmtp->SetModeSetList(0x07);  // mode-set=0,1,2
        pFmtp->SetVisibleModeSet(IMS_TRUE);
        pPayload->SetFmtp(pFmtp);
        m_pAudioProfile->AddPayload(pPayload);
    }

    virtual void TearDown() override {}
};

TEST_F(AudioSdpGeneratorFullTest, TestGenerateFullSdp)
{
    // Set expectations on the mock descriptors
    EXPECT_CALL(*m_pMockISessionDescriptor, SetConnectionAddress(_)).Times(1);
    EXPECT_CALL(*m_pMockISessionDescriptor, SetOriginAddress(_)).Times(1);
    EXPECT_CALL(*m_pMockIMediaDescriptor, SetMediaDescription(_, _, _, _)).Times(1);
    EXPECT_CALL(*m_pMockIMediaDescriptor, SetBandwidthInfo(_)).Times(1);

    // GeneratePayload, check payload type and number except the fmtp
    EXPECT_CALL(*m_pMockIMediaDescriptor,
            SetMediaFormat(SdpMediaFormat::TYPE_RTP, AString("97"), AString("AMR-WB/16000"), _))
            .Times(1);

    // GenerateDirection
    EXPECT_CALL(*m_pMockIMediaDescriptor, SetDirection(_)).Times(1);
    EXPECT_CALL(*m_pMockISessionDescriptor, SetDirection(_)).Times(1);

    // GeneratePtime
    EXPECT_CALL(*m_pMockIMediaDescriptor, AddAttributeInt(SdpAttribute::PTIME, 20, _)).Times(1);

    // GenerateMaxPtime
    EXPECT_CALL(*m_pMockIMediaDescriptor, AddAttributeInt(SdpAttribute::MAXPTIME, 100, _)).Times(1);

    // GenerateCandidateAttribute
    EXPECT_CALL(*m_pMockIMediaDescriptor,
            AddAttribute(SdpAttribute::CANDIDATE,
                    AString("1, 1 1 UDP 2122260223 192.168.1.1 8000 typ host"),
                    AString::ConstNull()))
            .Times(1);

    // GenerateRtcpXr
    EXPECT_CALL(*m_pMockIMediaDescriptor,
            AddAttribute(SdpAttribute::RTCP_XR, AString("voip-metrics"), _))
            .Times(1);
    EXPECT_CALL(*m_pMockIMediaDescriptor,
            AddAttribute(SdpAttribute::RTCP_XR, AString("stat-summary=loss,dup,jitt,HL"), _))
            .Times(1);

    // GenerateAnbr
    EXPECT_CALL(*m_pMockIMediaDescriptor, AddAttribute(SdpAttribute::ANBR, _, _)).Times(1);

    // Act
    IMS_BOOL result = Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pAudioProfile.get());

    // Assert
    EXPECT_TRUE(result);
}

class AudioSdpGeneratorEvsTest : public AudioSdpGenerator, public ::testing::Test
{
public:
    std::shared_ptr<AudioProfile::EvsFmtp> m_pEvsFmtpFull;
    std::shared_ptr<AudioProfile::EvsFmtp> m_pEvsFmtpEmpty;
    std::shared_ptr<AudioProfile::EvsFmtp> m_pEvsFmtpNull;

protected:
    virtual void SetUp() override
    {
        m_pEvsFmtpFull = std::make_shared<AudioProfile::EvsFmtp>();
        m_pEvsFmtpEmpty = std::make_shared<AudioProfile::EvsFmtp>();
        m_pEvsFmtpNull = IMS_NULL;

        m_pEvsFmtpEmpty->SetShowBrList(IMS_FALSE);
        m_pEvsFmtpEmpty->SetShowBwList(IMS_FALSE);

        m_pEvsFmtpFull->SetDtx(DTX);
        m_pEvsFmtpFull->SetHfOnly(HFONLY);
        m_pEvsFmtpFull->SetEvsModeSwitch(EVS_MODE_SWITCH);
        m_pEvsFmtpFull->SetMaxRed(MAXRED);
        m_pEvsFmtpFull->SetBrList(BRLIST);
        m_pEvsFmtpFull->SetBrSend(BRLIST);
        m_pEvsFmtpFull->SetBrRecv(BRLIST);
        m_pEvsFmtpFull->SetBwList(BW_LIST);
        m_pEvsFmtpFull->SetBwSend(BW_LIST);
        m_pEvsFmtpFull->SetBwRecv(BW_LIST);
        m_pEvsFmtpFull->SetCmr(CMR);
        m_pEvsFmtpFull->SetChAwRecv(CH_AW_RECV);
        m_pEvsFmtpFull->SetReceivedChAwRecv(CH_AW_RECV);
        m_pEvsFmtpFull->SetModeSetList(MODESET_LIST);
        m_pEvsFmtpFull->SetDefaultRtpModeSet(DEFAULT_RTP_MODESET);
        m_pEvsFmtpFull->SetModeChangeCapability(MODE_CHANGE_CAPABILITY);
        m_pEvsFmtpFull->SetModeChangePeriod(MODE_CHANGE_PERIOD);
        m_pEvsFmtpFull->SetModeChangeNeighbor(MODE_CHANGE_NEIGHBOR);
        m_pEvsFmtpFull->SetVisibleDtx(IMS_TRUE);
        m_pEvsFmtpFull->SetShowHfOnly(IMS_TRUE);
        m_pEvsFmtpFull->SetShowEvsModeSwitch(IMS_TRUE);
        m_pEvsFmtpFull->SetVisibleMaxRed(IMS_TRUE);
        m_pEvsFmtpFull->SetShowCmr(IMS_TRUE);
        m_pEvsFmtpFull->SetShowChannelAwMode(IMS_TRUE);
        m_pEvsFmtpFull->SetVisibleModeChangeCapability(IMS_TRUE);
        m_pEvsFmtpFull->SetVisibleModeChangePeriod(IMS_TRUE);
        m_pEvsFmtpFull->SetVisibleModeChangeNeighbor(IMS_TRUE);
        m_pEvsFmtpFull->SetSendCmr(IMS_TRUE);
        m_pEvsFmtpFull->SetVisibleModeSet(IMS_TRUE);
    }

    virtual void TearDown() override {}
};

TEST_F(AudioSdpGeneratorEvsTest, TestGenerateEvsFmtp)
{
    AString strFmtp = AString::ConstNull();

    strFmtp = GenerateEvsFmtp(m_pEvsFmtpNull);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = GenerateEvsFmtp(m_pEvsFmtpEmpty);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = GenerateEvsFmtp(m_pEvsFmtpFull);

    EXPECT_TRUE(strFmtp.Contains(STR_MODESET_LIST));
    EXPECT_TRUE(strFmtp.Contains(STR_MODE_CHANGE_CAPABILITY));
    EXPECT_TRUE(strFmtp.Contains(STR_MODE_CHANGE_PERIOD));
    EXPECT_TRUE(strFmtp.Contains(STR_MODE_CHANGE_NEIGHBOR));
    EXPECT_TRUE(strFmtp.Contains(STR_MAXRED));
    EXPECT_TRUE(strFmtp.Contains(STR_HFONLY));
    EXPECT_TRUE(strFmtp.Contains(STR_EVS_MODE_SWITCH));
    EXPECT_TRUE(strFmtp.Contains(STR_BW_LIST));
    EXPECT_TRUE(strFmtp.Contains(STR_BR_LIST));
    EXPECT_TRUE(strFmtp.Contains(STR_BW_SEND_LIST));
    EXPECT_TRUE(strFmtp.Contains(STR_BW_RECV_LIST));
    EXPECT_TRUE(strFmtp.Contains(STR_BR_SEND_LIST));
    EXPECT_TRUE(strFmtp.Contains(STR_BR_RECV_LIST));

    EXPECT_TRUE(strFmtp.Contains(STR_CMR));
    EXPECT_FALSE(strFmtp.Contains(STR_CH_AW_RECV));

    m_pEvsFmtpFull->SetEvsModeSwitch(0);
    strFmtp = GenerateEvsFmtp(m_pEvsFmtpFull);

    EXPECT_FALSE(strFmtp.Contains(STR_MODE_CHANGE_CAPABILITY));
    EXPECT_FALSE(strFmtp.Contains(STR_MODE_CHANGE_PERIOD));
    EXPECT_FALSE(strFmtp.Contains(STR_MODE_CHANGE_NEIGHBOR));
    EXPECT_FALSE(strFmtp.Contains(STR_EVS_MODE_SWITCH));

    EXPECT_TRUE(strFmtp.Contains(STR_CMR));
    EXPECT_TRUE(strFmtp.Contains(STR_CH_AW_RECV));
}

TEST_F(AudioSdpGeneratorEvsTest, TestAppendSeparatorIfNotEmpty)
{
    AString strFmtp = AString::ConstNull();

    AppendSeparatorIfNotEmpty(strFmtp, COMMA);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = "test";

    AppendSeparatorIfNotEmpty(strFmtp, COMMA);
    EXPECT_EQ(strFmtp, "test,");

    AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);
    EXPECT_EQ(strFmtp, "test,;");
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddDtxToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddDtxToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddDtxToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddDtxToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_DTX);
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddHfOnlyToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddHfOnlyToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddHfOnlyToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddHfOnlyToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_HFONLY);
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddEvsModeSwitchToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddEvsModeSwitchToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddEvsModeSwitchToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddEvsModeSwitchToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_EVS_MODE_SWITCH);
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddMaxRedToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddMaxRedToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddMaxRedToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddMaxRedToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MAXRED);
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddBwToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddBwToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBwToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BW_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwList(1);
    AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw=nb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwList(2);
    AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw=wb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwList(4);
    AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw=swb");
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddBrToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddBrToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBrToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BR_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrList(1);
    AddBrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br=5.9");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrList(64);
    AddBrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br=24.4");
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddCmrToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddCmrToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddCmrToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddCmrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_CMR);
    strFmtp = AString::ConstNull();

    m_pEvsFmtpFull->SetEvsModeSwitch(0);
    AddCmrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_CMR);
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddChannelAwModeToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddChannelAwModeToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddChannelAwModeToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddChannelAwModeToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    m_pEvsFmtpFull->SetEvsModeSwitch(0);
    AddChannelAwModeToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_CH_AW_RECV);
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddModeSetListToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddModeSetListToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeSetListToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeSetListToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODESET_LIST);
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddModeChangeCapabilityToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddModeChangeCapabilityToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangeCapabilityToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangeCapabilityToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODE_CHANGE_CAPABILITY);
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddModeChangePeriodToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddModeChangePeriodToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangePeriodToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangePeriodToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODE_CHANGE_PERIOD);
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddModeChangeNeighborToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddModeChangeNeighborToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangeNeighborToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddModeChangeNeighborToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODE_CHANGE_NEIGHBOR);
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddBwSendToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddBwSendToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBwSendToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BW_SEND_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwSend(1);
    AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-send=nb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwSend(2);
    AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-send=wb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwSend(4);
    AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-send=swb");
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddBwRecvToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddBwRecvToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBwRecvToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BW_RECV_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwRecv(1);
    AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-recv=nb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwRecv(2);
    AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-recv=wb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwRecv(4);
    AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-recv=swb");
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddBrSendToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddBrSendToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBrSendToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBrSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BR_SEND_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrSend(1);
    AddBrSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-send=5.9");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrSend(64);
    AddBrSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-send=24.4");
}

TEST_F(AudioSdpGeneratorEvsTest, TestAddBrRecvToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AddBrRecvToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBrRecvToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AddBrRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BR_RECV_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrRecv(1);
    AddBrRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-recv=5.9");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrRecv(64);
    AddBrRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-recv=24.4");
}

class AudioSdpGeneratorTest : public AudioSdpGenerator, public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(AudioSdpGeneratorTest, TestGenerateFmtpNullPayload)
{
    AString strFmtp = "some_initial_value";
    AudioProfile::Payload* pPayload = IMS_NULL;

    IMS_BOOL result = GenerateFmtp(strFmtp, pPayload);

    EXPECT_FALSE(result);
    EXPECT_EQ(strFmtp, "some_initial_value");
}

TEST_F(AudioSdpGeneratorTest, TestGenerateFmtp)
{
    AString strFmtp;
    AudioProfile::Payload payload;
    IMS_BOOL result;

    // --- AMR/AMR-WB ---
    strFmtp = "initial";
    payload.GetRtpMap().SetPayloadType("AMR-WB");
    auto amrFmtp = std::make_shared<AudioProfile::AmrFmtp>();
    amrFmtp->SetModeSetList(0x01);  // mode-set=0
    amrFmtp->SetVisibleModeSet(IMS_TRUE);
    payload.SetFmtp(amrFmtp);
    result = GenerateFmtp(strFmtp, &payload);
    EXPECT_TRUE(result);
    EXPECT_EQ(strFmtp, "mode-set=0");

    strFmtp = "initial";
    payload.GetRtpMap().SetPayloadType("AMR");
    payload.SetFmtp(IMS_NULL);
    result = GenerateFmtp(strFmtp, &payload);
    EXPECT_FALSE(result);
    EXPECT_EQ(strFmtp, "initial");

    // --- telephone-event ---
    strFmtp = "initial";
    payload.GetRtpMap().SetPayloadType("telephone-event");
    auto teFmtp = std::make_shared<AudioProfile::TelephoneEventFmtp>("0-15");
    payload.SetFmtp(teFmtp);
    result = GenerateFmtp(strFmtp, &payload);
    EXPECT_TRUE(result);
    EXPECT_EQ(strFmtp, "0-15");

    strFmtp = "initial";
    payload.GetRtpMap().SetPayloadType("telephone-event");
    payload.SetFmtp(IMS_NULL);
    result = GenerateFmtp(strFmtp, &payload);
    EXPECT_FALSE(result);
    EXPECT_EQ(strFmtp, "initial");

    // --- EVS ---
    strFmtp = "initial";
    payload.GetRtpMap().SetPayloadType("EVS");
    auto evsFmtp = std::make_shared<AudioProfile::EvsFmtp>();
    evsFmtp->SetBrList(1 << 4);  // br=13.2
    payload.SetFmtp(evsFmtp);
    result = GenerateFmtp(strFmtp, &payload);
    EXPECT_TRUE(result);
    EXPECT_EQ(strFmtp, "br=13.2");

    strFmtp = "initial";
    payload.GetRtpMap().SetPayloadType("EVS");
    payload.SetFmtp(IMS_NULL);
    result = GenerateFmtp(strFmtp, &payload);
    EXPECT_FALSE(result);
    EXPECT_EQ(strFmtp, "initial");

    // --- PCMU/PCMA ---
    strFmtp = "initial";
    payload.GetRtpMap().SetPayloadType("pcmu");
    payload.SetFmtp(IMS_NULL);
    result = GenerateFmtp(strFmtp, &payload);
    EXPECT_TRUE(result);
    EXPECT_EQ(strFmtp, "initial");

    // --- Unsupported ---
    strFmtp = "initial";
    payload.GetRtpMap().SetPayloadType("unsupported-codec");
    payload.SetFmtp(IMS_NULL);
    result = GenerateFmtp(strFmtp, &payload);
    EXPECT_FALSE(result);
    EXPECT_EQ(strFmtp, "initial");
}

TEST_F(AudioSdpGeneratorTest, TestGenerateRtpMapNoPayload)
{
    AString rtpMap, payloadNum;
    MediaBaseProfile::RtpMap rtpMapObj;
    // No payload set, so it should be empty/zero
    EXPECT_FALSE(GenerateRtpMap(rtpMap, payloadNum, rtpMapObj));
}

TEST_F(AudioSdpGeneratorTest, TestGenerateRtpMapInvalidPayload)
{
    AString rtpMap, payloadNum;
    MediaBaseProfile::RtpMap rtpMapObj;

    // Empty payload type
    rtpMapObj.SetPayloadNumber(96);
    rtpMapObj.SetPayloadType("");
    rtpMapObj.SetSamplingRate(8000);
    EXPECT_FALSE(GenerateRtpMap(rtpMap, payloadNum, rtpMapObj));

    // Null payload type
    rtpMapObj.SetPayloadType(AString::ConstNull());
    rtpMapObj.SetPayloadNumber(99);
    rtpMapObj.SetSamplingRate(8000);
    EXPECT_FALSE(GenerateRtpMap(rtpMap, payloadNum, rtpMapObj));
}
