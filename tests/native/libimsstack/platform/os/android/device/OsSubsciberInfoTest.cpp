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
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ImsEventDef.h"
#include "ImsTypeDef.h"
#include "MockISystem.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"
#include "device/OsSubscriberInfo.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

class OsSubscriberInfoTest : public ::testing::Test
{
public:
    MockISystem m_objMockSystem;

    ISystem* m_piDefaultSystem;
    ISystemListener* m_piSystemListener;
    OsSubscriberInfo* m_pOsSubscriberInfo;

    TestPhoneInfoService m_objPhoneInfoService;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);

        m_piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&m_objMockSystem);

        m_pOsSubscriberInfo = new OsSubscriberInfo(IMS_SLOT_0);
        ASSERT_TRUE(m_pOsSubscriberInfo != nullptr);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        if (m_pOsSubscriberInfo != IMS_NULL)
        {
            delete m_pOsSubscriberInfo;
            m_pOsSubscriberInfo = IMS_NULL;
        }
    }
};

TEST_F(OsSubscriberInfoTest, GetPhoneNumber)
{
    AString strValue("9924434376");
    EXPECT_CALL(m_objMockSystem, GetPhoneNumber(_, _))
            .Times(1)
            .WillOnce(Invoke(
                    [strValue](AString& strPhoneNumber, Unused)
                    {
                        strPhoneNumber = strValue;
                        return 1;
                    }));

    AString strOut;
    EXPECT_EQ(IMS_TRUE, m_pOsSubscriberInfo->GetPhoneNumber(strOut));
    EXPECT_EQ(strValue, strOut);
}

TEST_F(OsSubscriberInfoTest, GetSimMcc)
{
    AString strValue("404");
    EXPECT_CALL(m_objMockSystem, GetSimMcc(_, _))
            .Times(1)
            .WillOnce(Invoke(
                    [strValue](AString& strMcc, Unused)
                    {
                        strMcc = strValue;
                        return 1;
                    }));

    AString strOut;
    EXPECT_EQ(IMS_TRUE, m_pOsSubscriberInfo->GetSimMcc(strOut));
    EXPECT_EQ(strValue, strOut);
}

TEST_F(OsSubscriberInfoTest, GetSimMnc)
{
    AString strValue("03");
    EXPECT_CALL(m_objMockSystem, GetSimMnc(_, _))
            .Times(1)
            .WillOnce(Invoke(
                    [strValue](AString& strMnc, Unused)
                    {
                        strMnc = strValue;
                        return 1;
                    }));

    AString strOut;
    EXPECT_EQ(IMS_TRUE, m_pOsSubscriberInfo->GetSimMnc(strOut));
    EXPECT_EQ(strValue, strOut);
}

TEST_F(OsSubscriberInfoTest, GetSimCountryIso)
{
    AString strValue("India");
    EXPECT_CALL(m_objMockSystem, GetSimCountryIso(_, _))
            .Times(1)
            .WillOnce(Invoke(
                    [strValue](AString& strCountry, Unused)
                    {
                        strCountry = strValue;
                        return 1;
                    }));

    AString strOut;
    EXPECT_EQ(IMS_TRUE, m_pOsSubscriberInfo->GetSimCountryIso(strOut));
    EXPECT_EQ(strValue, strOut);
}

TEST_F(OsSubscriberInfoTest, GetNetworkCountryIso)
{
    AString strValue("IN");
    EXPECT_CALL(m_objMockSystem, GetNetworkCountryIso(_, _))
            .Times(1)
            .WillOnce(Invoke(
                    [strValue](AString& strCountry, Unused)
                    {
                        strCountry = strValue;
                        return 1;
                    }));

    AString strOut;
    EXPECT_EQ(IMS_TRUE, m_pOsSubscriberInfo->GetNetworkCountryIso(strOut));
    EXPECT_EQ(strValue, strOut);
}

TEST_F(OsSubscriberInfoTest, GetSubscriberId)
{
    AString strValue("405861095771871");
    EXPECT_CALL(m_objMockSystem, GetSubscriberId(_, _))
            .Times(1)
            .WillOnce(Invoke(
                    [strValue](AString& strImsi, Unused)
                    {
                        strImsi = strValue;
                        return 1;
                    }));

    AString strOut;
    EXPECT_EQ(IMS_TRUE, m_pOsSubscriberInfo->GetSubscriberId(strOut));
    EXPECT_EQ(strValue, strOut);
}

TEST_F(OsSubscriberInfoTest, SetAndGetPreference)
{
    AString strFileName("impu_list");
    AString strKey("imsi");
    AString strValue("234159556188095");

    AString strOutValue;

    EXPECT_CALL(m_objMockSystem, SetPreference(_, _, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [&](Unused, Unused, IN const AString& strValue, Unused)
                    {
                        strOutValue = strValue;
                        return 1;
                    }));

    EXPECT_EQ(IMS_TRUE, m_pOsSubscriberInfo->SetPreference(strFileName, strKey, strValue));

    EXPECT_CALL(m_objMockSystem, GetPreference(_, _, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [strOutValue](Unused, Unused, Unused, AString& strValue)
                    {
                        strValue = strOutValue;
                        return 1;
                    }));
    AString strOut;
    EXPECT_EQ(IMS_TRUE, m_pOsSubscriberInfo->GetPreference(strFileName, strKey, strOut));
    EXPECT_EQ(strValue, strOut);
}

}  // namespace android
