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

#include "AString.h"
#include "ByteArray.h"
#include "ServiceMemory.h"
#include "GeolocationPidfWriter.h"
#include "IXmlStreamWriter.h"
#include "XmlFactory.h"
#include <gtest/gtest.h>

using namespace enabler;

class GeolocationPidfWriterTest : public ::testing::Test
{
public:
    XmlFactory* pXmlFactory;
    IXmlStreamWriter* pWriter;

    void SetUp() override
    {
        pXmlFactory = XmlFactory::GetInstance();
        pWriter = pXmlFactory->CreateStreamWriter();
    }

    void TearDown() override
    {
        pWriter->Close();
        pXmlFactory->DestroyStreamWriter(pWriter);
    }

    AString GetResultString(IN IXmlStreamWriter* pWriter)
    {
        IMS_CHAR* pszXml = pWriter->Flush();
        if (pszXml == IMS_NULL)
        {
            return AString::ConstNull();
        }

        ByteArray objContent;
        objContent.Attach(reinterpret_cast<IMS_BYTE*>(pszXml), pWriter->GetContentLength());
        objContent.Detach();
        IMS_MEM_Free(pszXml);

        return objContent.ToString();
    }

    void AssertXmlStringEquality(IN AString strActual, IN AString strExpected)
    {
        EXPECT_STREQ(strActual.Replace("\n", "").GetStr(), strExpected.Replace("\n", "").GetStr());
    }
};

