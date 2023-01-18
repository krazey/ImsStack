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
#include "ServiceEvent.h"
#include "network/AosNetTracker.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTrackerListener.h"
#include "../../../platform/interface/MockINetworkWatcher.h"
#include "../../../platform/interface/MockIWifiWatcher.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;
const IMS_UINT32 TIMER_DURATION_FIVE_SEC = 5;

class AosNetTrackerTest : public ::testing::Test
{
public:
    AosNetTracker* m_pAosNetTracker;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosNetTrackerListener m_objMockIAosNetTrackerListener;
    MockINetworkWatcher m_objMockINetworkWatcher;
    MockIWifiWatcher m_objMockIWifiWatcher;

    IMSVector<IMS_SINT32> objRats;

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

        objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN);
        objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN);
        objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
        objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN);
        objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);
        EXPECT_CALL(m_objMockIAosNConfiguration, GetSupportedRats())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(objRats));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetSupportedRoamingRats())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(objRats));

        EXPECT_CALL(m_objMockIAosNConfiguration, GetSmsOverImsSupportedRats())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(objRats));

        m_pAosNetTracker =
                new AosNetTracker(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
        ASSERT_TRUE(m_pAosNetTracker != nullptr);

        InitObject();
    }

    virtual void TearDown() override
    {
        if (m_pAosNetTracker)
        {
            delete m_pAosNetTracker;
        }
    }

    void Initialize() { m_pAosNetTracker->Init(); }

    void InitObject()
    {
        m_pAosNetTracker->m_piAosNConfig = &m_objMockIAosNConfiguration;
        m_pAosNetTracker->InitObject();
    }

    void SetNetworkWatcher(IN INetworkWatcher* piNw)
    {
        m_pAosNetTracker->m_piNetWatcherInfo = piNw;
    }

    void SetWifiWatcher(IN IWifiWatcher* piWw) { m_pAosNetTracker->m_piWifiWatcher = piWw; }

    void SetWifiConnected(IN IMS_BOOL bConnected)
    {
        m_pAosNetTracker->SetWifiConnected(bConnected);
    }

    void SetStatus(IN IMS_SINT32 nService, IN IMS_UINT32 nRadioTech, IN IMS_BOOL bIsIn)
    {
        m_pAosNetTracker->m_nNetServiceType = nService;
        m_pAosNetTracker->m_nNetRadioType = nRadioTech;
        m_pAosNetTracker->m_bIsNetAvailable = bIsIn;
    }

    void SetCnxPolicy(IN IMS_UINT32 nPolicy)
    {
        m_pAosNetTracker->m_nCnxPolicy = nPolicy;
        m_pAosNetTracker->m_nCnxPolicyInRoaming = nPolicy;
    }

    void SetDataConnected(IN IMS_BOOL bConnected)
    {
        m_pAosNetTracker->SetDataConnected(bConnected);
    }

    void SetChangingRat(IN IMS_UINT32 nRat) { m_pAosNetTracker->m_nChangingRat = nRat; }

    void SetEpdgEnabled(IN IMS_BOOL bEnabled) { m_pAosNetTracker->SetEpdgEnabled(bEnabled); }

    void SetFeature(IMS_UINT32 nFeature) { m_pAosNetTracker->m_nFeature = nFeature; }

    INetworkWatcher* GetNetworkWatcher() { return m_pAosNetTracker->m_piNetWatcherInfo; }

    IMS_UINT32 GetAccessPolicy() { return m_pAosNetTracker->GetAccessPolicy(); }

    IMS_UINT32 GetFeature() { return m_pAosNetTracker->m_nFeature; }

    IMS_UINT32 GetListenerSize() { return m_pAosNetTracker->m_objListeners.GetSize(); }

    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
    {
        m_pAosNetTracker->StartTimer(nType, nDuration);
    }

    void StopTimer(IN IMS_UINT32 nType) { m_pAosNetTracker->StopTimer(nType); }

    void NotifyTimerExpired(IN IMS_UINT32 nType)
    {
        switch (nType)
        {
            case AosNetTracker::TIMER_IN_GUARD:
                m_pAosNetTracker->Timer_TimerExpired(m_pAosNetTracker->m_piServiceInTimer);
                break;

            case AosNetTracker::TIMER_OUT_GUARD:
                m_pAosNetTracker->Timer_TimerExpired(m_pAosNetTracker->m_piServiceOutTimer);
                break;

            case AosNetTracker::TIMER_RAT_GUARD:
                m_pAosNetTracker->Timer_TimerExpired(m_pAosNetTracker->m_piRatTimer);
                break;

            case AosNetTracker::TIMER_VOICE_RAT_GUARD:
                m_pAosNetTracker->Timer_TimerExpired(m_pAosNetTracker->m_piVoiceRatTimer);
                break;

            default:
                m_pAosNetTracker->Timer_TimerExpired(IMS_NULL);
        }
    }

    void NotifyStatusChanged() { m_pAosNetTracker->Notify(); }

    void UpdateVoiceNetwork() { m_pAosNetTracker->UpdateVoiceNetwork(); }

    void NotifyConnectionStateChanged(IN IMS_UINT32 nState)
    {
        m_pAosNetTracker->AosConnection_StateChanged(nState);
    }

    void NotifyIpChanged() { m_pAosNetTracker->AosConnection_IpChanged(); }

    void NotifyIpcanChanged() { m_pAosNetTracker->AosConnection_IpcanCatChanged(); }

    void NotifyPcscfChanged() { m_pAosNetTracker->AosConnection_PcscfChanged(); }

    void NotifyConnectionFailed() { m_pAosNetTracker->AosConnection_ConnectionFailed(); }

    IMS_CHAR* FeaturesToString() { return m_pAosNetTracker->FeaturesToString().GetStr(); }

    const IMS_CHAR* RadioTypeToString(IMS_UINT32 nState)
    {
        return m_pAosNetTracker->RadioTypeToString(nState);
    }

    const IMS_CHAR* ServiceTypeToString(IMS_UINT32 nState)
    {
        return m_pAosNetTracker->ServiceTypeToString(nState);
    }
};

