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
#include "ImsVector.h"
#include "MockICoreService.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockIPhoneInfoCall.h"
#include "MockIPhoneInfoDevice.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "MockISubscriberConfig.h"
#include "MtcDef.h"
#include "PlatformContext.h"
#include "ServiceNetworkPolicy.h"
#include "SipParameter.h"
#include "TestNetworkService.h"
#include "TestPhoneInfoService.h"
#include "call/MockIMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "call/message/EmergencyMessageFormatter.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MockIMtcDialingPlan.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MtcSupplementaryService.h"
#include "utility/MessageUtil.h"
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
    EmergencyMessageFormatterTest() :
            objParticipantInfo(objContext)
    {
    }

    EmergencyMessageFormatter* pFormatter;

    CallInfo objCallInfo;
    ParticipantInfo objParticipantInfo;
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
    MockISubscriberConfig objSubscriberConfig;
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
        ON_CALL(objContext, GetParticipantInfo).WillByDefault(ReturnRef(objParticipantInfo));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetAosConnector).WillByDefault(Return(&objAosConnector));
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetSubscriberConfig).WillByDefault(Return(&objSubscriberConfig));
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

        ImsVector<AString> lstPpi;
        lstPpi.Add("");
        lstPpi.Add("");
        lstPpi.Add("");
        lstPpi.Add("");
        ON_CALL(*pConfigurationProxy,
                GetStringArray(ConfigEmergency::
                                KEY_P_PREFERRED_IDENTITY_INFO_HEADER_IN_INVITE_STRING_ARRAY))
                .WillByDefault(Return(lstPpi));

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
    const ImsVector<AString> objEmptyNumbers;
    ON_CALL(*pConfigurationProxy, GetStringArray(ConfigEmergency::KEY_NUMBER_NEED_OIR_STRING_ARRAY))
            .WillByDefault(Return(objEmptyNumbers));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, FormStartMessageSetsOirIfConfigured)
{
    // Set ParticipantInfo
    MockIMtcDialingPlan objDialingPlan;
    ON_CALL(objContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(AString::ConstEmpty()));
    objParticipantInfo.UpdateFromRemoteNumber("184110");

    ImsVector<AString> objConfigNumbers;
    objConfigNumbers.Push("184");
    ON_CALL(*pConfigurationProxy, GetStringArray(ConfigEmergency::KEY_NUMBER_NEED_OIR_STRING_ARRAY))
            .WillByDefault(Return(objConfigNumbers));

    // To skip other 'SetHeader' invocations.
    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, _, _, AString::ConstNull())).Times(2);

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SESSION_PRIVACY_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::SESSION_PRIVACY_TYPE_ID));

    EXPECT_CALL(objMessageUtils,
            SetHeader(&objMessage, AString(MessageUtil::STR_ID), ISipHeader::PRIVACY,
                    AString::ConstNull()))
            .Times(1);
    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, _, ISipHeader::FROM, AString::ConstNull()))
            .Times(1);

    pFormatter->FormStartMessage(CallType::VOIP);
}

TEST_F(EmergencyMessageFormatterTest, FormStartMessageDoesNotSetOirIfConfiguredButNotContains)
{
    ImsVector<AString> objConfigNumbers;
    AString strNotContainedNumber("1111");
    objConfigNumbers.Push(strNotContainedNumber);
    ON_CALL(*pConfigurationProxy, GetStringArray(ConfigEmergency::KEY_NUMBER_NEED_OIR_STRING_ARRAY))
            .WillByDefault(Return(objConfigNumbers));

    // To skip other 'SetHeader' invocations.
    EXPECT_CALL(objMessageUtils, SetHeader(&objMessage, _, _, AString::ConstNull())).Times(2);

    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_SESSION_PRIVACY_TYPE_INT))
            .WillByDefault(Return(ConfigVoice::SESSION_PRIVACY_TYPE_ID));

    EXPECT_CALL(objMessageUtils,
            SetHeader(&objMessage, AString(MessageUtil::STR_ID), ISipHeader::PRIVACY,
                    AString::ConstNull()))
            .Times(0);

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

