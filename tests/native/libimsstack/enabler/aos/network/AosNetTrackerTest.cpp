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

const IMS_UINT32 TIMER_DURATION_FIVE_SEC = 5;

class AosNetTrackerTest : public ::testing::Test
{
public:
    AosNetTracker* pAosNetTracker;
    MockIAosAppContext objMockIAosAppContext;
    MockIAosConnection objMockIAosConnection;
    MockIAosNConfiguration objMockIAosNConfiguration;
    MockINetworkWatcher objMockINetworkWatcher;
    MockIWifiWatcher objMockIWifiWatcher;

    IMSVector<IMS_SINT32> objRats;

protected:
    virtual void SetUp() override
    {
        EXPECT_CALL(objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(objMockIAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&objMockIAosConnection));

        EXPECT_CALL(objMockIAosConnection, GetConnectionType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(NetworkPolicy::APN_IMS));

        EXPECT_CALL(objMockIAosConnection, RemoveListener(_)).Times(AnyNumber());

        objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
        EXPECT_CALL(objMockIAosNConfiguration, GetSupportedRats())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(objRats));

        EXPECT_CALL(objMockIAosNConfiguration, GetSupportedRoamingRats())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(objRats));

        EXPECT_CALL(objMockIAosNConfiguration, IsSmsOverImsSupported())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        EXPECT_CALL(objMockIAosNConfiguration, IsImsOverNrEnabled())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_FALSE));

        pAosNetTracker = new AosNetTracker(static_cast<IAosAppContext*>(&objMockIAosAppContext));
        ASSERT_TRUE(pAosNetTracker != nullptr);

        SetNConfig();
    }

    virtual void TearDown() override
    {
        if (pAosNetTracker)
        {
            delete pAosNetTracker;
        }
    }

    void Initialize() { pAosNetTracker->Init(); }

    void SetNConfig() { pAosNetTracker->m_piAosNConfig = &objMockIAosNConfiguration; }

    void SetNetworkWatcher(IN INetworkWatcher* piNw) { pAosNetTracker->m_piNetWatcherInfo = piNw; }

    void SetWifiWatcher(IN IWifiWatcher* piWw) { pAosNetTracker->m_piWifiWatcher = piWw; }

    void SetWifiConnected(IN IMS_BOOL bConnected) { pAosNetTracker->SetWifiConnected(bConnected); }

    void SetStatus(IN IMS_SINT32 nService, IN IMS_UINT32 nRadioTech, IN IMS_BOOL bIsIn)
    {
        pAosNetTracker->m_nNetServiceType = nService;
        pAosNetTracker->m_nNetRadioType = nRadioTech;
        pAosNetTracker->m_bIsNetAvailable = bIsIn;
    }

    void SetCnxPolicy(IN IMS_UINT32 nPolicy) { pAosNetTracker->m_nCnxPolicy |= nPolicy; }

    void SetDataConnected(IN IMS_BOOL bConnected) { pAosNetTracker->SetDataConnected(bConnected); }

    void SetChangingRat(IN IMS_UINT32 nRat) { pAosNetTracker->m_nChangingRat = nRat; }

    void SetEpdgEnabled(IN IMS_BOOL bEnabled) { pAosNetTracker->SetEpdgEnabled(bEnabled); }

    INetworkWatcher* GetNetworkWatcher() { return pAosNetTracker->m_piNetWatcherInfo; }

    IMS_UINT32 GetAccessPolicy() { return pAosNetTracker->GetAccessPolicy(); }

    IMS_UINT32 GetFeature() { return pAosNetTracker->m_nFeature; }

    IMS_UINT32 GetListenerSize() { return pAosNetTracker->m_objListeners.GetSize(); }

    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
    {
        pAosNetTracker->StartTimer(nType, nDuration);
    }

    void StopTimer(IN IMS_UINT32 nType) { pAosNetTracker->StopTimer(nType); }

    void NotifyTimerExpired(IN IMS_UINT32 nType)
    {
        switch (nType)
        {
            case AosNetTracker::TIMER_IN_GUARD:
                pAosNetTracker->Timer_TimerExpired(pAosNetTracker->m_piServiceInTimer);
                break;

            case AosNetTracker::TIMER_OUT_GUARD:
                pAosNetTracker->Timer_TimerExpired(pAosNetTracker->m_piServiceOutTimer);
                break;

            case AosNetTracker::TIMER_RAT_GUARD:
                pAosNetTracker->Timer_TimerExpired(pAosNetTracker->m_piRatTimer);
                break;

            case AosNetTracker::TIMER_VOICE_RAT_GUARD:
                pAosNetTracker->Timer_TimerExpired(pAosNetTracker->m_piVoiceRatTimer);
                break;
        }
    }

    void NotifyStatusChanged() { pAosNetTracker->Notify(); }

    void UpdateVoiceNetwork() { pAosNetTracker->UpdateVoiceNetwork(); }

    void NotifyConnectionStateChanged(IN IMS_UINT32 nState)
    {
        pAosNetTracker->AosConnection_StateChanged(nState);
    }

    void NotifyIpChanged() { pAosNetTracker->AosConnection_IpChanged(); }

    void NotifyIpcanChanged() { pAosNetTracker->AosConnection_IpcanCatChanged(); }

    void NotifyPcscfChanged() { pAosNetTracker->AosConnection_PcscfChanged(); }

    void NotifyConnectionFailed() { pAosNetTracker->AosConnection_ConnectionFailed(); }
};

