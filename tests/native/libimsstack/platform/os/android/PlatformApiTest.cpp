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

#include "ImsPrivateProperties.h"
#include "MockISystem.h"
#include "OsUtil.h"
#include "PlatformApi.h"
#include "PlatformContext.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Unused;

namespace android
{

class PlatformApiTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(PlatformApiTest, CheckIpAndPortAvailability)
{
    EXPECT_EQ(PlatformApi::CheckIpAndPortAvailability(
                      IpAddress("127.0.0.1"), 1260, ISocket::TYPE_DGRAM),
            IMS_TRUE);

    IpAddress objIpa;
    EXPECT_EQ(
            PlatformApi::CheckIpAndPortAvailability(objIpa, 5689, ISocket::TYPE_DGRAM), IMS_FALSE);
}

TEST_F(PlatformApiTest, SetDebugOn)
{
    PlatformApi::SetDebugOn(IMS_TRUE);
    EXPECT_EQ(OsUtil::GetInstance()->IsDebugMode(), IMS_TRUE);
}

TEST_F(PlatformApiTest, SetAndGetPrivateProperty)
{
    MockISystem objMockSystem;
    ISystem* piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&objMockSystem);

    AString strKey(ImsPrivateProperties::Persistent::KEY_TEST_LOG_OPTIONS);
    AString strValue("0x00010000");
    AString strOutValue;

    EXPECT_CALL(objMockSystem, SetPrivateProperty(_, _, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [&](Unused, Unused, const AString& strValue, Unused)
                    {
                        strOutValue = strValue;
                        return IMS_TRUE;
                    }));
    PlatformApi::SetPrivateProperty(IMS_TRUE, strKey, strValue, IMS_SLOT_0);

    EXPECT_CALL(objMockSystem, GetPrivateProperty(_, _, _))
            .Times(1)
            .WillOnce(Invoke(
                    [strOutValue](Unused, Unused, Unused)
                    {
                        return strOutValue;
                    }));
    EXPECT_EQ(PlatformApi::GetPrivateProperty(IMS_TRUE, strKey, IMS_SLOT_0), strValue);

    PlatformContext::GetInstance()->SetSystem(piDefaultSystem);
}

}  // namespace android
