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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ConfigMtc.h"
#include "ConfigMts.h"
#include "ConfigSipDelegate.h"
#include "ConfigUce.h"
#include "ImsServiceConfig.h"

using ::testing::AnyOf;

namespace android
{

class ImsServiceConfigTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(ImsServiceConfigTest, GetAppId)
{
    ImsAppId eAppId = ImsServiceConfig::GetAppId(ImsServiceId::UNKNOWN);

    EXPECT_EQ(eAppId, ImsAppId::UNKNOWN);

    eAppId = ImsServiceConfig::GetAppId(ImsServiceId::MTC);

    EXPECT_EQ(eAppId, ImsAppId::MTC);

    eAppId = ImsServiceConfig::GetAppId(ImsServiceId::MTC_EMERGENCY);

    EXPECT_EQ(eAppId, ImsAppId::MTC);

    eAppId = ImsServiceConfig::GetAppId(ImsServiceId::MTS);

    EXPECT_EQ(eAppId, ImsAppId::MTS);

    eAppId = ImsServiceConfig::GetAppId(ImsServiceId::MTS_EMERGENCY);

    EXPECT_EQ(eAppId, ImsAppId::MTS);

    eAppId = ImsServiceConfig::GetAppId(ImsServiceId::UCE);

    EXPECT_EQ(eAppId, ImsAppId::UCE);

    eAppId = ImsServiceConfig::GetAppId(ImsServiceId::SIP_DELEGATE);

