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

#include "AString.h"
#include "MediaDef.h"

#include "MediaBaseProfile.h"

const IMS_UINT32 PAYLOAD_NUMBER = 100;
const AString PAYLOAD_TYPE = "EVS";
const IMS_UINT32 SAMPLEING_RATE = 16000;
const IMS_SINT32 CHANNEL = 1;
const IpAddress IP_ADDRESS = IpAddress(AString("255.255.255.255"));
const IMS_UINT32 DATA_PORT = 1234;
const IMS_UINT32 CONTROL_PORT = DATA_PORT + 1;
const AString TRANSPORT_TYPE = "RTP/AVPF";
const IMS_UINT32 RTCP_INTERVAL = 5000;
const IMS_SINT32 AS = 2000;
const IMS_SINT32 RS = 100;
const IMS_SINT32 RR = 200;
const MEDIA_DIRECTION DIRECTION = MEDIA_DIRECTION_SEND_RECEIVE;
const IMS_SINT32 NEGOTIATED_PAYLOAD_INDEX = 0;

class MediaBaseProfileTest : public ::testing::Test
{
protected:
    void InitializeProfile(OUT MediaBaseProfile* pProfile)
    {
        if (pProfile != IMS_NULL)
        {
            pProfile->SetIpAddress(IP_ADDRESS);
            pProfile->SetDataPort(DATA_PORT);
            pProfile->SetControlPort(CONTROL_PORT);
            pProfile->SetTransportType(TRANSPORT_TYPE);
            pProfile->SetRtcpInterval(RTCP_INTERVAL);
            pProfile->SetBandwidthAs(AS);
            pProfile->SetBandwidthRs(RS);
            pProfile->SetBandwidthRr(RR);
            pProfile->SetDirection(DIRECTION);
        }
    };
};

TEST_F(MediaBaseProfileTest, TestRtpMapPayloadNumber)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    EXPECT_EQ(rtpMap->GetPayloadNumber(), 0);

    rtpMap->SetPayloadNumber(PAYLOAD_NUMBER);
    EXPECT_EQ(rtpMap->GetPayloadNumber(), PAYLOAD_NUMBER);

    delete rtpMap;
}

TEST_F(MediaBaseProfileTest, TestRtpMapPayloadType)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    EXPECT_EQ(rtpMap->GetPayloadType(), AString::ConstNull());

    rtpMap->SetPayloadType(PAYLOAD_TYPE);
    EXPECT_EQ(rtpMap->GetPayloadType(), PAYLOAD_TYPE);

    delete rtpMap;
}

TEST_F(MediaBaseProfileTest, TestRtpSamplingRate)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    EXPECT_EQ(rtpMap->GetSamplingRate(), 0);

    rtpMap->SetSamplingRate(SAMPLEING_RATE);
    EXPECT_EQ(rtpMap->GetSamplingRate(), SAMPLEING_RATE);

    delete rtpMap;
}

TEST_F(MediaBaseProfileTest, TestRtpChannel)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    EXPECT_EQ(rtpMap->GetChannel(), 0);

    rtpMap->SetChannel(CHANNEL);
    EXPECT_EQ(rtpMap->GetChannel(), CHANNEL);

    delete rtpMap;
}

