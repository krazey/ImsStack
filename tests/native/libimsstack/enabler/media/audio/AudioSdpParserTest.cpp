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

#include "SdpAttribute.h"
#include "SdpBandwidth.h"

#include "audio/AudioProfile.h"
#include "audio/AudioSdpParser.h"
#include "offeranswer/SdpAvCodec.h"

#include "core/MockISessionDescriptor.h"
#include "media/MockIMediaDescriptor.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class AudioSdpParserTest : public ::testing::Test
{
protected:
    // cppcheck-suppress knownConditionTrueFalse
    MockIMediaDescriptor m_objMockMediaDescriptor;
    MockISessionDescriptor m_objMockSessionDescriptor;
};

// A testable version of AudioSdpParser to expose protected methods for testing.
class TestableAudioSdpParser : public AudioSdpParser
{
public:
    using AudioSdpParser::ParseAmrFmtp;
    using AudioSdpParser::ParseAnbr;
    using AudioSdpParser::ParseAudioFmtp;
    using AudioSdpParser::ParseBr;
    using AudioSdpParser::ParseBrRecv;
    using AudioSdpParser::ParseBrSend;
    using AudioSdpParser::ParseBw;
    using AudioSdpParser::ParseBwRecv;
    using AudioSdpParser::ParseBwSend;
    using AudioSdpParser::ParseChAwMode;
    using AudioSdpParser::ParseCmr;
    using AudioSdpParser::ParseDtx;
    using AudioSdpParser::ParseEvents;
    using AudioSdpParser::ParseEvsFmtp;
    using AudioSdpParser::ParseEvsSwitchMode;
    using AudioSdpParser::ParseFmtp;
    using AudioSdpParser::ParseHfOnly;
    using AudioSdpParser::ParseMaxPtime;
    using AudioSdpParser::ParseMaxRed;
    using AudioSdpParser::ParseModeChangeCapability;
    using AudioSdpParser::ParseModeChangeNeighbor;
    using AudioSdpParser::ParseModeChangePeriod;
    using AudioSdpParser::ParseModeSet;
    using AudioSdpParser::ParseOctetAlign;
    using AudioSdpParser::ParsePayload;
    using AudioSdpParser::ParsePayloads;
    using AudioSdpParser::ParsePtime;
    using AudioSdpParser::ParseRtcpXr;
    using AudioSdpParser::ParseTelephoneEventFmtp;
    using AudioSdpParser::SetEvsBrVisible;
    using AudioSdpParser::SetEvsBwVisible;
    using AudioSdpParser::SetEvsFullBr;
    using AudioSdpParser::SetEvsFullBw;
    using MediaSdpParser::ParseCapaNego;
};

TEST_F(AudioSdpParserTest, Parse)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;
    ImsList<AString> lstRtcpXrAttr;
    AString objAnbrAttr = AString::ConstEmpty();

    // Mock basic session and media descriptor properties
    ON_CALL(m_objMockMediaDescriptor, GetRemoteAddress())
            .WillByDefault(Return(IpAddress("127.0.0.1")));
    ON_CALL(m_objMockMediaDescriptor, GetRemotePort()).WillByDefault(Return(5000));
    ON_CALL(m_objMockMediaDescriptor, GetDirection())
            .WillByDefault(Return(MEDIA_DIRECTION_SEND_RECEIVE));
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(lstMediaFormats));
    ON_CALL(m_objMockMediaDescriptor, GetBandwidth(SdpBandwidth::TYPE_AS, _))
            .WillByDefault(Return(64));
    ON_CALL(m_objMockMediaDescriptor, GetBandwidth(SdpBandwidth::TYPE_RS, _))
            .WillByDefault(Return(1000));
    ON_CALL(m_objMockMediaDescriptor, GetBandwidth(SdpBandwidth::TYPE_RR, _))
            .WillByDefault(Return(1000));

    ON_CALL(m_objMockMediaDescriptor, GetAttributeInt(SdpAttribute::PTIME, _))
            .WillByDefault(Return(0));
    ON_CALL(m_objMockMediaDescriptor, GetAttributeInt(SdpAttribute::MAXPTIME, _))
            .WillByDefault(Return(0));
    ON_CALL(m_objMockMediaDescriptor, GetAttributes(SdpAttribute::RTCP_XR, _))
            .WillByDefault(Return(lstRtcpXrAttr));
    ON_CALL(m_objMockMediaDescriptor, GetAttribute(SdpAttribute::ANBR, _))
            .WillByDefault(ReturnRef(objAnbrAttr));

    EXPECT_TRUE(
            objParser.Parse(&m_objMockSessionDescriptor, &m_objMockMediaDescriptor, &objProfile));
}

