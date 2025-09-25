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
#include "provider/AosProvider.h"

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

#define DECLARE_USING(Base)                     \
    using Base::Init;                           \
    using Base::InitObject;                     \
    using Base::UpdateNetworkStatus;            \
    using Base::Notify;                         \
    using Base::GetAccessPolicy;                \
    using Base::SetDataConnected;               \
    using Base::SetEpdgEnabled;                 \
    using Base::SetWifiConnected;               \
    using Base::StartTimer;                     \
    using Base::StopTimer;                      \
    using Base::AosConnection_StateChanged;     \
    using Base::AosConnection_IpChanged;        \
    using Base::AosConnection_IpcanCatChanged;  \
    using Base::AosConnection_PcscfChanged;     \
    using Base::AosConnection_ConnectionFailed; \
    using Base::Timer_TimerExpired;             \
    using Base::FeaturesToString;               \
    using Base::RadioTypeToString;              \
    using Base::ServiceTypeToString;

const IMS_SINT32 SLOT_ID = 0;
const AString PROFILE_ID = AString("test");
const IMS_UINT32 TIMER_DURATION_FIVE_SEC = 5;

class TestAosNetTracker : public AosNetTracker
{
public:
    DECLARE_USING(AosNetTracker)

    explicit inline TestAosNetTracker(IN const IAosAppContext* piAppContext) :
            AosNetTracker(piAppContext)
    {
    }

    inline void SetCnxPolicy(IN IMS_UINT32 nPolicy) { m_nCnxPolicy = nPolicy; }
    inline void SetCnxPolicyInRoaming(IN IMS_UINT32 nPolicy) { m_nCnxPolicyInRoaming = nPolicy; }
    inline void SetNetworkWatcher(IN INetworkWatcher* piNw) { m_piNetWatcherInfo = piNw; }
    inline INetworkWatcher* GetNetworkWatcher() { return m_piNetWatcherInfo; }
    inline void SetWifiWatcher(IN IWifiWatcher* piWw) { m_piWifiWatcher = piWw; }
    inline void SetStatus(IN IMS_SINT32 nService, IN IMS_UINT32 nRadioTech, IN IMS_BOOL bIsIn)
    {
        m_nNetServiceType = nService;
        m_nNetRadioType = nRadioTech;
        m_bIsNetAvailable = bIsIn;
    }
    inline void SetChangingRat(IN IMS_UINT32 nRat) { m_nChangingRat = nRat; }
    inline void SetFeature(IMS_UINT32 nFeature) { m_nFeature = nFeature; }
    inline IMS_UINT32 GetFeature() { return m_nFeature; }
    inline ImsList<IAosNetTrackerListener*> GetListeners() { return m_objListeners; }

    ITimer* GetTimer(IN IMS_UINT32 nType)
    {
        switch (nType)
        {
            case AosNetTracker::TIMER_IN_GUARD:
                return m_piServiceInTimer;
            case AosNetTracker::TIMER_OUT_GUARD:
                return m_piServiceOutTimer;
            case AosNetTracker::TIMER_RAT_GUARD:
                return m_piRatTimer;
            case AosNetTracker::TIMER_VOICE_RAT_GUARD:
                return m_piVoiceRatTimer;
            default:
                return IMS_NULL;
        }
    }
};

class AosNetTrackerTest : public ::testing::Test
{
public:
    AosNetTrackerTest() :
            m_pAosNetTracker(IMS_NULL)
    {
        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration(SLOT_ID);
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration, SLOT_ID);
    }
    virtual ~AosNetTrackerTest()
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
    }

    TestAosNetTracker* m_pAosNetTracker;
    IAosNConfiguration* m_piAosNConfiguration;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosNConfiguration m_objMockIAosNConfiguration;
    MockIAosNetTrackerListener m_objMockIAosNetTrackerListener;
    MockINetworkWatcher m_objMockINetworkWatcher;
    MockIWifiWatcher m_objMockIWifiWatcher;

    ImsVector<IMS_SINT32> objRats;