TEST_F(MediaBaseProfileTest, TestRtpMapCreation)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    EXPECT_EQ(rtpMap->GetPayloadNumber(), 0);
    EXPECT_EQ(rtpMap->GetPayloadType(), AString::ConstNull());
    EXPECT_EQ(rtpMap->GetSamplingRate(), 0);
    EXPECT_EQ(rtpMap->GetChannel(), 0);

    MediaBaseProfile::RtpMap* rtpMap1 = new MediaBaseProfile::RtpMap(CHANNEL);
    EXPECT_EQ(rtpMap1->GetPayloadNumber(), 0);
    EXPECT_EQ(rtpMap1->GetPayloadType(), AString::ConstNull());
    EXPECT_EQ(rtpMap1->GetSamplingRate(), 0);
    EXPECT_EQ(rtpMap1->GetChannel(), CHANNEL);

    rtpMap1->SetPayloadNumber(PAYLOAD_NUMBER);
    rtpMap1->SetPayloadType(PAYLOAD_TYPE);
    rtpMap1->SetSamplingRate(SAMPLEING_RATE);

    MediaBaseProfile::RtpMap* rtpMap2 = new MediaBaseProfile::RtpMap(*rtpMap1);
    EXPECT_EQ(rtpMap2->GetPayloadNumber(), PAYLOAD_NUMBER);
    EXPECT_EQ(rtpMap2->GetPayloadType(), PAYLOAD_TYPE);
    EXPECT_EQ(rtpMap2->GetSamplingRate(), SAMPLEING_RATE);
    EXPECT_EQ(rtpMap2->GetChannel(), CHANNEL);

    delete rtpMap;
    delete rtpMap1;
    delete rtpMap2;
}

TEST_F(MediaBaseProfileTest, TestRtpMapAssign)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    MediaBaseProfile::RtpMap* rtpMap1 = new MediaBaseProfile::RtpMap(CHANNEL);
    rtpMap1->SetPayloadNumber(PAYLOAD_NUMBER);
    rtpMap1->SetPayloadType(PAYLOAD_TYPE);
    rtpMap1->SetSamplingRate(SAMPLEING_RATE);

    EXPECT_EQ(rtpMap->GetPayloadNumber(), 0);
    EXPECT_EQ(rtpMap->GetPayloadType(), AString::ConstNull());
    EXPECT_EQ(rtpMap->GetSamplingRate(), 0);
    EXPECT_EQ(rtpMap->GetChannel(), 0);

    *rtpMap = *rtpMap1;

    EXPECT_EQ(rtpMap->GetPayloadNumber(), PAYLOAD_NUMBER);
    EXPECT_EQ(rtpMap->GetPayloadType(), PAYLOAD_TYPE);
    EXPECT_EQ(rtpMap->GetSamplingRate(), SAMPLEING_RATE);
    EXPECT_EQ(rtpMap->GetChannel(), CHANNEL);

    delete rtpMap;
    delete rtpMap1;
}

TEST_F(MediaBaseProfileTest, TestRtpMapAssignEqual)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    MediaBaseProfile::RtpMap* rtpMap1 = new MediaBaseProfile::RtpMap(CHANNEL);
    rtpMap1->SetPayloadNumber(PAYLOAD_NUMBER);
    rtpMap1->SetPayloadType(PAYLOAD_TYPE);
    rtpMap1->SetSamplingRate(SAMPLEING_RATE);

    EXPECT_NE(*rtpMap, *rtpMap1);

    *rtpMap = *rtpMap1;
    EXPECT_EQ(*rtpMap, *rtpMap1);

    rtpMap1->SetSamplingRate(SAMPLEING_RATE + 1);
    EXPECT_NE(*rtpMap, *rtpMap1);

    delete rtpMap;
    delete rtpMap1;
}

TEST_F(MediaBaseProfileTest, TestRtpMapEqual)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    MediaBaseProfile::RtpMap* rtpMap1 = new MediaBaseProfile::RtpMap(CHANNEL);
    rtpMap1->SetPayloadNumber(PAYLOAD_NUMBER);
    rtpMap1->SetPayloadType(PAYLOAD_TYPE);
    rtpMap1->SetSamplingRate(SAMPLEING_RATE);

    EXPECT_NE(*rtpMap, *rtpMap1);

    rtpMap->SetPayloadNumber(PAYLOAD_NUMBER);
    EXPECT_NE(*rtpMap, *rtpMap1);

    rtpMap->SetPayloadType(PAYLOAD_TYPE);
    EXPECT_NE(*rtpMap, *rtpMap1);

    rtpMap->SetSamplingRate(SAMPLEING_RATE);
    EXPECT_NE(*rtpMap, *rtpMap1);

    rtpMap->SetChannel(CHANNEL);
    EXPECT_EQ(*rtpMap, *rtpMap1);

    rtpMap->SetPayloadNumber(PAYLOAD_NUMBER + 1);
    EXPECT_NE(*rtpMap, *rtpMap1);

    delete rtpMap;
    delete rtpMap1;
}