TEST_F(AudioSdpParserTest, ParsePayloadsValidAmrWbAndTelephoneEvent)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock AMR-WB codec
    auto pAmrWbCodec = std::make_unique<SdpAvCodec>();
    pAmrWbCodec->SetParameters("97 AMR-WB/16000", "97 octet-align=1;mode-set=0,1,2");

    // Mock telephone-event codec
    auto pDtmfCodec = std::make_unique<SdpAvCodec>();
    pDtmfCodec->SetParameters("101 telephone-event/8000", "101 0-15");

    lstMediaFormats.Append(pAmrWbCodec.get());
    lstMediaFormats.Append(pDtmfCodec.get());

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect 2 payloads to be added
    ASSERT_EQ(objProfile.GetPayloadList().GetSize(), 2);

    // Verify AMR-WB payload
    AudioProfile::Payload* pAmrWbPayload = objProfile.GetPayloadAt(0);
    ASSERT_NE(pAmrWbPayload, nullptr);
    EXPECT_EQ(pAmrWbPayload->GetRtpMap().GetPayloadNumber(), 97);
    EXPECT_TRUE(pAmrWbPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB"));
    EXPECT_EQ(pAmrWbPayload->GetRtpMap().GetSamplingRate(), 16000);

    auto pAmrFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(pAmrWbPayload->GetFmtp());
    ASSERT_NE(pAmrFmtp, nullptr);
    EXPECT_EQ(pAmrFmtp->GetOctetAlign(), 1);
    EXPECT_TRUE(pAmrFmtp->IsOctetAlignVisible());
    EXPECT_EQ(pAmrFmtp->GetModeSetList(), 7);  // 1<<0 | 1<<1 | 1<<2
    EXPECT_TRUE(pAmrFmtp->IsModeSetVisible());

    // Verify telephone-event payload
    AudioProfile::Payload* pDtmfPayload = objProfile.GetPayloadAt(1);
    ASSERT_NE(pDtmfPayload, nullptr);
    EXPECT_EQ(pDtmfPayload->GetRtpMap().GetPayloadNumber(), 101);
    EXPECT_TRUE(pDtmfPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"));
    EXPECT_EQ(pDtmfPayload->GetRtpMap().GetSamplingRate(), 8000);

    auto pDtmfFmtp =
            std::static_pointer_cast<AudioProfile::TelephoneEventFmtp>(pDtmfPayload->GetFmtp());
    ASSERT_NE(pDtmfFmtp, nullptr);
    EXPECT_TRUE(pDtmfFmtp->GetEvents().Equals("0-15"));
}

TEST_F(AudioSdpParserTest, ParsePayloadsInvalidCodecName)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock a codec with an unsupported name
    auto pInvalidCodec = std::make_unique<SdpAvCodec>();
    pInvalidCodec->SetParameters("98 unsupported/8000", "");
    lstMediaFormats.Append(pInvalidCodec.get());

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect no payloads to be added
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 0);
}

TEST_F(AudioSdpParserTest, ParsePayloadsStaticPayloadPCM)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock a PCMA codec with a static payload type
    auto pPcmCodec = std::make_unique<SdpAvCodec>();
    pPcmCodec->SetParameters("8 PCMA/8000", "");
    lstMediaFormats.Append(pPcmCodec.get());

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect 1 payload to be added for PCM
    ASSERT_EQ(objProfile.GetPayloadList().GetSize(), 1);

    // Verify PCM payload
    AudioProfile::Payload* pPcmPayload = objProfile.GetPayloadAt(0);
    ASSERT_NE(pPcmPayload, nullptr);
    EXPECT_EQ(pPcmPayload->GetRtpMap().GetPayloadNumber(), 8);
    EXPECT_TRUE(pPcmPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMA"));
    EXPECT_EQ(pPcmPayload->GetRtpMap().GetSamplingRate(), 8000);
    // PCM has no fmtp parameters to parse
    EXPECT_EQ(pPcmPayload->GetFmtp(), nullptr);
}

