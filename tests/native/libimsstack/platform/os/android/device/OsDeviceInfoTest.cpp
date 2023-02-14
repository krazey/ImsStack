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
#include "device/OsDeviceInfo.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

class OsDeviceInfoTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        m_pOldSystem = PlatformContext::GetInstance()->SetSystem(&m_objSystem);
    }

    virtual void TearDown() override { PlatformContext::GetInstance()->SetSystem(m_pOldSystem); }

protected:
    ISystem* m_pOldSystem;
    MockISystem m_objSystem;
};

TEST_F(OsDeviceInfoTest, GetDeviceId)
{
    OsDeviceInfo objOsDeviceInfo;

    EXPECT_CALL(m_objSystem, GetDeviceId(_, _)).Times(2);
    ON_CALL(m_objSystem, GetDeviceId(_, _))
            .WillByDefault(Invoke(
                    [](OUT AString& strId, Unused)
                    {
                        strId = "";
                        return 0;
                    }));

    AString strDeviceId;
    EXPECT_EQ(IMS_FALSE, objOsDeviceInfo.GetDeviceId(IMS_SLOT_0, strDeviceId));

    AString strDevId("DeviceId");
    ON_CALL(m_objSystem, GetDeviceId(_, _))
            .WillByDefault(Invoke(
                    [strDevId](OUT AString& strId, Unused)
                    {
                        strId = strDevId;
                        return 1;
                    }));

    EXPECT_EQ(IMS_TRUE, objOsDeviceInfo.GetDeviceId(IMS_SLOT_0, strDeviceId));
    EXPECT_EQ(strDevId, strDeviceId);
}

TEST_F(OsDeviceInfoTest, GetDeviceSoftwareVersion)
{
    OsDeviceInfo objOsDeviceInfo;

    EXPECT_CALL(m_objSystem, GetDeviceSoftwareVersion(_, _)).Times(2).WillOnce(Return(IMS_FALSE));

    AString strSoftwareVersion;
    EXPECT_EQ(IMS_FALSE, objOsDeviceInfo.GetDeviceSoftwareVersion(IMS_SLOT_0, strSoftwareVersion));

    AString strSwVersion("SoftwareVersion");
    ON_CALL(m_objSystem, GetDeviceSoftwareVersion(_, _))
            .WillByDefault(Invoke(
                    [strSwVersion](OUT AString& strSv, Unused)
                    {
                        strSv = strSwVersion;
                        return 1;
                    }));

    EXPECT_EQ(IMS_TRUE, objOsDeviceInfo.GetDeviceSoftwareVersion(IMS_SLOT_0, strSoftwareVersion));
    EXPECT_EQ(strSwVersion, strSoftwareVersion);
}

TEST_F(OsDeviceInfoTest, GetDeviceName)
{
    OsDeviceInfo objOsDeviceInfo;

    EXPECT_CALL(m_objSystem, GetDeviceName(_)).Times(2).WillOnce(Return(IMS_FALSE));

    AString strDevName;
    EXPECT_EQ(IMS_FALSE, objOsDeviceInfo.GetDeviceName(strDevName));

    AString strName("DeviceName");
    ON_CALL(m_objSystem, GetDeviceName(_))
            .WillByDefault(Invoke(
                    [strName](OUT AString& strDeviceName)
                    {
                        strDeviceName = strName;
                        return 1;
                    }));

    EXPECT_EQ(IMS_TRUE, objOsDeviceInfo.GetDeviceName(strDevName));
    EXPECT_EQ(strName, strDevName);
}

}  // namespace android