TEST_F(MediaBaseProfileTest, TestBasePayloadCreation)
{
    MediaBaseProfile::BasePayload* basePayload = new MediaBaseProfile::BasePayload();
    MediaBaseProfile::RtpMap tempRtpMap;

    // Compare default values
    EXPECT_EQ(basePayload->GetRtpMap(), tempRtpMap);

    MediaBaseProfile::BasePayload* basePayload1 = new MediaBaseProfile::BasePayload(CHANNEL);
    EXPECT_NE(basePayload1->GetRtpMap(), tempRtpMap);

    tempRtpMap.SetChannel(CHANNEL);
    EXPECT_EQ(basePayload1->GetRtpMap(), tempRtpMap);

    basePayload1->GetRtpMap().SetPayloadNumber(PAYLOAD_NUMBER);
    basePayload1->GetRtpMap().SetPayloadType(PAYLOAD_TYPE);
    basePayload1->GetRtpMap().SetSamplingRate(SAMPLEING_RATE);

    MediaBaseProfile::BasePayload* basePayload2 = new MediaBaseProfile::BasePayload(*basePayload1);
    EXPECT_EQ(basePayload2->GetRtpMap(), basePayload1->GetRtpMap());

    delete basePayload;
    delete basePayload1;
    delete basePayload2;
}

TEST_F(MediaBaseProfileTest, TestBasePayloadAssign)
{
    MediaBaseProfile::BasePayload* basePayload = new MediaBaseProfile::BasePayload(CHANNEL);
    basePayload->GetRtpMap().SetPayloadNumber(PAYLOAD_NUMBER);
    basePayload->GetRtpMap().SetPayloadType(PAYLOAD_TYPE);
    basePayload->GetRtpMap().SetSamplingRate(SAMPLEING_RATE);

    MediaBaseProfile::BasePayload* basePayload1 = new MediaBaseProfile::BasePayload();

    EXPECT_NE(basePayload->GetRtpMap(), basePayload1->GetRtpMap());

    *basePayload1 = *basePayload;

    EXPECT_EQ(basePayload1->GetRtpMap(), basePayload->GetRtpMap());

    delete basePayload;
    delete basePayload1;
}

TEST_F(MediaBaseProfileTest, TestBasePayloadRtpMap)
{
    MediaBaseProfile::BasePayload* basePayload = new MediaBaseProfile::BasePayload();

    basePayload->SetRtpMap(PAYLOAD_NUMBER, PAYLOAD_TYPE, SAMPLEING_RATE);
    EXPECT_EQ(basePayload->GetRtpMap().GetPayloadNumber(), PAYLOAD_NUMBER);
    EXPECT_EQ(basePayload->GetRtpMap().GetPayloadType(), PAYLOAD_TYPE);
    EXPECT_EQ(basePayload->GetRtpMap().GetSamplingRate(), SAMPLEING_RATE);
    EXPECT_EQ(basePayload->GetRtpMap().GetChannel(), 0);

    basePayload->SetRtpMap(PAYLOAD_NUMBER, PAYLOAD_TYPE, SAMPLEING_RATE, CHANNEL);
    EXPECT_EQ(basePayload->GetRtpMap().GetPayloadNumber(), PAYLOAD_NUMBER);
    EXPECT_EQ(basePayload->GetRtpMap().GetPayloadType(), PAYLOAD_TYPE);
    EXPECT_EQ(basePayload->GetRtpMap().GetSamplingRate(), SAMPLEING_RATE);
    EXPECT_EQ(basePayload->GetRtpMap().GetChannel(), CHANNEL);

    MediaBaseProfile::RtpMap tempRtpMap;

    basePayload->SetRtpMap(tempRtpMap);
    EXPECT_EQ(basePayload->GetRtpMap().GetPayloadNumber(), 0);
    EXPECT_EQ(basePayload->GetRtpMap().GetPayloadType(), AString::ConstNull());
    EXPECT_EQ(basePayload->GetRtpMap().GetSamplingRate(), 0);
    EXPECT_EQ(basePayload->GetRtpMap().GetChannel(), 0);

    tempRtpMap.SetPayloadNumber(PAYLOAD_NUMBER);
    tempRtpMap.SetPayloadType(PAYLOAD_TYPE);
    tempRtpMap.SetSamplingRate(SAMPLEING_RATE);
    tempRtpMap.SetChannel(CHANNEL);

    basePayload->SetRtpMap(tempRtpMap);

    EXPECT_EQ(basePayload->GetRtpMap().GetPayloadNumber(), PAYLOAD_NUMBER);
    EXPECT_EQ(basePayload->GetRtpMap().GetPayloadType(), PAYLOAD_TYPE);
    EXPECT_EQ(basePayload->GetRtpMap().GetSamplingRate(), SAMPLEING_RATE);
    EXPECT_EQ(basePayload->GetRtpMap().GetChannel(), CHANNEL);

    delete basePayload;
}

