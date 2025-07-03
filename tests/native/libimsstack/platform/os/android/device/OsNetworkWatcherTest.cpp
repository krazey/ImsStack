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
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ImsEventDef.h"
#include "ImsTypeDef.h"
#include "MockISystem.h"
#include "MockIThread.h"
#include "PlatformContext.h"
#include "SystemConstants.h"
#include "TestNetworkServicePolicy.h"
#include "TestPhoneInfoService.h"
#include "TestThreadService.h"
#include "device/OsNetworkWatcher.h"
#include "network/OsNetworkConstants.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;

namespace android
{

class OsNetworkWatcherTest : public ::testing::Test
{
public:
    MockINetworkWatcher m_objMockINetworkWatcher;
    MockINetworkWatcherListener m_objMockINetworkWatcherListener;
    MockISystem m_objMockSystem;
    MockIThread m_objMockThread;

    ISystem* m_piDefaultSystem;
    ISystemListener* m_piSystemListener;
    OsNetworkWatcher* m_pOsNetworkWatcher;

    TestNetworkServicePolicy m_objNetworkServicePolicy;
    TestPhoneInfoService m_objPhoneInfoService;
    TestThreadService m_objThreadService;

protected:
    virtual void SetUp() override
    {
        m_objThreadService.SetThread(&m_objMockThread);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK_POLICY, &m_objNetworkServicePolicy);

        EXPECT_CALL(m_objMockSystem, AddListener(_, _, _)).Times(1);
        m_piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&m_objMockSystem);

        m_pOsNetworkWatcher = new OsNetworkWatcher(IMS_SLOT_0);
        ASSERT_TRUE(m_pOsNetworkWatcher != nullptr);

        m_piSystemListener = static_cast<ISystemListener*>(m_pOsNetworkWatcher);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK_POLICY, IMS_NULL);

        if (m_pOsNetworkWatcher != IMS_NULL)
        {
            delete m_pOsNetworkWatcher;
            m_pOsNetworkWatcher = IMS_NULL;
        }
    }
};

TEST_F(OsNetworkWatcherTest, GetNetworkStatus)
{
    AString strProfile("mobile_ims");

    NetworkPolicy objPolicy(IMS_TRUE, strProfile.GetStr(), NetworkPolicy::APN_IMS);
    m_objNetworkServicePolicy.AddPolicy(strProfile.GetStr(), objPolicy, IMS_SLOT_0);

    IMS_UINT32 networkStatus = (IMS_UINT32)(NW_REPORT_RADIO_NOSRV) |
            (IMS_UINT32)(NW_REPORT_SRV_SRV) | (IMS_UINT32)(NW_REPORT_DOMAIN_NOSRV);
    EXPECT_EQ(networkStatus, m_pOsNetworkWatcher->GetNetworkStatus(strProfile));
}

TEST_F(OsNetworkWatcherTest, GetNetRadioTechType)
{
    AString strProfile("mobile_ims");
    NetworkPolicy objPolicy(IMS_TRUE, strProfile.GetStr(), NetworkPolicy::APN_IMS);
    m_objNetworkServicePolicy.AddPolicy(strProfile.GetStr(), objPolicy, IMS_SLOT_0);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_LTE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(strProfile), NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_EHRPD));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(strProfile), NW_REPORT_RADIO_EHRPD);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_NR));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(strProfile), NW_REPORT_RADIO_NR);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_UMTS));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(strProfile), NW_REPORT_RADIO_WCDMA);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_HSPA));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(strProfile), NW_REPORT_RADIO_HSPA);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_GSM));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(strProfile), NW_REPORT_RADIO_GSM);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_EDGE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(strProfile), NW_REPORT_RADIO_EDGE);

    strProfile = "wifi";
    NetworkPolicy objWifiPolicy(IMS_TRUE, strProfile.GetStr(), NetworkPolicy::APN_WIFI);
    m_objNetworkServicePolicy.AddPolicy(strProfile.GetStr(), objWifiPolicy, IMS_SLOT_0);

    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(strProfile, NetworkPolicy::APN_WIFI),
            NW_REPORT_RADIO_WLAN);
}

