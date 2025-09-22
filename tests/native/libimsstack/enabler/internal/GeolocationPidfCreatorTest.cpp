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
#include "GeolocationPidfCreator.h"
#include "MockIPhoneInfoLocation.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"
#include "device/OsLocationInfo.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL const AString UNKNOWN_POSITION = "0.0";

class GeolocationPidfCreatorTest : public ::testing::Test
{
public:
    const AString strLatitude = "lat";
    const AString strLongitude = "long";
    const AString strRadius = "radius";
    const AString strShape = "Ellipsoid";
    const AString strConfidence = "confidence";
    const AString strCurrentTime = "time";
    const AString strMethod = "method";
    const AString strCountry = "country";
    const AString strState = "state";
    const AString strCity = "city";
    const AString strPostal = "postal";
    const AString strAltitude = "alt";
    const AString strVerticalAccuracy = "v_accuracy";
    const AString strDeviceId = "device_id";
    const AString strRetransmissionAllowed = "allowed";

    TestPhoneInfoService objPhoneInfoService;
    MockILocationProperties objLocationProperties;
    GeolocationPidfCreator* pCreator;

    void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);

        SetUpTestLocationProperties();
        ON_CALL(objPhoneInfoService.GetMockLocationInfo(), GetLocationProperties(_))
                .WillByDefault(Return(&objLocationProperties));

        pCreator = new GeolocationPidfCreator(0);
        pCreator->SetDeviceId(strDeviceId);
        pCreator->SetRetransmissionAllowed(strRetransmissionAllowed);
    }

    void TearDown() override
    {
        delete pCreator;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
    }

    void SetUpTestLocationProperties()
    {
        ON_CALL(objLocationProperties, GetLatitude).WillByDefault(ReturnRef(strLatitude));
        ON_CALL(objLocationProperties, GetLongitude).WillByDefault(ReturnRef(strLongitude));
        ON_CALL(objLocationProperties, GetRadius).WillByDefault(ReturnRef(strRadius));
        ON_CALL(objLocationProperties, GetShape).WillByDefault(ReturnRef(strShape));
        ON_CALL(objLocationProperties, GetConfidence).WillByDefault(ReturnRef(strConfidence));
        ON_CALL(objLocationProperties, GetCurrentTime).WillByDefault(ReturnRef(strCurrentTime));
        ON_CALL(objLocationProperties, GetMethod).WillByDefault(ReturnRef(strMethod));
        ON_CALL(objLocationProperties, GetCountry).WillByDefault(ReturnRef(strCountry));
        ON_CALL(objLocationProperties, GetState).WillByDefault(ReturnRef(strState));
        ON_CALL(objLocationProperties, GetCity).WillByDefault(ReturnRef(strCity));
        ON_CALL(objLocationProperties, GetPostal).WillByDefault(ReturnRef(strPostal));
        ON_CALL(objLocationProperties, GetAltitude).WillByDefault(ReturnRef(strAltitude));
        ON_CALL(objLocationProperties, GetVerticalAccuracy)
                .WillByDefault(ReturnRef(strVerticalAccuracy));
    }

    void SetUpUnknownLocationProperties()
    {
        ON_CALL(objLocationProperties, GetLatitude).WillByDefault(ReturnRef(UNKNOWN_POSITION));
        ON_CALL(objLocationProperties, GetLongitude).WillByDefault(ReturnRef(UNKNOWN_POSITION));
        ON_CALL(objLocationProperties, GetRadius).WillByDefault(ReturnRef(UNKNOWN_POSITION));
        ON_CALL(objLocationProperties, GetShape).WillByDefault(ReturnRef(AString::ConstEmpty()));
        ON_CALL(objLocationProperties, GetMethod).WillByDefault(ReturnRef(AString::ConstEmpty()));
        ON_CALL(objLocationProperties, GetState).WillByDefault(ReturnRef(AString::ConstEmpty()));
        ON_CALL(objLocationProperties, GetCity).WillByDefault(ReturnRef(AString::ConstEmpty()));
        ON_CALL(objLocationProperties, GetPostal).WillByDefault(ReturnRef(AString::ConstEmpty()));
        ON_CALL(objLocationProperties, GetAltitude).WillByDefault(ReturnRef(UNKNOWN_POSITION));
    }

    void AssertXmlStringEquality(IN AString strActual, IN AString strExpected)
    {
        EXPECT_STREQ(strActual.Replace("\n", "").GetStr(), strExpected.Replace("\n", "").GetStr());
    }
};