TEST_F(MediaBaseProfileTest, TestCapaNegoCreation)
{
    MediaBaseProfile::CapaNego* capaNego = new MediaBaseProfile::CapaNego();

    EXPECT_EQ(capaNego->GetMapTcap().GetSize(), 0);
    EXPECT_EQ(capaNego->GetListPcfg().GetSize(), 0);
    EXPECT_EQ(capaNego->GetMapAcap().GetSize(), 0);
    EXPECT_EQ(capaNego->GetAcfg(), AString::ConstNull());
    EXPECT_EQ(capaNego->IsAttCapaInPcfg(), IMS_FALSE);

    delete capaNego;
}

TEST_F(MediaBaseProfileTest, TestCapaNegoTcap)
{
    MediaBaseProfile::CapaNego* capaNego = new MediaBaseProfile::CapaNego();

    EXPECT_EQ(capaNego->GetMapTcap().GetSize(), 0);

    AString tempStr1 = "RTP/AVP";
    AString tempStr2 = "RTP/AVPF";

    capaNego->GetMapTcap().SetValue(1, tempStr1);
    EXPECT_EQ(capaNego->GetMapTcap().GetSize(), 1);
    EXPECT_EQ(capaNego->GetMapTcap().GetValue(1), tempStr1);

    capaNego->GetMapTcap().SetValue(2, tempStr2);
    EXPECT_EQ(capaNego->GetMapTcap().GetSize(), 2);
    EXPECT_EQ(capaNego->GetMapTcap().GetValue(2), tempStr2);

    delete capaNego;
}

TEST_F(MediaBaseProfileTest, TestCapaNegoAcap)
{
    MediaBaseProfile::CapaNego* capaNego = new MediaBaseProfile::CapaNego();

    AString tempStr1 = "rtcp-fb:* trr-int 2000";
    AString tempStr2 = "rtcp-fb:* nack";
    AString tempStr3 = "rtcp-fb:* nack pli";
    AString tempStr4 = "rtcp-fb:* ccm fir";
    AString tempStr5 = "rtcp-fb:* ccm tmmbr";

    EXPECT_EQ(capaNego->GetMapAcap().GetSize(), 0);

    capaNego->GetMapAcap().SetValue(1, tempStr1);
    EXPECT_EQ(capaNego->GetMapAcap().GetSize(), 1);
    EXPECT_EQ(capaNego->GetMapAcap().GetValue(1), tempStr1);

    capaNego->GetMapAcap().SetValue(2, tempStr2);
    EXPECT_EQ(capaNego->GetMapAcap().GetSize(), 2);
    EXPECT_EQ(capaNego->GetMapAcap().GetValue(2), tempStr2);

    capaNego->GetMapAcap().SetValue(3, tempStr3);
    EXPECT_EQ(capaNego->GetMapAcap().GetSize(), 3);
    EXPECT_EQ(capaNego->GetMapAcap().GetValue(3), tempStr3);

    capaNego->GetMapAcap().SetValue(4, tempStr4);
    EXPECT_EQ(capaNego->GetMapAcap().GetSize(), 4);
    EXPECT_EQ(capaNego->GetMapAcap().GetValue(4), tempStr4);

    capaNego->GetMapAcap().SetValue(5, tempStr5);
    EXPECT_EQ(capaNego->GetMapAcap().GetSize(), 5);
    EXPECT_EQ(capaNego->GetMapAcap().GetValue(5), tempStr5);

    delete capaNego;
}

