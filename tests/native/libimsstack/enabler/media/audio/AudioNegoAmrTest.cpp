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
#include "audio/AudioNegoAmr.h"

const AString SEMICOLON = ";";
const AString COMMA = ",";

const AString MODESETLIST = "mode-set=0,1,2";
const AString OCTETALIGN = "octet-align=1";
const AString MODECHANGECAPABILITY = "mode-change-capability=2";
const AString MODECHANGEPERIOD = "mode-change-period=1";
const AString MODECHANGENEIGHBOR = "mode-change-neighbor=1";
const AString MAXRED = "max-red=220";
const AString PTIME = "ptime=20";
const AString MAXPTIME = "maxptime=240";

class AudioNegoAmrTest : public ::testing::Test
{
public:
    AudioProfile::AmrFmtp* m_pAmrFmtpFull;
    AudioProfile::AmrFmtp* m_pAmrFmtpEmpty;

protected:
    virtual void SetUp() override
    {
        m_pAmrFmtpFull = new AudioProfile::AmrFmtp();
        m_pAmrFmtpEmpty = new AudioProfile::AmrFmtp();

        m_pAmrFmtpFull->nModeSetList = 7;
        m_pAmrFmtpFull->nOctetAlign = 1;
        m_pAmrFmtpFull->nModeChangeCapability = 2;
        m_pAmrFmtpFull->nModeChangePeriod = 1;
        m_pAmrFmtpFull->nModeChangeNeighbor = 1;
        m_pAmrFmtpFull->nMaxRed = 220;
        m_pAmrFmtpFull->nPtime = 20;
        m_pAmrFmtpFull->nMaxPtime = 240;

        m_pAmrFmtpFull->bShowModeSet = IMS_TRUE;
        m_pAmrFmtpFull->bShow_OctetAlign = IMS_TRUE;
        m_pAmrFmtpFull->bShowModeChangeCapability = IMS_TRUE;
        m_pAmrFmtpFull->bShowModeChangePeriod = IMS_TRUE;
        m_pAmrFmtpFull->bShowModeChangeNeighbor = IMS_TRUE;
        m_pAmrFmtpFull->bShowMaxRed = IMS_TRUE;
        m_pAmrFmtpFull->bShowPtime = IMS_TRUE;
        m_pAmrFmtpFull->bShowMaxPtime = IMS_TRUE;
    }

    virtual void TearDown() override
    {
        delete m_pAmrFmtpFull;
        delete m_pAmrFmtpEmpty;
    }
};

TEST_F(AudioNegoAmrTest, TestSetSdpFmtpFromAmrFmtp)
{
    AString strFmtp = AString::ConstNull();

    strFmtp = AudioNegoAmr::SetSdpFmtpFromAmrFmtp(m_pAmrFmtpEmpty);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = AudioNegoAmr::SetSdpFmtpFromAmrFmtp(m_pAmrFmtpFull);

    EXPECT_EQ(strFmtp.Contains(MODESETLIST), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(OCTETALIGN), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(MODECHANGECAPABILITY), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(MODECHANGEPERIOD), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(MODECHANGENEIGHBOR), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(MAXRED), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(PTIME), IMS_TRUE);
    EXPECT_EQ(strFmtp.Contains(MAXPTIME), IMS_TRUE);

    AString strResult = MODESETLIST;
    AudioNegoAmr::AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(OCTETALIGN);
    AudioNegoAmr::AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(MODECHANGECAPABILITY);
    AudioNegoAmr::AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(MODECHANGEPERIOD);
    AudioNegoAmr::AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(MODECHANGENEIGHBOR);
    AudioNegoAmr::AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(MAXRED);
    AudioNegoAmr::AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(PTIME);
    AudioNegoAmr::AppendSeparatorIfNotEmpty(strResult, SEMICOLON);
    strResult.Append(MAXPTIME);

    EXPECT_EQ(strFmtp, strResult);
}

TEST_F(AudioNegoAmrTest, TestAppendSeparatorIfNotEmpty)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoAmr::AppendSeparatorIfNotEmpty(strFmtp, COMMA);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoAmr::AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    strFmtp = "test";

    AudioNegoAmr::AppendSeparatorIfNotEmpty(strFmtp, COMMA);
    EXPECT_EQ(strFmtp, "test,");

    AudioNegoAmr::AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);
    EXPECT_EQ(strFmtp, "test,;");
}

TEST_F(AudioNegoAmrTest, TestAddModeSetListToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoAmr::AddModeSetListToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoAmr::AddModeSetListToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MODESETLIST);
}

TEST_F(AudioNegoAmrTest, TestAddOctetAlignToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoAmr::AddOctetAlignToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoAmr::AddOctetAlignToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, OCTETALIGN);
}

TEST_F(AudioNegoAmrTest, TestAddModeChangeCapabilityToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoAmr::AddModeChangeCapabilityToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoAmr::AddModeChangeCapabilityToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MODECHANGECAPABILITY);
}

TEST_F(AudioNegoAmrTest, TestAddModeChangePeriodToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoAmr::AddModeChangePeriodToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoAmr::AddModeChangePeriodToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MODECHANGEPERIOD);
}

TEST_F(AudioNegoAmrTest, TestAddModeChangeNeighborToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoAmr::AddModeChangeNeighborToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoAmr::AddModeChangeNeighborToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MODECHANGENEIGHBOR);
}

TEST_F(AudioNegoAmrTest, TestAddMaxRedToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoAmr::AddMaxRedToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoAmr::AddMaxRedToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MAXRED);
}

TEST_F(AudioNegoAmrTest, TestAddPtimeToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoAmr::AddPtimeToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoAmr::AddPtimeToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, PTIME);
}

TEST_F(AudioNegoAmrTest, TestAddMaxPtimeToFmtp)
{
    AString strFmtp = AString::ConstNull();

    AudioNegoAmr::AddMaxPtimeToFmtp(m_pAmrFmtpEmpty, strFmtp);
    EXPECT_EQ(strFmtp, AString::ConstNull());

    AudioNegoAmr::AddMaxPtimeToFmtp(m_pAmrFmtpFull, strFmtp);
    EXPECT_EQ(strFmtp, MAXPTIME);
}
