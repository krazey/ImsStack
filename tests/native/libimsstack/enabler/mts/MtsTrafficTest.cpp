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
#include "MockIMtsContext.h"
#include "MockIMtsMessageController.h"
#include "MockIMtsTrafficListener.h"
#include "MockITimer.h"
#include "MtsTraffic.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;

class MtsTrafficTest : public ::testing::Test
{
public:
    inline MtsTrafficTest() :
            objMtsTraffics(ImsList<MtsTraffic*>()),
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

    ImsList<MtsTraffic*> objMtsTraffics;
    TestTimerService* pTimerService;

    MockIMtsContext objContext;
    MockIMtsMessageController objMessageController;
    MockIMtsTrafficListener objMockMtsTrafficListener;
    MockITimer& objTimer;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetMessageController).WillByDefault(ReturnRef(objMessageController));

        objMtsTraffics.Append(new MtsTraffic(objContext, IImsRadio::DIRECTION_MO,
                IImsRadio::TRAFFIC_TYPE_SMS, objMockMtsTrafficListener));
        objMtsTraffics.Append(new MtsTraffic(objContext, IImsRadio::DIRECTION_MT,
                IImsRadio::TRAFFIC_TYPE_SMS, objMockMtsTrafficListener));
        objMtsTraffics.Append(new MtsTraffic(objContext, IImsRadio::DIRECTION_MO,
                IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, objMockMtsTrafficListener));
        objMtsTraffics.Append(new MtsTraffic(objContext, IImsRadio::DIRECTION_MT,
                IImsRadio::TRAFFIC_TYPE_EMERGENCY_SMS, objMockMtsTrafficListener));
    }

    virtual void TearDown() override
    {
        for (IMS_UINT32 i = 0; i < objMtsTraffics.GetSize(); i++)
        {
            MtsTraffic* pTmpMtsTraffic = objMtsTraffics.GetAt(i);

            if (pTmpMtsTraffic)
            {
                delete pTmpMtsTraffic;
            }
        }

        objMtsTraffics.Clear();
    }

    MtsTraffic* GetTraffic(IN IMS_UINT32 nTrafficType, IN IMS_UINT32 nDirection)
    {
        for (IMS_UINT32 i = 0; i < objMtsTraffics.GetSize(); i++)
        {
            MtsTraffic* pTmpMtsTraffic = objMtsTraffics.GetAt(i);

            if ((pTmpMtsTraffic->GetDirection() == nDirection) &&
                    (pTmpMtsTraffic->GetTrafficType() == nTrafficType))
            {
                return pTmpMtsTraffic;
            }
        }

        return IMS_NULL;
    }
};

TEST_F(MtsTrafficTest, Constructor)
{
    for (IMS_UINT32 i = 0; i < objMtsTraffics.GetSize(); i++)
    {
        MtsTraffic* pTmpMtsTraffic = objMtsTraffics.GetAt(i);

        ASSERT_NE(pTmpMtsTraffic, nullptr);
    }
}

TEST_F(MtsTrafficTest, ImsRadio_OnConnectionFailedWithInternalErrorReason)
{
    MtsTraffic* pMtsTraffic = GetTraffic(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);

    EXPECT_CALL(objMockMtsTrafficListener,
            Traffic_OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO,
                    IImsRadio::REASON_INTERNAL_ERROR, 0, 0))
            .Times(1);
    pMtsTraffic->ImsRadio_OnConnectionFailed(IImsRadio::REASON_INTERNAL_ERROR, 0, 0);
}

TEST_F(MtsTrafficTest, ImsRadio_OnConnectionFailedWithIgnoredReason)
{
    MtsTraffic* pMtsTraffic = GetTraffic(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);

    EXPECT_CALL(objMockMtsTrafficListener,
            Traffic_OnConnectionFailed(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO,
                    IImsRadio::REASON_RF_BUSY, 0, 0))
            .Times(0);
    pMtsTraffic->ImsRadio_OnConnectionFailed(IImsRadio::REASON_RF_BUSY, 0, 0);
}

TEST_F(MtsTrafficTest, ImsRadio_OnConnectionSetupPrepared)
{
    MtsTraffic* pMtsTraffic = GetTraffic(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);

    EXPECT_CALL(objMockMtsTrafficListener,
            Traffic_OnConnectionSetupPrepared(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO))
            .Times(1);
    pMtsTraffic->ImsRadio_OnConnectionSetupPrepared();
}

TEST_F(MtsTrafficTest, Timer_TimerExpired)
{
    MtsTraffic* pMtsTraffic = GetTraffic(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT);

    EXPECT_CALL(objMockMtsTrafficListener,
            Traffic_GuardTimerExpired(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MT))
            .Times(1);
    pMtsTraffic->StartRadioGuardTimer();
    pMtsTraffic->Timer_TimerExpired(&objTimer);
}

TEST_F(MtsTrafficTest, Timer_TimerExpiredAndNoMoPending)
{
    MtsTraffic* pMtsTraffic = GetTraffic(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);

    EXPECT_CALL(objMockMtsTrafficListener,
            Traffic_GuardTimerExpired(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO))
            .Times(1);
    EXPECT_CALL(objMessageController, HasPendingMoSms()).Times(1).WillOnce(Return(IMS_FALSE));

    pMtsTraffic->StartRadioGuardTimer();
    pMtsTraffic->Timer_TimerExpired(&objTimer);
}

TEST_F(MtsTrafficTest, Timer_TimerExpiredWhenExtendedGuardTimerExpired)
{
    MtsTraffic* pMtsTraffic = GetTraffic(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);

    EXPECT_CALL(objMockMtsTrafficListener,
            Traffic_GuardTimerExpired(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO))
            .Times(1);
    EXPECT_CALL(objMessageController, HasPendingMoSms()).Times(1).WillOnce(Return(IMS_TRUE));

    pMtsTraffic->StartRadioGuardTimer();
    pMtsTraffic->Timer_TimerExpired(&objTimer);
    pMtsTraffic->StartRadioGuardTimer(MTS_RADIO_EXTENDED_GUARD_TIMER_MS);
    pMtsTraffic->Timer_TimerExpired(&objTimer);
}

TEST_F(MtsTrafficTest, Timer_TimerExpiredWithNullTimer)
{
    MtsTraffic* pMtsTraffic = GetTraffic(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO);

    EXPECT_CALL(objMockMtsTrafficListener,
            Traffic_GuardTimerExpired(IImsRadio::TRAFFIC_TYPE_SMS, IImsRadio::DIRECTION_MO))
            .Times(0);
    pMtsTraffic->StartRadioGuardTimer();
    pMtsTraffic->Timer_TimerExpired(IMS_NULL);
}

}  // namespace android
