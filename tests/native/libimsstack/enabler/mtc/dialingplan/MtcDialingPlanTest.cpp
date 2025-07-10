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

#include "MockIMtcService.h"
#include "MockINetworkConnection.h"
#include "MockIPhoneInfoSubscriber.h"
#include "MockISubscriberConfig.h"
#include "PlatformContext.h"
#include "ServiceNetworkPolicy.h"
#include "TestNetworkService.h"
#include "TestPhoneInfoService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/ImsIdentityProxy.h"
#include "dialingplan/MockImsIdentityProxy.h"
#include "dialingplan/MtcDialingPlan.h"
#include "helper/MockIMtcAosConnector.h"
#include <gtest/gtest.h>

namespace android
{

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;

class TestMtcDialingPlan : public MtcDialingPlan
{
public:
    inline TestMtcDialingPlan() :
            MtcDialingPlan()
    {
    }
    inline ~TestMtcDialingPlan() override {}

    inline void ReplaceImsIdentityProxy(IN ImsIdentityProxy* pProxy)
    {
        delete m_pIdentityProxy;
        m_pIdentityProxy = pProxy;
    }
};

class MtcDialingPlanTest : public ::testing::Test
{
public:
    TestMtcDialingPlan* pDialingPlan;
    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MockISubscriberInfo objSubscriberInfo;
    MockImsIdentityProxy* pIdentityProxy;
    MockIMtcAosConnector objAosConnector;
    MockINetworkConnection objNetworkConnection;
    MockISubscriberConfig objSubscriberConfig;
    TestPhoneInfoService objPhoneInfoService;
    TestNetworkService objNetworkService;
    CallInfo objCallInfo;

protected:
    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(0));
        ON_CALL(objContext, GetServiceByType).WillByDefault(Return(&objService));

        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        ON_CALL(objAosConnector, GetConnectionType).WillByDefault(Return(NetworkPolicy::APN_IMS));
        ON_CALL(objContext, GetSubscriberConfig).WillByDefault(Return(&objSubscriberConfig));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, &objNetworkService);
        objNetworkService.SetConnection(&objNetworkConnection);

        pDialingPlan = new TestMtcDialingPlan();
        pIdentityProxy = new MockImsIdentityProxy();
        pDialingPlan->ReplaceImsIdentityProxy(pIdentityProxy);
    }

    virtual void TearDown() override
    {
        delete pDialingPlan;
        delete pConfigurationProxy;
    }
};

