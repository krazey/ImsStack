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

#include "ImsTypeDef.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "precondition/IQosTimerListener.h"
#include "precondition/QosTimer.h"
#include <gtest/gtest.h>

using ::testing::Return;

namespace android
{

class QosTimerTest : public ::testing::Test, public IQosTimerListener
{
public:
    TestTimerService objTimerService;
    IMS_BOOL bTimerExpired;
    IMS_UINT32 nAnyDuration = 10000;
    QosTimer* pQosTimer;

    void OnTimerExpired(IN QosTimer* /*pTimer*/, IN QosTimerType /*eType*/) override
    {
        bTimerExpired = IMS_TRUE;
    }

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);

        bTimerExpired = IMS_FALSE;
        pQosTimer = new QosTimer(this);
    }

    virtual void TearDown() override
    {
        delete pQosTimer;

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }
};

TEST_F(QosTimerTest, StartQosTimerStopQosTimerAddsRemovesTimer)
{
    pQosTimer->StartQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE, nAnyDuration);

    EXPECT_TRUE(pQosTimer->IsQosTimerActivated(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE));
    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AFTER_LOST));

    pQosTimer->StopQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE);

    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE));
    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AFTER_LOST));
}

TEST_F(QosTimerTest, TimerTimerExpiredRemovesTimerAndNotifies)
{
    pQosTimer->StartQosTimer(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE, nAnyDuration);

    EXPECT_TRUE(pQosTimer->IsQosTimerActivated(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE));
    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AFTER_LOST));

    pQosTimer->Timer_TimerExpired(&(objTimerService.GetMockTimer()));

    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::WAIT_VIDEO_TEXT_AVAILABLE));
    EXPECT_FALSE(pQosTimer->IsQosTimerActivated(QosTimerType::GUARD_AFTER_LOST));
    EXPECT_TRUE(bTimerExpired);
}

TEST_F(QosTimerTest, DestructorClearsITimer)
{
    MockITimer objMockITimer;
    objTimerService.SetTimer(&objMockITimer);

    QosTimer* pQosTimerForDestructor = new QosTimer(this);
    pQosTimerForDestructor->StartQosTimer(
            QosTimerType::WAIT_AVAILABLE_AFTER_W2L_HANDOVER, nAnyDuration);
    EXPECT_CALL(objMockITimer, KillTimer);

    delete pQosTimerForDestructor;
}

}  // namespace android
