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
#include <gtest/gtest.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "ImsSocketState.h"
#include "network/OsSelectFdSet.h"
#include "network/OsSocket.h"
#include "network/OsSocketBase.h"
#include "network/OsSocketService.h"

namespace android
{

class TestOsSocketBase : public OsSocketBase
{
public:
    inline TestOsSocketBase() :
            OsSocketBase(),
            m_bNotifyDataReceived(IMS_FALSE),
            m_bNotifySendEnabled(IMS_FALSE),
            m_bNotifyConnectionReceived(IMS_FALSE),
            m_bNotifyConnected(IMS_FALSE),
            m_bNotifyClosed(IMS_FALSE),
            m_bNotifyAcceptCompleted(IMS_FALSE),
            m_nSocketState(OsSocketBase::SOCKET_STATE_CLOSED),
            m_nSocketId(0),
            m_nSetLastError(0),
            m_bWaitForClose(IMS_FALSE),
            m_objCondition(PTHREAD_COND_INITIALIZER),
            m_objMutex(PTHREAD_MUTEX_INITIALIZER)
    {
        memset(m_arrBuffer, 0, sizeof(m_arrBuffer));
    }
    inline ~TestOsSocketBase() {}

public:
    IMS_SINT32 GetSocketId() const override { return m_nSocketId; }

    SOCKET_RESULT Open(IN SOCKET_ENTYPE /*eType*/, IN ISocketListener* /*piListener*/,
            IN ADDRESS_FAMILY_ENTYPE /*eAddressFamily = ADDRESS_FAMILY_INET*/) override
    {
        return RESULT_SUCCESS;
    }

    SOCKET_RESULT Open(IN SOCKET_ENTYPE /*eType*/,
            IN ADDRESS_FAMILY_ENTYPE /*eAddressFamily = ADDRESS_FAMILY_INET*/) override
    {
        return RESULT_SUCCESS;
    }

    void SetListener(IN ISocketListener* /*piListener*/) override {}

    SOCKET_RESULT Close() override { return RESULT_SUCCESS; }

    ISocket* Accept() override { return IMS_NULL; }

    SOCKET_RESULT Bind(
            IN const IpAddress& /*objSocketAddress*/, IN IMS_UINT32 /*nSocketPort*/) override
    {
        return RESULT_SUCCESS;
    }

    SOCKET_RESULT Connect(
            IN const IpAddress& /*objHostAddress*/, IN IMS_UINT32 /*nHostPort*/) override
    {
        return RESULT_SUCCESS;
    }

    SOCKET_RESULT Listen(IN IMS_SINT32 /*nBackLog = MAX_BACKLOG*/) override
    {
        return RESULT_SUCCESS;
    }

    IMS_SINT32 Receive(OUT IMS_BYTE* /*pBuffer*/, IN IMS_SINT32 /*nBuffLen*/) override { return 0; }

    IMS_SINT32 Send(IN const IMS_BYTE* /*pBuffer*/, IN IMS_SINT32 /*nBuffLen*/) override
    {
        return 0;
    }

    IMS_SINT32 ReceiveFrom(OUT IMS_BYTE* /*pBuffer*/, IN IMS_SINT32 /*nBuffLen*/,
            OUT IpAddress& /*objHostAddress*/, OUT IMS_UINT32& /*nHostPort*/) override
    {
        return 0;
    }

    IMS_SINT32 SendTo(IN const IMS_BYTE* /*pBuffer*/, IN IMS_SINT32 /*nBuffLen*/,
            IN const IpAddress& /*objHostAddress*/, IN IMS_UINT32 /*nHostPort*/) override
    {
        return 0;
    }

    SOCKET_RESULT GetPeerName(
            OUT IpAddress& /*objPeerAddress*/, OUT IMS_UINT32& /*nPeerPort*/) override
    {
        return RESULT_SUCCESS;
    }

    SOCKET_RESULT GetSockName(
            OUT IpAddress& /*objSocketAddress*/, OUT IMS_UINT32& /*nSocketPort*/) override
    {
        return RESULT_SUCCESS;
    }

    IMS_BOOL Equals(IN const ISocket* /*piSocket*/) override { return IMS_TRUE; }

    IMS_SINT32 GetOption(IN IMS_SINT32 /*nOption*/) override { return 0; }