TEST_F(AosNetTrackerTest, Init_ImsType)
{
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_IMS));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsSmsOverImsSupported()).WillOnce(Return(IMS_TRUE));
    EXPECT_CALL(m_objMockIAosNConfiguration, IsImsOverNrEnabled()).WillOnce(Return(IMS_TRUE));

    Initialize();
    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    EXPECT_EQ(GetAccessPolicy(),
            nCnxPolicy | NW_REPORT_RADIO_GSM | NW_REPORT_RADIO_EDGE | NW_REPORT_RADIO_WCDMA |
                    NW_REPORT_RADIO_HSPA | NW_REPORT_RADIO_LTE | NW_REPORT_RADIO_NR |
                    NW_REPORT_RADIO_WLAN);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_IN_GUARD | AosNetTracker::FEATURE_OUT_GUARD);
    EXPECT_NE(GetNetworkWatcher(), nullptr);
}

TEST_F(AosNetTrackerTest, Init_EmergencyType)
{
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_EMERGENCY));

    Initialize();
    EXPECT_EQ(GetAccessPolicy(), 0xFFFFFFFF);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_IN_GUARD | AosNetTracker::FEATURE_OUT_GUARD);
    EXPECT_NE(GetNetworkWatcher(), nullptr);
}

TEST_F(AosNetTrackerTest, Init_WifiType)
{
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_WIFI));

    Initialize();
    EXPECT_EQ(GetAccessPolicy(), 0x01000000);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_IN_GUARD | AosNetTracker::FEATURE_OUT_GUARD);
    EXPECT_NE(GetNetworkWatcher(), nullptr);
}

