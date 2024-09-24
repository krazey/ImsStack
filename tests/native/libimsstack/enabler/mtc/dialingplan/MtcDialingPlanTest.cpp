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

#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockINetworkConnection.h"
#include "MockIPhoneInfoSubscriber.h"
#include "PlatformContext.h"
#include "ServiceNetworkPolicy.h"
#include "TestNetworkService.h"
#include "call/IMtcCall.h"
#include "configuration/MockIMtcConfigurationManager.h"
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
    inline explicit TestMtcDialingPlan(
            IN IMtcContext& objContext, IN ISubscriberInfo& objSubscriberInfo) :
            MtcDialingPlan(objContext, objSubscriberInfo)
    {
    }
    inline ~TestMtcDialingPlan() {}

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
    MockIMtcContext objContext;
    MockIMtcService objService;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockISubscriberInfo objSubscriberInfo;
    MockImsIdentityProxy* pIdentityProxy;
    MockIMtcAosConnector objAosConnector;
    MockINetworkConnection objNetworkConnection;
    TestNetworkService objNetworkService;
    CallInfo objCallInfo;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(0));
        ON_CALL(objContext, GetServiceByType).WillByDefault(Return(&objService));

        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));

        ON_CALL(objAosConnector, GetConnectionType).WillByDefault(Return(NetworkPolicy::APN_IMS));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, &objNetworkService);
        objNetworkService.SetConnection(&objNetworkConnection);

        pDialingPlan = new TestMtcDialingPlan(objContext, objSubscriberInfo);
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
    ON_CALL(*pConfigurationManager, GetConferenceFactoryUri).WillByDefault(Return(strUri));

    EXPECT_EQ(strUri, pDialingPlan->GetToUri("", objCallInfo));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsDefaultConferenceUriIfMncIs2Digit)
{
    objCallInfo.bConference = IMS_TRUE;

    ON_CALL(*pConfigurationManager, GetConferenceFactoryUri)
            .WillByDefault(Return(AString::ConstEmpty()));

    const AString strMcc3("123");
    const AString strMnc2("56");
    const AString strMnc3("056");
    ON_CALL(objSubscriberInfo, GetSimMcc)
            .WillByDefault(Invoke(
                    [strMcc3](AString& strMcc)
                    {
                        strMcc = strMcc3;
                        return IMS_TRUE;
                    }));
    ON_CALL(objSubscriberInfo, GetSimMnc)
            .WillByDefault(Invoke(
                    [strMnc2](AString& strMnc)
                    {
                        strMnc = strMnc2;
                        return IMS_TRUE;
                    }));

    AString strUri("sip:mmtel@conf-factory.ims.mnc[MNC].mcc[MCC].3gppnetwork.org");
    strUri = strUri.Replace("[MCC]", strMcc3).Replace("[MNC]", strMnc3);

    EXPECT_EQ(strUri, pDialingPlan->GetToUri("", objCallInfo));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsDefaultConferenceUriIfMncIs3Digit)
{
    objCallInfo.bConference = IMS_TRUE;
    ON_CALL(*pConfigurationManager, GetConferenceFactoryUri)
            .WillByDefault(Return(AString::ConstEmpty()));

    const AString strMcc3("123");
    const AString strMnc2("56");
    const AString strMnc3("056");
    ON_CALL(objSubscriberInfo, GetSimMcc)
            .WillByDefault(Invoke(
                    [strMcc3](AString& strMcc)
                    {
                        strMcc = strMcc3;
                        return IMS_TRUE;
                    }));
    ON_CALL(objSubscriberInfo, GetSimMnc)
            .WillByDefault(Invoke(
                    [strMnc3](AString& strMnc)
                    {
                        strMnc = strMnc3;
                        return IMS_TRUE;
                    }));

    AString strUri("sip:mmtel@conf-factory.ims.mnc[MNC].mcc[MCC].3gppnetwork.org");
    strUri = strUri.Replace("[MCC]", strMcc3).Replace("[MNC]", strMnc3);

    EXPECT_EQ(strUri, pDialingPlan->GetToUri("", objCallInfo));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsConferenceFactoryUriWithConvertingHomaDomainName)
{
    objCallInfo.bConference = IMS_TRUE;
    AString strUri("sip:anyUserPart@[DOMAIN]");
    ON_CALL(*pConfigurationManager, GetConferenceFactoryUri).WillByDefault(Return(strUri));
    AString strDomainName("any_domain_name");
    EXPECT_CALL(*pIdentityProxy, GetHomeDomainName(_)).WillOnce(Return(strDomainName));

    strUri = strUri.Replace("[DOMAIN]", strDomainName);

    EXPECT_STREQ(strUri.GetStr(), pDialingPlan->GetToUri("", objCallInfo).GetStr());
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsDefaultUriInSipUriScheme)
{
    objCallInfo.bConference = IMS_TRUE;
    AString strUriWithoutScheme("conf@ims.google.com");
    AString strUriWithScheme("sip:conf@ims.google.com");

    ON_CALL(*pConfigurationManager, GetConferenceFactoryUri)
            .WillByDefault(Return(strUriWithoutScheme));

    EXPECT_EQ(strUriWithScheme, pDialingPlan->GetToUri("", objCallInfo));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsInputValueIfItIsUriForm)
{
    AString strUriWithScheme("sip:1234@ims.google.com");

    EXPECT_EQ(strUriWithScheme, pDialingPlan->GetToUri(strUriWithScheme, objCallInfo));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsStoredServiceUrn)
{
    AString strNumber("123");
    AString strServiceUrn("sos.country-specific.test");

    pDialingPlan->OnCountrySpecificServiceUrnReceived(strNumber, strServiceUrn);
    EXPECT_EQ(strServiceUrn, pDialingPlan->GetToUri(strNumber, objCallInfo));
}

TEST_F(MtcDialingPlanTest, GetToUriUsingDifferentNumberResetsTemporaryUrn)
{
    AString strNumber("123");
    AString strServiceUrn("sos.country-specific.test");
    AString strDifferentNumber("123");

    pDialingPlan->OnCountrySpecificServiceUrnReceived(strNumber, strServiceUrn);
    pDialingPlan->GetToUri(strDifferentNumber, objCallInfo);
    EXPECT_NE(strServiceUrn, pDialingPlan->GetToUri(strNumber, objCallInfo));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsDialStringFormatIfUssi)
{
    objCallInfo.bUssi = IMS_TRUE;
    AString strNumber("123");
    AString strExpectedUri("sip:123;phone-context=ims.google.com@ims.google.com;user=dialstring");
    ON_CALL(*pIdentityProxy, CreateSipUserIdWithDialString(strNumber, _, _))
            .WillByDefault(Return(strExpectedUri));
    EXPECT_STREQ(strExpectedUri.GetStr(), pDialingPlan->GetToUri(strNumber, objCallInfo).GetStr());
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsDialNumberWithPhone)
{
    AString strNumber("+1123");
    AString strExpectedUri("sip:+1123;user=phone");
    ON_CALL(*pIdentityProxy, CreateSipUserId(strNumber, _, IMS_TRUE, _))
            .WillByDefault(Return(strExpectedUri));
    EXPECT_STREQ(strExpectedUri.GetStr(),
            pDialingPlan->GetToUri(strNumber, objCallInfo, Scheme::SIP).GetStr());
}

}  // namespace android
