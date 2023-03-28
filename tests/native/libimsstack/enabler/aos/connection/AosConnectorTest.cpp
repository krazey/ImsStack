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

#include "CarrierConfig.h"
#include "IIpcan.h"
#include "ImsPrivateProperty.h"
#include "ServiceNetworkPolicy.h"
#include "connection/AosConnector.h"
#include "interface/IAosNConfiguration.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosConnectorListener.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosRegistration.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;
const IMS_UINT32 TIMER_DURATION_FIVE_SEC = 5;
const IMS_UINT32 WAITING_PCO_VALUE_TIMEOUT_MILLIS = 2000;

class AosConnectorTest : public ::testing::Test
{
public:
    AosConnector* m_pAosConnector;
    AStringArray m_objPcscfs;
    IAosNConfiguration* m_piAosNConfiguration;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosConnectorListener m_objMockIAosConnectorListener;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosPcscf m_objMockIAosPcscf;

protected:
    virtual void SetUp() override
    {
        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(SLOT_ID));

        const AString strValue = AString("test");
        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosConnection));

        EXPECT_CALL(m_objMockIAosAppContext, GetPcscf())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosPcscf));

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration(SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration), SLOT_ID);

        m_pAosConnector = new AosConnector(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
        SetEmergencyType(IMS_FALSE);
        SetState(AosConnector::STATE_IDLE);
        SetFeature(AosConnector::PENDING_NONE);
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        if (m_pAosConnector)
        {
            m_pAosConnector->CleanUp();
            delete m_pAosConnector;
        }
    }

    void SetState(IN IMS_UINT32 nState) { m_pAosConnector->SetState(nState); }

    IMS_UINT32 GetState() { return m_pAosConnector->m_nState; }

    void SetFeature(IN IMS_UINT32 nFeature) { m_pAosConnector->m_nPendingFeature = nFeature; }

    IMS_UINT32 GetFeature() { return m_pAosConnector->m_nPendingFeature; }

    void SetDataConnected(IN IMS_BOOL bConnected) { m_pAosConnector->SetDataConnected(bConnected); }

    IMS_BOOL IsDataConnected() { return m_pAosConnector->IsDataConnected(); }

    void SetEmergencyType(IN IMS_BOOL bEmergency) { m_pAosConnector->SetEmergencyType(bEmergency); }

    IMS_BOOL IsPcscfConfigured() { return m_pAosConnector->IsPcscfConfigured(); }

    IMS_BOOL IsIpv6DelayRequired() { return m_pAosConnector->IsIpv6DelayRequired(); }

    IMS_BOOL IsPcoWaitingRequired() { return m_pAosConnector->IsPcoWaitingRequired(); }

    IMS_BOOL IsIpChangedForEmergency() { return m_pAosConnector->CheckIpChangedForEmergency(); }

    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
    {
        m_pAosConnector->StartTimer(nType, nDuration);
    }

    void StopTimer(IN IMS_UINT32 nType) { m_pAosConnector->StopTimer(nType); }

    void NotifyTimerExpired(IN IMS_UINT32 nType)
    {
        switch (nType)
        {
            case AosConnector::TIMER_IPV6:
                m_pAosConnector->Timer_TimerExpired(m_pAosConnector->m_piIpv6Timer);
                break;

            case AosConnector::TIMER_STOP_DELAY:
                m_pAosConnector->Timer_TimerExpired(m_pAosConnector->m_piStopDelayTimer);
                break;

            case AosConnector::TIMER_READY_RECOVERY:
                m_pAosConnector->Timer_TimerExpired(m_pAosConnector->m_piReadyRecoveryTimer);
                break;

            case AosConnector::TIMER_PCO_WAITING:
                m_pAosConnector->Timer_TimerExpired(m_pAosConnector->m_piPcoWaitingTimer);
                break;
        }
    }

    IMS_BOOL IsTimerRunning(IN IMS_UINT32 nType) { return m_pAosConnector->IsTimerRunning(nType); }

    void NotifyStateChanged(IN IMS_UINT32 nState)
    {
        m_pAosConnector->AosConnection_StateChanged(nState);
    }

    void NotifyIpChanged() { m_pAosConnector->AosConnection_IpChanged(); }

    void NotifyIpcanChanged() { m_pAosConnector->AosConnection_IpcanCatChanged(); }

    void NotifyPcscfChanged() { m_pAosConnector->AosConnection_PcscfChanged(); }

    void NotifyConnectionFailed() { m_pAosConnector->AosConnection_ConnectionFailed(); }

    void NotifyPcscfConfigured(IN IMS_BOOL bResult)
    {
        m_pAosConnector->Pcscf_NotifyResult(bResult);
    }

    void ServicePhone_PcoValueChanged(IN IMS_SINT32 nValue)
    {
        m_pAosConnector->ServicePhone_PcoValueChanged(nValue);
    }

    void SetIntToProperty(IN const AString& strKey, IN IMS_SINT32 nValue, IN IMS_SINT32 nSlotId)
    {
        ImsPrivateProperty* pImsPrivateProperty = ImsPrivateProperty::GetInstance();
        pImsPrivateProperty->SetInt(strKey, nValue, nSlotId);
    }

    IMS_SINT32 GetIntFromProperty(IN const AString& strKey, IN IMS_SINT32 nSlotId)
    {
        ImsPrivateProperty* pImsPrivateProperty = ImsPrivateProperty::GetInstance();
        return pImsPrivateProperty->GetInt(strKey, nSlotId);
    }
};

