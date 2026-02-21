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

#include "IpAddress.h"
#include "SdpBandwidth.h"
#include "offeranswer/SdpMediaFormat.h"
#include "offeranswer/SdpAvCodec.h"
#include "text/TextSdpParser.h"
#include "text/TextProfile.h"

#include "core/MockISessionDescriptor.h"
#include "media/MockIMediaDescriptor.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class TextSdpParserTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_pParser = std::make_unique<TextSdpParser>();
        m_pProfile = std::make_unique<TextProfile>();
    }

    void TearDown() override {}

    std::unique_ptr<TextSdpParser> m_pParser;
    std::unique_ptr<TextProfile> m_pProfile;
    MockISessionDescriptor m_objMockSessionDescriptor;
    // cppcheck-suppress unusedStructMember
    MockIMediaDescriptor m_objMockMediaDescriptor;
};

class TestableTextSdpParser : public TextSdpParser
{
public:
    using TextSdpParser::ParseFmtp;
    using TextSdpParser::ParsePayloads;
    using TextSdpParser::ParseRedFmtp;
    using TextSdpParser::ParseRedSubPtExist;
};

TEST_F(TextSdpParserTest, ParsePayloadsValidT140AndRed)
{
    TestableTextSdpParser objParser;
    TextProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock t140 codec
    auto pT140Codec = std::make_unique<SdpAvCodec>();
    pT140Codec->SetParameters("100 t140/1000", "100 cps=30");

    // Mock red codec
    auto pRedCodec = std::make_unique<SdpAvCodec>();
    pRedCodec->SetParameters("101 red/1000", "101 100/100");

    lstMediaFormats.Append(pT140Codec.get());
    lstMediaFormats.Append(pRedCodec.get());

    // The mock returns a const reference, so we need a const variable to hold it for the ReturnRef
    // call.
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(lstMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect 2 payloads to be added
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 2);

    // Verify t140 objPayload
    TextProfile::Payload* pT140Payload = objProfile.GetPayloadAt(0);
    ASSERT_NE(pT140Payload, nullptr);
    EXPECT_EQ(pT140Payload->GetRtpMap().GetPayloadNumber(), 100);
    EXPECT_TRUE(pT140Payload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140"));
    auto pT140Fmtp = std::static_pointer_cast<TextProfile::T140Fmtp>(pT140Payload->GetFmtp());
    ASSERT_NE(pT140Fmtp, nullptr);
    EXPECT_EQ(pT140Fmtp->GetCps(), 30);

    // Verify red objPayload
    TextProfile::Payload* pRedPayload = objProfile.GetPayloadAt(1);
    ASSERT_NE(pRedPayload, nullptr);
    EXPECT_EQ(pRedPayload->GetRtpMap().GetPayloadNumber(), 101);
    EXPECT_TRUE(pRedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"));
    auto pRedFmtp = std::static_pointer_cast<TextProfile::RedFmtp>(pRedPayload->GetFmtp());
    ASSERT_NE(pRedFmtp, nullptr);
    EXPECT_EQ(pRedFmtp->GetRedPayload(), 100);
    EXPECT_EQ(pRedFmtp->GetRedLevel(), 2);
}

TEST_F(TextSdpParserTest, ParsePayloadsInvalidPayloadType)
{
    TestableTextSdpParser objParser;
    TextProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock a codec with a static objPayload type, which should be rejected
    auto pStaticCodec = std::make_unique<SdpAvCodec>();
    pStaticCodec->SetParameters("8 PCMA/8000", "");
    lstMediaFormats.Append(pStaticCodec.get());

    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(lstMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect no payloads to be added
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 0);
}

TEST_F(TextSdpParserTest, ParsePayloadsInvalidCodecName)
{
    TestableTextSdpParser objParser;
    TextProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock a codec with an unsupported name
    auto pInvalidCodec = std::make_unique<SdpAvCodec>();
    pInvalidCodec->SetParameters("98 unsupported/1000", "");
    lstMediaFormats.Append(pInvalidCodec.get());

    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(lstMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect no payloads to be added
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 0);
}

TEST_F(TextSdpParserTest, ParsePayloadsRedFmtpWithoutMatchingSubtype)
{
    TestableTextSdpParser objParser;
    TextProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock a 'red' codec whose subtype (105) is not in the media format list
    auto pRedCodec = std::make_unique<SdpAvCodec>();
    pRedCodec->SetParameters("101 red/1000", "101 105/105");
    lstMediaFormats.Append(pRedCodec.get());

    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(lstMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect only 'red' objPayload to be added
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 1);
}

TEST_F(TextSdpParserTest, ParseFullSdp)
{
    const IpAddress objRemoteAddress("192.168.1.1");
    ImsList<SdpMediaFormat*> lstMediaFormats;
    auto pT140Codec = std::make_unique<SdpAvCodec>();
    pT140Codec->SetParameters("100 t140/1000", "100 cps=30");
    lstMediaFormats.Append(pT140Codec.get());

    // Mock media descriptor attributes
    ON_CALL(m_objMockMediaDescriptor, GetRemoteAddress()).WillByDefault(Return(objRemoteAddress));
    ON_CALL(m_objMockMediaDescriptor, GetRemotePort()).WillByDefault(Return(8000));
    ON_CALL(m_objMockMediaDescriptor, GetDirection()).WillByDefault(Return(MEDIA_DIRECTION_SEND));
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(lstMediaFormats));
    ON_CALL(m_objMockMediaDescriptor, GetBandwidth(SdpBandwidth::TYPE_AS, _))
            .WillByDefault(Return(10));

    // Act
    m_pParser->Parse(&m_objMockSessionDescriptor, &m_objMockMediaDescriptor, m_pProfile.get());

    // Assert
    EXPECT_EQ(m_pProfile->GetIpAddress(), objRemoteAddress);
    EXPECT_EQ(m_pProfile->GetDataPort(), 8000);
    EXPECT_EQ(m_pProfile->GetDirection(), MEDIA_DIRECTION_SEND);
    EXPECT_EQ(m_pProfile->GetBandwidthAs(), 10);
    ASSERT_EQ(m_pProfile->GetPayloadListSize(), 1);
    EXPECT_EQ(m_pProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), 100);
}

