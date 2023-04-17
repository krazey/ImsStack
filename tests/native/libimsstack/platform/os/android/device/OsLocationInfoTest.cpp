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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockISystem.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"
#include "device/OsLocationInfo.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

class OsLocationInfoTest : public ::testing::Test
{
protected:
    inline OsLocationInfoTest() :
            m_strNetworkCountry("US"),
            m_pOldSystem(IMS_NULL),
            m_pPhoneInfoService(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override
    {
        m_pPhoneInfoService = new TestPhoneInfoService();

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, m_pPhoneInfoService);
        m_pOldSystem = PlatformContext::GetInstance()->SetSystem(&m_objSystem);

        ON_CALL(m_pPhoneInfoService->GetMockSubscriberInfo(), GetNetworkCountry(_))
                .WillByDefault(Invoke(
                        [&](AString& strCountry)
                        {
                            strCountry = m_strNetworkCountry;
                            return IMS_TRUE;
                        }));
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetSystem(m_pOldSystem);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);

        if (m_pPhoneInfoService != IMS_NULL)
        {
            m_pPhoneInfoService->Destroy();
        }
    }

protected:
    AString m_strNetworkCountry;
    ISystem* m_pOldSystem;
    MockISystem m_objSystem;
    TestPhoneInfoService* m_pPhoneInfoService;
};