TEST_F(AudioSdpParserTest, ParsePayloadsValidEvs)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock EVS codec
    auto pEvsCodec = std::make_unique<SdpAvCodec>();
    pEvsCodec->SetParameters("98 EVS/16000", "98 br=9.6-24.4;bw=swb;cmr=1");

    lstMediaFormats.Append(pEvsCodec.get());

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect 1 payload to be added
    ASSERT_EQ(objProfile.GetPayloadList().GetSize(), 1);

    // Verify EVS payload
    AudioProfile::Payload* pEvsPayload = objProfile.GetPayloadAt(0);
    ASSERT_NE(pEvsPayload, nullptr);
    EXPECT_EQ(pEvsPayload->GetRtpMap().GetPayloadNumber(), 98);
    EXPECT_TRUE(pEvsPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"));
    EXPECT_EQ(pEvsPayload->GetRtpMap().GetSamplingRate(), 16000);

    auto pEvsFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pEvsPayload->GetFmtp());
    ASSERT_NE(pEvsFmtp, nullptr);
    // br=9.6-24.4 -> EVS_BR_9_6 to EVS_BR_24_4
    // (1<<3) | (1<<4) | (1<<5) | (1<<6) = 8 | 16 | 32 | 64 = 120
    EXPECT_EQ(pEvsFmtp->GetBrList(), 120);
    // bw=swb -> EVS_BW_SWB
    // (1<<2) = 4
    EXPECT_EQ(pEvsFmtp->GetBwList(), 4);
    EXPECT_EQ(pEvsFmtp->GetCmr(), 1);
    EXPECT_TRUE(pEvsFmtp->IsCmrVisible());
}

TEST_F(AudioSdpParserTest, ParseAudioFmtpModeChangeAttributes)
{
    TestableAudioSdpParser objParser;
    auto pFmtp = std::make_shared<AudioProfile::AudioFmtp>();
    ImsList<AString> objSplitEqual;

    // Test ParseModeChangeCapability
    objSplitEqual.Clear();
    objSplitEqual.Append("mode-change-capability");
    objSplitEqual.Append("1");
    EXPECT_TRUE(objParser.ParseModeChangeCapability(objSplitEqual, pFmtp));
    EXPECT_EQ(pFmtp->GetModeChangeCapability(), 1);
    EXPECT_TRUE(pFmtp->IsModeChangeCapabilityVisible());

    // Test ParseModeChangePeriod
    objSplitEqual.Clear();
    objSplitEqual.Append("mode-change-period");
    objSplitEqual.Append("2");
    EXPECT_TRUE(objParser.ParseModeChangePeriod(objSplitEqual, pFmtp));
    EXPECT_EQ(pFmtp->GetModeChangePeriod(), 2);
    EXPECT_TRUE(pFmtp->IsModeChangePeriodVisible());

    // Test ParseModeChangeNeighbor
    objSplitEqual.Clear();
    objSplitEqual.Append("mode-change-neighbor");
    objSplitEqual.Append("3");
    EXPECT_TRUE(objParser.ParseModeChangeNeighbor(objSplitEqual, pFmtp));
    EXPECT_EQ(pFmtp->GetModeChangeNeighbor(), 3);
    EXPECT_TRUE(pFmtp->IsModeChangeNeighborVisible());

    // Test ParseMaxRed
    objSplitEqual.Clear();
    objSplitEqual.Append("max-red");
    objSplitEqual.Append("100");
    EXPECT_TRUE(objParser.ParseMaxRed(objSplitEqual, pFmtp));
    EXPECT_EQ(pFmtp->GetMaxRed(), 100);
    EXPECT_TRUE(pFmtp->IsMaxRedVisible());
}