TEST_F(AosNetTrackerTest, IsServiceIn)
{
    // both Mobile and WLAN are not ServiceIn
    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    SetWifiConnected(IMS_FALSE);
    EXPECT_FALSE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_MOBILE));
    EXPECT_FALSE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_WLAN));
    EXPECT_FALSE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_DEFAULT));

    // Mobile is ServiceIn but WLAN is not ServiceIn
    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_LTE, IMS_TRUE);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_MOBILE));
    EXPECT_TRUE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_DEFAULT));
    EXPECT_FALSE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_WLAN));

    SetEpdgEnabled(IMS_TRUE);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_DEFAULT));
    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE | NW_REPORT_RADIO_WLAN);

    // WLAN isServiceIn
    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_WLAN, IMS_FALSE);
    SetWifiConnected(IMS_TRUE);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_WLAN));
    EXPECT_TRUE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_DEFAULT));
}

TEST_F(AosNetTrackerTest, IsDataIn)
{
    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);

    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    EXPECT_FALSE(m_pAosNetTracker->IsDataIn());

    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    EXPECT_TRUE(m_pAosNetTracker->IsDataIn());
}

TEST_F(AosNetTrackerTest, IsNetworkIn)
{
    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);

    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    EXPECT_FALSE(m_pAosNetTracker->IsNetworkIn());

    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    EXPECT_TRUE(m_pAosNetTracker->IsNetworkIn());
}

TEST_F(AosNetTrackerTest, IsEmergencyLteAttach)
{
    EXPECT_CALL(m_objMockINetworkWatcher, IsLteEmergencyOnly())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    EXPECT_TRUE(m_pAosNetTracker->IsEmergencyLteAttach());
    EXPECT_FALSE(m_pAosNetTracker->IsEmergencyLteAttach());
}

TEST_F(AosNetTrackerTest, IsSuspended)
{
    EXPECT_FALSE(m_pAosNetTracker->IsSuspended());
    SetDataConnected(IMS_TRUE);
    EXPECT_TRUE(m_pAosNetTracker->IsSuspended());
    SetEpdgEnabled(IMS_TRUE);
    EXPECT_FALSE(m_pAosNetTracker->IsSuspended());
}

TEST_F(AosNetTrackerTest, IsSessionContinuitySupported)
{
    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE | NW_REPORT_RADIO_NR);
    EXPECT_FALSE(m_pAosNetTracker->IsSessionContinuitySupported());

    nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE | NW_REPORT_RADIO_WLAN);
    EXPECT_TRUE(m_pAosNetTracker->IsSessionContinuitySupported());

    nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_NR | NW_REPORT_RADIO_WLAN);
    EXPECT_TRUE(m_pAosNetTracker->IsSessionContinuitySupported());
}

TEST_F(AosNetTrackerTest, IsServiceTimerRunning)
{
    StartTimer(AosNetTracker::TIMER_IN_GUARD, TIMER_DURATION_FIVE_SEC);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceTimerRunning());

    // If the timer is running, stop previous timer and start again.
    StartTimer(AosNetTracker::TIMER_IN_GUARD, TIMER_DURATION_FIVE_SEC);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceTimerRunning());

    StopTimer(AosNetTracker::TIMER_IN_GUARD);
    EXPECT_FALSE(m_pAosNetTracker->IsServiceTimerRunning());

    StartTimer(AosNetTracker::TIMER_OUT_GUARD, TIMER_DURATION_FIVE_SEC);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceTimerRunning());

    StopTimer(AosNetTracker::TIMER_OUT_GUARD);
    EXPECT_FALSE(m_pAosNetTracker->IsServiceTimerRunning());
}

TEST_F(AosNetTrackerTest, IsImsVoiceCallSupported)
{
    EXPECT_CALL(m_objMockINetworkWatcher, IsImsVoiceCallSupported())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    EXPECT_TRUE(m_pAosNetTracker->IsImsVoiceCallSupported());
    EXPECT_FALSE(m_pAosNetTracker->IsImsVoiceCallSupported());
}

