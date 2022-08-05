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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "DeviceConfig.h"

#include "NativeCommands.h"
#include "NativeCommandsHandler.h"

using ::testing::Eq;
using ::testing::Return;

namespace android
{

class MockIEnablerLoader : public IEnablerLoader
{
public:
    inline MockIEnablerLoader() {}
    inline virtual ~MockIEnablerLoader() {}

    MOCK_METHOD(void, StartEnabler, (IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, StopEnabler, (IN IMS_SINT32 nSlotId), (override));
};

class NativeCommandsHandlerTest : public ::testing::Test
{
protected:
    virtual void SetUp() override { m_pEnablerLoader = new MockIEnablerLoader(); }
    virtual void TearDown() override { delete m_pEnablerLoader; }

protected:
    MockIEnablerLoader* m_pEnablerLoader;
};

TEST_F(NativeCommandsHandlerTest, OnCommandForSetDeviceConfig)
{
    NativeCommandsHandler objCommandsHandler;
    // <active modem count, emergency, volte, vt, wfc>
    __DeviceConfig objConfig(2, 1, 1, 1, 1);
    IMS_UINTP pnParam = reinterpret_cast<IMS_UINTP>(&objConfig);
    objCommandsHandler.OnCommand(NativeCommands::CMD_SET_DEVICE_CONFIG, -1, pnParam);

    EXPECT_EQ(DeviceConfig::GetActiveModemCount(), 2);
    EXPECT_TRUE(DeviceConfig::IsImsEmergencyEnabled());
    EXPECT_TRUE(DeviceConfig::IsVoLteEnabled());
    EXPECT_TRUE(DeviceConfig::IsVtEnabled());
    EXPECT_TRUE(DeviceConfig::IsWfcEnabled());

    // <active modem count, emergency, volte, vt, wfc>
    __DeviceConfig objConfig2(1, 0, 0, 0, 0);
    pnParam = reinterpret_cast<IMS_UINTP>(&objConfig2);
    objCommandsHandler.OnCommand(NativeCommands::CMD_SET_DEVICE_CONFIG, -1, pnParam);

    EXPECT_EQ(DeviceConfig::GetActiveModemCount(), 1);
    EXPECT_FALSE(DeviceConfig::IsImsEmergencyEnabled());
    EXPECT_FALSE(DeviceConfig::IsVoLteEnabled());
    EXPECT_FALSE(DeviceConfig::IsVtEnabled());
    EXPECT_FALSE(DeviceConfig::IsWfcEnabled());
}

TEST_F(NativeCommandsHandlerTest, OnCommandForStartEnabler)
{
    EXPECT_CALL(*m_pEnablerLoader, StartEnabler(IMS_SLOT_0)).Times(1);

    NativeCommandsHandler objCommandsHandler;
    objCommandsHandler.OnCommand(NativeCommands::CMD_START_ENABLER, IMS_SLOT_0, 0);
    objCommandsHandler.SetEnablerLoader(m_pEnablerLoader);
    objCommandsHandler.OnCommand(NativeCommands::CMD_START_ENABLER, IMS_SLOT_0, 0);
}

TEST_F(NativeCommandsHandlerTest, OnCommandForStopEnabler)
{
    EXPECT_CALL(*m_pEnablerLoader, StopEnabler(IMS_SLOT_0)).Times(1);

    NativeCommandsHandler objCommandsHandler;
    objCommandsHandler.OnCommand(NativeCommands::CMD_STOP_ENABLER, IMS_SLOT_0, 0);
    objCommandsHandler.SetEnablerLoader(m_pEnablerLoader);
    objCommandsHandler.OnCommand(NativeCommands::CMD_STOP_ENABLER, IMS_SLOT_0, 0);
}

TEST_F(NativeCommandsHandlerTest, OnCommandForDefault)
{
    NativeCommandsHandler objCommandsHandler;
    objCommandsHandler.OnCommand(100, IMS_SLOT_0, 0);
    // no-op & no expectation
}

}  // namespace android
