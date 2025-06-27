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
#include "audio/AudioSdpGenerator.h"

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
    AudioProfile::AmrFmtp* m_pAmrFmtpFull;
    AudioProfile::AmrFmtp* m_pAmrFmtpEmpty;
    AudioProfile::AmrFmtp* m_pAmrFmtpNull;

protected:
    virtual void SetUp() override
    {
        m_pAmrFmtpFull = new AudioProfile::AmrFmtp();
        m_pAmrFmtpEmpty = new AudioProfile::AmrFmtp();
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

    virtual void TearDown() override
    {
        delete m_pAmrFmtpFull;
        delete m_pAmrFmtpEmpty;
    }
};

TEST_F(AudioSdpGeneratorAmrTest, TestGenerateAmrFmtp)
{
    AString strFmtp = AString::ConstNull();

    strFmtp = GenerateAmrFmtp(m_pAmrFmtpNull);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = GenerateAmrFmtp(m_pAmrFmtpEmpty);
    EXPECT_EQ(strFmtp, STR_OCTETALIGN0);

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

class AudioSdpGeneratorEvsTest : public AudioSdpGenerator, public ::testing::Test
{
public:
    AudioProfile::EvsFmtp* m_pEvsFmtpFull;
    AudioProfile::EvsFmtp* m_pEvsFmtpEmpty;
    AudioProfile::EvsFmtp* m_pEvsFmtpNull;

protected:
    virtual void SetUp() override
    {
        m_pEvsFmtpFull = new AudioProfile::EvsFmtp();
        m_pEvsFmtpEmpty = new AudioProfile::EvsFmtp();
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

    virtual void TearDown() override
    {
        delete m_pEvsFmtpFull;
        delete m_pEvsFmtpEmpty;
    }
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

    EXPECT_FALSE(strFmtp.Contains(STR_CMR));
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
    EXPECT_EQ(strFmtp, AString::ConstNull());

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