TEST_F(AosNetTrackerTest, SetListener)
{
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);

    // do not handle invalid listener
    m_pAosNetTracker->SetListener(nullptr);

    // set listenr
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));

    // do not set same listenr agaiin
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
}

TEST_F(AosNetTrackerTest, RemoveListener)
{
    // Invoke listener only when setting it
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);

    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    EXPECT_EQ(GetListenerSize(), 1);

    // do not handle invalid listener
    m_pAosNetTracker->RemoveListener(nullptr);
    EXPECT_EQ(GetListenerSize(), 1);

    m_pAosNetTracker->RemoveListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    EXPECT_EQ(GetListenerSize(), 0);

    NotifyStatusChanged();
}

TEST_F(AosNetTrackerTest, GetMobileChangingNetworkType)
{
    EXPECT_EQ(m_pAosNetTracker->GetMobileChangingNetworkType(), NW_REPORT_RADIO_NOSRV);
    SetChangingRat(NW_REPORT_RADIO_LTE);
    EXPECT_EQ(m_pAosNetTracker->GetMobileChangingNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosNetTrackerTest, GetMobileNetworkType)
{
    EXPECT_EQ(m_pAosNetTracker->GetMobileNetworkType(), NW_REPORT_RADIO_NOSRV);
    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    EXPECT_EQ(m_pAosNetTracker->GetMobileNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosNetTrackerTest, GetMobileVoiceServiceState)
{
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceServiceType())
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    EXPECT_EQ(m_pAosNetTracker->GetMobileVoiceServiceState(), NW_REPORT_SRV_SRV);
}

TEST_F(AosNetTrackerTest, GetMobileVoiceNetworkType)
{
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    EXPECT_EQ(m_pAosNetTracker->GetMobileVoiceNetworkType(), NW_REPORT_RADIO_NOSRV);
    UpdateVoiceNetwork();
    EXPECT_EQ(m_pAosNetTracker->GetMobileVoiceNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosNetTrackerTest, GetNetworkType)
{
    EXPECT_EQ(m_pAosNetTracker->GetNetworkType(), NW_REPORT_RADIO_NOSRV);
    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    EXPECT_EQ(m_pAosNetTracker->GetNetworkType(), NW_REPORT_RADIO_LTE);
    SetEpdgEnabled(IMS_TRUE);
    EXPECT_EQ(m_pAosNetTracker->GetNetworkType(), NW_REPORT_RADIO_WLAN);
}

TEST_F(AosNetTrackerTest, SetRatGuardTime)
{
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);

    m_pAosNetTracker->SetRatGuardTime(TIMER_DURATION_FIVE_SEC);
    IMS_UINT32 nExpect = AosNetTracker::FEATURE_RAT_GUARD | AosNetTracker::FEATURE_VOICE_RAT_GUARD;
    EXPECT_EQ(GetFeature(), nExpect);

    m_pAosNetTracker->SetRatGuardTime(0);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);
}

TEST_F(AosNetTrackerTest, SetSrvOutGuardTime)
{
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);

    m_pAosNetTracker->SetSrvOutGuardTime(TIMER_DURATION_FIVE_SEC);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_OUT_GUARD);

    m_pAosNetTracker->SetSrvOutGuardTime(0);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);
}

TEST_F(AosNetTrackerTest, SetSrvInGuardTime)
{
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);

    m_pAosNetTracker->SetSrvInGuardTime(TIMER_DURATION_FIVE_SEC);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_IN_GUARD);

    m_pAosNetTracker->SetSrvInGuardTime(0);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);
}

TEST_F(AosNetTrackerTest, NetworkWatcher_NotifyStatus_FromOtherNetworkWatcher)
{
    MockINetworkWatcher objOtherMockINetworkWatcher;

    // Do not handle because it is invoked from the other NetworkWatcher
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _)).Times(0);
    EXPECT_CALL(objOtherMockINetworkWatcher, GetNetServiceType(_, _)).Times(0);

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_WCDMA, IMS_TRUE);
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));

    m_pAosNetTracker->NetworkWatcher_NotifyStatus(
            static_cast<INetworkWatcher*>(&objOtherMockINetworkWatcher));
}

