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

#include "FeatureCaps.h"
#include "IImsAosInfo.h"
#include "MockICoreService.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockIPhoneInfoCall.h"
#include "MockIPhoneInfoDevice.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "MtcDef.h"
#include "PlatformContext.h"
#include "ServiceNetworkPolicy.h"
#include "SipParameter.h"
#include "TestNetworkService.h"
#include "TestPhoneInfoService.h"
#include "call/MockIMtcCallContext.h"
#include "call/message/EmergencyMessageFormatter.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MtcSupplementaryService.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::Unused;

LOCAL const AString HEADER_P_EMERGENCY_INFO = "P-Emergency-Info";

namespace android
{

class EmergencyMessageFormatterTest : public ::testing::Test
{
public:
    EmergencyMessageFormatter* pFormatter;

    CallInfo objCallInfo;
    MockIMessage objMessage;
    MockISipMessage objSipMessage;
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockISession objSession;
    MockICoreService objCoreService;
    MockIMtcAosConnector objAosConnector;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MtcSupplementaryService* pSupplementaryService;
    MockIMessageUtils objMessageUtils;
    TestNetworkService objNetworkService;
    TestPhoneInfoService objPhoneInfoService;
    FeatureCaps* pFeatureCaps;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();
        pSupplementaryService = new MtcSupplementaryService(objContext, *pConfigurationProxy);
        pFeatureCaps = new FeatureCaps();

        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        ON_CALL(objContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetAosConnector).WillByDefault(Return(&objAosConnector));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
        ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(pFeatureCaps));
        ON_CALL(objCoreService, GetUserIdentities)
                .WillByDefault(ReturnRef(AStringArray::ConstNull()));
        ON_CALL(objAosConnector, GetRegistrationMode)
                .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));
        ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));
        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, &objNetworkService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);

        pFormatter = new EmergencyMessageFormatter(objContext, objSession);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_NETWORK, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        delete pFormatter;
        delete pConfigurationProxy;
        delete pSupplementaryService;
        delete pFeatureCaps;
    }
};

