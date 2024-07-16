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

#include <audio/AudioProfile.h>

const IMS_UINT32 AUDIO_FMTP_MODESET_LIST = 7;
const IMS_UINT32 AUDIO_FMTP_DEFAULT_MODESET = 7;
const IMS_SINT32 AUDIO_FMTP_MODE_CHANGE_CAPABILITY = 2;
const IMS_SINT32 AUDIO_FMTP_MODE_CHANGE_PERIOD = 2;
const IMS_SINT32 AUDIO_FMTP_MODE_CHANGE_NEIGHBOR = 2;
const IMS_SINT32 AUDIO_FMTP_MAX_RED = 220;
const IMS_SINT32 AUDIO_FMTP_PTIME = 20;
const IMS_SINT32 AUDIO_FMTP_MAX_PTIME = 240;
const IMS_BOOL AUDIO_FMTP_DTX = IMS_FALSE;
const IMS_BOOL AUDIO_FMTP_SHOW_MODESET = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_MAX_RED = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_PTIME = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_MAX_PTIME = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_DTX = IMS_TRUE;
const IMS_SINT32 AMR_FMTP_OCTET_ALIGN = 1;
const IMS_BOOL AMR_FMTP_SHOW_OCTET_ALIGN = IMS_TRUE;

class AudioProfileTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(AudioProfileTest, testAudioFmtpModeSetList)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetModeSetList(), 0);

    pFmtp->SetModeSetList(AUDIO_FMTP_MODESET_LIST);
    EXPECT_EQ(pFmtp->GetModeSetList(), AUDIO_FMTP_MODESET_LIST);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpDefaultModeSet)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetDefaultRtpModeSet(), 0);

    pFmtp->SetDefaultRtpModeSet(AUDIO_FMTP_DEFAULT_MODESET);
    EXPECT_EQ(pFmtp->GetDefaultRtpModeSet(), AUDIO_FMTP_DEFAULT_MODESET);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpModeChangeCapability)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetModeChangeCapability(), 1);

    pFmtp->SetModeChangeCapability(AUDIO_FMTP_MODE_CHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp->GetModeChangeCapability(), AUDIO_FMTP_MODE_CHANGE_CAPABILITY);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpModeChangePeriod)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetModeChangePeriod(), 1);

    pFmtp->SetModeChangePeriod(AUDIO_FMTP_MODE_CHANGE_PERIOD);
    EXPECT_EQ(pFmtp->GetModeChangePeriod(), AUDIO_FMTP_MODE_CHANGE_PERIOD);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpModeChangeNeighbor)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetModeChangeNeighbor(), 0);

    pFmtp->SetModeChangeNeighbor(AUDIO_FMTP_MODE_CHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp->GetModeChangeNeighbor(), AUDIO_FMTP_MODE_CHANGE_NEIGHBOR);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpMaxRed)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetMaxRed(), -1);

    pFmtp->SetMaxRed(AUDIO_FMTP_MAX_RED);
    EXPECT_EQ(pFmtp->GetMaxRed(), AUDIO_FMTP_MAX_RED);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpPtime)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetPtime(), -1);

    pFmtp->SetPtime(AUDIO_FMTP_PTIME);
    EXPECT_EQ(pFmtp->GetPtime(), AUDIO_FMTP_PTIME);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpMaxPtime)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetMaxPtime(), -1);

    pFmtp->SetMaxPtime(AUDIO_FMTP_MAX_PTIME);
    EXPECT_EQ(pFmtp->GetMaxPtime(), AUDIO_FMTP_MAX_PTIME);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpDtx)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->IsDtxEnabled(), IMS_TRUE);

    pFmtp->SetDtx(AUDIO_FMTP_DTX);
    EXPECT_EQ(pFmtp->IsDtxEnabled(), AUDIO_FMTP_DTX);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpShowModeSet)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->IsModeSetVisible(), IMS_FALSE);

    pFmtp->SetShowModeSet(AUDIO_FMTP_SHOW_MODESET);
    EXPECT_EQ(pFmtp->IsModeSetVisible(), AUDIO_FMTP_SHOW_MODESET);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpShowModeChangeCapability)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->IsModeChangeCapabilityVisible(), IMS_FALSE);

    pFmtp->SetShowModeChangeCapability(AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp->IsModeChangeCapabilityVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpShowModeChangePeriod)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->IsModeChangePeriodVisible(), IMS_FALSE);

    pFmtp->SetShowModeChangePeriod(AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD);
    EXPECT_EQ(pFmtp->IsModeChangePeriodVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpShowModeChangeNeighbor)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->IsModeChangeNeighborVisible(), IMS_FALSE);

    pFmtp->SetShowModeChangeNeighbor(AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp->IsModeChangeNeighborVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpShowMaxRed)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->IsMaxRedVisible(), IMS_FALSE);

    pFmtp->SetShowMaxRed(AUDIO_FMTP_SHOW_MAX_RED);
    EXPECT_EQ(pFmtp->IsMaxRedVisible(), AUDIO_FMTP_SHOW_MAX_RED);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpShowPtime)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->IsPtimeVisible(), IMS_FALSE);

    pFmtp->SetShowPtime(AUDIO_FMTP_SHOW_PTIME);
    EXPECT_EQ(pFmtp->IsPtimeVisible(), AUDIO_FMTP_SHOW_PTIME);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpShowMaxPtime)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->IsMaxPtimeVisible(), IMS_FALSE);

    pFmtp->SetShowMaxPtime(AUDIO_FMTP_SHOW_MAX_PTIME);
    EXPECT_EQ(pFmtp->IsMaxPtimeVisible(), AUDIO_FMTP_SHOW_MAX_PTIME);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpShowDtx)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->IsDtxVisible(), IMS_FALSE);

    pFmtp->SetShowDtx(AUDIO_FMTP_SHOW_DTX);
    EXPECT_EQ(pFmtp->IsDtxVisible(), AUDIO_FMTP_SHOW_DTX);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpCreationDefault)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetModeSetList(), 0);
    EXPECT_EQ(pFmtp->GetDefaultRtpModeSet(), 0);
    EXPECT_EQ(pFmtp->GetModeChangeCapability(), 1);
    EXPECT_EQ(pFmtp->GetModeChangePeriod(), 1);
    EXPECT_EQ(pFmtp->GetModeChangeNeighbor(), 0);
    EXPECT_EQ(pFmtp->GetMaxRed(), -1);
    EXPECT_EQ(pFmtp->GetPtime(), -1);
    EXPECT_EQ(pFmtp->GetMaxPtime(), -1);
    EXPECT_EQ(pFmtp->IsDtxEnabled(), IMS_TRUE);
    EXPECT_EQ(pFmtp->IsModeSetVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangeCapabilityVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangePeriodVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangeNeighborVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsMaxRedVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsPtimeVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsMaxPtimeVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsDtxVisible(), IMS_FALSE);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpCreation)
{
    AudioProfile::AudioFmtp* pFmtp1 = new AudioProfile::AudioFmtp();

    pFmtp1->SetModeSetList(AUDIO_FMTP_MODESET_LIST);
    pFmtp1->SetDefaultRtpModeSet(AUDIO_FMTP_DEFAULT_MODESET);
    pFmtp1->SetModeChangeCapability(AUDIO_FMTP_MODE_CHANGE_CAPABILITY);
    pFmtp1->SetModeChangePeriod(AUDIO_FMTP_MODE_CHANGE_PERIOD);
    pFmtp1->SetModeChangeNeighbor(AUDIO_FMTP_MODE_CHANGE_NEIGHBOR);
    pFmtp1->SetMaxRed(AUDIO_FMTP_MAX_RED);
    pFmtp1->SetPtime(AUDIO_FMTP_PTIME);
    pFmtp1->SetMaxPtime(AUDIO_FMTP_MAX_PTIME);
    pFmtp1->SetDtx(AUDIO_FMTP_DTX);
    pFmtp1->SetShowModeSet(AUDIO_FMTP_SHOW_MODESET);
    pFmtp1->SetShowModeChangeCapability(AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY);
    pFmtp1->SetShowModeChangePeriod(AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD);
    pFmtp1->SetShowModeChangeNeighbor(AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR);
    pFmtp1->SetShowMaxRed(AUDIO_FMTP_SHOW_MAX_RED);
    pFmtp1->SetShowPtime(AUDIO_FMTP_SHOW_PTIME);
    pFmtp1->SetShowMaxPtime(AUDIO_FMTP_SHOW_MAX_PTIME);
    pFmtp1->SetShowDtx(AUDIO_FMTP_SHOW_DTX);

    AudioProfile::AudioFmtp* pFmtp2 = new AudioProfile::AudioFmtp(*pFmtp1);

    EXPECT_EQ(pFmtp2->GetModeSetList(), AUDIO_FMTP_MODESET_LIST);
    EXPECT_EQ(pFmtp2->GetDefaultRtpModeSet(), AUDIO_FMTP_DEFAULT_MODESET);
    EXPECT_EQ(pFmtp2->GetModeChangeCapability(), AUDIO_FMTP_MODE_CHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp2->GetModeChangePeriod(), AUDIO_FMTP_MODE_CHANGE_PERIOD);
    EXPECT_EQ(pFmtp2->GetModeChangeNeighbor(), AUDIO_FMTP_MODE_CHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp2->GetMaxRed(), AUDIO_FMTP_MAX_RED);
    EXPECT_EQ(pFmtp2->GetPtime(), AUDIO_FMTP_PTIME);
    EXPECT_EQ(pFmtp2->GetMaxPtime(), AUDIO_FMTP_MAX_PTIME);
    EXPECT_EQ(pFmtp2->IsDtxEnabled(), AUDIO_FMTP_DTX);
    EXPECT_EQ(pFmtp2->IsModeSetVisible(), AUDIO_FMTP_SHOW_MODESET);
    EXPECT_EQ(pFmtp2->IsModeChangeCapabilityVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp2->IsModeChangePeriodVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD);
    EXPECT_EQ(pFmtp2->IsModeChangeNeighborVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp2->IsMaxRedVisible(), AUDIO_FMTP_SHOW_MAX_RED);
    EXPECT_EQ(pFmtp2->IsPtimeVisible(), AUDIO_FMTP_SHOW_PTIME);
    EXPECT_EQ(pFmtp2->IsMaxPtimeVisible(), AUDIO_FMTP_SHOW_MAX_PTIME);
    EXPECT_EQ(pFmtp2->IsDtxVisible(), AUDIO_FMTP_SHOW_DTX);

    delete pFmtp1;
    delete pFmtp2;
}