protected:
    void SetUp() override
    {
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(PROFILE_ID));
        ON_CALL(m_objMockIAosAppContext, GetConnection())
                .WillByDefault(Return(&m_objMockIAosConnection));

        objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN);
        objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN);
        objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
        objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN);
        objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);
        ON_CALL(m_objMockIAosNConfiguration, GetSupportedRats()).WillByDefault(ReturnRef(objRats));
        ON_CALL(m_objMockIAosNConfiguration, GetSupportedRoamingRats())
                .WillByDefault(ReturnRef(objRats));
        ON_CALL(m_objMockIAosNConfiguration, GetSmsOverImsSupportedRats())
                .WillByDefault(ReturnRef(objRats));

        m_pAosNetTracker = new TestAosNetTracker(&m_objMockIAosAppContext);
        ASSERT_TRUE(m_pAosNetTracker != nullptr);

        m_pAosNetTracker->SetListener(&m_objMockIAosNetTrackerListener);
        m_pAosNetTracker->InitObject();
    }

    void TearDown() override
    {
        if (m_pAosNetTracker)
        {
            delete m_pAosNetTracker;
            m_pAosNetTracker = IMS_NULL;
        }
    }
};

TEST_F(AosNetTrackerTest, Init_ImsType)
{
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_IMS));

    m_pAosNetTracker->Init();

    IMS_UINT32 nCnxPolicy = NW_REPORT_SRV_SRV | NW_REPORT_RADIO_GSM | NW_REPORT_RADIO_EDGE |
            NW_REPORT_RADIO_WCDMA | NW_REPORT_RADIO_HSPA | NW_REPORT_RADIO_LTE |
            NW_REPORT_RADIO_NR | NW_REPORT_RADIO_WLAN;
    EXPECT_EQ(m_pAosNetTracker->GetAccessPolicy(), nCnxPolicy);
    EXPECT_EQ(m_pAosNetTracker->GetFeature(),
            AosNetTracker::FEATURE_IN_GUARD | AosNetTracker::FEATURE_OUT_GUARD);
    EXPECT_NE(m_pAosNetTracker->GetNetworkWatcher(), nullptr);
}

TEST_F(AosNetTrackerTest, Init_EmergencyType)
{
    ImsVector<IMS_SINT32> objRats;
    objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN);
    ON_CALL(m_objMockIAosNConfiguration, GetEmergencyOverImsSupportedRats())
            .WillByDefault(ReturnRef(objRats));
    ON_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillByDefault(Return(NetworkPolicy::APN_EMERGENCY));

    m_pAosNetTracker->Init();

    IMS_UINT32 nExpectedCnxPolicy = 0xFFFFFFFF & (~NW_REPORT_RADIO_WLAN);
    EXPECT_EQ(m_pAosNetTracker->GetAccessPolicy(), nExpectedCnxPolicy);
    EXPECT_EQ(m_pAosNetTracker->GetFeature(),
            AosNetTracker::FEATURE_IN_GUARD | AosNetTracker::FEATURE_OUT_GUARD);
    EXPECT_NE(m_pAosNetTracker->GetNetworkWatcher(), nullptr);
}

TEST_F(AosNetTrackerTest, Init_WifiType)
{
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_WIFI));

    m_pAosNetTracker->Init();

    EXPECT_EQ(m_pAosNetTracker->GetAccessPolicy(), 0x01000000);
    EXPECT_EQ(m_pAosNetTracker->GetFeature(),
            AosNetTracker::FEATURE_IN_GUARD | AosNetTracker::FEATURE_OUT_GUARD);
    EXPECT_NE(m_pAosNetTracker->GetNetworkWatcher(), nullptr);
}

TEST_F(AosNetTrackerTest, IsServiceIn)
{
    // both Mobile and WLAN are not ServiceIn
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    m_pAosNetTracker->SetWifiConnected(IMS_FALSE);
    EXPECT_FALSE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_MOBILE));
    EXPECT_FALSE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_WLAN));
    EXPECT_FALSE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_DEFAULT));

    // Mobile is ServiceIn but WLAN is not ServiceIn
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_LTE, IMS_TRUE);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_MOBILE));
    EXPECT_TRUE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_DEFAULT));
    EXPECT_FALSE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_WLAN));

    m_pAosNetTracker->SetEpdgEnabled(IMS_TRUE);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_DEFAULT));
    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE | NW_REPORT_RADIO_WLAN);

    // WLAN isServiceIn
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_WLAN, IMS_FALSE);
    m_pAosNetTracker->SetWifiConnected(IMS_TRUE);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_WLAN));
    EXPECT_TRUE(m_pAosNetTracker->IsServiceIn(IAosNetTracker::TYPE_DEFAULT));
}

