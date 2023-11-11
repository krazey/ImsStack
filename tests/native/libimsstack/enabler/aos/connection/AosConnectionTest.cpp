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
#include "PlatformContext.h"
#include "TestNetworkService.h"
#include "connection/AosConnection.h"
#include "network/OsNetworkConnection.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosConnectionListener.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosTransaction.h"
#include "../../../platform/interface/MockINetworkConnection.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

class TestAosConnection : public AosConnection
{
public:
    TestAosConnection(IN IAosAppContext* piAppContext) :
            AosConnection(piAppContext)
    {
    }

    FRIEND_TEST(AosConnectionTest, Constructor_NotSetListenerForNullConnection);
    FRIEND_TEST(AosConnectionTest, Activate_NotifyStateChangeImmediatelyIfActiveState);
    FRIEND_TEST(AosConnectionTest, Activate_ReturnIfAlreadyActivationRequested);
    FRIEND_TEST(AosConnectionTest, Activate_NotifyStateChangeImmediatelyIfGetResultDone);
    FRIEND_TEST(AosConnectionTest, Activate_ActivatingIsNotYetComplete);
    FRIEND_TEST(AosConnectionTest, Deactivate_NotInvokeDeactivateIfActivationNotRequested);
    FRIEND_TEST(AosConnectionTest, Deactivate_InvokeDeactivate);
    FRIEND_TEST(AosConnectionTest, GetConnectionType);
    FRIEND_TEST(AosConnectionTest, SetListener_NotAddSameListenerAgain);
    FRIEND_TEST(AosConnectionTest, RemoveListener_IgnoreIfNotInList);
    FRIEND_TEST(AosConnectionTest, GetMtu_ReturnZeroIfNotActiveState);
    FRIEND_TEST(AosConnectionTest, GetMtuThroughNetworkConnection);
    FRIEND_TEST(AosConnectionTest, GetLocalAddressThroughNetworkConnection);
    FRIEND_TEST(AosConnectionTest, GetPcscfAddressThroughNetworkConnection);
    FRIEND_TEST(AosConnectionTest, GetHostByNameThroughNetworkConnection);
    FRIEND_TEST(AosConnectionTest, GetIfaceNameThroughNetworkConnection);
    FRIEND_TEST(AosConnectionTest, CheckEpdgEnabledThroughNetworkConnection);
    FRIEND_TEST(AosConnectionTest, CheckIpv6PreferredThroughNetworkConnection);
    FRIEND_TEST(AosConnectionTest, CheckIpcanCategoryThroughNetworkConnection);
    FRIEND_TEST(AosConnectionTest, CheckPcoValue_ReturnTrueIfConfiguredAsLimitedServiceValue);
    FRIEND_TEST(AosConnectionTest, CheckPcoValue_ReturnFalseIfNotSupportLimitedAdminSmsMode);
    FRIEND_TEST(AosConnectionTest, CheckPcoValue_ReturnFalseIfConfiguredAsNotLimitedServiceValue);
    FRIEND_TEST(AosConnectionTest, StateToString_ReturnStringOfCurrentState);
    FRIEND_TEST(AosConnectionTest, NotUpdateIpcanForNonImsConnectionType);
    FRIEND_TEST(AosConnectionTest, HandleOnConnected_NotifyStateChanged);
    FRIEND_TEST(AosConnectionTest, HandleOnDisconnected_NotifyStateChanged);
    FRIEND_TEST(AosConnectionTest, HandleOnConnectionFailed_NotifyConnectionFailed);
    FRIEND_TEST(AosConnectionTest, HandleOnIpChanged_NotNotifyIpChangedIfNotConnected);
    FRIEND_TEST(AosConnectionTest, HandleOnIpChanged_NotifyIpChanged);
    FRIEND_TEST(AosConnectionTest, HandleOnIpcanChanged_NotifyIpcanChanged);
    FRIEND_TEST(AosConnectionTest, HandleOnPcscfChanged_NotifyPcscfChanged);
    FRIEND_TEST(AosConnectionTest, NotHandleNetworkConnectionCallbackForOtherNetworkConnection);
    FRIEND_TEST(AosConnectionTest, NotifyExceptionHandlingForNullListener_Ignore);
    FRIEND_TEST(AosConnectionTest, NotifyExceptionHandlingForUnknownType_Ignore);
};

