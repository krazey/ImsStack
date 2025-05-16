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

#include "config/CodecAudioConfig.h"
#include "audio/AudioProfile.h"

const AString AMR_PAYLOAD_TYPE = "AMR";
const AString AMR_WB_PAYLOAD_TYPE = "AMR-WB";
const AString EVS_PAYLOAD_TYPE = "EVS";
const AString TELEPHONY_EVENT_PAYLOAD_TYPE = "telephone-event";
const IMS_UINT32 AUDIO_FMTP_MODESET_LIST = 7;
const IMS_UINT32 AUDIO_FMTP_DEFAULT_MODESET = 7;
const IMS_SINT32 AUDIO_FMTP_MODE_CHANGE_CAPABILITY = 1;
const IMS_SINT32 AUDIO_FMTP_MODE_CHANGE_PERIOD = 2;
const IMS_SINT32 AUDIO_FMTP_MODE_CHANGE_NEIGHBOR = 2;
const IMS_SINT32 AUDIO_FMTP_MAX_RED = 220;
const IMS_BOOL AUDIO_FMTP_DTX = IMS_FALSE;
const IMS_BOOL AUDIO_FMTP_SHOW_MODESET = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_MAX_RED = IMS_TRUE;
const IMS_BOOL AUDIO_FMTP_SHOW_DTX = IMS_TRUE;
const IMS_SINT32 AMR_FMTP_OCTET_ALIGN = 1;
const IMS_BOOL AMR_FMTP_SHOW_OCTET_ALIGN = IMS_TRUE;
const IMS_UINT32 EVS_FMTP_HF_ONLY = 1;
const IMS_UINT32 EVS_FMTP_MODE_SWITCH = 1;
const IMS_UINT32 EVS_FMTP_BR_LIST = 1;
const IMS_SINT32 EVS_FMTP_BR_SEND = 1;
const IMS_SINT32 EVS_FMTP_BR_RECV = 1;
const IMS_UINT32 EVS_FMTP_BW_LIST = 1;
const IMS_SINT32 EVS_FMTP_BW_SEND = 1;
const IMS_SINT32 EVS_FMTP_BW_RECV = 1;
const IMS_SINT32 EVS_FMTP_CMR = 1;
const IMS_SINT32 EVS_FMTP_CH_AW_MODE = 1;
const IMS_SINT32 EVS_FMTP_RECEIVED_CH_AW_MODE = 1;
const IMS_BOOL EVS_FMTP_SHOW_HF_ONLY = IMS_TRUE;
const IMS_BOOL EVS_FMTP_SHOW_MODE_SWITCH = IMS_TRUE;
const IMS_BOOL EVS_FMTP_SHOW_CMR = IMS_TRUE;
const IMS_BOOL EVS_FMTP_SHOW_CH_AW_MODE = IMS_TRUE;
const IMS_BOOL EVS_FMTP_SHOW_BR_LIST = IMS_FALSE;
const IMS_BOOL EVS_FMTP_SHOW_BW_LIST = IMS_FALSE;
const IMS_BOOL EVS_FMTP_SEND_CMR = IMS_TRUE;
const AString TELEPHONY_EVENT_FMTP_EVENTS = "1-14";
const IMS_BOOL RTCP_XR_ATTR_SUPPORT_STATISTIC_METRICS = IMS_TRUE;
const IMS_BOOL RTCP_XR_ATTR_SUPPORT_VOIP_METRICS = IMS_TRUE;
const IMS_BOOL RTCP_XR_ATTR_SUPPORT_PACKET_LOSS_RLE = IMS_TRUE;
const IMS_BOOL RTCP_XR_ATTR_SUPPORT_PACKET_DUP_RLE = IMS_TRUE;
const IMS_SINT32 AUDIO_PROFILE_PTIME = 20;
const IMS_SINT32 AUDIO_PROFILE_MAX_PTIME = 240;
const AString AUDIO_PROFILE_CANDIDATE_ATTR1 = "1, UDP, 1119400811, 10.3.210.77, 7010, typ, host";
const AString AUDIO_PROFILE_CANDIDATE_ATTR2 = "2, TCP";
const IMS_BOOL AUDIO_PROFILE_SUPPORT_RTXP_XR = IMS_TRUE;
const IMS_BOOL AUDIO_PROFILE_ANBR = IMS_TRUE;

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
    EXPECT_EQ(pFmtp->GetModeChangeCapability(), CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY);

    pFmtp->SetModeChangeCapability(AUDIO_FMTP_MODE_CHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp->GetModeChangeCapability(), AUDIO_FMTP_MODE_CHANGE_CAPABILITY);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpModeChangePeriod)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetModeChangePeriod(), CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD);

    pFmtp->SetModeChangePeriod(AUDIO_FMTP_MODE_CHANGE_PERIOD);
    EXPECT_EQ(pFmtp->GetModeChangePeriod(), AUDIO_FMTP_MODE_CHANGE_PERIOD);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpModeChangeNeighbor)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetModeChangeNeighbor(), CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR);

    pFmtp->SetModeChangeNeighbor(AUDIO_FMTP_MODE_CHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp->GetModeChangeNeighbor(), AUDIO_FMTP_MODE_CHANGE_NEIGHBOR);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpMaxRed)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->GetMaxRed(), CodecAudioConfig::DEFAULT_MAXRED);

    pFmtp->SetMaxRed(AUDIO_FMTP_MAX_RED);
    EXPECT_EQ(pFmtp->GetMaxRed(), AUDIO_FMTP_MAX_RED);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAudioFmtpDtx)
{
    AudioProfile::AudioFmtp* pFmtp = new AudioProfile::AudioFmtp();
    EXPECT_EQ(pFmtp->IsDtxEnabled(), CodecAudioConfig::DEFAULT_DTX);

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
    EXPECT_EQ(pFmtp->GetModeChangeCapability(), CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp->GetModeChangePeriod(), CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD);
    EXPECT_EQ(pFmtp->GetModeChangeNeighbor(), CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp->GetMaxRed(), CodecAudioConfig::DEFAULT_MAXRED);
    EXPECT_EQ(pFmtp->IsDtxEnabled(), CodecAudioConfig::DEFAULT_DTX);
    EXPECT_EQ(pFmtp->IsModeSetVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangeCapabilityVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangePeriodVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangeNeighborVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsMaxRedVisible(), IMS_FALSE);
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
    pFmtp1->SetDtx(AUDIO_FMTP_DTX);
    pFmtp1->SetShowModeSet(AUDIO_FMTP_SHOW_MODESET);
    pFmtp1->SetShowModeChangeCapability(AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY);
    pFmtp1->SetShowModeChangePeriod(AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD);
    pFmtp1->SetShowModeChangeNeighbor(AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR);
    pFmtp1->SetShowMaxRed(AUDIO_FMTP_SHOW_MAX_RED);

    pFmtp1->SetShowDtx(AUDIO_FMTP_SHOW_DTX);

    AudioProfile::AudioFmtp* pFmtp2 = new AudioProfile::AudioFmtp(*pFmtp1);

    EXPECT_EQ(pFmtp2->GetModeSetList(), AUDIO_FMTP_MODESET_LIST);
    EXPECT_EQ(pFmtp2->GetDefaultRtpModeSet(), AUDIO_FMTP_DEFAULT_MODESET);
    EXPECT_EQ(pFmtp2->GetModeChangeCapability(), AUDIO_FMTP_MODE_CHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp2->GetModeChangePeriod(), AUDIO_FMTP_MODE_CHANGE_PERIOD);
    EXPECT_EQ(pFmtp2->GetModeChangeNeighbor(), AUDIO_FMTP_MODE_CHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp2->GetMaxRed(), AUDIO_FMTP_MAX_RED);
    EXPECT_EQ(pFmtp2->IsDtxEnabled(), AUDIO_FMTP_DTX);
    EXPECT_EQ(pFmtp2->IsModeSetVisible(), AUDIO_FMTP_SHOW_MODESET);
    EXPECT_EQ(pFmtp2->IsModeChangeCapabilityVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp2->IsModeChangePeriodVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD);
    EXPECT_EQ(pFmtp2->IsModeChangeNeighborVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp2->IsMaxRedVisible(), AUDIO_FMTP_SHOW_MAX_RED);
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

    EXPECT_EQ(pFmtp->GetModeSetList(), 0);
    EXPECT_EQ(pFmtp->GetDefaultRtpModeSet(), 0);
    EXPECT_EQ(pFmtp->GetModeChangeCapability(), CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp->GetModeChangePeriod(), CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD);
    EXPECT_EQ(pFmtp->GetModeChangeNeighbor(), CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp->GetMaxRed(), CodecAudioConfig::DEFAULT_MAXRED);
    EXPECT_EQ(pFmtp->IsDtxEnabled(), CodecAudioConfig::DEFAULT_DTX);
    EXPECT_EQ(pFmtp->IsModeSetVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangeCapabilityVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangePeriodVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangeNeighborVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsMaxRedVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsDtxVisible(), IMS_FALSE);

    EXPECT_EQ(pFmtp->GetOctetAlign(), 0);
    EXPECT_EQ(pFmtp->IsOctetAlignVisible(), IMS_FALSE);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testAmrFmtpCreation)
{
    AudioProfile::AmrFmtp* pFmtp1 = new AudioProfile::AmrFmtp();

    pFmtp1->SetModeSetList(AUDIO_FMTP_MODESET_LIST);
    pFmtp1->SetDefaultRtpModeSet(AUDIO_FMTP_DEFAULT_MODESET);
    pFmtp1->SetModeChangeCapability(AUDIO_FMTP_MODE_CHANGE_CAPABILITY);
    pFmtp1->SetModeChangePeriod(AUDIO_FMTP_MODE_CHANGE_PERIOD);
    pFmtp1->SetModeChangeNeighbor(AUDIO_FMTP_MODE_CHANGE_NEIGHBOR);
    pFmtp1->SetMaxRed(AUDIO_FMTP_MAX_RED);
    pFmtp1->SetDtx(AUDIO_FMTP_DTX);
    pFmtp1->SetShowModeSet(AUDIO_FMTP_SHOW_MODESET);
    pFmtp1->SetShowModeChangeCapability(AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY);
    pFmtp1->SetShowModeChangePeriod(AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD);
    pFmtp1->SetShowModeChangeNeighbor(AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR);
    pFmtp1->SetShowMaxRed(AUDIO_FMTP_SHOW_MAX_RED);
    pFmtp1->SetShowDtx(AUDIO_FMTP_SHOW_DTX);

    pFmtp1->SetOctetAlign(AMR_FMTP_OCTET_ALIGN);
    pFmtp1->SetShowOctetAlign(AMR_FMTP_SHOW_OCTET_ALIGN);

    AudioProfile::AmrFmtp* pFmtp2 = new AudioProfile::AmrFmtp(*pFmtp1);

    EXPECT_EQ(pFmtp2->GetModeSetList(), AUDIO_FMTP_MODESET_LIST);
    EXPECT_EQ(pFmtp2->GetDefaultRtpModeSet(), AUDIO_FMTP_DEFAULT_MODESET);
    EXPECT_EQ(pFmtp2->GetModeChangeCapability(), AUDIO_FMTP_MODE_CHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp2->GetModeChangePeriod(), AUDIO_FMTP_MODE_CHANGE_PERIOD);
    EXPECT_EQ(pFmtp2->GetModeChangeNeighbor(), AUDIO_FMTP_MODE_CHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp2->GetMaxRed(), AUDIO_FMTP_MAX_RED);
    EXPECT_EQ(pFmtp2->IsDtxEnabled(), AUDIO_FMTP_DTX);
    EXPECT_EQ(pFmtp2->IsModeSetVisible(), AUDIO_FMTP_SHOW_MODESET);
    EXPECT_EQ(pFmtp2->IsModeChangeCapabilityVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp2->IsModeChangePeriodVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD);
    EXPECT_EQ(pFmtp2->IsModeChangeNeighborVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp2->IsMaxRedVisible(), AUDIO_FMTP_SHOW_MAX_RED);
    EXPECT_EQ(pFmtp2->IsDtxVisible(), AUDIO_FMTP_SHOW_DTX);

    EXPECT_EQ(pFmtp2->GetOctetAlign(), AMR_FMTP_OCTET_ALIGN);
    EXPECT_EQ(pFmtp2->IsOctetAlignVisible(), AMR_FMTP_SHOW_OCTET_ALIGN);

    delete pFmtp1;
    delete pFmtp2;
}

