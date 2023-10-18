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

#include "SdpReader.h"

namespace android
{

class SdpReaderTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(SdpReaderTest, CreateWithEmptySdp)
{
    SdpReader objSdpReader(ByteArray::ConstNull());

    EXPECT_EQ(objSdpReader.GetSessionDescriptor(), nullptr);
    EXPECT_TRUE(objSdpReader.GetMediaDescriptors().IsEmpty());
}

TEST_F(SdpReaderTest, CreateWithNonEmptySdp)
{
    AString strSdp("v=0\r\n"
                   "o=SIP-UE 10000 10000 IN IP4 192.168.0.1\r\n"
                   "s=-\r\n"
                   "c=IN IP4 192.168.0.1\r\n"
                   "t=0 0\r\n"
                   "m=audio 50010 RTP/AVP 110\r\n"
                   "b=AS:49\r\n"
                   "b=RS:0\r\n"
                   "b=RR:2500\r\n"
                   "a=rtpmap:110 AMR-WB/16000/1\r\n"
                   "a=fmtp:110 mode-change-capability=2;max-red=220\r\n"
                   "a=sendrecv\r\n"
                   "a=ptime:20\r\n"
                   "a=maxptime:240\r\n");
    ByteArray objSdp(strSdp);
    SdpReader objSdpReader(objSdp);

    EXPECT_NE(objSdpReader.GetSessionDescriptor(), nullptr);
    EXPECT_EQ(objSdpReader.GetMediaDescriptors().GetSize(), 1);
}

}  // namespace android