TEST_F(EmergencyMessageFormatterTest,
        FormStartMessageDoesNotAddPComServiceTypeHeaderIfNotConfigured)
{
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_COM_SERVICETYPE))
            .WillByDefault(Return(IMS_FALSE));

    ImsVector<AString> lstRules;
    ON_CALL(*pConfigurationProxy,
            GetStringArray(CarrierConfig::KEY_IWLAN_HANDOVER_POLICY_STRING_ARRAY))
            .WillByDefault(Return(lstRules));

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, AString("Static-Emergency"), ISipHeader::UNKNOWN,
                    AString("P-Com.ServiceType")))
            .Times(0);
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest,
        FormStartMessageDoesNotAddPComServiceTypeHeaderIfWifiToLteHandoverIsAllowed)
{
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_COM_SERVICETYPE))
            .WillByDefault(Return(IMS_TRUE));

    ImsVector<AString> lstRules;
    lstRules.Add("source=UNKNOWN, target=EUTRAN|GERAN, type=allowed, capabilities=EIMS|XCAP");
    lstRules.Add("source=GERAN|IWLAN|UNKNOWN, target=EUTRAN|GERAN, type=allowed, "
                 "capabilities=EIMS|XCAP");
    ON_CALL(*pConfigurationProxy,
            GetStringArray(CarrierConfig::KEY_IWLAN_HANDOVER_POLICY_STRING_ARRAY))
            .WillByDefault(Return(lstRules));

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, AString("Static-Emergency"), ISipHeader::UNKNOWN,
                    AString("P-Com.ServiceType")))
            .Times(0);
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest,
        FormStartMessageAddsPComServiceTypeHeaderIfWifiToLteHandoverIsDisallowed)
{
    ON_CALL(*pConfigurationProxy,
            Contains(ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                    MessageUtil::STR_P_COM_SERVICETYPE))
            .WillByDefault(Return(IMS_TRUE));

    ImsVector<AString> lstRules;
    lstRules.Add(
            "source=GERAN|UTRAN|EUTRAN|NGRAN|IWLAN|UNKNOWN, target=GERAN|UTRAN|EUTRAN|NGRAN|IWLAN, "
            "roaming=true, type=disallowed, capabilities=IMS|EIMS|MMS|XCAP|CBS");
    lstRules.Add("source=IWLAN|UNKNOWN, target=GERAN|UTRAN, type=disallowed, "
                 "capabilities=IMS|EIMS|MMS|XCAP|CBS");
    lstRules.Add("source=GERAN|UTRAN, target=IWLAN, type=disallowed, "
                 "capabilities=IMS|EIMS|MMS|XCAP|CBS");
    lstRules.Add("source=EUTRAN|NGRAN|IWLAN|UNKNOWN, target=EUTRAN|NGRAN|IWLAN, type=disallowed, "
                 "capabilities=EIMS");
    lstRules.Add("source=EUTRAN|NGRAN|IWLAN, target=EUTRAN|NGRAN|IWLAN, type=allowed, "
                 "capabilities=IMS|MMS|XCAP|CBS");

    ON_CALL(*pConfigurationProxy,
            GetStringArray(CarrierConfig::KEY_IWLAN_HANDOVER_POLICY_STRING_ARRAY))
            .WillByDefault(Return(lstRules));

    EXPECT_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, AString("Static-Emergency"), ISipHeader::UNKNOWN,
                    AString("P-Com.ServiceType")));
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, GetAoSRegMode)
{
    ON_CALL(objContext, GetAosConnector).WillByDefault(Return(nullptr));

    IMS_RESULT nResult = pFormatter->FormStartMessage(CallType::VOIP);

    EXPECT_EQ(nResult, IMS_FAILURE);
}

