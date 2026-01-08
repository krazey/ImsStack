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
#include "GeolocationHelper.h"
#include "IImsAosInfo.h"
#include "INetworkWatcher.h"
#include "ImsEventDef.h"
#include "MockIMessage.h"
#include "MockIMessageBodyPart.h"
#include "MockIMtcImsEventReceiver.h"
#include "MockIMtcService.h"
#include "MockIPhoneInfoLocation.h"
#include "MockIPhoneInfoSubscriber.h"
#include "MockISubscriberConfig.h"
#include "PlatformContext.h"
#include "SipHeaderName.h"
#include "TestPhoneInfoService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MtcLocationObject.h"
#include "helper/MtcSupplementaryService.h"
#include "private/ConfigurationManager.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::SafeMatcherCast;
using ::testing::SetArgReferee;
using ::testing::StrEq;

LOCAL const IMS_SINT32 SLOT_ID = 0;
LOCAL const AString HOME_DOMAIN = "homedomain";
LOCAL const AString PRIVATE_USER_ID = "prid";

class MtcLocationObjectTest : public ::testing::Test
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

    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockMtcConfigurationProxy objConfigurationProxy;
    CallInfo objCallInfo;
    ParticipantInfo* pParticipantInfo;
    MtcSupplementaryService* pSupplementaryService;
    TestPhoneInfoService objPhoneInfoService;
    MockILocationProperties objLocationProperties;
    MockISubscriberConfig objSubscriberConfig;
    MockIMessageUtils objMessageUtils;
    MockIMtcImsEventReceiver objImsEventReceiver;
    MockIMtcAosConnector objAosConnector;

