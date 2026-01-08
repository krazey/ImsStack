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

#define DECLARE_USING(Base)                           \
    using Base::Notify;                               \
    using Base::SetActivationRequested;               \
    using Base::SetState;                             \
    using Base::UpdateIpcanForImsTraffic;             \
    using Base::NetworkConnection_OnConnected;        \
    using Base::NetworkConnection_OnDisconnected;     \
    using Base::NetworkConnection_OnConnectionFailed; \
    using Base::NetworkConnection_OnIpChanged;        \
    using Base::NetworkConnection_OnIpcanChanged;     \
    using Base::NetworkConnection_OnPcscfChanged;

const IMS_SINT32 SLOT_ID = 0;

class TestAosConnection : public AosConnection
{
public:
    DECLARE_USING(AosConnection)

    explicit inline TestAosConnection(IN IAosAppContext* piAppContext) :
            AosConnection(piAppContext)
    {
    }

    inline void SetConnectionType(IN IMS_SINT32 nType) { m_nCnxType = nType; }
    inline INetworkConnection* GetNetworkConnection() { return m_piConnection; }
    inline ImsList<IAosConnectionListener*> GetListeners() { return m_objListeners; }
};

class AosConnectionTest : public ::testing::Test
{
public:
    inline AosConnectionTest() :
            m_pAosConnection(IMS_NULL)
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
    TestAosConnection* m_pAosConnection;
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
    void SetUp() override
    {
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetStaticProfile())
                .WillByDefault(Return(m_pAosStaticProfile));

        m_pAosConnection = new TestAosConnection(&m_objMockIAosAppContext);
        ASSERT_TRUE(m_pAosConnection != nullptr);

        m_pAosConnection->SetListener(&m_objMockIAosConnectionListener);
    }

    void TearDown() override
    {
        if (m_pAosConnection)
        {
            delete m_pAosConnection;
            m_pAosConnection = IMS_NULL;
        }
    }
};

TEST_F(AosConnectionTest, ConnectionIsNullIfFailToCreateConnectionWhenConstructAosConnection)
{
    // set listener as IMS_NULL when destrcut AosConnection
    EXPECT_CALL(m_objMockINetworkConnection, SetListener(IMS_NULL));
    if (m_pAosConnection)
    {
        delete m_pAosConnection;
    }

    // set to return IMS_NULL when NetworkService create connection
    m_objNetworkService.SetConnection(IMS_NULL);

    // not set listenr because NetworkService return IMS_NULL for CreateConnection
    EXPECT_CALL(m_objMockINetworkConnection, SetListener(_)).Times(0);

    m_pAosConnection = new TestAosConnection(&m_objMockIAosAppContext);

    EXPECT_EQ(m_pAosConnection->GetNetworkConnection(), nullptr);

    m_objNetworkService.SetConnection(&m_objMockINetworkConnection);
}

TEST_F(AosConnectionTest, NotifyStateChangeImmediatelyIfAlreadyInActiveStateWhenActivate)
{
    m_pAosConnection->SetState(AosConnection::STATE_ACTIVE);

    // notify state change without invoking Activate
    EXPECT_CALL(m_objMockIAosConnectionListener,
            AosConnection_StateChanged(AosConnection::STATE_ACTIVE));
    EXPECT_CALL(m_objMockINetworkConnection, Activate(_)).Times(0);

    m_pAosConnection->Activate();

    EXPECT_FALSE(m_pAosConnection->IsActivationRequested());
}

TEST_F(AosConnectionTest, DoNotInvokeActivateAgainIfAlreadyRequestedWhenActivate)
{
    m_pAosConnection->SetActivationRequested(IMS_TRUE);

    // return without invoking Activate
    EXPECT_CALL(m_objMockINetworkConnection, Activate(_)).Times(0);

    m_pAosConnection->Activate();
}

TEST_F(AosConnectionTest, NotifyStateChangeImmediatelyIfResultIsDoneWhenActivate)
{
    ON_CALL(m_objMockINetworkConnection, Activate(_))
            .WillByDefault(Return(INetworkConnection::RESULT_DONE));

    // notify state change
    EXPECT_CALL(m_objMockIAosConnectionListener,
            AosConnection_StateChanged(AosConnection::STATE_ACTIVE));

    m_pAosConnection->Activate();

    EXPECT_TRUE(m_pAosConnection->IsActivationRequested());
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_ACTIVE);
}