TEST_F(GeolocationPidfCreatorTest, CreateWithoutPositionFailsWhenCountryUnknown)
{
    const IMS_BOOL bAny = IMS_FALSE;
    ByteArray objContent;

    const AString strUnknownCountry = "ZZ";
    ON_CALL(objLocationProperties, GetCountry).WillByDefault(ReturnRef(strUnknownCountry));
    EXPECT_FALSE(pCreator->CreateWithoutPosition("entity_uri", IMS_FALSE, bAny, objContent));

    ON_CALL(objLocationProperties, GetCountry).WillByDefault(ReturnRef(AString::ConstEmpty()));
    EXPECT_FALSE(pCreator->CreateWithoutPosition("entity_uri", IMS_FALSE, bAny, objContent));
}

TEST_F(GeolocationPidfCreatorTest, CreateWithoutPositionFailsWhenTimestampUnknown)
{
    const IMS_BOOL bAny = IMS_FALSE;
    ByteArray objContent;

    ON_CALL(objLocationProperties, GetCurrentTime).WillByDefault(ReturnRef(AString::ConstEmpty()));
    EXPECT_FALSE(pCreator->CreateWithoutPosition("entity_uri", IMS_FALSE, bAny, objContent));
}

TEST_F(GeolocationPidfCreatorTest, CreateWithoutPosition)
{
    const IMS_BOOL bAny = IMS_FALSE;
    ByteArray objContent;
    ASSERT_TRUE(pCreator->CreateWithoutPosition("entity_uri", IMS_TRUE, bAny, objContent));

    const AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                                "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                                "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
                                "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
                                "entity=\"entity_uri\">"
                                "<dm:device id=\"Phone\">"
                                "<gp:geopriv>"
                                "<gp:location-info>"
                                "<cl:civicAddress>"
                                "<cl:country>country</cl:country>"
                                "</cl:civicAddress>"
                                "</gp:location-info>"
                                "<gp:usage-rules>"
                                "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
                                "</gp:usage-rules>"
                                "</gp:geopriv>"
                                "<dm:deviceID>device_id</dm:deviceID>"
                                "<dm:timestamp>time</dm:timestamp>"
                                "</dm:device>"
                                "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithoutPositionAsTuple)
{
    const IMS_BOOL bAny = IMS_FALSE;
    ByteArray objContent;
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_FORMAT_TUPLE);
    ASSERT_TRUE(pCreator->CreateWithoutPosition("entity_uri", IMS_TRUE, bAny, objContent));

    const AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                                "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                                "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
                                "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
                                "entity=\"entity_uri\">"
                                "<tuple id=\"VoLte\">"
                                "<status>"
                                "<gp:geopriv>"
                                "<gp:location-info>"
                                "<cl:civicAddress>"
                                "<cl:country>country</cl:country>"
                                "</cl:civicAddress>"
                                "</gp:location-info>"
                                "<gp:usage-rules>"
                                "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
                                "</gp:usage-rules>"
                                "</gp:geopriv>"
                                "</status>"
                                "<dm:timestamp>time</dm:timestamp>"
                                "</tuple>"
                                "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionFailsWhenPositionUnknown)
{
    ByteArray objContent;

    SetUpUnknownLocationProperties();
    EXPECT_FALSE(pCreator->CreateWithPosition("entity_uri", objContent));
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionWhenPositionUnknown)
{
    ByteArray objContent;

    SetUpUnknownLocationProperties();
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_ALLOW_NO_POSITION);
    ASSERT_TRUE(pCreator->CreateWithPosition("entity_uri", objContent));

    const AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                                "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                                "xmlns:gml=\"http://www.opengis.net/gml\" "
                                "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
                                "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
                                "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
                                "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
                                "entity=\"entity_uri\">"
                                "<dm:device id=\"Phone\">"
                                "<gp:geopriv>"
                                "<gp:location-info>"
                                "<cl:civicAddress>"
                                "<cl:country>country</cl:country>"
                                "</cl:civicAddress>"
                                "<con:confidence pdf=\"normal\">confidence</con:confidence>"
                                "</gp:location-info>"
                                "<gp:usage-rules>"
                                "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
                                "</gp:usage-rules>"
                                "</gp:geopriv>"
                                "<dm:deviceID>device_id</dm:deviceID>"
                                "<dm:timestamp>time</dm:timestamp>"
                                "</dm:device>"
                                "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionFailsWhenTimestampUnknown)
{
    ByteArray objContent;

    ON_CALL(objLocationProperties, GetCurrentTime).WillByDefault(ReturnRef(AString::ConstEmpty()));
    EXPECT_FALSE(pCreator->CreateWithPosition("entity_uri", objContent));
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionWithNoMethod)
{
    ByteArray objContent;
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_NO_METHOD);
    ASSERT_TRUE(pCreator->CreateWithPosition("entity_uri", objContent));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<cl:civicAddress>"
            "<cl:country>country</cl:country>"
            "<cl:A1>state</cl:A1>"
            "<cl:A2>city</cl:A2>"
            "<cl:PC>postal</cl:PC>"
            "</cl:civicAddress>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPosition)
{
    ByteArray objContent;
    ASSERT_TRUE(pCreator->CreateWithPosition("entity_uri", objContent));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<cl:civicAddress>"
            "<cl:country>country</cl:country>"
            "<cl:A1>state</cl:A1>"
            "<cl:A2>city</cl:A2>"
            "<cl:PC>postal</cl:PC>"
            "</cl:civicAddress>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionAsTuple)
{
    ByteArray objContent;
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_FORMAT_TUPLE);
    ASSERT_TRUE(pCreator->CreateWithPosition("entity_uri", objContent));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<tuple id=\"VoLte\">"
            "<status>"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<cl:civicAddress>"
            "<cl:country>country</cl:country>"
            "<cl:A1>state</cl:A1>"
            "<cl:A2>city</cl:A2>"
            "<cl:PC>postal</cl:PC>"
            "</cl:civicAddress>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "</status>"
            "<dm:timestamp>time</dm:timestamp>"
            "</tuple>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionWithNoUnknownCountry)
{
    ByteArray objContent;
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_NO_COUNTRY_IF_UNKNOWN);

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<cl:civicAddress>"
            "<cl:A1>state</cl:A1>"
            "<cl:A2>city</cl:A2>"
            "<cl:PC>postal</cl:PC>"
            "</cl:civicAddress>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";

    const AString strUnknownCountry = "ZZ";
    ON_CALL(objLocationProperties, GetCountry).WillByDefault(ReturnRef(strUnknownCountry));
    ASSERT_TRUE(pCreator->CreateWithPosition("entity_uri", objContent));
    AssertXmlStringEquality(objContent.ToString(), strExpected);

    ON_CALL(objLocationProperties, GetCountry).WillByDefault(ReturnRef(AString::ConstEmpty()));
    ASSERT_TRUE(pCreator->CreateWithPosition("entity_uri", objContent));
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionWithUnknownCountry)
{
    ByteArray objContent;

    const AString strUnknownCountry = "ZZ";
    ON_CALL(objLocationProperties, GetCountry).WillByDefault(ReturnRef(strUnknownCountry));
    ASSERT_TRUE(pCreator->CreateWithPosition("entity_uri", objContent));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<cl:civicAddress>"
            "<cl:country>ZZ</cl:country>"
            "<cl:A1>state</cl:A1>"
            "<cl:A2>city</cl:A2>"
            "<cl:PC>postal</cl:PC>"
            "</cl:civicAddress>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionWithConfidence)
{
    IMS_SINT32 nConfidence = 99;
    ByteArray objContent;
    ASSERT_TRUE(pCreator->CreateWithPosition("entity_uri", objContent, nConfidence));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<cl:civicAddress>"
            "<cl:country>country</cl:country>"
            "<cl:A1>state</cl:A1>"
            "<cl:A2>city</cl:A2>"
            "<cl:PC>postal</cl:PC>"
            "</cl:civicAddress>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">99</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionAndCountryFailsWhenPositionUnknown)
{
    ByteArray objContent;

    SetUpUnknownLocationProperties();
    EXPECT_FALSE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent));
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionAndCountryWhenPositionUnknown)
{
    ByteArray objContent;

    SetUpUnknownLocationProperties();
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_ALLOW_NO_POSITION);
    ASSERT_TRUE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent));

    const AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                                "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                                "xmlns:gml=\"http://www.opengis.net/gml\" "
                                "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
                                "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
                                "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
                                "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
                                "entity=\"entity_uri\">"
                                "<dm:device id=\"Phone\">"
                                "<gp:geopriv>"
                                "<gp:location-info>"
                                "<cl:civicAddress>"
                                "<cl:country>country</cl:country>"
                                "</cl:civicAddress>"
                                "<con:confidence pdf=\"normal\">confidence</con:confidence>"
                                "</gp:location-info>"
                                "<gp:usage-rules>"
                                "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
                                "</gp:usage-rules>"
                                "</gp:geopriv>"
                                "<dm:deviceID>device_id</dm:deviceID>"
                                "<dm:timestamp>time</dm:timestamp>"
                                "</dm:device>"
                                "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionAndCountryFailsWhenTimestampUnknown)
{
    ByteArray objContent;

    ON_CALL(objLocationProperties, GetCurrentTime).WillByDefault(ReturnRef(AString::ConstEmpty()));
    EXPECT_FALSE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent));
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionAndCountryWithNoMethod)
{
    ByteArray objContent;
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_NO_METHOD);
    ASSERT_TRUE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<cl:civicAddress>"
            "<cl:country>country</cl:country>"
            "</cl:civicAddress>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionAndCountry)
{
    ByteArray objContent;
    ASSERT_TRUE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<cl:civicAddress>"
            "<cl:country>country</cl:country>"
            "</cl:civicAddress>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionAndCountryAsTuple)
{
    ByteArray objContent;
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_FORMAT_TUPLE);
    ASSERT_TRUE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<tuple id=\"VoLte\">"
            "<status>"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<cl:civicAddress>"
            "<cl:country>country</cl:country>"
            "</cl:civicAddress>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "</status>"
            "<dm:timestamp>time</dm:timestamp>"
            "</tuple>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionAndCountryWithNoUnknownCountry)
{
    ByteArray objContent;
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_NO_COUNTRY_IF_UNKNOWN);

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";

    AString strUnknownCountry = "ZZ";
    ON_CALL(objLocationProperties, GetCountry).WillByDefault(ReturnRef(strUnknownCountry));
    ASSERT_TRUE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent));
    AssertXmlStringEquality(objContent.ToString(), strExpected);

    ON_CALL(objLocationProperties, GetCountry).WillByDefault(ReturnRef(AString::ConstEmpty()));
    ASSERT_TRUE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent));
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionAndCountryWithUnknownCountry)
{
    ByteArray objContent;

    AString strUnknownCountry = "ZZ";
    ON_CALL(objLocationProperties, GetCountry).WillByDefault(ReturnRef(strUnknownCountry));
    ASSERT_TRUE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<cl:civicAddress>"
            "<cl:country>ZZ</cl:country>"
            "</cl:civicAddress>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithPositionAndCountryWithConfidence)
{
    IMS_SINT32 nConfidence = 99;
    ByteArray objContent;
    ASSERT_TRUE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent, nConfidence));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<cl:civicAddress>"
            "<cl:country>country</cl:country>"
            "</cl:civicAddress>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">99</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithoutCivicFailsWhenPositionUnknown)
{
    ByteArray objContent;

    SetUpUnknownLocationProperties();
    EXPECT_FALSE(pCreator->CreateWithoutCivic("entity_uri", objContent));
}