TEST_F(AosNetTrackerTest, IsDataIn)
{
    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);

    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    EXPECT_FALSE(m_pAosNetTracker->IsDataIn());

    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    EXPECT_TRUE(m_pAosNetTracker->IsDataIn());
}

TEST_F(AosNetTrackerTest, IsNetworkIn)
{
    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);

    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    EXPECT_FALSE(m_pAosNetTracker->IsNetworkIn());

    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    EXPECT_TRUE(m_pAosNetTracker->IsNetworkIn());
}

TEST_F(AosNetTrackerTest, IsEmergencyAttach)
{
    EXPECT_CALL(m_objMockINetworkWatcher, IsEmergencyOnly())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    EXPECT_TRUE(m_pAosNetTracker->IsEmergencyAttach());
    EXPECT_FALSE(m_pAosNetTracker->IsEmergencyAttach());
}

TEST_F(AosNetTrackerTest, IsSuspended)
{
    EXPECT_FALSE(m_pAosNetTracker->IsSuspended());
    m_pAosNetTracker->SetDataConnected(IMS_TRUE);
    EXPECT_TRUE(m_pAosNetTracker->IsSuspended());
    m_pAosNetTracker->SetEpdgEnabled(IMS_TRUE);
    EXPECT_FALSE(m_pAosNetTracker->IsSuspended());
}

TEST_F(AosNetTrackerTest, IsSessionContinuitySupported)
{
    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE | NW_REPORT_RADIO_NR);
    EXPECT_FALSE(m_pAosNetTracker->IsSessionContinuitySupported());

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE | NW_REPORT_RADIO_WLAN);
    EXPECT_TRUE(m_pAosNetTracker->IsSessionContinuitySupported());

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_NR | NW_REPORT_RADIO_WLAN);
    EXPECT_TRUE(m_pAosNetTracker->IsSessionContinuitySupported());
}

TEST_F(AosNetTrackerTest, IsServiceTimerRunning)
{
    m_pAosNetTracker->StartTimer(
            AosNetTracker::TIMER_IN_GUARD, AosNetTracker::SERVICE_IN_TIME_MILLI_SEC);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceTimerRunning());

    // If the timer is running, stop previous timer and start again.
    m_pAosNetTracker->StartTimer(
            AosNetTracker::TIMER_IN_GUARD, AosNetTracker::SERVICE_IN_TIME_MILLI_SEC);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceTimerRunning());

    m_pAosNetTracker->StopTimer(AosNetTracker::TIMER_IN_GUARD);
    EXPECT_FALSE(m_pAosNetTracker->IsServiceTimerRunning());

    m_pAosNetTracker->StartTimer(
            AosNetTracker::TIMER_OUT_GUARD, AosNetTracker::SERVICE_OUT_TIME_MILLI_SEC);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceTimerRunning());

    m_pAosNetTracker->StopTimer(AosNetTracker::TIMER_OUT_GUARD);
    EXPECT_FALSE(m_pAosNetTracker->IsServiceTimerRunning());
}

TEST_F(AosNetTrackerTest, IsImsVoiceCallSupported)
{
    EXPECT_CALL(m_objMockINetworkWatcher, IsImsVoiceCallSupported())
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    EXPECT_TRUE(m_pAosNetTracker->IsImsVoiceCallSupported());
    EXPECT_FALSE(m_pAosNetTracker->IsImsVoiceCallSupported());
}

TEST_F(AosNetTrackerTest, SetListener_NotAddNullOrSameListenerAgain)
{
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(0);

    // not set invalid listener
    m_pAosNetTracker->SetListener(IMS_NULL);

    // not set same listenr agaiin
    m_pAosNetTracker->SetListener(&m_objMockIAosNetTrackerListener);
}

TEST_F(AosNetTrackerTest, RemoveListener)
{
    EXPECT_EQ(m_pAosNetTracker->GetListeners().GetSize(), 1);

    // not remove invalid listener
    m_pAosNetTracker->RemoveListener(nullptr);
    EXPECT_EQ(m_pAosNetTracker->GetListeners().GetSize(), 1);

    m_pAosNetTracker->RemoveListener(&m_objMockIAosNetTrackerListener);
    EXPECT_EQ(m_pAosNetTracker->GetListeners().GetSize(), 0);

    m_pAosNetTracker->Notify();
}