    EXPECT_EQ(eAppId, ImsAppId::SIP_DELEGATE);
}

TEST_F(ImsServiceConfigTest, GetAppName)
{
    AString strAppName = ImsServiceConfig::GetAppName(ImsAppId::UNKNOWN);

    EXPECT_EQ(strAppName, AString::ConstNull());

    strAppName = ImsServiceConfig::GetAppName(ImsAppId::MTC);

    EXPECT_EQ(strAppName, AString(ConfigMtc::APP_NAME));

    strAppName = ImsServiceConfig::GetAppName(ImsAppId::MTS);

    EXPECT_EQ(strAppName, AString(ConfigMts::APP_NAME));

    strAppName = ImsServiceConfig::GetAppName(ImsAppId::UCE);

    EXPECT_EQ(strAppName, AString(ConfigUce::APP_NAME));

    strAppName = ImsServiceConfig::GetAppName(ImsAppId::SIP_DELEGATE);

    EXPECT_EQ(strAppName, AString(ConfigSipDelegate::APP_NAME));
}

TEST_F(ImsServiceConfigTest, GetServiceName)
{
    AString strServiceName = ImsServiceConfig::GetServiceName(ImsServiceId::UNKNOWN);

    EXPECT_EQ(strServiceName, AString::ConstNull());

    strServiceName = ImsServiceConfig::GetServiceName(ImsServiceId::MTC);

    EXPECT_EQ(strServiceName, AString(ConfigMtc::SERVICE_NAME));

    strServiceName = ImsServiceConfig::GetServiceName(ImsServiceId::MTC_EMERGENCY);

    EXPECT_EQ(strServiceName, AString(ConfigMtc::EMERGENCY_SERVICE_NAME));

    strServiceName = ImsServiceConfig::GetServiceName(ImsServiceId::MTS);

    EXPECT_EQ(strServiceName, AString(ConfigMts::SERVICE_NAME));

    strServiceName = ImsServiceConfig::GetServiceName(ImsServiceId::MTS_EMERGENCY);

    EXPECT_EQ(strServiceName, AString(ConfigMts::EMERGENCY_SERVICE_NAME));

    strServiceName = ImsServiceConfig::GetServiceName(ImsServiceId::UCE);

    EXPECT_EQ(strServiceName, AString(ConfigUce::SERVICE_NAME));

    strServiceName = ImsServiceConfig::GetServiceName(ImsServiceId::SIP_DELEGATE);

    EXPECT_EQ(strServiceName, AString(ConfigSipDelegate::SERVICE_NAME));
}

TEST_F(ImsServiceConfigTest, GetAppNameC)
{
    const IMS_CHAR* pszAppName = ImsServiceConfig::GetAppNameC(ImsAppId::UNKNOWN);

    EXPECT_EQ(pszAppName, nullptr);

    pszAppName = ImsServiceConfig::GetAppNameC(ImsAppId::MTC);

    EXPECT_STREQ(pszAppName, ConfigMtc::APP_NAME);

    pszAppName = ImsServiceConfig::GetAppNameC(ImsAppId::MTS);

    EXPECT_STREQ(pszAppName, ConfigMts::APP_NAME);

    pszAppName = ImsServiceConfig::GetAppNameC(ImsAppId::UCE);

    EXPECT_STREQ(pszAppName, ConfigUce::APP_NAME);

    pszAppName = ImsServiceConfig::GetAppNameC(ImsAppId::SIP_DELEGATE);

    EXPECT_STREQ(pszAppName, ConfigSipDelegate::APP_NAME);
}

TEST_F(ImsServiceConfigTest, GetServiceNameC)
{
    const IMS_CHAR* pszServiceName = ImsServiceConfig::GetServiceNameC(ImsServiceId::UNKNOWN);

    EXPECT_EQ(pszServiceName, nullptr);

    pszServiceName = ImsServiceConfig::GetServiceNameC(ImsServiceId::MTC);

    EXPECT_STREQ(pszServiceName, ConfigMtc::SERVICE_NAME);

    pszServiceName = ImsServiceConfig::GetServiceNameC(ImsServiceId::MTC_EMERGENCY);

    EXPECT_STREQ(pszServiceName, ConfigMtc::EMERGENCY_SERVICE_NAME);

    pszServiceName = ImsServiceConfig::GetServiceNameC(ImsServiceId::MTS);

    EXPECT_STREQ(pszServiceName, ConfigMts::SERVICE_NAME);

    pszServiceName = ImsServiceConfig::GetServiceNameC(ImsServiceId::MTS_EMERGENCY);

    EXPECT_STREQ(pszServiceName, ConfigMts::EMERGENCY_SERVICE_NAME);

    pszServiceName = ImsServiceConfig::GetServiceNameC(ImsServiceId::UCE);

    EXPECT_STREQ(pszServiceName, ConfigUce::SERVICE_NAME);

    pszServiceName = ImsServiceConfig::GetServiceNameC(ImsServiceId::SIP_DELEGATE);

    EXPECT_STREQ(pszServiceName, ConfigSipDelegate::SERVICE_NAME);
}

TEST_F(ImsServiceConfigTest, GetServiceProfile)
{
    ImsServiceProfile objServiceProfile = ImsServiceConfig::GetServiceProfile();

    for (IMS_SINT32 i = 0; i < objServiceProfile.nCount; ++i)
    {
        const ImsServiceIdentifier& objServiceId = objServiceProfile.pServiceIds[i];

        EXPECT_THAT(objServiceId.eAppId,
                AnyOf(ImsAppId::MTC, ImsAppId::MTS, ImsAppId::UCE, ImsAppId::SIP_DELEGATE));
        EXPECT_THAT(objServiceId.eServiceId,
                AnyOf(ImsServiceId::MTC, ImsServiceId::MTS, ImsServiceId::UCE,
                        ImsServiceId::SIP_DELEGATE));
    }
}

TEST_F(ImsServiceConfigTest, GetEmergencyServiceProfile)
{
    ImsServiceProfile objServiceProfile = ImsServiceConfig::GetEmergencyServiceProfile();

    for (IMS_SINT32 i = 0; i < objServiceProfile.nCount; ++i)
    {
        const ImsServiceIdentifier& objServiceId = objServiceProfile.pServiceIds[i];

        EXPECT_THAT(objServiceId.eAppId, AnyOf(ImsAppId::MTC, ImsAppId::MTS));
        EXPECT_THAT(objServiceId.eServiceId,
                AnyOf(ImsServiceId::MTC_EMERGENCY, ImsServiceId::MTS_EMERGENCY));
    }
}

TEST_F(ImsServiceConfigTest, GetServiceNamesForServiceProfile)
{
    ImsServiceProfile objServiceProfile = ImsServiceConfig::GetServiceProfile();
    ImsList<ImsServiceName> objServiceNames = ImsServiceConfig::GetServiceNames(objServiceProfile);

    for (IMS_UINT32 i = 0; i < objServiceNames.GetSize(); ++i)
    {
        const ImsServiceName& objServiceName = objServiceNames.GetAt(i);

        EXPECT_THAT(objServiceName.GetAppId(),
                AnyOf(AString(ConfigMtc::APP_NAME), AString(ConfigMts::APP_NAME),
                        AString(ConfigUce::APP_NAME), AString(ConfigSipDelegate::APP_NAME)));
        EXPECT_THAT(objServiceName.GetServiceId(),
                AnyOf(AString(ConfigMtc::SERVICE_NAME), AString(ConfigMts::SERVICE_NAME),
                        AString(ConfigUce::SERVICE_NAME),
                        AString(ConfigSipDelegate::SERVICE_NAME)));
    }
}

TEST_F(ImsServiceConfigTest, GetServiceNamesForEmergencyServiceProfile)
{
    ImsServiceProfile objServiceProfile = ImsServiceConfig::GetEmergencyServiceProfile();
    ImsList<ImsServiceName> objServiceNames = ImsServiceConfig::GetServiceNames(objServiceProfile);

    for (IMS_UINT32 i = 0; i < objServiceNames.GetSize(); ++i)
    {
        const ImsServiceName& objServiceName = objServiceNames.GetAt(i);

        EXPECT_THAT(objServiceName.GetAppId(),
                AnyOf(AString(ConfigMtc::APP_NAME), AString(ConfigMts::APP_NAME)));
        EXPECT_THAT(objServiceName.GetServiceId(),
                AnyOf(AString(ConfigMtc::EMERGENCY_SERVICE_NAME),
                        AString(ConfigMts::EMERGENCY_SERVICE_NAME)));
    }
}

}  // namespace android