TEST_F(AudioSdpParserTest, ParseAmrFmtpOctetAlign)
{
    TestableAudioSdpParser objParser;
    auto pFmtp = std::make_shared<AudioProfile::AmrFmtp>();
    ImsList<AString> objSplitEqual;

    // Test octet-align=1
    objSplitEqual.Clear();
    objSplitEqual.Append("octet-align");
    objSplitEqual.Append("1");
    EXPECT_TRUE(objParser.ParseOctetAlign(objSplitEqual, pFmtp));
    EXPECT_EQ(pFmtp->GetOctetAlign(), 1);
    EXPECT_TRUE(pFmtp->IsOctetAlignVisible());

    // Test octet-align=0
    pFmtp = std::make_shared<AudioProfile::AmrFmtp>();  // Reset fmtp
    objSplitEqual.Clear();
    objSplitEqual.Append("octet-align");
    objSplitEqual.Append("0");
    EXPECT_TRUE(objParser.ParseOctetAlign(objSplitEqual, pFmtp));
    EXPECT_EQ(pFmtp->GetOctetAlign(), 0);
    EXPECT_TRUE(pFmtp->IsOctetAlignVisible());
}

TEST_F(AudioSdpParserTest, ParseEvsFmtpDtxHfOnlyEvsModeSwitchCmrChAwMode)
{
    TestableAudioSdpParser objParser;
    auto pFmtp = std::make_shared<AudioProfile::EvsFmtp>();
    ImsList<AString> objSplitEqual;

    // Test dtx=1
    objSplitEqual.Clear();
    objSplitEqual.Append("dtx");
    objSplitEqual.Append("1");
    EXPECT_TRUE(objParser.ParseDtx(objSplitEqual, pFmtp));
    EXPECT_TRUE(pFmtp->IsDtxEnabled());
    EXPECT_TRUE(pFmtp->IsDtxVisible());

    // Test hf-only=1
    objSplitEqual.Clear();
    objSplitEqual.Append("hf-only");
    objSplitEqual.Append("1");
    EXPECT_TRUE(objParser.ParseHfOnly(objSplitEqual, pFmtp));
    EXPECT_EQ(pFmtp->GetHfOnly(), 1);
    EXPECT_TRUE(pFmtp->IsHfOnlyVisible());

    // Test evs-mode-switch=1
    objSplitEqual.Clear();
    objSplitEqual.Append("evs-mode-switch");
    objSplitEqual.Append("1");
    EXPECT_TRUE(objParser.ParseEvsSwitchMode(objSplitEqual, pFmtp));
    EXPECT_EQ(pFmtp->GetEvsModeSwitch(), 1);
    EXPECT_TRUE(pFmtp->IsEvsModeSwitchVisible());

    // Test cmr=1
    objSplitEqual.Clear();
    objSplitEqual.Append("cmr");
    objSplitEqual.Append("1");
    EXPECT_TRUE(objParser.ParseCmr(objSplitEqual, pFmtp));
    EXPECT_EQ(pFmtp->GetCmr(), 1);
    EXPECT_TRUE(pFmtp->IsCmrVisible());

    // Test ch-aw-recv=2
    objSplitEqual.Clear();
    objSplitEqual.Append("ch-aw-recv");
    objSplitEqual.Append("2");
    EXPECT_TRUE(objParser.ParseChAwMode(objSplitEqual, pFmtp));
    EXPECT_EQ(pFmtp->GetChAwRecv(), 2);
    EXPECT_EQ(pFmtp->GetReceivedChAwRecv(), 2);
    EXPECT_TRUE(pFmtp->IsChannelAwModeVisible());
}