TEST_F(AosNetTrackerTest, GetMobileChangingNetworkType)
{
    EXPECT_EQ(m_pAosNetTracker->GetMobileChangingNetworkType(), NW_REPORT_RADIO_NOSRV);
    m_pAosNetTracker->SetChangingRat(NW_REPORT_RADIO_LTE);
    EXPECT_EQ(m_pAosNetTracker->GetMobileChangingNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosNetTrackerTest, GetMobileNetworkType)
{
    EXPECT_EQ(m_pAosNetTracker->GetMobileNetworkType(), NW_REPORT_RADIO_NOSRV);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    EXPECT_EQ(m_pAosNetTracker->GetMobileNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosNetTrackerTest, GetMobileNetworkRegistrationRejectCause)
{
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetworkRegistrationRejectCause()).WillOnce(Return(3));
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    EXPECT_EQ(m_pAosNetTracker->GetMobileNetworkRegistrationRejectCause(), 3);
}

TEST_F(AosNetTrackerTest, GetMobileNetworkPlmn)
{
    AString strPlmn("00101");
    EXPECT_CALL(m_objMockINetworkWatcher, GetAccessNetworkPlmn()).WillOnce(Return(strPlmn));
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    EXPECT_EQ(m_pAosNetTracker->GetMobileNetworkPlmn(), strPlmn);
}

TEST_F(AosNetTrackerTest, GetMobileVoiceServiceState)
{
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceServiceType())
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    EXPECT_EQ(m_pAosNetTracker->GetMobileVoiceServiceState(), NW_REPORT_SRV_SRV);
}

TEST_F(AosNetTrackerTest, GetMobileVoiceNetworkType)
{
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    EXPECT_EQ(m_pAosNetTracker->GetMobileVoiceNetworkType(), NW_REPORT_RADIO_NOSRV);
    m_pAosNetTracker->UpdateNetworkStatus();
    EXPECT_EQ(m_pAosNetTracker->GetMobileVoiceNetworkType(), NW_REPORT_RADIO_LTE);
}

TEST_F(AosNetTrackerTest, ReturnOutOfServiceWhenGetMobileServiceState)
{
    ON_CALL(m_objMockINetworkWatcher, GetCellularServiceState())
            .WillByDefault(Return(MockINetworkWatcher::STATE_OUT_OF_SERVICE));
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);

    EXPECT_EQ(m_pAosNetTracker->GetMobileServiceState(), INetworkWatcher::STATE_OUT_OF_SERVICE);
}

TEST_F(AosNetTrackerTest, GetNetworkType)
{
    EXPECT_EQ(m_pAosNetTracker->GetNetworkType(), NW_REPORT_RADIO_NOSRV);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    EXPECT_EQ(m_pAosNetTracker->GetNetworkType(), NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetEpdgEnabled(IMS_TRUE);
    EXPECT_EQ(m_pAosNetTracker->GetNetworkType(), NW_REPORT_RADIO_WLAN);
}

TEST_F(AosNetTrackerTest, GetNetworkOperatorReturnsGivenPlmnValue)
{
    // GIVEN
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    ON_CALL(m_objMockINetworkWatcher, GetNetworkOperator())
            .WillByDefault(Return(AString("311480")));

    // WHEN & THEN
    EXPECT_STREQ(m_pAosNetTracker->GetNetworkOperator().GetStr(), AString("311480").GetStr());
}

TEST_F(AosNetTrackerTest, GetNetworkOperatorReturnsEmptyIfGivenValueIsZeroLength)
{
    // GIVEN
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    ON_CALL(m_objMockINetworkWatcher, GetNetworkOperator()).WillByDefault(Return(AString("")));

    // WHEN & THEN
    EXPECT_TRUE(m_pAosNetTracker->GetNetworkOperator().IsEmpty());
}

TEST_F(AosNetTrackerTest, SetRatGuardTime)
{
    EXPECT_EQ(m_pAosNetTracker->GetFeature(), AosNetTracker::FEATURE_NONE);

    m_pAosNetTracker->SetRatGuardTime(TIMER_DURATION_FIVE_SEC);
    IMS_UINT32 nExpect = AosNetTracker::FEATURE_RAT_GUARD | AosNetTracker::FEATURE_VOICE_RAT_GUARD;
    EXPECT_EQ(m_pAosNetTracker->GetFeature(), nExpect);

    m_pAosNetTracker->SetRatGuardTime(0);
    EXPECT_EQ(m_pAosNetTracker->GetFeature(), AosNetTracker::FEATURE_NONE);
}

TEST_F(AosNetTrackerTest, SetSrvOutGuardTime)
{
    EXPECT_EQ(m_pAosNetTracker->GetFeature(), AosNetTracker::FEATURE_NONE);

    m_pAosNetTracker->SetSrvOutGuardTime(TIMER_DURATION_FIVE_SEC);
    EXPECT_EQ(m_pAosNetTracker->GetFeature(), AosNetTracker::FEATURE_OUT_GUARD);

    m_pAosNetTracker->SetSrvOutGuardTime(0);
    EXPECT_EQ(m_pAosNetTracker->GetFeature(), AosNetTracker::FEATURE_NONE);
}

TEST_F(AosNetTrackerTest, SetSrvInGuardTime)
{
    EXPECT_EQ(m_pAosNetTracker->GetFeature(), AosNetTracker::FEATURE_NONE);

    m_pAosNetTracker->SetSrvInGuardTime(TIMER_DURATION_FIVE_SEC);
    EXPECT_EQ(m_pAosNetTracker->GetFeature(), AosNetTracker::FEATURE_IN_GUARD);

    m_pAosNetTracker->SetSrvInGuardTime(0);
    EXPECT_EQ(m_pAosNetTracker->GetFeature(), AosNetTracker::FEATURE_NONE);
}

TEST_F(AosNetTrackerTest, NetworkWatcher_NotifyStatus_FromOtherNetworkWatcher)
{
    MockINetworkWatcher objOtherMockINetworkWatcher;

    // not handle because it is invoked from the other NetworkWatcher
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _)).Times(0);
    EXPECT_CALL(objOtherMockINetworkWatcher, GetNetServiceType(_, _)).Times(0);

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_WCDMA, IMS_TRUE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);

    m_pAosNetTracker->NetworkWatcher_NotifyStatus(&objOtherMockINetworkWatcher);
}

