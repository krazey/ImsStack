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

const IMS_UINT32 PAYLOAD_NUM = 100;
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

TEST(MediaBaseProfileTest, TestRtpMapCreation)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    EXPECT_EQ(rtpMap->nPayloadNum, 0);
    EXPECT_EQ(rtpMap->strPayloadType, AString::ConstNull());
    EXPECT_EQ(rtpMap->nSamplingRate, 0);
    EXPECT_EQ(rtpMap->nChannel, 0);

    MediaBaseProfile::RtpMap* rtpMap1 = new MediaBaseProfile::RtpMap(CHANNEL);
    EXPECT_EQ(rtpMap1->nPayloadNum, 0);
    EXPECT_EQ(rtpMap1->strPayloadType, AString::ConstNull());
    EXPECT_EQ(rtpMap1->nSamplingRate, 0);
    EXPECT_EQ(rtpMap1->nChannel, CHANNEL);

    rtpMap1->nPayloadNum = PAYLOAD_NUM;
    rtpMap1->strPayloadType = PAYLOAD_TYPE;
    rtpMap1->nSamplingRate = SAMPLEING_RATE;

    MediaBaseProfile::RtpMap* rtpMap2 = new MediaBaseProfile::RtpMap(*rtpMap1);
    EXPECT_EQ(rtpMap2->nPayloadNum, PAYLOAD_NUM);
    EXPECT_EQ(rtpMap2->strPayloadType, PAYLOAD_TYPE);
    EXPECT_EQ(rtpMap2->nSamplingRate, SAMPLEING_RATE);
    EXPECT_EQ(rtpMap2->nChannel, CHANNEL);

    delete rtpMap;
    delete rtpMap1;
    delete rtpMap2;
}

TEST(MediaBaseProfileTest, TestRtpMapAssign)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    MediaBaseProfile::RtpMap* rtpMap1 = new MediaBaseProfile::RtpMap(CHANNEL);
    rtpMap1->nPayloadNum = PAYLOAD_NUM;
    rtpMap1->strPayloadType = PAYLOAD_TYPE;
    rtpMap1->nSamplingRate = SAMPLEING_RATE;

    EXPECT_EQ(rtpMap->nPayloadNum, 0);
    EXPECT_EQ(rtpMap->strPayloadType, AString::ConstNull());
    EXPECT_EQ(rtpMap->nSamplingRate, 0);
    EXPECT_EQ(rtpMap->nChannel, 0);

    *rtpMap = *rtpMap1;

    EXPECT_EQ(rtpMap->nPayloadNum, PAYLOAD_NUM);
    EXPECT_EQ(rtpMap->strPayloadType, PAYLOAD_TYPE);
    EXPECT_EQ(rtpMap->nSamplingRate, SAMPLEING_RATE);
    EXPECT_EQ(rtpMap->nChannel, CHANNEL);

    delete rtpMap;
    delete rtpMap1;
}

TEST(MediaBaseProfileTest, TestRtpMapAssignEqual)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    MediaBaseProfile::RtpMap* rtpMap1 = new MediaBaseProfile::RtpMap(CHANNEL);
    rtpMap1->nPayloadNum = PAYLOAD_NUM;
    rtpMap1->strPayloadType = PAYLOAD_TYPE;
    rtpMap1->nSamplingRate = SAMPLEING_RATE;

    EXPECT_NE(*rtpMap, *rtpMap1);

    *rtpMap = *rtpMap1;
    EXPECT_EQ(*rtpMap, *rtpMap1);

    rtpMap1->nSamplingRate = SAMPLEING_RATE + 1;
    EXPECT_NE(*rtpMap, *rtpMap1);

    delete rtpMap;
    delete rtpMap1;
}

