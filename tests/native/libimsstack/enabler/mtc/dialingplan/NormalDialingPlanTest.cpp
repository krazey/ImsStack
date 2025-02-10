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
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockINetworkConnection.h"
#include "PlatformContext.h"
#include "ServiceNetworkPolicy.h"
#include "TestNetworkService.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MockImsIdentityProxy.h"
#include "dialingplan/NormalDialingPlan.h"
#include "helper/MockIMtcAosConnector.h"
#include <gtest/gtest.h>

using LocalnumberPolicy = NormalDialingPlan::LocalNumberPolicy;
using NumberFormat = NormalDialingPlan::NumberFormat;
using Scheme = NormalDialingPlan::Scheme;

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

LOCAL IMS_UINT32 ANY_COUNTRY_CODE = 1;
LOCAL IMS_SINT32 SLOT_ID = 0;

class NormalDialingPlanTest : public ::testing::Test
{
public:
    NormalDialingPlanTest() :
            objContext(),
            pConfigurationProxy(new MockMtcConfigurationProxy()),
            objIdentityProxy(),
            strNumber(AString::ConstNull()),
            eScheme(Scheme::UNKNOWN)
    {
    }

    ~NormalDialingPlanTest() { delete pConfigurationProxy; }

protected:
    MockIMtcContext objContext;
    MockMtcConfigurationProxy* pConfigurationProxy;
    MockIMtcService objService;
    MockImsIdentityProxy objIdentityProxy;
    MockIMtcAosConnector objAosConnector;
    MockINetworkConnection objNetworkConnection;
    TestNetworkService objNetworkService;
    AString strNumber;
    Scheme eScheme;

    virtual void SetUp() override
    {
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetServiceByType).WillByDefault(Return(&objService));

        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));

        ON_CALL(objAosConnector, GetConnectionType).WillByDefault(Return(NetworkPolicy::APN_IMS));

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, &objNetworkService);
        objNetworkService.SetConnection(&objNetworkConnection);

        ON_CALL(*pConfigurationProxy, GetInt(ConfigWfc::KEY_COUNTRY_CODE_INT))
                .WillByDefault(Return(ANY_COUNTRY_CODE));
        ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_OF_LOCAL_NUMBERS_INT))
                .WillByDefault(Return(0));  // LocalNumberPolicy::HOME
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_NETWORK, IMS_NULL);
    }

    IMS_CHAR* GetTranslatedUri()
    {
        return NormalDialingPlan::GetTranslatedUri(objContext, strNumber, eScheme, objIdentityProxy)
                .GetStr();
    }
};

