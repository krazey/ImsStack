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
#include "MockIPhoneInfoSubscriber.h"
#include "call/IMtcCall.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MtcDialingPlan.h"
#include <gtest/gtest.h>

namespace android
{

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;

class MtcDialingPlanTest : public ::testing::Test
{
public:
    MtcDialingPlan* pDialingPlan;
    MockIMtcContext objContext;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    MockISubscriberInfo objSubscriberInfo;
    CallInfo objCallInfo;

protected:
    virtual void SetUp() override
    {
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        pDialingPlan = new MtcDialingPlan(objContext, objSubscriberInfo);
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

TEST_F(MtcDialingPlanTest, GetToUriEmergency)
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
    ON_CALL(objSubscriberInfo, GetMcc)
            .WillByDefault(Invoke(
                    [strMcc3](AString& strMcc)
                    {
                        strMcc = strMcc3;
                        return IMS_TRUE;
                    }));
    ON_CALL(objSubscriberInfo, GetMnc)
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
    ON_CALL(objSubscriberInfo, GetMcc)
            .WillByDefault(Invoke(
                    [strMcc3](AString& strMcc)
                    {
                        strMcc = strMcc3;
                        return IMS_TRUE;
                    }));
    ON_CALL(objSubscriberInfo, GetMnc)
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

TEST_F(MtcDialingPlanTest, GetToUriReturnsDefaultUriInSipUriScheme)
{
    objCallInfo.bConference = IMS_TRUE;
    AString strUriWithoutScheme("conf@ims.google.com");
    AString strUriWithScheme("sip:conf@ims.google.com");

    ON_CALL(*pConfigurationManager, GetConferenceFactoryUri)
            .WillByDefault(Return(strUriWithoutScheme));

    EXPECT_EQ(strUriWithScheme, pDialingPlan->GetToUri("", objCallInfo));
}

TEST_F(MtcDialingPlanTest, GetToUriReturnsInputValueIfItsUriForm)
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

}  // namespace android
