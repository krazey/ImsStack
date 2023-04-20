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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ServiceNetworkPolicy.h"
#include "connection/AosConnection.h"
#include "network/OsNetworkConnection.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosConnectionListener.h"
#include "interface/MockIAosNConfiguration.h"
#include "../../../platform/interface/MockINetworkConnection.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

class AosConnectionTest : public ::testing::Test
{
public:
    AosConnection* m_pAosConnection;
    AosStaticProfile* m_pAosStaticProfile;
    IAosNConfiguration* m_piOriginConfiguration;
    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosConnectionListener m_objMockIAosConnectionListener;
    MockINetworkConnection m_objMockINetworkConnection;
    MockINetworkConnection m_objMockOtherNetworkConnection;

protected:
    virtual void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pAosStaticProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(SLOT_ID));

        EXPECT_CALL(m_objMockIAosAppContext, GetStaticProfile())
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_pAosStaticProfile));

        m_pAosConnection =
                new AosConnection(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
        ASSERT_TRUE(m_pAosConnection != nullptr);

        m_piOriginConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piOriginConfiguration);

        if (m_pAosConnection)
        {
            delete m_pAosConnection;
        }
        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }
    }

    INetworkConnection* GetNetworkConnection() { return m_pAosConnection->m_piConnection; }

    void SetNetworkConnection(IN INetworkConnection* piConnection)
    {
        m_pAosConnection->m_piConnection = piConnection;
    }

    void Initialize()
    {
        m_pAosConnection->m_nCnxType = NetworkPolicy::APN_IMS;
        m_pAosConnection->m_nState = AosConnection::STATE_IDLE;
        m_pAosConnection->m_bActivationRequested = IMS_FALSE;
    }

    void SetState(IN IMS_UINT32 nState) { m_pAosConnection->m_nState = nState; }

    void SetConnectionType(IN IMS_SINT32 nType) { m_pAosConnection->m_nCnxType = nType; }

    void SetActivationRequested(IN IMS_BOOL bRequested)
    {
        m_pAosConnection->m_bActivationRequested = bRequested;
    }

    IMS_BOOL IsActivationRequested() { return m_pAosConnection->IsActivationRequested(); }

    IMS_UINT32 GetListenerSize() { return m_pAosConnection->m_objListeners.GetSize(); }

    void NotifyListenerEvent(IMS_UINT32 nEvent)
    {
        switch (nEvent)
        {
            case OsNetworkConnection::NET_CONNECTED:
                m_pAosConnection->NetworkConnection_OnConnected(&m_objMockINetworkConnection);
                break;

            case OsNetworkConnection::NET_DISCONNECTED:
                m_pAosConnection->NetworkConnection_OnDisconnected(&m_objMockINetworkConnection, 0);
                break;

            case OsNetworkConnection::NET_CONNECT_FAILED:
                m_pAosConnection->NetworkConnection_OnConnectionFailed(
                        &m_objMockINetworkConnection, 0);
                break;

            case OsNetworkConnection::NET_IP_CHANGED:
                m_pAosConnection->NetworkConnection_OnIpChanged(&m_objMockINetworkConnection);
                break;

            case OsNetworkConnection::NET_IPCAN_CAT_CHANGED:
                m_pAosConnection->NetworkConnection_OnIpcanChanged(&m_objMockINetworkConnection);
                break;

            case OsNetworkConnection::NET_PCSCF_CHANGED:
                m_pAosConnection->NetworkConnection_OnPcscfChanged(&m_objMockINetworkConnection);
                break;

            default:
                break;
        }
    }
};

TEST_F(AosConnectionTest, Constructor)
{
    EXPECT_NE(m_pAosConnection->GetConnectionType(), NetworkPolicy::APN_NONE);
    EXPECT_NE(GetNetworkConnection(), nullptr);
}

TEST_F(AosConnectionTest, Activate)
{
    EXPECT_CALL(m_objMockINetworkConnection, Activate(_))
            .Times(2)
            .WillOnce(Return(INetworkConnection::RESULT_DONE))
            .WillOnce(Return(INetworkConnection::RESULT_DOING));

    SetNetworkConnection(&m_objMockINetworkConnection);
    Initialize();
    SetState(AosConnection::STATE_ACTIVE);
    EXPECT_TRUE(m_pAosConnection->Activate());
    EXPECT_FALSE(IsActivationRequested());

    Initialize();
    SetActivationRequested(IMS_TRUE);
    EXPECT_TRUE(m_pAosConnection->Activate());

    Initialize();
    m_pAosConnection->Activate();
    EXPECT_TRUE(IsActivationRequested());
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_ACTIVE);

    Initialize();
    m_pAosConnection->Activate();
    EXPECT_TRUE(IsActivationRequested());
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_ACTIVATING);
}

TEST_F(AosConnectionTest, Deactivate)
{
    EXPECT_CALL(m_objMockINetworkConnection, Deactivate(_))
            .Times(1)
            .WillOnce(Return(INetworkConnection::RESULT_DONE));

    SetNetworkConnection(&m_objMockINetworkConnection);
    Initialize();
    m_pAosConnection->Deactivate();

    SetActivationRequested(IMS_TRUE);
    m_pAosConnection->Deactivate();
    EXPECT_FALSE(IsActivationRequested());
}

