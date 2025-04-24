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
    MockIMtcNetworkWatcherListener objNetworkWatcherListener;

protected:
    virtual void SetUp() override
    {
        pOldPhoneInfoService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &objPhoneInfoService);
        ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
                .WillByDefault(Return(NW_REPORT_RADIO_LTE));
        ON_CALL(objMtcService, IsActive).WillByDefault(Return(IMS_TRUE));
        ON_CALL(objMtcService, GetServiceType).WillByDefault(Return(ServiceType::NORMAL));
        ON_CALL(objMtcService, IsWlanIpCanType).WillByDefault(Return(IMS_FALSE));

        pNetworkWatcher = new MtcNetworkWatcher(objMtcService, 0);
        pNetworkWatcher->AddListener(objNetworkWatcherListener);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, pOldPhoneInfoService);
        delete pNetworkWatcher;
    }
};

TEST_F(MtcNetworkWatcherTest, GetMobileRatTypeReturnsConvertedType)
{
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));
    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_NR, pNetworkWatcher->GetMobileRatType());

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));
    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetMobileRatType());

    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_WCDMA));
    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_UNKNOWN, pNetworkWatcher->GetMobileRatType());
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenMobileRatChangedOnMobileIpcan)
{
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_MOBILE);
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(objNetworkWatcherListener,
            OnRatChanged(
                    _, INetworkWatcher::RADIOTECH_TYPE_LTE, INetworkWatcher::RADIOTECH_TYPE_NR));
    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_NR, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_NR, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_NR, pNetworkWatcher->GetLastConnectedRatType());
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenMobileRatDoesNotChangedOnMobileIpcan)
{
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_MOBILE);
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(_, _, _)).Times(0);
    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetLastConnectedRatType());
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenMobileRatChangedAsInvalidOne)
{
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_MOBILE);
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_INVALID));

    EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(_, _, _)).Times(0);
    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenMobileRatChangedOnWlanIpcan)
{
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_WLAN);
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(_, _, _)).Times(0);
    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_IWLAN, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_NR, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_IWLAN, pNetworkWatcher->GetLastConnectedRatType());
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenMobileRatDoesNotChangedOnWlanIpcan)
{
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_WLAN);
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_LTE));

    EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(_, _, _)).Times(0);
    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_IWLAN, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_IWLAN, pNetworkWatcher->GetLastConnectedRatType());
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenMobileRatChangedDuringServiceDisconnected)
{
    pNetworkWatcher->OnDisconnected();
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(_, _, _)).Times(0);
    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_INVALID, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_INVALID, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetLastConnectedRatType());
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenMobileRatChangedDuringServiceDisconnectedAndConnected)
{
    pNetworkWatcher->OnDisconnected();
    ON_CALL(objPhoneInfoService.GetMockNetworkWatcher(), GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_NR));

    EXPECT_CALL(objNetworkWatcherListener,
            OnRatChanged(_, INetworkWatcher::RADIOTECH_TYPE_INVALID,
                    INetworkWatcher::RADIOTECH_TYPE_NR));
    pNetworkWatcher->NetworkWatcher_NotifyStatus(objPhoneInfoService.GetNetworkWatcher(0));
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_NR, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_NR, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_NR, pNetworkWatcher->GetLastConnectedRatType());
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenServiceConnectedWithDifferentIpcan)
{
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_WLAN);

    EXPECT_CALL(objNetworkWatcherListener,
            OnRatChanged(
                    _, INetworkWatcher::RADIOTECH_TYPE_IWLAN, INetworkWatcher::RADIOTECH_TYPE_LTE));
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetLastConnectedRatType());

    EXPECT_CALL(objNetworkWatcherListener,
            OnRatChanged(
                    _, INetworkWatcher::RADIOTECH_TYPE_LTE, INetworkWatcher::RADIOTECH_TYPE_IWLAN));
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_WLAN);
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_IWLAN, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_IWLAN, pNetworkWatcher->GetLastConnectedRatType());

    EXPECT_CALL(objNetworkWatcherListener,
            OnRatChanged(
                    _, INetworkWatcher::RADIOTECH_TYPE_IWLAN, INetworkWatcher::RADIOTECH_TYPE_LTE));
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetLastConnectedRatType());
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenServiceConnectedWithSameIpcan)
{
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_WLAN);

    EXPECT_CALL(objNetworkWatcherListener, OnRatChanged(_, _, _)).Times(0);
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_WLAN);
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_IWLAN, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_IWLAN, pNetworkWatcher->GetLastConnectedRatType());
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenServiceDisconnectedOnWlan)
{
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_WLAN);

    EXPECT_CALL(objNetworkWatcherListener,
            OnRatChanged(_, INetworkWatcher::RADIOTECH_TYPE_IWLAN,
                    INetworkWatcher::RADIOTECH_TYPE_INVALID));
    pNetworkWatcher->OnDisconnected();
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_INVALID, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_INVALID, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_IWLAN, pNetworkWatcher->GetLastConnectedRatType());
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenServiceDisconnectedOnLte)
{
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_MOBILE);

    EXPECT_CALL(objNetworkWatcherListener,
            OnRatChanged(_, INetworkWatcher::RADIOTECH_TYPE_LTE,
                    INetworkWatcher::RADIOTECH_TYPE_INVALID));
    pNetworkWatcher->OnDisconnected();
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_INVALID, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_INVALID, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetLastConnectedRatType());
}

TEST_F(MtcNetworkWatcherTest, UpdatesWhenServiceDisconnectedAndConnectedOnMobileIpcan)
{
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_MOBILE);
    pNetworkWatcher->OnDisconnected();

    EXPECT_CALL(objNetworkWatcherListener,
            OnRatChanged(_, INetworkWatcher::RADIOTECH_TYPE_INVALID,
                    INetworkWatcher::RADIOTECH_TYPE_LTE));
    pNetworkWatcher->OnConnected(IIpcan::CATEGORY_MOBILE);
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetMobileRatType());
    EXPECT_EQ(INetworkWatcher::RADIOTECH_TYPE_LTE, pNetworkWatcher->GetLastConnectedRatType());
}

}  // namespace android
