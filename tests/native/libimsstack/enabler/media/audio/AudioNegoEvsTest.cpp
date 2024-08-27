
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
#include "audio/AudioNegoEvs.h"

const int DTX = 1;
const int HFONLY = 1;
const int EVS_MODE_SWITCH = 1;
const int MAXRED = 220;
const int BW_LIST = 7;
const int BRLIST = 31;
const int CMR = 1;
const int CH_AW_RECV = 1;
const int MODESET_LIST = 7;
const int DEFAULT_RTP_MODESET = 15;
const int MODE_CHANGE_CAPABILITY = 2;
const int MODE_CHANGE_PERIOD = 1;
const int MODE_CHANGE_NEIGHBOR = 1;

const AString SEMICOLON = ";";
const AString COMMA = ",";
const AString STR_MODESET_LIST = "mode-set=0,1,2";
const AString STR_MODE_CHANGE_CAPABILITY = "mode-change-capability=2";
const AString STR_MODE_CHANGE_PERIOD = "mode-change-period=1";
const AString STR_MODE_CHANGE_NEIGHBOR = "mode-change-neighbor=1";
const AString STR_MAXRED = "max-red=220";
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

class AudioNegoEvsTest : public ::testing::Test
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
        m_pEvsFmtpFull->SetShowDtx(IMS_TRUE);
        m_pEvsFmtpFull->SetShowHfOnly(IMS_TRUE);
        m_pEvsFmtpFull->SetShowEvsModeSwitch(IMS_TRUE);
        m_pEvsFmtpFull->SetShowMaxRed(IMS_TRUE);
        m_pEvsFmtpFull->SetShowCmr(IMS_TRUE);
        m_pEvsFmtpFull->SetShowChannelAwMode(IMS_TRUE);
        m_pEvsFmtpFull->SetShowModeChangeCapability(IMS_TRUE);
        m_pEvsFmtpFull->SetShowModeChangePeriod(IMS_TRUE);
        m_pEvsFmtpFull->SetShowModeChangeNeighbor(IMS_TRUE);
        m_pEvsFmtpFull->SetSendCmr(IMS_TRUE);
        m_pEvsFmtpFull->SetShowModeSet(IMS_TRUE);
    }

    virtual void TearDown() override
    {
        delete m_pEvsFmtpFull;
        delete m_pEvsFmtpEmpty;
    }
};

TEST_F(AudioNegoEvsTest, TestSetSdpFmtpFromEvsFmtp)
{
    AString strFmtp = AString::ConstNull();

    strFmtp = AudioNegoEvs::SetSdpFmtpFromEvsFmtp(m_pEvsFmtpNull);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = AudioNegoEvs::SetSdpFmtpFromEvsFmtp(m_pEvsFmtpEmpty);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = AudioNegoEvs::SetSdpFmtpFromEvsFmtp(m_pEvsFmtpFull);

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
    strFmtp = AudioNegoEvs::SetSdpFmtpFromEvsFmtp(m_pEvsFmtpFull);

    EXPECT_FALSE(strFmtp.Contains(STR_MODE_CHANGE_CAPABILITY));
    EXPECT_FALSE(strFmtp.Contains(STR_MODE_CHANGE_PERIOD));
    EXPECT_FALSE(strFmtp.Contains(STR_MODE_CHANGE_NEIGHBOR));
    EXPECT_FALSE(strFmtp.Contains(STR_EVS_MODE_SWITCH));

    EXPECT_TRUE(strFmtp.Contains(STR_CMR));
    EXPECT_TRUE(strFmtp.Contains(STR_CH_AW_RECV));
}

TEST_F(AudioNegoEvsTest, TestAppendSeparatorIfNotEmpty)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AppendSeparatorIfNotEmpty(strFmtp, COMMA);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = "test";

    AudioNegoEvs::AppendSeparatorIfNotEmpty(strFmtp, COMMA);
    EXPECT_EQ(strFmtp, "test,");

    AudioNegoEvs::AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);
    EXPECT_EQ(strFmtp, "test,;");
}