TEST_F(EmergencyMessageFormatterTest, SetPPreferredIdentityHeaderDoesNotSetIfAlreadyPresent)
{
    ON_CALL(objMessageUtils, IsHeaderPresent(_, ISipHeader::P_PREFERRED_IDENTITY, _))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_CALL(objMessageUtils, SetHeader(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils, SetHeader(_, _, ISipHeader::P_PREFERRED_IDENTITY, _)).Times(0);
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, SetPPreferredIdentityHeaderFormatsForRegModes)
{
    ON_CALL(objMessageUtils, IsHeaderPresent(_, ISipHeader::P_PREFERRED_IDENTITY, _))
            .WillByDefault(Return(IMS_FALSE));
    ImsVector<AString> lstPpi;
    lstPpi.Add("normal@ppi");
    lstPpi.Add("admin@ppi");
    lstPpi.Add("internal@ppi");
    lstPpi.Add("nouicc@ppi");
    ON_CALL(*pConfigurationProxy,
            GetStringArray(
                    ConfigEmergency::KEY_P_PREFERRED_IDENTITY_INFO_HEADER_IN_INVITE_STRING_ARRAY))
            .WillByDefault(Return(lstPpi));
    AString strPuid = "puid";
    ON_CALL(objSubscriberConfig, GetPublicUserId(_)).WillByDefault(ReturnRef(strPuid));
    ON_CALL(objPhoneInfoService.GetMockDeviceInfo(), GetDeviceId(_, _))
            .WillByDefault(Invoke(
                    [](Unused, OUT AString& strImei)
                    {
                        strImei = "imei";
                        return IMS_TRUE;
                    }));
    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return("ip"));
    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(5060));
    EXPECT_CALL(objMessageUtils, SetHeader(_, _, _, _)).Times(AnyNumber());

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));
    EXPECT_CALL(objMessageUtils,
            SetHeader(_, AString("normal@ppi"), ISipHeader::P_PREFERRED_IDENTITY, _));
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_ADMIN));
    EXPECT_CALL(objMessageUtils,
            SetHeader(_, AString("admin@ppi"), ISipHeader::P_PREFERRED_IDENTITY, _));
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));
    EXPECT_CALL(objMessageUtils,
            SetHeader(_, AString("internal@ppi"), ISipHeader::P_PREFERRED_IDENTITY, _));
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NOUICC));
    EXPECT_CALL(objMessageUtils,
            SetHeader(_, AString("nouicc@ppi"), ISipHeader::P_PREFERRED_IDENTITY, _));
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
}

TEST_F(EmergencyMessageFormatterTest, SetPPreferredIdentityHeaderFormatsByTokens)
{
    ON_CALL(objMessageUtils, IsHeaderPresent(_, ISipHeader::P_PREFERRED_IDENTITY, _))
            .WillByDefault(Return(IMS_FALSE));
    ImsVector<AString> lstPpi;
    lstPpi.Add("#PUID#-#IMEI#-#IP#-#PORT#");
    ON_CALL(*pConfigurationProxy,
            GetStringArray(
                    ConfigEmergency::KEY_P_PREFERRED_IDENTITY_INFO_HEADER_IN_INVITE_STRING_ARRAY))
            .WillByDefault(Return(lstPpi));
    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));

    AString strPuid = "puid";
    ON_CALL(objSubscriberConfig, GetPublicUserId(_)).WillByDefault(ReturnRef(strPuid));
    ON_CALL(objPhoneInfoService.GetMockDeviceInfo(), GetDeviceId(_, _))
            .WillByDefault(Invoke(
                    [](Unused, OUT AString& strImei)
                    {
                        strImei = "imei";
                        return IMS_TRUE;
                    }));
    ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return("ip"));
    ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(5060));

    EXPECT_CALL(objMessageUtils, SetHeader(_, _, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMessageUtils,
            SetHeader(_, AString("puid-imei-ip-5060"), ISipHeader::P_PREFERRED_IDENTITY, _));
    EXPECT_EQ(pFormatter->FormStartMessage(CallType::VOIP), IMS_SUCCESS);
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
