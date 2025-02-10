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
#include "device/OsPhoneInfoCall.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;

namespace android
{

class OsPhoneInfoCallTest : public ::testing::Test
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

TEST_F(OsPhoneInfoCallTest, IsEmergencyNumber)
{
    OsPhoneInfoCall objOsPhoneInfoCall(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, IsEmergencyNumber(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_EQ(IMS_TRUE, objOsPhoneInfoCall.IsEmergencyNumber(AString("911")));
}

TEST_F(OsPhoneInfoCallTest, GetTtyMode)
{
    OsPhoneInfoCall objOsPhoneInfoCall(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, GetTtyMode(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(ICallInfo::TTY_MODE_FULL));

    EXPECT_EQ(ICallInfo::TTY_MODE_FULL, objOsPhoneInfoCall.GetTtyMode());
}

TEST_F(OsPhoneInfoCallTest, GetRttMode)
{
    OsPhoneInfoCall objOsPhoneInfoCall(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, GetRttMode(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(ICallInfo::RTT_MODE_ALWAYS_VISIBLE));

    EXPECT_EQ(ICallInfo::RTT_MODE_ALWAYS_VISIBLE, objOsPhoneInfoCall.GetRttMode());
}

TEST_F(OsPhoneInfoCallTest, IsWifiCallingEnabled)
{
    OsPhoneInfoCall objOsPhoneInfoCall(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, IsWifiCallingEnabled(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_EQ(IMS_TRUE, objOsPhoneInfoCall.IsWifiCallingEnabled());
}

TEST_F(OsPhoneInfoCallTest, GetWifiCallingPreferences)
{
    OsPhoneInfoCall objOsPhoneInfoCall(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, GetWifiCallingPreferences(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(ICallInfo::WFC_MODE_WFC_PREFERRED));

    EXPECT_EQ(ICallInfo::WFC_MODE_WFC_PREFERRED, objOsPhoneInfoCall.GetWifiCallingPreferences());
}

TEST_F(OsPhoneInfoCallTest, IsWifiCallingProvisioned)
{
    OsPhoneInfoCall objOsPhoneInfoCall(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, IsWifiCallingProvisioned(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_EQ(IMS_TRUE, objOsPhoneInfoCall.IsWifiCallingProvisioned());
}

TEST_F(OsPhoneInfoCallTest, GetWifiCallingAddressId)
{
    OsPhoneInfoCall objOsPhoneInfoCall(IMS_SLOT_0);

    AString strWifiAddrId("Wifi-Address-Id");

    EXPECT_CALL(m_objSystem, GetWifiCallingAddressId(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(strWifiAddrId));

    EXPECT_EQ(strWifiAddrId, objOsPhoneInfoCall.GetWifiCallingAddressId());
}

TEST_F(OsPhoneInfoCallTest, GetCsCallStateInOtherSlot)
{
    OsPhoneInfoCall objOsPhoneInfoCall(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, GetCsCallStateInOtherSlot(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(1));

    EXPECT_EQ(1, objOsPhoneInfoCall.GetCsCallStateInOtherSlot());
}

TEST_F(OsPhoneInfoCallTest, IsCrossSimRedialingAvailable)
{
    OsPhoneInfoCall objOsPhoneInfoCall(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, IsCrossSimRedialingAvailable(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_EQ(IMS_TRUE, objOsPhoneInfoCall.IsCrossSimRedialingAvailable());
}

}  // namespace android
