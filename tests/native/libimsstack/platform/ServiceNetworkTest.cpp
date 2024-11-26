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

#include "ImsMessageDef.h"
#include "ImsNetworkConnectionState.h"
#include "ImsSocketState.h"
#include "MockIOsFactory.h"
#include "MockINetworkIpSec.h"
#include "MockIIpcan.h"
#include "PlatformContext.h"
#include "ServiceNetwork.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;

namespace android
{
static const AString PROFILE_NAME("TestConnectionProfile");
static const IMS_SINT32 APN_TYPE = 999;
static const IpAddress IPADDRESS(AString("192.168.2.20"));

class TestOsSocket : public ImsSocket
{
public:
    inline TestOsSocket() :
            m_bServiceMsgDispatched(IMS_FALSE)
    {
    }
    inline ~TestOsSocket() {}

public:
    IMS_SINT32 GetSocketId() const override { return 1; }

    SOCKET_ENTYPE GetSocketType() const override { return ISocket::TYPE_STREAM; }

    SOCKET_RESULT Open(IN SOCKET_ENTYPE /*eType*/, IN ISocketListener* /*piListener*/,
            IN ADDRESS_FAMILY_ENTYPE /*eAddressFamily = ADDRESS_FAMILY_INET*/) override
    {
        return ISocket::RESULT_SUCCESS;
    }

    SOCKET_RESULT Open(IN SOCKET_ENTYPE /*eType*/,
            IN ADDRESS_FAMILY_ENTYPE /*eAddressFamily = ADDRESS_FAMILY_INET*/) override
    {
        return ISocket::RESULT_SUCCESS;
    }

    void SetListener(IN ISocketListener* /*piListener*/) override {}

    SOCKET_RESULT Close() override { return ISocket::RESULT_SUCCESS; }

    ISocket* Accept() override { return IMS_NULL; }

    SOCKET_RESULT Bind(
            IN const IpAddress& /*objSocketAddress*/, IN IMS_UINT32 /*nSocketPort*/) override
    {
        return ISocket::RESULT_SUCCESS;
    }

    SOCKET_RESULT Connect(
            IN const IpAddress& /*objHostAddress*/, IN IMS_UINT32 /*nHostPort*/) override
    {
        return ISocket::RESULT_SUCCESS;
    }

    SOCKET_RESULT Listen(IN IMS_SINT32 /*nBackLog = MAX_BACKLOG*/) override
    {
        return ISocket::RESULT_SUCCESS;
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
        return ISocket::RESULT_SUCCESS;
    }

    SOCKET_RESULT GetSockName(
            OUT IpAddress& /*objSocketAddress*/, OUT IMS_UINT32& /*nSocketPort*/) override
    {
        return ISocket::RESULT_SUCCESS;
    }

    IMS_BOOL Equals(IN const ISocket* /*piSocket*/) override { return IMS_TRUE; }

    IMS_SINT32 GetOption(IN IMS_SINT32 /*nOption*/) override { return 0; }

    IMS_BOOL SetOption(IN IMS_SINT32 /*nOption*/, IN IMS_SINT32 /*nOptionValue*/) override
    {
        return IMS_TRUE;
    }

    IMS_BOOL IsClosedOrBeingClosed() const override { return IMS_FALSE; }

    void Destroy() override {}

    void DispatchServiceMessage(IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/) override
    {
        m_bServiceMsgDispatched = IMS_TRUE;
    }

    IMS_BOOL GetDispatchMessageStatus() const { return m_bServiceMsgDispatched; }

    void ResetDispatchMessageStatus() { m_bServiceMsgDispatched = IMS_FALSE; }

    SOCKET_RESULT Abort() override { return ISocket::RESULT_SUCCESS; }

    void ClosedByDataConnection() override {}

private:
    IMS_BOOL m_bServiceMsgDispatched;
};

class TestOsNetworkConnection : public ImsNetworkConnection
{
public:
    inline TestOsNetworkConnection() :
            ImsNetworkConnection(IMS_SLOT_0),
            m_nConnectionHandle(0),
            m_nApnType(0),
            m_bServiceMsgDispatched(IMS_FALSE)
    {
    }
    inline ~TestOsNetworkConnection() {}

public:
    RESULT_ENTYPE Activate(IN IMS_BOOL /*bEnableApn = IMS_FALSE*/) override { return RESULT_DONE; }

    RESULT_ENTYPE Deactivate(IN IMS_BOOL /*bDisableApn = IMS_FALSE*/) override
    {
        return RESULT_DONE;
    }

