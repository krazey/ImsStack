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
    MockINetworkConnection objMockINetworkConnection;

protected:
    virtual void SetUp() override
    {
        pAosStaticProfile = new AosStaticProfile();
        pMockAosAppContext = new MockAosAppContext(pAosStaticProfile);

        EXPECT_CALL(*pMockAosAppContext, GetSlotId()).Times(AnyNumber()).WillRepeatedly(Return(0));

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
        SetNetworkConnection(&objMockINetworkConnection);

        switch (nEvent)
        {
            case OsNetworkConnection::NET_CONNECTED:
                pAosConnection->NetworkConnection_OnConnected(&objMockINetworkConnection);
                break;

            case OsNetworkConnection::NET_DISCONNECTED:
                pAosConnection->NetworkConnection_OnDisconnected(&objMockINetworkConnection, 0);
                break;

            case OsNetworkConnection::NET_CONNECT_FAILED:
                pAosConnection->NetworkConnection_OnConnectionFailed(&objMockINetworkConnection, 0);
                break;

            case OsNetworkConnection::NET_IP_CHANGED:
                pAosConnection->NetworkConnection_OnIpChanged(&objMockINetworkConnection);
                break;

            case OsNetworkConnection::NET_IPCAN_CAT_CHANGED:
                pAosConnection->NetworkConnection_OnIpcanChanged(&objMockINetworkConnection);
                break;

            case OsNetworkConnection::NET_PCSCF_CHANGED:
                pAosConnection->NetworkConnection_OnPcscfChanged(&objMockINetworkConnection);
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
    SetNetworkConnection(&objMockINetworkConnection);
    EXPECT_CALL(objMockINetworkConnection, Activate(_))
            .Times(2)
            .WillOnce(Return(INetworkConnection::RESULT_DONE))
            .WillOnce(Return(INetworkConnection::RESULT_DOING));

    Initialize();
    SetState(AosConnection::STATE_ACTIVE);
    pAosConnection->Activate();
    EXPECT_FALSE(IsActivationRequested());

    Initialize();
    pAosConnection->Activate();
    EXPECT_TRUE(IsActivationRequested());
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_ACTIVE);

    Initialize();
    pAosConnection->Activate();
    EXPECT_TRUE(IsActivationRequested());
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_ACTIVATING);
}

TEST_F(AosConnectionTest, Deactivate)
{
    SetNetworkConnection(&objMockINetworkConnection);
    EXPECT_CALL(objMockINetworkConnection, Deactivate(_))
            .Times(1)
            .WillOnce(Return(INetworkConnection::RESULT_DONE));

    Initialize();
    pAosConnection->Deactivate();

    SetActivationRequested(IMS_TRUE);
    pAosConnection->Deactivate();
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
    IMS_UINT32 ListerSize = GetListenerSize();
    MockIAosConnectionListener objMockIAosConnectionListener;

    pAosConnection->SetListener(
            static_cast<IAosConnectionListener*>(&objMockIAosConnectionListener));
    ListerSize++;
    EXPECT_EQ(ListerSize, GetListenerSize());

    pAosConnection->SetListener(
            static_cast<IAosConnectionListener*>(&objMockIAosConnectionListener));
    EXPECT_EQ(ListerSize, GetListenerSize());

    EXPECT_CALL(objMockIAosConnectionListener, AosConnection_StateChanged(_)).Times(2);

    EXPECT_CALL(objMockIAosConnectionListener, AosConnection_ConnectionFailed()).Times(1);

    EXPECT_CALL(objMockIAosConnectionListener, AosConnection_IpChanged()).Times(1);

    EXPECT_CALL(objMockIAosConnectionListener, AosConnection_IpcanCatChanged()).Times(1);

    EXPECT_CALL(objMockIAosConnectionListener, AosConnection_PcscfChanged()).Times(1);

    SetState(AosConnection::STATE_IDLE);
    NotifyListenerEvent(OsNetworkConnection::NET_CONNECTED);
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_ACTIVE);

    SetState(AosConnection::STATE_ACTIVE);
    NotifyListenerEvent(OsNetworkConnection::NET_DISCONNECTED);
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_IDLE);

    SetState(AosConnection::STATE_ACTIVATING);
    NotifyListenerEvent(OsNetworkConnection::NET_CONNECT_FAILED);
    EXPECT_EQ(pAosConnection->GetState(), AosConnection::STATE_IDLE);

    SetState(AosConnection::STATE_IDLE);
    NotifyListenerEvent(OsNetworkConnection::NET_IP_CHANGED);
    SetState(AosConnection::STATE_ACTIVE);
    NotifyListenerEvent(OsNetworkConnection::NET_IP_CHANGED);

    NotifyListenerEvent(OsNetworkConnection::NET_IPCAN_CAT_CHANGED);

    NotifyListenerEvent(OsNetworkConnection::NET_PCSCF_CHANGED);
}

