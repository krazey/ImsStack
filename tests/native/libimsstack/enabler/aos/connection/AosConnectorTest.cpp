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

#include "IIpcan.h"
#include "ServiceNetworkPolicy.h"
#include "connection/AosConnector.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosConnectorListener.h"
#include "interface/MockIAosPcscf.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_UINT32 TIMER_DURATION_FIVE_SEC = 5;

class AosConnectorTest : public ::testing::Test
{
public:
    AosConnector* pAosConnector;
    MockIAosAppContext objIMockAosAppContext;
    MockIAosConnection objMockIAosConnection;
    MockIAosConnectorListener objMockIAosConnectorListener;
    MockIAosPcscf objMockIAosPcscf;

protected:
    virtual void SetUp() override
    {
        EXPECT_CALL(objIMockAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(objIMockAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(objIMockAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&objMockIAosConnection));

        EXPECT_CALL(objIMockAosAppContext, GetPcscf())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&objMockIAosPcscf));

        pAosConnector = new AosConnector(static_cast<IAosAppContext*>(&objIMockAosAppContext));
        ASSERT_TRUE(pAosConnector != nullptr);
    }

    virtual void TearDown() override
    {
        if (pAosConnector)
        {
            delete pAosConnector;
        }
    }

    IAosConnection* GetConnection() { return pAosConnector->m_piConnection; }

    IAosPcscf* GetPcscf() { return pAosConnector->m_piPcscf; }

    void SetState(IN IMS_UINT32 nState) { pAosConnector->SetState(nState); }

    IMS_UINT32 GetState() { return pAosConnector->m_nState; }

    void SetFeature(IN IMS_UINT32 nFeature) { pAosConnector->m_nPendingFeature |= nFeature; }

    IMS_UINT32 GetFeature() { return pAosConnector->m_nPendingFeature; }

    void ClearFeature() { pAosConnector->ClearPending(); }

    void SetDataConnected(IN IMS_BOOL bConnected) { pAosConnector->m_bDataConnected = bConnected; }

    IMS_BOOL IsDataConnected() { return pAosConnector->IsDataConnected(); }

    void SetEmergencyType(IN IMS_BOOL bEmergency) { pAosConnector->m_bEmergencyType = bEmergency; }

    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
    {
        pAosConnector->StartTimer(nType, nDuration);
    }

    void StopTimer(IN IMS_UINT32 nType) { pAosConnector->StopTimer(nType); }

    void NotifyTimerExpired(IN IMS_UINT32 nType)
    {
        switch (nType)
        {
            case AosConnector::TIMER_IPV6:
                pAosConnector->Timer_TimerExpired(pAosConnector->m_piIpv6Timer);
                break;

            case AosConnector::TIMER_STOP_DELAY:
                pAosConnector->Timer_TimerExpired(pAosConnector->m_piStopDelayTimer);
                break;

            case AosConnector::TIMER_READY_RECOVERY:
                pAosConnector->Timer_TimerExpired(pAosConnector->m_piReadyRecoveryTimer);
                break;
        }
    }

    IMS_BOOL IsTimerRunning(IN IMS_UINT32 nType) { return pAosConnector->IsTimerRunning(nType); }

    IMS_BOOL IsListenerExist() { return (pAosConnector->m_piListener != IMS_NULL) ? true : false; }

    void NotifyStateChanged(IN IMS_UINT32 nState)
    {
        pAosConnector->AosConnection_StateChanged(nState);
    }

    void NotifyIpChanged() { pAosConnector->AosConnection_IpChanged(); }

    void NotifyIpcanChanged() { pAosConnector->AosConnection_IpcanCatChanged(); }

    void NotifyPcscfChanged() { pAosConnector->AosConnection_PcscfChanged(); }

    void NotifyConnectionFailed() { pAosConnector->AosConnection_ConnectionFailed(); }

    void NotifyPcscfConfigured(IN IMS_BOOL bResult) { pAosConnector->Pcscf_NotifyResult(bResult); }

    void CleanUp() { pAosConnector->CleanUp(); }
};

TEST_F(AosConnectorTest, Constructor)
{
    EXPECT_NE(GetConnection(), nullptr);
    EXPECT_NE(GetPcscf(), nullptr);
}

TEST_F(AosConnectorTest, Start)
{
    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Activated()).Times(1);
    EXPECT_CALL(objMockIAosConnection, Activate()).Times(1).WillOnce(Return(true));

    // notify Activated because already ready
    SetState(AosConnector::STATE_READY);
    EXPECT_FALSE(pAosConnector->Start());
    SetState(AosConnector::STATE_IDLE);

    // it do not invoke Activate() because connector is pending
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    EXPECT_FALSE(pAosConnector->Start());
    ClearFeature();

    // invoke Activate()
    EXPECT_TRUE(pAosConnector->Start());
}

