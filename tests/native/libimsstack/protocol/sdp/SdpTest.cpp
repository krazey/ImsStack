/*
 * Copyright (C) 2022 The Android Open Source Project
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

#include "AStringArray.h"

#include "Sdp.h"

namespace android
{

class SdpTest : public ::testing::Test
{
};

TEST_F(SdpTest, IsDigitString)
{
    // All digit string
    AString strTestValue = "1234567890";
    EXPECT_TRUE(Sdp::IsDigitString(strTestValue));

    // Empty string
    EXPECT_FALSE(Sdp::IsDigitString(AString::ConstNull()));
    EXPECT_FALSE(Sdp::IsDigitString(AString::ConstEmpty()));

    // Non-digit string
    strTestValue = "hello";

    EXPECT_FALSE(Sdp::IsDigitString(strTestValue));

    strTestValue = "123hi";
    EXPECT_FALSE(Sdp::IsDigitString(strTestValue));
}

TEST_F(SdpTest, IsFqdnString)
{
    // FQDN = 4 *(alpha-numeric / "-" / ".")

    // Normal FQDN string
    AString strTestValue = "abcde";
    EXPECT_TRUE(Sdp::IsFqdnString(strTestValue));

    strTestValue = "12345";
    EXPECT_TRUE(Sdp::IsFqdnString(strTestValue));

    strTestValue = "abc-123";
    EXPECT_TRUE(Sdp::IsFqdnString(strTestValue));

    strTestValue = "abc.123";
    EXPECT_TRUE(Sdp::IsFqdnString(strTestValue));

    // String that is less than 4 characters
    EXPECT_FALSE(Sdp::IsFqdnString(AString::ConstNull()));
    EXPECT_FALSE(Sdp::IsFqdnString(AString::ConstEmpty()));

    strTestValue = "abc";
    EXPECT_FALSE(Sdp::IsFqdnString(strTestValue));

    strTestValue = "123";
    EXPECT_FALSE(Sdp::IsFqdnString(strTestValue));

    // String that contain the special characters
    strTestValue = "abc.123.!#1";
    EXPECT_FALSE(Sdp::IsFqdnString(strTestValue));

    strTestValue = "abc:123";
    EXPECT_FALSE(Sdp::IsFqdnString(strTestValue));
}

TEST_F(SdpTest, IsTextString)
{
    // Text string
    AString strTestValue = "abc123";
    EXPECT_TRUE(Sdp::IsTextString(strTestValue));

    strTestValue = "abc123!@&^$#*";
    EXPECT_TRUE(Sdp::IsTextString(strTestValue));

    // Empty string
    EXPECT_FALSE(Sdp::IsTextString(AString::ConstNull()));
    EXPECT_FALSE(Sdp::IsTextString(AString::ConstEmpty()));

    // String with non-byte characters (0x00, 0x0A, 0x0D)
    const IMS_CHAR acValue[] = {0x61, 0x62, 0x63, 0x00, 0x31, 0x32, 0x33};

    // 0x00 is included.
    strTestValue = AString(acValue, sizeof(acValue));
    EXPECT_FALSE(Sdp::IsTextString(strTestValue));

    // 0x0A is included.
    strTestValue = "abc123\n";
    EXPECT_FALSE(Sdp::IsTextString(strTestValue));

    // 0x0D is included.
    strTestValue = "abc123\r";
    EXPECT_FALSE(Sdp::IsTextString(strTestValue));
}

TEST_F(SdpTest, IsTokenString)
{
    // Token string
    AString strTestValue = "abcABC123!#$%*+-";
    EXPECT_TRUE(Sdp::IsTokenString(strTestValue));

    strTestValue = "abc 123";
    EXPECT_TRUE(Sdp::IsTokenString(strTestValue, IMS_TRUE));

    // Empty string
    EXPECT_FALSE(Sdp::IsTokenString(AString::ConstNull()));
    EXPECT_FALSE(Sdp::IsTokenString(AString::ConstEmpty()));

    // Non-Token string
    strTestValue = "abc(123)";
    EXPECT_FALSE(Sdp::IsTokenString(strTestValue));

    strTestValue = "abc<123";
    EXPECT_FALSE(Sdp::IsTokenString(strTestValue));

    strTestValue = "abc@123";
    EXPECT_FALSE(Sdp::IsTokenString(strTestValue));

    strTestValue = "abc 123";
    EXPECT_FALSE(Sdp::IsTokenString(strTestValue));

    strTestValue = "abc?123";
    EXPECT_FALSE(Sdp::IsTokenString(strTestValue));

    strTestValue = "abc=123";
    EXPECT_FALSE(Sdp::IsTokenString(strTestValue));
}

TEST_F(SdpTest, IsTypedTimeString)
{
    AString strTestValue = "12";
    EXPECT_TRUE(Sdp::IsTypedTimeString(strTestValue));

    strTestValue = "12d";
    EXPECT_TRUE(Sdp::IsTypedTimeString(strTestValue));

    strTestValue = "12h";
    EXPECT_TRUE(Sdp::IsTypedTimeString(strTestValue));

    strTestValue = "12m";
    EXPECT_TRUE(Sdp::IsTypedTimeString(strTestValue));

    strTestValue = "12s";
    EXPECT_TRUE(Sdp::IsTypedTimeString(strTestValue));

    // Empty string
    EXPECT_FALSE(Sdp::IsTypedTimeString(AString::ConstNull()));
    EXPECT_FALSE(Sdp::IsTypedTimeString(AString::ConstEmpty()));

    strTestValue = "12A";
    EXPECT_FALSE(Sdp::IsTypedTimeString(strTestValue));

    strTestValue = "12D";
    EXPECT_FALSE(Sdp::IsTypedTimeString(strTestValue));

    strTestValue = "12H";
    EXPECT_FALSE(Sdp::IsTypedTimeString(strTestValue));
}

TEST_F(SdpTest, IsNonWsString)
{
    // 0x21 ~ 0x7E, 0x80 ~ 0xFF

    AString strTestValue = "abc123";
    EXPECT_TRUE(Sdp::IsNonWsString(strTestValue));

    strTestValue = "abcABC123^#!@?+-*$%";
    EXPECT_TRUE(Sdp::IsNonWsString(strTestValue));

    // Empty string
    EXPECT_FALSE(Sdp::IsNonWsString(AString::ConstNull()));
    EXPECT_FALSE(Sdp::IsNonWsString(AString::ConstEmpty()));

    strTestValue = "abc 123";
    EXPECT_FALSE(Sdp::IsNonWsString(strTestValue));

    strTestValue = "abc\r123";
    EXPECT_FALSE(Sdp::IsNonWsString(strTestValue));

    strTestValue = "abc\n123";
    EXPECT_FALSE(Sdp::IsNonWsString(strTestValue));
}

TEST_F(SdpTest, IsUriString)
{
    // TODO: add the unit tests when the implementation is ready.
    EXPECT_TRUE(Sdp::IsUriString("sip:ims-user@ims.com"));
}

TEST_F(SdpTest, SplitLine)
{
    AString strTestValue("audio 49152 RTP/AVP 96 97");
    AStringArray objTokens;
    IMS_SINT32 nSplitCount;

    nSplitCount = 5;
    ASSERT_TRUE(Sdp::SplitLine(strTestValue, nSplitCount, objTokens));
    ASSERT_EQ(objTokens.GetCount(), nSplitCount);
    EXPECT_STREQ(objTokens.GetElementAt(0).GetStr(), "audio");
    EXPECT_STREQ(objTokens.GetElementAt(1).GetStr(), "49152");
    EXPECT_STREQ(objTokens.GetElementAt(2).GetStr(), "RTP/AVP");
    EXPECT_STREQ(objTokens.GetElementAt(3).GetStr(), "96");
    EXPECT_STREQ(objTokens.GetElementAt(4).GetStr(), "97");

    objTokens = AStringArray::ConstNull();
    nSplitCount = 4;
    ASSERT_TRUE(Sdp::SplitLine(strTestValue, nSplitCount, objTokens));
    ASSERT_EQ(objTokens.GetCount(), nSplitCount);
    EXPECT_STREQ(objTokens.GetElementAt(0).GetStr(), "audio");
    EXPECT_STREQ(objTokens.GetElementAt(1).GetStr(), "49152");
    EXPECT_STREQ(objTokens.GetElementAt(2).GetStr(), "RTP/AVP");
    EXPECT_STREQ(objTokens.GetElementAt(3).GetStr(), "96 97");

    objTokens = AStringArray::ConstNull();
    nSplitCount = 1;
    ASSERT_TRUE(Sdp::SplitLine(strTestValue, nSplitCount, objTokens));
    ASSERT_EQ(objTokens.GetCount(), nSplitCount);
    EXPECT_EQ(objTokens.GetElementAt(0), strTestValue);

    objTokens = AStringArray::ConstNull();
    ASSERT_TRUE(Sdp::SplitLine(strTestValue, 0, objTokens));
    ASSERT_EQ(objTokens.GetCount(), 1);
    EXPECT_EQ(objTokens.GetElementAt(0), strTestValue);

    objTokens = AStringArray::ConstNull();
    nSplitCount = 1;
    ASSERT_TRUE(Sdp::SplitLine(AString::ConstNull(), nSplitCount, objTokens));
    ASSERT_EQ(objTokens.GetCount(), nSplitCount);
    EXPECT_EQ(objTokens.GetElementAt(0), AString::ConstNull());

    objTokens = AStringArray::ConstNull();
    nSplitCount = 1;
    ASSERT_TRUE(Sdp::SplitLine(AString::ConstEmpty(), nSplitCount, objTokens));
    ASSERT_EQ(objTokens.GetCount(), nSplitCount);
    EXPECT_EQ(objTokens.GetElementAt(0), AString::ConstEmpty());

    objTokens = AStringArray::ConstNull();
    EXPECT_FALSE(Sdp::SplitLine(strTestValue, 6, objTokens));

    EXPECT_FALSE(Sdp::SplitLine(AString::ConstNull(), 2, objTokens));
    EXPECT_FALSE(Sdp::SplitLine(AString::ConstEmpty(), 2, objTokens));
}

TEST_F(SdpTest, ConvertTypedTimeToSeconds)
{
    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds(AString::ConstNull()), 0);
    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds(AString::ConstEmpty()), 0);

    IMS_UINT32 nSecondsForOneDay = 86400;

    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds("1d"), nSecondsForOneDay * 1);
    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds("5d"), nSecondsForOneDay * 5);

    IMS_UINT32 nSecondsForOneHour = 3600;

    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds("1h"), nSecondsForOneHour * 1);
    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds("5h"), nSecondsForOneHour * 5);

    IMS_UINT32 nSecondsForOneMinute = 60;

    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds("1m"), nSecondsForOneMinute * 1);
    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds("5m"), nSecondsForOneMinute * 5);

    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds("1s"), 1);
    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds("5s"), 5);

    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds("1"), 1);
    EXPECT_EQ(Sdp::ConvertTypedTimeToSeconds("5"), 5);
}

TEST_F(SdpTest, GetPayloadTypeFromAttribute)
{
    AString strTestValue = "96 AMR/8000/1";
    EXPECT_EQ(Sdp::GetPayloadTypeFromAttribute(strTestValue), 96);

    // No space
    strTestValue = "96";
    EXPECT_EQ(Sdp::GetPayloadTypeFromAttribute(strTestValue), -1);

    // Non-integer value
    strTestValue = "abc AMR/8000/1";
    EXPECT_EQ(Sdp::GetPayloadTypeFromAttribute(strTestValue), -1);

    EXPECT_EQ(Sdp::GetPayloadTypeFromAttribute(AString::ConstNull()), -1);
    EXPECT_EQ(Sdp::GetPayloadTypeFromAttribute(AString::ConstEmpty()), -1);
}

TEST_F(SdpTest, IncreaseSessionVersion)
{
    EXPECT_EQ(Sdp::IncreaseSessionVersion(AString::ConstNull()), AString::ConstNull());
    EXPECT_EQ(Sdp::IncreaseSessionVersion(AString::ConstEmpty()), AString::ConstEmpty());

    AString strTestValue("1");
    AString strExpected("2");
    EXPECT_EQ(Sdp::IncreaseSessionVersion(strTestValue), strExpected);

    strTestValue = "9";
    strExpected = "10";
    EXPECT_EQ(Sdp::IncreaseSessionVersion(strTestValue), strExpected);

    strTestValue = "99";
    strExpected = "100";
    EXPECT_EQ(Sdp::IncreaseSessionVersion(strTestValue), strExpected);

    strTestValue = "238299";
    strExpected = "238300";
    EXPECT_EQ(Sdp::IncreaseSessionVersion(strTestValue), strExpected);
}

TEST_F(SdpTest, ParseAttributeRtpmap)
{
    AString strTestValue = "100 AMR-WB/16000/1";
    IMS_SINT32 nPayloadType = -1;
    AString strEncodingName;
    IMS_UINT32 nClockRate = 0;
    AString strEncodingParameters;

    ASSERT_TRUE(Sdp::ParseAttributeRtpmap(
            strTestValue, nPayloadType, strEncodingName, nClockRate, strEncodingParameters));
    EXPECT_EQ(nPayloadType, 100);
    EXPECT_STREQ(strEncodingName.GetStr(), "AMR-WB");
    EXPECT_EQ(nClockRate, 16000);
    EXPECT_STREQ(strEncodingParameters.GetStr(), "1");

    strTestValue = "100 AMR-WB/16000";
    nPayloadType = -1;
    strEncodingName = AString::ConstNull();
    nClockRate = 0;
    strEncodingParameters = AString::ConstNull();

    ASSERT_TRUE(Sdp::ParseAttributeRtpmap(
            strTestValue, nPayloadType, strEncodingName, nClockRate, strEncodingParameters));
    EXPECT_EQ(nPayloadType, 100);
    EXPECT_STREQ(strEncodingName.GetStr(), "AMR-WB");
    EXPECT_EQ(nClockRate, 16000);
    EXPECT_EQ(strEncodingParameters, AString::ConstNull());

    nPayloadType = -1;
    strEncodingName = AString::ConstNull();
    nClockRate = 0;
    strEncodingParameters = AString::ConstNull();

    strTestValue = "100AMR-WB/16000/1";
    EXPECT_FALSE(Sdp::ParseAttributeRtpmap(
            strTestValue, nPayloadType, strEncodingName, nClockRate, strEncodingParameters));

    strTestValue = "100 AMR-WB";
    EXPECT_FALSE(Sdp::ParseAttributeRtpmap(
            strTestValue, nPayloadType, strEncodingName, nClockRate, strEncodingParameters));

    EXPECT_FALSE(Sdp::ParseAttributeRtpmap(AString::ConstNull(), nPayloadType, strEncodingName,
            nClockRate, strEncodingParameters));
    EXPECT_FALSE(Sdp::ParseAttributeRtpmap(AString::ConstEmpty(), nPayloadType, strEncodingName,
            nClockRate, strEncodingParameters));
}

TEST_F(SdpTest, ParseAttributeFmtp)
{
    AString strTestValue = "100 octet-align=1;max-red=220";
    IMS_SINT32 nPayloadType = -1;
    AString strParameters;

    ASSERT_TRUE(Sdp::ParseAttributeFmtp(strTestValue, nPayloadType, strParameters));
    EXPECT_EQ(nPayloadType, 100);
    EXPECT_STREQ(strParameters.GetStr(), "octet-align=1;max-red=220");

    strTestValue = "100 ";
    nPayloadType = -1;
    strParameters = AString::ConstNull();
    ASSERT_TRUE(Sdp::ParseAttributeFmtp(strTestValue, nPayloadType, strParameters));
    EXPECT_EQ(nPayloadType, 100);
    EXPECT_EQ(strParameters, AString::ConstEmpty());

    nPayloadType = -1;
    strParameters = AString::ConstNull();

    strTestValue = "100";
    ASSERT_TRUE(Sdp::ParseAttributeFmtp(strTestValue, nPayloadType, strParameters));
    EXPECT_EQ(nPayloadType, 100);
    EXPECT_EQ(strParameters, AString::ConstNull());

    strTestValue = "100a octet-align=1;max-red=220";
    EXPECT_FALSE(Sdp::ParseAttributeFmtp(strTestValue, nPayloadType, strParameters));

    EXPECT_FALSE(Sdp::ParseAttributeFmtp(AString::ConstNull(), nPayloadType, strParameters));
    EXPECT_FALSE(Sdp::ParseAttributeFmtp(AString::ConstEmpty(), nPayloadType, strParameters));
}

TEST_F(SdpTest, ParseAttributeRtcp)
{
    // a=rtcp:53020
    AString strTestValue = "53020";
    IMS_SINT32 nPort = -1;

    ASSERT_TRUE(Sdp::ParseAttributeRtcp(strTestValue, nPort));
    EXPECT_EQ(nPort, 53020);

    // a=rtcp:53020 IN IP4 126.16.64.4
    strTestValue = "53020 IN IP4 126.16.64.4";
    nPort = -1;
    ASSERT_TRUE(Sdp::ParseAttributeRtcp(strTestValue, nPort));
    EXPECT_EQ(nPort, 53020);

    // a=rtcp:53020 IN IP6 2001:2345:6789:ABCD:EF01:2345:6789:ABCD
    strTestValue = "53020 IN IP6 2001:2345:6789:ABCD:EF01:2345:6789:ABCD";
    nPort = -1;
    ASSERT_TRUE(Sdp::ParseAttributeRtcp(strTestValue, nPort));
    EXPECT_EQ(nPort, 53020);

    nPort = -1;

    EXPECT_FALSE(Sdp::ParseAttributeRtcp(AString::ConstNull(), nPort));
    EXPECT_FALSE(Sdp::ParseAttributeRtcp(AString::ConstEmpty(), nPort));

    // a=rtcp:5302a
    strTestValue = "5302a";
    EXPECT_FALSE(Sdp::ParseAttributeRtcp(strTestValue, nPort));

    // a=rtcp:5302b IN IP4 126.16.64.4
    strTestValue = "5302b IN IP4 126.16.64.4";
    EXPECT_FALSE(Sdp::ParseAttributeRtcp(strTestValue, nPort));
}

TEST_F(SdpTest, ParseAttributeSetup)
{
    // a=setup:<type>
    IMS_SINT32 nTypeOfSetup = Sdp::SETUP_NONE;

    for (IMS_SINT32 i = 0; i < Sdp::SETUP_MAX; ++i)
    {
        Sdp::ParseAttributeSetup(Sdp::STR_A_SETUP[i], nTypeOfSetup);
        EXPECT_EQ(nTypeOfSetup, i);
    }

    AString strTestValue = "tcpconn";
    nTypeOfSetup = Sdp::SETUP_MAX;
    Sdp::ParseAttributeSetup(strTestValue, nTypeOfSetup);
    EXPECT_EQ(nTypeOfSetup, Sdp::SETUP_NONE);

    nTypeOfSetup = Sdp::SETUP_MAX;
    Sdp::ParseAttributeSetup(AString::ConstNull(), nTypeOfSetup);
    EXPECT_EQ(nTypeOfSetup, Sdp::SETUP_NONE);

    nTypeOfSetup = Sdp::SETUP_MAX;
    Sdp::ParseAttributeSetup(AString::ConstEmpty(), nTypeOfSetup);
    EXPECT_EQ(nTypeOfSetup, Sdp::SETUP_NONE);
}

TEST_F(SdpTest, ParseAttributeConnection)
{
    // a=connection:<type>
    IMS_SINT32 nTypeOfConnection = Sdp::CONNECTION_NONE;

    for (IMS_SINT32 i = 0; i < Sdp::CONNECTION_MAX; ++i)
    {
        Sdp::ParseAttributeConnection(Sdp::STR_A_CONNECTION[i], nTypeOfConnection);
        EXPECT_EQ(nTypeOfConnection, i);
    }

    AString strTestValue = "tcpconn";
    nTypeOfConnection = Sdp::CONNECTION_MAX;
    Sdp::ParseAttributeConnection(strTestValue, nTypeOfConnection);
    EXPECT_EQ(nTypeOfConnection, Sdp::CONNECTION_NONE);

    nTypeOfConnection = Sdp::CONNECTION_MAX;
    Sdp::ParseAttributeConnection(AString::ConstNull(), nTypeOfConnection);
    EXPECT_EQ(nTypeOfConnection, Sdp::CONNECTION_NONE);

    nTypeOfConnection = Sdp::CONNECTION_MAX;
    Sdp::ParseAttributeConnection(AString::ConstEmpty(), nTypeOfConnection);
    EXPECT_EQ(nTypeOfConnection, Sdp::CONNECTION_NONE);
}

TEST_F(SdpTest, ParseAttributeFramesize)
{
    // a=framesize:<payload type>SP<width>-<height>
    AString strTestValue = "101 320-240";
    IMS_SINT32 nPayloadType = -1;
    IMS_SINT32 nWidth = -1;
    IMS_SINT32 nHeight = -1;

    ASSERT_TRUE(Sdp::ParseAttributeFramesize(strTestValue, nPayloadType, nWidth, nHeight));
    EXPECT_EQ(nPayloadType, 101);
    EXPECT_EQ(nWidth, 320);
    EXPECT_EQ(nHeight, 240);

    nPayloadType = -1;
    nWidth = -1;
    nHeight = -1;

    strTestValue = "101";
    EXPECT_FALSE(Sdp::ParseAttributeFramesize(strTestValue, nPayloadType, nWidth, nHeight));

    strTestValue = "101 ";
    EXPECT_FALSE(Sdp::ParseAttributeFramesize(strTestValue, nPayloadType, nWidth, nHeight));

    strTestValue = "101 320:240";
    EXPECT_FALSE(Sdp::ParseAttributeFramesize(strTestValue, nPayloadType, nWidth, nHeight));

    strTestValue = "10a 320-240";
    EXPECT_FALSE(Sdp::ParseAttributeFramesize(strTestValue, nPayloadType, nWidth, nHeight));

    strTestValue = "101 32a-240";
    EXPECT_FALSE(Sdp::ParseAttributeFramesize(strTestValue, nPayloadType, nWidth, nHeight));

    strTestValue = "101 320-24a";
    EXPECT_FALSE(Sdp::ParseAttributeFramesize(strTestValue, nPayloadType, nWidth, nHeight));
}

}  // namespace android