TEST_F(AosConnectorTest, Start_AlreadyReadyState)
{
    // it notify Activated without invoke Activate() because already ready
    EXPECT_CALL(m_objMockIAosConnection, Activate()).Times(0);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());

    SetState(AosConnector::STATE_READY);
    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    EXPECT_FALSE(m_pAosConnector->Start());
}

TEST_F(AosConnectorTest, Start_Pending)
{
    // it does not invoke Activate() because connector is now pending
    EXPECT_CALL(m_objMockIAosConnection, Activate()).Times(0);

    SetFeature(AosConnector::PENDING_IPV6_DELAY);

    EXPECT_FALSE(m_pAosConnector->Start());
}

TEST_F(AosConnectorTest, Start)
{
    // invoke Activate() successfully
    EXPECT_CALL(m_objMockIAosConnection, Activate());

    // invoke Activate()
    EXPECT_TRUE(m_pAosConnector->Start());
}

TEST_F(AosConnectorTest, Stop)
{
    // invoke Stop() successfully
    EXPECT_CALL(m_objMockIAosConnection, Deactivate());
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE));

    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);
    StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_FIVE_SEC);
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    SetState(AosConnector::STATE_READY);
    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    m_pAosConnector->Stop();
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_NONE);
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, StopWithZeroDelay)
{
    // invoke Stop() without TIMER_STOP_DELAY
    EXPECT_CALL(m_objMockIAosConnection, Deactivate());
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE));

    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);
    StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_FIVE_SEC);
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    SetState(AosConnector::STATE_READY);
    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    m_pAosConnector->Stop();
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_NONE);
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, StopWithDelay)
{
    // invoke Stop() when TIMER_STOP_DELAY timer is expired
    EXPECT_CALL(m_objMockIAosConnection, Deactivate()).Times(1);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE))
            .Times(1);

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    // start TIMER_STOP_DELAY before invoke Stop()
    m_pAosConnector->Stop(TIMER_DURATION_FIVE_SEC);
    // if TIMER_DURATION_FIVE_SEC has already started, do not handle again
    m_pAosConnector->Stop(TIMER_DURATION_FIVE_SEC);
    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_STOP_DELAY));

    NotifyTimerExpired(AosConnector::TIMER_STOP_DELAY);
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, IsReady)
{
    // current state is not STATE_READY
    EXPECT_FALSE(m_pAosConnector->IsReady());

    // current state is STATE_READY
    SetState(AosConnector::STATE_READY);
    EXPECT_TRUE(m_pAosConnector->IsReady());

    // if TIMER_STOP_DELAY is running, it should not be treated as STATE_READY
    StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_FIVE_SEC);
    EXPECT_FALSE(m_pAosConnector->IsReady());
    StopTimer(AosConnector::TIMER_STOP_DELAY);
}