TEST_F(EmergencyMessageFormatterTest, FormStartMessageNormalCase)
{
    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, FormStartMessageFailureCase)
{
    ON_CALL(objSession, GetNextRequest).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(EmergencyMessageFormatterTest, FormStartMessageNotAddsPeiHeaderIfNotWifiCall)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, _, ISipHeader::UNKNOWN, HEADER_P_EMERGENCY_INFO))
            .Times(0);
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, FormStartMessageNotAddsPeiHeaderIfEmpty)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    ON_CALL(*pConfigurationProxy,
            GetString(ConfigEmergency::KEY_P_EMERGENCY_INFO_HEADER_IN_INVITE_STRING))
            .WillByDefault(Return(AString::ConstEmpty()));

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, _, ISipHeader::UNKNOWN, HEADER_P_EMERGENCY_INFO))
            .Times(0);
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, FormStartMessageAddsPeiHeaderWithPlainText)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    AString strPei = "PEI field value";
    ON_CALL(*pConfigurationProxy,
            GetString(ConfigEmergency::KEY_P_EMERGENCY_INFO_HEADER_IN_INVITE_STRING))
            .WillByDefault(Return(strPei));

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, strPei, ISipHeader::UNKNOWN, HEADER_P_EMERGENCY_INFO));
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, FormStartMessageAddsPeiHeaderWithAid)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    AString strPei = "IEEE-802.11b; I-wlan-node-id=#AID#";
    ON_CALL(*pConfigurationProxy,
            GetString(ConfigEmergency::KEY_P_EMERGENCY_INFO_HEADER_IN_INVITE_STRING))
            .WillByDefault(Return(strPei));

    ON_CALL(objPhoneInfoService.GetMockCallInfo(), GetWifiCallingAddressId)
            .WillByDefault(Return(AString("aid-123-456")));

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, AString("IEEE-802.11b; I-wlan-node-id=aid-123-456"),
                    ISipHeader::UNKNOWN, HEADER_P_EMERGENCY_INFO));
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, FormStartMessageAddsPeiHeaderWithImei)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    AString strPei = "WSS-Wi-Fi-KEY;generic-key=#IMEI#";
    ON_CALL(*pConfigurationProxy,
            GetString(ConfigEmergency::KEY_P_EMERGENCY_INFO_HEADER_IN_INVITE_STRING))
            .WillByDefault(Return(strPei));

    ON_CALL(objPhoneInfoService.GetMockDeviceInfo(), GetDeviceId(_, _))
            .WillByDefault(Invoke(
                    [](Unused, OUT AString& strImei)
                    {
                        strImei = "imei-123-456";
                        return IMS_TRUE;
                    }));

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, AString("WSS-Wi-Fi-KEY;generic-key=imei-123-456"),
                    ISipHeader::UNKNOWN, HEADER_P_EMERGENCY_INFO));
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, FormStartMessageAddsPeiHeaderWithMacAddress)
{
    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    AString strPei = "WSS-Wi-Fi-KEY;generic-key=#MAC#";
    ON_CALL(*pConfigurationProxy,
            GetString(ConfigEmergency::KEY_P_EMERGENCY_INFO_HEADER_IN_INVITE_STRING))
            .WillByDefault(Return(strPei));

    ON_CALL(objNetworkService.GetMockConnection(), GetExtraInfo(AString("mac_address"), _))
            .WillByDefault(Invoke(
                    [](Unused, OUT AString& strInfo)
                    {
                        strInfo = "mac-123-456";
                        return IMS_TRUE;
                    }));

    ON_CALL(objService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, AString("WSS-Wi-Fi-KEY;generic-key=mac-123-456"),
                    ISipHeader::UNKNOWN, HEADER_P_EMERGENCY_INFO));
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, GetAoSRegMode)
{
    ON_CALL(objContext, GetAosConnector).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(EmergencyMessageFormatterTest, SetPPreferredIdentityHeader)
{
    ON_CALL(objSipMessage, IsHeaderPresent).WillByDefault(Return(IMS_TRUE));
    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objSipMessage, IsHeaderPresent).WillByDefault(Return(IMS_FALSE));
    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, SetPPreferredIdentityHeaderByDeviceId)
{
    const AString strLocalIpv6 = "::1";
    const AString strLocalIpv4 = "127.0.0.1";
    const IMS_UINT32 nLocalPort = 5060;

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));
    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(strLocalIpv6));
    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(nLocalPort));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(strLocalIpv4));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, GetLocalIpAddress)
{
    const AString strLocalIp = "::1";

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));

    EXPECT_CALL(objContext, GetAosConnector)
            .WillOnce(Return(&objAosConnector))
            .WillOnce(Return(&objAosConnector))
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&objAosConnector));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(AString::ConstEmpty()));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(strLocalIp));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, GetLocalPort)
{
    const AString strLocalIp = "::1";
    const IMS_UINT32 nLocalPort = 5060;
    const IMS_UINT32 nLocalPortZero = 0;

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));
    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(strLocalIp));

    EXPECT_CALL(objContext, GetAosConnector)
            .WillOnce(Return(&objAosConnector))
            .WillOnce(Return(&objAosConnector))
            .WillOnce(Return(&objAosConnector))
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&objAosConnector));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(nLocalPortZero));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(nLocalPort));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, SetPPreferredIdentityHeaderByUserId)
{
    const AString strUserIdentity = "sip:user@google.com";

    EXPECT_CALL(objService, GetICoreService)
            .WillOnce(Return(&objCoreService))
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&objCoreService));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    AStringArray objUserIdentities;
    objUserIdentities.AddElement(AString::ConstEmpty());
    objUserIdentities.AddElement(strUserIdentity);
    ON_CALL(objCoreService, GetUserIdentities).WillByDefault(ReturnRef(objUserIdentities));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, SetSipInstanceFeature)
{
    ON_CALL(objSipMessage, IsHeaderPresent).WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objService, GetICoreService)
            .WillOnce(Return(&objCoreService))
            .WillOnce(Return(&objCoreService))
            .WillOnce(Return(nullptr))
            .WillRepeatedly(Return(&objCoreService));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objCoreService, GetInstanceParameter).WillByDefault(Return(nullptr));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    SipParameter objParameter;
    ON_CALL(objCoreService, GetInstanceParameter).WillByDefault(Return(&objParameter));

    ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(nullptr));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(objCoreService, GetFeatureCaps).WillByDefault(Return(pFeatureCaps));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, SetCurrentLocationDiscovery)
{
    ON_CALL(*pConfigurationProxy,
            GetBoolean(
                    ConfigEmergency::KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_TRUE));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);

    ON_CALL(*pConfigurationProxy,
            GetBoolean(
                    ConfigEmergency::KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));

    nResult = pFormatter->FormStartMessage(CallType::VOIP);
    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, FormUpdateMessageSetsLocation)
{
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF_INT_ARRAY,
                    static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::INVITE)))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY,
                    static_cast<IMS_SINT32>(
                            ConfigIms::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objContext, GetSlotId).Times(1).WillOnce(Return(0));

    EXPECT_EQ(IMS_SUCCESS, pFormatter->FormUpdateMessage(UpdateType::LOCATION, IMS_FALSE));
}

}  // namespace android