TEST_F(AosNetTrackerTest, NetworkWatcher_NotifyStatus_WithSrvInGuardTime)
{
    // 1. Set Listener, 2. Notify RAT change from WCDM to LTE, 3. Notify Voice RAT from NOSRV to LTE
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(3);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE));

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_WCDMA, IMS_TRUE);
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    m_pAosNetTracker->SetSrvInGuardTime(TIMER_DURATION_FIVE_SEC);

    m_pAosNetTracker->NetworkWatcher_NotifyStatus(
            static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
}

TEST_F(AosNetTrackerTest, NetworkWatcher_NotifyStatus_WithoutGuardTime)
{
    // 1. Set Listener, 2. Notify RAT change from WCDM to LTE
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE));

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_WCDMA, IMS_TRUE);
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));

    m_pAosNetTracker->NetworkWatcher_NotifyStatus(
            static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
}

TEST_F(AosNetTrackerTest, NetworkWatcher_NotifyStatus_NoChanges)
{
    // Set Listener but do not notify because there are no changes
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_LTE, IMS_TRUE);
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));

    m_pAosNetTracker->NetworkWatcher_NotifyStatus(
            static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
}

TEST_F(AosNetTrackerTest, ProcessNetworkChanged_NoChanges)
{
    // Set Listener but do not notify because there are no changes
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_NOSRV));

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_LTE, IMS_TRUE);
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));

    // with FEATURE_IN_GUARD
    m_pAosNetTracker->SetSrvInGuardTime(TIMER_DURATION_FIVE_SEC);
    m_pAosNetTracker->NetworkWatcher_NotifyStatus(
            static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetSrvInGuardTime(0);

    // with FEATURE_RAT_GUARD
    m_pAosNetTracker->SetRatGuardTime(TIMER_DURATION_FIVE_SEC);
    m_pAosNetTracker->NetworkWatcher_NotifyStatus(
            static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetRatGuardTime(0);
}

TEST_F(AosNetTrackerTest, ProcessNetworkChanged_OutSrvToInSrv_WithSrvInGuardTime)
{
    // Set Listener and do not notify any more
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    m_pAosNetTracker->SetSrvInGuardTime(TIMER_DURATION_FIVE_SEC);

    // Do not notify because service status is changed with FEATURE_IN_GUARD
    m_pAosNetTracker->NetworkWatcher_NotifyStatus(
            static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    EXPECT_TRUE(m_pAosNetTracker->IsServiceTimerRunning());
}

TEST_F(AosNetTrackerTest, ProcessNetworkChanged_OutSrvToInSrv_WithRatGuardTime)
{
    // 1. Set Listener, 2. Notify service status change
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    m_pAosNetTracker->SetRatGuardTime(TIMER_DURATION_FIVE_SEC);

    // Notify status change because service status is changed without FEATURE_IN_GUARD
    m_pAosNetTracker->NetworkWatcher_NotifyStatus(
            static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
}

TEST_F(AosNetTrackerTest, ProcessNetworkChanged_InSrvToOutSrv_WithSrvOutGuardTime)
{
    // Set Listener and do not notify any more
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_NOSRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_LTE, IMS_TRUE);
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    m_pAosNetTracker->SetSrvOutGuardTime(TIMER_DURATION_FIVE_SEC);

    // Do not notify because service status is changed with FEATURE_IN_GUARD
    m_pAosNetTracker->NetworkWatcher_NotifyStatus(
            static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    EXPECT_TRUE(m_pAosNetTracker->IsServiceTimerRunning());
}

TEST_F(AosNetTrackerTest, ProcessNetworkChanged_InSrvToOutSrv_WithRatGuardTime)
{
    // 1. Set Listener, 2. Notify service status change
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_NOSRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_LTE, IMS_TRUE);
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    m_pAosNetTracker->SetRatGuardTime(TIMER_DURATION_FIVE_SEC);

    // Notify status change because service status is changed without FEATURE_OUT_GUARD
    m_pAosNetTracker->NetworkWatcher_NotifyStatus(
            static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
}

TEST_F(AosNetTrackerTest, WifiWatcher_NotifyStateChanged_FromOtherWifiWatcher)
{
    MockIWifiWatcher objOtherMockIWifiWatcher;

    // Do not handle because it is invoked from the other WifiWatcher
    EXPECT_CALL(m_objMockIWifiWatcher, GetState()).Times(0);
    EXPECT_CALL(objOtherMockIWifiWatcher, GetState()).Times(0);

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));

    m_pAosNetTracker->WifiWatcher_NotifyStateChanged(
            static_cast<IWifiWatcher*>(&objOtherMockIWifiWatcher));
}

TEST_F(AosNetTrackerTest, WifiWatcher_NotifyStateChanged)
{
    // 1. Set Listener, 2. Notify STATE_CONNECTED, 3. Notify STATE_DISCONNECTED
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(3);
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(2)
            .WillOnce(Return(IWifiWatcher::STATE_CONNECTED))
            .WillOnce(Return(IWifiWatcher::STATE_DISCONNECTED));

    SetWifiWatcher(static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));

    m_pAosNetTracker->WifiWatcher_NotifyStateChanged(
            static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
    m_pAosNetTracker->WifiWatcher_NotifyStateChanged(
            static_cast<IWifiWatcher*>(&m_objMockIWifiWatcher));
}

TEST_F(AosNetTrackerTest, Event_NotifyEvent_WithRatGuardTime)
{
    // 1. Set Listener, 2. ProcessNetworkChanged
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE));

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    m_pAosNetTracker->SetRatGuardTime(TIMER_DURATION_FIVE_SEC);

    m_pAosNetTracker->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_ON);
}