TEST_F(AosConnectorTest, Stop)
{
    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE))
            .Times(1);
    EXPECT_CALL(objMockIAosConnection, Deactivate()).Times(1);

    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);
    StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_FIVE_SEC);
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    SetState(AosConnector::STATE_READY);

    pAosConnector->Stop();

    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
    EXPECT_TRUE(GetFeature() == AosConnector::PENDING_NONE);
    EXPECT_TRUE(GetState() == AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, StopWithDelay)
{
    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE))
            .Times(1);
    EXPECT_CALL(objMockIAosConnection, Deactivate()).Times(1);

    // invoke Stop() immediately
    pAosConnector->Stop(0);
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_STOP_DELAY));

    // start timer before invoke Stop()
    pAosConnector->Stop(TIMER_DURATION_FIVE_SEC);
    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
    StopTimer(AosConnector::TIMER_STOP_DELAY);
}

TEST_F(AosConnectorTest, SetListener)
{
    EXPECT_FALSE(IsListenerExist());
    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_TRUE(IsListenerExist());
    CleanUp();
}

TEST_F(AosConnectorTest, IsReady)
{
    // TIMER_STOP_DELAY is running
    StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_FIVE_SEC);
    EXPECT_FALSE(pAosConnector->IsReady());
    StopTimer(AosConnector::TIMER_STOP_DELAY);

    // current state is not STATE_READY
    SetState(AosConnector::STATE_IDLE);
    EXPECT_FALSE(pAosConnector->IsReady());

    // current state is STATE_READY
    SetState(AosConnector::STATE_READY);
    EXPECT_TRUE(pAosConnector->IsReady());
}

TEST_F(AosConnectorTest, StateChanged_Active_AlreadyReadyState)
{
    // it do not handle state change because it is already STATE_READY
    SetState(AosConnector::STATE_READY);
    NotifyStateChanged(IAosConnection::STATE_ACTIVE);
    EXPECT_FALSE(IsDataConnected());
}

TEST_F(AosConnectorTest, StateChanged_Active_Ipv6DelayRequired)
{
    SetState(AosConnector::STATE_IDLE);
    ClearFeature();
    EXPECT_CALL(objMockIAosConnection, GetConnectionType())
            .Times(1)
            .WillOnce(Return(NetworkPolicy::APN_NONE));
    EXPECT_CALL(objMockIAosConnection, GetIpcanCategory())
            .Times(1)
            .WillOnce(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_CALL(objMockIAosConnection, GetLocalAddress(_))
            .Times(1)
            .WillOnce(ReturnRef(IPAddress::LOOPBACK));

    // it do not handle state change because need to wait IPv6 address
    NotifyStateChanged(IAosConnection::STATE_ACTIVE);
    EXPECT_TRUE(IsDataConnected());
    EXPECT_TRUE(GetFeature() == AosConnector::PENDING_IPV6_DELAY);
    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_IPV6));
    StopTimer(AosConnector::TIMER_IPV6);
}

TEST_F(AosConnectorTest, StateChanged_Active_ConfigurePcscf)
{
    SetState(AosConnector::STATE_IDLE);
    ClearFeature();
    EXPECT_CALL(objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_NONE));
    EXPECT_CALL(objMockIAosConnection, GetIpcanCategory())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IIpcan::CATEGORY_WLAN));
    EXPECT_CALL(objMockIAosPcscf, Configure(_)).Times(AnyNumber());
    EXPECT_CALL(objMockIAosPcscf, IsConfigured())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Activated()).Times(1);

    // success to configure PCSCF address
    NotifyStateChanged(IAosConnection::STATE_ACTIVE);
    EXPECT_TRUE(GetState() == AosConnector::STATE_READY);

    SetState(AosConnector::STATE_IDLE);
    ClearFeature();

    // fail to configure PCSCF address
    NotifyStateChanged(IAosConnection::STATE_ACTIVE);
    EXPECT_TRUE(GetFeature() == AosConnector::PENDING_PCSCF_CONFIG_READY);
}

TEST_F(AosConnectorTest, StateChanged_Idle)
{
    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(
            objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_DISCONNECTED))
            .Times(1);
    NotifyStateChanged(IAosConnection::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Ready)
{
    AStringArray strPcscfs;
    strPcscfs.AddElement("0.0.0.0");
    EXPECT_CALL(objMockIAosPcscf, GetPcscfs()).Times(2).WillRepeatedly(ReturnRef(strPcscfs));
    EXPECT_CALL(objMockIAosConnection, GetLocalAddress(_))
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(IPAddress::LOOPBACK));

    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_IP_CHANGED))
            .Times(1);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED))
            .Times(1);

    // Success to select IP version
    SetState(AosConnector::STATE_READY);
    NotifyIpChanged();

    // Fail to select IP version
    strPcscfs.RemoveElementAt(0);
    NotifyIpChanged();
    EXPECT_TRUE(GetState() == AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Idle)
{
    EXPECT_CALL(objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_NONE));
    EXPECT_CALL(objMockIAosConnection, GetLocalAddress(_))
            .Times(1)
            .WillOnce(ReturnRef(IPAddress::LOOPBACK));
    EXPECT_CALL(objMockIAosPcscf, Configure(_)).Times(AnyNumber());
    EXPECT_CALL(objMockIAosPcscf, IsConfigured())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Activated()).Times(1);

    SetState(AosConnector::STATE_IDLE);
    SetDataConnected(IMS_TRUE);

    // changed as IPv4 address during waiting for IPv6 address
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    NotifyIpChanged();
    EXPECT_TRUE(GetFeature() == AosConnector::PENDING_IPV6_DELAY);
    ClearFeature();

    // PCSCF address is not configured yet
    NotifyIpChanged();
    EXPECT_TRUE(GetFeature() == AosConnector::PENDING_PCSCF_CONFIG_READY);
    ClearFeature();

    // both local address and PCSCF address is available
    NotifyIpChanged();
    EXPECT_TRUE(GetState() == AosConnector::STATE_READY);
}

