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
#include "MockISipKeepAliveHelper.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "call/MockIMtcCallContext.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/UdpKeepAliveSender.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL const IMS_SINT32 ANY_KEEP_ALIVE_INTERVAL = 2000;

LOCAL const AString LOCAL_IP_ADDRESS = "192.168.0.1";
LOCAL const IMS_UINT32 LOCAL_PORT_NUMBER = 12345;
LOCAL const AString PCSCF_IP_ADDRESS = "192.168.0.10";
LOCAL const IMS_UINT32 PCSCF_PORT_NUMBER = 5060;

class UdpKeepAliveSenderTest : public ::testing::Test
{
public:
    inline UdpKeepAliveSenderTest() :
            pConfigurationProxy(),
            objTimerService(),
            objTimer(objTimerService.GetMockTimer()),
            pSender(IMS_NULL)
    {
    }

    MockIMtcCallContext objContext;
    MockIMtcService objService;
    MockMtcConfigurationProxy* pConfigurationProxy;
    TestTimerService objTimerService;
    MockITimer& objTimer;
    MockIMtcAosConnector objAosConnector;
    MockISipKeepAliveHelper objKeepAliveHelper;

    UdpKeepAliveSender* pSender;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);

        ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));
        pConfigurationProxy = new MockMtcConfigurationProxy();
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(*pConfigurationProxy,
                GetInt(ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT))
                .WillByDefault(Return(ANY_KEEP_ALIVE_INTERVAL));

        ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
        ON_CALL(objAosConnector, GetLocalAddress).WillByDefault(Return(LOCAL_IP_ADDRESS));
        ON_CALL(objAosConnector, GetLocalPort).WillByDefault(Return(LOCAL_PORT_NUMBER));
        ON_CALL(objAosConnector, GetPcscfAddress).WillByDefault(Return(PCSCF_IP_ADDRESS));
        ON_CALL(objAosConnector, GetPcscfPort).WillByDefault(Return(PCSCF_PORT_NUMBER));

        pSender = new UdpKeepAliveSender(&objKeepAliveHelper, objContext);
    }

    virtual void TearDown() override
    {
        delete pSender;
        delete pConfigurationProxy;

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }
};

TEST_F(UdpKeepAliveSenderTest, IsRequiredChecksConfiguration)
{
    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT))
            .WillByDefault(Return(-1));
    EXPECT_FALSE(UdpKeepAliveSender::IsRequired(*pConfigurationProxy));

    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT))
            .WillByDefault(Return(0));
    EXPECT_FALSE(UdpKeepAliveSender::IsRequired(*pConfigurationProxy));

    ON_CALL(*pConfigurationProxy,
            GetInt(ConfigVoice::KEY_SEND_UDP_KEEP_ALIVE_INTERVAL_TIME_MILLIS_INT))
            .WillByDefault(Return(2000));
    EXPECT_TRUE(UdpKeepAliveSender::IsRequired(*pConfigurationProxy));
}

TEST_F(UdpKeepAliveSenderTest, StartStartsKeepAlive)
{
    EXPECT_CALL(objKeepAliveHelper, SendPacket(_));
    EXPECT_CALL(objTimer, SetTimer(ANY_KEEP_ALIVE_INTERVAL, pSender));

    pSender->Start();
}

TEST_F(UdpKeepAliveSenderTest, DestructorStopsKeepAlive)
{
    pSender->Start();

    EXPECT_CALL(objTimer, KillTimer);

    delete pSender;
    pSender = IMS_NULL;
}

TEST_F(UdpKeepAliveSenderTest, StopStopsKeepAlive)
{
    pSender->Start();

    EXPECT_CALL(objTimer, KillTimer);

    pSender->Stop();
}

TEST_F(UdpKeepAliveSenderTest, TimerExpiredRestartsKeepAlive)
{
    pSender->Start();

    EXPECT_CALL(objKeepAliveHelper, SendPacket(_));

    pSender->Timer_TimerExpired(&objTimer);
}

TEST_F(UdpKeepAliveSenderTest, InvalidTimerExpiredInvokesNothing)
{
    pSender->Start();

    EXPECT_CALL(objTimer, KillTimer).Times(1);  // Once by destructor
    EXPECT_CALL(objKeepAliveHelper, SendPacket(_)).Times(0);

    pSender->Timer_TimerExpired(IMS_NULL);
    MockITimer* pNotRelatedTimer = new MockITimer();
    pSender->Timer_TimerExpired(pNotRelatedTimer);
}
