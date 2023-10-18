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
#include "MockINetworkWatcher.h"
#include "MockIPhoneInfoPower.h"
#include "MockIWifiWatcher.h"
#include "MockISystem.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include "ServicePhoneInfo.h"

using ::testing::_;
using ::testing::AnyNumber;

namespace android
{

class PhoneInfoServiceTest : public ::testing::Test
{
public:
    MockISystem m_objMockSystem;
    MockIThread m_objMockThread;
    MockIPowerInfoListener m_objMockPowerInfoListener;
    MockINetworkWatcherListener m_objMockNetworkWatcherListener;
    MockIWifiWatcherListener m_objMockWifiWatcherListener;

    ISystem* m_piDefaultSystem;
    PhoneInfoService* m_pPhoneInfoService;

    TestThreadService m_objThreadService;

protected:
    virtual void SetUp() override
    {
        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);

        m_piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&m_objMockSystem);

        m_pPhoneInfoService = new PhoneInfoService();

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, m_pPhoneInfoService);
        ASSERT_TRUE(PhoneInfoService::GetPhoneInfoService() == m_pPhoneInfoService);
    }

    virtual void TearDown() override
    {
        if (m_pPhoneInfoService != IMS_NULL)
        {
            EXPECT_CALL(m_objMockSystem, RemoveListener(_, _, _)).Times(AnyNumber());
            m_pPhoneInfoService->Destroy();
            m_pPhoneInfoService = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
    }
};

TEST_F(PhoneInfoServiceTest, GetPhoneInfos)
{
    ASSERT_NE(m_pPhoneInfoService->GetDeviceInfo(), nullptr);

    ASSERT_NE(m_pPhoneInfoService->GetCallInfo(IMS_SLOT_0), nullptr);
    ASSERT_NE(m_pPhoneInfoService->GetLocationInfo(IMS_SLOT_0), nullptr);
    ASSERT_NE(m_pPhoneInfoService->GetSubscriberInfo(IMS_SLOT_0), nullptr);
}

TEST_F(PhoneInfoServiceTest, GetPowerInfo)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    IPowerInfo* piPowerInfo = m_pPhoneInfoService->GetPowerInfo();
    ASSERT_NE(piPowerInfo, nullptr);

    piPowerInfo->RegisterObserver(&m_objMockPowerInfoListener);

    ImsMessage objMsg(IMS_MSG_BATTERY, 0, 0, IMS_NULL);

    EXPECT_CALL(m_objMockPowerInfoListener, PowerInfo_NotifyPowerLevel(piPowerInfo)).Times(1);
    m_pPhoneInfoService->DispatchServiceMessage(objMsg);

    piPowerInfo->RemoveObserver(&m_objMockPowerInfoListener);
}

TEST_F(PhoneInfoServiceTest, GetIsim)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    IIsim* pIsim = m_pPhoneInfoService->GetIsim(IMS_SLOT_0);
    ASSERT_NE(pIsim, nullptr);
}

TEST_F(PhoneInfoServiceTest, GetUsim)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    ASSERT_NE(m_pPhoneInfoService->GetUsim(IMS_SLOT_0), nullptr);
}

TEST_F(PhoneInfoServiceTest, GetNetworkWatcher)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    INetworkWatcher* pNetworkWatcher = m_pPhoneInfoService->GetNetworkWatcher(IMS_SLOT_0);
    ASSERT_NE(pNetworkWatcher, nullptr);

    pNetworkWatcher->RegisterObserver(&m_objMockNetworkWatcherListener);

    ImsMessage objMsg(IMS_MSG_NETWORK_STATUS, 0, 0, IMS_NULL);

    EXPECT_CALL(m_objMockNetworkWatcherListener, NetworkWatcher_NotifyStatus(pNetworkWatcher))
            .Times(1);
    m_pPhoneInfoService->DispatchServiceMessage(objMsg);

    pNetworkWatcher->RemoveObserver(&m_objMockNetworkWatcherListener);
}

TEST_F(PhoneInfoServiceTest, GetWifiWatcher)
{
    EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
    IWifiWatcher* pWifiWatcher = m_pPhoneInfoService->GetWifiWatcher();
    ASSERT_NE(pWifiWatcher, nullptr);

    pWifiWatcher->RegisterObserver(&m_objMockWifiWatcherListener);

    ImsMessage objMsg(IMS_MSG_WIFI_STATUS, 0, 0, IMS_NULL);

    EXPECT_CALL(m_objMockWifiWatcherListener, WifiWatcher_NotifyStateChanged(pWifiWatcher))
            .Times(1);
    m_pPhoneInfoService->DispatchServiceMessage(objMsg);

    pWifiWatcher->RemoveObserver(&m_objMockWifiWatcherListener);
}

}  // namespace android