TEST_F(AosNetTrackerTest, Init)
{
    Initialize();
    EXPECT_NE(GetAccessPolicy(), 0);
    EXPECT_NE(GetFeature(), AosNetTracker::FEATURE_NONE);
    EXPECT_NE(GetNetworkWatcher(), nullptr);
}

TEST_F(AosNetTrackerTest, IsServiceIn)
{
    // both Mobile and WLAN are not ServiceIn
    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    SetWifiConnected(IMS_FALSE);
    EXPECT_FALSE(pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_MOBILE));
    EXPECT_FALSE(pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_WLAN));
    EXPECT_FALSE(pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_DEFAULT));

    // Mobile is ServiceIn but WLAN is not ServiceIn
    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_TRUE);
    EXPECT_TRUE(pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_MOBILE));
    EXPECT_TRUE(pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_DEFAULT));
    EXPECT_FALSE(pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_WLAN));

    // WLAN isServiceIn
    SetWifiConnected(IMS_TRUE);
    EXPECT_TRUE(pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_WLAN));
}

TEST_F(AosNetTrackerTest, IsDataIn)
{
    SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);

    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    EXPECT_FALSE(pAosNetTracker->IsDataIn());

    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    EXPECT_TRUE(pAosNetTracker->IsDataIn());
}

TEST_F(AosNetTrackerTest, IsNetworkIn)
{
    SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);

    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    EXPECT_FALSE(pAosNetTracker->IsNetworkIn());

    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    EXPECT_TRUE(pAosNetTracker->IsNetworkIn());
}

TEST_F(AosNetTrackerTest, IsEmergencyLteAttach)
{
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&objMockINetworkWatcher));
    EXPECT_CALL(objMockINetworkWatcher, IsLteEmergencyOnly())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_TRUE(pAosNetTracker->IsEmergencyLteAttach());
    EXPECT_FALSE(pAosNetTracker->IsEmergencyLteAttach());
}

TEST_F(AosNetTrackerTest, IsSuspended)
{
    EXPECT_FALSE(pAosNetTracker->IsSuspended());

    SetDataConnected(IMS_TRUE);
    EXPECT_TRUE(pAosNetTracker->IsSuspended());
}

TEST_F(AosNetTrackerTest, IsSessionContinuitySupported)
{
    SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    EXPECT_FALSE(pAosNetTracker->IsSessionContinuitySupported());

    SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE | NW_REPORT_RADIO_WLAN);
    EXPECT_TRUE(pAosNetTracker->IsSessionContinuitySupported());
}

TEST_F(AosNetTrackerTest, IsServiceTimerRunning)
{
    StartTimer(AosNetTracker::TIMER_IN_GUARD, TIMER_DURATION_FIVE_SEC);
    EXPECT_TRUE(pAosNetTracker->IsServiceTimerRunning());

    StopTimer(AosNetTracker::TIMER_IN_GUARD);
    EXPECT_FALSE(pAosNetTracker->IsServiceTimerRunning());

    StartTimer(AosNetTracker::TIMER_OUT_GUARD, TIMER_DURATION_FIVE_SEC);
    EXPECT_TRUE(pAosNetTracker->IsServiceTimerRunning());

    StopTimer(AosNetTracker::TIMER_OUT_GUARD);
    EXPECT_FALSE(pAosNetTracker->IsServiceTimerRunning());
}