TEST_F(OsNetworkWatcherTest, GetNetRadioTechTypeDefault)
{
    EXPECT_CALL(m_objMockSystem, GetNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_UNKNOWN));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(), NW_REPORT_RADIO_NOSRV);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_LTE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(), NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_NR));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(), NW_REPORT_RADIO_NR);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_EHRPD));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(), NW_REPORT_RADIO_EHRPD);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_UMTS));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(), NW_REPORT_RADIO_WCDMA);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_HSPA));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(), NW_REPORT_RADIO_HSPA);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_GSM));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(), NW_REPORT_RADIO_GSM);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_)).Times(1).WillOnce(Return(RADIOTECH_TYPE_EDGE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(), NW_REPORT_RADIO_EDGE);

    EXPECT_CALL(m_objMockSystem, GetNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_INVALID));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetRadioTechType(), NW_REPORT_RADIO_EDGE);
}

TEST_F(OsNetworkWatcherTest, GetNetVoiceRadioTechType)
{
    EXPECT_CALL(m_objMockSystem, GetVoiceNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_UNKNOWN));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceRadioTechType(), NW_REPORT_RADIO_NOSRV);

    EXPECT_CALL(m_objMockSystem, GetVoiceNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_GPRS));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceRadioTechType(), NW_REPORT_RADIO_GSM);

    EXPECT_CALL(m_objMockSystem, GetVoiceNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_EDGE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceRadioTechType(), NW_REPORT_RADIO_EDGE);

    EXPECT_CALL(m_objMockSystem, GetVoiceNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_UMTS));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceRadioTechType(), NW_REPORT_RADIO_WCDMA);

    EXPECT_CALL(m_objMockSystem, GetVoiceNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_EHRPD));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceRadioTechType(), NW_REPORT_RADIO_EHRPD);

    EXPECT_CALL(m_objMockSystem, GetVoiceNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_HSDPA));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceRadioTechType(), NW_REPORT_RADIO_HSPA);

    EXPECT_CALL(m_objMockSystem, GetVoiceNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_INVALID));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceRadioTechType(), NW_REPORT_RADIO_NOSRV);

    EXPECT_CALL(m_objMockSystem, GetVoiceNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_LTE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceRadioTechType(), NW_REPORT_RADIO_LTE);

    EXPECT_CALL(m_objMockSystem, GetVoiceNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_NR));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceRadioTechType(), NW_REPORT_RADIO_NR);

    EXPECT_CALL(m_objMockSystem, GetVoiceNetworkType(_))
            .Times(1)
            .WillOnce(Return(RADIOTECH_TYPE_HSPA));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceRadioTechType(), NW_REPORT_RADIO_HSPA);
}

TEST_F(OsNetworkWatcherTest, GetNetServiceType)
{
    AString strProfile("mobile_ims");
    NetworkPolicy objPolicy(IMS_TRUE, strProfile.GetStr(), NetworkPolicy::APN_IMS);
    m_objNetworkServicePolicy.AddPolicy(strProfile.GetStr(), objPolicy, IMS_SLOT_0);

    EXPECT_CALL(m_objMockSystem, GetServiceState(_)).Times(1).WillOnce(Return(STATE_IN_SERVICE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetServiceType(strProfile), NW_REPORT_SRV_SRV);

    strProfile = "wifi";
    NetworkPolicy objWifiPolicy(IMS_TRUE, strProfile.GetStr(), NetworkPolicy::APN_WIFI);
    m_objNetworkServicePolicy.AddPolicy(strProfile.GetStr(), objWifiPolicy, IMS_SLOT_0);

    EXPECT_EQ(m_pOsNetworkWatcher->GetNetServiceType(strProfile, NetworkPolicy::APN_WIFI),
            NW_REPORT_SRV_SRV);
}

TEST_F(OsNetworkWatcherTest, GetNetServiceTypeDefault)
{
    EXPECT_CALL(m_objMockSystem, GetServiceState(_)).Times(1).WillOnce(Return(STATE_IN_SERVICE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetServiceType(), NW_REPORT_SRV_SRV);

    EXPECT_CALL(m_objMockSystem, GetServiceState(_))
            .Times(1)
            .WillOnce(Return(STATE_OUT_OF_SERVICE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetServiceType(), NW_REPORT_SRV_NOSRV);

    EXPECT_CALL(m_objMockSystem, GetServiceState(_))
            .Times(1)
            .WillOnce(Return(STATE_EMERGENCY_ONLY));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetServiceType(), NW_REPORT_SRV_LIMITED);

    EXPECT_CALL(m_objMockSystem, GetServiceState(_)).Times(1).WillOnce(Return(STATE_POWER_OFF));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetServiceType(), NW_REPORT_SRV_PWRSAVE);

    EXPECT_CALL(m_objMockSystem, GetServiceState(_)).Times(1).WillOnce(Return(STATE_INVALID));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetServiceType(), NW_REPORT_SRV_PWRSAVE);
}

