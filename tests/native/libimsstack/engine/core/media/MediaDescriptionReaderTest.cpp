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

#include "SdpConnection.h"
#include "SdpParser.h"
#include "offeranswer/SdpAvCodec.h"
#include "offeranswer/SdpSegmentedPrecondition.h"

#include "media/MediaDescriptionReader.h"

namespace android
{

class MediaDescriptionReaderTest : public ::testing::Test
{
public:
    inline MediaDescriptionReaderTest()
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
                       "b=test:30\r\n"
                       "a=rtpmap:110 AMR-WB/16000/1\r\n"
                       "a=fmtp:110 mode-change-capability=2;max-red=220\r\n"
                       "a=sendrecv\r\n"
                       "a=ptime:20\r\n"
                       "a=maxptime:240\r\n"
                       "a=test:20\r\n"
                       "a=testname:tc20\r\n"
                       "a=setup:actpass\r\n"
                       "a=connection:new\r\n"
                       "a=curr:qos local none\r\n"
                       "a=curr:qos remote none\r\n"
                       "a=des:qos mandatory local sendrecv\r\n"
                       "a=des:qos optional remote sendrecv\r\n"
                       "a=conf:qos remote sendrecv\r\n"
                       "a=framerate:7.5\r\n");
        m_objSdpParser.Decode(strSdp);
    }

protected:
    void SetUp() override {}
    void TearDown() override {}

protected:
    SdpParser m_objSdpParser;
};

TEST_F(MediaDescriptionReaderTest, Setter)
{
    const SdpConnection* pConnection = m_objSdpParser.GetSessionDescription().GetConnection();
    const AString& strAddress =
            (pConnection != IMS_NULL) ? pConnection->GetAddress() : AString::ConstNull();
    MediaDescriptionReader objReader(m_objSdpParser.GetMediaDescriptions().GetAt(0), strAddress);

    ImsList<AString> objBandwidths;
    objBandwidths.Append("TIAS:3000");

    SdpAttribute objAttr;
    objAttr.SetValue(SdpAttribute::ATTRIBUTE_OTHER, "20", "test");

    AStringArray objPayloadFormats;
    objPayloadFormats.AddElement("112");

    SdpAvCodec objAvCodec;
    objAvCodec.SetValue("112");
    objAvCodec.SetParameters("112 AMR/8000/1", "112 mode-set=2,4,6");

    SdpSegmentedPrecondition objPrecondition;
    objPrecondition.AddStatus(SdpPrecondition::STATUS_LOCAL, SdpPrecondition::DIRECTION_SENDRECV);
    objPrecondition.AddStatus(SdpPrecondition::STATUS_REMOTE, SdpPrecondition::DIRECTION_SENDRECV);

    EXPECT_EQ(objReader.AddAttribute("test1:30"), IMS_FAILURE);
    EXPECT_EQ(objReader.RemoveAttribute("test:20"), IMS_FAILURE);
    EXPECT_EQ(objReader.SetBandwidthInfo(objBandwidths), IMS_FAILURE);
    EXPECT_EQ(objReader.SetMediaTitle("A voice call"), IMS_FAILURE);
    EXPECT_EQ(objReader.AddAttribute(SdpAttribute::ATTRIBUTE_OTHER, "30", "test1"), IMS_FAILURE);
    EXPECT_EQ(objReader.AddAttributeInt(SdpAttribute::ATTRIBUTE_OTHER, 30, "test1"), IMS_FAILURE);
    EXPECT_EQ(objReader.AddBandwidth(SdpBandwidth::TYPE_TIAS, 3000), IMS_FAILURE);
    EXPECT_EQ(objReader.RemoveAttribute(objAttr), IMS_FAILURE);
    EXPECT_EQ(objReader.RemoveAttribute(SdpAttribute::ATTRIBUTE_OTHER, "20", "test"), IMS_FAILURE);
    EXPECT_EQ(objReader.RemoveMediaFormat(SdpMediaFormat::TYPE_RTP, "110"), IMS_FAILURE);
    EXPECT_EQ(objReader.SetConnectionAddress("192.168.0.2"), IMS_FAILURE);
    EXPECT_EQ(objReader.SetDirection(Sdp::DIRECTION_SENDRECV), IMS_FAILURE);
    EXPECT_EQ(objReader.SetMediaDescription(
                      SdpMedia::TYPE_AUDIO, 48192, SdpMedia::TRANSPORT_RTP_AVP, objPayloadFormats),
            IMS_FAILURE);
    EXPECT_EQ(objReader.SetMediaFormat(&objAvCodec), IMS_FAILURE);
    EXPECT_EQ(objReader.SetMediaFormat(
                      SdpMediaFormat::TYPE_RTP, "112", "AMR/8000/1", "mode-set=2,4,6"),
            IMS_FAILURE);
    EXPECT_EQ(objReader.SetPort(48192), IMS_FAILURE);
    EXPECT_EQ(objReader.RemovePrecondition(SdpAttribute::CURR), IMS_FAILURE);
    EXPECT_EQ(objReader.SetPrecondition(SdpAttribute::CURR, &objPrecondition), IMS_FAILURE);
}