TEST_F(AosNetTrackerTest, SetListener)
{
    MockIAosNetTrackerListener objMockIAosNetTrackerListener;
    EXPECT_CALL(objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);

    pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));
    NotifyStatusChanged();
}

TEST_F(AosNetTrackerTest, RemoveListener)
{
    MockIAosNetTrackerListener objMockIAosNetTrackerListener;
    EXPECT_CALL(objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);

    pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));
    EXPECT_EQ(GetListenerSize(), 1);

    pAosNetTracker->RemoveListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));
    EXPECT_EQ(GetListenerSize(), 0);
    NotifyStatusChanged();
}

TEST_F(AosNetTrackerTest, GetMobileChangingNetworkType)
{
    EXPECT_EQ(pAosNetTracker->GetMobileChangingNetworkType(), NW_REPORT_RADIO_NOSRV);
    SetChangingRat(NW_REPORT_RADIO_LTE);
    EXPECT_EQ(pAosNetTracker->GetMobileChangingNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosNetTrackerTest, GetMobileNetworkType)
{
    EXPECT_EQ(pAosNetTracker->GetMobileNetworkType(), NW_REPORT_RADIO_NOSRV);
    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    EXPECT_EQ(pAosNetTracker->GetMobileNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosNetTrackerTest, GetMobileVoiceServiceState)
{
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&objMockINetworkWatcher));
    EXPECT_CALL(objMockINetworkWatcher, GetNetVoiceServiceType())
            .Times(1)
            .WillOnce(Return(NW_REPORT_SRV_SRV));

    EXPECT_EQ(pAosNetTracker->GetMobileVoiceServiceState(), NW_REPORT_SRV_SRV);
}

TEST_F(AosNetTrackerTest, GetMobileVoiceNetworkType)
{
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&objMockINetworkWatcher));
    EXPECT_CALL(objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .Times(1)
            .WillOnce(Return(NW_REPORT_RADIO_LTE));

    EXPECT_EQ(pAosNetTracker->GetMobileVoiceNetworkType(), NW_REPORT_RADIO_NOSRV);
    UpdateVoiceNetwork();
    EXPECT_EQ(pAosNetTracker->GetMobileVoiceNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosNetTrackerTest, GetNetworkType)
{
    EXPECT_EQ(pAosNetTracker->GetNetworkType(), NW_REPORT_RADIO_NOSRV);
    SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    EXPECT_EQ(pAosNetTracker->GetNetworkType(), NW_REPORT_RADIO_LTE);
    SetEpdgEnabled(IMS_TRUE);
    EXPECT_EQ(pAosNetTracker->GetNetworkType(), NW_REPORT_RADIO_WLAN);
}

TEST_F(AosNetTrackerTest, SetRatGuardTime)
{
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);

    pAosNetTracker->SetRatGuardTime(TIMER_DURATION_FIVE_SEC);
    IMS_UINT32 nExpect = AosNetTracker::FEATURE_RAT_GUARD | AosNetTracker::FEATURE_VOICE_RAT_GUARD;
    EXPECT_EQ(GetFeature(), nExpect);

    pAosNetTracker->SetRatGuardTime(0);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);
}

TEST_F(AosNetTrackerTest, SetSrvOutGuardTime)
{
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);

    pAosNetTracker->SetSrvOutGuardTime(TIMER_DURATION_FIVE_SEC);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_OUT_GUARD);

    pAosNetTracker->SetSrvOutGuardTime(0);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);
}

TEST_F(AosNetTrackerTest, SetSrvInGuardTime)
{
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);

    pAosNetTracker->SetSrvInGuardTime(TIMER_DURATION_FIVE_SEC);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_IN_GUARD);

    pAosNetTracker->SetSrvInGuardTime(0);
    EXPECT_EQ(GetFeature(), AosNetTracker::FEATURE_NONE);
}