TEST_F(MediaBaseProfileTest, TestCapaNegoPcfg)
{
    MediaBaseProfile::CapaNego* capaNego = new MediaBaseProfile::CapaNego();

    EXPECT_EQ(capaNego->GetListPcfg().GetSize(), 0);

    ImsList<AString> tempList;
    AString tempStr1 = "t=1 a=1,2,3,4,5";
    tempList.Append(tempStr1);

    capaNego->SetListPcfg(tempList);
    EXPECT_EQ(capaNego->GetListPcfg().GetSize(), 1);
    EXPECT_EQ(capaNego->GetListPcfg().GetAt(0), tempStr1);

    delete capaNego;
}

TEST_F(MediaBaseProfileTest, TestCapaNegoNegotiatedAcfg)
{
    MediaBaseProfile::CapaNego* capaNego = new MediaBaseProfile::CapaNego();

    EXPECT_EQ(capaNego->GetAcfg(), AString::ConstNull());

    AString tempStr1 = "t=1 a=1";

    capaNego->SetAcfg(tempStr1);
    EXPECT_TRUE(capaNego->GetAcfg().GetLength() > 0);
    EXPECT_EQ(capaNego->GetAcfg(), tempStr1);

    delete capaNego;
}

TEST_F(MediaBaseProfileTest, TestCapaNegoAttCapaInPcfg)
{
    MediaBaseProfile::CapaNego* capaNego = new MediaBaseProfile::CapaNego();

    EXPECT_EQ(capaNego->IsAttCapaInPcfg(), IMS_FALSE);

    IMS_BOOL testBool = IMS_TRUE;

    capaNego->SetAttCapaInPcfg(testBool);
    EXPECT_EQ(capaNego->IsAttCapaInPcfg(), testBool);

    delete capaNego;
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileIpAddress)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_EQ(objBaseProfile.GetIpAddress(), IpAddress::IPv6NONE);

    objBaseProfile.SetIpAddress(IP_ADDRESS);
    EXPECT_EQ(objBaseProfile.GetIpAddress(), IP_ADDRESS);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileDataPort)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_EQ(objBaseProfile.GetDataPort(), 0);

    objBaseProfile.SetDataPort(DATA_PORT);
    EXPECT_EQ(objBaseProfile.GetDataPort(), DATA_PORT);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileControlPort)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_EQ(objBaseProfile.GetControlPort(), 0);

    objBaseProfile.SetControlPort(CONTROL_PORT);
    EXPECT_EQ(objBaseProfile.GetControlPort(), CONTROL_PORT);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileTransportType)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_EQ(objBaseProfile.GetTransportType(), "RTP/AVP");

    objBaseProfile.SetTransportType(TRANSPORT_TYPE);
    EXPECT_EQ(objBaseProfile.GetTransportType(), TRANSPORT_TYPE);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileRtcpInterval)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_EQ(objBaseProfile.GetRtcpInterval(), 0);

    objBaseProfile.SetRtcpInterval(RTCP_INTERVAL);
    EXPECT_EQ(objBaseProfile.GetRtcpInterval(), RTCP_INTERVAL);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileBandwidthAs)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_EQ(objBaseProfile.GetBandwidthAs(), 0);

    objBaseProfile.SetBandwidthAs(AS);
    EXPECT_EQ(objBaseProfile.GetBandwidthAs(), AS);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileBandwidthRs)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_EQ(objBaseProfile.GetBandwidthRs(), 0);

    objBaseProfile.SetBandwidthRs(RS);
    EXPECT_EQ(objBaseProfile.GetBandwidthRs(), RS);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileBandwidthRr)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_EQ(objBaseProfile.GetBandwidthRr(), 0);

    objBaseProfile.SetBandwidthRr(RR);
    EXPECT_EQ(objBaseProfile.GetBandwidthRr(), RR);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileDirection)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_EQ(objBaseProfile.GetDirection(), MEDIA_DIRECTION_INVALID);

    objBaseProfile.SetDirection(DIRECTION);
    EXPECT_EQ(objBaseProfile.GetDirection(), DIRECTION);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileCapaNego)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_FALSE(objBaseProfile.GetCapaNego().IsAttCapaInPcfg());

    objBaseProfile.GetCapaNego().SetAttCapaInPcfg(IMS_TRUE);
    EXPECT_TRUE(objBaseProfile.GetCapaNego().IsAttCapaInPcfg());
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileNegotiatedPayloadIndex)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_EQ(objBaseProfile.GetNegotiatedPayloadIndex(), -1);

    objBaseProfile.SetNegotiatedPayloadIndex(NEGOTIATED_PAYLOAD_INDEX);
    EXPECT_EQ(objBaseProfile.GetNegotiatedPayloadIndex(), NEGOTIATED_PAYLOAD_INDEX);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileGetPayloadList)
{
    MediaBaseProfile objBaseProfile;

    MediaBaseProfile::BasePayload* basePayload = new MediaBaseProfile::BasePayload();
    basePayload->SetRtpMap(PAYLOAD_NUMBER, PAYLOAD_TYPE, SAMPLEING_RATE, CHANNEL);
    objBaseProfile.GetPayloadList().Append(basePayload);

    EXPECT_EQ(objBaseProfile.GetPayloadList().GetSize(), 1);

    EXPECT_EQ(objBaseProfile.GetPayloadList().GetAt(0)->GetRtpMap().GetPayloadNumber(),
            PAYLOAD_NUMBER);
    EXPECT_EQ(objBaseProfile.GetPayloadList().GetAt(0)->GetRtpMap().GetPayloadType(), PAYLOAD_TYPE);
    EXPECT_EQ(objBaseProfile.GetPayloadList().GetAt(0)->GetRtpMap().GetSamplingRate(),
            SAMPLEING_RATE);
    EXPECT_EQ(objBaseProfile.GetPayloadList().GetAt(0)->GetRtpMap().GetChannel(), CHANNEL);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileGetPayloadAt)
{
    MediaBaseProfile objBaseProfile;

    MediaBaseProfile::BasePayload* basePayload = new MediaBaseProfile::BasePayload();
    basePayload->SetRtpMap(PAYLOAD_NUMBER, PAYLOAD_TYPE, SAMPLEING_RATE, CHANNEL);
    objBaseProfile.GetPayloadList().Append(basePayload);

    EXPECT_EQ(objBaseProfile.GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), PAYLOAD_NUMBER);
    EXPECT_EQ(objBaseProfile.GetPayloadAt(0)->GetRtpMap().GetPayloadType(), PAYLOAD_TYPE);
    EXPECT_EQ(objBaseProfile.GetPayloadAt(0)->GetRtpMap().GetSamplingRate(), SAMPLEING_RATE);
    EXPECT_EQ(objBaseProfile.GetPayloadAt(0)->GetRtpMap().GetChannel(), CHANNEL);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileCreation)
{
    MediaBaseProfile objBaseProfile;
    EXPECT_EQ(objBaseProfile.GetIpAddress(), IpAddress::IPv6NONE);
    EXPECT_EQ(objBaseProfile.GetDataPort(), 0);
    EXPECT_EQ(objBaseProfile.GetControlPort(), 0);
    EXPECT_EQ(objBaseProfile.GetTransportType(), "RTP/AVP");
    EXPECT_EQ(objBaseProfile.GetRtcpInterval(), 0);
    EXPECT_EQ(objBaseProfile.GetBandwidthAs(), 0);
    EXPECT_EQ(objBaseProfile.GetBandwidthRs(), 0);
    EXPECT_EQ(objBaseProfile.GetBandwidthRr(), 0);
    EXPECT_EQ(objBaseProfile.GetDirection(), MEDIA_DIRECTION_INVALID);
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileAssign)
{
    MediaBaseProfile* baseProfile1 = new MediaBaseProfile();
    MediaBaseProfile* baseProfile2 = new MediaBaseProfile();

    InitializeProfile(baseProfile1);
    EXPECT_NE(*baseProfile2, *baseProfile1);

    *baseProfile2 = *baseProfile1;
    EXPECT_EQ(*baseProfile2, *baseProfile1);

    delete baseProfile1;
    delete baseProfile2;
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileEqual)
{
    MediaBaseProfile* baseProfile1 = new MediaBaseProfile();
    MediaBaseProfile* baseProfile2 = new MediaBaseProfile();

    EXPECT_EQ(*baseProfile1, *baseProfile2);

    InitializeProfile(baseProfile1);
    EXPECT_NE(*baseProfile1, *baseProfile2);

    InitializeProfile(baseProfile2);
    EXPECT_EQ(*baseProfile1, *baseProfile2);

    delete baseProfile1;
    delete baseProfile2;
}