    void GetAccessNetworkInfo(OUT AccessNetworkInfo& /*objAccessNetInfo*/) override {}

    void GetLastAccessNetworkInfo(OUT AccessNetworkInfo& /*objAccessNetInfo*/,
            OUT AString& /*strTimestamp*/, OUT AString& /*strCellInfoAge*/) override
    {
    }

    IMS_BOOL GetExtraInfo(IN const AString& /*strType*/, OUT AString& /*strInfo*/) override
    {
        return IMS_TRUE;
    }

    IMS_SINT32 GetHostByName(IN const AString& /*strHostName*/,
            OUT ImsList<IpAddress>& /*objIpAddrs*/,
            IN IMS_SINT32 /*nIpVersion = 0*/ /* default-local-address-based */) override
    {
        return 0;
    }

    IMS_SINT32 GetIfaceId() const override { return 0; }

    const AString& GetIfaceName() const override { return AString::ConstNull(); }

    const IpAddress& GetLocalAddress(
            IN IMS_SINT32 /*nIpVersion = 0 */ /* configuration-based */) const override
    {
        return *(new IpAddress());
    }

    const AStringArray& GetPcscfAddress(
            IN IMS_SINT32 /*nIpVersion = 0 */ /* configuration-based */) override
    {
        return AStringArray::ConstNull();
    }

    STATE_ENTYPE GetState() const override { return STATE_CONNECTED; }

    IMS_BOOL IsConnected(IN IMS_SINT32 /*nCategory = IIpcan::CATEGORY_ANY*/) const override
    {
        return IMS_TRUE;
    }

    IMS_BOOL IsePDGEnabled() const override { return IMS_TRUE; }

    IMS_BOOL IsIpv6Preferred() const override { return IMS_FALSE; }

    IMS_BOOL IsMobileDataEnabled() const override { return IMS_TRUE; }

    IMS_SINT32 GetMtu() const override { return 0; }

    void SetListener(IN INetworkConnectionListener* /*piListener*/) override {}

    void SetPreferredIpVersion(
            IN IMS_SINT32 /*nPreferredIpVersion = 0*/ /* default-aos-connection-profile */) override
    {
    }

    void AddReferenceListener(IN INetworkConnectionListener* /*piListener*/) override {}

    void RemoveReferenceListener(IN INetworkConnectionListener* /*piListener*/) override {}

    IMS_BOOL Create(IN const AString& /*strNetProfile*/) override
    {
        m_nConnectionHandle = ImsNetworkConnectionState::GetInstance()->GetAndIncrementHandle();
        ImsNetworkConnectionState::GetInstance()->AttachHandle(this);
        return IMS_TRUE;
    }

    IMS_BOOL Create(IN IMS_SINT32 /*nApnType*/) override
    {
        m_nConnectionHandle = ImsNetworkConnectionState::GetInstance()->GetAndIncrementHandle();
        ImsNetworkConnectionState::GetInstance()->AttachHandle(this);
        return IMS_TRUE;
    }

    void DispatchServiceMessage(IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/) override
    {
        m_bServiceMsgDispatched = IMS_TRUE;
    }

    IMS_BOOL GetDispatchMessageStatus() const { return m_bServiceMsgDispatched; }

    void ResetDispatchMessageStatus() { m_bServiceMsgDispatched = IMS_FALSE; }

    IMS_BOOL Equals(IN const IpAddress& /*objIpAddr*/) const override { return IMS_TRUE; }

    IMS_CONNECTION GetHandle() const override { return m_nConnectionHandle; }

    const AString& GetProfileName() const override { return m_strProfile; }

    IMS_SINT32 GetApnType() const override { return m_nApnType; }