protected:
    virtual void SetUp() override
    {
        pParticipantInfo = new ParticipantInfo(objContext);

        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetParticipantInfo).WillByDefault(ReturnRef(*pParticipantInfo));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetSubscriberConfig).WillByDefault(Return(&objSubscriberConfig));
        ON_CALL(objContext, GetImsEventReceiver).WillByDefault(ReturnRef(objImsEventReceiver));
        ConfigurationManager::GetInstance()->DestroyConfigs();

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

        pSupplementaryService = new MtcSupplementaryService(objContext, objConfigurationProxy);
        ON_CALL(objContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));

        ON_CALL(objSubscriberConfig, GetHomeDomainName).WillByDefault(ReturnRef(HOME_DOMAIN));
        ON_CALL(objSubscriberConfig, GetPrivateUserId).WillByDefault(ReturnRef(PRIVATE_USER_ID));

        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
        ON_CALL(objAosConnector, GetRegistrationMode)
                .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        delete pParticipantInfo;
        delete pSupplementaryService;
    }

    void SetupDeviceLocation()
    {
        ON_CALL(objPhoneInfoService.GetMockLocationInfo(), GetLocationProperties(_))
                .WillByDefault(Return(&objLocationProperties));

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

    void AddGeolocationValue(IN IMS_BOOL bGeoLocation)
    {
        pSupplementaryService->Add(SuppType::GEOLOCATION, bGeoLocation);
    }

    void AssertPidfLoXmlExceptDeviceId(IN const AString& strActual, IN const AString& strExpected)
    {
        AString strActualExceptDeviceId = strActual;
        strActualExceptDeviceId.Replace("\n", "");

        // dm:deviceId is not consistent, don't compare.
        IMS_SINT32 nStart = strActualExceptDeviceId.GetIndexOf("<dm:deviceID>");
        IMS_SINT32 nEnd = strActualExceptDeviceId.GetIndexOf("</dm:deviceID>") + 14;
        strActualExceptDeviceId.Erase(nStart, nEnd - nStart);

        EXPECT_STREQ(strActualExceptDeviceId.GetStr(), strExpected.GetStr());
    }
};

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsFalseIfNoUiccAndNotAllowedByPlmn)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(IMS_TRUE));
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objConfigurationProxy,
            Contains(ConfigEmergency::
                             KEY_PLMN_ALLOWING_GEOLOCATION_PIDF_IN_SIP_INVITE_NO_UICC_STRING_ARRAY,
                    SafeMatcherCast<const IMS_CHAR*>(StrEq("00101"))))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetworkOperator)
            .WillByDefault(Return(AString("00101")));
    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NOUICC));

    EXPECT_FALSE(MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsTrueIfNoUiccAndPlmnIsEmpty)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(IMS_TRUE));
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objConfigurationProxy,
            Contains(ConfigEmergency::
                             KEY_PLMN_ALLOWING_GEOLOCATION_PIDF_IN_SIP_INVITE_NO_UICC_STRING_ARRAY,
                    SafeMatcherCast<const IMS_CHAR*>(StrEq("00101"))))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetworkOperator)
            .WillByDefault(Return(AString("")));
    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NOUICC));

    EXPECT_TRUE(MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsTrueIfNoUiccAndAllowedByPlmn)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(IMS_TRUE));
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    ON_CALL(objConfigurationProxy,
            Contains(ConfigEmergency::
                             KEY_PLMN_ALLOWING_GEOLOCATION_PIDF_IN_SIP_INVITE_NO_UICC_STRING_ARRAY,
                    SafeMatcherCast<const IMS_CHAR*>(StrEq("00101"))))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetworkOperator)
            .WillByDefault(Return(AString("00101")));
    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NOUICC));

    EXPECT_TRUE(MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForWifiNormal)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
            .WillByDefault(Return(IMS_TRUE));

    objCallInfo.eEmergencyType = EmergencyType::NONE;

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(MtcLocationObject::IsGeolocationInfoRequired(objContext));

    AddGeolocationValue(IMS_TRUE);

    EXPECT_TRUE(MtcLocationObject::IsGeolocationInfoRequired(objContext));

    AddGeolocationValue(IMS_FALSE);

    EXPECT_FALSE(MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForWifiEmergency)
{
    const IMS_BOOL bConfig = IMS_TRUE;
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI))
            .WillByDefault(Return(bConfig));

    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(bConfig, MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForCellularNormal)
{
    const IMS_BOOL bConfig = IMS_TRUE;
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(bConfig));

    objCallInfo.eEmergencyType = EmergencyType::NONE;

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(bConfig, MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsConfigForCellularEmergency)
{
    const IMS_BOOL bConfig = IMS_TRUE;
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(bConfig));

    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    EXPECT_EQ(bConfig, MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredReturnsFalseIfBlockedByInRoamingCondition)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
            .WillByDefault(Return(IMS_TRUE));
    objCallInfo.eEmergencyType = EmergencyType::NONE;
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY,
                    ConfigVoice::GEOLOCATION_BLOCK_CONDITION_IN_ROAMING))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_ROAMING_STATE))
            .WillByDefault(Return(IMS_ROAMING_STATE_ON));

    EXPECT_FALSE(MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredBehavesAsNormalCallIfAosConnectorIsNull)
{
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI))
            .WillByDefault(Return(IMS_TRUE));
    objCallInfo.eEmergencyType = EmergencyType::NONE;
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    ON_CALL(objService, GetAosConnector).WillByDefault(Return(IMS_NULL));

    EXPECT_TRUE(MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredForNormalRoutingEmergencyCall)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_NORMAL_ROUTING_EMERGENCY_ON_WIFI))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));
    AddGeolocationValue(IMS_TRUE);

    objCallInfo.eEmergencyType = EmergencyType::NORMAL_ROUTING;
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY,
                    ConfigVoice::GEOLOCATION_BLOCK_CONDITION_FOR_NORMAL_ROUTING_EMERGENCY_CALL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_TRUE(MtcLocationObject::IsGeolocationInfoRequired(objContext));

    objCallInfo.eEmergencyType = EmergencyType::NORMAL_ROUTING;
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY,
                    ConfigVoice::GEOLOCATION_BLOCK_CONDITION_FOR_NORMAL_ROUTING_EMERGENCY_CALL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_FALSE(MtcLocationObject::IsGeolocationInfoRequired(objContext));

    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY,
                    ConfigVoice::GEOLOCATION_BLOCK_CONDITION_FOR_NORMAL_ROUTING_EMERGENCY_CALL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_TRUE(MtcLocationObject::IsGeolocationInfoRequired(objContext));

    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY,
                    ConfigVoice::GEOLOCATION_BLOCK_CONDITION_FOR_NORMAL_ROUTING_EMERGENCY_CALL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(MtcLocationObject::IsGeolocationInfoRequired(objContext));
}