TEST_F(TextSdpParserTest, ParseFmtpNullSdpCodec)
{
    TestableTextSdpParser objParser;
    TextProfile::Payload objPayload;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Calling with a null codec should fail gracefully.
    EXPECT_FALSE(objParser.ParseFmtp(nullptr, &objPayload, lstMediaFormats));
}

TEST_F(TextSdpParserTest, ParseFmtpT140WithEmptyFmtp)
{
    TestableTextSdpParser objParser;
    TextProfile::Payload objPayload;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    auto pT140Codec = std::make_unique<SdpAvCodec>();
    pT140Codec->SetParameters("100 t140/1000", "");  // Empty fmtp string

    // ParseFmtp for t140 should success
    EXPECT_TRUE(objParser.ParseFmtp(pT140Codec.get(), &objPayload, lstMediaFormats));
    auto pRedFmtp = objPayload.GetFmtp();
    ASSERT_NE(pRedFmtp, nullptr);
}

TEST_F(TextSdpParserTest, ParseRtpMapNullSdpCodec)
{
    // Arrange: Set up the media descriptor to return a list with a nullptr codec.
    ImsList<SdpMediaFormat*> lstMediaFormats;
    lstMediaFormats.Append(nullptr);

    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(lstMediaFormats));

    // Act: Call the public Parse method.
    m_pParser->Parse(&m_objMockSessionDescriptor, &m_objMockMediaDescriptor, m_pProfile.get());

    // Assert: The parser should handle the null codec gracefully and add no payloads.
    EXPECT_EQ(m_pProfile->GetPayloadListSize(), 0);
}

TEST_F(TextSdpParserTest, ParseRedFmtpWithZeroLevel)
{
    TestableTextSdpParser objParser;
    TextProfile::Payload objPayload;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    auto pRedCodec = std::make_unique<SdpAvCodec>();
    // fmtp has a level of 0, which is invalid.
    pRedCodec->SetParameters("102 red/1000", "102");
    auto pRedFmtp = std::make_shared<TextProfile::RedFmtp>();

    // The ParseRedFmtp should fail
    EXPECT_FALSE(objParser.ParseRedFmtp(pRedCodec->GetFormatSpecificParameter(), pRedFmtp));
    EXPECT_EQ(pRedFmtp->GetRedPayload(), -1);
    EXPECT_EQ(pRedFmtp->GetRedLevel(), 0);
}

TEST_F(TextSdpParserTest, ParseRedSubPtExistWithNullCodec)
{
    TestableTextSdpParser objParser;
    ImsList<SdpMediaFormat*> lstMediaFormats;
    lstMediaFormats.Append(nullptr);  // Add a null codec to the list.

    // The function should handle the null codec gracefully and return false
    // as the subtype is not found.
    EXPECT_FALSE(objParser.ParseRedSubPtExist(100, lstMediaFormats));
}

TEST_F(TextSdpParserTest, ParsePayloadsWithNonRtpFormat)
{
    TestableTextSdpParser objParser;
    TextProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    SdpMediaFormat objMmsrpFormat(SdpMediaFormat::TYPE_MSRP);
    lstMediaFormats.Append(&objMmsrpFormat);

    // Add a valid RTP codec
    auto pT140Codec = std::make_unique<SdpAvCodec>();
    pT140Codec->SetParameters("100 t140/1000", "");
    lstMediaFormats.Append(pT140Codec.get());

    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(lstMediaFormats));

    // This should skip the non-RTP format and only process the T140 codec.
    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 1);
    ASSERT_NE(objProfile.GetPayloadAt(0), nullptr);
    EXPECT_EQ(objProfile.GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), 100);
}

TEST_F(TextSdpParserTest, ParseRedSubPtExistWithNonRtpFormat)
{
    TestableTextSdpParser objParser;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    SdpMediaFormat objMmsrpFormat(SdpMediaFormat::TYPE_MSRP);
    lstMediaFormats.Append(&objMmsrpFormat);

    // Should skip non-RTP format even if payload number matches, and return false
    EXPECT_FALSE(objParser.ParseRedSubPtExist(100, lstMediaFormats));
}