TEST_F(AosNetTrackerTest, NetworkWatcher_NotifyStatus_WithSrvInGuardTime)
{
    // 1. Notify RAT change from WCDM to LTE, 2. Notify Voice RAT from NOSRV to LTE
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE));

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_WCDMA, IMS_TRUE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    m_pAosNetTracker->SetSrvInGuardTime(TIMER_DURATION_FIVE_SEC);

    m_pAosNetTracker->NetworkWatcher_NotifyStatus(&m_objMockINetworkWatcher);
}

TEST_F(AosNetTrackerTest, NetworkWatcher_NotifyStatus_WithoutGuardTime)
{
    // notify RAT change from WCDM to LTE
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE));

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_WCDMA, IMS_TRUE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);

    m_pAosNetTracker->NetworkWatcher_NotifyStatus(&m_objMockINetworkWatcher);
}

TEST_F(AosNetTrackerTest, NetworkWatcher_NotifyStatus_NoChanges)
{
    // not notify because there are no changes
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(0);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_LTE, IMS_TRUE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);

    m_pAosNetTracker->NetworkWatcher_NotifyStatus(&m_objMockINetworkWatcher);
}

TEST_F(AosNetTrackerTest, ProcessNetworkChanged_NoChanges)
{
    // not notify because there are no changes
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(0);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NW_REPORT_RADIO_NOSRV));

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_LTE, IMS_TRUE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);

    // with FEATURE_IN_GUARD
    m_pAosNetTracker->SetSrvInGuardTime(TIMER_DURATION_FIVE_SEC);
    m_pAosNetTracker->NetworkWatcher_NotifyStatus(&m_objMockINetworkWatcher);
    m_pAosNetTracker->SetSrvInGuardTime(0);

    // with FEATURE_RAT_GUARD
    m_pAosNetTracker->SetRatGuardTime(TIMER_DURATION_FIVE_SEC);
    m_pAosNetTracker->NetworkWatcher_NotifyStatus(&m_objMockINetworkWatcher);
    m_pAosNetTracker->SetRatGuardTime(0);
}

TEST_F(AosNetTrackerTest, ProcessNetworkChanged_OutSrvToInSrv_WithSrvInGuardTime)
{
    // not notify any more
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(0);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    m_pAosNetTracker->SetSrvInGuardTime(TIMER_DURATION_FIVE_SEC);

    // Do not notify because service status is changed with FEATURE_IN_GUARD
    m_pAosNetTracker->NetworkWatcher_NotifyStatus(&m_objMockINetworkWatcher);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceTimerRunning());
}