TEST_F(GeolocationPidfCreatorTest, CreateWithoutCivicWhenPositionUnknown)
{
    ByteArray objContent;

    SetUpUnknownLocationProperties();
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_ALLOW_NO_POSITION);
    ASSERT_TRUE(pCreator->CreateWithoutCivic("entity_uri", objContent));

    const AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                                "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                                "xmlns:gml=\"http://www.opengis.net/gml\" "
                                "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
                                "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
                                "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
                                "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
                                "entity=\"entity_uri\">"
                                "<dm:device id=\"Phone\">"
                                "<gp:geopriv>"
                                "<gp:location-info>"
                                "<con:confidence pdf=\"normal\">confidence</con:confidence>"
                                "</gp:location-info>"
                                "<gp:usage-rules>"
                                "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
                                "</gp:usage-rules>"
                                "</gp:geopriv>"
                                "<dm:deviceID>device_id</dm:deviceID>"
                                "<dm:timestamp>time</dm:timestamp>"
                                "</dm:device>"
                                "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithoutCivicFailsWhenTimestampUnknown)
{
    ByteArray objContent;

    ON_CALL(objLocationProperties, GetCurrentTime).WillByDefault(ReturnRef(AString::ConstEmpty()));
    EXPECT_FALSE(pCreator->CreateWithoutCivic("entity_uri", objContent));
}