class AosConnectionTest : public ::testing::Test
{
public:
    inline AosConnectionTest()
    {
        m_pAosStaticProfile = new AosStaticProfile();

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, &m_objNetworkService);
        m_objNetworkService.SetConnection(&m_objMockINetworkConnection);

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration(SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration, SLOT_ID);
        m_piAosTransaction = AosProvider::GetInstance()->GetTransaction(SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(&m_objMockIAosTransaction, SLOT_ID);
    }

    inline virtual ~AosConnectionTest()
    {
        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_NETWORK, IMS_NULL);
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetTransaction(m_piAosTransaction, SLOT_ID);
    }

public:
    TestAosConnection* m_pTestAosConnection;
    TestNetworkService m_objNetworkService;

    AosStaticProfile* m_pAosStaticProfile;
    IAosNConfiguration* m_piAosNConfiguration;
    IAosTransaction* m_piAosTransaction;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosConnectionListener m_objMockIAosConnectionListener;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosTransaction m_objMockIAosTransaction;
    MockINetworkConnection m_objMockINetworkConnection;

protected:
    virtual void SetUp() override
    {
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetStaticProfile())
                .WillByDefault(Return(m_pAosStaticProfile));

        m_pTestAosConnection = new TestAosConnection(&m_objMockIAosAppContext);
        m_pTestAosConnection->SetListener(&m_objMockIAosConnectionListener);
    }

    virtual void TearDown() override
    {
        if (m_pTestAosConnection)
        {
            delete m_pTestAosConnection;
        }
    }
};
TEST_F(AosConnectionTest, Constructor_NotSetListenerForNullConnection)
{
    // set listener as IMS_NULL when destrcut AosConnection
    EXPECT_CALL(m_objMockINetworkConnection, SetListener(IMS_NULL));
    if (m_pTestAosConnection)
    {
        delete m_pTestAosConnection;
    }

    // set to return IMS_NULL when NetworkService create connection
    m_objNetworkService.SetConnection(IMS_NULL);

    // not set listenr because NetworkService return IMS_NULL for CreateConnection
    EXPECT_CALL(m_objMockINetworkConnection, SetListener(_)).Times(0);

    m_pTestAosConnection = new TestAosConnection(&m_objMockIAosAppContext);

    EXPECT_EQ(m_pTestAosConnection->m_piConnection, nullptr);

    m_objNetworkService.SetConnection(&m_objMockINetworkConnection);
}

TEST_F(AosConnectionTest, Activate_NotifyStateChangeImmediatelyIfActiveState)
{
    m_pTestAosConnection->SetState(AosConnection::STATE_ACTIVE);

    // notify state change without invoking Activate
    EXPECT_CALL(m_objMockIAosConnectionListener,
            AosConnection_StateChanged(AosConnection::STATE_ACTIVE));
    EXPECT_CALL(m_objMockINetworkConnection, Activate(_)).Times(0);

    EXPECT_TRUE(m_pTestAosConnection->Activate());

    EXPECT_FALSE(m_pTestAosConnection->IsActivationRequested());
}

TEST_F(AosConnectionTest, Activate_ReturnIfAlreadyActivationRequested)
{
    m_pTestAosConnection->SetActivationRequested(IMS_TRUE);

    // return without invoking Activate
    EXPECT_CALL(m_objMockINetworkConnection, Activate(_)).Times(0);

    EXPECT_TRUE(m_pTestAosConnection->Activate());
}

TEST_F(AosConnectionTest, Activate_NotifyStateChangeImmediatelyIfGetResultDone)
{
    ON_CALL(m_objMockINetworkConnection, Activate(_))
            .WillByDefault(Return(INetworkConnection::RESULT_DONE));

    // notify state change
    EXPECT_CALL(m_objMockIAosConnectionListener,
            AosConnection_StateChanged(AosConnection::STATE_ACTIVE));

    EXPECT_TRUE(m_pTestAosConnection->Activate());

    EXPECT_TRUE(m_pTestAosConnection->IsActivationRequested());
    EXPECT_EQ(m_pTestAosConnection->GetState(), AosConnection::STATE_ACTIVE);
}