TEST_F(OsNetworkWatcherTest, GetNetVoiceServiceType)
{
    EXPECT_CALL(m_objMockSystem, GetVoiceServiceState(_))
            .Times(1)
            .WillOnce(Return(STATE_IN_SERVICE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceServiceType(), NW_REPORT_SRV_SRV);

    EXPECT_CALL(m_objMockSystem, GetVoiceServiceState(_))
            .Times(1)
            .WillOnce(Return(STATE_OUT_OF_SERVICE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceServiceType(), NW_REPORT_SRV_NOSRV);

    EXPECT_CALL(m_objMockSystem, GetVoiceServiceState(_))
            .Times(1)
            .WillOnce(Return(STATE_EMERGENCY_ONLY));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceServiceType(), NW_REPORT_SRV_LIMITED);

    EXPECT_CALL(m_objMockSystem, GetVoiceServiceState(_))
            .Times(1)
            .WillOnce(Return(STATE_POWER_OFF));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceServiceType(), NW_REPORT_SRV_PWRSAVE);

    EXPECT_CALL(m_objMockSystem, GetVoiceServiceState(_)).Times(1).WillOnce(Return(STATE_MAX));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetVoiceServiceType(), NW_REPORT_SRV_PWRSAVE);
}

TEST_F(OsNetworkWatcherTest, GetCellularServiceState)
{
    EXPECT_CALL(m_objMockSystem, GetCellularServiceState(_))
            .Times(1)
            .WillOnce(Return(INetworkWatcher::STATE_IN_SERVICE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetCellularServiceState(), INetworkWatcher::STATE_IN_SERVICE);

    EXPECT_CALL(m_objMockSystem, GetCellularServiceState(_))
            .Times(1)
            .WillOnce(Return(INetworkWatcher::STATE_OUT_OF_SERVICE));
    EXPECT_EQ(
            m_pOsNetworkWatcher->GetCellularServiceState(), INetworkWatcher::STATE_OUT_OF_SERVICE);

    EXPECT_CALL(m_objMockSystem, GetCellularServiceState(_))
            .Times(1)
            .WillOnce(Return(INetworkWatcher::STATE_EMERGENCY_ONLY));
    EXPECT_EQ(
            m_pOsNetworkWatcher->GetCellularServiceState(), INetworkWatcher::STATE_EMERGENCY_ONLY);

    EXPECT_CALL(m_objMockSystem, GetCellularServiceState(_))
            .Times(1)
            .WillOnce(Return(INetworkWatcher::STATE_POWER_OFF));
    EXPECT_EQ(m_pOsNetworkWatcher->GetCellularServiceState(), INetworkWatcher::STATE_POWER_OFF);

    EXPECT_CALL(m_objMockSystem, GetCellularServiceState(_))
            .Times(1)
            .WillOnce(Return(INetworkWatcher::STATE_INVALID));
    EXPECT_EQ(m_pOsNetworkWatcher->GetCellularServiceState(), INetworkWatcher::STATE_INVALID);
}

TEST_F(OsNetworkWatcherTest, GetRoamingState)
{
    EXPECT_CALL(m_objMockSystem, GetRoamingState(_)).Times(1).WillOnce(Return(IMS_TRUE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetRoamingState(), IMS_TRUE);
}

TEST_F(OsNetworkWatcherTest, GetVoiceRoamingType)
{
    EXPECT_CALL(m_objMockSystem, GetVoiceRoamingType(_)).Times(1).WillOnce(Return(IMS_TRUE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetVoiceRoamingType(), IMS_TRUE);
}

TEST_F(OsNetworkWatcherTest, GetDataRoamingType)
{
    EXPECT_CALL(m_objMockSystem, GetDataRoamingType(_)).Times(1).WillOnce(Return(IMS_FALSE));
    EXPECT_EQ(m_pOsNetworkWatcher->GetDataRoamingType(), IMS_FALSE);
}

TEST_F(OsNetworkWatcherTest, IsImsEmergencyCallSupported)
{
    EXPECT_CALL(m_objMockSystem, IsImsEmergencyCallSupported(_))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));
    EXPECT_EQ(m_pOsNetworkWatcher->IsImsEmergencyCallSupported(), IMS_TRUE);
}

TEST_F(OsNetworkWatcherTest, IsImsVoiceCallSupported)
{
    EXPECT_CALL(m_objMockSystem, IsImsVoiceCallSupported(_)).Times(1).WillOnce(Return(IMS_TRUE));
    EXPECT_EQ(m_pOsNetworkWatcher->IsImsVoiceCallSupported(), IMS_TRUE);
}

TEST_F(OsNetworkWatcherTest, IsEmergencyOnly)
{
    EXPECT_CALL(m_objMockSystem, IsEmergencyOnly(_)).Times(1).WillOnce(Return(IMS_FALSE));
    EXPECT_EQ(m_pOsNetworkWatcher->IsEmergencyOnly(), IMS_FALSE);
}

TEST_F(OsNetworkWatcherTest, IsEmergencyAttachSupported)
{
    EXPECT_CALL(m_objMockSystem, IsEmergencyAttachSupported(_)).Times(1).WillOnce(Return(IMS_TRUE));
    EXPECT_EQ(m_pOsNetworkWatcher->IsEmergencyAttachSupported(), IMS_TRUE);
}

TEST_F(OsNetworkWatcherTest, GetNetworkRegistrationRejectCause)
{
    EXPECT_CALL(m_objMockSystem, GetNetworkRegistrationRejectCause(_)).Times(1).WillOnce(Return(0));
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetworkRegistrationRejectCause(), 0);
}