TEST_F(NormalDialingPlanTest, GetTranslatedUriForDialStringReturnsUriWithDialString)
{
    strNumber = "123";
    AString strExpectedUri(
            "sip:" + strNumber + ";phone-context=ims.google.com@ims.google.com;user=dialstring");
    ON_CALL(objIdentityProxy, CreateSipUserIdWithDialString(strNumber, _, _))
            .WillByDefault(Return(strExpectedUri));
    strNumber = NormalDialingPlan::GetTranslatedUriForDialString(
            objContext, strNumber, objIdentityProxy);
    EXPECT_STREQ(strNumber.GetStr(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriReturnsEmptyIfNumberIsEmpty)
{
    AString strExpectedUri;
    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriReturnsOriginalStringIfNameAddressFormat)
{
    strNumber = "<123>";
    AString strExpectedUri(strNumber);
    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriReturnsOriginalStringWithAquotIfAddressSpec)
{
    strNumber = "sip:123;phone-context=ims.mnc001.mcc001.3gppnetwork.org";
    AString strExpectedUri("<" + strNumber + ">");
    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriReturnsTelUriIfSchemeIsUnknownAndTelConfigured)
{
    strNumber = "12345";
    eScheme = Scheme::UNKNOWN;
    AString strAnyPhoneContext("anyPhoneContext");
    AString strExpectedUri("<tel:" + strNumber + ";phone-context=" + strAnyPhoneContext + ">");

    ON_CALL(objIdentityProxy,
            GetPhoneContext(ImsIdentity::DIALING_POLICY_HOME_LOCAL, SLOT_ID, _, _))
            .WillByDefault(Return(strAnyPhoneContext));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigIms::KEY_REQUEST_URI_TYPE_INT))
            .WillByDefault(Return(ConfigIms::REQUEST_URI_FORMAT_TEL));

    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriReturnsTelUriAsLocalNumber)
{
    strNumber = "12345";
    eScheme = Scheme::TEL;
    AString strAnyPhoneContext("anyPhoneContext");
    AString strExpectedUri("<tel:" + strNumber + ";phone-context=" + strAnyPhoneContext + ">");

    ON_CALL(objIdentityProxy,
            GetPhoneContext(ImsIdentity::DIALING_POLICY_HOME_LOCAL, SLOT_ID, _, _))
            .WillByDefault(Return(strAnyPhoneContext));

    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriReturnsTelUriAsGeoLocalNumber)
{
    strNumber = "12345";
    eScheme = Scheme::TEL;
    AString strAnyPhoneContext("anyPhoneContext");
    AString strExpectedUri("<tel:" + strNumber + ";phone-context=" + strAnyPhoneContext + ">");

    ON_CALL(objIdentityProxy, GetPhoneContext(ImsIdentity::DIALING_POLICY_GEO_LOCAL, SLOT_ID, _, _))
            .WillByDefault(Return(strAnyPhoneContext));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_OF_LOCAL_NUMBERS_INT))
            .WillByDefault(Return(1));  // LocalNumberPolicy::GEO

    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriReturnsTelUriAsGlobalNumber)
{
    strNumber = "+12345";
    eScheme = Scheme::TEL;
    AString strAnyPhoneContext("anyPhoneContext");
    AString strExpectedUri("tel:" + strNumber);

    ON_CALL(objIdentityProxy,
            GetPhoneContext(ImsIdentity::DIALING_POLICY_HOME_LOCAL, SLOT_ID, _, _))
            .WillByDefault(Return(strAnyPhoneContext));

    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriReturnsSipUriIfSchemeIsUnknownAndSipConfigured)
{
    strNumber = "12345";
    eScheme = Scheme::UNKNOWN;
    AString strAnyPhoneContext("anyPhoneContext");
    AString strExpectedUri("<sip:" + strNumber + ";phone-context=" + strAnyPhoneContext + ">");

    ON_CALL(objIdentityProxy,
            GetPhoneContext(ImsIdentity::DIALING_POLICY_HOME_LOCAL, SLOT_ID, _, _))
            .WillByDefault(Return(strAnyPhoneContext));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigIms::KEY_REQUEST_URI_TYPE_INT))
            .WillByDefault(Return(ConfigIms::REQUEST_URI_FORMAT_SIP));
    ON_CALL(objIdentityProxy, CreateSipUserIdWithPhone(strNumber, SLOT_ID, strAnyPhoneContext))
            .WillByDefault(Return(strExpectedUri));

    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriReturnsSipUriAsLocalNumber)
{
    strNumber = "12345";
    eScheme = Scheme::SIP;
    AString strAnyPhoneContext("anyPhoneContext");
    AString strExpectedUri("<sip:" + strNumber + ";phone-context=" + strAnyPhoneContext + ">");

    ON_CALL(objIdentityProxy,
            GetPhoneContext(ImsIdentity::DIALING_POLICY_HOME_LOCAL, SLOT_ID, _, _))
            .WillByDefault(Return(strAnyPhoneContext));
    ON_CALL(objIdentityProxy, CreateSipUserIdWithPhone(strNumber, SLOT_ID, strAnyPhoneContext))
            .WillByDefault(Return(strExpectedUri));

    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriReturnsSipUriAsGeoLocalNumber)
{
    strNumber = "12345";
    eScheme = Scheme::SIP;
    AString strAnyPhoneContext("anyPhoneContext");
    AString strExpectedUri("<sip:" + strNumber + ";phone-context=" + strAnyPhoneContext + ">");

    ON_CALL(objIdentityProxy, GetPhoneContext(ImsIdentity::DIALING_POLICY_GEO_LOCAL, SLOT_ID, _, _))
            .WillByDefault(Return(strAnyPhoneContext));
    ON_CALL(objIdentityProxy, CreateSipUserIdWithPhone(strNumber, SLOT_ID, strAnyPhoneContext))
            .WillByDefault(Return(strExpectedUri));
    ON_CALL(*pConfigurationProxy, GetInt(ConfigVoice::KEY_POLICY_OF_LOCAL_NUMBERS_INT))
            .WillByDefault(Return(1));  // LocalNumberPolicy::GEO

    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriForNonNumberReturnsSipUriAsLocalNumber)
{
    strNumber = "123:456";
    eScheme = Scheme::SIP;
    AString strAnyPhoneContext("anyPhoneContext");
    AString strExpectedUri("<sip:" + strNumber + ";phone-context=" + strAnyPhoneContext + ">");

    ON_CALL(objIdentityProxy,
            GetPhoneContext(ImsIdentity::DIALING_POLICY_HOME_LOCAL, SLOT_ID, _, _))
            .WillByDefault(Return(strAnyPhoneContext));
    ON_CALL(objIdentityProxy, CreateSipUserIdWithPhone(strNumber, SLOT_ID, strAnyPhoneContext))
            .WillByDefault(Return(strExpectedUri));

    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriForNonNumberWithPlusReturnsSipUriAsLocalNumber)
{
    strNumber = "+123-456";
    eScheme = Scheme::SIP;
    AString strAnyPhoneContext("anyPhoneContext");
    AString strExpectedUri("<sip:" + strNumber + ";phone-context=" + strAnyPhoneContext + ">");

    ON_CALL(objIdentityProxy, CreateSipUserId(strNumber, SLOT_ID, _, _))
            .WillByDefault(Return(strExpectedUri));

    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

TEST_F(NormalDialingPlanTest, GetTranslatedUriReturnsSipUriAsGlobalNumber)
{
    strNumber = "+123-456";
    eScheme = Scheme::SIP;
    AString strAnyHomeDomainName("anyDomain");
    AString strExpectedUri = "sip:" + strNumber + "@" + strAnyHomeDomainName + ";user=phone";

    ON_CALL(objIdentityProxy, CreateSipUserId(strNumber, SLOT_ID, _, _))
            .WillByDefault(Return(strExpectedUri));

    EXPECT_STREQ(GetTranslatedUri(), strExpectedUri.GetStr());
}

}  // namespace android
