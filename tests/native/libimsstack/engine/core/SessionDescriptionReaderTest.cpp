/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "SdpParser.h"

#include "SessionDescriptionReader.h"

namespace android
{

class SessionDescriptionReaderTest : public ::testing::Test
{
public:
    inline SessionDescriptionReaderTest()
    {
        AString strSdp("v=0\r\n"
                       "o=SIP-UE 10000 20000 IN IP4 192.168.0.1\r\n"
                       "s=-\r\n"
                       "c=IN IP4 192.168.0.1\r\n"
                       "t=0 0\r\n"
                       "b=AS:49\r\n"
                       "b=test:30\r\n"
                       "a=type:meeting\r\n"
                       "a=test:20\r\n"
                       "a=testname:tc20\r\n"
                       "a=setup:actpass\r\n"
                       "a=connection:new\r\n"
                       "m=audio 50010 RTP/AVP 110\r\n"
                       "b=AS:49\r\n"
                       "b=RS:0\r\n"
                       "b=RR:2500\r\n"
                       "a=rtpmap:110 AMR-WB/16000/1\r\n"
                       "a=fmtp:110 mode-change-capability=2;max-red=220\r\n"
                       "a=sendrecv\r\n"
                       "a=ptime:20\r\n"
                       "a=maxptime:240\r\n"
                       "a=test:20\r\n"
                       "a=testname:tc20\r\n"
                       "a=setup:actpass\r\n"
                       "a=connection:new\r\n");
        m_objSdpParser.Decode(strSdp);
    }

protected:
    void SetUp() override {}
    void TearDown() override {}

protected:
    SdpParser m_objSdpParser;
};

TEST_F(SessionDescriptionReaderTest, Setter)
{
    SessionDescriptionReader objReader(m_objSdpParser.GetSessionDescription());

    EXPECT_EQ(objReader.AddAttribute("test1:30"), IMS_FAILURE);
    EXPECT_EQ(objReader.RemoveAttribute("test:20"), IMS_FAILURE);
    EXPECT_EQ(objReader.SetSessionInfo("Voice call test"), IMS_FAILURE);
    EXPECT_EQ(objReader.SetSessionName("VoLteCall"), IMS_FAILURE);
    EXPECT_EQ(objReader.AddAttribute(SdpAttribute::ATTRIBUTE_OTHER, "30", "test1"), IMS_FAILURE);
    EXPECT_EQ(objReader.AddAttributeInt(SdpAttribute::ATTRIBUTE_OTHER, 30, "test1"), IMS_FAILURE);
    EXPECT_EQ(objReader.AddBandwidth(SdpBandwidth::TYPE_RR, 3000), IMS_FAILURE);

    SdpAttribute objAttr;
    objAttr.SetValue(SdpAttribute::ATTRIBUTE_OTHER, "20", "test");
    EXPECT_EQ(objReader.RemoveAttribute(objAttr), IMS_FAILURE);
    EXPECT_EQ(objReader.RemoveAttribute(SdpAttribute::ATTRIBUTE_OTHER, "20", "test"), IMS_FAILURE);
    EXPECT_EQ(objReader.RemoveAllBandwidths(), IMS_FAILURE);
    EXPECT_EQ(objReader.SetConnectionAddress("192.168.0.2"), IMS_FAILURE);
    EXPECT_EQ(objReader.SetDirection(Sdp::DIRECTION_SENDRECV), IMS_FAILURE);
    EXPECT_EQ(objReader.SetOriginAddress("192.168.0.2"), IMS_FAILURE);
}

TEST_F(SessionDescriptionReaderTest, Getter)
{
    SessionDescriptionReader objReader(m_objSdpParser.GetSessionDescription());
    ImsList<AString> objAttrs = objReader.GetAttributes();

    ASSERT_EQ(objAttrs.GetSize(), 5);
    EXPECT_EQ(objReader.GetProtocolVersion(), AString("0"));
    EXPECT_EQ(objReader.GetSessionId(), AString("10000"));
    EXPECT_EQ(objReader.GetSessionInfo(), AString::ConstNull());
    EXPECT_EQ(objReader.GetSessionName(), AString("-"));
    EXPECT_EQ(objReader.GetAttribute(SdpAttribute::SENDRECV), AString::ConstNull());
    EXPECT_EQ(objReader.GetAttribute(SdpAttribute::TYPE), AString("meeting"));
    EXPECT_EQ(objReader.GetAttribute(SdpAttribute::ATTRIBUTE_OTHER, "test"), AString("20"));
    EXPECT_EQ(objReader.GetBandwidth(SdpBandwidth::TYPE_AS), 49);
    EXPECT_EQ(objReader.GetBandwidth(SdpBandwidth::TYPE_OTHER, "test"), 30);
    EXPECT_EQ(objReader.GetDirection(), Sdp::DIRECTION_NONE);
    EXPECT_EQ(objReader.GetSessionVersion(), AString("20000"));
    EXPECT_EQ(objReader.GetUsername(), AString("SIP-UE"));
    EXPECT_EQ(objReader.GetLocalAddress(), IpAddress::NONE);
    EXPECT_EQ(objReader.GetRemoteAddress(), IpAddress(AString("192.168.0.1")));

    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::ATTRIBUTE_OTHER, "test"), 20);
    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::ATTRIBUTE_OTHER, "testname"),
            ISessionDescriptor::INVALID_VALUE);
    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::SENDRECV), ISessionDescriptor::INVALID_VALUE);
    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::TYPE), ISessionDescriptor::INVALID_VALUE);
    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::SETUP), Sdp::SETUP_ACTPASS);
    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::CONNECTION), Sdp::CONNECTION_NEW);
}

}  // namespace android