TEST_F(AosConnectionTest, Activate_ActivatingIsNotYetComplete)
{
    ON_CALL(m_objMockINetworkConnection, Activate(_))
            .WillByDefault(Return(INetworkConnection::RESULT_DOING));

    // not notify state change because it is not yet complete
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_)).Times(0);

    EXPECT_TRUE(m_pTestAosConnection->Activate());

    EXPECT_TRUE(m_pTestAosConnection->IsActivationRequested());
    EXPECT_EQ(m_pTestAosConnection->GetState(), AosConnection::STATE_ACTIVATING);
}

TEST_F(AosConnectionTest, Deactivate_NotInvokeDeactivateIfActivationNotRequested)
{
    m_pTestAosConnection->SetActivationRequested(IMS_FALSE);

    EXPECT_CALL(m_objMockINetworkConnection, Deactivate(_)).Times(0);

    m_pTestAosConnection->Deactivate();

    EXPECT_EQ(m_pTestAosConnection->GetState(), AosConnection::STATE_IDLE);
}

TEST_F(AosConnectionTest, Deactivate_InvokeDeactivate)
{
    m_pTestAosConnection->SetActivationRequested(IMS_TRUE);

    EXPECT_CALL(m_objMockINetworkConnection, Deactivate(IMS_TRUE));

    m_pTestAosConnection->Deactivate();

    EXPECT_EQ(m_pTestAosConnection->GetState(), AosConnection::STATE_IDLE);
}

TEST_F(AosConnectionTest, GetConnectionType)
{
    EXPECT_EQ(m_pTestAosConnection->GetConnectionType(), NetworkPolicy::APN_IMS);

    m_pTestAosConnection->m_nCnxType = NetworkPolicy::APN_EMERGENCY;
    EXPECT_EQ(m_pTestAosConnection->GetConnectionType(), NetworkPolicy::APN_EMERGENCY);
}

TEST_F(AosConnectionTest, SetListener_NotAddSameListenerAgain)
{
    IMS_UINT32 nListenerSize = m_pTestAosConnection->m_objListeners.GetSize();

    // not add same listener again
    m_pTestAosConnection->SetListener(&m_objMockIAosConnectionListener);

    EXPECT_EQ(m_pTestAosConnection->m_objListeners.GetSize(), nListenerSize);
}

TEST_F(AosConnectionTest, RemoveListener_IgnoreIfNotInList)
{
    IMS_UINT32 nListenerSize = m_pTestAosConnection->m_objListeners.GetSize();

    // remove listener
    m_pTestAosConnection->RemoveListener(&m_objMockIAosConnectionListener);
    EXPECT_EQ(m_pTestAosConnection->m_objListeners.GetSize(), nListenerSize - 1);

    // ignore remove request for listener not in the list
    m_pTestAosConnection->RemoveListener(&m_objMockIAosConnectionListener);
    EXPECT_EQ(m_pTestAosConnection->m_objListeners.GetSize(), nListenerSize - 1);
}

TEST_F(AosConnectionTest, GetMtu_ReturnZeroIfNotActiveState)
{
    EXPECT_EQ(m_pTestAosConnection->GetState(), AosConnection::STATE_IDLE);
    EXPECT_CALL(m_objMockINetworkConnection, GetMtu()).Times(0);

    EXPECT_EQ(m_pTestAosConnection->GetMtu(), 0);
}

TEST_F(AosConnectionTest, GetMtuThroughNetworkConnection)
{
    m_pTestAosConnection->SetState(AosConnection::STATE_ACTIVE);
    EXPECT_CALL(m_objMockINetworkConnection, GetMtu()).Times(1).WillOnce(Return(3000));

    EXPECT_EQ(m_pTestAosConnection->GetMtu(), 3000);
}

TEST_F(AosConnectionTest, GetLocalAddressThroughNetworkConnection)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetLocalAddress(_))
            .Times(1)
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));

    EXPECT_EQ(m_pTestAosConnection->GetLocalAddress(), IpAddress::LOOPBACK);
}

TEST_F(AosConnectionTest, GetPcscfAddressThroughNetworkConnection)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetPcscfAddress(_))
            .Times(1)
            .WillOnce(ReturnRef(AStringArray::ConstNull()));

    m_pTestAosConnection->GetPcscfAddress();
}