TEST_F(AudioProfileTest, testAmrFmtpOctetAlign)
{
    AudioProfile::AmrFmtp* pFmtp = new AudioProfile::AmrFmtp();
    EXPECT_EQ(pFmtp->GetOctetAlign(), 0);

    pFmtp->SetOctetAlign(AMR_FMTP_OCTET_ALIGN);
    EXPECT_EQ(pFmtp->GetOctetAlign(), AMR_FMTP_OCTET_ALIGN);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAmrFmtpShowOctetAlign)
{
    AudioProfile::AmrFmtp* pFmtp = new AudioProfile::AmrFmtp();
    EXPECT_EQ(pFmtp->IsOctetAlignVisible(), IMS_FALSE);

    pFmtp->SetShowOctetAlign(AMR_FMTP_SHOW_OCTET_ALIGN);
    EXPECT_EQ(pFmtp->IsOctetAlignVisible(), AMR_FMTP_SHOW_OCTET_ALIGN);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAmrFmtpCreationDefault)
{
    AudioProfile::AmrFmtp* pFmtp = new AudioProfile::AmrFmtp();

    EXPECT_EQ(pFmtp->GetOctetAlign(), 0);
    EXPECT_EQ(pFmtp->IsOctetAlignVisible(), IMS_FALSE);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAmrFmtpCreation)
{
    AudioProfile::AmrFmtp* pFmtp1 = new AudioProfile::AmrFmtp();

    pFmtp1->SetOctetAlign(AMR_FMTP_OCTET_ALIGN);
    pFmtp1->SetShowOctetAlign(AMR_FMTP_SHOW_OCTET_ALIGN);

    AudioProfile::AmrFmtp* pFmtp2 = new AudioProfile::AmrFmtp(*pFmtp1);

    EXPECT_EQ(pFmtp2->GetOctetAlign(), AMR_FMTP_OCTET_ALIGN);
    EXPECT_EQ(pFmtp2->IsOctetAlignVisible(), AMR_FMTP_SHOW_OCTET_ALIGN);

    delete pFmtp1;
    delete pFmtp2;
}