TEST_F(GeolocationPidfWriterTest, WriteNestedElements)
{
    // clang-format off
    ByteArray objContent = PidfLoXml{
        new Tuple{"id", {
            new Method{"method"},
        }},
        new Geopriv{},
    }.Write();
    // clang-format on

    const AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                "<tuple id=\"id\">"
                                "<gp:method>method</gp:method>"
                                "</tuple>"
                                "<gp:geopriv>"
                                "</gp:geopriv>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteAppendedElements)
{
    PidfLoXml objElement{};
    // clang-format off
    objElement.Append(
        new Tuple{"id", {
            new Method{"method"},
        }}
    );
    objElement.Append(new Geopriv{});
    // clang-format on
    ByteArray objContent = objElement.Write();

    const AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                "<tuple id=\"id\">"
                                "<gp:method>method</gp:method>"
                                "</tuple>"
                                "<gp:geopriv>"
                                "</gp:geopriv>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePidfLoXml)
{
    ByteArray objContent = PidfLoXml{}.Write();

    const AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePresenceWithNone)
{
    Presence{Presence::Namespace::NONE, "uri", {}}.Write(*pWriter);

    const AString strExpected = "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" entity=\"uri\">"
                                "</presence>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePresenceWithDm)
{
    Presence{Presence::Namespace::DM, "uri", {}}.Write(*pWriter);

    const AString strExpected =
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" entity=\"uri\">"
            "</presence>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePresenceWithGp)
{
    Presence{Presence::Namespace::GP, "uri", {}}.Write(*pWriter);

    const AString strExpected = "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" entity=\"uri\">"
                                "</presence>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePresenceWithGml)
{
    Presence{Presence::Namespace::GML, "uri", {}}.Write(*pWriter);

    const AString strExpected = "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                "xmlns:gml=\"http://www.opengis.net/gml\" entity=\"uri\">"
                                "</presence>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePresenceWithGs)
{
    Presence{Presence::Namespace::GS, "uri", {}}.Write(*pWriter);

    const AString strExpected = "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" entity=\"uri\">"
                                "</presence>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePresenceWithCl)
{
    Presence{Presence::Namespace::CL, "uri", {}}.Write(*pWriter);

    const AString strExpected =
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" entity=\"uri\">"
            "</presence>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePresenceWithCon)
{
    Presence{Presence::Namespace::CON, "uri", {}}.Write(*pWriter);

    const AString strExpected = "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" entity=\"uri\">"
                                "</presence>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePresenceWithGbp)
{
    Presence{Presence::Namespace::GBP, "uri", {}}.Write(*pWriter);

    const AString strExpected = "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
                                "entity=\"uri\">"
                                "</presence>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePresenceWithAll)
{
    Presence{Presence::Namespace::ALL, "uri", {}}.Write(*pWriter);

    const AString strExpected = "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                                "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                                "xmlns:gml=\"http://www.opengis.net/gml\" "
                                "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
                                "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
                                "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
                                "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
                                "entity=\"uri\">"
                                "</presence>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePresenceWithCountry)
{
    Presence{Presence::Namespace::COUNTRY, "uri", {}}.Write(*pWriter);

    const AString strExpected =
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" entity=\"uri\">"
            "</presence>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WritePerson)
{
    Person{"id", {}}.Write(*pWriter);

    const AString strExpected = "<dm:person id=\"id\">"
                                "</dm:person>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteDevice)
{
    Device{"id", {}}.Write(*pWriter);

    const AString strExpected = "<dm:device id=\"id\">"
                                "</dm:device>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteTuple)
{
    Tuple{"id", {}}.Write(*pWriter);

    const AString strExpected = "<tuple id=\"id\">"
                                "</tuple>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteStatus)
{
    Status{}.Write(*pWriter);

    const AString strExpected = "<status>"
                                "</status>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteGeopriv)
{
    Geopriv{}.Write(*pWriter);

    const AString strExpected = "<gp:geopriv>"
                                "</gp:geopriv>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteLocationInfo)
{
    LocationInfo{}.Write(*pWriter);

    const AString strExpected = "<gp:location-info>"
                                "</gp:location-info>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteUsageRules)
{
    UsageRules{}.Write(*pWriter);

    const AString strExpected = "<gp:usage-rules>"
                                "</gp:usage-rules>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteRetransmissionAllowed)
{
    RetransmissionAllowed{"true"}.Write(*pWriter);

    const AString strExpected = "<gbp:retransmission-allowed>true</gbp:retransmission-allowed>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteRetransmissionAllowedEmpty)
{
    RetransmissionAllowed{""}.Write(*pWriter);

    AssertXmlStringEquality(GetResultString(pWriter), "");
}

TEST_F(GeolocationPidfWriterTest, WriteMethod)
{
    Method{"method"}.Write(*pWriter);

    const AString strExpected = "<gp:method>method</gp:method>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteMethodEmpty)
{
    Method{""}.Write(*pWriter);

    AssertXmlStringEquality(GetResultString(pWriter), "");
}

TEST_F(GeolocationPidfWriterTest, WriteTimestamp)
{
    Timestamp{"timestamp"}.Write(*pWriter);

    const AString strExpected = "<dm:timestamp>timestamp</dm:timestamp>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteTimestampEmpty)
{
    Timestamp{""}.Write(*pWriter);

    AssertXmlStringEquality(GetResultString(pWriter), "");
}

TEST_F(GeolocationPidfWriterTest, WriteDeviceId)
{
    DeviceId{"id"}.Write(*pWriter);

    const AString strExpected = "<dm:deviceID>id</dm:deviceID>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteDeviceIdEmpty)
{
    DeviceId{""}.Write(*pWriter);

    AssertXmlStringEquality(GetResultString(pWriter), "");
}

TEST_F(GeolocationPidfWriterTest, WriteCircle)
{
    Circle{"lat", "long", "radius"}.Write(*pWriter);

    const AString strExpected = "<gs:Circle srsName=\"urn:ogc:def:crs:EPSG::4326\">"
                                "<gml:pos>lat long</gml:pos><gs:radius "
                                "uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:radius>"
                                "</gs:Circle>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteCircleEmptyPos)
{
    Circle{"", "long", "radius"}.Write(*pWriter);
    Circle{"lat", "", "radius"}.Write(*pWriter);

    AssertXmlStringEquality(GetResultString(pWriter), "");
}

TEST_F(GeolocationPidfWriterTest, WriteCircleEmptyRadius)
{
    Circle{"lat", "long", ""}.Write(*pWriter);

    const AString strExpected = "<gs:Circle srsName=\"urn:ogc:def:crs:EPSG::4326\">"
                                "<gml:pos>lat long</gml:pos>"
                                "</gs:Circle>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteEllipsoid)
{
    Ellipsoid{"lat", "long", "alt", "radius", "vaccu"}.Write(*pWriter);

    const AString strExpected =
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis "
            "uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis><gs:semiMinorAxis "
            "uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis><gs:verticalAxis "
            "uom=\"urn:ogc:def:uom:EPSG::9001\">vaccu</gs:verticalAxis><gs:orientation "
            "uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteEllipsoidEmptyPos)
{
    Ellipsoid{"", "long", "alt", "radius", "vaccu"}.Write(*pWriter);
    Ellipsoid{"lat", "", "alt", "radius", "vaccu"}.Write(*pWriter);
    Ellipsoid{"lat", "long", "", "radius", "vaccu"}.Write(*pWriter);

    AssertXmlStringEquality(GetResultString(pWriter), "");
}

TEST_F(GeolocationPidfWriterTest, WriteEllipsoidEmptyRadius)
{
    Ellipsoid{"lat", "long", "alt", "", "vaccu"}.Write(*pWriter);

    const AString strExpected = "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
                                "<gml:pos>lat long alt</gml:pos>"
                                "</gs:Ellipsoid>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteEllipsoidEmptyVerticalAccuracy)
{
    Ellipsoid{"lat", "long", "alt", "radius", ""}.Write(*pWriter);

    const AString strExpected = "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
                                "<gml:pos>lat long alt</gml:pos>"
                                "</gs:Ellipsoid>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteConfidence)
{
    Confidence{"confidence"}.Write(*pWriter);

    const AString strExpected = "<con:confidence pdf=\"normal\">confidence</con:confidence>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteConfidenceEmpty)
{
    Confidence{""}.Write(*pWriter);

    AssertXmlStringEquality(GetResultString(pWriter), "");
}

TEST_F(GeolocationPidfWriterTest, WriteCivicAddress)
{
    CivicAddress{"country", "state", "city", "postal"}.Write(*pWriter);

    const AString strExpected = "<cl:civicAddress>"
                                "<cl:country>country</cl:country>"
                                "<cl:A1>state</cl:A1>"
                                "<cl:A2>city</cl:A2>"
                                "<cl:PC>postal</cl:PC>"
                                "</cl:civicAddress>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteCivicAddressEmpty)
{
    CivicAddress{"", "", "", ""}.Write(*pWriter);

    AssertXmlStringEquality(GetResultString(pWriter), "");
}

TEST_F(GeolocationPidfWriterTest, WriteCivicAddressEmptyCountry)
{
    CivicAddress{"", "state", "city", "postal"}.Write(*pWriter);

    const AString strExpected = "<cl:civicAddress>"
                                "<cl:A1>state</cl:A1>"
                                "<cl:A2>city</cl:A2>"
                                "<cl:PC>postal</cl:PC>"
                                "</cl:civicAddress>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteCivicAddressEmptyState)
{
    CivicAddress{"country", "", "city", "postal"}.Write(*pWriter);

    const AString strExpected = "<cl:civicAddress>"
                                "<cl:country>country</cl:country>"
                                "<cl:A2>city</cl:A2>"
                                "<cl:PC>postal</cl:PC>"
                                "</cl:civicAddress>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteCivicAddressEmptyCity)
{
    CivicAddress{"country", "state", "", "postal"}.Write(*pWriter);

    const AString strExpected = "<cl:civicAddress>"
                                "<cl:country>country</cl:country>"
                                "<cl:A1>state</cl:A1>"
                                "<cl:PC>postal</cl:PC>"
                                "</cl:civicAddress>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}

TEST_F(GeolocationPidfWriterTest, WriteCivicAddressEmptyPostal)
{
    CivicAddress{"country", "state", "city", ""}.Write(*pWriter);

    const AString strExpected = "<cl:civicAddress>"
                                "<cl:country>country</cl:country>"
                                "<cl:A1>state</cl:A1>"
                                "<cl:A2>city</cl:A2>"
                                "</cl:civicAddress>";
    AssertXmlStringEquality(GetResultString(pWriter), strExpected);
}