TEST_F(MediaBaseProfileTest, TestMediaBaseProfileNotEqual)
{
    MediaBaseProfile* baseProfile1 = new MediaBaseProfile();
    MediaBaseProfile* baseProfile2 = new MediaBaseProfile();

    InitializeProfile(baseProfile1);
    InitializeProfile(baseProfile2);

    EXPECT_EQ(*baseProfile1, *baseProfile2);
    baseProfile1->SetIpAddress(IpAddress(AString("255.255.255.254")));
    EXPECT_NE(*baseProfile1, *baseProfile2);
    baseProfile1->SetIpAddress(IP_ADDRESS);

    EXPECT_EQ(*baseProfile1, *baseProfile2);
    baseProfile1->SetDataPort(DATA_PORT + 10);
    EXPECT_NE(*baseProfile1, *baseProfile2);
    baseProfile1->SetDataPort(DATA_PORT);

    EXPECT_EQ(*baseProfile1, *baseProfile2);
    baseProfile1->SetControlPort(CONTROL_PORT + 10);
    EXPECT_NE(*baseProfile1, *baseProfile2);
    baseProfile1->SetControlPort(CONTROL_PORT);

    EXPECT_EQ(*baseProfile1, *baseProfile2);
    baseProfile1->SetTransportType("RTP/AVP");
    EXPECT_NE(*baseProfile1, *baseProfile2);
    baseProfile1->SetTransportType(TRANSPORT_TYPE);

    EXPECT_EQ(*baseProfile1, *baseProfile2);
    baseProfile1->SetRtcpInterval(RTCP_INTERVAL + 10);
    EXPECT_NE(*baseProfile1, *baseProfile2);
    baseProfile1->SetRtcpInterval(RTCP_INTERVAL);

    EXPECT_EQ(*baseProfile1, *baseProfile2);
    baseProfile1->SetBandwidthAs(AS + 10);
    EXPECT_NE(*baseProfile1, *baseProfile2);
    baseProfile1->SetBandwidthAs(AS);

    EXPECT_EQ(*baseProfile1, *baseProfile2);
    baseProfile1->SetBandwidthRs(RS + 10);
    EXPECT_NE(*baseProfile1, *baseProfile2);
    baseProfile1->SetBandwidthRs(RS);

    EXPECT_EQ(*baseProfile1, *baseProfile2);
    baseProfile1->SetBandwidthRr(RR + 10);
    EXPECT_NE(*baseProfile1, *baseProfile2);
    baseProfile1->SetBandwidthRr(RR);

    EXPECT_EQ(*baseProfile1, *baseProfile2);
    baseProfile1->SetDirection(MEDIA_DIRECTION_RECEIVE);
    EXPECT_NE(*baseProfile1, *baseProfile2);
    baseProfile1->SetDirection(DIRECTION);

    delete baseProfile1;
    delete baseProfile2;
}
