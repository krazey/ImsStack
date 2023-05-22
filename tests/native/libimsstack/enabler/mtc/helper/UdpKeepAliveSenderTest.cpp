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

#include "MockIMtcService.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "call/MockIMtcCallContext.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/UdpKeepAliveSender.h"
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;

LOCAL AString LOCAL_IP_ADDRESS = "192.168.0.1";
LOCAL IMS_UINT32 LOCAL_PORT_NUMBER = 12345;
LOCAL AString PCSCF_IP_ADDRESS = "192.168.0.10";
LOCAL IMS_UINT32 PCSCF_PORT_NUMBER = 5060;

class UdpKeepAliveSenderTest : public ::testing::Test
{
public:
    inline UdpKeepAliveSenderTest() :
            pConfigurationManager(IMS_NULL),
            pConfigurationProxy(IMS_NULL),
            objTimerService(),
            objTimer(objTimerService.GetMockTimer()),
            pSender(IMS_NULL)
    {
    }

    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;
    TestTimerService objTimerService;
    MockITimer& objTimer;
    MockIMtcAosConnector objAosConnector;

    UdpKeepAliveSender* pSender;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);

        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
        ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(LOCAL_IP_ADDRESS));
        ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(LOCAL_PORT_NUMBER));
        ON_CALL(objAosConnector, GetPcscfAddress).WillByDefault(Return(PCSCF_IP_ADDRESS));
        ON_CALL(objAosConnector, GetPcscfPort).WillByDefault(Return(PCSCF_PORT_NUMBER));
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }
};

TEST_F(UdpKeepAliveSenderTest, IsRequiredChecksConfiguration)
{
    ON_CALL(*pConfigurationManager, GetSendUdpKeepAliveIntervalTime).WillByDefault(Return(-1));
    EXPECT_FALSE(pSender->IsRequired(*pConfigurationProxy));

    ON_CALL(*pConfigurationManager, GetSendUdpKeepAliveIntervalTime).WillByDefault(Return(0));
    EXPECT_FALSE(pSender->IsRequired(*pConfigurationProxy));

    ON_CALL(*pConfigurationManager, GetSendUdpKeepAliveIntervalTime).WillByDefault(Return(2000));
    EXPECT_TRUE(pSender->IsRequired(*pConfigurationProxy));
}

TEST_F(UdpKeepAliveSenderTest, ConstructorStartsAndDestructorStopsKeepAlive)
{
    // TODO: add MockISipKeepAliveHelper expectation.
    IMS_SINT32 nAnyKeepAliveTime = 2000;
    ON_CALL(*pConfigurationManager, GetSendUdpKeepAliveIntervalTime)
            .WillByDefault(Return(nAnyKeepAliveTime));

    pSender = new UdpKeepAliveSender(objContext);
    EXPECT_CALL(objTimer, SetTimer(nAnyKeepAliveTime, pSender));
    pSender->Start();

    EXPECT_CALL(objTimer, KillTimer);
    delete pSender;
}

TEST_F(UdpKeepAliveSenderTest, StopStopsKeepAlive)
{
    IMS_SINT32 nAnyKeepAliveTime = 2000;
    ON_CALL(*pConfigurationManager, GetSendUdpKeepAliveIntervalTime)
            .WillByDefault(Return(nAnyKeepAliveTime));

    pSender = new UdpKeepAliveSender(objContext);
    pSender->Start();

    EXPECT_CALL(objTimer, KillTimer);
    pSender->Stop();

    delete pSender;
}

TEST_F(UdpKeepAliveSenderTest, TimerExpiredInvokesReStart)
{
    // TODO: add MockISipKeepAliveHelper expectation.

    pSender = new UdpKeepAliveSender(objContext);
    pSender->Start();

    // 1st is by timerexpired, 2nd is by the destructor.
    EXPECT_CALL(objTimer, KillTimer).Times(2);

    pSender->Timer_TimerExpired(&objTimer);

    delete pSender;
}

TEST_F(UdpKeepAliveSenderTest, InvalidTimerExpiredInvokesNothing)
{
    pSender = new UdpKeepAliveSender(objContext);
    pSender->Start();

    EXPECT_CALL(objTimer, KillTimer).Times(0);
    pSender->Timer_TimerExpired(IMS_NULL);
    MockITimer* pDiffTimer = new MockITimer();
    pSender->Timer_TimerExpired(pDiffTimer);

    EXPECT_CALL(objTimer, KillTimer).Times(1);
    delete pSender;
}
