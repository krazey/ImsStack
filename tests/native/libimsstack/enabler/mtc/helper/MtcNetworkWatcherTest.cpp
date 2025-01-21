/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "IIpcan.h"
#include "INetworkWatcher.h"
#include "MockIMtcService.h"
#include "PlatformContext.h"
#include "TestPhoneInfoService.h"
#include "helper/MockIMtcNetworkWatcherListener.h"
#include "helper/MtcNetworkWatcher.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class MtcNetworkWatcherTest : public ::testing::Test
{
public:
    MockIMtcService objMtcService;
    MtcNetworkWatcher* pNetworkWatcher;
    TestPhoneInfoService objPhoneInfoService;
    PlatformService* pOldPhoneInfoService;

protected:
    virtual void SetUp() override
    {
        pOldPhoneInfoService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);
        ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
                .WillByDefault(Return(NW_REPORT_RADIO_LTE));
        ON_CALL(objMtcService, GetServiceType).WillByDefault(Return(ServiceType::NORMAL));
        ON_CALL(objMtcService, IsWlanIpCanType).WillByDefault(Return(IMS_TRUE));

        pNetworkWatcher = new MtcNetworkWatcher(objMtcService, 0);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, pOldPhoneInfoService);
        delete pNetworkWatcher;
    }
};

TEST_F(MtcNetworkWatcherTest, GetRatTypeReturnsCorrectValue)
{
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_IWLAN, pNetworkWatcher->GetRatType());
}

TEST_F(MtcNetworkWatcherTest, GetMobileRatTypeReturnsCorrectValue)
{
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetMobileRatType());
}

TEST_F(MtcNetworkWatcherTest, OnServiceConnectedInvokesNotifying)
{
    MockIMtcNetworkWatcherListener objNetworkWatcherListener;
    pNetworkWatcher->AddListener(objNetworkWatcherListener);
    EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(INetworkWatcher::RADIOTECH_TYPE_LTE))
            .Times(1);

    pNetworkWatcher->OnServiceConnected(IIpcan::CATEGORY_MOBILE);

    EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(INetworkWatcher::RADIOTECH_TYPE_LTE))
            .Times(0);
    pNetworkWatcher->OnServiceConnected(IIpcan::CATEGORY_MOBILE);

    pNetworkWatcher->RemoveListener(objNetworkWatcherListener);
    EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(INetworkWatcher::RADIOTECH_TYPE_LTE))
            .Times(0);
    pNetworkWatcher->OnServiceConnected(IIpcan::CATEGORY_WLAN);
}

TEST_F(MtcNetworkWatcherTest, NetworkWatcher_NotifyStatusInvokesNotifying)
{
    MockIMtcNetworkWatcherListener objNetworkWatcherListener;
    pNetworkWatcher->AddListener(objNetworkWatcherListener);
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));

    EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(INetworkWatcher::RADIOTECH_TYPE_NR));

    pNetworkWatcher->OnServiceConnected(IIpcan::CATEGORY_MOBILE);

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_GSM));
    EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(INetworkWatcher::RADIOTECH_TYPE_UNKNOWN));

    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));
}

TEST_F(MtcNetworkWatcherTest, SetTestRatChangedInvokesNotifying)
{
    MockIMtcNetworkWatcherListener objNetworkWatcherListener;
    pNetworkWatcher->AddListener(objNetworkWatcherListener);
    pNetworkWatcher->OnServiceConnected(IIpcan::CATEGORY_MOBILE);

    for (IMS_SINT32 i = INetworkWatcher::RADIOTECH_TYPE_INVALID;
            i <= INetworkWatcher::RADIOTECH_TYPE_MAX; i++)
    {
        EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(i));
        pNetworkWatcher->SetTestRatChanged(i);
    }
}

TEST_F(MtcNetworkWatcherTest, SetTestRatChangedDoesNotInvokeNotifyingIfIpcanIsWlan)
{
    MockIMtcNetworkWatcherListener objNetworkWatcherListener;
    pNetworkWatcher->AddListener(objNetworkWatcherListener);
    pNetworkWatcher->OnServiceConnected(IIpcan::CATEGORY_WLAN);

    for (IMS_SINT32 i = INetworkWatcher::RADIOTECH_TYPE_INVALID;
            i <= INetworkWatcher::RADIOTECH_TYPE_MAX; i++)
    {
        EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(_)).Times(0);
        pNetworkWatcher->SetTestRatChanged(i);
    }
}

}  // namespace android