TEST_F(AudioProfileTest, testEvsFmtpHfOnly)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->GetHfOnly(), 0);

    pFmtp->SetHfOnly(EVS_FMTP_HF_ONLY);
    EXPECT_EQ(pFmtp->GetHfOnly(), EVS_FMTP_HF_ONLY);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpEvsModeSwitch)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->GetEvsModeSwitch(), 0);

    pFmtp->SetEvsModeSwitch(EVS_FMTP_MODE_SWITCH);
    EXPECT_EQ(pFmtp->GetEvsModeSwitch(), EVS_FMTP_MODE_SWITCH);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpBrList)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->GetBrList(), 0);

    pFmtp->SetBrList(EVS_FMTP_BR_LIST);
    EXPECT_EQ(pFmtp->GetBrList(), EVS_FMTP_BR_LIST);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpBrSend)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->GetBrSend(), 0);

    pFmtp->SetBrSend(EVS_FMTP_BR_SEND);
    EXPECT_EQ(pFmtp->GetBrSend(), EVS_FMTP_BR_SEND);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpBrRecv)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->GetBrRecv(), 0);

    pFmtp->SetBrRecv(EVS_FMTP_BR_RECV);
    EXPECT_EQ(pFmtp->GetBrRecv(), EVS_FMTP_BR_RECV);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpBwList)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->GetBwList(), 0);

    pFmtp->SetBwList(EVS_FMTP_BW_LIST);
    EXPECT_EQ(pFmtp->GetBwList(), EVS_FMTP_BW_LIST);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpBwSend)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->GetBwSend(), 0);

    pFmtp->SetBwSend(EVS_FMTP_BW_SEND);
    EXPECT_EQ(pFmtp->GetBwSend(), EVS_FMTP_BW_SEND);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpBwRecv)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->GetBwRecv(), 0);

    pFmtp->SetBwRecv(EVS_FMTP_BW_RECV);
    EXPECT_EQ(pFmtp->GetBwRecv(), EVS_FMTP_BW_RECV);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpCmr)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->GetCmr(), 0);

    pFmtp->SetCmr(EVS_FMTP_CMR);
    EXPECT_EQ(pFmtp->GetCmr(), EVS_FMTP_CMR);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpChAwMode)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->GetChAwRecv(), 0);

    pFmtp->SetChAwRecv(EVS_FMTP_CH_AW_MODE);
    EXPECT_EQ(pFmtp->GetChAwRecv(), EVS_FMTP_CH_AW_MODE);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpReceivedChAwMode)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->GetReceivedChAwRecv(), 0);

    pFmtp->SetReceivedChAwRecv(EVS_FMTP_RECEIVED_CH_AW_MODE);
    EXPECT_EQ(pFmtp->GetReceivedChAwRecv(), EVS_FMTP_RECEIVED_CH_AW_MODE);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpShowHfOnly)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->IsHfOnlyVisible(), IMS_FALSE);

    pFmtp->SetShowHfOnly(EVS_FMTP_SHOW_HF_ONLY);
    EXPECT_EQ(pFmtp->IsHfOnlyVisible(), EVS_FMTP_SHOW_HF_ONLY);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpShowEvsModeSwitch)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->IsEvsModeSwitchVisible(), IMS_FALSE);

    pFmtp->SetShowEvsModeSwitch(EVS_FMTP_SHOW_MODE_SWITCH);
    EXPECT_EQ(pFmtp->IsEvsModeSwitchVisible(), EVS_FMTP_SHOW_MODE_SWITCH);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpShowCmr)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->IsCmrVisible(), IMS_FALSE);

    pFmtp->SetShowCmr(EVS_FMTP_SHOW_CMR);
    EXPECT_EQ(pFmtp->IsCmrVisible(), EVS_FMTP_SHOW_CMR);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpShowChAwMode)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->IsChannelAwModeVisible(), IMS_FALSE);

    pFmtp->SetShowChannelAwMode(EVS_FMTP_SHOW_CH_AW_MODE);
    EXPECT_EQ(pFmtp->IsChannelAwModeVisible(), EVS_FMTP_SHOW_CH_AW_MODE);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpShowBrList)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->IsBrListVisible(), IMS_TRUE);

    pFmtp->SetShowBrList(EVS_FMTP_SHOW_BR_LIST);
    EXPECT_EQ(pFmtp->IsBrListVisible(), EVS_FMTP_SHOW_BR_LIST);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpShowBwList)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->IsBwListVisible(), IMS_TRUE);

    pFmtp->SetShowBwList(EVS_FMTP_SHOW_BW_LIST);
    EXPECT_EQ(pFmtp->IsBwListVisible(), EVS_FMTP_SHOW_BW_LIST);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpSendCmr)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    EXPECT_EQ(pFmtp->IsSendCmrEnabled(), IMS_FALSE);

    pFmtp->SetSendCmr(EVS_FMTP_SEND_CMR);
    EXPECT_EQ(pFmtp->IsSendCmrEnabled(), EVS_FMTP_SEND_CMR);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpCreationDefault)
{
    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();

    EXPECT_EQ(pFmtp->GetModeSetList(), 0);
    EXPECT_EQ(pFmtp->GetDefaultRtpModeSet(), 0);
    EXPECT_EQ(pFmtp->GetModeChangeCapability(), CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp->GetModeChangePeriod(), CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD);
    EXPECT_EQ(pFmtp->GetModeChangeNeighbor(), CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp->GetMaxRed(), CodecAudioConfig::DEFAULT_MAXRED);
    EXPECT_EQ(pFmtp->IsDtxEnabled(), CodecAudioConfig::DEFAULT_DTX);
    EXPECT_EQ(pFmtp->IsModeSetVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangeCapabilityVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangePeriodVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsModeChangeNeighborVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsMaxRedVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsDtxVisible(), IMS_FALSE);

    EXPECT_EQ(pFmtp->GetHfOnly(), 0);
    EXPECT_EQ(pFmtp->GetEvsModeSwitch(), 0);
    EXPECT_EQ(pFmtp->GetBrList(), 0);
    EXPECT_EQ(pFmtp->GetBrSend(), 0);
    EXPECT_EQ(pFmtp->GetBrRecv(), 0);
    EXPECT_EQ(pFmtp->GetBwList(), 0);
    EXPECT_EQ(pFmtp->GetBwSend(), 0);
    EXPECT_EQ(pFmtp->GetBwRecv(), 0);
    EXPECT_EQ(pFmtp->GetCmr(), 0);
    EXPECT_EQ(pFmtp->GetChAwRecv(), 0);
    EXPECT_EQ(pFmtp->GetReceivedChAwRecv(), 0);
    EXPECT_EQ(pFmtp->IsHfOnlyVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsEvsModeSwitchVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsCmrVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsChannelAwModeVisible(), IMS_FALSE);
    EXPECT_EQ(pFmtp->IsBrListVisible(), IMS_TRUE);
    EXPECT_EQ(pFmtp->IsBwListVisible(), IMS_TRUE);
    EXPECT_EQ(pFmtp->IsSendCmrEnabled(), IMS_FALSE);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testEvsFmtpCreation)
{
    AudioProfile::EvsFmtp* pFmtp1 = new AudioProfile::EvsFmtp();

    pFmtp1->SetModeSetList(AUDIO_FMTP_MODESET_LIST);
    pFmtp1->SetDefaultRtpModeSet(AUDIO_FMTP_DEFAULT_MODESET);
    pFmtp1->SetModeChangeCapability(AUDIO_FMTP_MODE_CHANGE_CAPABILITY);
    pFmtp1->SetModeChangePeriod(AUDIO_FMTP_MODE_CHANGE_PERIOD);
    pFmtp1->SetModeChangeNeighbor(AUDIO_FMTP_MODE_CHANGE_NEIGHBOR);
    pFmtp1->SetMaxRed(AUDIO_FMTP_MAX_RED);
    pFmtp1->SetDtx(AUDIO_FMTP_DTX);
    pFmtp1->SetShowModeSet(AUDIO_FMTP_SHOW_MODESET);
    pFmtp1->SetShowModeChangeCapability(AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY);
    pFmtp1->SetShowModeChangePeriod(AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD);
    pFmtp1->SetShowModeChangeNeighbor(AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR);
    pFmtp1->SetShowMaxRed(AUDIO_FMTP_SHOW_MAX_RED);
    pFmtp1->SetShowDtx(AUDIO_FMTP_SHOW_DTX);

    pFmtp1->SetHfOnly(EVS_FMTP_HF_ONLY);
    pFmtp1->SetEvsModeSwitch(EVS_FMTP_MODE_SWITCH);
    pFmtp1->SetBrList(EVS_FMTP_BR_LIST);
    pFmtp1->SetBrSend(EVS_FMTP_BR_SEND);
    pFmtp1->SetBrRecv(EVS_FMTP_BR_RECV);
    pFmtp1->SetBwList(EVS_FMTP_BW_LIST);
    pFmtp1->SetBwSend(EVS_FMTP_BW_SEND);
    pFmtp1->SetBwRecv(EVS_FMTP_BW_RECV);
    pFmtp1->SetCmr(EVS_FMTP_CMR);
    pFmtp1->SetChAwRecv(EVS_FMTP_CH_AW_MODE);
    pFmtp1->SetReceivedChAwRecv(EVS_FMTP_RECEIVED_CH_AW_MODE);
    pFmtp1->SetShowHfOnly(EVS_FMTP_SHOW_HF_ONLY);
    pFmtp1->SetShowEvsModeSwitch(EVS_FMTP_SHOW_MODE_SWITCH);
    pFmtp1->SetShowCmr(EVS_FMTP_SHOW_CMR);
    pFmtp1->SetShowChannelAwMode(EVS_FMTP_SHOW_CH_AW_MODE);
    pFmtp1->SetShowBrList(EVS_FMTP_SHOW_BR_LIST);
    pFmtp1->SetShowBwList(EVS_FMTP_SHOW_BW_LIST);
    pFmtp1->SetSendCmr(EVS_FMTP_SEND_CMR);

    AudioProfile::EvsFmtp* pFmtp2 = new AudioProfile::EvsFmtp(*pFmtp1);

    EXPECT_EQ(pFmtp2->GetModeSetList(), AUDIO_FMTP_MODESET_LIST);
    EXPECT_EQ(pFmtp2->GetDefaultRtpModeSet(), AUDIO_FMTP_DEFAULT_MODESET);
    EXPECT_EQ(pFmtp2->GetModeChangeCapability(), AUDIO_FMTP_MODE_CHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp2->GetModeChangePeriod(), AUDIO_FMTP_MODE_CHANGE_PERIOD);
    EXPECT_EQ(pFmtp2->GetModeChangeNeighbor(), AUDIO_FMTP_MODE_CHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp2->GetMaxRed(), AUDIO_FMTP_MAX_RED);
    EXPECT_EQ(pFmtp2->IsDtxEnabled(), AUDIO_FMTP_DTX);
    EXPECT_EQ(pFmtp2->IsModeSetVisible(), AUDIO_FMTP_SHOW_MODESET);
    EXPECT_EQ(pFmtp2->IsModeChangeCapabilityVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_CAPABILITY);
    EXPECT_EQ(pFmtp2->IsModeChangePeriodVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_PERIOD);
    EXPECT_EQ(pFmtp2->IsModeChangeNeighborVisible(), AUDIO_FMTP_SHOW_MODE_CHANGE_NEIGHBOR);
    EXPECT_EQ(pFmtp2->IsMaxRedVisible(), AUDIO_FMTP_SHOW_MAX_RED);
    EXPECT_EQ(pFmtp2->IsDtxVisible(), AUDIO_FMTP_SHOW_DTX);

    EXPECT_EQ(pFmtp2->GetHfOnly(), EVS_FMTP_HF_ONLY);
    EXPECT_EQ(pFmtp2->GetEvsModeSwitch(), EVS_FMTP_MODE_SWITCH);
    EXPECT_EQ(pFmtp2->GetBrList(), EVS_FMTP_BR_LIST);
    EXPECT_EQ(pFmtp2->GetBrSend(), EVS_FMTP_BR_SEND);
    EXPECT_EQ(pFmtp2->GetBrRecv(), EVS_FMTP_BR_RECV);
    EXPECT_EQ(pFmtp2->GetBwList(), EVS_FMTP_BW_LIST);
    EXPECT_EQ(pFmtp2->GetBwSend(), EVS_FMTP_BW_SEND);
    EXPECT_EQ(pFmtp2->GetBwRecv(), EVS_FMTP_BW_RECV);
    EXPECT_EQ(pFmtp2->GetCmr(), EVS_FMTP_CMR);
    EXPECT_EQ(pFmtp2->GetChAwRecv(), EVS_FMTP_CH_AW_MODE);
    EXPECT_EQ(pFmtp2->GetReceivedChAwRecv(), EVS_FMTP_RECEIVED_CH_AW_MODE);
    EXPECT_EQ(pFmtp2->IsHfOnlyVisible(), EVS_FMTP_SHOW_HF_ONLY);
    EXPECT_EQ(pFmtp2->IsEvsModeSwitchVisible(), EVS_FMTP_SHOW_MODE_SWITCH);
    EXPECT_EQ(pFmtp2->IsCmrVisible(), EVS_FMTP_SHOW_CMR);
    EXPECT_EQ(pFmtp2->IsChannelAwModeVisible(), EVS_FMTP_SHOW_CH_AW_MODE);
    EXPECT_EQ(pFmtp2->IsBrListVisible(), EVS_FMTP_SHOW_BR_LIST);
    EXPECT_EQ(pFmtp2->IsBwListVisible(), EVS_FMTP_SHOW_BW_LIST);
    EXPECT_EQ(pFmtp2->IsSendCmrEnabled(), EVS_FMTP_SEND_CMR);

    delete pFmtp1;
    delete pFmtp2;
}