TEST_F(AosNetTrackerTest, Event_NotifyEvent_WithoutGuardTime)
{
    // 1. Set Listener, 2. Notify change of roaming state
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE));

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV;
    SetCnxPolicy(nCnxPolicy | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_WCDMA, IMS_TRUE);
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&m_objMockINetworkWatcher));
    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));

    m_pAosNetTracker->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_ON);
}

TEST_F(AosNetTrackerTest, NotifyInGuardTimerExpired)
{
    // 1. Set Listener, 2. Notify timer is expired
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);

    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    StartTimer(AosNetTracker::TIMER_IN_GUARD, TIMER_DURATION_FIVE_SEC);

    NotifyTimerExpired(AosNetTracker::TIMER_IN_GUARD);
}

TEST_F(AosNetTrackerTest, NotifyOutGuardTimerExpired)
{
    // 1. Set Listener, 2. Notify timer is expired
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);

    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    StartTimer(AosNetTracker::TIMER_OUT_GUARD, TIMER_DURATION_FIVE_SEC);

    NotifyTimerExpired(AosNetTracker::TIMER_OUT_GUARD);
}

TEST_F(AosNetTrackerTest, NotifyRatGuardTimerExpired)
{
    // 1. Set Listener, 2. Notify timer is expired
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);

    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    StartTimer(AosNetTracker::TIMER_RAT_GUARD, TIMER_DURATION_FIVE_SEC);

    NotifyTimerExpired(AosNetTracker::TIMER_RAT_GUARD);
}

TEST_F(AosNetTrackerTest, NotifyVoiceRatGuardTimerExpired)
{
    // 1. Set Listener, 2. Notify timer is expired
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);

    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));
    StartTimer(AosNetTracker::TIMER_VOICE_RAT_GUARD, TIMER_DURATION_FIVE_SEC);

    NotifyTimerExpired(AosNetTracker::TIMER_VOICE_RAT_GUARD);
}

