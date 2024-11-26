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

#include "PlatformContext.h"
#include "ImsSocketState.h"
#include "TestMutexService.h"

using ::testing::AnyNumber;

namespace android
{

class TestImsSocket : public ImsSocket
{
public:
    inline TestImsSocket() :
            ImsSocket()
    {
    }
    inline ~TestImsSocket() {}

public:
    IMS_SINT32 GetSocketId() const override { return 0; }

    SOCKET_ENTYPE GetSocketType() const override { return TYPE_STREAM; }

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

    IMS_BOOL IsClosedOrBeingClosed() const override { return IMS_FALSE; }

    void Destroy() override {}
    void DispatchServiceMessage(IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/) override {}
    SOCKET_RESULT Abort() override { return RESULT_SUCCESS; }
    void ClosedByDataConnection() override
    {
        BindNetworkConnection(-1);  // Setting to -1 to verify in test.
    }
};

class ImsSocketStateTest : public ::testing::Test
{
public:
    inline ImsSocketStateTest() :
            m_pTestMutexService(IMS_NULL),
            m_pOldTestMutexService(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override
    {
        ImsSocketState::GetInstance()->DetachAll();

        m_pTestMutexService = new TestMutexService();
        ASSERT_TRUE(m_pTestMutexService != nullptr);

        m_pOldTestMutexService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_MUTEX, m_pTestMutexService);

        EXPECT_CALL(m_pTestMutexService->GetMockMutex(), Lock()).Times(AnyNumber());
        EXPECT_CALL(m_pTestMutexService->GetMockMutex(), Unlock()).Times(AnyNumber());

        ImsSocketState_InitInstance();
    }

    virtual void TearDown() override
    {
        ImsSocketState::GetInstance()->DetachAll();
        ImsSocketState_ExitInstance();

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_MUTEX, m_pOldTestMutexService);
        delete m_pTestMutexService;
    }

protected:
    TestMutexService* m_pTestMutexService;
    PlatformService* m_pOldTestMutexService;
};

TEST_F(ImsSocketStateTest, AttachAndDetachHandle)
{
    ImsSocketState* pImsSocketState = ImsSocketState::GetInstance();
    ASSERT_TRUE(pImsSocketState != nullptr);

    TestImsSocket* pSocket = new TestImsSocket();
    ASSERT_TRUE(pSocket != nullptr);

    IMS_SOCKET hSocket = 1;
    IMS_CONNECTION hConnection = 20;

    pSocket->BindNetworkConnection(hConnection);

    pImsSocketState->AttachHandle(hSocket, pSocket);

    ImsSocket* pImsSocket = pImsSocketState->LookupHandle(hSocket);
    ASSERT_TRUE(pImsSocket != nullptr);

    EXPECT_EQ(pImsSocket->GetNetworkConnection(), pSocket->GetNetworkConnection());

    pImsSocketState->DetachHandle(hSocket);

    pImsSocket = pImsSocketState->LookupHandle(hSocket);
    ASSERT_TRUE(pImsSocket == nullptr);
}

TEST_F(ImsSocketStateTest, AddAndDestroyDeadSockets)
{
    ImsSocketState* pImsSocketState = ImsSocketState::GetInstance();
    ASSERT_TRUE(pImsSocketState != nullptr);

    TestImsSocket* pSocket = new TestImsSocket();
    ASSERT_TRUE(pSocket != nullptr);

    pImsSocketState->AddDeadSocket(pSocket);

    pSocket = new TestImsSocket();
    ASSERT_TRUE(pSocket != nullptr);

    pImsSocketState->AddDeadSocket(pSocket);

    pImsSocketState->DestroyDeadSockets();
}

TEST_F(ImsSocketStateTest, DetachAll)
{
    ImsSocketState* pImsSocketState = ImsSocketState::GetInstance();
    ASSERT_TRUE(pImsSocketState != nullptr);

    TestImsSocket* pSocket = new TestImsSocket();
    ASSERT_TRUE(pSocket != nullptr);

    IMS_SOCKET hSocket = 1;
    IMS_CONNECTION hConnection = 20;

    pSocket->BindNetworkConnection(hConnection);

    pImsSocketState->AttachHandle(hSocket, pSocket);

    ImsSocket* pImsSocket = pImsSocketState->LookupHandle(hSocket);
    ASSERT_TRUE(pImsSocket != nullptr);

    EXPECT_EQ(pImsSocket->GetNetworkConnection(), pSocket->GetNetworkConnection());

    pImsSocketState->DetachAll();

    pImsSocket = pImsSocketState->LookupHandle(hSocket);
    EXPECT_TRUE(pImsSocket == nullptr);
}

TEST_F(ImsSocketStateTest, DetachAll_Connections)
{
    ImsSocketState* pImsSocketState = ImsSocketState::GetInstance();
    ASSERT_TRUE(pImsSocketState != nullptr);

    // 1. Attach Socket with connection1, socket 1
    TestImsSocket* pSocket = new TestImsSocket();
    ASSERT_TRUE(pSocket != nullptr);

    IMS_SOCKET hSocket1 = 1;
    IMS_CONNECTION hConnection1 = 20;

    pSocket->BindNetworkConnection(hConnection1);

    pImsSocketState->AttachHandle(hSocket1, pSocket);

    // 2. Attach Socket with connection1, socket 2 - connection is same as above
    pSocket = new TestImsSocket();
    ASSERT_TRUE(pSocket != nullptr);

    IMS_SOCKET hSocket2 = 2;
    pSocket->BindNetworkConnection(hConnection1);

    pImsSocketState->AttachHandle(hSocket2, pSocket);

    // 3. Attach Socket with connection2, socket 3 - connection is different
    pSocket = new TestImsSocket();
    ASSERT_TRUE(pSocket != nullptr);

    IMS_SOCKET hSocket3 = 3;
    IMS_CONNECTION hConnection2 = 30;
    pSocket->BindNetworkConnection(hConnection2);

    pImsSocketState->AttachHandle(hSocket3, pSocket);

    EXPECT_TRUE(pImsSocketState->LookupHandle(hSocket1) != nullptr);
    EXPECT_TRUE(pImsSocketState->LookupHandle(hSocket2) != nullptr);
    EXPECT_TRUE(pImsSocketState->LookupHandle(hSocket3) != nullptr);

    // detach all connections of connection1
    pImsSocketState->DetachAll(hConnection1);

    EXPECT_TRUE(pImsSocketState->LookupHandle(hSocket1)->GetNetworkConnection() == -1);
    EXPECT_TRUE(pImsSocketState->LookupHandle(hSocket2)->GetNetworkConnection() == -1);
    EXPECT_TRUE(pImsSocketState->LookupHandle(hSocket3)->GetNetworkConnection() != -1);

    // detach all connections of connection2
    pImsSocketState->DetachAll(hConnection2);
    EXPECT_TRUE(pImsSocketState->LookupHandle(hSocket3)->GetNetworkConnection() == -1);

    pImsSocketState->DetachAll();
}

}  // namespace android