TEST_F(AosConnectionTest, GetHostByNameThroughNetworkConnection)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetHostByName(_, _, _)).Times(1).WillOnce(Return(1));

    const AString strHostName = AString("hostName");
    ImsList<IpAddress> objIpas;
    EXPECT_EQ(m_pTestAosConnection->GetHostByName(strHostName, objIpas), 1);
}

TEST_F(AosConnectionTest, GetIfaceNameThroughNetworkConnection)
{
    const AString strIfaceName = AString("ifaceName");
    EXPECT_CALL(m_objMockINetworkConnection, GetIfaceName())
            .Times(1)
            .WillOnce(ReturnRef(strIfaceName));

    EXPECT_EQ(m_pTestAosConnection->GetIfaceName(), strIfaceName);
}

TEST_F(AosConnectionTest, CheckEpdgEnabledThroughNetworkConnection)
{
    EXPECT_CALL(m_objMockINetworkConnection, IsePDGEnabled()).Times(1).WillOnce(Return(IMS_TRUE));

    EXPECT_EQ(m_pTestAosConnection->IsEpdgEnabled(), IMS_TRUE);
}

TEST_F(AosConnectionTest, CheckIpv6PreferredThroughNetworkConnection)
{
    EXPECT_CALL(m_objMockINetworkConnection, IsIpv6Preferred()).Times(1).WillOnce(Return(IMS_TRUE));

    EXPECT_EQ(m_pTestAosConnection->IsIpv6Preferred(), IMS_TRUE);
}

TEST_F(AosConnectionTest, CheckIpcanCategoryThroughNetworkConnection)
{
    EXPECT_CALL(m_objMockINetworkConnection, IsePDGEnabled())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_EQ(m_pTestAosConnection->GetIpcanCategory(), IIpcan::CATEGORY_WLAN);
    EXPECT_EQ(m_pTestAosConnection->GetIpcanCategory(), IIpcan::CATEGORY_MOBILE);
}

TEST_F(AosConnectionTest, CheckPcoValue_ReturnTrueIfConfiguredAsLimitedServiceValue)
{
    m_pTestAosConnection->SetCarrierSignalPcoValue(TestAosConnection::PCO_LIMITED_SERVICE_VALUE);
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTestAosConnection->IsLimitedServicePcoValue());
}

TEST_F(AosConnectionTest, CheckPcoValue_ReturnFalseIfNotSupportLimitedAdminSmsMode)
{
    m_pTestAosConnection->SetCarrierSignalPcoValue(TestAosConnection::PCO_LIMITED_SERVICE_VALUE);
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_FALSE(m_pTestAosConnection->IsLimitedServicePcoValue());
}

TEST_F(AosConnectionTest, CheckPcoValue_ReturnFalseIfConfiguredAsNotLimitedServiceValue)
{
    m_pTestAosConnection->SetCarrierSignalPcoValue(TestAosConnection::PCO_INVALID_VALUE);
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_FALSE(m_pTestAosConnection->IsLimitedServicePcoValue());
}

TEST_F(AosConnectionTest, StateToString_ReturnStringOfCurrentState)
{
    IMS_SINT32 nInvalidState = -1;

    EXPECT_STREQ(m_pTestAosConnection->StateToString(IAosConnection::STATE_IDLE), "STATE_IDLE");
    EXPECT_STREQ(m_pTestAosConnection->StateToString(IAosConnection::STATE_ACTIVE), "STATE_ACTIVE");
    EXPECT_STREQ(m_pTestAosConnection->StateToString(IAosConnection::STATE_ACTIVATING),
            "STATE_ACTIVATING");
    EXPECT_STREQ(m_pTestAosConnection->StateToString(nInvalidState), "__INVALID__");
}

TEST_F(AosConnectionTest, NotUpdateIpcanForNonImsConnectionType)
{
    m_pTestAosConnection->m_nCnxType = NetworkPolicy::APN_EMERGENCY;

    EXPECT_CALL(m_objMockIAosTransaction, SetWlan(_)).Times(0);

    m_pTestAosConnection->UpdateIpcanForImsTraffic();
}