TEST(MediaBaseProfileTest, TestRtpMapEqual)
{
    MediaBaseProfile::RtpMap* rtpMap = new MediaBaseProfile::RtpMap();
    MediaBaseProfile::RtpMap* rtpMap1 = new MediaBaseProfile::RtpMap(CHANNEL);
    rtpMap1->nPayloadNum = PAYLOAD_NUM;
    rtpMap1->strPayloadType = PAYLOAD_TYPE;
    rtpMap1->nSamplingRate = SAMPLEING_RATE;

    EXPECT_NE(*rtpMap, *rtpMap1);

    rtpMap->nPayloadNum = PAYLOAD_NUM;
    EXPECT_NE(*rtpMap, *rtpMap1);

    rtpMap->strPayloadType = PAYLOAD_TYPE;
    EXPECT_NE(*rtpMap, *rtpMap1);

    rtpMap->nSamplingRate = SAMPLEING_RATE;
    EXPECT_NE(*rtpMap, *rtpMap1);

    rtpMap->nChannel = CHANNEL;
    EXPECT_EQ(*rtpMap, *rtpMap1);

    rtpMap->nPayloadNum = PAYLOAD_NUM + 1;
    EXPECT_NE(*rtpMap, *rtpMap1);

    delete rtpMap;
    delete rtpMap1;
}

TEST(MediaBaseProfileTest, TestBasePayloadCreation)
{
    MediaBaseProfile::BasePayload* basePayload = new MediaBaseProfile::BasePayload();
    MediaBaseProfile::RtpMap tempRtpMap;

    // Compare default values
    EXPECT_EQ(basePayload->objRtpMap, tempRtpMap);
    EXPECT_EQ(basePayload->pFmtp, nullptr);

    MediaBaseProfile::BasePayload* basePayload1 = new MediaBaseProfile::BasePayload(CHANNEL);
    EXPECT_NE(basePayload1->objRtpMap, tempRtpMap);

    tempRtpMap.nChannel = CHANNEL;
    EXPECT_EQ(basePayload1->objRtpMap, tempRtpMap);
    EXPECT_EQ(basePayload1->pFmtp, nullptr);

    basePayload1->objRtpMap.nPayloadNum = PAYLOAD_NUM;
    basePayload1->objRtpMap.strPayloadType = PAYLOAD_TYPE;
    basePayload1->objRtpMap.nSamplingRate = SAMPLEING_RATE;

    MediaBaseProfile::BasePayload* basePayload2 = new MediaBaseProfile::BasePayload(*basePayload1);
    EXPECT_EQ(basePayload2->objRtpMap, basePayload1->objRtpMap);
    EXPECT_EQ(basePayload2->pFmtp, nullptr);

    delete basePayload;
    delete basePayload1;
    delete basePayload2;
}

TEST(MediaBaseProfileTest, TestBasePayloadAssign)
{
    MediaBaseProfile::BasePayload* basePayload = new MediaBaseProfile::BasePayload(CHANNEL);
    basePayload->objRtpMap.nPayloadNum = PAYLOAD_NUM;
    basePayload->objRtpMap.strPayloadType = PAYLOAD_TYPE;
    basePayload->objRtpMap.nSamplingRate = SAMPLEING_RATE;

    MediaBaseProfile::BasePayload* basePayload1 = new MediaBaseProfile::BasePayload();

    EXPECT_NE(basePayload->objRtpMap, basePayload1->objRtpMap);
    EXPECT_EQ(basePayload->pFmtp, nullptr);

    *basePayload1 = *basePayload;

    EXPECT_EQ(basePayload1->objRtpMap, basePayload->objRtpMap);
    EXPECT_EQ(basePayload1->pFmtp, nullptr);

    delete basePayload;
    delete basePayload1;
}