TEST_F(AudioProfileTest, testTelephonyEventFmtpEvents)
{
    AudioProfile::TelephoneEventFmtp* pFmtp = new AudioProfile::TelephoneEventFmtp();

    EXPECT_EQ(pFmtp->GetEvents(), "0-15");

    pFmtp->SetEvents(TELEPHONY_EVENT_FMTP_EVENTS);
    EXPECT_EQ(pFmtp->GetEvents(), TELEPHONY_EVENT_FMTP_EVENTS);

    delete pFmtp;
}

TEST_F(AudioProfileTest, testTelephonyEventFmtpCreationDefault)
{
    AudioProfile::TelephoneEventFmtp* pFmtp = new AudioProfile::TelephoneEventFmtp();

    EXPECT_EQ(pFmtp->GetEvents(), "0-15");

    delete pFmtp;
}

TEST_F(AudioProfileTest, testTelephonyEventFmtpCreation)
{
    AudioProfile::TelephoneEventFmtp* pFmtp1 =
            new AudioProfile::TelephoneEventFmtp(TELEPHONY_EVENT_FMTP_EVENTS);

    EXPECT_EQ(pFmtp1->GetEvents(), TELEPHONY_EVENT_FMTP_EVENTS);

    AudioProfile::TelephoneEventFmtp* pFmtp2 = new AudioProfile::TelephoneEventFmtp(*pFmtp1);
    EXPECT_EQ(pFmtp2->GetEvents(), TELEPHONY_EVENT_FMTP_EVENTS);

    delete pFmtp1;
    delete pFmtp2;
}