TEST_F(AosConnectionTest, GetState)
{
    Initialize();
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_IDLE);
    SetState(AosConnection::STATE_ACTIVATING);
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_ACTIVATING);
    SetState(AosConnection::STATE_ACTIVE);
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_ACTIVE);
}

TEST_F(AosConnectionTest, GetConnectionType)
{
    Initialize();
    EXPECT_EQ(m_pAosConnection->GetConnectionType(), NetworkPolicy::APN_IMS);
    SetConnectionType(NetworkPolicy::APN_EMERGENCY);
    EXPECT_EQ(m_pAosConnection->GetConnectionType(), NetworkPolicy::APN_EMERGENCY);
}

TEST_F(AosConnectionTest, SetListener)
{
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_)).Times(2);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_ConnectionFailed()).Times(1);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpChanged()).Times(1);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpcanCatChanged()).Times(1);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_PcscfChanged()).Times(1);

    IMS_UINT32 CurrentListenerSize = GetListenerSize();
    // add new listener
    m_pAosConnection->SetListener(
            static_cast<IAosConnectionListener*>(&m_objMockIAosConnectionListener));
    CurrentListenerSize++;
    EXPECT_EQ(CurrentListenerSize, GetListenerSize());
    // do not add same listener again
    m_pAosConnection->SetListener(
            static_cast<IAosConnectionListener*>(&m_objMockIAosConnectionListener));
    EXPECT_EQ(CurrentListenerSize, GetListenerSize());

    SetNetworkConnection(&m_objMockINetworkConnection);
    SetState(AosConnection::STATE_IDLE);
    NotifyListenerEvent(OsNetworkConnection::NET_CONNECTED);
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_ACTIVE);

    SetState(AosConnection::STATE_ACTIVE);
    NotifyListenerEvent(OsNetworkConnection::NET_DISCONNECTED);
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_IDLE);

    SetState(AosConnection::STATE_ACTIVATING);
    NotifyListenerEvent(OsNetworkConnection::NET_CONNECT_FAILED);
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_IDLE);

    SetState(AosConnection::STATE_IDLE);
    NotifyListenerEvent(OsNetworkConnection::NET_IP_CHANGED);
    SetState(AosConnection::STATE_ACTIVE);
    NotifyListenerEvent(OsNetworkConnection::NET_IP_CHANGED);

    NotifyListenerEvent(OsNetworkConnection::NET_IPCAN_CAT_CHANGED);

    NotifyListenerEvent(OsNetworkConnection::NET_PCSCF_CHANGED);
}

TEST_F(AosConnectionTest, RemoveListener)
{
    m_pAosConnection->SetListener(
            static_cast<IAosConnectionListener*>(&m_objMockIAosConnectionListener));
    IMS_UINT32 CurrentListenerSize = GetListenerSize();

    // remove listener
    m_pAosConnection->RemoveListener(&m_objMockIAosConnectionListener);
    CurrentListenerSize--;
    EXPECT_EQ(CurrentListenerSize, GetListenerSize());

    // The listener not in the list cannot be removed
    m_pAosConnection->RemoveListener(&m_objMockIAosConnectionListener);
    EXPECT_EQ(CurrentListenerSize, GetListenerSize());
}

TEST_F(AosConnectionTest, GetMtu)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetMtu()).Times(1).WillOnce(Return(3000));

    SetNetworkConnection(&m_objMockINetworkConnection);
    Initialize();
    EXPECT_EQ(m_pAosConnection->GetMtu(), 0);

    SetState(AosConnection::STATE_ACTIVE);
    EXPECT_EQ(m_pAosConnection->GetMtu(), 3000);
}

TEST_F(AosConnectionTest, GetLocalAddress)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetLocalAddress(_))
            .Times(1)
            .WillOnce(ReturnRef(IpAddress::NONE));

    SetNetworkConnection(&m_objMockINetworkConnection);
    EXPECT_EQ(m_pAosConnection->GetLocalAddress(), IpAddress::NONE);
}

TEST_F(AosConnectionTest, GetPcscfAddress)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetPcscfAddress(_))
            .Times(1)
            .WillOnce(ReturnRef(AStringArray::ConstNull()));

    SetNetworkConnection(&m_objMockINetworkConnection);
    m_pAosConnection->GetPcscfAddress();
}

TEST_F(AosConnectionTest, GetHostByName)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetHostByName(_, _, _)).Times(1).WillOnce(Return(1));

    SetNetworkConnection(&m_objMockINetworkConnection);

    const AString strHostName = AString("hostName");
    ImsList<IpAddress> objIpas;
    EXPECT_EQ(m_pAosConnection->GetHostByName(strHostName, objIpas), 1);
}

TEST_F(AosConnectionTest, GetIfaceName)
{
    const AString strIfaceName = AString("ifaceName");
    EXPECT_CALL(m_objMockINetworkConnection, GetIfaceName())
            .Times(1)
            .WillOnce(ReturnRef(strIfaceName));

    SetNetworkConnection(&m_objMockINetworkConnection);

    EXPECT_EQ(m_pAosConnection->GetIfaceName(), strIfaceName);
}