TEST_F(AosNetTrackerTest, NetworkWatcher_NotifyStatus)
{
    SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_WCDMA, IMS_TRUE);

    SetNetworkWatcher(static_cast<INetworkWatcher*>(&objMockINetworkWatcher));
    EXPECT_CALL(objMockINetworkWatcher, GetNetServiceType(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    MockIAosNetTrackerListener objMockIAosNetTrackerListener;
    EXPECT_CALL(objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(3);

    pAosNetTracker->SetSrvInGuardTime(TIMER_DURATION_FIVE_SEC);
    pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));
    pAosNetTracker->NetworkWatcher_NotifyStatus(
            static_cast<INetworkWatcher*>(&objMockINetworkWatcher));
}

TEST_F(AosNetTrackerTest, WifiWatcher_NotifyStateChanged)
{
    SetWifiWatcher(static_cast<IWifiWatcher*>(&objMockIWifiWatcher));
    EXPECT_CALL(objMockIWifiWatcher, GetState())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IWifiWatcher::STATE_CONNECTED));
    MockIAosNetTrackerListener objMockIAosNetTrackerListener;
    EXPECT_CALL(objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);

    pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));
    pAosNetTracker->WifiWatcher_NotifyStateChanged(
            static_cast<IWifiWatcher*>(&objMockIWifiWatcher));
}

TEST_F(AosNetTrackerTest, Event_NotifyEvent)
{
    SetNetworkWatcher(static_cast<INetworkWatcher*>(&objMockINetworkWatcher));
    EXPECT_CALL(objMockINetworkWatcher, GetNetServiceType(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    MockIAosNetTrackerListener objMockIAosNetTrackerListener;
    EXPECT_CALL(objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);

    pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));
    pAosNetTracker->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_ON);
}

TEST_F(AosNetTrackerTest, NotifyInGuardTimerExpired)
{
    StartTimer(AosNetTracker::TIMER_IN_GUARD, TIMER_DURATION_FIVE_SEC);
    MockIAosNetTrackerListener objMockIAosNetTrackerListener;
    EXPECT_CALL(objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);

    pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));
    NotifyTimerExpired(AosNetTracker::TIMER_IN_GUARD);
}

TEST_F(AosNetTrackerTest, NotifyOutGuardTimerExpired)
{
    StartTimer(AosNetTracker::TIMER_OUT_GUARD, TIMER_DURATION_FIVE_SEC);
    MockIAosNetTrackerListener objMockIAosNetTrackerListener;
    EXPECT_CALL(objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);

    pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));
    NotifyTimerExpired(AosNetTracker::TIMER_OUT_GUARD);
}

TEST_F(AosNetTrackerTest, NotifyRatGuardTimerExpired)
{
    StartTimer(AosNetTracker::TIMER_RAT_GUARD, TIMER_DURATION_FIVE_SEC);
    MockIAosNetTrackerListener objMockIAosNetTrackerListener;
    EXPECT_CALL(objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);

    pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));

    NotifyTimerExpired(AosNetTracker::TIMER_RAT_GUARD);
}

TEST_F(AosNetTrackerTest, NotifyVoiceRatGuardTimerExpired)
{
    StartTimer(AosNetTracker::TIMER_VOICE_RAT_GUARD, TIMER_DURATION_FIVE_SEC);
    MockIAosNetTrackerListener objMockIAosNetTrackerListener;
    EXPECT_CALL(objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);

    pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));

    NotifyTimerExpired(AosNetTracker::TIMER_VOICE_RAT_GUARD);
}

TEST_F(AosNetTrackerTest, AosConnection_StateChanged)
{
    MockIAosNetTrackerListener objMockIAosNetTrackerListener;
    EXPECT_CALL(objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);
    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));

    NotifyConnectionStateChanged(IAosConnection::STATE_ACTIVE);
}

TEST_F(AosNetTrackerTest, AosConnection_IpcanCatChanged)
{
    MockIAosNetTrackerListener objMockIAosNetTrackerListener;
    EXPECT_CALL(objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);
    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    pAosNetTracker->SetListener(
            static_cast<IAosNetTrackerListener*>(&objMockIAosNetTrackerListener));

    NotifyIpcanChanged();
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