TEST_F(AudioProfileTest, testTelephonyEventFmtpAssign)
{
    AudioProfile::TelephoneEventFmtp* pFmtp1 =
            new AudioProfile::TelephoneEventFmtp(TELEPHONY_EVENT_FMTP_EVENTS);
    EXPECT_EQ(pFmtp1->GetEvents(), TELEPHONY_EVENT_FMTP_EVENTS);

    AudioProfile::TelephoneEventFmtp* pFmtp2 = new AudioProfile::TelephoneEventFmtp();
    *pFmtp2 = *pFmtp1;
    EXPECT_EQ(pFmtp2->GetEvents(), TELEPHONY_EVENT_FMTP_EVENTS);

    delete pFmtp1;
    delete pFmtp2;
}

TEST_F(AudioProfileTest, testTelephonyEventFmtpEqual)
{
    AudioProfile::TelephoneEventFmtp* pFmtp1 =
            new AudioProfile::TelephoneEventFmtp(TELEPHONY_EVENT_FMTP_EVENTS);
    AudioProfile::TelephoneEventFmtp* pFmtp2 = new AudioProfile::TelephoneEventFmtp();

    EXPECT_NE(*pFmtp1, *pFmtp2);

    pFmtp2->SetEvents(TELEPHONY_EVENT_FMTP_EVENTS);

    EXPECT_EQ(*pFmtp1, *pFmtp2);

    delete pFmtp1;
    delete pFmtp2;
}

