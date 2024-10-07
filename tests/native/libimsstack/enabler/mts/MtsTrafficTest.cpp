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

#include "IImsRadio.h"
#include "MockIMtsTrafficListener.h"
#include "MockITimer.h"
#include "MtsTraffic.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include <gtest/gtest.h>

namespace android
{

class MtsTrafficTest : public ::testing::Test
{
public:
    inline MtsTrafficTest() :
            pMtsTraffic(IMS_NULL),
            pTimerService(new TestTimerService()),
            objTimer(pTimerService->GetMockTimer())
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, pTimerService);
    }
    inline virtual ~MtsTrafficTest()
    {
        delete pTimerService;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

    MtsTraffic* pMtsTraffic;
    TestTimerService* pTimerService;

    MockIMtsTrafficListener objMockMtsTrafficListener;
    MockITimer& objTimer;

protected:
    virtual void SetUp() override
    {
        pMtsTraffic = new MtsTraffic(
                IImsRadio::DIRECTION_MO, IImsRadio::TRAFFIC_TYPE_SMS, objMockMtsTrafficListener);
    }

    virtual void TearDown() override { delete pMtsTraffic; }
};

TEST_F(MtsTrafficTest, Constructor)
{
    ASSERT_NE(pMtsTraffic, nullptr);
}

TEST_F(MtsTrafficTest, ImsRadio_OnConnectionFailed)
{
    EXPECT_CALL(objMockMtsTrafficListener,
            Traffic_OnConnectionFailed(
                    IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO, 0, 0, 0))
            .Times(1);
    pMtsTraffic->ImsRadio_OnConnectionFailed(0, 0, 0);
}

TEST_F(MtsTrafficTest, ImsRadio_OnConnectionSetupPrepared)
{
    EXPECT_CALL(objMockMtsTrafficListener,
            Traffic_OnConnectionSetupPrepared(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO))
            .Times(1);
    pMtsTraffic->ImsRadio_OnConnectionSetupPrepared();
}

TEST_F(MtsTrafficTest, Timer_TimerExpired)
{
    EXPECT_CALL(objMockMtsTrafficListener,
            Traffic_GuardTimerExpired(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO))
            .Times(1);
    pMtsTraffic->StartRadioGuardTimer();
    pMtsTraffic->Timer_TimerExpired(&objTimer);
}

TEST_F(MtsTrafficTest, Timer_TimerExpiredWithNullTimer)
{
    EXPECT_CALL(objMockMtsTrafficListener,
            Traffic_GuardTimerExpired(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO))
            .Times(0);
    pMtsTraffic->StartRadioGuardTimer();
    pMtsTraffic->Timer_TimerExpired(IMS_NULL);
}

}  // namespace android