TEST_F(AudioSdpParserTest, ParseEvsFmtpBrBwLists)
{
    TestableAudioSdpParser objParser;
    auto pFmtp = std::make_shared<AudioProfile::EvsFmtp>();
    ImsList<AString> objSplitEqual;

    // Test br=5.9-13.2 (hyphenated range)
    objSplitEqual.Clear();
    objSplitEqual.Append("br");
    objSplitEqual.Append("5.9-13.2");
    EXPECT_TRUE(objParser.ParseBr(objSplitEqual, pFmtp));
    // 5.9 (0), 7.2 (1), 8 (2), 9.6 (3), 13.2 (4) -> (1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4) = 31
    EXPECT_EQ(pFmtp->GetBrList(), 31);

    // Test bw=nb,wb (comma-separated list)
    pFmtp = std::make_shared<AudioProfile::EvsFmtp>();  // Reset fmtp
    objSplitEqual.Clear();
    objSplitEqual.Append("bw");
    objSplitEqual.Append("nb,wb");
    EXPECT_TRUE(objParser.ParseBw(objSplitEqual, pFmtp));
    // nb (0), wb (1) -> (1<<0)|(1<<1) = 3
    EXPECT_EQ(pFmtp->GetBwList(), 3);

    // Test br-send=5.9-7.2
    pFmtp = std::make_shared<AudioProfile::EvsFmtp>();  // Reset fmtp
    objSplitEqual.Clear();
    objSplitEqual.Append("br-send");
    objSplitEqual.Append("5.9-7.2");
    EXPECT_TRUE(objParser.ParseBrSend(objSplitEqual, pFmtp));
    // 5.9 (0), 7.2 (1) -> (1<<0)|(1<<1) = 3
    EXPECT_EQ(pFmtp->GetBrRecv(), 3);  // Note: br-send parses into GetBrRecv

    // Test br-recv=8,9.6
    pFmtp = std::make_shared<AudioProfile::EvsFmtp>();  // Reset fmtp
    objSplitEqual.Clear();
    objSplitEqual.Append("br-recv");
    objSplitEqual.Append("8,9.6");
    EXPECT_TRUE(objParser.ParseBrRecv(objSplitEqual, pFmtp));
    // 8 (2), 9.6 (3) -> (1<<2)|(1<<3) = 12
    EXPECT_EQ(pFmtp->GetBrSend(), 12);  // Note: br-recv parses into GetBrSend

    // Test bw-send=nb-swb
    pFmtp = std::make_shared<AudioProfile::EvsFmtp>();  // Reset fmtp
    objSplitEqual.Clear();
    objSplitEqual.Append("bw-send");
    objSplitEqual.Append("nb-swb");
    EXPECT_TRUE(objParser.ParseBwSend(objSplitEqual, pFmtp));
    // nb (0), wb (1), swb (2) -> (1<<0)|(1<<1)|(1<<2) = 7
    EXPECT_EQ(pFmtp->GetBwRecv(), 7);  // Note: bw-send parses into GetBwRecv

    // Test bw-recv=fb
    pFmtp = std::make_shared<AudioProfile::EvsFmtp>();  // Reset fmtp
    objSplitEqual.Clear();
    objSplitEqual.Append("bw-recv");
    objSplitEqual.Append("fb");
    EXPECT_TRUE(objParser.ParseBwRecv(objSplitEqual, pFmtp));
    // fb (3) -> (1<<3) = 8
    EXPECT_EQ(pFmtp->GetBwSend(), 8);  // Note: bw-recv parses into GetBwSend
}

TEST_F(AudioSdpParserTest, ParseEventsEmptyString)
{
    TestableAudioSdpParser objParser;
    auto pFmtp = std::make_shared<AudioProfile::TelephoneEventFmtp>();
    AString emptyEvents = "";

    objParser.ParseEvents(emptyEvents, pFmtp);
    EXPECT_EQ(pFmtp->GetEvents(), "0-15");  // Should default to "0-15"
}

TEST_F(AudioSdpParserTest, ParsePtimeNotPresent)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;

    // Mock GetAttributeInt to return default/invalid value
    ON_CALL(m_objMockMediaDescriptor, GetAttributeInt(SdpAttribute::PTIME, AString::ConstNull()))
            .WillByDefault(Return(-1));  // Default for not present

    objParser.ParsePtime(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_EQ(objProfile.GetPtime(), -1);  // Should remain default -1
}

TEST_F(AudioSdpParserTest, ParseMaxPtimeNotPresent)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;

    // Mock GetAttributeInt to return default/invalid value
    ON_CALL(m_objMockMediaDescriptor, GetAttributeInt(SdpAttribute::MAXPTIME, AString::ConstNull()))
            .WillByDefault(Return(-1));  // Default for not present

    objParser.ParseMaxPtime(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_EQ(objProfile.GetMaxPtime(), -1);  // Should remain default -1
}