TEST_F(AosConnectorTest, IsIpv6DelayRequired_EmergencyType)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, GetEmergencyPreferredIpType())
            .Times(3)
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_4))
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_6))
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_6));
    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .Times(2)
            .WillOnce(Return(IIpcan::CATEGORY_WLAN))
            .WillOnce(Return(IIpcan::CATEGORY_MOBILE));

    SetEmergencyType(IMS_TRUE);

    // Delay is required only when the preferred ip type is IPv6 and mobile IPCAN
    EXPECT_FALSE(IsIpv6DelayRequired());
    EXPECT_FALSE(IsIpv6DelayRequired());
    EXPECT_TRUE(IsIpv6DelayRequired());
}

TEST_F(AosConnectorTest, IsIpv6DelayRequired_ImsType)
{
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_IMS));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredIpType())
            .Times(3)
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_4))
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_6))
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_6));
    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .Times(2)
            .WillOnce(Return(IIpcan::CATEGORY_WLAN))
            .WillOnce(Return(IIpcan::CATEGORY_MOBILE));

    // Delay is required only when the preferred ip type is IPv6 and mobile IPCAN
    EXPECT_FALSE(IsIpv6DelayRequired());
    EXPECT_FALSE(IsIpv6DelayRequired());
    EXPECT_TRUE(IsIpv6DelayRequired());
}

TEST_F(AosConnectorTest, IsPcoWaitingRequired_NotSupportLimitedAdminSmsMode)
{
    IMS_SINT32 nOriginValue = GetIntFromProperty(
            ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST, IMS_SLOT_0);
    SetIntToProperty(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST, 1, IMS_SLOT_0);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosConnection, GetCarrierSignalPcoValue()).Times(0);

    EXPECT_FALSE(IsPcoWaitingRequired());
    EXPECT_FALSE(IsPcoWaitingRequired());

    SetIntToProperty(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST, nOriginValue,
            IMS_SLOT_0);
}

TEST_F(AosConnectorTest, IsPcoWaitingRequired_IsNotEnabled)
{
    IMS_SINT32 nOriginValue = GetIntFromProperty(
            ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST, IMS_SLOT_0);
    SetIntToProperty(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST, 1, IMS_SLOT_0);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsSupportLimitedAdminSmsMode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, GetCarrierSignalPcoValue()).Times(0);

    EXPECT_FALSE(IsPcoWaitingRequired());
    EXPECT_FALSE(IsPcoWaitingRequired());

    SetIntToProperty(ImsPrivateProperties::Persistent::KEY_CARRIER_SIGNAL_PCO_TEST, nOriginValue,
            IMS_SLOT_0);
}

TEST_F(AosConnectorTest, CheckIpChangedForEmergency)
{
    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .WillOnce(Return(&objMockIAosRegistration));
    EXPECT_CALL(objMockIAosRegistration, GetState())
            .WillOnce(Return(IAosRegistration::STATE_OFFLINE));

    SetEmergencyType(IMS_TRUE);
    // Since registration state is offline, it return false.
    EXPECT_FALSE(IsIpChangedForEmergency());
}

TEST_F(AosConnectorTest, StateChanged_Active_AlreadyReadyState)
{
    // it does not handle changing to STATE_ACTIVE because it is already STATE_READY
    SetState(AosConnector::STATE_READY);
    NotifyStateChanged(IAosConnection::STATE_ACTIVE);
    EXPECT_FALSE(IsDataConnected());
}