    IMS_BOOL SetOption(IN IMS_SINT32 /*nOption*/, IN IMS_SINT32 /*nOptionValue*/) override
    {
        return IMS_TRUE;
    }

    inline void SetSocketId(IMS_SINT32 nSocketId) { m_nSocketId = nSocketId; }

    inline void SetServerSocket(IN IMS_BOOL bServerSocket)
    {
        OsSocketBase::SetServerSocket(bServerSocket);
    }

    inline void SetSocketType(IN SOCKET_ENTYPE eType) { OsSocketBase::SetSocketType(eType); }

    inline void SetSocketConnected(IN IMS_BOOL bConnected)
    {
        OsSocketBase::SetSocketConnected(bConnected);
    }

    inline void SetSocketState(IMS_SINT32 nSocketState) { m_nSocketState = nSocketState; }

    inline void SetLastError(IMS_SINT32 nSetLastError) { m_nSetLastError = nSetLastError; }

    inline void SetWaitForClose(IMS_BOOL bWaitForClose) { m_bWaitForClose = bWaitForClose; }

    IMS_BOOL IsDataReceivedNotified() const { return m_bNotifyDataReceived; }

    IMS_BOOL IsSendEnabledNotified() const { return m_bNotifySendEnabled; }

    IMS_BOOL IsConnectionReceivedNotified() const { return m_bNotifyConnectionReceived; }

    IMS_BOOL IsConnectedNotified() const { return m_bNotifyConnected; }

    IMS_BOOL IsClosedNotified() const { return m_bNotifyClosed; }

    void NotifyDataReceived(IN IMS_SINT32 /*nErrorCode*/) override
    {
        m_bNotifyDataReceived = IMS_TRUE;
        memset(m_arrBuffer, 0, sizeof(m_arrBuffer));
        read(GetSocketId(), m_arrBuffer, sizeof(m_arrBuffer));
        Release();
    }

    void NotifySendEnabled(IN IMS_SINT32 /*nErrorCode*/) override
    {
        m_bNotifySendEnabled = IMS_TRUE;

        Release();
    }

    void NotifyConnectionReceived(IN IMS_SINT32 /*nErrorCode*/) override
    {
        m_bNotifyConnectionReceived = IMS_TRUE;

        memset(m_arrBuffer, 0, sizeof(m_arrBuffer));
        read(GetSocketId(), m_arrBuffer, sizeof(m_arrBuffer));
        Release();
    }

    void NotifyConnected(IN IMS_SINT32 /*nErrorCode*/) override
    {
        m_bNotifyConnected = IMS_TRUE;

        Release();
    }

    void NotifyClosed(IN IMS_SINT32 /*nErrorCode*/) override
    {
        if (m_bWaitForClose == IMS_FALSE)
        {
            memset(m_arrBuffer, 0, sizeof(m_arrBuffer));
            read(GetSocketId(), m_arrBuffer, sizeof(m_arrBuffer));
        }

        m_bNotifyClosed = IMS_TRUE;
        m_bWaitForClose = IMS_FALSE;

        Release();
    }

    void NotifyAcceptCompleted(IN IMS_SOCKET /*hSocket*/) override
    {
        m_bNotifyAcceptCompleted = IMS_TRUE;
    }

    inline IMS_SINT32 GetLastError() const override { return m_nSetLastError; }

    inline IMS_SINT32 GetSocketState() const override { return m_nSocketState; }

    void Destroy() override {}

    void DispatchServiceMessage(IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/) override {}

    SOCKET_RESULT Abort() override { return RESULT_SUCCESS; }

    void ClosedByDataConnection() override {}

    void Wait()
    {
        pthread_mutex_lock(&m_objMutex);

        struct timespec time;
        ASSERT_TRUE(clock_gettime(CLOCK_REALTIME, &time) != -1);
        time.tv_sec += 1;

        pthread_cond_timedwait(&m_objCondition, &m_objMutex, &time);
        pthread_mutex_unlock(&m_objMutex);
    }