TEST_F(GeolocationPidfCreatorTest, CreateWithoutCivicWithNoMethod)
{
    ByteArray objContent;
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_NO_METHOD);
    ASSERT_TRUE(pCreator->CreateWithoutCivic("entity_uri", objContent));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithoutCivic)
{
    ByteArray objContent;
    ASSERT_TRUE(pCreator->CreateWithoutCivic("entity_uri", objContent));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithoutCivicAsTuple)
{
    ByteArray objContent;
    pCreator->SetFeatures(GeolocationPidfCreator::FEATURE_FORMAT_TUPLE);
    ASSERT_TRUE(pCreator->CreateWithoutCivic("entity_uri", objContent));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<tuple id=\"VoLte\">"
            "<status>"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">confidence</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "</status>"
            "<dm:timestamp>time</dm:timestamp>"
            "</tuple>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateWithoutCivicWithConfidence)
{
    IMS_SINT32 nConfidence = 99;
    ByteArray objContent;
    ASSERT_TRUE(pCreator->CreateWithoutCivic("entity_uri", objContent, nConfidence));

    const AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"entity_uri\">"
            "<dm:device id=\"Phone\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<gs:Ellipsoid srsName=\"urn:ogc:def:crs:EPSG::4979\">"
            "<gml:pos>lat long alt</gml:pos>"
            "<gs:semiMajorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMajorAxis>"
            "<gs:semiMinorAxis uom=\"urn:ogc:def:uom:EPSG::9001\">radius</gs:semiMinorAxis>"
            "<gs:verticalAxis uom=\"urn:ogc:def:uom:EPSG::9001\">v_accuracy</gs:verticalAxis>"
            "<gs:orientation uom=\"urn:ogc:def:uom:EPSG::9102\">0</gs:orientation>"
            "</gs:Ellipsoid>"
            "<con:confidence pdf=\"normal\">99</con:confidence>"
            "</gp:location-info>"
            "<gp:usage-rules>"
            "<gbp:retransmission-allowed>allowed</gbp:retransmission-allowed>"
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:deviceID>device_id</dm:deviceID>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertXmlStringEquality(objContent.ToString(), strExpected);
}

TEST_F(GeolocationPidfCreatorTest, CreateFailsWhenLocationInfoIsNull)
{
    objPhoneInfoService.SetLocationInfo(IMS_NULL);
    ByteArray objContent;

    EXPECT_FALSE(pCreator->CreateWithoutPosition("entity_uri", IMS_TRUE, IMS_FALSE, objContent));
    EXPECT_FALSE(pCreator->CreateWithPosition("entity_uri", objContent));
    EXPECT_FALSE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent));
    EXPECT_FALSE(pCreator->CreateWithoutCivic("entity_uri", objContent));
}

TEST_F(GeolocationPidfCreatorTest, CreateFailsWhenLocationPropertiesIsNull)
{
    ON_CALL(objPhoneInfoService.GetMockLocationInfo(), GetLocationProperties(_))
            .WillByDefault(Return(IMS_NULL));
    ByteArray objContent;

    EXPECT_FALSE(pCreator->CreateWithoutPosition("entity_uri", IMS_TRUE, IMS_FALSE, objContent));
    EXPECT_FALSE(pCreator->CreateWithPosition("entity_uri", objContent));
    EXPECT_FALSE(pCreator->CreateWithPositionAndCountry("entity_uri", objContent));
    EXPECT_FALSE(pCreator->CreateWithoutCivic("entity_uri", objContent));
}