TEST_F(AosConnectorTest, StateChanged_Active_Ipv6DelayRequired)
{
    // it does not handle state change because need to wait IPv6 address
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_)).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillOnce(Return(NetworkPolicy::APN_IMS));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredIpType())
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_6));
    EXPECT_CALL(m_objMockIAosConnection, GetIpcanCategory())
            .WillOnce(Return(IIpcan::CATEGORY_MOBILE));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));

    NotifyStateChanged(IAosConnection::STATE_ACTIVE);
    EXPECT_TRUE(IsDataConnected());
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_IPV6_DELAY);
    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, StateChanged_Active_Pending)
{
    // it does not handle state change because there is pending feature
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_)).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillOnce(Return(NetworkPolicy::APN_IMS));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredIpType())
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_4));

    SetFeature(AosConnector::PENDING_PCSCF_CONFIG_READY);

    NotifyStateChanged(IAosConnection::STATE_ACTIVE);
    EXPECT_TRUE(IsDataConnected());
}

TEST_F(AosConnectorTest, StateChanged_Active_FailToConfigurePcscf)
{
    // it does not handle state change because fail to configure PCSCF address
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_IMS));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredIpType())
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_4));
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_));
    EXPECT_CALL(m_objMockIAosPcscf, IsConfigured()).WillOnce(Return(IMS_FALSE));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    NotifyStateChanged(IAosConnection::STATE_ACTIVE);
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_PCSCF_CONFIG_READY);
}

TEST_F(AosConnectorTest, StateChanged_Active_EmergencyTypeFailure)
{
    // it notify Deactivated when the connection request for emergency type is failed.
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED))
            .Times(1);
    EXPECT_CALL(m_objMockIAosNConfiguration, GetEmergencyPreferredIpType())
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_4));
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_));
    EXPECT_CALL(m_objMockIAosPcscf, IsConfigured()).WillOnce(Return(IMS_FALSE));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetEmergencyType(IMS_TRUE);

    NotifyStateChanged(IAosConnection::STATE_ACTIVE);
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_NONE);
    EXPECT_FALSE(IsDataConnected());
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, StateChanged_Active_InvalidIpAddress)
{
    // it does not handle state change because IP address is not valid
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_IMS));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredIpType())
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_4));
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_));
    EXPECT_CALL(m_objMockIAosPcscf, IsConfigured()).WillOnce(Return(IMS_TRUE));
    m_objPcscfs.AddElement(AString("::1"));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV6))
            .WillOnce(ReturnRef(IpAddress::NONE));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    NotifyStateChanged(IAosConnection::STATE_ACTIVE);
    EXPECT_TRUE(IsDataConnected());
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, StateChanged_Active)
{
    // it handle the state change of Active
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(1);
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_IMS));
    EXPECT_CALL(m_objMockIAosNConfiguration, GetPreferredIpType())
            .WillOnce(Return(CarrierConfig::Assets::IP_VERSION_4));
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_));
    EXPECT_CALL(m_objMockIAosPcscf, IsConfigured()).WillOnce(Return(IMS_TRUE));
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    NotifyStateChanged(IAosConnection::STATE_ACTIVE);
    EXPECT_TRUE(IsDataConnected());
    EXPECT_EQ(GetState(), AosConnector::STATE_READY);
}