TEST_F(AosConnectionTest, IsEpdgEnabled)
{
    EXPECT_CALL(m_objMockINetworkConnection, IsePDGEnabled()).Times(1).WillOnce(Return(IMS_TRUE));
    SetNetworkConnection(&m_objMockINetworkConnection);
    EXPECT_EQ(m_pAosConnection->IsEpdgEnabled(), IMS_TRUE);
}

TEST_F(AosConnectionTest, GetIpcanCategory)
{
    EXPECT_CALL(m_objMockINetworkConnection, IsePDGEnabled())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    SetNetworkConnection(&m_objMockINetworkConnection);

    EXPECT_EQ(m_pAosConnection->GetIpcanCategory(), IIpcan::CATEGORY_WLAN);
    EXPECT_EQ(m_pAosConnection->GetIpcanCategory(), IIpcan::CATEGORY_MOBILE);
}

TEST_F(AosConnectionTest, IsLimitedServicePcoValue_SupportLimitedAdminSmsMode)
{
    // Set IAosNConfiguration
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);
    EXPECT_CALL(objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosConnection->SetCarrierSignalPcoValue(-1); /*PCO_INVALID_VALUE*/
    EXPECT_EQ(m_pAosConnection->GetCarrierSignalPcoValue(), -1);
    EXPECT_FALSE(m_pAosConnection->IsLimitedServicePcoValue());

    m_pAosConnection->SetCarrierSignalPcoValue(5); /*PCO_LIMITED_SERVICE_VALUE*/
    EXPECT_EQ(m_pAosConnection->GetCarrierSignalPcoValue(), 5);
    EXPECT_TRUE(m_pAosConnection->IsLimitedServicePcoValue());

    m_pAosConnection->SetCarrierSignalPcoValue(0);
    EXPECT_EQ(m_pAosConnection->GetCarrierSignalPcoValue(), 0);
    EXPECT_FALSE(m_pAosConnection->IsLimitedServicePcoValue());
}

TEST_F(AosConnectionTest, IsLimitedServicePcoValue_NotSupportLimitedAdminSmsMode)
{
    // Set IAosNConfiguration
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);
    EXPECT_CALL(objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosConnection->SetCarrierSignalPcoValue(-1); /*PCO_INVALID_VALUE*/
    EXPECT_EQ(m_pAosConnection->GetCarrierSignalPcoValue(), -1);
    EXPECT_FALSE(m_pAosConnection->IsLimitedServicePcoValue());

    m_pAosConnection->SetCarrierSignalPcoValue(5); /*PCO_LIMITED_SERVICE_VALUE*/
    EXPECT_EQ(m_pAosConnection->GetCarrierSignalPcoValue(), -1);
    EXPECT_FALSE(m_pAosConnection->IsLimitedServicePcoValue());

    m_pAosConnection->SetCarrierSignalPcoValue(0);
    EXPECT_EQ(m_pAosConnection->GetCarrierSignalPcoValue(), -1);
    EXPECT_FALSE(m_pAosConnection->IsLimitedServicePcoValue());
}

TEST_F(AosConnectionTest, StateToString)
{
    IMS_SINT32 nInvalidState = -1;

    EXPECT_STREQ(m_pAosConnection->StateToString(IAosConnection::STATE_IDLE), "STATE_IDLE");
    EXPECT_STREQ(m_pAosConnection->StateToString(IAosConnection::STATE_ACTIVE), "STATE_ACTIVE");
    EXPECT_STREQ(
            m_pAosConnection->StateToString(IAosConnection::STATE_ACTIVATING), "STATE_ACTIVATING");
    EXPECT_STREQ(m_pAosConnection->StateToString(nInvalidState), "__INVALID__");
}

TEST_F(AosConnectionTest, OtherNetworkConnection)
{
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_)).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_ConnectionFailed()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpChanged()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpcanCatChanged()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_PcscfChanged()).Times(0);

    m_pAosConnection->SetListener(
            static_cast<IAosConnectionListener*>(&m_objMockIAosConnectionListener));
    SetNetworkConnection(&m_objMockOtherNetworkConnection);
    SetState(AosConnection::STATE_IDLE);

    NotifyListenerEvent(OsNetworkConnection::NET_CONNECTED);
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_IDLE);

    SetState(AosConnection::STATE_ACTIVE);
    NotifyListenerEvent(OsNetworkConnection::NET_DISCONNECTED);
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_ACTIVE);

    SetState(AosConnection::STATE_ACTIVATING);
    NotifyListenerEvent(OsNetworkConnection::NET_CONNECT_FAILED);
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_ACTIVATING);

    NotifyListenerEvent(OsNetworkConnection::NET_IP_CHANGED);
    NotifyListenerEvent(OsNetworkConnection::NET_IPCAN_CAT_CHANGED);
    NotifyListenerEvent(OsNetworkConnection::NET_PCSCF_CHANGED);
}