    void Release()
    {
        if (m_bWaitForClose)
        {
            return;
        }
        pthread_mutex_lock(&m_objMutex);
        pthread_cond_signal(&m_objCondition);
        pthread_mutex_unlock(&m_objMutex);
    }

private:
    IMS_BOOL m_bNotifyDataReceived;
    IMS_BOOL m_bNotifySendEnabled;
    IMS_BOOL m_bNotifyConnectionReceived;
    IMS_BOOL m_bNotifyConnected;
    IMS_BOOL m_bNotifyClosed;
    IMS_BOOL m_bNotifyAcceptCompleted;
    IMS_SINT32 m_nSocketState;
    IMS_SINT32 m_nSocketId;
    IMS_SINT32 m_nSetLastError;
    IMS_BOOL m_bWaitForClose;
    pthread_cond_t m_objCondition;
    pthread_mutex_t m_objMutex;
    char m_arrBuffer[1024];
};

class OsSocketServiceTest : public ::testing::Test
{
public:
    inline OsSocketServiceTest() :
            m_pObjOsSocketService(IMS_NULL),
            m_pTestOsSocketBase(IMS_NULL)
    {
        m_nSocket[0] = 111;
        m_nSocket[1] = 222;
    }

protected:
    virtual void SetUp() override
    {
        ASSERT_TRUE(socketpair(AF_UNIX, SOCK_STREAM, 0, m_nSocket) == 0);

        m_pObjOsSocketService = OsSocketService::GetInstance();
        ASSERT_TRUE(m_pObjOsSocketService != nullptr);

        EXPECT_EQ(IMS_TRUE, m_pObjOsSocketService->StartUp());
        EXPECT_EQ(IMS_TRUE, m_pObjOsSocketService->IsStarted());

        m_pTestOsSocketBase = new TestOsSocketBase();
    }
    virtual void TearDown() override
    {
        close(m_nSocket[0]);
        close(m_nSocket[1]);

        m_pObjOsSocketService->RemoveEvent(
                m_pTestOsSocketBase->GetSocketId(), FD_CLOSE | FD_READ | FD_WRITE);
        m_pObjOsSocketService->DetachHandle(m_pTestOsSocketBase->GetSocketId());
        m_pObjOsSocketService->SendControlEvent();

        EXPECT_TRUE(
                m_pObjOsSocketService->LookupHandle(m_pTestOsSocketBase->GetSocketId()) == nullptr);

        usleep(3000);

        delete m_pTestOsSocketBase;
    }