TEST_F(MtcLocationObjectTest, IsGeolocationInfoRequiredForNormalRoutingEmergencyCallOnCellular)
{
    // GIVEN: The device is on a cellular network
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    // GIVEN: Geolocation is allowed for normal routing emergency calls on cellular
    ON_CALL(objConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    ConfigIms::GEOLOCATION_PIDF_FOR_NORMAL_ROUTING_EMERGENCY_ON_CELLULAR))
            .WillByDefault(Return(IMS_TRUE));

    // GIVEN: Other conditions are met (not roaming, SS enabled, etc.)
    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));
    AddGeolocationValue(IMS_TRUE);

    // GIVEN: The call is a normal routing emergency call
    objCallInfo.eEmergencyType = EmergencyType::NORMAL_ROUTING;

    // WHEN: The block condition for normal routing emergency calls is NOT set
    // THEN: Geolocation info is required
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY,
                    ConfigVoice::GEOLOCATION_BLOCK_CONDITION_FOR_NORMAL_ROUTING_EMERGENCY_CALL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_TRUE(MtcLocationObject::IsGeolocationInfoRequired(objContext));

    // WHEN: The block condition for normal routing emergency calls IS set
    // THEN: Geolocation info is NOT required
    ON_CALL(objConfigurationProxy,
            Contains(ConfigVoice::KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY,
                    ConfigVoice::GEOLOCATION_BLOCK_CONDITION_FOR_NORMAL_ROUTING_EMERGENCY_CALL))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_FALSE(MtcLocationObject::IsGeolocationInfoRequired(objContext));
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
                                 "<gp:usage-rules>"
                                 "</gp:usage-rules>"
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
                                 "<gp:usage-rules>"
                                 "</gp:usage-rules>"
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
    EXPECT_STREQ(pLocation->GetLatitude().GetStr(), "-34.407");
    EXPECT_STREQ(pLocation->GetLongitude().GetStr(), "150.883");
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
            "<gp:usage-rules>"
            "</gp:usage-rules>"
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
    EXPECT_STREQ(pLocation->GetLatitude().GetStr(), "47.577866");
    EXPECT_STREQ(pLocation->GetLongitude().GetStr(), "-122.164080");
    EXPECT_STREQ(pLocation->GetRadius().GetStr(), "30");
    delete pLocation;
}

TEST_F(MtcLocationObjectTest, SetLocationToMessageDoesNothingIfContentEmpty)
{
    ByteArray objEmptyContent;
    MockIMessage objMessage;

    EXPECT_CALL(objMessage, AddHeader(_, _)).Times(0);
    EXPECT_CALL(objMessage, GetMessage).Times(0);
    MtcLocationObject(objContext).SetLocationToMessage(objMessage, IMS_FALSE, objEmptyContent);
}

TEST_F(MtcLocationObjectTest, SetLocationToMessageSetHeadersAndBodyPartWithNoGeolocationRouting)
{
    const AString strCid = "c-i-d";
    ON_CALL(objMessageUtils, GenerateContentId(_)).WillByDefault(Return(strCid));
    ON_CALL(objConfigurationProxy, GetString(ConfigVoice::KEY_CONTENT_ID_FOR_GEOLOCATION_STRING))
            .WillByDefault(Return(AString::ConstEmpty()));

    ByteArray objContent("PIDF-LO XML Content");
    MockIMessage objMessage;
    MockIMessageBodyPart objBodyPart;
    ON_CALL(objMessage, CreateBodyPart).WillByDefault(Return(&objBodyPart));

    EXPECT_CALL(objMessage, AddHeader(AString("Geolocation"), AString("<cid:c-i-d>")));
    EXPECT_CALL(objMessage, AddHeader(AString("Geolocation-Routing"), AString("no")));

    EXPECT_CALL(objBodyPart, SetContent(objContent));
    EXPECT_CALL(objBodyPart, SetHeader(AString(SipHeaderName::CONTENT_LENGTH), AString("19")));
    EXPECT_CALL(objBodyPart, SetHeader(AString(SipHeaderName::CONTENT_ID), AString("<c-i-d>")));
    EXPECT_CALL(objBodyPart,
            SetHeader(AString(SipHeaderName::CONTENT_TYPE), AString("application/pidf+xml")));
    EXPECT_CALL(objBodyPart,
            SetHeader(AString(SipHeaderName::CONTENT_DISPOSITION),
                    AString("render;handling=optional")));

    MtcLocationObject(objContext).SetLocationToMessage(objMessage, IMS_FALSE, objContent);
}