TEST_F(AosConnectionTest, SetAsActivatingStateIfNotYetDoneWhenActivate)
{
    ON_CALL(m_objMockINetworkConnection, Activate(_))
            .WillByDefault(Return(INetworkConnection::RESULT_DOING));

    // not notify state change because it is not yet complete
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_)).Times(0);

    m_pAosConnection->Activate();

    EXPECT_TRUE(m_pAosConnection->IsActivationRequested());
    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_ACTIVATING);
}

TEST_F(AosConnectionTest, NotInvokeDeactivateIfActivationWasNotRequestedWhenDeactivate)
{
    m_pAosConnection->SetActivationRequested(IMS_FALSE);

    EXPECT_CALL(m_objMockINetworkConnection, Deactivate(_)).Times(0);

    m_pAosConnection->Deactivate();

    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_IDLE);
}

TEST_F(AosConnectionTest, InvokeDeactivateIfActivationWasRequestedWhenDeactivate)
{
    m_pAosConnection->SetActivationRequested(IMS_TRUE);

    EXPECT_CALL(m_objMockINetworkConnection, Deactivate(IMS_TRUE));

    m_pAosConnection->Deactivate();

    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_IDLE);
}

TEST_F(AosConnectionTest, GetConnectionTypeReturnsCurrentConnectionType)
{
    m_pAosConnection->SetConnectionType(NetworkPolicy::APN_IMS);
    EXPECT_EQ(m_pAosConnection->GetConnectionType(), NetworkPolicy::APN_IMS);

    m_pAosConnection->SetConnectionType(NetworkPolicy::APN_EMERGENCY);
    EXPECT_EQ(m_pAosConnection->GetConnectionType(), NetworkPolicy::APN_EMERGENCY);
}

TEST_F(AosConnectionTest, NotAddNullOrSameListenerAgainWhenSetListener)
{
    IMS_UINT32 nListenerSize = m_pAosConnection->GetListeners().GetSize();

    // not add NULL listener
    m_pAosConnection->SetListener(IMS_NULL);

    // not add same listener again
    m_pAosConnection->SetListener(&m_objMockIAosConnectionListener);

    EXPECT_EQ(m_pAosConnection->GetListeners().GetSize(), nListenerSize);
}

TEST_F(AosConnectionTest, IgnoreIfListenerIsNullOrNotInListWhenRemoveListener)
{
    IMS_UINT32 nListenerSize = m_pAosConnection->GetListeners().GetSize();

    // remove listener
    m_pAosConnection->RemoveListener(&m_objMockIAosConnectionListener);
    EXPECT_EQ(m_pAosConnection->GetListeners().GetSize(), nListenerSize - 1);

    // ignore remove request for NULL listener
    m_pAosConnection->RemoveListener(IMS_NULL);
    EXPECT_EQ(m_pAosConnection->GetListeners().GetSize(), nListenerSize - 1);

    // ignore remove request for listener not in the list
    m_pAosConnection->RemoveListener(&m_objMockIAosConnectionListener);
    EXPECT_EQ(m_pAosConnection->GetListeners().GetSize(), nListenerSize - 1);
}

TEST_F(AosConnectionTest, ReturnZeroIfNotActiveStateWhenGetMtu)
{
    m_pAosConnection->SetState(AosConnection::STATE_IDLE);

    EXPECT_CALL(m_objMockINetworkConnection, GetMtu()).Times(0);

    IMS_SINT32 nResult = m_pAosConnection->GetMtu();

    EXPECT_EQ(nResult, 0);
}

TEST_F(AosConnectionTest, AcquireThroughNetworkConnectionWhenGetMtu)
{
    m_pAosConnection->SetState(AosConnection::STATE_ACTIVE);

    IMS_SINT32 nExpectedMtu = 3000;
    EXPECT_CALL(m_objMockINetworkConnection, GetMtu()).WillOnce(Return(nExpectedMtu));

    IMS_SINT32 nResult = m_pAosConnection->GetMtu();

    EXPECT_EQ(nResult, nExpectedMtu);
}

TEST_F(AosConnectionTest, AcquireThroughNetworkConnectionWhenGetLocalAddress)
{
    const IpAddress objExpectedAddress = IpAddress(AString("1.1.1.1"));

    EXPECT_CALL(m_objMockINetworkConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(objExpectedAddress));

    const IpAddress& objResult = m_pAosConnection->GetLocalAddress();

    EXPECT_TRUE(objExpectedAddress.Equals(objResult));
}