TEST_F(AosConnectionTest, RemoveListener)
{
    MockIAosConnectionListener objMockIAosConnectionListener;
    pAosConnection->SetListener(
            static_cast<IAosConnectionListener*>(&objMockIAosConnectionListener));

    IMS_UINT32 ListerSize = GetListenerSize();
    pAosConnection->RemoveListener(&objMockIAosConnectionListener);
    ListerSize--;
    EXPECT_EQ(ListerSize, GetListenerSize());

    pAosConnection->RemoveListener(&objMockIAosConnectionListener);
    EXPECT_EQ(ListerSize, GetListenerSize());
}

TEST_F(AosConnectionTest, GetMtu)
{
    SetNetworkConnection(&objMockINetworkConnection);
    EXPECT_CALL(objMockINetworkConnection, GetMtu()).Times(1).WillOnce(Return(3000));

    Initialize();
    EXPECT_EQ(pAosConnection->GetMtu(), 0);

    SetState(AosConnection::STATE_ACTIVE);
    EXPECT_EQ(pAosConnection->GetMtu(), 3000);
}

TEST_F(AosConnectionTest, GetLocalAddress)
{
    SetNetworkConnection(&objMockINetworkConnection);
    EXPECT_CALL(objMockINetworkConnection, GetLocalAddress(_))
            .Times(1)
            .WillOnce(ReturnRef(IPAddress::NONE));

    EXPECT_EQ(pAosConnection->GetLocalAddress(), IPAddress::NONE);
}

TEST_F(AosConnectionTest, GetPcscfAddress)
{
    SetNetworkConnection(&objMockINetworkConnection);
    EXPECT_CALL(objMockINetworkConnection, GetPcscfAddress(_))
            .Times(1)
            .WillOnce(ReturnRef(AStringArray::ConstNull()));

    pAosConnection->GetPcscfAddress();
}

TEST_F(AosConnectionTest, GetHostByName)
{
    SetNetworkConnection(&objMockINetworkConnection);
    EXPECT_CALL(objMockINetworkConnection, GetHostByName(_, _, _)).Times(1).WillOnce(Return(1));

    const AString strHostName = AString("hostName");
    IMSList<IPAddress> objIpas;
    EXPECT_EQ(pAosConnection->GetHostByName(strHostName, objIpas), 1);
}

TEST_F(AosConnectionTest, GetIfaceName)
{
    SetNetworkConnection(&objMockINetworkConnection);
    const AString strIfaceName = AString("ifaceName");
    EXPECT_CALL(objMockINetworkConnection, GetIfaceName())
            .Times(1)
            .WillOnce(ReturnRef(strIfaceName));

    EXPECT_EQ(pAosConnection->GetIfaceName(), strIfaceName);
}

TEST_F(AosConnectionTest, IsEpdgEnabled)
{
    SetNetworkConnection(&objMockINetworkConnection);
    EXPECT_CALL(objMockINetworkConnection, IsePDGEnabled()).Times(1).WillOnce(Return(IMS_TRUE));

    EXPECT_EQ(pAosConnection->IsEpdgEnabled(), IMS_TRUE);
}

TEST_F(AosConnectionTest, GetIpcanCategory)
{
    SetNetworkConnection(&objMockINetworkConnection);
    EXPECT_CALL(objMockINetworkConnection, IsePDGEnabled())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_EQ(pAosConnection->GetIpcanCategory(), IIpcan::CATEGORY_WLAN);
    EXPECT_EQ(pAosConnection->GetIpcanCategory(), IIpcan::CATEGORY_MOBILE);
}