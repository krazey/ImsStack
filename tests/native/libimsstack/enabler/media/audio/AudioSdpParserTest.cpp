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

#include <memory>

#include <gtest/gtest.h>

#include "audio/AudioProfile.h"
#include "audio/AudioSdpParser.h"
#include "offeranswer/SdpAvCodec.h"

#include "media/MockIMediaDescriptor.h"

using ::testing::Return;
using ::testing::ReturnRef;

class AudioSdpParserTest : public ::testing::Test
{
protected:
    MockIMediaDescriptor m_objMockMediaDescriptor;
};

// A testable version of AudioSdpParser to expose protected methods for testing.
class TestableAudioSdpParser : public AudioSdpParser
{
public:
    using AudioSdpParser::ParseAnbr;
    using AudioSdpParser::ParseMaxPtime;
    using AudioSdpParser::ParsePayloads;
    using AudioSdpParser::ParsePtime;
    using AudioSdpParser::ParseRtcpXr;
    using MediaSdpParser::ParseCapaNego;
};

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

TEST_F(AudioSdpParserTest, ParseRtcpXr)
{
    TestableAudioSdpParser objParser;
    AudioProfile objProfile;
    ImsList<AString> lstRtcpXrAttr;
    lstRtcpXrAttr.Append("voip-metrics");
    lstRtcpXrAttr.Append("stat-summary=loss,dup,jitt");

    ON_CALL(m_objMockMediaDescriptor, GetAttributes(SdpAttribute::RTCP_XR, AString::ConstNull()))
            .WillByDefault(Return(lstRtcpXrAttr));

    objParser.ParseRtcpXr(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_TRUE(objProfile.IsRtcpXrSupported());
    EXPECT_TRUE(objProfile.GetRtcpXrAttr().IsVoipMetricsSupported());
    EXPECT_TRUE(objProfile.GetRtcpXrAttr().IsStatisticMetricsSupported());
    EXPECT_FALSE(objProfile.GetRtcpXrAttr().IsPacketLossRleSupported());
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
