/*
 * Copyright (C) 2021 The Android Open Source Project
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
#include <gmock/gmock.h>

#include "ServiceNetworkPolicy.h"
#include "connection/AosConnection.h"
#include "network/OsNetworkConnection.h"

#include "app/MockAosAppContext.h"
#include "interface/MockIAosConnectionListener.h"
#include "../../../platform/interface/MockINetworkConnection.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Assign;
using ::testing::Return;
using ::testing::ReturnRef;

class AosConnectionTest : public ::testing::Test
{
public:
    AosConnection* pAosConnection;
    AosStaticProfile* pAosStaticProfile;
    MockAosAppContext* pMockAosAppContext;
    MockINetworkConnection* pMockINetworkConnection;

protected:
    virtual void SetUp() override
    {
        pAosStaticProfile = new AosStaticProfile();
        pMockAosAppContext = new MockAosAppContext(pAosStaticProfile);
        pMockINetworkConnection = new MockINetworkConnection();

        EXPECT_CALL(*pMockAosAppContext, GetSlotId()).Times(AnyNumber()).WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(*pMockAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(*pMockAosAppContext, GetStaticProfile())
                .Times(AnyNumber())
                .WillRepeatedly(Return(pAosStaticProfile));

        pAosConnection = new AosConnection(static_cast<IAosAppContext*>(pMockAosAppContext));
        ASSERT_TRUE(pAosConnection != nullptr);
    }

    virtual void TearDown() override
    {
        if (pAosConnection)
        {
            delete pAosConnection;
        }
        if (pMockAosAppContext)
        {
            delete pMockAosAppContext;
        }
        if (pAosStaticProfile)
        {
            delete pAosStaticProfile;
        }
    }

    INetworkConnection* GetNetworkConnection() { return pAosConnection->m_piConnection; }

    void SetNetworkConnection(IN INetworkConnection* piConnection)
    {
        pAosConnection->m_piConnection = piConnection;
    }

    void Initialize()
    {
        pAosConnection->m_nCnxType = NetworkPolicy::APN_IMS;
        pAosConnection->m_nState = AosConnection::STATE_IDLE;
        pAosConnection->m_bActivationRequested = IMS_FALSE;
    }

    void SetState(IN IMS_UINT32 nState) { pAosConnection->m_nState = nState; }

    void SetConnectionType(IN IMS_SINT32 nType) { pAosConnection->m_nCnxType = nType; }

    void SetActivationRequested(IN IMS_BOOL bRequested)
    {
        pAosConnection->m_bActivationRequested = bRequested;
    }

    IMS_BOOL IsActivationRequested() { return pAosConnection->IsActivationRequested(); }

    IMS_UINT32 GetListenerSize() { return pAosConnection->m_objListeners.GetSize(); }

    void NotifyListenerEvent(IMS_UINT32 nEvent)
    {
        SetNetworkConnection(pMockINetworkConnection);

        switch (nEvent)
        {
            case OsNetworkConnection::NET_CONNECTED:
                pAosConnection->NetworkConnection_OnConnected(pMockINetworkConnection);
                break;

            case OsNetworkConnection::NET_DISCONNECTED:
                pAosConnection->NetworkConnection_OnDisconnected(pMockINetworkConnection, 0);
                break;

            case OsNetworkConnection::NET_CONNECT_FAILED:
                pAosConnection->NetworkConnection_OnConnectionFailed(pMockINetworkConnection, 0);
                break;

            case OsNetworkConnection::NET_IP_CHANGED:
                pAosConnection->NetworkConnection_OnIpChanged(pMockINetworkConnection);
                break;

            case OsNetworkConnection::NET_IPCAN_CAT_CHANGED:
                pAosConnection->NetworkConnection_OnIpcanChanged(pMockINetworkConnection);
                break;

            case OsNetworkConnection::NET_PCSCF_CHANGED:
                pAosConnection->NetworkConnection_OnPcscfChanged(pMockINetworkConnection);
                break;

            default:
                break;
        }
    }
};

TEST_F(AosConnectionTest, Constructor)
{
    EXPECT_NE(pAosConnection->GetConnectionType(), NetworkPolicy::APN_NONE);
    EXPECT_NE(GetNetworkConnection(), nullptr);
}

TEST_F(AosConnectionTest, Activate)
{
    bool isInvokeActivate = false;
    SetNetworkConnection(pMockINetworkConnection);
    EXPECT_CALL(*pMockINetworkConnection, Activate(_, _))
            .Times(2)
            .WillOnce(
                    DoAll(Assign(&isInvokeActivate, true), Return(INetworkConnection::RESULT_DONE)))
            .WillOnce(DoAll(
                    Assign(&isInvokeActivate, true), Return(INetworkConnection::RESULT_DOING)));

    Initialize();
    SetState(AosConnection::STATE_ACTIVE);
    pAosConnection->Activate();
    EXPECT_FALSE(IsActivationRequested());

    Initialize();
    pAosConnection->Activate();
    EXPECT_TRUE(IsActivationRequested());
    EXPECT_TRUE(isInvokeActivate);
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_ACTIVE);

    Initialize();
    isInvokeActivate = false;
    pAosConnection->Activate();
    EXPECT_TRUE(IsActivationRequested());
    EXPECT_TRUE(isInvokeActivate);
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_ACTIVATING);
}

TEST_F(AosConnectionTest, Deactivate)
{
    bool isInvokeDeactivate = false;
    SetNetworkConnection(pMockINetworkConnection);
    EXPECT_CALL(*pMockINetworkConnection, Deactivate(_, _))
            .Times(1)
            .WillOnce(DoAll(
                    Assign(&isInvokeDeactivate, true), Return(INetworkConnection::RESULT_DONE)));

    Initialize();
    pAosConnection->Deactivate();
    EXPECT_FALSE(isInvokeDeactivate);

    SetActivationRequested(IMS_TRUE);
    pAosConnection->Deactivate();
    EXPECT_TRUE(isInvokeDeactivate);
    EXPECT_FALSE(IsActivationRequested());
}

TEST_F(AosConnectionTest, GetState)
{
    Initialize();
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_IDLE);
    SetState(AosConnection::STATE_ACTIVATING);
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_ACTIVATING);
    SetState(AosConnection::STATE_ACTIVE);
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_ACTIVE);
}

TEST_F(AosConnectionTest, GetConnectionType)
{
    Initialize();
    EXPECT_EQ(pAosConnection->GetConnectionType(), NetworkPolicy::APN_IMS);
    SetConnectionType(NetworkPolicy::APN_EMERGENCY);
    EXPECT_EQ(pAosConnection->GetConnectionType(), NetworkPolicy::APN_EMERGENCY);
}

TEST_F(AosConnectionTest, SetListener)
{
    bool isInvokeNotify = false;
    MockIAosConnectionListener* pMockIAosConnectionListener = new MockIAosConnectionListener();

    EXPECT_CALL(*pMockIAosConnectionListener, AosConnection_StateChanged(_))
            .Times(2)
            .WillOnce(Assign(&isInvokeNotify, true))
            .WillOnce(Assign(&isInvokeNotify, true));

    EXPECT_CALL(*pMockIAosConnectionListener, AosConnection_IpChanged())
            .Times(1)
            .WillOnce(Assign(&isInvokeNotify, true));

    EXPECT_CALL(*pMockIAosConnectionListener, AosConnection_IpcanCatChanged())
            .Times(1)
            .WillOnce(Assign(&isInvokeNotify, true));

    EXPECT_CALL(*pMockIAosConnectionListener, AosConnection_PcscfChanged())
            .Times(1)
            .WillOnce(Assign(&isInvokeNotify, true));

    EXPECT_CALL(*pMockIAosConnectionListener, AosConnection_ConnectionFailed())
            .Times(1)
            .WillOnce(Assign(&isInvokeNotify, true));

    IMS_UINT32 ListerSize = GetListenerSize();
    pAosConnection->SetListener(static_cast<IAosConnectionListener*>(pMockIAosConnectionListener));
    ListerSize++;
    EXPECT_EQ(ListerSize, GetListenerSize());

    pAosConnection->SetListener(static_cast<IAosConnectionListener*>(pMockIAosConnectionListener));
    EXPECT_EQ(ListerSize, GetListenerSize());

    SetState(AosConnection::STATE_IDLE);
    NotifyListenerEvent(OsNetworkConnection::NET_CONNECTED);
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_ACTIVE);
    EXPECT_TRUE(isInvokeNotify);

    isInvokeNotify = false;
    SetState(AosConnection::STATE_ACTIVE);
    NotifyListenerEvent(OsNetworkConnection::NET_DISCONNECTED);
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_IDLE);
    EXPECT_TRUE(isInvokeNotify);

    isInvokeNotify = false;
    SetState(AosConnection::STATE_ACTIVATING);
    NotifyListenerEvent(OsNetworkConnection::NET_CONNECT_FAILED);
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_IDLE);
    EXPECT_TRUE(isInvokeNotify);

    isInvokeNotify = false;
    SetState(AosConnection::STATE_IDLE);
    NotifyListenerEvent(OsNetworkConnection::NET_IP_CHANGED);
    EXPECT_FALSE(isInvokeNotify);
    SetState(AosConnection::STATE_ACTIVE);
    NotifyListenerEvent(OsNetworkConnection::NET_IP_CHANGED);
    EXPECT_TRUE(isInvokeNotify);

    isInvokeNotify = false;
    NotifyListenerEvent(OsNetworkConnection::NET_IPCAN_CAT_CHANGED);
    EXPECT_TRUE(isInvokeNotify);

    isInvokeNotify = false;
    NotifyListenerEvent(OsNetworkConnection::NET_PCSCF_CHANGED);
    EXPECT_TRUE(isInvokeNotify);
}

TEST_F(AosConnectionTest, RemoveListener)
{
    MockIAosConnectionListener* pMockIAosConnectionListener = new MockIAosConnectionListener();

    pAosConnection->SetListener(pMockIAosConnectionListener);
    IMS_UINT32 ListerSize = GetListenerSize();

    pAosConnection->RemoveListener(pMockIAosConnectionListener);
    ListerSize--;
    EXPECT_EQ(ListerSize, GetListenerSize());
}

TEST_F(AosConnectionTest, GetMtu)
{
    bool isInvokeGetMtu = false;
    SetNetworkConnection(pMockINetworkConnection);
    EXPECT_CALL(*pMockINetworkConnection, GetMtu())
            .Times(1)
            .WillOnce(DoAll(Assign(&isInvokeGetMtu, true), Return(3000)));

    Initialize();
    EXPECT_EQ(pAosConnection->GetMtu(), 0);
    EXPECT_FALSE(isInvokeGetMtu);

    SetState(AosConnection::STATE_ACTIVE);
    EXPECT_EQ(pAosConnection->GetMtu(), 3000);
    EXPECT_TRUE(isInvokeGetMtu);
}

TEST_F(AosConnectionTest, GetLocalAddress)
{
    bool isInvokeGetLocalAddress = false;
    SetNetworkConnection(pMockINetworkConnection);
    EXPECT_CALL(*pMockINetworkConnection, GetLocalAddress(_))
            .Times(1)
            .WillOnce(DoAll(Assign(&isInvokeGetLocalAddress, true), ReturnRef(IPAddress::NONE)));

    EXPECT_EQ(pAosConnection->GetLocalAddress(), IPAddress::NONE);
    EXPECT_TRUE(isInvokeGetLocalAddress);
}

TEST_F(AosConnectionTest, GetPcscfAddress)
{
    bool isInvokeGetPcscfAddress = false;
    SetNetworkConnection(pMockINetworkConnection);
    EXPECT_CALL(*pMockINetworkConnection, GetPcscfAddress(_))
            .Times(1)
            .WillOnce(DoAll(
                    Assign(&isInvokeGetPcscfAddress, true), ReturnRef(AStringArray::ConstNull())));

    pAosConnection->GetPcscfAddress();
    EXPECT_TRUE(isInvokeGetPcscfAddress);
}

TEST_F(AosConnectionTest, GetHostByName)
{
    bool isInvokeGetHostByName = false;
    SetNetworkConnection(pMockINetworkConnection);
    EXPECT_CALL(*pMockINetworkConnection, GetHostByName(_, _, _))
            .Times(1)
            .WillOnce(DoAll(Assign(&isInvokeGetHostByName, true), Return(1)));

    const AString strHostName = AString("hostName");
    IMSList<IPAddress> objIpas;
    EXPECT_EQ(pAosConnection->GetHostByName(strHostName, objIpas), 1);
    EXPECT_TRUE(isInvokeGetHostByName);
}

TEST_F(AosConnectionTest, GetIfaceName)
{
    bool isInvokeGetIfaceName = false;
    SetNetworkConnection(pMockINetworkConnection);
    const AString strIfaceName = AString("ifaceName");
    EXPECT_CALL(*pMockINetworkConnection, GetIfaceName())
            .Times(1)
            .WillOnce(DoAll(Assign(&isInvokeGetIfaceName, true), ReturnRef(strIfaceName)));

    EXPECT_EQ(pAosConnection->GetIfaceName(), strIfaceName);
    EXPECT_TRUE(isInvokeGetIfaceName);
}

TEST_F(AosConnectionTest, IsEpdgEnabled)
{
    bool isInvokeIsEpdgEnabled = false;
    SetNetworkConnection(pMockINetworkConnection);
    EXPECT_CALL(*pMockINetworkConnection, IsePDGEnabled())
            .Times(1)
            .WillOnce(DoAll(Assign(&isInvokeIsEpdgEnabled, true), Return(IMS_TRUE)));

    EXPECT_EQ(pAosConnection->IsEpdgEnabled(), IMS_TRUE);
    EXPECT_TRUE(isInvokeIsEpdgEnabled);
}

TEST_F(AosConnectionTest, GetIpcanCategory)
{
    SetNetworkConnection(pMockINetworkConnection);
    EXPECT_CALL(*pMockINetworkConnection, IsePDGEnabled())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_EQ(pAosConnection->GetIpcanCategory(), IIpcan::CATEGORY_WLAN);
    EXPECT_EQ(pAosConnection->GetIpcanCategory(), IIpcan::CATEGORY_MOBILE);
}

TEST_F(AosConnectionTest, SendPingToHostAddress)
{
    bool isInvokeSendPingToHostAddress = false;
    SetNetworkConnection(pMockINetworkConnection);
    EXPECT_CALL(*pMockINetworkConnection, SendPingToHostAddress(_))
            .Times(1)
            .WillOnce(DoAll(Assign(&isInvokeSendPingToHostAddress, true), Return(IMS_TRUE)));

    EXPECT_EQ(pAosConnection->SendPingToHostAddress(IPAddress::NONE), IMS_TRUE);
    EXPECT_TRUE(isInvokeSendPingToHostAddress);
}