TEST_F(AosNetTrackerTest, HandleInvalidTimer)
{
    IMS_UINT32 nInvalidTimer = -1;
    StartTimer(nInvalidTimer, TIMER_DURATION_FIVE_SEC);
    StopTimer(nInvalidTimer);
    NotifyTimerExpired(nInvalidTimer);
}

TEST_F(AosNetTrackerTest, AosConnection_StateChanged)
{
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillOnce(Return(IMS_FALSE));

    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));

    NotifyConnectionStateChanged(IAosConnection::STATE_ACTIVE);
}

TEST_F(AosNetTrackerTest, AosConnection_IpcanCatChanged)
{
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&m_objMockIAosNetTrackerListener));

    // notify that EPDG is enabled
    NotifyIpcanChanged();

    // notify again that EPDG is enabled (this would be not handled)
    NotifyIpcanChanged();
}

TEST_F(AosNetTrackerTest, FeaturesToString)
{
    SetFeature(AosNetTracker::FEATURE_IN_GUARD | AosNetTracker::FEATURE_OUT_GUARD |
            AosNetTracker::FEATURE_RAT_GUARD | AosNetTracker::FEATURE_VOICE_RAT_GUARD);
    EXPECT_STREQ(FeaturesToString(),
            "| FEATURE_IN_GUARD | FEATURE_OUT_GUARD | FEATURE_RAT_GUARD | FEATURE_VOICE_RAT_GUARD "
            "| ");
}

TEST_F(AosNetTrackerTest, RadioTypeToString)
{
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_NOSRV), "NW_REPORT_RADIO_NOSRV");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_AMPS), "NW_REPORT_RADIO_AMPS");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_CDMA), "NW_REPORT_RADIO_CDMA");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_GSM), "NW_REPORT_RADIO_GSM");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_HDR), "NW_REPORT_RADIO_HDR");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_WCDMA), "NW_REPORT_RADIO_WCDMA");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_GPS), "NW_REPORT_RADIO_GPS");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_EDGE), "NW_REPORT_RADIO_EDGE");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_WLAN), "NW_REPORT_RADIO_WLAN");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_EVDODO), "NW_REPORT_RADIO_EVDODO");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_EHRPD), "NW_REPORT_RADIO_EHRPD");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_LTE), "NW_REPORT_RADIO_LTE");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_HSPA), "NW_REPORT_RADIO_HSPA");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_NR), "NW_REPORT_RADIO_NR");
    EXPECT_STREQ(RadioTypeToString(NW_REPORT_RADIO_INVALID), "__INVALID__");
}

TEST_F(AosNetTrackerTest, ServiceTypeToString)
{
    EXPECT_STREQ(ServiceTypeToString(NW_REPORT_SRV_NOSRV), "NW_REPORT_SRV_NOSRV");
    EXPECT_STREQ(ServiceTypeToString(NW_REPORT_SRV_LIMITED), "NW_REPORT_SRV_LIMITED");
    EXPECT_STREQ(ServiceTypeToString(NW_REPORT_SRV_SRV), "NW_REPORT_SRV_SRV");
    EXPECT_STREQ(ServiceTypeToString(NW_REPORT_SRV_LIMITEDREGION), "NW_REPORT_SRV_LIMITEDREGION");
    EXPECT_STREQ(ServiceTypeToString(NW_REPORT_SRV_PWRSAVE), "NW_REPORT_SRV_PWRSAVE");
}

TEST_F(AosNetTrackerTest, AosConnection_IpChanged)
{
    // Currently, there is no logic that requires tests.
    NotifyIpChanged();
}

TEST_F(AosNetTrackerTest, AosConnection_PcscfChanged)
{
    // Currently, there is no logic that requires tests.
    NotifyPcscfChanged();
}

TEST_F(AosNetTrackerTest, AosConnection_ConnectionFailed)
{
    // Currently, there is no logic that requires tests.
    NotifyConnectionFailed();
}