TEST_F(MtcLocationObjectTest, SetLocationToMessageSetHeadersAndBodyPartWithGeolocationRouting)
{
    const AString strCid = "c-i-d";
    ON_CALL(objMessageUtils, GenerateContentId(_)).WillByDefault(Return(strCid));
    ON_CALL(objConfigurationProxy, GetString(ConfigVoice::KEY_CONTENT_ID_FOR_GEOLOCATION_STRING))
            .WillByDefault(Return(AString::ConstEmpty()));

    ByteArray objContent("PIDF-LO XML Content");
    MockIMessage objMessage;
    MockIMessageBodyPart objBodyPart;
    ON_CALL(objMessage, CreateBodyPart).WillByDefault(Return(&objBodyPart));

    EXPECT_CALL(objMessage, AddHeader(AString("Geolocation"), AString("<cid:c-i-d>")));
    EXPECT_CALL(objMessage, AddHeader(AString("Geolocation-Routing"), AString("yes")));

    EXPECT_CALL(objBodyPart, SetContent(objContent));
    EXPECT_CALL(objBodyPart, SetHeader(AString(SipHeaderName::CONTENT_LENGTH), AString("19")));
    EXPECT_CALL(objBodyPart, SetHeader(AString(SipHeaderName::CONTENT_ID), AString("<c-i-d>")));
    EXPECT_CALL(objBodyPart,
            SetHeader(AString(SipHeaderName::CONTENT_TYPE), AString("application/pidf+xml")));
    EXPECT_CALL(objBodyPart,
            SetHeader(AString(SipHeaderName::CONTENT_DISPOSITION),
                    AString("render;handling=optional")));

    MtcLocationObject(objContext).SetLocationToMessage(objMessage, IMS_TRUE, objContent);
}

TEST_F(MtcLocationObjectTest, CreateLocationBodyReturnsEmptyIfNoCreator)
{
    EXPECT_STREQ(MtcLocationObject(objContext)
                         .CreateLocationBody()
                         .ToString()
                         .Replace("\n", "")
                         .GetStr(),
            "");
}

TEST_F(MtcLocationObjectTest, CreateLocationBodyReturnsLocationWithLatAndLong)
{
    GeolocationHelper::GetInstance()->CreatePidfCreator(SLOT_ID);
    ON_CALL(objConfigurationProxy,
            GetIntFromArray(ConfigIms::KEY_INFORMATION_LEVEL_OF_GEOLOCATION_PIDF_INT_ARRAY, _))
            .WillByDefault(Return(ConfigIms::GEOLOCATION_PIDF_INFO_LAT_AND_LONG));
    SetupDeviceLocation();

    AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"\">"
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
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertPidfLoXmlExceptDeviceId(
            MtcLocationObject(objContext).CreateLocationBody().ToString(), strExpected);
}

TEST_F(MtcLocationObjectTest, CreateLocationBodyReturnsLocationWithLatAndLongAndCivic)
{
    GeolocationHelper::GetInstance()->CreatePidfCreator(SLOT_ID);
    ON_CALL(objConfigurationProxy,
            GetIntFromArray(ConfigIms::KEY_INFORMATION_LEVEL_OF_GEOLOCATION_PIDF_INT_ARRAY, _))
            .WillByDefault(Return(ConfigIms::GEOLOCATION_PIDF_INFO_LAT_AND_LONG_AND_CIVIC));
    SetupDeviceLocation();

    AString strExpected =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
            "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
            "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
            "xmlns:gml=\"http://www.opengis.net/gml\" "
            "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" "
            "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
            "xmlns:con=\"urn:ietf:params:xml:ns:geopriv:conf\" "
            "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
            "entity=\"\">"
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
            "</gp:usage-rules>"
            "<gp:method>method</gp:method>"
            "</gp:geopriv>"
            "<dm:timestamp>time</dm:timestamp>"
            "</dm:device>"
            "</presence>";
    AssertPidfLoXmlExceptDeviceId(
            MtcLocationObject(objContext).CreateLocationBody().ToString(), strExpected);
}

TEST_F(MtcLocationObjectTest, CreateLocationBodyReturnsLocationWithCountry)
{
    GeolocationHelper::GetInstance()->CreatePidfCreator(SLOT_ID);
    ON_CALL(objConfigurationProxy,
            GetIntFromArray(ConfigIms::KEY_INFORMATION_LEVEL_OF_GEOLOCATION_PIDF_INT_ARRAY, _))
            .WillByDefault(Return(ConfigIms::GEOLOCATION_PIDF_INFO_COUNTRY_CODE_ONLY));
    SetupDeviceLocation();

    AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                          "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                          "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                          "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                          "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
                          "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
                          "entity=\"\">"
                          "<dm:device id=\"Phone\">"
                          "<gp:geopriv>"
                          "<gp:location-info>"
                          "<cl:civicAddress>"
                          "<cl:country>country</cl:country>"
                          "</cl:civicAddress>"
                          "</gp:location-info>"
                          "<gp:usage-rules>"
                          "</gp:usage-rules>"
                          "</gp:geopriv>"
                          "<dm:timestamp>time</dm:timestamp>"
                          "</dm:device>"
                          "</presence>";
    AssertPidfLoXmlExceptDeviceId(
            MtcLocationObject(objContext).CreateLocationBody().ToString(), strExpected);
}