TEST_F(MediaDescriptionReaderTest, Getter)
{
    const SdpConnection* pConnection = m_objSdpParser.GetSessionDescription().GetConnection();
    const AString& strAddress =
            (pConnection != IMS_NULL) ? pConnection->GetAddress() : AString::ConstNull();
    MediaDescriptionReader objReader(m_objSdpParser.GetMediaDescriptions().GetAt(0), strAddress);
    ImsList<AString> objAttrs = objReader.GetAttributes();

    // rtpmap / fmtp / qos-attributes(curr/des/conf) will be excluded.
    ASSERT_EQ(objAttrs.GetSize(), 8);
    EXPECT_EQ(objReader.GetAttribute(SdpAttribute::SENDRECV), AString::ConstNull());
    EXPECT_EQ(objReader.GetAttribute(SdpAttribute::PTIME), AString("20"));
    EXPECT_EQ(objReader.GetAttribute(SdpAttribute::ATTRIBUTE_OTHER, "test"), AString("20"));
    objAttrs = objReader.GetAttributes(SdpAttribute::MAXPTIME);
    ASSERT_EQ(objAttrs.GetSize(), 1);
    EXPECT_EQ(objAttrs.GetAt(0), AString("240"));
    objAttrs = objReader.GetAttributes(SdpAttribute::ATTRIBUTE_OTHER, "testname");
    ASSERT_EQ(objAttrs.GetSize(), 1);
    EXPECT_EQ(objAttrs.GetAt(0), AString("tc20"));
    ImsList<AString> objBandwidths = objReader.GetBandwidthInfo();
    EXPECT_EQ(objBandwidths.GetSize(), 4);
    EXPECT_EQ(objReader.GetBandwidth(SdpBandwidth::TYPE_AS), 49);
    EXPECT_EQ(objReader.GetBandwidth(SdpBandwidth::TYPE_RS), 0);
    EXPECT_EQ(objReader.GetBandwidth(SdpBandwidth::TYPE_RR), 2500);
    EXPECT_EQ(objReader.GetBandwidth(SdpBandwidth::TYPE_OTHER, "test"), 30);
    EXPECT_EQ(objReader.GetDirection(), Sdp::DIRECTION_SENDRECV);
    EXPECT_EQ(objReader.GetMediaTitle(), AString::ConstNull());
    EXPECT_EQ(objReader.GetLocalAddress(), IpAddress::NONE);
    EXPECT_EQ(objReader.GetLocalPort(), 0);
    EXPECT_EQ(objReader.GetRemoteAddress(), IpAddress(AString("192.168.0.1")));
    EXPECT_EQ(objReader.GetRemotePort(), 50010);

    EXPECT_EQ(objReader.GetMediaDescription(), AString("audio 50010 RTP/AVP 110"));
    EXPECT_EQ(objReader.GetMediaDescriptionExAsLocal(), nullptr);

    const SdpMedia* pSdpMedia = objReader.GetMediaDescriptionEx();
    ASSERT_NE(pSdpMedia, nullptr);
    EXPECT_EQ(pSdpMedia->GetType(), SdpMedia::TYPE_AUDIO);
    EXPECT_EQ(pSdpMedia->GetPort(), 50010);
    EXPECT_EQ(pSdpMedia->GetTransportProtocol(), SdpMedia::TRANSPORT_RTP_AVP);
    const AStringArray& objFormats = pSdpMedia->GetFormats();
    ASSERT_EQ(objFormats.GetCount(), 1);
    EXPECT_EQ(objFormats.GetFirstElement(), AString("110"));

    const ImsList<SdpMediaFormat*>& objMediaFormats = objReader.GetMediaFormats();
    ASSERT_EQ(objMediaFormats.GetSize(), 1);
    const SdpAvCodec* pAvCodec = static_cast<const SdpAvCodec*>(objMediaFormats.GetAt(0));
    ASSERT_NE(pAvCodec, nullptr);
    EXPECT_EQ(pAvCodec->GetType(), SdpMediaFormat::TYPE_RTP);
    EXPECT_EQ(pAvCodec->GetValue(), AString("110"));
    EXPECT_EQ(pAvCodec->GetPayloadType(), 110);
    EXPECT_EQ(pAvCodec->GetName(), AString("AMR-WB"));
    EXPECT_EQ(pAvCodec->GetClockRate(), 16000);
    EXPECT_EQ(pAvCodec->GetEncodingParameters(), AString("1"));

    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::PTIME), 20);
    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::ATTRIBUTE_OTHER, "test"), 20);
    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::FRAMERATE), 7);
    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::ATTRIBUTE_OTHER, "testname"),
            IMediaDescriptor::INVALID_VALUE);
    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::SENDRECV), IMediaDescriptor::INVALID_VALUE);
    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::SETUP), Sdp::SETUP_ACTPASS);
    EXPECT_EQ(objReader.GetAttributeInt(SdpAttribute::CONNECTION), Sdp::CONNECTION_NEW);

    const SdpSegmentedPrecondition* pCurr = static_cast<const SdpSegmentedPrecondition*>(
            objReader.GetPrecondition(SdpAttribute::CURR));
    const SdpSegmentedPrecondition* pDes = static_cast<const SdpSegmentedPrecondition*>(
            objReader.GetPrecondition(SdpAttribute::DES));
    const SdpSegmentedPrecondition* pConf = static_cast<const SdpSegmentedPrecondition*>(
            objReader.GetPrecondition(SdpAttribute::CONF));

    ASSERT_NE(pCurr, nullptr);
    EXPECT_EQ(pCurr->GetType(), SdpPrecondition::TYPE_QOS);
    EXPECT_EQ(pCurr->GetSubType(), SdpPrecondition::SUBTYPE_SEGMENTED);
    EXPECT_TRUE(pCurr->IsPreconditionPresent());
    const SdpPrecondition::DetailInfo& objCurrLocal = pCurr->GetLocalDetails().GetAt(0);
    EXPECT_EQ(objCurrLocal.GetStatus(), SdpPrecondition::STATUS_LOCAL);
    EXPECT_EQ(objCurrLocal.GetStrength(), SdpPrecondition::STRENGTH_NOTUSED);
    EXPECT_EQ(objCurrLocal.GetDirection(), SdpPrecondition::DIRECTION_NONE);
    const SdpPrecondition::DetailInfo& objCurrRemote = pCurr->GetRemoteDetails().GetAt(0);
    EXPECT_EQ(objCurrRemote.GetStatus(), SdpPrecondition::STATUS_REMOTE);
    EXPECT_EQ(objCurrRemote.GetStrength(), SdpPrecondition::STRENGTH_NOTUSED);
    EXPECT_EQ(objCurrRemote.GetDirection(), SdpPrecondition::DIRECTION_NONE);

    ASSERT_NE(pDes, nullptr);
    EXPECT_EQ(pDes->GetType(), SdpPrecondition::TYPE_QOS);
    EXPECT_EQ(pDes->GetSubType(), SdpPrecondition::SUBTYPE_SEGMENTED);
    EXPECT_TRUE(pDes->IsPreconditionPresent());
    const SdpPrecondition::DetailInfo& objDesLocal = pDes->GetLocalDetails().GetAt(0);
    EXPECT_EQ(objDesLocal.GetStatus(), SdpPrecondition::STATUS_LOCAL);
    EXPECT_EQ(objDesLocal.GetStrength(), SdpPrecondition::STRENGTH_MANDATORY);
    EXPECT_EQ(objDesLocal.GetDirection(), SdpPrecondition::DIRECTION_SENDRECV);
    const SdpPrecondition::DetailInfo& objDesRemote = pDes->GetRemoteDetails().GetAt(0);
    EXPECT_EQ(objDesRemote.GetStatus(), SdpPrecondition::STATUS_REMOTE);
    EXPECT_EQ(objDesRemote.GetStrength(), SdpPrecondition::STRENGTH_OPTIONAL);
    EXPECT_EQ(objDesRemote.GetDirection(), SdpPrecondition::DIRECTION_SENDRECV);

    ASSERT_NE(pConf, nullptr);
    EXPECT_EQ(pConf->GetType(), SdpPrecondition::TYPE_QOS);
    EXPECT_EQ(pConf->GetSubType(), SdpPrecondition::SUBTYPE_SEGMENTED);
    const SdpPrecondition::DetailInfo& objConfRemote = pConf->GetRemoteDetails().GetAt(0);
    EXPECT_EQ(objConfRemote.GetStatus(), SdpPrecondition::STATUS_REMOTE);
    EXPECT_EQ(objConfRemote.GetStrength(), SdpPrecondition::STRENGTH_NOTUSED);
    EXPECT_EQ(objConfRemote.GetDirection(), SdpPrecondition::DIRECTION_SENDRECV);
}

}  // namespace android