TEST_F(AosConnectorTest, IpcanCatChanged)
{
    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(
            objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_IPCAN_CAT_CHANGED))
            .Times(1);

    // notify IP change
    NotifyIpcanChanged();
}

TEST_F(AosConnectorTest, PcscfChanged)
{
    MockIAosApplication objMockIAosApplication;
    EXPECT_CALL(objIMockAosAppContext, GetApp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&objMockIAosApplication));
    EXPECT_CALL(objMockIAosApplication, GetAppState())
            .Times(3)
            .WillOnce(Return(IAosApplication::STATE_NOTREADY))
            .WillOnce(Return(IAosApplication::STATE_CONNECTED))
            .WillOnce(Return(IAosApplication::STATE_CONNECTED));
    EXPECT_CALL(objMockIAosPcscf, CheckAndProcessChangeFromPco())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED))
            .Times(1);

    // PCSCF change is not available due to the app state
    NotifyPcscfChanged();

    // ignore PCSCF change
    NotifyPcscfChanged();

    // notify PCSCF change
    NotifyPcscfChanged();
}

TEST_F(AosConnectorTest, ConnectionFailed)
{
    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_PERMANENTLY_FAILED))
            .Times(1);

    // notify Deactivated
    NotifyConnectionFailed();
}

TEST_F(AosConnectorTest, Pcscf_NotifyResult)
{
    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Activated()).Times(1);

    // Already ready state
    SetState(AosConnector::STATE_READY);
    NotifyPcscfConfigured(IMS_TRUE);
    SetState(AosConnector::STATE_IDLE);

    // Data connection is not connected
    SetDataConnected(IMS_FALSE);
    NotifyPcscfConfigured(IMS_TRUE);
    SetDataConnected(IMS_TRUE);

    // Pending
    SetFeature(AosConnector::PENDING_PCSCF_CONFIG_READY);
    NotifyPcscfConfigured(IMS_TRUE);
    ClearFeature();

    // notify Activated
    NotifyPcscfConfigured(IMS_TRUE);
}

TEST_F(AosConnectorTest, Ipv6TimerExpired)
{
    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    SetState(AosConnector::STATE_IDLE);
    SetDataConnected(IMS_TRUE);

    EXPECT_CALL(objMockIAosConnection, GetConnectionType())
            .Times(2)
            .WillOnce(Return(NetworkPolicy::APN_IMS))
            .WillOnce(Return(NetworkPolicy::APN_NONE));
    EXPECT_CALL(objMockIAosPcscf, IsSinglePcoScheme()).Times(1).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(objMockIAosPcscf, IsAsyncDnsDiscovery()).Times(1).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosPcscf, Configure(_)).Times(AnyNumber());
    EXPECT_CALL(objMockIAosPcscf, IsConfigured())
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Activated()).Times(1);

    // PCSCF is not configured yet
    NotifyTimerExpired(AosConnector::TIMER_IPV6);

    ClearFeature();
    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);

    // notify Activated
    NotifyTimerExpired(AosConnector::TIMER_IPV6);
    EXPECT_TRUE(GetState() == AosConnector::STATE_READY);
}

TEST_F(AosConnectorTest, StopDelayTimerExpired)
{
    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE))
            .Times(1);

    SetState(AosConnector::STATE_READY);
    StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_FIVE_SEC);

    // notify Deactivated
    NotifyTimerExpired(AosConnector::TIMER_STOP_DELAY);
    EXPECT_TRUE(GetState() == AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, ReadyRecoveryTimerExpired)
{
    pAosConnector->SetListener(&objMockIAosConnectorListener);
    EXPECT_CALL(objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE))
            .Times(1);

    SetState(AosConnector::STATE_READY);
    StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_FIVE_SEC);

    // notify Deactivated
    NotifyTimerExpired(AosConnector::TIMER_READY_RECOVERY);
    EXPECT_TRUE(GetState() == AosConnector::STATE_IDLE);
}