TEST_F(AudioProfileTest, testAudioPayloadCreationForAmrFmtp)
{
    AudioProfile::Payload* pPayload1 = new AudioProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(AMR_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    AudioProfile::Payload* pPayload2 = new AudioProfile::Payload(*pPayload1);
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    AudioProfile::AmrFmtp* pFmtp = new AudioProfile::AmrFmtp();
    pPayload1->SetFmtp(pFmtp);

    AudioProfile::Payload* pPayload3 = new AudioProfile::Payload(*pPayload1);
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    static_cast<AudioProfile::AmrFmtp*>(pPayload3->GetFmtp())->SetOctetAlign(AMR_FMTP_OCTET_ALIGN);
    EXPECT_EQ(static_cast<AudioProfile::AmrFmtp*>(pPayload3->GetFmtp())->GetOctetAlign(),
            AMR_FMTP_OCTET_ALIGN);

    delete pPayload1;
    delete pPayload2;
    delete pPayload3;
}

TEST_F(AudioProfileTest, testAudioPayloadCreationForAmrWbFmtp)
{
    AudioProfile::Payload* pPayload1 = new AudioProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(AMR_WB_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    AudioProfile::Payload* pPayload2 = new AudioProfile::Payload(*pPayload1);
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    AudioProfile::AmrFmtp* pFmtp = new AudioProfile::AmrFmtp();
    pPayload1->SetFmtp(pFmtp);

    AudioProfile::Payload* pPayload3 = new AudioProfile::Payload(*pPayload1);
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    static_cast<AudioProfile::AmrFmtp*>(pPayload3->GetFmtp())
            ->SetShowOctetAlign(AMR_FMTP_SHOW_OCTET_ALIGN);
    EXPECT_EQ(static_cast<AudioProfile::AmrFmtp*>(pPayload3->GetFmtp())->IsOctetAlignVisible(),
            AMR_FMTP_SHOW_OCTET_ALIGN);

    delete pPayload1;
    delete pPayload2;
    delete pPayload3;
}

TEST_F(AudioProfileTest, testAudioPayloadCreationForEvsFmtp)
{
    AudioProfile::Payload* pPayload1 = new AudioProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(EVS_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    AudioProfile::Payload* pPayload2 = new AudioProfile::Payload(*pPayload1);
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    pPayload1->SetFmtp(pFmtp);

    AudioProfile::Payload* pPayload3 = new AudioProfile::Payload(*pPayload1);
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    static_cast<AudioProfile::EvsFmtp*>(pPayload3->GetFmtp())->SetHfOnly(EVS_FMTP_HF_ONLY);
    EXPECT_EQ(static_cast<AudioProfile::EvsFmtp*>(pPayload3->GetFmtp())->GetHfOnly(),
            EVS_FMTP_HF_ONLY);

    delete pPayload1;
    delete pPayload2;
    delete pPayload3;
}

TEST_F(AudioProfileTest, testAudioPayloadCreationForTelephoneEventFmtp)
{
    AudioProfile::Payload* pPayload1 = new AudioProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(TELEPHONY_EVENT_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    AudioProfile::Payload* pPayload2 = new AudioProfile::Payload(*pPayload1);
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    AudioProfile::TelephoneEventFmtp* pFmtp = new AudioProfile::TelephoneEventFmtp();
    pPayload1->SetFmtp(pFmtp);

    AudioProfile::Payload* pPayload3 = new AudioProfile::Payload(*pPayload1);
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    static_cast<AudioProfile::TelephoneEventFmtp*>(pPayload3->GetFmtp())
            ->SetEvents(TELEPHONY_EVENT_FMTP_EVENTS);
    EXPECT_EQ(static_cast<AudioProfile::TelephoneEventFmtp*>(pPayload3->GetFmtp())->GetEvents(),
            TELEPHONY_EVENT_FMTP_EVENTS);

    delete pPayload1;
    delete pPayload2;
    delete pPayload3;
}

TEST_F(AudioProfileTest, testAudioPayloadAssignForAmrFmtp)
{
    AudioProfile::Payload* pPayload1 = new AudioProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(AMR_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    AudioProfile::Payload* pPayload2 = new AudioProfile::Payload();
    *pPayload2 = *pPayload1;
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    AudioProfile::AmrFmtp* pFmtp = new AudioProfile::AmrFmtp();
    pPayload1->SetFmtp(pFmtp);

    *pPayload2 = *pPayload1;
    EXPECT_NE(pPayload2->GetFmtp(), nullptr);

    static_cast<AudioProfile::AmrFmtp*>(pPayload2->GetFmtp())->SetOctetAlign(AMR_FMTP_OCTET_ALIGN);
    EXPECT_EQ(static_cast<AudioProfile::AmrFmtp*>(pPayload2->GetFmtp())->GetOctetAlign(),
            AMR_FMTP_OCTET_ALIGN);

    delete pPayload1;
    delete pPayload2;
}

TEST_F(AudioProfileTest, testAudioPayloadAssignForAmrWbFmtp)
{
    AudioProfile::Payload* pPayload1 = new AudioProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(AMR_WB_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    AudioProfile::Payload* pPayload2 = new AudioProfile::Payload();
    *pPayload2 = *pPayload1;
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    AudioProfile::AmrFmtp* pFmtp = new AudioProfile::AmrFmtp();
    pPayload1->SetFmtp(pFmtp);

    *pPayload2 = *pPayload1;
    EXPECT_NE(pPayload2->GetFmtp(), nullptr);

    static_cast<AudioProfile::AmrFmtp*>(pPayload2->GetFmtp())
            ->SetShowOctetAlign(AMR_FMTP_SHOW_OCTET_ALIGN);
    EXPECT_EQ(static_cast<AudioProfile::AmrFmtp*>(pPayload2->GetFmtp())->IsOctetAlignVisible(),
            AMR_FMTP_SHOW_OCTET_ALIGN);

    delete pPayload1;
    delete pPayload2;
}

TEST_F(AudioProfileTest, testAudioPayloadAssignForEvsFmtp)
{
    AudioProfile::Payload* pPayload1 = new AudioProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(EVS_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    AudioProfile::Payload* pPayload2 = new AudioProfile::Payload();
    *pPayload2 = *pPayload1;
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    AudioProfile::EvsFmtp* pFmtp = new AudioProfile::EvsFmtp();
    pPayload1->SetFmtp(pFmtp);

    *pPayload2 = *pPayload1;
    EXPECT_NE(pPayload2->GetFmtp(), nullptr);

    static_cast<AudioProfile::EvsFmtp*>(pPayload2->GetFmtp())->SetHfOnly(EVS_FMTP_HF_ONLY);
    EXPECT_EQ(static_cast<AudioProfile::EvsFmtp*>(pPayload2->GetFmtp())->GetHfOnly(),
            EVS_FMTP_HF_ONLY);

    delete pPayload1;
    delete pPayload2;
}

TEST_F(AudioProfileTest, testAudioPayloadAssignForTelephoneEventFmtp)
{
    AudioProfile::Payload* pPayload1 = new AudioProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(TELEPHONY_EVENT_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    AudioProfile::Payload* pPayload2 = new AudioProfile::Payload();
    *pPayload2 = *pPayload1;
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    AudioProfile::TelephoneEventFmtp* pFmtp = new AudioProfile::TelephoneEventFmtp();
    pPayload1->SetFmtp(pFmtp);

    *pPayload2 = *pPayload1;
    EXPECT_NE(pPayload2->GetFmtp(), nullptr);

    static_cast<AudioProfile::TelephoneEventFmtp*>(pPayload2->GetFmtp())
            ->SetEvents(TELEPHONY_EVENT_FMTP_EVENTS);
    EXPECT_EQ(static_cast<AudioProfile::TelephoneEventFmtp*>(pPayload2->GetFmtp())->GetEvents(),
            TELEPHONY_EVENT_FMTP_EVENTS);

    delete pPayload1;
    delete pPayload2;
}

TEST_F(AudioProfileTest, testRtcpXrAttributesSupportStatisticMetrics)
{
    AudioProfile::RtcpXrAttributes* pRtcpXrAttr = new AudioProfile::RtcpXrAttributes();

    EXPECT_EQ(pRtcpXrAttr->IsStatisticMetricsSupported(), IMS_FALSE);

    pRtcpXrAttr->SetSupportStatisticMetrics(RTCP_XR_ATTR_SUPPORT_STATISTIC_METRICS);
    EXPECT_EQ(pRtcpXrAttr->IsStatisticMetricsSupported(), RTCP_XR_ATTR_SUPPORT_STATISTIC_METRICS);

    delete pRtcpXrAttr;
}

TEST_F(AudioProfileTest, testRtcpXrAttributesSupportVoipMetrics)
{
    AudioProfile::RtcpXrAttributes* pRtcpXrAttr = new AudioProfile::RtcpXrAttributes();

    EXPECT_EQ(pRtcpXrAttr->IsVoipMetricsSupported(), IMS_FALSE);

    pRtcpXrAttr->SetSupportVoipMetrics(RTCP_XR_ATTR_SUPPORT_VOIP_METRICS);
    EXPECT_EQ(pRtcpXrAttr->IsVoipMetricsSupported(), RTCP_XR_ATTR_SUPPORT_VOIP_METRICS);

    delete pRtcpXrAttr;
}

TEST_F(AudioProfileTest, testRtcpXrAttributesSupportPacketLossRle)
{
    AudioProfile::RtcpXrAttributes* pRtcpXrAttr = new AudioProfile::RtcpXrAttributes();

    EXPECT_EQ(pRtcpXrAttr->IsPacketLossRleSupported(), IMS_FALSE);

    pRtcpXrAttr->SetSupportPacketLossRle(RTCP_XR_ATTR_SUPPORT_PACKET_LOSS_RLE);
    EXPECT_EQ(pRtcpXrAttr->IsPacketLossRleSupported(), RTCP_XR_ATTR_SUPPORT_PACKET_LOSS_RLE);

    delete pRtcpXrAttr;
}

TEST_F(AudioProfileTest, testRtcpXrAttributesSupportPacketDuplicatedRle)
{
    AudioProfile::RtcpXrAttributes* pRtcpXrAttr = new AudioProfile::RtcpXrAttributes();

    EXPECT_EQ(pRtcpXrAttr->IsPacketDuplicatedRleSupported(), IMS_FALSE);

    pRtcpXrAttr->SetSupportPacketDuplicatedRle(RTCP_XR_ATTR_SUPPORT_PACKET_DUP_RLE);
    EXPECT_EQ(pRtcpXrAttr->IsPacketDuplicatedRleSupported(), RTCP_XR_ATTR_SUPPORT_PACKET_DUP_RLE);

    delete pRtcpXrAttr;
}

TEST_F(AudioProfileTest, testRtcpXrAttributesCreation)
{
    AudioProfile::RtcpXrAttributes* pRtcpXrAttr = new AudioProfile::RtcpXrAttributes();

    EXPECT_EQ(pRtcpXrAttr->IsStatisticMetricsSupported(), IMS_FALSE);
    EXPECT_EQ(pRtcpXrAttr->IsVoipMetricsSupported(), IMS_FALSE);
    EXPECT_EQ(pRtcpXrAttr->IsPacketLossRleSupported(), IMS_FALSE);
    EXPECT_EQ(pRtcpXrAttr->IsPacketDuplicatedRleSupported(), IMS_FALSE);

    delete pRtcpXrAttr;
}

TEST_F(AudioProfileTest, testRtcpXrAttributesAssign)
{
    AudioProfile::RtcpXrAttributes* pRtcpXrAttr1 = new AudioProfile::RtcpXrAttributes();
    AudioProfile::RtcpXrAttributes* pRtcpXrAttr2 = new AudioProfile::RtcpXrAttributes();

    pRtcpXrAttr1->SetSupportStatisticMetrics(RTCP_XR_ATTR_SUPPORT_STATISTIC_METRICS);
    pRtcpXrAttr1->SetSupportVoipMetrics(RTCP_XR_ATTR_SUPPORT_VOIP_METRICS);
    pRtcpXrAttr1->SetSupportPacketLossRle(RTCP_XR_ATTR_SUPPORT_PACKET_LOSS_RLE);
    pRtcpXrAttr1->SetSupportPacketDuplicatedRle(RTCP_XR_ATTR_SUPPORT_PACKET_DUP_RLE);

    *pRtcpXrAttr2 = *pRtcpXrAttr1;

    EXPECT_EQ(pRtcpXrAttr2->IsStatisticMetricsSupported(), RTCP_XR_ATTR_SUPPORT_STATISTIC_METRICS);
    EXPECT_EQ(pRtcpXrAttr2->IsVoipMetricsSupported(), RTCP_XR_ATTR_SUPPORT_VOIP_METRICS);
    EXPECT_EQ(pRtcpXrAttr2->IsPacketLossRleSupported(), RTCP_XR_ATTR_SUPPORT_PACKET_LOSS_RLE);
    EXPECT_EQ(pRtcpXrAttr2->IsPacketDuplicatedRleSupported(), RTCP_XR_ATTR_SUPPORT_PACKET_DUP_RLE);

    delete pRtcpXrAttr1;
    delete pRtcpXrAttr2;
}

TEST_F(AudioProfileTest, testRtcpXrAttributesEqualNotEqual)
{
    AudioProfile::RtcpXrAttributes* pRtcpXrAttr1 = new AudioProfile::RtcpXrAttributes();
    AudioProfile::RtcpXrAttributes* pRtcpXrAttr2 = new AudioProfile::RtcpXrAttributes();

    EXPECT_EQ(*pRtcpXrAttr2, *pRtcpXrAttr1);

    pRtcpXrAttr1->SetSupportStatisticMetrics(RTCP_XR_ATTR_SUPPORT_STATISTIC_METRICS);
    EXPECT_NE(*pRtcpXrAttr2, *pRtcpXrAttr1);

    pRtcpXrAttr2->SetSupportStatisticMetrics(RTCP_XR_ATTR_SUPPORT_STATISTIC_METRICS);
    EXPECT_EQ(*pRtcpXrAttr2, *pRtcpXrAttr1);

    pRtcpXrAttr1->SetSupportVoipMetrics(RTCP_XR_ATTR_SUPPORT_VOIP_METRICS);
    EXPECT_NE(*pRtcpXrAttr2, *pRtcpXrAttr1);

    pRtcpXrAttr2->SetSupportVoipMetrics(RTCP_XR_ATTR_SUPPORT_VOIP_METRICS);
    EXPECT_EQ(*pRtcpXrAttr2, *pRtcpXrAttr1);

    pRtcpXrAttr1->SetSupportPacketLossRle(RTCP_XR_ATTR_SUPPORT_PACKET_LOSS_RLE);
    EXPECT_NE(*pRtcpXrAttr2, *pRtcpXrAttr1);

    pRtcpXrAttr2->SetSupportPacketLossRle(RTCP_XR_ATTR_SUPPORT_PACKET_LOSS_RLE);
    EXPECT_EQ(*pRtcpXrAttr2, *pRtcpXrAttr1);

    pRtcpXrAttr1->SetSupportPacketDuplicatedRle(RTCP_XR_ATTR_SUPPORT_PACKET_DUP_RLE);
    EXPECT_NE(*pRtcpXrAttr2, *pRtcpXrAttr1);

    pRtcpXrAttr2->SetSupportPacketDuplicatedRle(RTCP_XR_ATTR_SUPPORT_PACKET_DUP_RLE);
    EXPECT_EQ(*pRtcpXrAttr2, *pRtcpXrAttr1);

    delete pRtcpXrAttr1;
    delete pRtcpXrAttr2;
}

TEST_F(AudioProfileTest, testAudioProfileCandidateAttr)
{
    AudioProfile* pProfile = new AudioProfile();

    EXPECT_EQ(pProfile->GetCandidateAttr(), ImsVector<AString>());

    ImsVector<AString> objCandidateAttr;
    objCandidateAttr.Add(AUDIO_PROFILE_CANDIDATE_ATTR1);
    objCandidateAttr.Add(AUDIO_PROFILE_CANDIDATE_ATTR2);

    pProfile->SetCandidateAttr(objCandidateAttr);
    EXPECT_EQ(pProfile->GetCandidateAttr().GetAt(0), AUDIO_PROFILE_CANDIDATE_ATTR1);
    EXPECT_EQ(pProfile->GetCandidateAttr().GetAt(1), AUDIO_PROFILE_CANDIDATE_ATTR2);

    delete pProfile;
}

TEST_F(AudioProfileTest, testAudioProfileSupportRtcpXr)
{
    AudioProfile* pProfile = new AudioProfile();

    EXPECT_EQ(pProfile->IsRtcpXrSupported(), IMS_FALSE);

    pProfile->SetSupportRtcpXr(AUDIO_PROFILE_SUPPORT_RTXP_XR);
    EXPECT_EQ(pProfile->IsRtcpXrSupported(), AUDIO_PROFILE_SUPPORT_RTXP_XR);

    delete pProfile;
}

TEST_F(AudioProfileTest, testAudioProfileRtcpXrAttr)
{
    AudioProfile* pProfile = new AudioProfile();

    EXPECT_EQ(pProfile->GetRtcpXrAttr(), AudioProfile::RtcpXrAttributes());

    AudioProfile::RtcpXrAttributes objRtcpXrAttr;
    objRtcpXrAttr.SetSupportStatisticMetrics(RTCP_XR_ATTR_SUPPORT_STATISTIC_METRICS);
    objRtcpXrAttr.SetSupportVoipMetrics(RTCP_XR_ATTR_SUPPORT_VOIP_METRICS);
    objRtcpXrAttr.SetSupportPacketLossRle(RTCP_XR_ATTR_SUPPORT_PACKET_LOSS_RLE);
    objRtcpXrAttr.SetSupportPacketDuplicatedRle(RTCP_XR_ATTR_SUPPORT_PACKET_DUP_RLE);

    pProfile->SetRtcpXrAttr(objRtcpXrAttr);
    EXPECT_EQ(pProfile->GetRtcpXrAttr(), objRtcpXrAttr);
    EXPECT_EQ(pProfile->GetRtcpXrAttr().IsStatisticMetricsSupported(),
            RTCP_XR_ATTR_SUPPORT_STATISTIC_METRICS);
    EXPECT_EQ(
            pProfile->GetRtcpXrAttr().IsVoipMetricsSupported(), RTCP_XR_ATTR_SUPPORT_VOIP_METRICS);
    EXPECT_EQ(pProfile->GetRtcpXrAttr().IsPacketLossRleSupported(),
            RTCP_XR_ATTR_SUPPORT_PACKET_LOSS_RLE);
    EXPECT_EQ(pProfile->GetRtcpXrAttr().IsPacketDuplicatedRleSupported(),
            RTCP_XR_ATTR_SUPPORT_PACKET_DUP_RLE);

    delete pProfile;
}

TEST_F(AudioProfileTest, testAudioProfileAnbr)
{
    AudioProfile* pProfile = new AudioProfile();

    EXPECT_EQ(pProfile->IsAnbrSupported(), IMS_FALSE);

    pProfile->SetAnbr(AUDIO_PROFILE_ANBR);
    EXPECT_EQ(pProfile->IsAnbrSupported(), AUDIO_PROFILE_ANBR);

    delete pProfile;
}

TEST_F(AudioProfileTest, testAudioProfileCreationDefault)
{
    AudioProfile* pProfile = new AudioProfile();

    EXPECT_EQ(pProfile->GetPtime(), 0);
    EXPECT_EQ(pProfile->GetMaxPtime(), 0);
    EXPECT_EQ(pProfile->GetCandidateAttr(), ImsVector<AString>());
    EXPECT_EQ(pProfile->IsRtcpXrSupported(), IMS_FALSE);
    EXPECT_EQ(pProfile->GetRtcpXrAttr(), AudioProfile::RtcpXrAttributes());
    EXPECT_EQ(pProfile->IsAnbrSupported(), IMS_FALSE);

    delete pProfile;
}

TEST_F(AudioProfileTest, testAudioProfileCreation)
{
    AudioProfile::RtcpXrAttributes objRtcpXrAttr;
    objRtcpXrAttr.SetSupportStatisticMetrics(RTCP_XR_ATTR_SUPPORT_STATISTIC_METRICS);
    objRtcpXrAttr.SetSupportVoipMetrics(RTCP_XR_ATTR_SUPPORT_VOIP_METRICS);

    ImsVector<AString> objCandidateAttr;
    objCandidateAttr.Add(AUDIO_PROFILE_CANDIDATE_ATTR1);

    AudioProfile* pProfile1 = new AudioProfile();

    pProfile1->SetPtime(AUDIO_PROFILE_PTIME);
    pProfile1->SetMaxPtime(AUDIO_PROFILE_MAX_PTIME);
    pProfile1->SetCandidateAttr(objCandidateAttr);
    pProfile1->SetSupportRtcpXr(AUDIO_PROFILE_SUPPORT_RTXP_XR);
    pProfile1->SetRtcpXrAttr(objRtcpXrAttr);
    pProfile1->SetAnbr(AUDIO_PROFILE_ANBR);

    AudioProfile* pProfile2 = new AudioProfile(*pProfile1);

    EXPECT_EQ(*pProfile1, *pProfile2);

    delete pProfile1;
    delete pProfile2;
}

TEST_F(AudioProfileTest, testAudioProfileAssign)
{
    AudioProfile::RtcpXrAttributes objRtcpXrAttr;
    objRtcpXrAttr.SetSupportPacketLossRle(RTCP_XR_ATTR_SUPPORT_PACKET_LOSS_RLE);
    objRtcpXrAttr.SetSupportPacketDuplicatedRle(RTCP_XR_ATTR_SUPPORT_PACKET_DUP_RLE);

    ImsVector<AString> objCandidateAttr;
    objCandidateAttr.Add(AUDIO_PROFILE_CANDIDATE_ATTR2);

    AudioProfile* pProfile1 = new AudioProfile();
    AudioProfile* pProfile2 = new AudioProfile();

    pProfile1->SetPtime(AUDIO_PROFILE_PTIME);
    pProfile1->SetMaxPtime(AUDIO_PROFILE_MAX_PTIME);
    pProfile1->SetCandidateAttr(objCandidateAttr);
    pProfile1->SetSupportRtcpXr(AUDIO_PROFILE_SUPPORT_RTXP_XR);
    pProfile1->SetRtcpXrAttr(objRtcpXrAttr);
    pProfile1->SetAnbr(AUDIO_PROFILE_ANBR);

    *pProfile2 = *pProfile1;

    EXPECT_EQ(*pProfile1, *pProfile2);

    delete pProfile1;
    delete pProfile2;
}

TEST_F(AudioProfileTest, testAudioProfileEqualNotEqual)
{
    AudioProfile::RtcpXrAttributes objRtcpXrAttr;
    objRtcpXrAttr.SetSupportPacketLossRle(RTCP_XR_ATTR_SUPPORT_PACKET_LOSS_RLE);
    objRtcpXrAttr.SetSupportPacketDuplicatedRle(RTCP_XR_ATTR_SUPPORT_PACKET_DUP_RLE);

    AudioProfile* pProfile1 = new AudioProfile();
    AudioProfile* pProfile2 = new AudioProfile();

    EXPECT_EQ(*pProfile1, *pProfile2);

    pProfile1->SetPtime(AUDIO_PROFILE_PTIME);
    EXPECT_NE(*pProfile1, *pProfile2);

    pProfile2->SetPtime(AUDIO_PROFILE_PTIME);
    EXPECT_EQ(*pProfile1, *pProfile2);

    pProfile1->SetMaxPtime(AUDIO_PROFILE_MAX_PTIME);
    EXPECT_NE(*pProfile1, *pProfile2);
    pProfile2->SetMaxPtime(AUDIO_PROFILE_MAX_PTIME);
    EXPECT_EQ(*pProfile1, *pProfile2);

    ImsVector<AString> objCandidateAttr;
    objCandidateAttr.Add(AUDIO_PROFILE_CANDIDATE_ATTR2);
    pProfile1->SetCandidateAttr(objCandidateAttr);
    EXPECT_NE(*pProfile1, *pProfile2);
    pProfile2->SetCandidateAttr(objCandidateAttr);
    EXPECT_EQ(*pProfile1, *pProfile2);

    pProfile1->SetSupportRtcpXr(AUDIO_PROFILE_SUPPORT_RTXP_XR);
    EXPECT_NE(*pProfile1, *pProfile2);

    pProfile2->SetSupportRtcpXr(AUDIO_PROFILE_SUPPORT_RTXP_XR);
    EXPECT_EQ(*pProfile1, *pProfile2);

    pProfile1->SetRtcpXrAttr(objRtcpXrAttr);
    EXPECT_NE(*pProfile1, *pProfile2);

    pProfile2->SetRtcpXrAttr(objRtcpXrAttr);
    EXPECT_EQ(*pProfile1, *pProfile2);

    pProfile1->SetAnbr(AUDIO_PROFILE_ANBR);
    EXPECT_NE(*pProfile1, *pProfile2);

    pProfile2->SetAnbr(AUDIO_PROFILE_ANBR);
    EXPECT_EQ(*pProfile1, *pProfile2);

    delete pProfile1;
    delete pProfile2;
}