TEST_F(AosNetTrackerTest, ProcessNetworkChanged_OutSrvToInSrv_WithRatGuardTime)
{
    // notify service status change
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    m_pAosNetTracker->SetRatGuardTime(TIMER_DURATION_FIVE_SEC);

    // notify status change because service status is changed without FEATURE_IN_GUARD
    m_pAosNetTracker->NetworkWatcher_NotifyStatus(&m_objMockINetworkWatcher);
}

TEST_F(AosNetTrackerTest, ProcessNetworkChanged_InSrvToOutSrv_WithSrvOutGuardTime)
{
    // not notify because service status is changed with FEATURE_IN_GUARD
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(0);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_NOSRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_LTE, IMS_TRUE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    m_pAosNetTracker->SetSrvOutGuardTime(TIMER_DURATION_FIVE_SEC);

    m_pAosNetTracker->NetworkWatcher_NotifyStatus(&m_objMockINetworkWatcher);
    EXPECT_TRUE(m_pAosNetTracker->IsServiceTimerRunning());
}

TEST_F(AosNetTrackerTest, ProcessNetworkChanged_InSrvToOutSrv_WithRatGuardTime)
{
    // notify service status change because service status is changed without FEATURE_OUT_GUARD
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_NOSRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_NOSRV));

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_LTE, IMS_TRUE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    m_pAosNetTracker->SetRatGuardTime(TIMER_DURATION_FIVE_SEC);

    m_pAosNetTracker->NetworkWatcher_NotifyStatus(&m_objMockINetworkWatcher);
}

TEST_F(AosNetTrackerTest, WifiWatcher_NotifyStateChanged_FromOtherWifiWatcher)
{
    MockIWifiWatcher objOtherMockIWifiWatcher;

    // not handle because it is invoked from the other WifiWatcher
    EXPECT_CALL(m_objMockIWifiWatcher, GetState()).Times(0);
    EXPECT_CALL(objOtherMockIWifiWatcher, GetState()).Times(0);

    m_pAosNetTracker->SetWifiWatcher(&m_objMockIWifiWatcher);

    m_pAosNetTracker->WifiWatcher_NotifyStateChanged(&objOtherMockIWifiWatcher);
}

TEST_F(AosNetTrackerTest, WifiWatcher_NotifyStateChanged)
{
    // 1. Notify STATE_CONNECTED, 2. Notify STATE_DISCONNECTED
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(2);
    EXPECT_CALL(m_objMockIWifiWatcher, GetState())
            .Times(2)
            .WillOnce(Return(IWifiWatcher::STATE_CONNECTED))
            .WillOnce(Return(IWifiWatcher::STATE_DISCONNECTED));

    m_pAosNetTracker->SetWifiWatcher(&m_objMockIWifiWatcher);

    m_pAosNetTracker->WifiWatcher_NotifyStateChanged(&m_objMockIWifiWatcher);
    m_pAosNetTracker->WifiWatcher_NotifyStateChanged(&m_objMockIWifiWatcher);
}

TEST_F(AosNetTrackerTest, NConfiguration_NotifyConfigChanged)
{
    // notify carrier configuration for supported RAT is changed
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
    EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
            .Times(AnyNumber())
            .WillRepeatedly(Return(NetworkPolicy::APN_IMS));

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_LTE, IMS_FALSE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);

    m_pAosNetTracker->NConfiguration_NotifyConfigChanged();
}

TEST_F(AosNetTrackerTest, UpdateCnxPolicyWhenNotifyConfigChanged)
{
    IMS_UINT32 nExpectedCnxPolicy = 0xFFFFFFFF & (~NW_REPORT_RADIO_WLAN);
    m_pAosNetTracker->SetCnxPolicy(nExpectedCnxPolicy);
    ImsVector<IMS_SINT32> objRats;
    objRats.Add(CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN);
    ON_CALL(m_objMockIAosNConfiguration, GetEmergencyOverImsSupportedRats())
            .WillByDefault(ReturnRef(objRats));
    ON_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillByDefault(Return(NetworkPolicy::APN_EMERGENCY));

    m_pAosNetTracker->NConfiguration_NotifyConfigChanged();

    nExpectedCnxPolicy |= NW_REPORT_RADIO_WLAN;
    EXPECT_EQ(m_pAosNetTracker->GetAccessPolicy(), nExpectedCnxPolicy);
}