    void SetConnectionDetails(
            IN const AString& strNetProfile, IMS_SINT32 nApnType, const IpAddress& objIpAddr)
    {
        m_strProfile = strNetProfile;
        m_nApnType = nApnType;
        m_objIpAddress = objIpAddr;
    }

private:
    IMS_CONNECTION m_nConnectionHandle;
    IMS_SINT32 m_nApnType;
    AString m_strProfile;
    IpAddress m_objIpAddress;
    IMS_BOOL m_bServiceMsgDispatched;
};

class ServiceNetworkTest : public ::testing::Test
{
public:
    inline ServiceNetworkTest() :
            m_pNetworkService(IMS_NULL),
            m_piOldOsFactory(IMS_NULL),
            m_pTestOsNetworkConnection(IMS_NULL)
    {
    }

protected:
    void SetUp() override
    {
        m_piOldOsFactory = PlatformContext::GetInstance()->SetOsFactory(&m_objOsFactory);
        m_pNetworkService = NetworkService::GetNetworkService();
        ASSERT_TRUE(m_pNetworkService != IMS_NULL);

        m_pTestOsNetworkConnection = new TestOsNetworkConnection();
        m_pTestOsNetworkConnection->SetConnectionDetails(PROFILE_NAME, APN_TYPE, IPADDRESS);
    }
    void TearDown() override
    {
        ImsNetworkConnectionState::GetInstance()->DetachHandle(PROFILE_NAME, 0);
        INetworkConnection* pINetworkConnection = m_pTestOsNetworkConnection;
        m_pNetworkService->DestroyConnection(pINetworkConnection);

        EXPECT_TRUE(pINetworkConnection == IMS_NULL);

        PlatformContext::GetInstance()->SetOsFactory(m_piOldOsFactory);
    }

public:
    NetworkService* m_pNetworkService;
    MockIOsFactory m_objOsFactory;
    IOsFactory* m_piOldOsFactory;
    TestOsNetworkConnection* m_pTestOsNetworkConnection;
};

TEST_F(ServiceNetworkTest, CreateConnection_ProfileName)
{
    EXPECT_CALL(m_objOsFactory, CreateNetworkConnection(PROFILE_NAME, _))
            .Times(2)
            .WillOnce(Return(static_cast<TestOsNetworkConnection*>(IMS_NULL)))
            .WillOnce(Return(m_pTestOsNetworkConnection));

    EXPECT_TRUE(m_pNetworkService->CreateConnection(PROFILE_NAME, 0) == IMS_NULL);

    INetworkConnection* pINetworkConnection = m_pNetworkService->CreateConnection(PROFILE_NAME, 0);
    EXPECT_TRUE(pINetworkConnection == m_pTestOsNetworkConnection);

    // OsFactory CreateNetworkConnection should not be called again.
    pINetworkConnection = m_pNetworkService->CreateConnection(PROFILE_NAME, 0);
    EXPECT_TRUE(pINetworkConnection == m_pTestOsNetworkConnection);
}

TEST_F(ServiceNetworkTest, CreateConnection_ApnType)
{
    EXPECT_CALL(m_objOsFactory, CreateNetworkConnection(PROFILE_NAME, _))
            .Times(2)
            .WillOnce(Return(static_cast<TestOsNetworkConnection*>(IMS_NULL)))
            .WillOnce(Return(m_pTestOsNetworkConnection));

    EXPECT_TRUE(m_pNetworkService->CreateConnection(PROFILE_NAME, 0) == IMS_NULL);

    INetworkConnection* pINetworkConnection = m_pNetworkService->CreateConnection(PROFILE_NAME, 0);
    EXPECT_TRUE(pINetworkConnection == m_pTestOsNetworkConnection);

    // OsFactory CreateNetworkConnection should not be called again.
    pINetworkConnection = m_pNetworkService->CreateConnection(PROFILE_NAME, 0);
    EXPECT_TRUE(pINetworkConnection == m_pTestOsNetworkConnection);
}

TEST_F(ServiceNetworkTest, FindConnection_ApnType)
{
    EXPECT_CALL(m_objOsFactory, CreateNetworkConnection(PROFILE_NAME, _))
            .Times(1)
            .WillOnce(Return(m_pTestOsNetworkConnection));

    INetworkConnection* pINetworkConnection = m_pNetworkService->CreateConnection(PROFILE_NAME, 0);
    EXPECT_TRUE(pINetworkConnection == m_pTestOsNetworkConnection);

    pINetworkConnection = m_pNetworkService->FindConnection(APN_TYPE, 0);

    EXPECT_TRUE(pINetworkConnection == m_pTestOsNetworkConnection);
}

TEST_F(ServiceNetworkTest, FindConnection_IpAddress)
{
    EXPECT_CALL(m_objOsFactory, CreateNetworkConnection(PROFILE_NAME, _))
            .Times(1)
            .WillOnce(Return(m_pTestOsNetworkConnection));

    INetworkConnection* pINetworkConnection = m_pNetworkService->CreateConnection(PROFILE_NAME, 0);
    EXPECT_TRUE(pINetworkConnection == m_pTestOsNetworkConnection);

    pINetworkConnection = m_pNetworkService->FindConnection(IPADDRESS);

    EXPECT_TRUE(pINetworkConnection == m_pTestOsNetworkConnection);
}

TEST_F(ServiceNetworkTest, CreateSocket_Connection)
{
    TestOsSocket objTestOsSocket;

    EXPECT_CALL(m_objOsFactory, CreateSocket())
            .Times(2)
            .WillOnce(Return(static_cast<ImsSocket*>(IMS_NULL)))
            .WillOnce(Return(&objTestOsSocket));

    EXPECT_TRUE(m_pNetworkService->CreateSocket(IMS_NULL) == IMS_NULL);
    EXPECT_TRUE(m_pNetworkService->CreateSocket(m_pTestOsNetworkConnection) == IMS_NULL);

    ISocket* pISocket = m_pNetworkService->CreateSocket(m_pTestOsNetworkConnection);

    EXPECT_TRUE(pISocket != IMS_NULL);
}

TEST_F(ServiceNetworkTest, CreateSocket_ProfileName)
{
    // No connection profile added
    EXPECT_TRUE(m_pNetworkService->CreateSocket(m_pTestOsNetworkConnection) == IMS_NULL);

    EXPECT_CALL(m_objOsFactory, CreateNetworkConnection(PROFILE_NAME, _))
            .Times(1)
            .WillOnce(Return(m_pTestOsNetworkConnection));

    // Add connection profile
    m_pNetworkService->CreateConnection(PROFILE_NAME, 0);

    TestOsSocket objTestOsSocket;

    EXPECT_CALL(m_objOsFactory, CreateSocket())
            .Times(2)
            .WillOnce(Return(static_cast<ImsSocket*>(IMS_NULL)))
            .WillOnce(Return(&objTestOsSocket));

    EXPECT_TRUE(m_pNetworkService->CreateSocket(IMS_NULL) == IMS_NULL);
    EXPECT_TRUE(m_pNetworkService->CreateSocket(m_pTestOsNetworkConnection) == IMS_NULL);

    ISocket* pISocket = m_pNetworkService->CreateSocket(m_pTestOsNetworkConnection);

    EXPECT_TRUE(pISocket != IMS_NULL);
}

TEST_F(ServiceNetworkTest, CreateSslSocket_Connection)
{
    TestOsSocket objTestOsSocket;
    SslCertificate objSslCertificate;

    EXPECT_CALL(m_objOsFactory, CreateSslSocket(_))
            .Times(2)
            .WillOnce(Return(static_cast<ImsSocket*>(IMS_NULL)))
            .WillOnce(Return(&objTestOsSocket));

    EXPECT_TRUE(m_pNetworkService->CreateSslSocket(IMS_NULL, &objSslCertificate) == IMS_NULL);
    EXPECT_TRUE(m_pNetworkService->CreateSslSocket(
                        m_pTestOsNetworkConnection, &objSslCertificate) == IMS_NULL);

    ISocket* pISocket =
            m_pNetworkService->CreateSslSocket(m_pTestOsNetworkConnection, &objSslCertificate);

    EXPECT_TRUE(pISocket != IMS_NULL);
}

TEST_F(ServiceNetworkTest, CreateSslSocket_ProfileName)
{
    SslCertificate objSslCertificate;

    // No connection profile added
    EXPECT_TRUE(m_pNetworkService->CreateSslSocket(
                        m_pTestOsNetworkConnection, &objSslCertificate) == IMS_NULL);

    EXPECT_CALL(m_objOsFactory, CreateNetworkConnection(PROFILE_NAME, _))
            .Times(1)
            .WillOnce(Return(m_pTestOsNetworkConnection));

    // Add connection profile
    m_pNetworkService->CreateConnection(PROFILE_NAME, 0);

    TestOsSocket objTestOsSocket;

    EXPECT_CALL(m_objOsFactory, CreateSslSocket(_))
            .Times(2)
            .WillOnce(Return(static_cast<ImsSocket*>(IMS_NULL)))
            .WillOnce(Return(&objTestOsSocket));

    EXPECT_TRUE(m_pNetworkService->CreateSslSocket(IMS_NULL, &objSslCertificate) == IMS_NULL);
    EXPECT_TRUE(m_pNetworkService->CreateSslSocket(
                        m_pTestOsNetworkConnection, &objSslCertificate) == IMS_NULL);

    ISocket* pISocket =
            m_pNetworkService->CreateSslSocket(m_pTestOsNetworkConnection, &objSslCertificate);

    EXPECT_TRUE(pISocket != IMS_NULL);
}

TEST_F(ServiceNetworkTest, DestroySocket)
{
    ISocket* pTestOsSocket = new TestOsSocket();

    EXPECT_TRUE(pTestOsSocket != IMS_NULL);

    m_pNetworkService->DestroySocket(pTestOsSocket);

    EXPECT_TRUE(pTestOsSocket == IMS_NULL);
}

TEST_F(ServiceNetworkTest, CheckIpAndPortAvailability) {}

TEST_F(ServiceNetworkTest, GetIpcan)
{
    MockIIpcan objMockIIpcan;

    EXPECT_CALL(m_objOsFactory, CreateIpcan()).Times(AnyNumber()).WillOnce(Return(&objMockIIpcan));
    EXPECT_TRUE(m_pNetworkService->GetIpcan() != IMS_NULL);
}

TEST_F(ServiceNetworkTest, GetIpSec)
{
    MockINetworkIpSec objMockINetworkIpSec;

    EXPECT_CALL(m_objOsFactory, CreateNetworkIpSec(_))
            .Times(AnyNumber())
            .WillOnce(Return(&objMockINetworkIpSec));

    EXPECT_TRUE(m_pNetworkService->GetIpSec(IMS_SLOT_0) != IMS_NULL);
}

TEST_F(ServiceNetworkTest, DispatchServiceMessage)
{
    EXPECT_CALL(m_objOsFactory, CreateNetworkConnection(PROFILE_NAME, _))
            .Times(1)
            .WillOnce(Return(m_pTestOsNetworkConnection));

    // Add connection profile
    m_pNetworkService->CreateConnection(PROFILE_NAME, IMS_SLOT_0);

    TestOsSocket objTestOsSocket;

    EXPECT_CALL(m_objOsFactory, CreateSocket()).Times(1).WillOnce(Return(&objTestOsSocket));

    ISocket* pISocket = m_pNetworkService->CreateSocket(m_pTestOsNetworkConnection);
    EXPECT_TRUE(pISocket != IMS_NULL);

    ImsSocketState* pSocketState = ImsSocketState::GetInstance();
    pSocketState->AttachHandle(m_pTestOsNetworkConnection->GetHandle(), &objTestOsSocket);

    EXPECT_FALSE(m_pTestOsNetworkConnection->GetDispatchMessageStatus());

    ImsMessage objImsNwMessage(IMS_MSG_NETWORK, 0, m_pTestOsNetworkConnection->GetHandle());

    // dispatch network msg
    m_pNetworkService->DispatchServiceMessage(objImsNwMessage);

    EXPECT_TRUE(m_pTestOsNetworkConnection->GetDispatchMessageStatus());

    m_pTestOsNetworkConnection->ResetDispatchMessageStatus();

    EXPECT_FALSE(m_pTestOsNetworkConnection->GetDispatchMessageStatus());

    ImsMessage objImsSocketMessage(IMS_MSG_SOCKET, 0, m_pTestOsNetworkConnection->GetHandle());

    // dispatch socket msg
    m_pNetworkService->DispatchServiceMessage(objImsSocketMessage);

    EXPECT_TRUE(objTestOsSocket.GetDispatchMessageStatus());

    objTestOsSocket.ResetDispatchMessageStatus();

    EXPECT_FALSE(objTestOsSocket.GetDispatchMessageStatus());

    ImsMessage objImsMessage(IMS_MSG_BATTERY, 0, m_pTestOsNetworkConnection->GetHandle());

    // dispatch invalid msg
    m_pNetworkService->DispatchServiceMessage(objImsMessage);

    EXPECT_FALSE(m_pTestOsNetworkConnection->GetDispatchMessageStatus());
    EXPECT_FALSE(objTestOsSocket.GetDispatchMessageStatus());

    pSocketState->DetachHandle(m_pTestOsNetworkConnection->GetHandle());
}

TEST_F(ServiceNetworkTest, GetSlotId_IpAddress)
{
    EXPECT_CALL(m_objOsFactory, CreateNetworkConnection(PROFILE_NAME, _))
            .Times(1)
            .WillOnce(Return(m_pTestOsNetworkConnection));

    // Add connection profile
    m_pNetworkService->CreateConnection(PROFILE_NAME, IMS_SLOT_0);

    EXPECT_EQ(m_pNetworkService->GetSlotId(IPADDRESS), IMS_SLOT_0);
}

TEST_F(ServiceNetworkTest, GetSlotId_Connection)
{
    EXPECT_EQ(m_pNetworkService->GetSlotId(m_pTestOsNetworkConnection), IMS_SLOT_0);
}

}  // namespace android
