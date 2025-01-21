/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "IMtcService.h"
#include "INetworkWatcher.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "PlatformContext.h"
#include "TestImsRadioService.h"
#include "TestSystemTimeService.h"
#include "call/IMtcCall.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "helper/SsacTimerHandler.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class SsacTimerHandlerTest : public ::testing::Test
{
public:
    MockIMtcContext objContext;
    MockIMtcService objService;
    MockIPassiveTimerHolder objPassiveTimerHolder;
    TestImsRadioService objImsRadioService;
    TestSystemTimeService objSystemTimeService;
    SsacInfo objSsacInfo;

    SsacTimerHandler* pSsacTimerHandler;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_RADIO, &objImsRadioService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_SYSTEM_TIME, &objSystemTimeService);

        objSsacInfo.nBarringTimeSecForVoice = 0;
        objSsacInfo.nBarringTimeSecForVideo = 0;
        ON_CALL(objImsRadioService.GetMockImsRadio(), GetSsacInfo())
                .WillByDefault(ReturnRef(objSsacInfo));

        ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(&objService));
        ON_CALL(objContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimerHolder));

        pSsacTimerHandler = new SsacTimerHandler(objContext);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_RADIO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_SYSTEM_TIME, IMS_NULL);

        ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL)).WillByDefault(Return(IMS_NULL));
        delete pSsacTimerHandler;
    }
};

TEST_F(SsacTimerHandlerTest, IsSsacTimerRunningReturnsCorrectValue)
{
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(IMS_FALSE, pSsacTimerHandler->IsSsacTimerRunning(CallType::UNKNOWN));
    EXPECT_EQ(IMS_FALSE, pSsacTimerHandler->IsSsacTimerRunning(CallType::RTT));
    EXPECT_EQ(IMS_TRUE, pSsacTimerHandler->IsSsacTimerRunning(CallType::VIDEO_RTT));

    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_EQ(IMS_TRUE, pSsacTimerHandler->IsSsacTimerRunning(CallType::UNKNOWN));
    EXPECT_EQ(IMS_TRUE, pSsacTimerHandler->IsSsacTimerRunning(CallType::RTT));
    EXPECT_EQ(IMS_TRUE, pSsacTimerHandler->IsSsacTimerRunning(CallType::VIDEO_RTT));
}

TEST_F(SsacTimerHandlerTest, StartBarringInvokesAddNetworkWatcherListenerAndAddTimer)
{
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING))
            .WillByDefault(Return(IMS_FALSE));

    objSsacInfo.nBarringTimeSecForVoice = 10;
    ON_CALL(objSystemTimeService.GetMockSystemTime(), GetRandom(_)).WillByDefault(Return(10));

    EXPECT_CALL(objService, AddNetworkWatcherListener(_)).Times(0);
    EXPECT_CALL(
            objPassiveTimerHolder, AddTimer(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING, _, _))
            .Times(1);
    pSsacTimerHandler->StartBarringTimer(CallType::VOIP);

    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_CALL(objService, AddNetworkWatcherListener(_)).Times(1);
    EXPECT_CALL(
            objPassiveTimerHolder, AddTimer(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING, _, _))
            .Times(1);
    pSsacTimerHandler->StartBarringTimer(CallType::VOIP);
}

TEST_F(SsacTimerHandlerTest, OnPassiveTimerExpiredInvokesRemoveNetworkWatcherListener)
{
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_TRUE));
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objService, RemoveNetworkWatcherListener(_)).Times(0);

    pSsacTimerHandler->OnPassiveTimerExpired(IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING);

    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objService, RemoveNetworkWatcherListener(_)).Times(1);

    pSsacTimerHandler->OnPassiveTimerExpired(IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING);
}

TEST_F(SsacTimerHandlerTest, OnRatChangedInvokesClear)
{
    EXPECT_CALL(objPassiveTimerHolder, RemoveTimer(_)).Times(0);
    EXPECT_CALL(objService, RemoveNetworkWatcherListener(_)).Times(0);

    pSsacTimerHandler->OnRatChanged(ServiceType::NORMAL, INetworkWatcher::RADIOTECH_TYPE_LTE,
            INetworkWatcher::RADIOTECH_TYPE_LTE);

    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objPassiveTimerHolder, RemoveTimer(_)).Times(0);
    EXPECT_CALL(objService, RemoveNetworkWatcherListener(_)).Times(0);

    pSsacTimerHandler->OnRatChanged(ServiceType::NORMAL, INetworkWatcher::RADIOTECH_TYPE_LTE,
            INetworkWatcher::RADIOTECH_TYPE_IWLAN);

    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VOICE_BARRING))
            .WillByDefault(Return(IMS_FALSE));
    ON_CALL(objPassiveTimerHolder, IsActive(IPassiveTimerHolder::Type::SSAC_VIDEO_BARRING))
            .WillByDefault(Return(IMS_TRUE));
    // includes destructor operation.
    EXPECT_CALL(objPassiveTimerHolder, RemoveTimer(_)).Times(4);
    EXPECT_CALL(objService, RemoveNetworkWatcherListener(_)).Times(1);

    pSsacTimerHandler->OnRatChanged(ServiceType::NORMAL, INetworkWatcher::RADIOTECH_TYPE_LTE,
            INetworkWatcher::RADIOTECH_TYPE_IWLAN);
}