TEST_F(AosNetTrackerTest, Event_NotifyEvent_WithRatGuardTime)
{
    // ProcessNetworkChanged
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE));

    m_pAosNetTracker->SetCnxPolicyInRoaming(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_NOSRV, NW_REPORT_RADIO_NOSRV, IMS_FALSE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    m_pAosNetTracker->SetRatGuardTime(TIMER_DURATION_FIVE_SEC);

    m_pAosNetTracker->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_ON);
}

TEST_F(AosNetTrackerTest, Event_NotifyEvent_WithoutGuardTime)
{
    // notify change of roaming state
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetServiceType(_, _))
            .WillOnce(Return(NW_REPORT_SRV_SRV));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetRadioTechType(_, _))
            .WillOnce(Return(NW_REPORT_RADIO_LTE));
    EXPECT_CALL(m_objMockINetworkWatcher, GetNetVoiceRadioTechType())
            .WillOnce(Return(NW_REPORT_RADIO_LTE));

    m_pAosNetTracker->SetCnxPolicy(NW_REPORT_SRV_SRV | NW_REPORT_RADIO_LTE);
    m_pAosNetTracker->SetStatus(NW_REPORT_SRV_SRV, NW_REPORT_RADIO_WCDMA, IMS_TRUE);
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);

    m_pAosNetTracker->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_ON);
}

TEST_F(AosNetTrackerTest, NotifyInGuardTimerExpired)
{
    // notify timer is expired
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);

    m_pAosNetTracker->StartTimer(
            AosNetTracker::TIMER_IN_GUARD, AosNetTracker::SERVICE_IN_TIME_MILLI_SEC);

    m_pAosNetTracker->Timer_TimerExpired(m_pAosNetTracker->GetTimer(AosNetTracker::TIMER_IN_GUARD));
}

TEST_F(AosNetTrackerTest, NotifyOutGuardTimerExpired)
{
    // notify timer is expired
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);

    m_pAosNetTracker->StartTimer(
            AosNetTracker::TIMER_OUT_GUARD, AosNetTracker::SERVICE_OUT_TIME_MILLI_SEC);

    m_pAosNetTracker->Timer_TimerExpired(
            m_pAosNetTracker->GetTimer(AosNetTracker::TIMER_OUT_GUARD));
}

TEST_F(AosNetTrackerTest, NotifyRatGuardTimerExpired)
{
    // notify timer is expired
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);

    m_pAosNetTracker->StartTimer(AosNetTracker::TIMER_RAT_GUARD, TIMER_DURATION_FIVE_SEC);

    m_pAosNetTracker->Timer_TimerExpired(
            m_pAosNetTracker->GetTimer(AosNetTracker::TIMER_RAT_GUARD));
}

TEST_F(AosNetTrackerTest, NotifyVoiceRatGuardTimerExpired)
{
    // notify timer is expired
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);

    m_pAosNetTracker->StartTimer(AosNetTracker::TIMER_VOICE_RAT_GUARD, TIMER_DURATION_FIVE_SEC);

    m_pAosNetTracker->Timer_TimerExpired(
            m_pAosNetTracker->GetTimer(AosNetTracker::TIMER_VOICE_RAT_GUARD));
}

TEST_F(AosNetTrackerTest, HandleInvalidTimer)
{
    IMS_UINT32 nInvalidTimer = -1;
    m_pAosNetTracker->StartTimer(nInvalidTimer, TIMER_DURATION_FIVE_SEC);
    m_pAosNetTracker->StopTimer(nInvalidTimer);
    m_pAosNetTracker->Timer_TimerExpired(IMS_NULL);
}

TEST_F(AosNetTrackerTest, AosConnection_StateChanged)
{
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled()).WillOnce(Return(IMS_FALSE));

    m_pAosNetTracker->AosConnection_StateChanged(IAosConnection::STATE_ACTIVE);
}

TEST_F(AosNetTrackerTest, AosConnection_IpcanCatChanged)
{
    EXPECT_CALL(m_objMockIAosNetTrackerListener, NetTracker_StatusChanged()).Times(1);
    EXPECT_CALL(m_objMockIAosConnection, IsEpdgEnabled())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    // notify that EPDG is enabled
    m_pAosNetTracker->AosConnection_IpcanCatChanged();

    // notify again that EPDG is enabled (this would be not handled)
    m_pAosNetTracker->AosConnection_IpcanCatChanged();
}