TEST_F(AosConnectionTest, AcquireThroughNetworkConnectionWhenGetPcscfAddress)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetPcscfAddress(_))
            .WillOnce(ReturnRef(AStringArray::ConstNull()));

    m_pAosConnection->GetPcscfAddress();
}

TEST_F(AosConnectionTest, AcquireThroughNetworkConnectionWhenGetHostByName)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetHostByName(_, _, _)).WillOnce(Return(1));

    const AString strHostName = AString("hostName");
    ImsList<IpAddress> objIpas;
    m_pAosConnection->GetHostByName(strHostName, objIpas);
}

TEST_F(AosConnectionTest, AcquireThroughNetworkConnectionWhenGetIfaceName)
{
    const AString strIfaceName = AString("ifaceName");
    EXPECT_CALL(m_objMockINetworkConnection, GetIfaceName()).WillOnce(ReturnRef(strIfaceName));

    EXPECT_EQ(m_pAosConnection->GetIfaceName(), strIfaceName);
}

TEST_F(AosConnectionTest, IsEpdgEnabledChecksThroughNetworkConnection)
{
    EXPECT_CALL(m_objMockINetworkConnection, IsePDGEnabled()).WillOnce(Return(IMS_TRUE));

    IMS_BOOL bResult = m_pAosConnection->IsEpdgEnabled();

    EXPECT_EQ(bResult, IMS_TRUE);
}

TEST_F(AosConnectionTest, IsIpv6PreferredChecksThroughNetworkConnection)
{
    EXPECT_CALL(m_objMockINetworkConnection, IsIpv6Preferred()).WillOnce(Return(IMS_TRUE));

    IMS_BOOL bResult = m_pAosConnection->IsIpv6Preferred();

    EXPECT_EQ(bResult, IMS_TRUE);
}

TEST_F(AosConnectionTest, ReturnWlanCategoryIfEpdgIsEnabledWhenGetIpcanCategory)
{
    EXPECT_CALL(m_objMockINetworkConnection, IsePDGEnabled()).WillOnce(Return(IMS_TRUE));

    IMS_SINT32 nResult = m_pAosConnection->GetIpcanCategory();

    EXPECT_EQ(nResult, IIpcan::CATEGORY_WLAN);
}

TEST_F(AosConnectionTest, ReturnMobileCategoryIfEpdgIsNotEnabledWhenGetIpcanCategory)
{
    EXPECT_CALL(m_objMockINetworkConnection, IsePDGEnabled()).WillOnce(Return(IMS_FALSE));

    IMS_SINT32 nResult = m_pAosConnection->GetIpcanCategory();

    EXPECT_EQ(nResult, IIpcan::CATEGORY_MOBILE);
}

TEST_F(AosConnectionTest, IsLimitedServicePcoValueReturnsTrueIfLimitedAdminSmsModeIsSupported)
{
    m_pAosConnection->SetCarrierSignalPcoValue(AosConnection::PCO_LIMITED_SERVICE_VALUE);
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));

    IMS_BOOL bResult = m_pAosConnection->IsLimitedServicePcoValue();

    EXPECT_TRUE(bResult);
}

TEST_F(AosConnectionTest, IsLimitedServicePcoValueReturnsFalseIfLimitedAdminSmsModeIsNotSupported)
{
    m_pAosConnection->SetCarrierSignalPcoValue(AosConnection::PCO_LIMITED_SERVICE_VALUE);
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_FALSE));

    IMS_BOOL bResult = m_pAosConnection->IsLimitedServicePcoValue();

    EXPECT_FALSE(bResult);
}

TEST_F(AosConnectionTest, IsLimitedServicePcoValueReturnsFalseIfInvalidPcoValue)
{
    m_pAosConnection->SetCarrierSignalPcoValue(AosConnection::PCO_INVALID_VALUE);
    ON_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .WillByDefault(Return(IMS_TRUE));

    IMS_BOOL bResult = m_pAosConnection->IsLimitedServicePcoValue();

    EXPECT_FALSE(bResult);
}

TEST_F(AosConnectionTest, ReturnStringOfCurrentStateWhenStateToString)
{
    IMS_SINT32 nInvalidState = -1;

    EXPECT_STREQ(m_pAosConnection->StateToString(IAosConnection::STATE_IDLE), "STATE_IDLE");
    EXPECT_STREQ(m_pAosConnection->StateToString(IAosConnection::STATE_ACTIVE), "STATE_ACTIVE");
    EXPECT_STREQ(
            m_pAosConnection->StateToString(IAosConnection::STATE_ACTIVATING), "STATE_ACTIVATING");
    EXPECT_STREQ(m_pAosConnection->StateToString(nInvalidState), "__INVALID__");
}

