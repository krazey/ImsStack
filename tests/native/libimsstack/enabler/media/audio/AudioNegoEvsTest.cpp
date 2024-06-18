
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

const AString SEMICOLON = ";";
const AString COMMA = ",";
const AString MODESETLIST = "mode-set=0,1,2";
const AString MODECHANGECAPABILITY = "mode-change-capability=2";
const AString MODECHANGEPERIOD = "mode-change-period=1";
const AString MODECHANGENEIGHBOR = "mode-change-neighbor=1";
const AString MAXRED = "max-red=220";
const AString PTIME = "ptime=20";
const AString MAXPTIME = "maxptime=240";
const AString DTX = "dtx=1";
const AString HFONLY = "hf-only=1";
const AString EVSMODESWITCH = "evs-mode-switch=1";
const AString BWLIST = "bw=nb-swb";
const AString BRLIST = "br=5.9-13.2";
const AString CMR = "cmr=1";
const AString CHAWRECV = "ch-aw-recv=1";
const AString BWSENDLIST = "bw-send=nb-swb";
const AString BWRECVLIST = "bw-recv=nb-swb";
const AString BRSENDLIST = "br-send=5.9-13.2";
const AString BRRECVLIST = "br-recv=5.9-13.2";

class AudioNegoEvsTest : public ::testing::Test
{
public:
    AudioProfile::EvsFmtp* m_pEvsFmtpFull;
    AudioProfile::EvsFmtp* m_pEvsFmtpEmpty;

protected:
    virtual void SetUp() override
    {
        m_pEvsFmtpEmpty = new AudioProfile::EvsFmtp();
        m_pEvsFmtpEmpty->bShowBrList = IMS_FALSE;
        m_pEvsFmtpEmpty->bShowBwList = IMS_FALSE;

        m_pEvsFmtpFull = new AudioProfile::EvsFmtp();
        m_pEvsFmtpFull->nPtime = 20;
        m_pEvsFmtpFull->nMaxPtime = 240;
        m_pEvsFmtpFull->bDtx = 1;
        m_pEvsFmtpFull->nHfOnly = 1;
        m_pEvsFmtpFull->nEvsModeSwitch = 1;
        m_pEvsFmtpFull->nMaxRed = 220;
        m_pEvsFmtpFull->nBrList = 31;
        m_pEvsFmtpFull->nBrSend = 31;
        m_pEvsFmtpFull->nBrRecv = 31;
        m_pEvsFmtpFull->nBwList = 7;
        m_pEvsFmtpFull->nBwSend = 7;
        m_pEvsFmtpFull->nBwRecv = 7;
        m_pEvsFmtpFull->nCmr = 1;
        m_pEvsFmtpFull->nChAwRecv = 1;
        m_pEvsFmtpFull->nReceivedChAwRecv = 1;
        m_pEvsFmtpFull->nModeSetList = 7;
        m_pEvsFmtpFull->nDefaultRtpModeSet = 15;
        m_pEvsFmtpFull->nModeChangeCapability = 2;
        m_pEvsFmtpFull->nModeChangePeriod = 1;
        m_pEvsFmtpFull->nModeChangeNeighbor = 1;

        m_pEvsFmtpFull->bShowPtime = IMS_TRUE;
        m_pEvsFmtpFull->bShowMaxPtime = IMS_TRUE;
        m_pEvsFmtpFull->bShowDtx = IMS_TRUE;
        m_pEvsFmtpFull->bShowHfOnly = IMS_TRUE;
        m_pEvsFmtpFull->bShowEvsModeSwitch = IMS_TRUE;
        m_pEvsFmtpFull->bShowMaxRed = IMS_TRUE;
        m_pEvsFmtpFull->bShowCmr = IMS_TRUE;
        m_pEvsFmtpFull->bShowChannelAwMode = IMS_TRUE;
        m_pEvsFmtpFull->bShowModeChangeCapability = IMS_TRUE;
        m_pEvsFmtpFull->bShowModeChangePeriod = IMS_TRUE;
        m_pEvsFmtpFull->bShowModeChangeNeighbor = IMS_TRUE;
        m_pEvsFmtpFull->bSendCmr = IMS_TRUE;
        m_pEvsFmtpFull->bShowModeSet = IMS_TRUE;
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

    strFmtp = AudioNegoEvs::SetSdpFmtpFromEvsFmtp(m_pEvsFmtpEmpty);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = AudioNegoEvs::SetSdpFmtpFromEvsFmtp(m_pEvsFmtpFull);

    EXPECT_TRUE(strFmtp.Contains(MODESETLIST));
    EXPECT_TRUE(strFmtp.Contains(MODECHANGECAPABILITY));
    EXPECT_TRUE(strFmtp.Contains(MODECHANGEPERIOD));
    EXPECT_TRUE(strFmtp.Contains(MODECHANGENEIGHBOR));
    EXPECT_TRUE(strFmtp.Contains(MAXRED));
    EXPECT_TRUE(strFmtp.Contains(PTIME));
    EXPECT_TRUE(strFmtp.Contains(PTIME));
    EXPECT_TRUE(strFmtp.Contains(HFONLY));
    EXPECT_TRUE(strFmtp.Contains(EVSMODESWITCH));
    EXPECT_TRUE(strFmtp.Contains(BWLIST));
    EXPECT_TRUE(strFmtp.Contains(BRLIST));
    EXPECT_TRUE(strFmtp.Contains(BWSENDLIST));
    EXPECT_TRUE(strFmtp.Contains(BWRECVLIST));
    EXPECT_TRUE(strFmtp.Contains(BRSENDLIST));
    EXPECT_TRUE(strFmtp.Contains(BRRECVLIST));

    EXPECT_FALSE(strFmtp.Contains(CMR));
    EXPECT_FALSE(strFmtp.Contains(CHAWRECV));

    m_pEvsFmtpFull->nEvsModeSwitch = 0;
    strFmtp = AudioNegoEvs::SetSdpFmtpFromEvsFmtp(m_pEvsFmtpFull);

    EXPECT_FALSE(strFmtp.Contains(MODECHANGECAPABILITY));
    EXPECT_FALSE(strFmtp.Contains(MODECHANGEPERIOD));
    EXPECT_FALSE(strFmtp.Contains(MODECHANGENEIGHBOR));
    EXPECT_FALSE(strFmtp.Contains(EVSMODESWITCH));

    EXPECT_TRUE(strFmtp.Contains(CMR));
    EXPECT_TRUE(strFmtp.Contains(CHAWRECV));
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

TEST_F(AudioNegoEvsTest, TestAddPtimeToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddPtimeToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddPtimeToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, PTIME);
}

TEST_F(AudioNegoEvsTest, TestAddMaxPtimeToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddMaxPtimeToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddMaxPtimeToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MAXPTIME);
}