TEST_F(AosNetTrackerTest, FeaturesToString)
{
    m_pAosNetTracker->SetFeature(AosNetTracker::FEATURE_IN_GUARD |
            AosNetTracker::FEATURE_OUT_GUARD | AosNetTracker::FEATURE_RAT_GUARD |
            AosNetTracker::FEATURE_VOICE_RAT_GUARD);
    EXPECT_STREQ(m_pAosNetTracker->FeaturesToString().GetStr(),
            "| FEATURE_IN_GUARD | FEATURE_OUT_GUARD | FEATURE_RAT_GUARD | FEATURE_VOICE_RAT_GUARD "
            "| ");
}

TEST_F(AosNetTrackerTest, RadioTypeToString)
{
    EXPECT_STREQ(
            m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_NOSRV), "NW_REPORT_RADIO_NOSRV");
    EXPECT_STREQ(m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_AMPS), "NW_REPORT_RADIO_AMPS");
    EXPECT_STREQ(m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_CDMA), "NW_REPORT_RADIO_CDMA");
    EXPECT_STREQ(m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_GSM), "NW_REPORT_RADIO_GSM");
    EXPECT_STREQ(m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_HDR), "NW_REPORT_RADIO_HDR");
    EXPECT_STREQ(
            m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_WCDMA), "NW_REPORT_RADIO_WCDMA");
    EXPECT_STREQ(m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_GPS), "NW_REPORT_RADIO_GPS");
    EXPECT_STREQ(m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_EDGE), "NW_REPORT_RADIO_EDGE");
    EXPECT_STREQ(m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_WLAN), "NW_REPORT_RADIO_WLAN");
    EXPECT_STREQ(
            m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_EVDODO), "NW_REPORT_RADIO_EVDODO");
    EXPECT_STREQ(
            m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_EHRPD), "NW_REPORT_RADIO_EHRPD");
    EXPECT_STREQ(m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_LTE), "NW_REPORT_RADIO_LTE");
    EXPECT_STREQ(m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_HSPA), "NW_REPORT_RADIO_HSPA");
    EXPECT_STREQ(m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_NR), "NW_REPORT_RADIO_NR");
    EXPECT_STREQ(m_pAosNetTracker->RadioTypeToString(NW_REPORT_RADIO_INVALID), "__INVALID__");
}

TEST_F(AosNetTrackerTest, ServiceTypeToString)
{
    EXPECT_STREQ(m_pAosNetTracker->ServiceTypeToString(NW_REPORT_SRV_NOSRV), "NW_REPORT_SRV_NOSRV");
    EXPECT_STREQ(
            m_pAosNetTracker->ServiceTypeToString(NW_REPORT_SRV_LIMITED), "NW_REPORT_SRV_LIMITED");
    EXPECT_STREQ(m_pAosNetTracker->ServiceTypeToString(NW_REPORT_SRV_SRV), "NW_REPORT_SRV_SRV");
    EXPECT_STREQ(m_pAosNetTracker->ServiceTypeToString(NW_REPORT_SRV_LIMITEDREGION),
            "NW_REPORT_SRV_LIMITEDREGION");
    EXPECT_STREQ(
            m_pAosNetTracker->ServiceTypeToString(NW_REPORT_SRV_PWRSAVE), "NW_REPORT_SRV_PWRSAVE");
}

TEST_F(AosNetTrackerTest, AosConnection_IpChanged)
{
    // Currently, there is no logic that requires tests.
    m_pAosNetTracker->AosConnection_IpChanged();
}

TEST_F(AosNetTrackerTest, AosConnection_PcscfChanged)
{
    // Currently, there is no logic that requires tests.
    m_pAosNetTracker->AosConnection_PcscfChanged();
}

TEST_F(AosNetTrackerTest, AosConnection_ConnectionFailed)
{
    // Currently, there is no logic that requires tests.
    m_pAosNetTracker->AosConnection_ConnectionFailed();
}

TEST_F(AosNetTrackerTest, IsRoaming)
{
    m_pAosNetTracker->SetNetworkWatcher(&m_objMockINetworkWatcher);
    EXPECT_CALL(m_objMockINetworkWatcher, GetRoamingState())
            .Times(2)
            .WillOnce(Return(1))
            .WillOnce(Return(0));
    EXPECT_TRUE(m_pAosNetTracker->IsRoaming());
    EXPECT_FALSE(m_pAosNetTracker->IsRoaming());
}
