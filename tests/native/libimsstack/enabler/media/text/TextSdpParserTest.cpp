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

#include "text/TextSdpParser.h"
#include "text/TextProfile.h"
#include "offeranswer/SdpAvCodec.h"

#include "media/MockIMediaDescriptor.h"

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
    MockIMediaDescriptor m_objMockMediaDescriptor;
};

class TestableTextSdpParser : public TextSdpParser
{
public:
    using TextSdpParser::ParsePayloads;
};

TEST_F(TextSdpParserTest, ParsePayloads_ValidT140AndRed)
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
    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect 2 payloads to be added
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 2);

    // Verify t140 payload
    TextProfile::Payload* pT140Payload = objProfile.GetPayloadAt(0);
    ASSERT_NE(pT140Payload, nullptr);
    EXPECT_EQ(pT140Payload->GetRtpMap().GetPayloadNumber(), 100);
    EXPECT_TRUE(pT140Payload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("t140"));
    auto pT140Fmtp = std::static_pointer_cast<TextProfile::T140Fmtp>(pT140Payload->GetFmtp());
    ASSERT_NE(pT140Fmtp, nullptr);
    EXPECT_EQ(pT140Fmtp->GetCps(), 30);

    // Verify red payload
    TextProfile::Payload* pRedPayload = objProfile.GetPayloadAt(1);
    ASSERT_NE(pRedPayload, nullptr);
    EXPECT_EQ(pRedPayload->GetRtpMap().GetPayloadNumber(), 101);
    EXPECT_TRUE(pRedPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("red"));
    auto pRedFmtp = std::static_pointer_cast<TextProfile::RedFmtp>(pRedPayload->GetFmtp());
    ASSERT_NE(pRedFmtp, nullptr);
    EXPECT_EQ(pRedFmtp->GetRedPayload(), 100);
    EXPECT_EQ(pRedFmtp->GetRedLevel(), 2);
}

TEST_F(TextSdpParserTest, ParsePayloads_InvalidPayloadType)
{
    TestableTextSdpParser objParser;
    TextProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock a codec with a static payload type, which should be rejected
    auto pStaticCodec = std::make_unique<SdpAvCodec>();
    pStaticCodec->SetParameters("8 PCMA/8000", "");
    lstMediaFormats.Append(pStaticCodec.get());

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect no payloads to be added
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 0);
}

TEST_F(TextSdpParserTest, ParsePayloads_InvalidCodecName)
{
    TestableTextSdpParser objParser;
    TextProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock a codec with an unsupported name
    auto pInvalidCodec = std::make_unique<SdpAvCodec>();
    pInvalidCodec->SetParameters("98 unsupported/1000", "");
    lstMediaFormats.Append(pInvalidCodec.get());

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect no payloads to be added
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 0);
}

TEST_F(TextSdpParserTest, ParsePayloads_RedFmtpWithoutMatchingSubtype)
{
    TestableTextSdpParser objParser;
    TextProfile objProfile;
    ImsList<SdpMediaFormat*> lstMediaFormats;

    // Mock a 'red' codec whose subtype (105) is not in the media format list
    auto pRedCodec = std::make_unique<SdpAvCodec>();
    pRedCodec->SetParameters("101 red/1000", "101 105/105");
    lstMediaFormats.Append(pRedCodec.get());

    const ImsList<SdpMediaFormat*>& constListMediaFormats = lstMediaFormats;
    ON_CALL(m_objMockMediaDescriptor, GetMediaFormats())
            .WillByDefault(ReturnRef(constListMediaFormats));

    objParser.ParsePayloads(&m_objMockMediaDescriptor, &objProfile);

    // Expect no payloads to be added because the 'red' subtype is missing
    EXPECT_EQ(objProfile.GetPayloadList().GetSize(), 0);
}