TEST_F(AosConnectionTest, HandleOnConnected_NotifyStateChanged)
{
    EXPECT_CALL(m_objMockIAosTransaction, SetWlan(_));
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_));

    m_pTestAosConnection->NetworkConnection_OnConnected(&m_objMockINetworkConnection);

    EXPECT_EQ(m_pTestAosConnection->GetState(), AosConnection::STATE_ACTIVE);
}

TEST_F(AosConnectionTest, HandleOnDisconnected_NotifyStateChanged)
{
    m_pTestAosConnection->SetState(AosConnection::STATE_ACTIVE);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_));

    m_pTestAosConnection->NetworkConnection_OnDisconnected(&m_objMockINetworkConnection, 0);

    EXPECT_EQ(m_pTestAosConnection->GetState(), AosConnection::STATE_IDLE);
}

TEST_F(AosConnectionTest, HandleOnConnectionFailed_NotifyConnectionFailed)
{
    m_pTestAosConnection->SetState(AosConnection::STATE_ACTIVATING);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_ConnectionFailed());

    m_pTestAosConnection->NetworkConnection_OnConnectionFailed(&m_objMockINetworkConnection, 0);

    EXPECT_EQ(m_pTestAosConnection->GetState(), AosConnection::STATE_IDLE);
}

TEST_F(AosConnectionTest, HandleOnIpChanged_NotNotifyIpChangedIfNotConnected)
{
    EXPECT_EQ(m_pTestAosConnection->GetState(), AosConnection::STATE_IDLE);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpChanged()).Times(0);

    m_pTestAosConnection->NetworkConnection_OnIpChanged(&m_objMockINetworkConnection);
}

TEST_F(AosConnectionTest, HandleOnIpChanged_NotifyIpChanged)
{
    m_pTestAosConnection->SetState(AosConnection::STATE_ACTIVE);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpChanged());

    m_pTestAosConnection->NetworkConnection_OnIpChanged(&m_objMockINetworkConnection);
}

TEST_F(AosConnectionTest, HandleOnIpcanChanged_NotifyIpcanChanged)
{
    EXPECT_CALL(m_objMockIAosTransaction, SetWlan(_));
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpcanCatChanged());

    m_pTestAosConnection->NetworkConnection_OnIpcanChanged(&m_objMockINetworkConnection);
}

TEST_F(AosConnectionTest, HandleOnPcscfChanged_NotifyPcscfChanged)
{
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_PcscfChanged());

    m_pTestAosConnection->NetworkConnection_OnPcscfChanged(&m_objMockINetworkConnection);
}

TEST_F(AosConnectionTest, NotHandleNetworkConnectionCallbackForOtherNetworkConnection)
{
    MockINetworkConnection m_objMockOtherNetworkConnection;

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_)).Times(0);
    m_pTestAosConnection->NetworkConnection_OnConnected(&m_objMockOtherNetworkConnection);
    m_pTestAosConnection->NetworkConnection_OnDisconnected(&m_objMockOtherNetworkConnection, 0);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_ConnectionFailed()).Times(0);
    m_pTestAosConnection->NetworkConnection_OnConnectionFailed(&m_objMockOtherNetworkConnection, 0);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpChanged()).Times(0);
    m_pTestAosConnection->NetworkConnection_OnIpChanged(&m_objMockOtherNetworkConnection);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpcanCatChanged()).Times(0);
    m_pTestAosConnection->NetworkConnection_OnIpcanChanged(&m_objMockOtherNetworkConnection);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_PcscfChanged()).Times(0);
    m_pTestAosConnection->NetworkConnection_OnPcscfChanged(&m_objMockOtherNetworkConnection);
}

TEST_F(AosConnectionTest, NotifyExceptionHandlingForNullListener_Ignore)
{
    m_pTestAosConnection->SetListener(IMS_NULL);
    EXPECT_EQ(m_pTestAosConnection->m_objListeners.GetSize(), 2);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_)).Times(1);

    m_pTestAosConnection->Notify();
}

TEST_F(AosConnectionTest, NotifyExceptionHandlingForUnknownType_Ignore)
{
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_)).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_ConnectionFailed()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpChanged()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpcanCatChanged()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_PcscfChanged()).Times(0);

    IMS_UINT32 nUnknownType = 100;
    m_pTestAosConnection->Notify(nUnknownType);
}