    void PostSocketHandleEvent(IMS_SINT32 nEvent)
    {
        // Attached socket handle
        m_pObjOsSocketService->AttachHandle(
                m_pTestOsSocketBase->GetSocketId(), m_pTestOsSocketBase);
        // update the socket fd for READ event
        m_pObjOsSocketService->SetEvent(m_pTestOsSocketBase->GetSocketId(), FD_CLOSE | nEvent);

        m_pObjOsSocketService->SendControlEvent();

        if ((nEvent & FD_READ) == FD_READ)
        {
            const char* pMessage = "Hello";
            write(m_nSocket[0], pMessage, strlen(pMessage));
        }

        // wait(1 second) for the update as socket read happens in another thread
        m_pTestOsSocketBase->Wait();
    }

public:
    IMS_SINT32 m_nSocket[2];
    OsSocketService* m_pObjOsSocketService;
    TestOsSocketBase* m_pTestOsSocketBase;
};

TEST_F(OsSocketServiceTest, AddDeadSocket)
{
    TestOsSocketBase* pTestOsSocketBase = new TestOsSocketBase();

    m_pObjOsSocketService->AddDeadSocket(pTestOsSocketBase);

    ImsSocketState::GetInstance()->DestroyDeadSockets();
}

TEST_F(OsSocketServiceTest, LookupHandle)
{
    EXPECT_TRUE(m_pObjOsSocketService->LookupHandle(m_nSocket[1]) == nullptr);

    IMS_CONNECTION hConnection = 20;

    m_pTestOsSocketBase->BindNetworkConnection(hConnection);

    m_pTestOsSocketBase->SetSocketId(m_nSocket[1]);
    m_pTestOsSocketBase->SetSocketType(ISocket::TYPE_DGRAM);

    m_pObjOsSocketService->AttachHandle(m_pTestOsSocketBase->GetSocketId(), m_pTestOsSocketBase);
    m_pObjOsSocketService->AttachHandle(
            m_pTestOsSocketBase->GetSocketId(), m_pTestOsSocketBase);  // will be added only once

    TestOsSocketBase* pTestSocket = static_cast<TestOsSocketBase*>(
            m_pObjOsSocketService->LookupHandle(m_pTestOsSocketBase->GetSocketId()));

    ASSERT_TRUE(pTestSocket != nullptr);

    EXPECT_EQ(pTestSocket->GetNetworkConnection(), m_pTestOsSocketBase->GetNetworkConnection());
}

TEST_F(OsSocketServiceTest, KillSocket)
{
    // No socket to kill
    m_pObjOsSocketService->KillSocket(m_nSocket[1]);

    IMS_CONNECTION hConnection = 20;

    m_pTestOsSocketBase->BindNetworkConnection(hConnection);

    m_pTestOsSocketBase->SetSocketId(m_nSocket[1]);
    m_pTestOsSocketBase->SetSocketType(ISocket::TYPE_DGRAM);

    m_pObjOsSocketService->AttachHandle(m_pTestOsSocketBase->GetSocketId(), m_pTestOsSocketBase);

    TestOsSocketBase* pTestSocket = static_cast<TestOsSocketBase*>(
            m_pObjOsSocketService->LookupHandle(m_pTestOsSocketBase->GetSocketId()));

    EXPECT_EQ(pTestSocket->GetNetworkConnection(), m_pTestOsSocketBase->GetNetworkConnection());

    m_pObjOsSocketService->KillSocket(m_pTestOsSocketBase->GetSocketId());
}

TEST_F(OsSocketServiceTest, SendControlEvent_NotifyDataReceived)
{
    m_pTestOsSocketBase->SetSocketId(m_nSocket[1]);
    m_pTestOsSocketBase->SetSocketType(ISocket::TYPE_DGRAM);

    // socket NotifyDataReceived not called
    EXPECT_EQ(m_pTestOsSocketBase->IsDataReceivedNotified(), IMS_FALSE);

    PostSocketHandleEvent(FD_READ);

    // socket NotifyDataReceived called
    EXPECT_EQ(m_pTestOsSocketBase->IsDataReceivedNotified(), IMS_TRUE);
}

TEST_F(OsSocketServiceTest, DoNotificationCallback_NotifyConnectionReceived)
{
    m_pTestOsSocketBase->SetSocketId(m_nSocket[1]);
    m_pTestOsSocketBase->SetSocketType(ISocket::TYPE_STREAM);
    m_pTestOsSocketBase->SetServerSocket(IMS_TRUE);  // Server socket, to receive connection

    // socket NotifyConnectionReceived not called
    EXPECT_EQ(m_pTestOsSocketBase->IsConnectionReceivedNotified(), IMS_FALSE);

    PostSocketHandleEvent(FD_READ);

    // socket NotifyConnectionReceived called
    EXPECT_EQ(m_pTestOsSocketBase->IsConnectionReceivedNotified(), IMS_TRUE);
}

TEST_F(OsSocketServiceTest, DoNotificationCallback_SocketStateClose_NotifyClosed)
{
    m_pTestOsSocketBase->SetSocketId(m_nSocket[1]);
    m_pTestOsSocketBase->SetSocketType(ISocket::TYPE_STREAM);
    m_pTestOsSocketBase->SetSocketState(OsSocketBase::SOCKET_STATE_CLOSED);

    // socket NotifyClosed not called
    EXPECT_EQ(m_pTestOsSocketBase->IsClosedNotified(), IMS_FALSE);

    PostSocketHandleEvent(FD_READ);

    // socket NotifyClosed called
    EXPECT_EQ(m_pTestOsSocketBase->IsClosedNotified(), IMS_TRUE);
}

TEST_F(OsSocketServiceTest, DoNotificationCallback_With_Error_NotifyDataReceivedAndClosed)
{
    m_pTestOsSocketBase->SetSocketId(m_nSocket[1]);
    m_pTestOsSocketBase->SetSocketType(ISocket::TYPE_STREAM);
    m_pTestOsSocketBase->SetSocketState(OsSocketBase::SOCKET_STATE_DATA_AVAILABLE);
    m_pTestOsSocketBase->SetLastError(1);
    m_pTestOsSocketBase->SetWaitForClose(
            IMS_TRUE);  // Both data receive and close will be notified (setting error)

    // socket NotifyDataReceived not called
    EXPECT_EQ(m_pTestOsSocketBase->IsDataReceivedNotified(), IMS_FALSE);
    // socket NotifyClosed not called
    EXPECT_EQ(m_pTestOsSocketBase->IsClosedNotified(), IMS_FALSE);

    PostSocketHandleEvent(FD_READ);

    // socket NotifyDataReceived called
    EXPECT_EQ(m_pTestOsSocketBase->IsDataReceivedNotified(), IMS_TRUE);
    // socket NotifyClosed called
    EXPECT_EQ(m_pTestOsSocketBase->IsClosedNotified(), IMS_TRUE);
}

TEST_F(OsSocketServiceTest, DoNotificationCallback_StreamSocket_NotifyDataReceived)
{
    m_pTestOsSocketBase->SetSocketId(m_nSocket[1]);
    m_pTestOsSocketBase->SetSocketType(ISocket::TYPE_STREAM);
    m_pTestOsSocketBase->SetSocketState(OsSocketBase::SOCKET_STATE_DATA_AVAILABLE);

    // socket NotifyDataReceived not called
    EXPECT_EQ(m_pTestOsSocketBase->IsDataReceivedNotified(), IMS_FALSE);

    PostSocketHandleEvent(FD_READ);

    // socket NotifyDataReceived called
    EXPECT_EQ(m_pTestOsSocketBase->IsDataReceivedNotified(), IMS_TRUE);
}

TEST_F(OsSocketServiceTest, DoNotificationCallback_NotifySendEnabled)
{
    m_pTestOsSocketBase->SetSocketId(m_nSocket[0]);
    m_pTestOsSocketBase->SetSocketType(ISocket::TYPE_DGRAM);

    // socket NotifySendEnabled not called
    EXPECT_EQ(m_pTestOsSocketBase->IsSendEnabledNotified(), IMS_FALSE);

    PostSocketHandleEvent(FD_WRITE);

    // socket NotifySendEnabled called
    EXPECT_EQ(m_pTestOsSocketBase->IsSendEnabledNotified(), IMS_TRUE);
}

TEST_F(OsSocketServiceTest, DoNotificationCallback_StreamSocketNotConnected_NotifyConnected)
{
    m_pTestOsSocketBase->SetSocketId(m_nSocket[0]);
    m_pTestOsSocketBase->SetSocketType(ISocket::TYPE_STREAM);  // By default, socket connected
                                                               // false.

    // socket NotifySendEnabled not called
    EXPECT_EQ(m_pTestOsSocketBase->IsConnectedNotified(), IMS_FALSE);

    PostSocketHandleEvent(FD_WRITE);

    // socket NotifySendEnabled called
    EXPECT_EQ(m_pTestOsSocketBase->IsConnectedNotified(), IMS_TRUE);
}

TEST_F(OsSocketServiceTest, DoNotificationCallback_StreamSocketConnected_NotifySendEnabled)
{
    m_pTestOsSocketBase->SetSocketId(m_nSocket[0]);
    m_pTestOsSocketBase->SetSocketType(ISocket::TYPE_STREAM);
    m_pTestOsSocketBase->SetSocketConnected(IMS_TRUE);

    // socket NotifySendEnabled not called
    EXPECT_EQ(m_pTestOsSocketBase->IsSendEnabledNotified(), IMS_FALSE);

    PostSocketHandleEvent(FD_WRITE);

    // socket NotifySendEnabled called
    EXPECT_EQ(m_pTestOsSocketBase->IsSendEnabledNotified(), IMS_TRUE);
}

TEST_F(OsSocketServiceTest, DoNotificationCallback_ServerSocket_NotifySendEnabled)
{
    m_pTestOsSocketBase->SetSocketId(m_nSocket[0]);
    m_pTestOsSocketBase->SetSocketType(ISocket::TYPE_STREAM);
    m_pTestOsSocketBase->SetServerSocket(IMS_TRUE);

    // socket NotifySendEnabled not called
    EXPECT_EQ(m_pTestOsSocketBase->IsSendEnabledNotified(), IMS_FALSE);

    PostSocketHandleEvent(FD_WRITE);

    // socket NotifySendEnabled called
    EXPECT_EQ(m_pTestOsSocketBase->IsSendEnabledNotified(), IMS_TRUE);
}

}  // namespace android