TEST_F(MtcDialingPlanTest, GetToUriNormal)
{
    // TODO
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsConferenceUriFromConfig)
{
    objCallInfo.bConference = IMS_TRUE;

    AString strUri = "sip:some_conference_uri";
    ON_CALL(*pConfigurationProxy, GetString(ConfigVoice::KEY_CONFERENCE_FACTORY_URI_STRING))
            .WillByDefault(Return(strUri));

    EXPECT_EQ(strUri, pDialingPlan->GetToUri("", objContext));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsDefaultConferenceUriIfMncIs2Digit)
{
    objCallInfo.bConference = IMS_TRUE;

    ON_CALL(*pConfigurationProxy, GetString(ConfigVoice::KEY_CONFERENCE_FACTORY_URI_STRING))
            .WillByDefault(Return(AString::ConstEmpty()));

    const AString strMcc3("123");
    const AString strMnc2("56");
    const AString strMnc3("056");
    ON_CALL(objPhoneInfoService.GetMockSubscriberInfo(), GetSimMcc(_))
            .WillByDefault(Invoke(
                    [strMcc3](AString& strMcc)
                    {
                        strMcc = strMcc3;
                        return IMS_TRUE;
                    }));
    ON_CALL(objPhoneInfoService.GetMockSubscriberInfo(), GetSimMnc(_))
            .WillByDefault(Invoke(
                    [strMnc2](AString& strMnc)
                    {
                        strMnc = strMnc2;
                        return IMS_TRUE;
                    }));

    AString strUri("sip:mmtel@conf-factory.ims.mnc#MNC#.mcc#MCC#.3gppnetwork.org");
    strUri = strUri.Replace("#MCC#", strMcc3).Replace("#MNC#", strMnc3);

    EXPECT_EQ(strUri, pDialingPlan->GetToUri("", objContext));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsDefaultConferenceUriIfMncIs3Digit)
{
    objCallInfo.bConference = IMS_TRUE;
    ON_CALL(*pConfigurationProxy, GetString(ConfigVoice::KEY_CONFERENCE_FACTORY_URI_STRING))
            .WillByDefault(Return(AString::ConstEmpty()));

    const AString strMcc3("123");
    const AString strMnc2("56");
    const AString strMnc3("056");
    ON_CALL(objPhoneInfoService.GetMockSubscriberInfo(), GetSimMcc(_))
            .WillByDefault(Invoke(
                    [strMcc3](AString& strMcc)
                    {
                        strMcc = strMcc3;
                        return IMS_TRUE;
                    }));
    ON_CALL(objPhoneInfoService.GetMockSubscriberInfo(), GetSimMnc(_))
            .WillByDefault(Invoke(
                    [strMnc3](AString& strMnc)
                    {
                        strMnc = strMnc3;
                        return IMS_TRUE;
                    }));

    AString strUri("sip:mmtel@conf-factory.ims.mnc#MNC#.mcc#MCC#.3gppnetwork.org");
    strUri = strUri.Replace("#MCC#", strMcc3).Replace("#MNC#", strMnc3);

    EXPECT_EQ(strUri, pDialingPlan->GetToUri("", objContext));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsConferenceFactoryUriWithConvertingHomaDomainName)
{
    objCallInfo.bConference = IMS_TRUE;
    AString strUri("sip:anyUserPart@#HOME_DOMAIN#");
    ON_CALL(*pConfigurationProxy, GetString(ConfigVoice::KEY_CONFERENCE_FACTORY_URI_STRING))
            .WillByDefault(Return(strUri));
    AString strDomainName("any_domain_name");
    ON_CALL(objSubscriberConfig, GetHomeDomainName).WillByDefault(ReturnRef(strDomainName));

    strUri = strUri.Replace("#HOME_DOMAIN#", strDomainName);

    EXPECT_STREQ(strUri.GetStr(), pDialingPlan->GetToUri("", objContext).GetStr());
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsDefaultUriInSipUriScheme)
{
    objCallInfo.bConference = IMS_TRUE;
    AString strUriWithoutScheme("conf@ims.google.com");
    AString strUriWithScheme("sip:conf@ims.google.com");

    ON_CALL(*pConfigurationProxy, GetString(ConfigVoice::KEY_CONFERENCE_FACTORY_URI_STRING))
            .WillByDefault(Return(strUriWithoutScheme));

    EXPECT_EQ(strUriWithScheme, pDialingPlan->GetToUri("", objContext));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsInputValueIfItIsUriForm)
{
    AString strUriWithScheme("sip:1234@ims.google.com");

    EXPECT_EQ(strUriWithScheme, pDialingPlan->GetToUri(strUriWithScheme, objContext));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsDialStringFormatIfUssi)
{
    objCallInfo.bUssi = IMS_TRUE;
    AString strNumber("123");
    AString strExpectedUri("sip:123;phone-context=ims.google.com@ims.google.com;user=dialstring");
    ON_CALL(*pIdentityProxy, CreateSipUserIdWithDialString(strNumber, _, _))
            .WillByDefault(Return(strExpectedUri));
    EXPECT_STREQ(strExpectedUri.GetStr(), pDialingPlan->GetToUri(strNumber, objContext).GetStr());
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsDialNumberWithPhone)
{
    AString strNumber("+1123");
    AString strExpectedUri("sip:+1123;user=phone");
    ON_CALL(*pIdentityProxy, CreateSipUserId(strNumber, _, IMS_TRUE, _))
            .WillByDefault(Return(strExpectedUri));
    EXPECT_STREQ(strExpectedUri.GetStr(),
            pDialingPlan->GetToUri(strNumber, objContext, Scheme::SIP).GetStr());
}

}  // namespace android