TEST(MediaBaseProfileTest, TestBasePayloadSetRtpMap)
{
    MediaBaseProfile::BasePayload* basePayload = new MediaBaseProfile::BasePayload();

    basePayload->SetRtpMap(PAYLOAD_NUM, PAYLOAD_TYPE, SAMPLEING_RATE);
    EXPECT_EQ(basePayload->objRtpMap.nPayloadNum, PAYLOAD_NUM);
    EXPECT_EQ(basePayload->objRtpMap.strPayloadType, PAYLOAD_TYPE);
    EXPECT_EQ(basePayload->objRtpMap.nSamplingRate, SAMPLEING_RATE);
    EXPECT_EQ(basePayload->objRtpMap.nChannel, 0);

    basePayload->SetRtpMap(PAYLOAD_NUM, PAYLOAD_TYPE, SAMPLEING_RATE, CHANNEL);
    EXPECT_EQ(basePayload->objRtpMap.nPayloadNum, PAYLOAD_NUM);
    EXPECT_EQ(basePayload->objRtpMap.strPayloadType, PAYLOAD_TYPE);
    EXPECT_EQ(basePayload->objRtpMap.nSamplingRate, SAMPLEING_RATE);
    EXPECT_EQ(basePayload->objRtpMap.nChannel, CHANNEL);

    MediaBaseProfile::RtpMap tempRtpMap;

    basePayload->SetRtpMap(tempRtpMap);
    EXPECT_EQ(basePayload->objRtpMap.nPayloadNum, 0);
    EXPECT_EQ(basePayload->objRtpMap.strPayloadType, AString::ConstNull());
    EXPECT_EQ(basePayload->objRtpMap.nSamplingRate, 0);
    EXPECT_EQ(basePayload->objRtpMap.nChannel, 0);

    tempRtpMap.nChannel = CHANNEL;
    tempRtpMap.nPayloadNum = PAYLOAD_NUM;
    tempRtpMap.strPayloadType = PAYLOAD_TYPE;
    tempRtpMap.nSamplingRate = SAMPLEING_RATE;

    basePayload->SetRtpMap(tempRtpMap);
    EXPECT_EQ(basePayload->objRtpMap.nPayloadNum, PAYLOAD_NUM);
    EXPECT_EQ(basePayload->objRtpMap.strPayloadType, PAYLOAD_TYPE);
    EXPECT_EQ(basePayload->objRtpMap.nSamplingRate, SAMPLEING_RATE);
    EXPECT_EQ(basePayload->objRtpMap.nChannel, CHANNEL);

    delete basePayload;
}

TEST(MediaBaseProfileTest, TestCapaNegoCreation)
{
    MediaBaseProfile::CapaNego* capaNego = new MediaBaseProfile::CapaNego();

    EXPECT_EQ(capaNego->mapTransportCapa.GetSize(), 0);
    EXPECT_EQ(capaNego->mapAttributeCapa.GetSize(), 0);
    EXPECT_EQ(capaNego->lstPotentialConfig.GetSize(), 0);
    EXPECT_EQ(capaNego->strNegotiatedAcfg, AString::ConstNull());
    EXPECT_EQ(capaNego->bIsAttCapaInPcfg, IMS_FALSE);

    delete capaNego;
}

TEST(MediaBaseProfileTest, TestMediaBaseProfileCreation)
{
    MediaBaseProfile* baseProfile1 = new MediaBaseProfile();
    EXPECT_EQ(baseProfile1->objIpAddress, IpAddress::IPv6NONE);
    EXPECT_EQ(baseProfile1->nDataPort, 0);
    EXPECT_EQ(baseProfile1->nControlPort, 0);
    EXPECT_EQ(baseProfile1->strTransportType, "RTP/AVP");
    EXPECT_EQ(baseProfile1->nRtcpInterval, 0);
    EXPECT_EQ(baseProfile1->nBandwidthAs, 0);
    EXPECT_EQ(baseProfile1->nBandwidthRs, 0);
    EXPECT_EQ(baseProfile1->nBandwidthRr, 0);
    EXPECT_EQ(baseProfile1->eDirection, MEDIA_DIRECTION_INVALID);

    baseProfile1->objIpAddress = IP_ADDRESS;
    baseProfile1->nDataPort = DATA_PORT;
    baseProfile1->nControlPort = CONTROL_PORT;
    baseProfile1->strTransportType = TRANSPORT_TYPE;
    baseProfile1->nRtcpInterval = RTCP_INTERVAL;
    baseProfile1->nBandwidthAs = AS;
    baseProfile1->nBandwidthRs = RS;
    baseProfile1->nBandwidthRr = RR;
    baseProfile1->eDirection = DIRECTION;

    MediaBaseProfile* baseProfile2 = new MediaBaseProfile(baseProfile1);
    EXPECT_EQ(baseProfile2->objIpAddress, IP_ADDRESS);
    EXPECT_EQ(baseProfile2->nDataPort, DATA_PORT);
    EXPECT_EQ(baseProfile2->nControlPort, CONTROL_PORT);
    EXPECT_EQ(baseProfile2->strTransportType, TRANSPORT_TYPE);
    EXPECT_EQ(baseProfile2->nRtcpInterval, RTCP_INTERVAL);
    EXPECT_EQ(baseProfile2->nBandwidthAs, AS);
    EXPECT_EQ(baseProfile2->nBandwidthRs, RS);
    EXPECT_EQ(baseProfile2->nBandwidthRr, RR);
    EXPECT_EQ(baseProfile2->eDirection, DIRECTION);

    MediaBaseProfile* baseProfile3 = new MediaBaseProfile(*baseProfile2);
    EXPECT_EQ(baseProfile3->objIpAddress, IP_ADDRESS);
    EXPECT_EQ(baseProfile3->nDataPort, DATA_PORT);
    EXPECT_EQ(baseProfile3->nControlPort, CONTROL_PORT);
    EXPECT_EQ(baseProfile3->strTransportType, TRANSPORT_TYPE);
    EXPECT_EQ(baseProfile3->nRtcpInterval, RTCP_INTERVAL);
    EXPECT_EQ(baseProfile3->nBandwidthAs, AS);
    EXPECT_EQ(baseProfile3->nBandwidthRs, RS);
    EXPECT_EQ(baseProfile3->nBandwidthRr, RR);
    EXPECT_EQ(baseProfile3->eDirection, DIRECTION);

    delete baseProfile1;
    delete baseProfile2;
    delete baseProfile3;
}