TEST_F(AudioSdpParserTest, ParseRtcpXrAllAttributes)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    ImsList<AString> lstRtcpXrAttr;
    lstRtcpXrAttr.Append("stat-summary=loss,dup,jitt,HL");
    lstRtcpXrAttr.Append("voip-metrics");
    lstRtcpXrAttr.Append("pkt-loss-rle");
    lstRtcpXrAttr.Append("pkt-dup-rle");

    ON_CALL(m_objMockMediaDescriptor, GetAttributes(SdpAttribute::RTCP_XR, AString::ConstNull()))
            .WillByDefault(Return(lstRtcpXrAttr));

    objParser.ParseRtcpXr(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_TRUE(objProfile.IsRtcpXrSupported());
    EXPECT_TRUE(objProfile.GetRtcpXrAttr().IsStatisticMetricsSupported());
    EXPECT_TRUE(objProfile.GetRtcpXrAttr().IsVoipMetricsSupported());
    EXPECT_TRUE(objProfile.GetRtcpXrAttr().IsPacketLossRleSupported());
    EXPECT_TRUE(objProfile.GetRtcpXrAttr().IsPacketDuplicatedRleSupported());
}

TEST_F(AudioSdpParserTest, ParseFmtpUnsupportedCodec)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    auto pPayload = new AudioProfile::Payload();
    pPayload->GetRtpMap().SetPayloadType("UNSUPPORTED");
    pPayload->GetRtpMap().SetPayloadNumber(99);

    auto pSdpCodec = std::make_unique<SdpAvCodec>();
    pSdpCodec->SetParameters("99 UNSUPPORTED/8000", "");

    EXPECT_FALSE(objParser.ParseFmtp(pSdpCodec.get(), pPayload, "UNSUPPORTED"));
    delete pPayload;
}

TEST_F(AudioSdpParserTest, ParsePayloadNullSdpAvCodec)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;

    objParser.ParsePayload(nullptr, &objProfile);
    // Expect no crash and no payloads added
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 0);
}

TEST_F(AudioSdpParserTest, SetEvsBrBwVisible)
{
    TestableAudioSdpParser objParser;
    auto pFmtp = std::make_shared<AudioProfile::EvsFmtp>();

    // Test SetEvsFullBr
    pFmtp->SetBrList(0);
    pFmtp->SetBrRecv(0);
    pFmtp->SetBrSend(0);
    objParser.SetEvsFullBr(pFmtp);
    EXPECT_EQ(pFmtp->GetBrList(), 0x0FFF);  // Should set to full list

    pFmtp->SetBrList(1);  // Not 0
    objParser.SetEvsFullBr(pFmtp);
    EXPECT_EQ(pFmtp->GetBrList(), 1);  // Should not change

    // Test SetEvsFullBw
    pFmtp->SetBwList(0);
    pFmtp->SetBwRecv(0);
    pFmtp->SetBwSend(0);
    objParser.SetEvsFullBw(pFmtp);
    EXPECT_EQ(pFmtp->GetBwList(), 0x0F);  // Should set to full list

    pFmtp->SetBwList(1);  // Not 0
    objParser.SetEvsFullBw(pFmtp);
    EXPECT_EQ(pFmtp->GetBwList(), 1);  // Should not change

    // Test SetEvsBrVisible
    pFmtp->SetBrRecv(1);
    pFmtp->SetBrSend(1);
    pFmtp->SetShowBrList(IMS_TRUE);  // Initially visible
    objParser.SetEvsBrVisible(pFmtp);
    EXPECT_FALSE(pFmtp->IsBrListVisible());  // Should become invisible

    pFmtp->SetBrRecv(0);  // One is zero
    pFmtp->SetBrSend(1);
    pFmtp->SetShowBrList(IMS_TRUE);
    objParser.SetEvsBrVisible(pFmtp);
    EXPECT_TRUE(pFmtp->IsBrListVisible());  // Should remain visible

    // Test SetEvsBwVisible
    pFmtp->SetBwRecv(1);
    pFmtp->SetBwSend(1);
    pFmtp->SetShowBwList(IMS_TRUE);  // Initially visible
    objParser.SetEvsBwVisible(pFmtp);
    EXPECT_FALSE(pFmtp->IsBwListVisible());  // Should become invisible

    pFmtp->SetBwRecv(0);  // One is zero
    pFmtp->SetBwSend(1);
    pFmtp->SetShowBwList(IMS_TRUE);
    objParser.SetEvsBwVisible(pFmtp);
    EXPECT_TRUE(pFmtp->IsBwListVisible());  // Should remain visible
}