TEST_F(AosConnectorTest, StateChanged_Idle)
{
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_DISCONNECTED))
            .Times(1);
    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    NotifyStateChanged(IAosConnection::STATE_IDLE);
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_NONE);
    EXPECT_FALSE(IsDataConnected());
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Ready_InvalidIpAddress)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED));
    const IpAddress objInvalidAddress = IpAddress(AString("IpAddress"));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_)).WillOnce(ReturnRef(objInvalidAddress));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetState(AosConnector::STATE_READY);

    NotifyIpChanged();
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Ready_NoPcscfAddress)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED))
            .Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(_)).Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetState(AosConnector::STATE_READY);

    NotifyIpChanged();
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Ready_NullPcscfAddress)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED))
            .Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(_)).Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetState(AosConnector::STATE_READY);
    m_objPcscfs.AddElement(IMS_NULL);

    NotifyIpChanged();
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Ready_EmptyPcscfAddress)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED))
            .Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(_)).Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetState(AosConnector::STATE_READY);
    m_objPcscfs.AddElement("");

    NotifyIpChanged();
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Ready_InvalidFormPcscfAddress)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED))
            .Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(_)).Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetState(AosConnector::STATE_READY);
    m_objPcscfs.AddElement("PcscfAddress");

    NotifyIpChanged();
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Ready_NoMatchingIpVersion)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED))
            .Times(AnyNumber());
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(_)).Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillRepeatedly(ReturnRef(IpAddress::LOOPBACK));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetState(AosConnector::STATE_READY);
    m_objPcscfs.AddElement(AString("::1"));

    NotifyIpChanged();
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Ready)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_IP_CHANGED))
            .Times(1);
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_FAILED))
            .Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetState(AosConnector::STATE_READY);
    m_objPcscfs.AddElement(AString("0.0.0.0"));

    NotifyIpChanged();
}

TEST_F(AosConnectorTest, IpChanged_Idle_WaitMoreIPv6Address)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetDataConnected(IMS_TRUE);
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);

    NotifyIpChanged();
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_IPV6_DELAY);
    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Idle_Pending)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(_))
            .WillOnce(ReturnRef(IpAddress::IPv6LOOPBACK));
    EXPECT_CALL(m_objMockIAosPcscf, IsAsyncDnsDiscovery()).WillOnce(Return(IMS_TRUE));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetDataConnected(IMS_TRUE);
    SetFeature(AosConnector::PENDING_IPV6_DELAY | AosConnector::PENDING_PCSCF_CONFIG_READY);
    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);

    NotifyIpChanged();
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_PCSCF_CONFIG_READY);
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Idle_FailToConfigurePcscf)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, IsConfigured()).WillOnce(Return(IMS_FALSE));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetDataConnected(IMS_TRUE);

    NotifyIpChanged();
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_PCSCF_CONFIG_READY);
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Idle_InvalidIpAddress)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, IsConfigured()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillOnce(Return(NetworkPolicy::APN_IMS));
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillOnce(ReturnRef(IpAddress::NONE));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetDataConnected(IMS_TRUE);
    SetFeature(AosConnector::PENDING_PCSCF_CONFIG_READY);

    NotifyIpChanged();
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_NONE);
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, IpChanged_Idle)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(1);
    EXPECT_CALL(m_objMockIAosPcscf, IsConfigured()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillOnce(Return(NetworkPolicy::APN_IMS));
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetDataConnected(IMS_TRUE);
    SetFeature(AosConnector::PENDING_PCSCF_CONFIG_READY);

    NotifyIpChanged();
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_NONE);
    EXPECT_EQ(GetState(), AosConnector::STATE_READY);
}

TEST_F(AosConnectorTest, IpcanCatChanged)
{
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Updated(AosConnector::REASON_IPCAN_CAT_CHANGED))
            .Times(1);

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    // notify IP change
    NotifyIpcanChanged();
}

TEST_F(AosConnectorTest, PcscfChanged_AppState_NotReady)
{
    // Not handle PCSCF change because the app state is not available
    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).Times(0);
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED))
            .Times(0);
    MockIAosApplication objMockIAosApplication;
    EXPECT_CALL(m_objMockIAosAppContext, GetApp()).WillOnce(Return(&objMockIAosApplication));
    EXPECT_CALL(objMockIAosApplication, GetAppState())
            .WillOnce(Return(IAosApplication::STATE_NOTREADY));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    NotifyPcscfChanged();
}

TEST_F(AosConnectorTest, PcscfChanged_InvalidPcscf)
{
    // Not handle PCSCF change because the PCSCF is invalid
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED))
            .Times(0);
    MockIAosApplication objMockIAosApplication;
    EXPECT_CALL(m_objMockIAosAppContext, GetApp()).WillOnce(Return(&objMockIAosApplication));
    EXPECT_CALL(objMockIAosApplication, GetAppState())
            .WillOnce(Return(IAosApplication::STATE_CONNECTING));
    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).WillOnce(Return(IMS_FALSE));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    NotifyPcscfChanged();
}