TEST(MediaBaseProfileTest, TestMediaBaseProfileAssign)
{
    MediaBaseProfile* baseProfile1 = new MediaBaseProfile();
    MediaBaseProfile* baseProfile2 = new MediaBaseProfile();

    baseProfile1->objIpAddress = IP_ADDRESS;
    baseProfile1->nDataPort = DATA_PORT;
    baseProfile1->nControlPort = CONTROL_PORT;
    baseProfile1->strTransportType = TRANSPORT_TYPE;
    baseProfile1->nRtcpInterval = RTCP_INTERVAL;
    baseProfile1->nBandwidthAs = AS;
    baseProfile1->nBandwidthRs = RS;
    baseProfile1->nBandwidthRr = RR;
    baseProfile1->eDirection = DIRECTION;

    *baseProfile2 = *baseProfile1;

    EXPECT_EQ(baseProfile2->objIpAddress, IP_ADDRESS);
    EXPECT_EQ(baseProfile2->nDataPort, DATA_PORT);
    EXPECT_EQ(baseProfile2->nControlPort, CONTROL_PORT);
    EXPECT_EQ(baseProfile2->strTransportType, TRANSPORT_TYPE);
    EXPECT_EQ(baseProfile2->nRtcpInterval, RTCP_INTERVAL);
    EXPECT_EQ(baseProfile2->nBandwidthAs, AS);
    EXPECT_EQ(baseProfile2->nBandwidthRs, RS);
    EXPECT_EQ(baseProfile2->nBandwidthRr, RR);
    EXPECT_EQ(baseProfile2->eDirection, DIRECTION);

    delete baseProfile1;
    delete baseProfile2;
}

TEST(MediaBaseProfileTest, TestMediaBaseProfileEqualNotEqual)
{
    MediaBaseProfile* baseProfile1 = new MediaBaseProfile();
    MediaBaseProfile* baseProfile2 = new MediaBaseProfile();

    EXPECT_EQ(*baseProfile1, *baseProfile2);

    baseProfile1->objIpAddress = IP_ADDRESS;
    baseProfile1->nDataPort = DATA_PORT;
    baseProfile1->nControlPort = CONTROL_PORT;
    baseProfile1->strTransportType = TRANSPORT_TYPE;
    baseProfile1->nRtcpInterval = RTCP_INTERVAL;
    baseProfile1->nBandwidthAs = AS;
    baseProfile1->nBandwidthRs = RS;
    baseProfile1->nBandwidthRr = RR;
    baseProfile1->eDirection = DIRECTION;

    EXPECT_NE(*baseProfile1, *baseProfile2);

    baseProfile2->objIpAddress = IP_ADDRESS;
    baseProfile2->nDataPort = DATA_PORT;
    baseProfile2->nControlPort = CONTROL_PORT;
    baseProfile2->strTransportType = TRANSPORT_TYPE;
    baseProfile2->nRtcpInterval = RTCP_INTERVAL;
    baseProfile2->nBandwidthAs = AS;
    baseProfile2->nBandwidthRs = RS;
    baseProfile2->nBandwidthRr = RR;
    baseProfile2->eDirection = DIRECTION;

    EXPECT_EQ(*baseProfile1, *baseProfile2);

    baseProfile2->nControlPort = DATA_PORT;

    EXPECT_NE(*baseProfile1, *baseProfile2);

    delete baseProfile1;
    delete baseProfile2;
}
