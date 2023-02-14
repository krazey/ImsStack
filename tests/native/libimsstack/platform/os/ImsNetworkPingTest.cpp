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
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ImsNetworkPing.h"

#include "MockINetworkPingListener.h"
#include "MockISocket.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "TestNetworkService.h"
#include "TestTimerService.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

class ImsNetworkPingTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        m_pNetworkService = new TestNetworkService();
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, m_pNetworkService);
        m_pTimerService = new TestTimerService();
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, m_pTimerService);
    }

    virtual void TearDown() override
    {
        delete m_pNetworkService;
        delete m_pTimerService;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_NETWORK, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

protected:
    MockINetworkPingListener m_objINetworkPingListener;
    TestNetworkService* m_pNetworkService;
    TestTimerService* m_pTimerService;
};

TEST_F(ImsNetworkPingTest, Ping)
{
    IpAddress objSrcIp(AString("192.168.1.3"));
    IpAddress objDstIp(AString("192.168.1.5"));
    IMS_SINT32 nDstPort = 5060;
    IMS_SINT32 nWaitTime = 100;

    MockITimer& objMockTimer = m_pTimerService->GetMockTimer();
    MockISocket& objMockSocket = m_pNetworkService->GetMockSocket();

    EXPECT_CALL(objMockSocket, SetListener(_)).Times(AnyNumber());

    ImsNetworkPing* pImsNetworkPing = new ImsNetworkPing();
    ASSERT_TRUE(pImsNetworkPing != nullptr);

    EXPECT_CALL(objMockTimer, KillTimer()).Times(AnyNumber());
    EXPECT_CALL(objMockSocket, Close()).Times(AnyNumber());

    // 1. Socket open fail
    EXPECT_CALL(objMockSocket, Open(_, _)).Times(1).WillRepeatedly(Return(ISocket::RESULT_ERROR));
    EXPECT_EQ(INetworkPing::PING_STATUS_NOK,
            pImsNetworkPing->Ping(objSrcIp, objDstIp, nDstPort, nWaitTime));

    EXPECT_CALL(objMockSocket, Open(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISocket::RESULT_SUCCESS));
    EXPECT_CALL(objMockSocket, SetOption(_, _)).Times(AnyNumber());

    // 2. Socket bind fail
    EXPECT_CALL(objMockSocket, Bind(_, _)).Times(1).WillRepeatedly(Return(ISocket::RESULT_ERROR));
    EXPECT_EQ(INetworkPing::PING_STATUS_NOK,
            pImsNetworkPing->Ping(objSrcIp, objDstIp, nDstPort, nWaitTime));

    EXPECT_CALL(objMockSocket, Bind(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISocket::RESULT_SUCCESS));

    // 3. Socket connect fail - PING_STATUS_NOK
    EXPECT_CALL(objMockSocket, Connect(_, _))
            .Times(1)
            .WillRepeatedly(Return(ISocket::RESULT_ERROR));
    EXPECT_EQ(INetworkPing::PING_STATUS_NOK,
            pImsNetworkPing->Ping(objSrcIp, objDstIp, nDstPort, nWaitTime));

    // 4. all success - PING_STATUS_OK
    EXPECT_CALL(objMockSocket, Connect(_, _))
            .Times(1)
            .WillRepeatedly(Return(ISocket::RESULT_SUCCESS));
    EXPECT_EQ(INetworkPing::PING_STATUS_OK,
            pImsNetworkPing->Ping(objSrcIp, objDstIp, nDstPort, nWaitTime));

    pImsNetworkPing->SetListener(&m_objINetworkPingListener);

    ISocketListener* pISocketListener = IMS_NULL;

    ON_CALL(objMockSocket, SetListener)
            .WillByDefault(Invoke(
                    [&](ISocketListener* piListener)
                    {
                        pISocketListener = piListener;
                        return 1;
                    }));
    EXPECT_CALL(objMockSocket, Connect(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(ISocket::RESULT_WOULDBLOCK));
    EXPECT_CALL(objMockTimer, SetTimer(_, _)).Times(AnyNumber());

    // 5. Connection would block and socket connected - PING_STATUS_OK
    EXPECT_EQ(INetworkPing::PING_STATUS_PENDING,
            pImsNetworkPing->Ping(objSrcIp, objDstIp, nDstPort, nWaitTime));
    // already ping started and pending state
    EXPECT_EQ(INetworkPing::PING_STATUS_PENDING,
            pImsNetworkPing->Ping(objSrcIp, objDstIp, nDstPort, nWaitTime));
    // on socket connected, PING_STATUS_OK should be notified
    EXPECT_CALL(
            m_objINetworkPingListener, NetworkPing_NotifyResult(_, INetworkPing::PING_STATUS_OK))
            .Times(1);
    pISocketListener->Socket_OnConnected(&objMockSocket);

    // 6. Connection would block and socket closed - PING_STATUS_DEAD_PEER
    EXPECT_EQ(INetworkPing::PING_STATUS_PENDING,
            pImsNetworkPing->Ping(objSrcIp, objDstIp, nDstPort, nWaitTime));
    // on socket closed, PING_STATUS_DEAD_PEER should be notified
    EXPECT_CALL(m_objINetworkPingListener,
            NetworkPing_NotifyResult(_, INetworkPing::PING_STATUS_DEAD_PEER))
            .Times(1);
    pISocketListener->Socket_OnClosed(&objMockSocket);

    ITimerListener* pITimerListener = IMS_NULL;
    ON_CALL(objMockTimer, SetTimer)
            .WillByDefault(Invoke(
                    [&](Unused, IN ITimerListener* piListener)
                    {
                        pITimerListener = piListener;
                        return 1;
                    }));

    // 7. Connection would block and timer expired - PING_STATUS_TIMEDOUT
    EXPECT_EQ(INetworkPing::PING_STATUS_PENDING,
            pImsNetworkPing->Ping(objSrcIp, objDstIp, nDstPort, nWaitTime));
    // null timer, should be ignored and not notified
    pITimerListener->Timer_TimerExpired(IMS_NULL);
    // null timer, should be ignored and not notified
    MockITimer objDiffMockTimer;
    pITimerListener->Timer_TimerExpired(&objDiffMockTimer);
    // on timer expired with proper timer object, PING_STATUS_TIMEDOUT should be notified
    EXPECT_CALL(m_objINetworkPingListener,
            NetworkPing_NotifyResult(_, INetworkPing::PING_STATUS_TIMEDOUT))
            .Times(1);
    pITimerListener->Timer_TimerExpired(&objMockTimer);

    pImsNetworkPing->Destroy();
}

}  // namespace android