TEST_F(AosConnectorTest, PcscfChanged)
{
    // handle PCSCF change
    EXPECT_CALL(
            m_objMockIAosConnectorListener, Connector_Updated(AosConnector::REASON_PCSCF_CHANGED));
    MockIAosApplication objMockIAosApplication;
    EXPECT_CALL(m_objMockIAosAppContext, GetApp()).WillOnce(Return(&objMockIAosApplication));
    EXPECT_CALL(objMockIAosApplication, GetAppState())
            .WillOnce(Return(IAosApplication::STATE_CONNECTING));
    EXPECT_CALL(m_objMockIAosPcscf, CheckAndProcessChangeFromPco()).WillOnce(Return(IMS_TRUE));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    NotifyPcscfChanged();
}

TEST_F(AosConnectorTest, ConnectionFailed)
{
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_PERMANENTLY_FAILED))
            .Times(1);

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    // notify Deactivated
    NotifyConnectionFailed();
}

TEST_F(AosConnectorTest, Pcscf_NotifyResult_ReadyState)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetState(AosConnector::STATE_READY);
    SetFeature(AosConnector::PENDING_PCSCF_CONFIG_READY);

    NotifyPcscfConfigured(IMS_TRUE);
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_NONE);
}

TEST_F(AosConnectorTest, Pcscf_NotifyResult_DataIsNotConnected)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetDataConnected(IMS_FALSE);
    SetFeature(AosConnector::PENDING_PCSCF_CONFIG_READY);

    NotifyPcscfConfigured(IMS_TRUE);
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_NONE);
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, Pcscf_NotifyResult_Pending)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetDataConnected(IMS_TRUE);
    SetFeature(AosConnector::PENDING_IPV6_DELAY | AosConnector::PENDING_PCSCF_CONFIG_READY);

    NotifyPcscfConfigured(IMS_TRUE);
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_IPV6_DELAY);
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, Pcscf_NotifyResult)
{
    // notify Activated
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(1);

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetDataConnected(IMS_TRUE);
    SetFeature(AosConnector::PENDING_PCSCF_CONFIG_READY);

    NotifyPcscfConfigured(IMS_TRUE);
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_NONE);
    EXPECT_EQ(GetState(), AosConnector::STATE_READY);
}

TEST_F(AosConnectorTest, ServicePhone_PcoValueChanged_NotReadyAndDataIsNotConnected)
{
    StartTimer(AosConnector::TIMER_PCO_WAITING, WAITING_PCO_VALUE_TIMEOUT_MILLIS);
    SetFeature(AosConnector::PENDING_PCO_WAITING);
    SetState(AosConnector::STATE_IDLE);
    SetDataConnected(IMS_FALSE);

    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
    EXPECT_TRUE(AosConnector::PENDING_PCO_WAITING & GetFeature());
    EXPECT_CALL(m_objMockIAosConnection, SetCarrierSignalPcoValue(5)).Times(1);

    ServicePhone_PcoValueChanged(5);

    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
    EXPECT_FALSE(AosConnector::PENDING_PCO_WAITING & GetFeature());
}

TEST_F(AosConnectorTest, ServicePhone_PcoValueChanged_ReadyAndLimitedModeChange)
{
    StartTimer(AosConnector::TIMER_PCO_WAITING, WAITING_PCO_VALUE_TIMEOUT_MILLIS);
    SetFeature(AosConnector::PENDING_PCO_WAITING);
    SetState(AosConnector::STATE_READY);

    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
    EXPECT_TRUE(AosConnector::PENDING_PCO_WAITING & GetFeature());
    EXPECT_CALL(m_objMockIAosConnection, SetCarrierSignalPcoValue(5)).Times(1);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .WillRepeatedly(Return(&objMockIAosRegistration));
    EXPECT_CALL(objMockIAosRegistration, GetState())
            .WillRepeatedly(Return(IAosRegistration::STATE_REGISTERED));
    EXPECT_CALL(objMockIAosRegistration, GetMode())
            .WillRepeatedly(Return(IAosRegistration::MODE_NORMAL));
    EXPECT_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue())
            .WillRepeatedly(Return(IMS_TRUE));

    // notify Activated
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_LIMITED_SERVICE_PCO))
            .Times(1);
    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    ServicePhone_PcoValueChanged(5);

    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
    EXPECT_FALSE(AosConnector::PENDING_PCO_WAITING & GetFeature());
}

