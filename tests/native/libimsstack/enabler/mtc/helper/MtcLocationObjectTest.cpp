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

#include "CarrierConfig.h"
#include "INetworkWatcher.h"
#include "MockIMessage.h"
#include "MockIMessageBodyPart.h"
#include "MockIMtcService.h"
#include "SipHeaderName.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcLocationObject.h"
#include "helper/MtcSupplementaryService.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class MtcLocationObjectTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    CallInfo objCallInfo;
    MtcSupplementaryService* pSupplementaryService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        pSupplementaryService = new MtcSupplementaryService(objContext, *pConfigurationProxy);
        ON_CALL(objContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
        delete pSupplementaryService;
    }

    void AddGeoLocationValue(IN IMS_BOOL bGeoLocation)
    {
        pSupplementaryService->Add(SuppType::GEOLOCATION, bGeoLocation);
    }
};

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsFalseIfAosConnectorIsNull)
{
    ON_CALL(objService, GetAosConnector()).WillByDefault(Return(nullptr));

    EXPECT_FALSE(MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForWifiNormal)
{
    ON_CALL(*pConfigurationManager,
            IsSupportGeolocationPidfInSipInvite(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
            .WillByDefault(Return(IMS_TRUE));

    objCallInfo.bEmergency = IMS_FALSE;

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(IMS_TRUE, MtcLocationObject::IsGeolocationInfoRequired(objContext));

    AddGeoLocationValue(IMS_TRUE);

    EXPECT_EQ(IMS_TRUE, MtcLocationObject::IsGeolocationInfoRequired(objContext));

    AddGeoLocationValue(IMS_FALSE);

    EXPECT_EQ(IMS_FALSE, MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForWifiEmergency)
{
    const IMS_BOOL bConfig = IMS_TRUE;
    ON_CALL(*pConfigurationManager,
            IsSupportGeolocationPidfInSipInvite(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI))
            .WillByDefault(Return(bConfig));

    objCallInfo.bEmergency = IMS_TRUE;

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(bConfig, MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForCellularNormal)
{
    const IMS_BOOL bConfig = IMS_TRUE;
    ON_CALL(*pConfigurationManager,
            IsSupportGeolocationPidfInSipInvite(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(bConfig));

    objCallInfo.bEmergency = IMS_FALSE;

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(bConfig, MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForCellularEmergency)
{
    const IMS_BOOL bConfig = IMS_TRUE;
    ON_CALL(*pConfigurationManager,
            IsSupportGeolocationPidfInSipInvite(
                    CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(bConfig));

    objCallInfo.bEmergency = IMS_TRUE;

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(bConfig, MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, GetLocationFromMessageIfNoContent)
{
    MockIMessageBodyPart objOtherBody;
    ON_CALL(objOtherBody, GetHeader(AString(SipHeaderName::CONTENT_TYPE)))
            .WillByDefault(Return(AString("not_location")));

    ImsList<IMessageBodyPart*> lstMessageBodies;
    lstMessageBodies.Append(&objOtherBody);

    MockIMessage objMessage;
    ON_CALL(objMessage, GetBodyParts).WillByDefault(Return(lstMessageBodies));

    EXPECT_EQ(MtcLocationObject::GetLocationFromMessage(objMessage), nullptr);
}

TEST_F(MtcLocationObjectTest, GetLocationFromMessageWithInvalidShape)
{
    MockIMessageBodyPart objLocationBody;
    ByteArray objLocationContent("<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                 "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                                 "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                                 "xmlns:gml=\"http://www.opengis.net/gml\" "
                                 "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
                                 "entity=\"tel:+491711234567\">"
                                 "<dm:person id=\"sh2204\">"
                                 "<gp:geopriv>"
                                 "<gp:location-info>"
                                 "<invalid-shape/>"  // Malformed
                                 "</gp:location-info>"
                                 "<gp:usage-rules/>"
                                 "</gp:geopriv>"
                                 "</dm:person>"
                                 "</presence>");

    ON_CALL(objLocationBody, GetHeader(AString(SipHeaderName::CONTENT_TYPE)))
            .WillByDefault(Return(AString("application/pidf+xml")));
    ON_CALL(objLocationBody, GetContent).WillByDefault(ReturnRef(objLocationContent));

    ImsList<IMessageBodyPart*> lstMessageBodies;
    lstMessageBodies.Append(&objLocationBody);

    MockIMessage objMessage;
    ON_CALL(objMessage, GetBodyParts).WillByDefault(Return(lstMessageBodies));

    EXPECT_EQ(MtcLocationObject::GetLocationFromMessage(objMessage), nullptr);
}

TEST_F(MtcLocationObjectTest, GetLocationFromMessageWithPointShape)
{
    MockIMessageBodyPart objOtherBody;
    ON_CALL(objOtherBody, GetHeader(AString(SipHeaderName::CONTENT_TYPE)))
            .WillByDefault(Return(AString("not_location")));

    MockIMessageBodyPart objLocationBody;
    ByteArray objLocationContent("<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                                 "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                                 "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                                 "xmlns:gml=\"http://www.opengis.net/gml\" "
                                 "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
                                 "entity=\"tel:+491711234567\">"
                                 "<dm:person id=\"sh2204\">"
                                 "<gp:geopriv>"
                                 "<gp:location-info>"
                                 "<gml:Point srsName=\"urn:ogc:def:crs:EPSG::4326\">"
                                 "<gml:pos>-34.407 150.883</gml:pos>"
                                 "</gml:Point>"
                                 "</gp:location-info>"
                                 "<gp:usage-rules/>"
                                 "</gp:geopriv>"
                                 "</dm:person>"
                                 "</presence>");

    ON_CALL(objLocationBody, GetHeader(AString(SipHeaderName::CONTENT_TYPE)))
            .WillByDefault(Return(AString("application/pidf+xml")));
    ON_CALL(objLocationBody, GetContent).WillByDefault(ReturnRef(objLocationContent));

    ImsList<IMessageBodyPart*> lstMessageBodies;
    lstMessageBodies.Append(&objOtherBody);
    lstMessageBodies.Append(&objLocationBody);

    MockIMessage objMessage;
    ON_CALL(objMessage, GetBodyParts).WillByDefault(Return(lstMessageBodies));

    MtcLocationProperties* pLocation = MtcLocationObject::GetLocationFromMessage(objMessage);
    ASSERT_NE(pLocation, nullptr);
    EXPECT_EQ(pLocation->GetLatitude(), AString("-34.407"));
    EXPECT_EQ(pLocation->GetLongitude(), AString("150.883"));
    EXPECT_EQ(pLocation->GetRadius().GetLength(), 0);
    delete pLocation;
}

TEST_F(MtcLocationObjectTest, GetLocationFromMessageWithCircleShape)
{
    MockIMessageBodyPart objOtherBody;
    ON_CALL(objOtherBody, GetHeader(AString(SipHeaderName::CONTENT_TYPE)))
            .WillByDefault(Return(AString("not_location")));

    MockIMessageBodyPart objLocationBody;
    ByteArray objLocationContent(  // From RCC.20
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "entity=\"tel:+491711234567\">"
            "<dm:person id=\"sh2204\">"
            "<gp:geopriv>"
            "<gp:location-info>"
            "<gs:Circle srsName=\"urn:ogc:def:crs:EPSG::4326\">"
            "<gml:pos>47.577866 -122.164080</gml:pos>"
            "<gs:radius uom=\"urn:ogc:def:uom:EPSG::9001\">30</gs:radius>"
            "</gs:Circle>"
            "</gp:location-info>"
            "<gp:usage-rules/>"
            "</gp:geopriv>"
            "</dm:person>"
            "</presence>");

    ON_CALL(objLocationBody, GetHeader(AString(SipHeaderName::CONTENT_TYPE)))
            .WillByDefault(Return(AString("application/pidf+xml")));
    ON_CALL(objLocationBody, GetContent).WillByDefault(ReturnRef(objLocationContent));

    ImsList<IMessageBodyPart*> lstMessageBodies;
    lstMessageBodies.Append(&objOtherBody);
    lstMessageBodies.Append(&objLocationBody);

    MockIMessage objMessage;
    ON_CALL(objMessage, GetBodyParts).WillByDefault(Return(lstMessageBodies));

    MtcLocationProperties* pLocation = MtcLocationObject::GetLocationFromMessage(objMessage);
    ASSERT_NE(pLocation, nullptr);
    EXPECT_EQ(pLocation->GetLatitude(), AString("47.577866"));
    EXPECT_EQ(pLocation->GetLongitude(), AString("-122.164080"));
    EXPECT_EQ(pLocation->GetRadius(), AString("30"));
    delete pLocation;
}

TEST_F(MtcLocationObjectTest, SetLocationToMessageDoesNothingIfContentEmpty)
{
    ByteArray objEmptyContent;
    MockIMessage objMessage;

    EXPECT_CALL(objMessage, AddHeader(_, _)).Times(0);
    EXPECT_CALL(objMessage, GetMessage).Times(0);
    MtcLocationObject(objContext).SetLocationToMessage(objMessage, objEmptyContent, IMS_FALSE);
}