TEST_F(AudioNegoEvsTest, TestAddDtxToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddDtxToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddDtxToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, DTX);
}

TEST_F(AudioNegoEvsTest, TestAddHfOnlyToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddHfOnlyToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddHfOnlyToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, HFONLY);
}

TEST_F(AudioNegoEvsTest, TestAddEvsModeSwitchToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddEvsModeSwitchToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddEvsModeSwitchToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, EVSMODESWITCH);
}

TEST_F(AudioNegoEvsTest, TestAddMaxRedToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddMaxRedToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddMaxRedToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MAXRED);
}

TEST_F(AudioNegoEvsTest, TestAddBwToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBwToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, BWLIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBwList = 1;
    AudioNegoEvs::AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw=nb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBwList = 2;
    AudioNegoEvs::AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw=wb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBwList = 4;
    AudioNegoEvs::AddBwToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw=swb");
}

TEST_F(AudioNegoEvsTest, TestAddBrToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBrToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, BRLIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBrList = 1;
    AudioNegoEvs::AddBrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br=5.9");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBrList = 64;
    AudioNegoEvs::AddBrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br=24.4");
}

TEST_F(AudioNegoEvsTest, TestAddCmrToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddCmrToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddCmrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    m_pEvsFmtpFull->nEvsModeSwitch = 0;
    AudioNegoEvs::AddCmrToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, CMR);
}

TEST_F(AudioNegoEvsTest, TestAddChannelAwModeToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddChannelAwModeToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddChannelAwModeToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    m_pEvsFmtpFull->nEvsModeSwitch = 0;
    AudioNegoEvs::AddChannelAwModeToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, CHAWRECV);
}

TEST_F(AudioNegoEvsTest, TestAddModeSetListToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddModeSetListToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeSetListToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MODESETLIST);
}

TEST_F(AudioNegoEvsTest, TestAddModeChangeCapabilityToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddModeChangeCapabilityToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeChangeCapabilityToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MODECHANGECAPABILITY);
}

TEST_F(AudioNegoEvsTest, TestAddModeChangePeriodToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddModeChangePeriodToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeChangePeriodToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MODECHANGEPERIOD);
}

TEST_F(AudioNegoEvsTest, TestAddModeChangeNeighborToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddModeChangeNeighborToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddModeChangeNeighborToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MODECHANGENEIGHBOR);
}

TEST_F(AudioNegoEvsTest, TestAddBwSendToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBwSendToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, BWSENDLIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBwSend = 1;
    AudioNegoEvs::AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-send=nb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBwSend = 2;
    AudioNegoEvs::AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-send=wb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBwSend = 4;
    AudioNegoEvs::AddBwSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-send=swb");
}

TEST_F(AudioNegoEvsTest, TestAddBwRecvToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBwRecvToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, BWRECVLIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBwRecv = 1;
    AudioNegoEvs::AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-recv=nb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBwRecv = 2;
    AudioNegoEvs::AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-recv=wb");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBwRecv = 4;
    AudioNegoEvs::AddBwRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "bw-recv=swb");
}

TEST_F(AudioNegoEvsTest, TestAddBrSendToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBrSendToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBrSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, BRSENDLIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBrSend = 1;
    AudioNegoEvs::AddBrSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-send=5.9");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBrSend = 64;
    AudioNegoEvs::AddBrSendToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-send=24.4");
}

TEST_F(AudioNegoEvsTest, TestAddBrRecvToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoEvs::AddBrRecvToFmtp(m_pEvsFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoEvs::AddBrRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, BRRECVLIST);

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBrRecv = 1;
    AudioNegoEvs::AddBrRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-recv=5.9");

    strFmtp = AString::ConstNull();
    m_pEvsFmtpFull->nBrRecv = 64;
    AudioNegoEvs::AddBrRecvToFmtp(m_pEvsFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, "br-recv=24.4");
}