TEST_F(AosConnectorTest, ServicePhone_PcoValueChanged_ReadyAndLimitedModeNotChange)
{
    StartTimer(AosConnector::TIMER_PCO_WAITING, WAITING_PCO_VALUE_TIMEOUT_MILLIS);
    SetFeature(AosConnector::PENDING_PCO_WAITING);
    SetState(AosConnector::STATE_READY);

    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
    EXPECT_TRUE(AosConnector::PENDING_PCO_WAITING & GetFeature());
    EXPECT_CALL(m_objMockIAosConnection, SetCarrierSignalPcoValue(5)).Times(1);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .WillRepeatedly(Return(&objMockIAosRegistration));
    EXPECT_CALL(objMockIAosRegistration, GetState())
            .WillRepeatedly(Return(IAosRegistration::STATE_REGISTERED));
    EXPECT_CALL(objMockIAosRegistration, GetMode())
            .WillRepeatedly(Return(IAosRegistration::MODE_LIMITED));
    EXPECT_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue())
            .WillRepeatedly(Return(IMS_TRUE));

    // notify Activated
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_LIMITED_SERVICE_PCO))
            .Times(0);
    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    ServicePhone_PcoValueChanged(5);

    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
    EXPECT_FALSE(AosConnector::PENDING_PCO_WAITING & GetFeature());
}

TEST_F(AosConnectorTest, ServicePhone_PcoValueChanged_ReadyAndDeregisteringState)
{
    StartTimer(AosConnector::TIMER_PCO_WAITING, WAITING_PCO_VALUE_TIMEOUT_MILLIS);
    SetFeature(AosConnector::PENDING_PCO_WAITING);
    SetState(AosConnector::STATE_READY);

    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
    EXPECT_TRUE(AosConnector::PENDING_PCO_WAITING & GetFeature());
    EXPECT_CALL(m_objMockIAosConnection, SetCarrierSignalPcoValue(5)).Times(1);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .WillRepeatedly(Return(&objMockIAosRegistration));
    EXPECT_CALL(objMockIAosRegistration, GetState())
            .WillRepeatedly(Return(IAosRegistration::STATE_DEREGISTERING));
    EXPECT_CALL(objMockIAosRegistration, GetMode()).Times(0);
    EXPECT_CALL(m_objMockIAosConnection, IsLimitedServicePcoValue()).Times(0);

    // notify Activated
    EXPECT_CALL(m_objMockIAosConnectorListener,
            Connector_Deactivated(AosConnector::REASON_LIMITED_SERVICE_PCO))
            .Times(0);
    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);

    ServicePhone_PcoValueChanged(5);

    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
    EXPECT_FALSE(AosConnector::PENDING_PCO_WAITING & GetFeature());
}

TEST_F(AosConnectorTest, Ipv6TimerExpired_Pending)
{
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_)).Times(0);

    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);
    SetFeature(AosConnector::PENDING_IPV6_DELAY | AosConnector::PENDING_PCSCF_CONFIG_READY);
    SetDataConnected(IMS_TRUE);

    NotifyTimerExpired(AosConnector::TIMER_IPV6);
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, Ipv6TimerExpired_ReadyState)
{
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_)).Times(0);

    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    SetState(AosConnector::STATE_READY);
    SetDataConnected(IMS_TRUE);

    NotifyTimerExpired(AosConnector::TIMER_IPV6);
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, Ipv6TimerExpired_DataIsNotConnected)
{
    EXPECT_CALL(m_objMockIAosPcscf, Configure(_)).Times(0);

    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    SetDataConnected(IMS_FALSE);

    NotifyTimerExpired(AosConnector::TIMER_IPV6);
}