TEST_F(AudioNegoEvsTest, TestAddDtxToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddDtxToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddDtxToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddDtxToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_DTX);
}

TEST_F(AudioNegoEvsTest, TestAddHfOnlyToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddHfOnlyToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddHfOnlyToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddHfOnlyToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_HFONLY);
}

TEST_F(AudioNegoEvsTest, TestAddEvsModeSwitchToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddEvsModeSwitchToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddEvsModeSwitchToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddEvsModeSwitchToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_EVS_MODE_SWITCH);
}

TEST_F(AudioNegoEvsTest, TestAddMaxRedToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddMaxRedToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddMaxRedToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddMaxRedToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MAXRED);
}

TEST_F(AudioNegoEvsTest, TestAddBwToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBwToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBwToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BW_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwList(1);
    AudioNegoEvs::AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw=nb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwList(2);
    AudioNegoEvs::AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw=wb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwList(4);
    AudioNegoEvs::AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw=swb");
}

TEST_F(AudioNegoEvsTest, TestAddBrToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBrToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBrToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BR_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrList(1);
    AudioNegoEvs::AddBrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br=5.9");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrList(64);
    AudioNegoEvs::AddBrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br=24.4");
}

TEST_F(AudioNegoEvsTest, TestAddCmrToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddCmrToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddCmrToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddCmrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    m_pEvsFmtpFull->SetEvsModeSwitch(0);
    AudioNegoEvs::AddCmrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_CMR);
}

TEST_F(AudioNegoEvsTest, TestAddChannelAwModeToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddChannelAwModeToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddChannelAwModeToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddChannelAwModeToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    m_pEvsFmtpFull->SetEvsModeSwitch(0);
    AudioNegoEvs::AddChannelAwModeToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_CH_AW_RECV);
}

TEST_F(AudioNegoEvsTest, TestAddModeSetListToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddModeSetListToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeSetListToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeSetListToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODESET_LIST);
}

TEST_F(AudioNegoEvsTest, TestAddModeChangeCapabilityToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddModeChangeCapabilityToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeChangeCapabilityToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeChangeCapabilityToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODE_CHANGE_CAPABILITY);
}

TEST_F(AudioNegoEvsTest, TestAddModeChangePeriodToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddModeChangePeriodToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeChangePeriodToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeChangePeriodToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODE_CHANGE_PERIOD);
}

TEST_F(AudioNegoEvsTest, TestAddModeChangeNeighborToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddModeChangeNeighborToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeChangeNeighborToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeChangeNeighborToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_MODE_CHANGE_NEIGHBOR);
}

TEST_F(AudioNegoEvsTest, TestAddBwSendToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBwSendToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBwSendToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BW_SEND_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwSend(1);
    AudioNegoEvs::AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-send=nb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwSend(2);
    AudioNegoEvs::AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-send=wb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwSend(4);
    AudioNegoEvs::AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-send=swb");
}

TEST_F(AudioNegoEvsTest, TestAddBwRecvToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBwRecvToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBwRecvToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BW_RECV_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwRecv(1);
    AudioNegoEvs::AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-recv=nb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwRecv(2);
    AudioNegoEvs::AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-recv=wb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBwRecv(4);
    AudioNegoEvs::AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-recv=swb");
}

TEST_F(AudioNegoEvsTest, TestAddBrSendToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBrSendToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBrSendToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBrSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BR_SEND_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrSend(1);
    AudioNegoEvs::AddBrSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-send=5.9");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrSend(64);
    AudioNegoEvs::AddBrSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-send=24.4");
}

TEST_F(AudioNegoEvsTest, TestAddBrRecvToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBrRecvToFmtp(m_pEvsFmtpNull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBrRecvToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBrRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, STR_BR_RECV_LIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrRecv(1);
    AudioNegoEvs::AddBrRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-recv=5.9");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->SetBrRecv(64);
    AudioNegoEvs::AddBrRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-recv=24.4");
}