TEST_F(MtcLocationObjectTest, CreateLocationBodyReturnsLocationWithCountryAndState)
{
    ConfigurationManager::GetInstance()->RefreshConfigs(objContext.GetSlotId());
    GeolocationHelper::GetInstance()->CreatePidfCreator(SLOT_ID);
    ON_CALL(objConfigurationProxy,
            GetIntFromArray(ConfigIms::KEY_INFORMATION_LEVEL_OF_GEOLOCATION_PIDF_INT_ARRAY, _))
            .WillByDefault(Return(ConfigIms::GEOLOCATION_PIDF_INFO_COUNTRY_CODE_AND_STATE));
    SetupDeviceLocation();

    AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                          "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                          "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                          "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                          "xmlns:cl=\"urn:ietf:params:xml:ns:pidf:geopriv10:civicAddr\" "
                          "xmlns:gbp=\"urn:ietf:params:xml:ns:pidf:geopriv10:basicPolicy\" "
                          "entity=\"pres:\">"
                          "<dm:device id=\"Phone\">"
                          "<gp:geopriv>"
                          "<gp:location-info>"
                          "<cl:civicAddress>"
                          "<cl:country>country</cl:country>"
                          "<cl:A1>state</cl:A1>"
                          "</cl:civicAddress>"
                          "</gp:location-info>"
                          "<gp:usage-rules>"
                          "</gp:usage-rules>"
                          "</gp:geopriv>"
                          "<dm:timestamp>time</dm:timestamp>"
                          "</dm:device>"
                          "</presence>";
    AssertPidfLoXmlExceptDeviceId(
            MtcLocationObject(objContext).CreateLocationBody().ToString(), strExpected);
}

TEST_F(MtcLocationObjectTest, CreateCallComposerLocationBodyReturnsLocation)
{
    MockISubscriberInfo* pSubscriberInfo = new MockISubscriberInfo();
    objPhoneInfoService.SetSubscriberInfo(pSubscriberInfo);

    const AString strPhoneNumber = "1234";
    ON_CALL(*pSubscriberInfo, GetPhoneNumber(_))
            .WillByDefault(DoAll(SetArgReferee<0>(strPhoneNumber), Return(IMS_TRUE)));

    AString strActual =
            MtcLocationObject(objContext).CreateCallComposerLocationBody("lat", "long").ToString();
    AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                          "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                          "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                          "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                          "xmlns:gml=\"http://www.opengis.net/gml\" "
                          "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" entity=\"pres:prid\">"
                          "<dm:person id=\"1234\">"
                          "<gp:geopriv>"
                          "<gp:location-info>"
                          "<gs:Circle srsName=\"urn:ogc:def:crs:EPSG::4326\">"
                          "<gml:pos>lat long</gml:pos>"
                          "</gs:Circle>"
                          "</gp:location-info>"
                          "<gp:usage-rules>"
                          "</gp:usage-rules>"
                          "</gp:geopriv>"
                          "</dm:person>"
                          "</presence>";
    EXPECT_STREQ(strActual.Replace("\n", "").GetStr(), strExpected.GetStr());
}

TEST_F(MtcLocationObjectTest, CreateCallComposerLocationBodyReturnsLocationIfSubscriberInfoIsNull)
{
    objPhoneInfoService.SetSubscriberInfo(IMS_NULL);

    AString strActual =
            MtcLocationObject(objContext).CreateCallComposerLocationBody("lat", "long").ToString();
    AString strExpected = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                          "<presence xmlns=\"urn:ietf:params:xml:ns:pidf\" "
                          "xmlns:dm=\"urn:ietf:params:xml:ns:pidf:data-model\" "
                          "xmlns:gp=\"urn:ietf:params:xml:ns:pidf:geopriv10\" "
                          "xmlns:gml=\"http://www.opengis.net/gml\" "
                          "xmlns:gs=\"http://www.opengis.net/pidflo/1.0\" entity=\"pres:prid\">"
                          "<dm:person id=\"\">"
                          "<gp:geopriv>"
                          "<gp:location-info>"
                          "<gs:Circle srsName=\"urn:ogc:def:crs:EPSG::4326\">"
                          "<gml:pos>lat long</gml:pos>"
                          "</gs:Circle>"
                          "</gp:location-info>"
                          "<gp:usage-rules>"
                          "</gp:usage-rules>"
                          "</gp:geopriv>"
                          "</dm:person>"
                          "</presence>";
    EXPECT_STREQ(strActual.Replace("\n", "").GetStr(), strExpected.GetStr());
}