TEST_F(AosConnectorTest, Ipv6TimerExpired_FailToConfigurePcscf)
{
    EXPECT_CALL(m_objMockIAosPcscf, IsConfigured()).WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillOnce(Return(NetworkPolicy::APN_IMS));
    EXPECT_CALL(m_objMockIAosPcscf, IsSinglePcoScheme()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosPcscf, IsAsyncDnsDiscovery()).WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosPcscf, Configure(_)).Times(1);

    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    SetDataConnected(IMS_TRUE);

    NotifyTimerExpired(AosConnector::TIMER_IPV6);
    EXPECT_EQ(GetFeature(), AosConnector::PENDING_PCSCF_CONFIG_READY);
    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, Ipv6TimerExpired_InvalidIpAddress)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated()).Times(0);
    EXPECT_CALL(m_objMockIAosPcscf, IsConfigured()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillOnce(Return(NetworkPolicy::APN_IMS));
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillOnce(ReturnRef(IpAddress::NONE));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    SetDataConnected(IMS_TRUE);

    NotifyTimerExpired(AosConnector::TIMER_IPV6);
    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, Ipv6TimerExpired_NotifyActivated)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Activated());
    EXPECT_CALL(m_objMockIAosPcscf, IsConfigured()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillOnce(Return(NetworkPolicy::APN_IMS));
    m_objPcscfs.AddElement(AString("1.1.1.1"));
    EXPECT_CALL(m_objMockIAosPcscf, GetPcscfs()).WillOnce(ReturnRef(m_objPcscfs));
    EXPECT_CALL(m_objMockIAosConnection, GetLocalAddress(IpAddress::IPV4))
            .WillOnce(ReturnRef(IpAddress::LOOPBACK));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    SetDataConnected(IMS_TRUE);

    NotifyTimerExpired(AosConnector::TIMER_IPV6);
    EXPECT_EQ(GetState(), AosConnector::STATE_READY);
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_IPV6));
}

TEST_F(AosConnectorTest, StopDelayTimerExpired)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetState(AosConnector::STATE_READY);
    StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_FIVE_SEC);

    // notify Deactivated
    NotifyTimerExpired(AosConnector::TIMER_STOP_DELAY);
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, ReadyRecoveryTimerExpired)
{
    EXPECT_CALL(m_objMockIAosConnectorListener, Connector_Deactivated(AosConnector::REASON_NONE));

    m_pAosConnector->SetListener(&m_objMockIAosConnectorListener);
    SetState(AosConnector::STATE_READY);
    StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_FIVE_SEC);

    // notify Deactivated
    NotifyTimerExpired(AosConnector::TIMER_READY_RECOVERY);
    EXPECT_EQ(GetState(), AosConnector::STATE_IDLE);
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
}

TEST_F(AosConnectorTest, PcoWaitingTimerExpired_StateReady)
{
    StartTimer(AosConnector::TIMER_PCO_WAITING, WAITING_PCO_VALUE_TIMEOUT_MILLIS);
    SetFeature(AosConnector::PENDING_PCO_WAITING);
    SetState(AosConnector::STATE_READY);

    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
    EXPECT_TRUE(AosConnector::PENDING_PCO_WAITING & GetFeature());

    NotifyTimerExpired(AosConnector::TIMER_PCO_WAITING);

    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_PCO_WAITING));
    EXPECT_FALSE(AosConnector::PENDING_PCO_WAITING & GetFeature());
}

TEST_F(AosConnectorTest, InvalidTimer)
{
    IMS_SINT32 nInvalidTimer = -1;
    StartTimer(nInvalidTimer, TIMER_DURATION_FIVE_SEC);
    EXPECT_FALSE(IsTimerRunning(nInvalidTimer));
    StopTimer(nInvalidTimer);
}