TEST_F(AudioSdpParserTest, ParsePayloadsEvsNoFmtp)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock EVS codec with an empty fmtp line
    auto pEvsCodec = std::make_unique<SdpAvCodec>();
    pEvsCodec->SetParameters("98 EVS/16000", "");

    lstMediaFormats.Append(pEvsCodec.get());

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect 1 payload to be added
    ASSERT_EQ(objProfile.GetPayloadList().GetSize(), 1);

    // Verify EVS payload
    AudioProfile::Payload* pEvsPayload = objProfile.GetPayloadAt(0);
    ASSERT_NE(pEvsPayload, nullptr);
    EXPECT_EQ(pEvsPayload->GetRtpMap().GetPayloadNumber(), 98);
    EXPECT_TRUE(pEvsPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"));
    EXPECT_EQ(pEvsPayload->GetFmtp(), nullptr);
}

TEST_F(AudioSdpParserTest, ParsePtime)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;

    ON_CALL(m_objMockMediaDescriptor, GetAttributeInt(SdpAttribute::PTIME, AString::ConstNull()))
            .WillByDefault(Return(20));

    objParser.ParsePtime(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_EQ(objProfile.GetPtime(), 20);
}

TEST_F(AudioSdpParserTest, ParseMaxPtime)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;

    ON_CALL(m_objMockMediaDescriptor, GetAttributeInt(SdpAttribute::MAXPTIME, AString::ConstNull()))
            .WillByDefault(Return(240));

    objParser.ParseMaxPtime(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_EQ(objProfile.GetMaxPtime(), 240);
}

TEST_F(AudioSdpParserTest, ParseRtcpXrNoAttributes)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    ImsList<AString> lstRtcpXrAttr;  // Empty list

    ON_CALL(m_objMockMediaDescriptor, GetAttributes(SdpAttribute::RTCP_XR, AString::ConstNull()))
            .WillByDefault(Return(lstRtcpXrAttr));

    objParser.ParseRtcpXr(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_FALSE(objProfile.IsRtcpXrSupported());
}

TEST_F(AudioSdpParserTest, ParseAnbrSupported)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;

    ON_CALL(m_objMockMediaDescriptor, GetAttribute(SdpAttribute::ANBR, AString::ConstNull()))
            .WillByDefault(ReturnRef(AString::ConstEmpty()));

    objParser.ParseAnbr(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_TRUE(objProfile.IsAnbrSupported());
}

TEST_F(AudioSdpParserTest, ParseAnbrNotSupported)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    const AString kStrAnbrValue("some-value");

    ON_CALL(m_objMockMediaDescriptor, GetAttribute(SdpAttribute::ANBR, AString::ConstNull()))
            .WillByDefault(ReturnRef(kStrAnbrValue));

    objParser.ParseAnbr(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_FALSE(objProfile.IsAnbrSupported());
}
TEST_F(AudioSdpParserTest, ParsePayloadsEvsWithoutbrbw)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock EVS codec
    auto pEvsCodec = std::make_unique<SdpAvCodec>();
    pEvsCodec->SetParameters("98 EVS/16000", "98 cmr=1");

    lstMediaFormats.Append(pEvsCodec.get());

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect 1 payload to be added
    ASSERT_EQ(objProfile.GetPayloadList().GetSize(), 1);

    // Verify EVS payload
    AudioProfile::Payload* pEvsPayload = objProfile.GetPayloadAt(0);
    ASSERT_NE(pEvsPayload, nullptr);
    EXPECT_EQ(pEvsPayload->GetRtpMap().GetPayloadNumber(), 98);
    EXPECT_TRUE(pEvsPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"));
    EXPECT_EQ(pEvsPayload->GetRtpMap().GetSamplingRate(), 16000);

    auto pEvsFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pEvsPayload->GetFmtp());
    ASSERT_NE(pEvsFmtp, nullptr);
    EXPECT_EQ(pEvsFmtp->GetBrList(), 0x0FFF);
    EXPECT_EQ(pEvsFmtp->GetBwList(), 0x0F);
    EXPECT_EQ(pEvsFmtp->GetCmr(), 1);
    EXPECT_TRUE(pEvsFmtp->IsCmrVisible());
}