TEST_F(OsNetworkWatcherTest, NotifyEvent)
{
    m_objPhoneInfoService.SetNetworkWatcher(m_pOsNetworkWatcher);
    m_pOsNetworkWatcher->RegisterObserver(&m_objMockINetworkWatcherListener);

    EXPECT_CALL(m_objMockThread, PostMessageI(_))
            .Times(AnyNumber())
            .WillRepeatedly(Invoke(
                    [&](IN ImsMessage& objMsg)
                    {
                        m_objPhoneInfoService.DispatchServiceMessage(objMsg);
                        return IMS_TRUE;
                    }));

    EXPECT_CALL(m_objMockINetworkWatcherListener, NetworkWatcher_NotifyStatus(_)).Times(14);

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_AIRPLANE_MODE_CHANGED, 0, 0);
    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_AIRPLANE_MODE_CHANGED, 1, 0);

    m_piSystemListener->System_NotifyEvent(
            IMS_SYSTEM_DATACONNECTION_STATE_CHANGED, DATA_DISCONNECTED, 0);
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetDomainType(), NW_REPORT_DOMAIN_CS);

    m_piSystemListener->System_NotifyEvent(
            IMS_SYSTEM_DATACONNECTION_STATE_CHANGED, DATA_CONNECTED, 0);
    EXPECT_EQ(m_pOsNetworkWatcher->GetNetDomainType(), NW_REPORT_DOMAIN_CSPS);

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_SERVICE_STATE_CHANGED, STATE_IN_SERVICE, 0);

    m_piSystemListener->System_NotifyEvent(
            IMS_SYSTEM_SERVICE_STATE_CHANGED, STATE_OUT_OF_SERVICE, 0);

    m_piSystemListener->System_NotifyEvent(
            IMS_SYSTEM_SERVICE_STATE_CHANGED, STATE_EMERGENCY_ONLY, 0);

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_SERVICE_STATE_CHANGED, STATE_POWER_OFF, 0);

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_RADIOTECH_STATE_CHANGED, RADIO_TECH_EHRPD, 0);

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_RADIOTECH_STATE_CHANGED, RADIO_TECH_LTE, 0);

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_RADIOTECH_STATE_CHANGED, RADIO_TECH_WCDMA, 0);

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_RADIOTECH_STATE_CHANGED, RADIO_TECH_GSM, 0);

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_RADIOTECH_STATE_CHANGED, RADIO_TECH_NR, 0);

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_VOICE_RADIOTECH_STATE_CHANGED, 0, 0);

    m_piSystemListener->System_NotifyEvent(IMS_SYSTEM_MAX, 0, 0);

    m_pOsNetworkWatcher->RemoveObserver(&m_objMockINetworkWatcherListener);
}

}  // namespace android