TEST_F(AosConnectionTest, DoNotUpdateForNonImsConnectionTypeWhenUpdateIpcanForImsTraffic)
{
    m_pAosConnection->SetConnectionType(NetworkPolicy::APN_EMERGENCY);

    EXPECT_CALL(m_objMockIAosTransaction, SetWlan(_)).Times(0);

    m_pAosConnection->UpdateIpcanForImsTraffic();
}

TEST_F(AosConnectionTest, NotifyStateChangedWhenOnConnected)
{
    EXPECT_CALL(m_objMockIAosTransaction, SetWlan(_));
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_));

    m_pAosConnection->NetworkConnection_OnConnected(&m_objMockINetworkConnection);

    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_ACTIVE);
}

TEST_F(AosConnectionTest, NotifyStateChangedOnDisconnected)
{
    m_pAosConnection->SetState(AosConnection::STATE_ACTIVE);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_));

    m_pAosConnection->NetworkConnection_OnDisconnected(&m_objMockINetworkConnection, 0);

    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_IDLE);
}

TEST_F(AosConnectionTest, NotifyConnectionFailedWhenOnConnectionFailed)
{
    m_pAosConnection->SetState(AosConnection::STATE_ACTIVATING);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_ConnectionFailed());

    m_pAosConnection->NetworkConnection_OnConnectionFailed(&m_objMockINetworkConnection, 0);

    EXPECT_EQ(m_pAosConnection->GetState(), AosConnection::STATE_IDLE);
}

TEST_F(AosConnectionTest, NotNotifyIpChangedIfNotConnectedWhenOnIpChanged)
{
    m_pAosConnection->SetState(AosConnection::STATE_IDLE);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpChanged()).Times(0);

    m_pAosConnection->NetworkConnection_OnIpChanged(&m_objMockINetworkConnection);
}

TEST_F(AosConnectionTest, NotifyIpChangedWhenOnIpChanged)
{
    m_pAosConnection->SetState(AosConnection::STATE_ACTIVE);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpChanged());

    m_pAosConnection->NetworkConnection_OnIpChanged(&m_objMockINetworkConnection);
}

TEST_F(AosConnectionTest, NotifyIpcanChangedWhenOnIpcanChanged)
{
    EXPECT_CALL(m_objMockIAosTransaction, SetWlan(_));
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpcanCatChanged());

    m_pAosConnection->NetworkConnection_OnIpcanChanged(&m_objMockINetworkConnection);
}

TEST_F(AosConnectionTest, NotifyPcscfChangedWhenOnPcscfChanged)
{
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_PcscfChanged());

    m_pAosConnection->NetworkConnection_OnPcscfChanged(&m_objMockINetworkConnection);
}

TEST_F(AosConnectionTest, NotHandleNetworkConnectionCallbackForOtherNetworkConnection)
{
    MockINetworkConnection m_objMockOtherNetworkConnection;

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_)).Times(0);
    m_pAosConnection->NetworkConnection_OnConnected(&m_objMockOtherNetworkConnection);
    m_pAosConnection->NetworkConnection_OnDisconnected(&m_objMockOtherNetworkConnection, 0);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_ConnectionFailed()).Times(0);
    m_pAosConnection->NetworkConnection_OnConnectionFailed(&m_objMockOtherNetworkConnection, 0);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpChanged()).Times(0);
    m_pAosConnection->NetworkConnection_OnIpChanged(&m_objMockOtherNetworkConnection);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpcanCatChanged()).Times(0);
    m_pAosConnection->NetworkConnection_OnIpcanChanged(&m_objMockOtherNetworkConnection);

    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_PcscfChanged()).Times(0);
    m_pAosConnection->NetworkConnection_OnPcscfChanged(&m_objMockOtherNetworkConnection);
}

TEST_F(AosConnectionTest, IgnoreUnknownTypeWhenNotify)
{
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_StateChanged(_)).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_ConnectionFailed()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpChanged()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_IpcanCatChanged()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectionListener, AosConnection_PcscfChanged()).Times(0);

    IMS_UINT32 nUnknownType = 100;
    m_pAosConnection->Notify(nUnknownType);
}
