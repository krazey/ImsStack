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

#include "MockIPhoneInfoPower.h"
#include "MockISystem.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "SystemConstants.h"
#include "TestPhoneInfoService.h"
#include "TestThreadService.h"
#include "device/OsPowerInfo.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;

namespace android
{

class OsPowerInfoTest : public ::testing::Test
{
public:
    MockISystem m_objMockSystem;
    MockIThread m_objMockThread;
    MockIPowerInfoListener m_objPowerInfoListener;

    ISystem* m_piDefaultSystem;
    ISystemListener* m_piSystemListener;
    OsPowerInfo* m_pOsPowerInfo;

    TestPhoneInfoService m_objPhoneInfoService;
    TestThreadService m_objThreadService;

protected:
    virtual void SetUp() override
    {
        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);

        EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
        m_piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&m_objMockSystem);

        m_pOsPowerInfo = new OsPowerInfo();
        ASSERT_TRUE(m_pOsPowerInfo != nullptr);

        m_piSystemListener = static_cast<ISystemListener*>(m_pOsPowerInfo);
    }

    virtual void TearDown() override
    {
        if (m_pOsPowerInfo != IMS_NULL)
        {
            EXPECT_CALL(m_objMockSystem, RemoveListener(_, _, _)).Times(1);
            delete m_pOsPowerInfo;
            m_pOsPowerInfo = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
    }
};

TEST_F(OsPowerInfoTest, GetPowerLevel)
{
    EXPECT_EQ(POWERLEVEL_OFF, m_pOsPowerInfo->GetPowerLevel());
}

TEST_F(OsPowerInfoTest, NotifyData)
{
    m_objPhoneInfoService.SetPowerInfo(m_pOsPowerInfo);
    m_pOsPowerInfo->RegisterObserver(&m_objPowerInfoListener);

    EXPECT_CALL(m_objMockThread, PostMessageI(_))
            .Times(AnyNumber())
            .WillRepeatedly(Invoke(
                    [&](IN ImsMessage& objMsg)
                    {
                        m_objPhoneInfoService.DispatchServiceMessage(objMsg);
                        return IMS_TRUE;
                    }));
    EXPECT_CALL(m_objPowerInfoListener, PowerInfo_NotifyPowerLevel(_)).Times(4);

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_BATTERY_CHANGED, 16, 0);

    EXPECT_EQ(POWERLEVEL_HIGH, m_pOsPowerInfo->GetPowerLevel());

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_BATTERY_CHANGED, 2, 0);

    EXPECT_EQ(POWERLEVEL_LOW, m_pOsPowerInfo->GetPowerLevel());

    m_pOsPowerInfo->RemoveObserver(&m_objPowerInfoListener);
}

}  // namespace android