TEST_F(OsLocationInfoTest, StartListeningForLocation)
{
    IMS_UINT32 nUpdateIntervalInSec = 10;
    OsLocationInfo objOsLocationInfo(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, StartListeningForLocation(_, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_EQ(IMS_TRUE, objOsLocationInfo.StartListeningForLocation(nUpdateIntervalInSec));

    EXPECT_CALL(m_objSystem, StartListeningForLocation(_, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_EQ(IMS_TRUE, objOsLocationInfo.StartListeningForLocation(nUpdateIntervalInSec));
    EXPECT_EQ(IMS_TRUE, objOsLocationInfo.StartListeningForLocation(nUpdateIntervalInSec));
}

TEST_F(OsLocationInfoTest, StopListeningForLocation)
{
    IMS_UINT32 nUpdateIntervalInSec = 10;
    OsLocationInfo objOsLocationInfo(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, StopListeningForLocation(_)).Times(0);

    objOsLocationInfo.StopListeningForLocation();

    EXPECT_CALL(m_objSystem, StartListeningForLocation(_, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    objOsLocationInfo.StartListeningForLocation(nUpdateIntervalInSec);

    EXPECT_CALL(m_objSystem, StopListeningForLocation(_)).Times(1);

    objOsLocationInfo.StopListeningForLocation();
    objOsLocationInfo.StopListeningForLocation();
}

TEST_F(OsLocationInfoTest, GetLocationProperties)
{
    AStringArray objLocationInformation;

    objLocationInformation.AddElement("Latitude");          // 0
    objLocationInformation.AddElement("Longitude");         // 1
    objLocationInformation.AddElement("Radius");            // 2
    objLocationInformation.AddElement("Shape");             // 3
    objLocationInformation.AddElement("Confidence");        // 4
    objLocationInformation.AddElement("CurrentTime");       // 5
    objLocationInformation.AddElement("Method");            // 6
    objLocationInformation.AddElement("");                  // 7 - No Country added
    objLocationInformation.AddElement("State");             // 8
    objLocationInformation.AddElement("City");              // 9
    objLocationInformation.AddElement("Postal");            // 10
    objLocationInformation.AddElement("Altitude");          // 11
    objLocationInformation.AddElement("VerticalAccuracy");  // 12

    OsLocationInfo objOsLocationInfo(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, GetLastKnownLocation(_, _, _)).Times(1).WillRepeatedly(Return(0));

    // 1. Returns null if GetLastKnownLocation fails
    EXPECT_EQ(IMS_NULL, objOsLocationInfo.GetLocationProperties());

    EXPECT_CALL(m_objSystem, GetLastKnownLocation(_, _, _)).Times(AnyNumber());

    ON_CALL(m_objSystem, GetLastKnownLocation(_, _, _))
            .WillByDefault(Invoke(
                    [&](AStringArray& objLocationInfo, Unused, Unused)
                    {
                        objLocationInfo = objLocationInformation;
                        return 1;
                    }));

    EXPECT_CALL(m_pPhoneInfoService->GetMockSubscriberInfo(), GetNetworkCountry(_))
            .Times(AnyNumber());
    EXPECT_CALL(m_pPhoneInfoService->GetMockSubscriberInfo(), GetCountry(_)).Times(0);

    // 2. Get default network country from GetNetworkCountry() if country not present in
    // location information received from GetLastKnownLocation()
    ILocationProperties* piLocationProperties = objOsLocationInfo.GetLocationProperties();

    EXPECT_TRUE(piLocationProperties != nullptr);

    EXPECT_EQ(objOsLocationInfo.GetLastKnownCountry(), m_strNetworkCountry);

    AString strCountryName("Country");

    objLocationInformation.SetElementAt(strCountryName, 7);  // Add Country at index 7

    // 3. Get network country from location information - GetLastKnownLocation()
    piLocationProperties = objOsLocationInfo.GetLocationProperties();

    EXPECT_TRUE(piLocationProperties != nullptr);

    EXPECT_EQ(objOsLocationInfo.GetLastKnownCountry(), strCountryName);

    AString strNetworkCountry("");

    ON_CALL(m_pPhoneInfoService->GetMockSubscriberInfo(), GetNetworkCountry(_))
            .WillByDefault(Invoke(
                    [strNetworkCountry](AString& strCountry)
                    {
                        strCountry = strNetworkCountry;
                        return IMS_TRUE;
                    }));

    objLocationInformation.SetElementAt(AString::ConstEmpty(), 7);  // Reset Country at index 7

    piLocationProperties = objOsLocationInfo.GetLocationProperties();

    EXPECT_TRUE(piLocationProperties != nullptr);

    // 4. Get network country UNKNOWN if location information from GetLastKnownLocation()
    // and GetNetworkCountry() does not provide country.
    EXPECT_EQ(
            objOsLocationInfo.GetLastKnownCountry(), AString(OsLocationInfo::COUNTRY_ISO_UNKNOWN));
}

TEST_F(OsLocationInfoTest, StartInstantLocationUpdate)
{
    OsLocationInfo objOsLocationInfo(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, StartInstantLocationUpdate(_))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_EQ(IMS_TRUE, objOsLocationInfo.StartInstantLocationUpdate());
}

TEST_F(OsLocationInfoTest, SetDefaultLocationProperties)
{
    OsLocationInfo objOsLocationInfo(IMS_SLOT_0);

    // 1. Default country set returned by GetNetworkCountry()
    objOsLocationInfo.SetDefaultLocationProperties();

    EXPECT_EQ(objOsLocationInfo.GetLastKnownCountry(), m_strNetworkCountry);

    AString strNetworkCountry("");

    ON_CALL(m_pPhoneInfoService->GetMockSubscriberInfo(), GetNetworkCountry(_))
            .WillByDefault(Invoke(
                    [strNetworkCountry](AString& strCountry)
                    {
                        strCountry = strNetworkCountry;
                        return IMS_TRUE;
                    }));

    // 2. UNKNOWN country set if GetNetworkCountry() does not provide country
    objOsLocationInfo.SetDefaultLocationProperties();

    EXPECT_EQ(
            objOsLocationInfo.GetLastKnownCountry(), AString(OsLocationInfo::COUNTRY_ISO_UNKNOWN));

    AString strUiccCountry("UK");

    ON_CALL(m_pPhoneInfoService->GetMockSubscriberInfo(), GetCountry(_))
            .WillByDefault(Invoke(
                    [strUiccCountry](AString& strCountry)
                    {
                        strCountry = strUiccCountry;
                        return IMS_TRUE;
                    }));

    // 3. Set country from UICC - GetCountry()
    objOsLocationInfo.SetDefaultLocationProperties(IMS_TRUE);  // Get from UICC

    EXPECT_EQ(objOsLocationInfo.GetLastKnownCountry(), strUiccCountry);
}

}  // namespace android
