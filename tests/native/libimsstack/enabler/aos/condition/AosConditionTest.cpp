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

#include "ImsEventDef.h"
#include "ServiceNetworkPolicy.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosConditionListener.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosSubscriber.h"
#include "condition/AosBlock.h"
#include "condition/AosCondition.h"
#include "condition/AosServiceAvailable.h"
#include "condition/AosServiceAvailableCellular.h"
#include "condition/AosServiceAvailableWifi.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"

#include "../../../platform/interface/MockIPhoneInfoLocation.h"
#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosConditionListener.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosRegistration.h"
#include "interface/MockIAosSubscriber.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

class TestAosCondition : public AosCondition
{
public:
    inline explicit TestAosCondition(IN IAosAppContext* piAppContext) :
            AosCondition(piAppContext)
    {
    }

    inline IMS_BOOL IsReady() final { return AosCondition::IsReady(); }

    inline void SetAosBlock(IN IAosBlock* piBlock) { m_piBlock = piBlock; }

    inline void SetAosBlockToCellur(IN IAosBlock* piBlock)
    {
        m_pAvailableCellular->SetBlock(piBlock);
    }

    inline void SetAosBlockToWifi(IN IAosBlock* piBlock) { m_pAvailableWifi->SetBlock(piBlock); }

    inline IMS_BOOL IsRoaming() { return m_pAvailableCellular->IsRoaming(); }

    inline IMS_BOOL IsVopsSupported() { return m_pAvailableCellular->IsVopsSupported(); }

    inline void SetTestLocation(IN ILocationProperties* piTestLocation)
    {
        m_pAvailableWifi->SetLocation(piTestLocation);
    }

    // TEST : Start
    FRIEND_TEST(AosConditionTest, ShouldCreateAvailableCellularWhenStart);
    FRIEND_TEST(AosConditionTest, ShouldCreateAvailableWifiWhenStart);
    FRIEND_TEST(AosConditionTest, ShouldSetBlockWithAosIncompletedWhenStart);
    FRIEND_TEST(AosConditionTest, ShouldSetBlockWithSubscriberIncompletedWhenStart);
    // TEST : Stop
    FRIEND_TEST(AosConditionTest, ShouldDeleteAvailableCellularWhenStop);
    FRIEND_TEST(AosConditionTest, ShouldDeleteAvailableWifiWhenStop);

    FRIEND_TEST(AosConditionTest, SetListener);
    FRIEND_TEST(AosConditionTest, SetBlock);
    FRIEND_TEST(AosConditionTest, ResetBlock);
    FRIEND_TEST(AosConditionTest, IsReasonBlocked);
    // TEST : IsReady
    FRIEND_TEST(AosConditionTest, ReturnTrueWhenCellularServiceIsReady);
    FRIEND_TEST(AosConditionTest, ReturnTrueWhenWifiServiceIsReady);
    FRIEND_TEST(AosConditionTest, ReturnTrueWhenWholeServiceIsReady);
    FRIEND_TEST(AosConditionTest, ReturnTrueWhenWholeServiceIsReadyByCellular);
    FRIEND_TEST(AosConditionTest, ReturnTrueWhenWholeServiceIsReadyByWifi);
    FRIEND_TEST(AosConditionTest, ReturnFalseWhenCellularServiceIsNotReady);
    FRIEND_TEST(AosConditionTest, ReturnFalseWhenWifiServiceIsNotReady);
    FRIEND_TEST(AosConditionTest, ReturnFalseWhenWholeServiceIsNotReady);

    FRIEND_TEST(AosConditionTest, CheckServiceAvailable);
    FRIEND_TEST(AosConditionTest, CheckBadNetwork);
    FRIEND_TEST(AosConditionTest, Event_NotifyEvent_RoamingState);
    FRIEND_TEST(AosConditionTest, Event_NotifyEvent_VopsState);
    FRIEND_TEST(AosConditionTest, Event_NotifyEvent_LteInfo);
    FRIEND_TEST(AosConditionTest, CallTracker_StateChanged_Cs_Offhook);
    FRIEND_TEST(AosConditionTest, CallTracker_StateChanged_Cs_NotOffhook);
    FRIEND_TEST(AosConditionTest, CallTracker_StateChanged_Normal_Offhook);
    FRIEND_TEST(AosConditionTest, ResetBlockWhenNetTrackerStatusChangedWithServiceIn);
    FRIEND_TEST(AosConditionTest, SetBlockWhenNetTrackerStatusChangedWithServiceOut);
    FRIEND_TEST(AosConditionTest, SetBlockWhenSubscriberStateChangedWithRefreshStarted);
    FRIEND_TEST(AosConditionTest, Block_Changed);
    FRIEND_TEST(AosConditionTest, Subscriber_StateChanged_RefreshCompleted_RefreshStartedFalse);
    FRIEND_TEST(AosConditionTest, Subscriber_StateChanged_RefreshCompleted_RefreshStartedTrue);
    FRIEND_TEST(AosConditionTest, Subscriber_StateChanged_RefreshFailed);
    FRIEND_TEST(AosConditionTest, ServiceAvailable_RequestCommand_ListenerIsNotNull);
    FRIEND_TEST(AosConditionTest, NConfiguration_NotifyConfigChanged_NConfigIsNotNull);
    FRIEND_TEST(AosConditionTest, NConfiguration_NotifyConfigChanged_NConfigIsNull);
    FRIEND_TEST(AosConditionTest, ServicePhone_AosStart);
    FRIEND_TEST(AosConditionTest, ServicePhone_LocationInfoChanged_Changed);
    FRIEND_TEST(AosConditionTest, ServicePhone_LocationInfoChanged_NotChanged);
    FRIEND_TEST(AosConditionTest, ServicePhone_LocationInfoChanged_ReturnByConfig);
    FRIEND_TEST(AosConditionTest, ServicePhone_PhoneNumberStateChanged_RetryFailure);
    FRIEND_TEST(AosConditionTest, ServicePhone_PhoneNumberStateChanged_ClearReasonSimState);
    FRIEND_TEST(AosConditionTest, ServicePhone_PlmnChanged_ClearReaconPlmlChanged);
    FRIEND_TEST(AosConditionTest, ServicePhone_PowerOff_ListenerIsNull);
    FRIEND_TEST(AosConditionTest, ServicePhone_PowerOff_ListenerIsNotNull);
    FRIEND_TEST(AosConditionTest, ServiceSetting_AirplaneChanged_True_MatchedClearReason);
    FRIEND_TEST(AosConditionTest, ServiceSetting_AirplaneChanged_False);
    FRIEND_TEST(AosConditionTest, ServiceSetting_ServiceChanged_HoldEvent);
    FRIEND_TEST(AosConditionTest, ServiceSetting_ServiceChanged_On);
    FRIEND_TEST(AosConditionTest, ServiceSetting_ServiceChanged_Off);
    FRIEND_TEST(AosConditionTest, ServiceSetting_TtyChanged_On_RttNotSupport);
    FRIEND_TEST(AosConditionTest, ServiceSetting_TtyChanged_True_CombindAttached);
    FRIEND_TEST(AosConditionTest, ServiceSetting_TtyChanged_False);
    FRIEND_TEST(AosConditionTest, ServiceSetting_TtyChanged_TtyNotSupport);
    // TEST : Init
    FRIEND_TEST(AosConditionTest, DisableNetTrackerListenerWhenConnectionTypeIsWifi);
    FRIEND_TEST(AosConditionTest, DisableNetTrackerListenerWhenConnectionTypeIsEmergency);
    FRIEND_TEST(AosConditionTest, EnableNetTrackerListenerWhenConnectionTypeIsIms);

    FRIEND_TEST(AosConditionTest, AddListener);
    FRIEND_TEST(AosConditionTest, RemoveListener);
    FRIEND_TEST(AosConditionTest, IsListenerEnabled);
    FRIEND_TEST(AosConditionTest, AddHold_Roaming);
    FRIEND_TEST(AosConditionTest, AddHold_ImsService);
    FRIEND_TEST(AosConditionTest, AddHold_IsNotEventReset);
    FRIEND_TEST(AosConditionTest, AddHold_UninterestingEvent);
    FRIEND_TEST(AosConditionTest, RemoveHold_Roaming);
    FRIEND_TEST(AosConditionTest, RemoveHold_ImsService);
    FRIEND_TEST(AosConditionTest, RemoveHold_IsNotEventReset);
    FRIEND_TEST(AosConditionTest, RemoveHold_UninterestingEvent);
    FRIEND_TEST(AosConditionTest, RequestCommand_ListenerIsNull);
    FRIEND_TEST(AosConditionTest, RequestCommand_ListenerIsNotNull);
    FRIEND_TEST(AosConditionTest, UpdateRegistrationMode_ImpuCountIsGreaterThanOne);
    FRIEND_TEST(AosConditionTest, UpdateRegistrationMode_ImpuCountIsOne);
    FRIEND_TEST(AosConditionTest, UpdateRegistrationMode_NoBlockReason);
    FRIEND_TEST(AosConditionTest, UpdateRegistrationMode_IsNotReady);
};

class AosConditionTest : public ::testing::Test
{
public:
    TestAosCondition* m_pAosCondition;

    AosBlock* m_pAosBlock;
    IAosNConfiguration* m_piOriginConfiguration;
    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockIAosSubscriber m_objMockIAosSubscriber;
    MockIAosRegistration m_objMockIAosRegistration;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

protected:
    virtual void SetUp() override
    {
        m_piOriginConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration);

        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(0));

        const AString strValue = AString("test");
        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(strValue));

        ON_CALL(m_objMockIAosAppContext, GetConnection()).WillByDefault(ReturnNull());

        ON_CALL(m_objMockIAosAppContext, GetRegistration())
                .WillByDefault(Return(&m_objMockIAosRegistration));

        ON_CALL(m_objMockIAosAppContext, GetBlock()).WillByDefault(ReturnNull());

        ON_CALL(m_objMockIAosAppContext, GetNetTracker()).WillByDefault(ReturnNull());

        ON_CALL(m_objMockIAosAppContext, GetSubscriber()).WillByDefault(ReturnNull());

        m_pAosCondition = new TestAosCondition(&m_objMockIAosAppContext);
        ASSERT_TRUE(m_pAosCondition != nullptr);

        m_pAosBlock = new AosBlock(&m_objMockIAosAppContext);
        ASSERT_TRUE(m_pAosBlock != nullptr);

        m_pAosCondition->SetAosBlock(m_pAosBlock);
    }

    virtual void TearDown() override
    {
        if (m_pAosBlock)
        {
            delete m_pAosBlock;
        }

        if (m_pAosCondition)
        {
            delete m_pAosCondition;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piOriginConfiguration);
    }
};

TEST_F(AosConditionTest, ShouldCreateAvailableCellularWhenStart)
{
    // GIVEN
    EXPECT_EQ(m_pAosCondition->m_pAvailableCellular, nullptr);

    // WHEN
    m_pAosCondition->Start();

    // THEN
    EXPECT_NE(m_pAosCondition->m_pAvailableCellular, nullptr);
}

TEST_F(AosConditionTest, ShouldCreateAvailableWifiWhenStart)
{
    // GIVEN
    EXPECT_EQ(m_pAosCondition->m_pAvailableWifi, nullptr);

    // WHEN
    m_pAosCondition->Start();

    // THEN
    EXPECT_NE(m_pAosCondition->m_pAvailableWifi, nullptr);
}

TEST_F(AosConditionTest, ShouldSetBlockWithAosIncompletedWhenStart)
{
    // GIVEN
    EXPECT_FALSE(m_pAosCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));

    // WHEN
    m_pAosCondition->Start();

    // THEN
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
}

TEST_F(AosConditionTest, ShouldSetBlockWithSubscriberIncompletedWhenStart)
{
    // GIVEN
    EXPECT_FALSE(m_pAosCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));

    // WHEN
    m_pAosCondition->Start();

    // THEN
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));
}

TEST_F(AosConditionTest, ShouldDeleteAvailableCellularWhenStop)
{
    // GIVEN
    m_pAosCondition->m_pAvailableCellular = new AosServiceAvailableCellular();
    EXPECT_NE(m_pAosCondition->m_pAvailableCellular, nullptr);

    // WHEN
    m_pAosCondition->Stop();

    // THEN
    EXPECT_EQ(m_pAosCondition->m_pAvailableCellular, nullptr);
}

TEST_F(AosConditionTest, ShouldDeleteAvailableWifiWhenStop)
{
    // GIVEN
    m_pAosCondition->m_pAvailableWifi = new AosServiceAvailableWifi();
    EXPECT_NE(m_pAosCondition->m_pAvailableWifi, nullptr);

    // WHEN
    m_pAosCondition->Stop();

    // THEN
    EXPECT_EQ(m_pAosCondition->m_pAvailableWifi, nullptr);
}

TEST_F(AosConditionTest, SetListener)
{
    EXPECT_EQ(m_pAosCondition->m_piListener, nullptr);

    IAosConditionListener* piAosConditionListener = new MockIAosConditionListener();
    m_pAosCondition->SetListener(piAosConditionListener);
    EXPECT_EQ(m_pAosCondition->m_piListener, piAosConditionListener);
}

TEST_F(AosConditionTest, SetBlock)
{
    ASSERT_TRUE(m_pAosBlock->IsCleared());

    m_pAosCondition->SetBlock(BLOCK_AC_INCOMPLETED);
    m_pAosCondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);
    m_pAosCondition->SetBlock(BLOCK_AOS_INCOMPLETED);
    m_pAosCondition->SetBlock(BLOCK_CSCALL_STARTED);
    m_pAosCondition->SetBlock(BLOCK_PERMANENT_DATA_FAILED);
    m_pAosCondition->SetBlock(BLOCK_ENABLER_DETACHED);
    m_pAosCondition->SetBlock(BLOCK_IMS_DISABLED);
    m_pAosCondition->SetBlock(BLOCK_PERMANENT_REG_FAILED);
    m_pAosCondition->SetBlock(BLOCK_POWER_OFF);
    m_pAosCondition->SetBlock(BLOCK_SERVICE_CONNECTING);
    m_pAosCondition->SetBlock(BLOCK_SUBSCRIBER_INCOMPLETED);
    m_pAosCondition->SetBlock(BLOCK_TTY_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_ROAMING);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_VOPS_OFF);
    m_pAosCondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosCondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosCondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_WIFI_NO_WIFI);
    m_pAosCondition->SetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);

    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_AC_INCOMPLETED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CSCALL_STARTED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_ENABLER_DETACHED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_IMS_DISABLED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_POWER_OFF));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_SERVICE_CONNECTING));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_TTY_MODE_ON));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_TEMPORARY_DATA_DEACTIVATED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_AIRPLANE_MODE_ON));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_NO_NETWORK));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_OUT_OF_SERVICE));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_ROAMING));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_VOPS_OFF));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_BAD_CONNECTION));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_AIRPLANE_MODE_ON));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_NO_WIFI));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_TEMPORARILY_BLOCKED));
}

TEST_F(AosConditionTest, ResetBlock)
{
    ASSERT_TRUE(m_pAosBlock->IsCleared());

    m_pAosCondition->SetBlock(BLOCK_AC_INCOMPLETED);
    m_pAosCondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);
    m_pAosCondition->SetBlock(BLOCK_AOS_INCOMPLETED);
    m_pAosCondition->SetBlock(BLOCK_CSCALL_STARTED);
    m_pAosCondition->SetBlock(BLOCK_PERMANENT_DATA_FAILED);
    m_pAosCondition->SetBlock(BLOCK_ENABLER_DETACHED);
    m_pAosCondition->SetBlock(BLOCK_IMS_DISABLED);
    m_pAosCondition->SetBlock(BLOCK_PERMANENT_REG_FAILED);
    m_pAosCondition->SetBlock(BLOCK_POWER_OFF);
    m_pAosCondition->SetBlock(BLOCK_SERVICE_CONNECTING);
    m_pAosCondition->SetBlock(BLOCK_SUBSCRIBER_INCOMPLETED);
    m_pAosCondition->SetBlock(BLOCK_TTY_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_ROAMING);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_VOPS_OFF);
    m_pAosCondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosCondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosCondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_WIFI_NO_WIFI);
    m_pAosCondition->SetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);

    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_AC_INCOMPLETED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CSCALL_STARTED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_ENABLER_DETACHED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_IMS_DISABLED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_POWER_OFF));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_SERVICE_CONNECTING));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_TTY_MODE_ON));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_TEMPORARY_DATA_DEACTIVATED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_AIRPLANE_MODE_ON));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_NO_NETWORK));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_OUT_OF_SERVICE));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_ROAMING));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_VOPS_OFF));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_BAD_CONNECTION));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_AIRPLANE_MODE_ON));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_NO_WIFI));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_TEMPORARILY_BLOCKED));

    m_pAosCondition->ResetBlock(BLOCK_AC_INCOMPLETED);
    m_pAosCondition->ResetBlock(BLOCK_AUTHENTICATION_FAILED);
    m_pAosCondition->ResetBlock(BLOCK_AOS_INCOMPLETED);
    m_pAosCondition->ResetBlock(BLOCK_CSCALL_STARTED);
    m_pAosCondition->ResetBlock(BLOCK_PERMANENT_DATA_FAILED);
    m_pAosCondition->ResetBlock(BLOCK_ENABLER_DETACHED);
    m_pAosCondition->ResetBlock(BLOCK_IMS_DISABLED);
    m_pAosCondition->ResetBlock(BLOCK_PERMANENT_REG_FAILED);
    m_pAosCondition->ResetBlock(BLOCK_POWER_OFF);
    m_pAosCondition->ResetBlock(BLOCK_SERVICE_CONNECTING);
    m_pAosCondition->ResetBlock(BLOCK_SUBSCRIBER_INCOMPLETED);
    m_pAosCondition->ResetBlock(BLOCK_TTY_MODE_ON);
    m_pAosCondition->ResetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    m_pAosCondition->ResetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosCondition->ResetBlock(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosCondition->ResetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    m_pAosCondition->ResetBlock(BLOCK_CELLULAR_ROAMING);
    m_pAosCondition->ResetBlock(BLOCK_CELLULAR_VOPS_OFF);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_NO_WIFI);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);

    EXPECT_TRUE(m_pAosBlock->IsCleared());
}

TEST_F(AosConditionTest, IsReasonBlocked)
{
    ASSERT_TRUE(m_pAosBlock->IsCleared());

    m_pAosCondition->SetBlock(BLOCK_AC_INCOMPLETED);
    m_pAosCondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);
    m_pAosCondition->SetBlock(BLOCK_AOS_INCOMPLETED);
    m_pAosCondition->SetBlock(BLOCK_CSCALL_STARTED);
    m_pAosCondition->SetBlock(BLOCK_PERMANENT_DATA_FAILED);
    m_pAosCondition->SetBlock(BLOCK_ENABLER_DETACHED);
    m_pAosCondition->SetBlock(BLOCK_IMS_DISABLED);
    m_pAosCondition->SetBlock(BLOCK_PERMANENT_REG_FAILED);
    m_pAosCondition->SetBlock(BLOCK_POWER_OFF);
    m_pAosCondition->SetBlock(BLOCK_SERVICE_CONNECTING);
    m_pAosCondition->SetBlock(BLOCK_SUBSCRIBER_INCOMPLETED);
    m_pAosCondition->SetBlock(BLOCK_TTY_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_NO_NETWORK);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_ROAMING);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_VOPS_OFF);
    m_pAosCondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosCondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosCondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_WIFI_NO_WIFI);
    m_pAosCondition->SetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);

    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_AC_INCOMPLETED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CSCALL_STARTED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_ENABLER_DETACHED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_IMS_DISABLED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_POWER_OFF));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_SERVICE_CONNECTING));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_TTY_MODE_ON));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_TEMPORARY_DATA_DEACTIVATED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_AIRPLANE_MODE_ON));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_NO_NETWORK));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_OUT_OF_SERVICE));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_ROAMING));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_VOPS_OFF));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_BAD_CONNECTION));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_AIRPLANE_MODE_ON));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_NO_WIFI));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_TEMPORARILY_BLOCKED));
}

TEST_F(AosConditionTest, ReturnTrueWhenCellularServiceIsReady)
{
    // GIVEN
    m_pAosCondition->m_eServiceType = SERVICE_CELLULAR;
    m_pAosCondition->m_bCellServiceAvailable = IMS_TRUE;
    m_pAosCondition->m_bWifiServiceAvailable = IMS_FALSE;

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosConditionTest, ReturnTrueWhenWifiServiceIsReady)
{
    // GIVEN
    m_pAosCondition->m_eServiceType = SERVICE_WIFI;
    m_pAosCondition->m_bCellServiceAvailable = IMS_FALSE;
    m_pAosCondition->m_bWifiServiceAvailable = IMS_TRUE;

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosConditionTest, ReturnTrueWhenWholeServiceIsReady)
{
    // GIVEN
    m_pAosCondition->m_eServiceType = SERVICE_WHOLE;
    m_pAosCondition->m_bCellServiceAvailable = IMS_TRUE;
    m_pAosCondition->m_bWifiServiceAvailable = IMS_TRUE;

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosConditionTest, ReturnTrueWhenWholeServiceIsReadyByCellular)
{
    // GIVEN
    m_pAosCondition->m_eServiceType = SERVICE_WHOLE;
    m_pAosCondition->m_bCellServiceAvailable = IMS_TRUE;
    m_pAosCondition->m_bWifiServiceAvailable = IMS_FALSE;

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosConditionTest, ReturnTrueWhenWholeServiceIsReadyByWifi)
{
    // GIVEN
    m_pAosCondition->m_eServiceType = SERVICE_WHOLE;
    m_pAosCondition->m_bCellServiceAvailable = IMS_FALSE;
    m_pAosCondition->m_bWifiServiceAvailable = IMS_TRUE;

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosConditionTest, ReturnFalseWhenCellularServiceIsNotReady)
{
    // GIVEN
    m_pAosCondition->m_eServiceType = SERVICE_CELLULAR;
    m_pAosCondition->m_bCellServiceAvailable = IMS_FALSE;
    m_pAosCondition->m_bWifiServiceAvailable = IMS_TRUE;

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosConditionTest, ReturnFalseWhenWifiServiceIsNotReady)
{
    // GIVEN
    m_pAosCondition->m_eServiceType = SERVICE_WIFI;
    m_pAosCondition->m_bCellServiceAvailable = IMS_TRUE;
    m_pAosCondition->m_bWifiServiceAvailable = IMS_FALSE;

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosConditionTest, ReturnFalseWhenWholeServiceIsNotReady)
{
    // GIVEN
    m_pAosCondition->m_eServiceType = SERVICE_WHOLE;
    m_pAosCondition->m_bCellServiceAvailable = IMS_FALSE;
    m_pAosCondition->m_bWifiServiceAvailable = IMS_FALSE;

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosConditionTest, CheckServiceAvailable)
{
    EXPECT_EQ(m_pAosCondition->m_pAvailableCellular, nullptr);
    EXPECT_EQ(m_pAosCondition->m_pAvailableWifi, nullptr);
    EXPECT_EQ(m_pAosCondition->CheckServiceAvailable(SERVICE_WHOLE), AosCondition::CHECK_NONE);
    EXPECT_EQ(m_pAosCondition->CheckServiceAvailable(SERVICE_CELLULAR), AosCondition::CHECK_NONE);
    EXPECT_EQ(m_pAosCondition->CheckServiceAvailable(SERVICE_WIFI), AosCondition::CHECK_NONE);

    m_pAosCondition->Start();
    EXPECT_NE(m_pAosCondition->m_pAvailableCellular, nullptr);
    EXPECT_NE(m_pAosCondition->m_pAvailableWifi, nullptr);
    EXPECT_EQ(m_pAosCondition->CheckServiceAvailable(SERVICE_WHOLE),
            AosCondition::CHECK_CELLULAR | AosCondition::CHECK_WIFI);
    EXPECT_EQ(
            m_pAosCondition->CheckServiceAvailable(SERVICE_CELLULAR), AosCondition::CHECK_CELLULAR);
    EXPECT_EQ(m_pAosCondition->CheckServiceAvailable(SERVICE_WIFI), AosCondition::CHECK_WIFI);
}

TEST_F(AosConditionTest, CheckBadNetwork)
{
    EXPECT_EQ(m_pAosCondition->m_pAvailableCellular, nullptr);
    EXPECT_EQ(m_pAosCondition->m_pAvailableWifi, nullptr);
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WIFI));

    m_pAosCondition->Start();
    EXPECT_NE(m_pAosCondition->m_pAvailableCellular, nullptr);
    EXPECT_NE(m_pAosCondition->m_pAvailableWifi, nullptr);
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WIFI));

    m_pAosCondition->m_bCellServiceAvailable = IMS_TRUE;
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_TRUE(m_pAosCondition->CheckBadNetwork(SERVICE_WIFI));

    m_pAosCondition->m_bCellServiceAvailable = IMS_FALSE;
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WIFI));
}

TEST_F(AosConditionTest, Event_NotifyEvent_RoamingState)
{
    m_pAosCondition->Start();

    m_pAosCondition->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_OFF);
    EXPECT_FALSE(m_pAosCondition->IsRoaming());

    m_pAosCondition->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_ON, IMS_ROAMING_STATE_OFF);
    EXPECT_TRUE(m_pAosCondition->IsRoaming());

    m_pAosCondition->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_OFF);
    EXPECT_FALSE(m_pAosCondition->IsRoaming());

    m_pAosCondition->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_ON);
    EXPECT_TRUE(m_pAosCondition->IsRoaming());
}

TEST_F(AosConditionTest, Event_NotifyEvent_VopsState)
{
    m_pAosCondition->Start();

    m_pAosCondition->Event_NotifyEvent(
            IMS_EVENT_IMS_VOICE_OVER_PS_STATE, IMS_VOICE_OVER_PS_SUPPORTED, 0);
    EXPECT_TRUE(m_pAosCondition->IsVopsSupported());

    m_pAosCondition->Event_NotifyEvent(
            IMS_EVENT_IMS_VOICE_OVER_PS_STATE, IMS_VOICE_OVER_PS_NOT_SUPPORTED, 0);
    EXPECT_FALSE(m_pAosCondition->IsVopsSupported());
}

TEST_F(AosConditionTest, Event_NotifyEvent_LteInfo)
{
    m_pAosCondition->Start();

    m_pAosCondition->Event_NotifyEvent(IMS_EVENT_LTE_INFO, IMS_LTE_INFO_EPS_ONLY_ATTACHED, 0);
    EXPECT_FALSE(m_pAosCondition->m_bIsCombinedAttached);

    m_pAosCondition->Event_NotifyEvent(IMS_EVENT_LTE_INFO, IMS_LTE_INFO_COMBINED_ATTACHED, 0);
    EXPECT_TRUE(m_pAosCondition->m_bIsCombinedAttached);
}

TEST_F(AosConditionTest, CallTracker_StateChanged_Cs_Offhook)
{
    m_pAosCondition->Start();

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
}

TEST_F(AosConditionTest, CallTracker_StateChanged_Cs_NotOffhook)
{
    m_pAosCondition->Start();

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::IDLE);
}

TEST_F(AosConditionTest, CallTracker_StateChanged_Normal_Offhook)
{
    m_pAosCondition->Start();

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK);
}

TEST_F(AosConditionTest, ResetBlockWhenNetTrackerStatusChangedWithServiceIn)
{
    // GIVEN
    ON_CALL(m_objMockIAosAppContext, GetNetTracker())
            .WillByDefault(Return(&m_objMockIAosNetTracker));

    ON_CALL(m_objMockIAosNetTracker, IsServiceIn(_)).WillByDefault(Return(IMS_TRUE));

    m_pAosCondition->Start();

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    m_pAosCondition->SetAosBlockToCellur(&objMockIAosBlock);

    // WHEN
    m_pAosCondition->NetTracker_StatusChanged();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, SetBlockWhenNetTrackerStatusChangedWithServiceOut)
{
    // GIVEN
    ON_CALL(m_objMockIAosAppContext, GetNetTracker())
            .WillByDefault(Return(&m_objMockIAosNetTracker));

    ON_CALL(m_objMockIAosNetTracker, IsServiceIn(_)).WillByDefault(Return(IMS_FALSE));

    m_pAosCondition->Start();

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlockToCellur(&objMockIAosBlock);

    // WHEN
    m_pAosCondition->NetTracker_StatusChanged();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, SetBlockWhenSubscriberStateChangedWithRefreshStarted)
{
    // GIVEN
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(1);

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    // WHEN
    m_pAosCondition->Subscriber_StateChanged(IAosSubscriber::REFRESH_STARTED);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, Block_Changed)
{
    m_pAosCondition->Start();
    m_pAosCondition->RemoveListener(AosCondition::LISTENER_ALL);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVoLteAvailable()).WillRepeatedly(Return(IMS_TRUE));

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    m_pAosCondition->SetAosBlockToCellur(&objMockIAosBlock);

    EXPECT_CALL(objMockIAosBlock, IsCleared(_)).Times(AnyNumber()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, PrintBlockReasons()).Times(1);

    m_pAosCondition->Block_Changed(0, 0);
}

TEST_F(AosConditionTest, Subscriber_StateChanged_RefreshCompleted_RefreshStartedFalse)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(2);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->m_bIsRefreshStarted = IMS_FALSE;
    EXPECT_FALSE(m_pAosCondition->IsRefreshStarted());

    m_pAosCondition->Subscriber_StateChanged(IAosSubscriber::REFRESH_COMPLETED);
}

TEST_F(AosConditionTest, Subscriber_StateChanged_RefreshCompleted_RefreshStartedTrue)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(3);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->m_bIsRefreshStarted = IMS_TRUE;
    EXPECT_TRUE(m_pAosCondition->IsRefreshStarted());

    m_pAosCondition->Subscriber_StateChanged(IAosSubscriber::REFRESH_COMPLETED);
}

TEST_F(AosConditionTest, Subscriber_StateChanged_RefreshFailed)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->Subscriber_StateChanged(IAosSubscriber::REFRESH_FAILED);
}

TEST_F(AosConditionTest, ServiceAvailable_RequestCommand_ListenerIsNotNull)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(1);

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    m_pAosCondition->ServiceAvailable_RequestCommand(AosCondition::REQUEST_PDN_DISCONNECT, 0);
}

TEST_F(AosConditionTest, NConfiguration_NotifyConfigChanged_NConfigIsNotNull)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsVoLteRoamingAvailable())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosCondition->NConfiguration_NotifyConfigChanged();
}

TEST_F(AosConditionTest, NConfiguration_NotifyConfigChanged_NConfigIsNull)
{
    // Set IAosNConfiguration
    AosProvider::GetInstance()->SetNConfiguration(IMS_NULL);

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVoLteRoamingAvailable()).Times(0);

    m_pAosCondition->NConfiguration_NotifyConfigChanged();
}

TEST_F(AosConditionTest, ServicePhone_AosStart)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->ServicePhone_AosStart();
}

TEST_F(AosConditionTest, ServicePhone_LocationInfoChanged_Changed)
{
    m_pAosCondition->Start();

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVoLteAvailable()).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, UseWfcCountryCodeAvailabilityCheck())
            .WillRepeatedly(Return(IMS_TRUE));

    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country_new");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    m_pAosCondition->SetTestLocation(&objMockILocationProperties);

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(AnyNumber());

    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(2);

    m_pAosCondition->SetAosBlockToWifi(&objMockIAosBlock);

    m_pAosCondition->ServicePhone_LocationInfoChanged(LocationInfo::CHANGED);
}

TEST_F(AosConditionTest, ServicePhone_LocationInfoChanged_NotChanged)
{
    m_pAosCondition->Start();

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVoLteAvailable()).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, UseWfcCountryCodeAvailabilityCheck())
            .WillRepeatedly(Return(IMS_TRUE));

    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country_new");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    m_pAosCondition->SetTestLocation(&objMockILocationProperties);

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(AnyNumber());

    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlockToWifi(&objMockIAosBlock);

    m_pAosCondition->ServicePhone_LocationInfoChanged(LocationInfo::AVAILABLE);
}

TEST_F(AosConditionTest, ServicePhone_LocationInfoChanged_ReturnByConfig)
{
    m_pAosCondition->Start();

    EXPECT_CALL(m_objMockIAosNConfiguration, IsVoLteAvailable()).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsWfcImsAvailable()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, UseWfcCountryCodeAvailabilityCheck())
            .WillRepeatedly(Return(IMS_FALSE));

    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country_new");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    m_pAosCondition->SetTestLocation(&objMockILocationProperties);

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(AnyNumber());

    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlockToWifi(&objMockIAosBlock);

    m_pAosCondition->ServicePhone_LocationInfoChanged(LocationInfo::CHANGED);
}

TEST_F(AosConditionTest, ServicePhone_PhoneNumberStateChanged_RetryFailure)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->ServicePhone_PhoneNumberStateChanged(
            IMS_FALSE, PhoneNumberState::RETRY_FAILURE);
}

TEST_F(AosConditionTest, ServicePhone_PhoneNumberStateChanged_ClearReasonSimState)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->ServicePhone_PhoneNumberStateChanged(
            IMS_FALSE, PhoneNumberState::RETRY_SUCCESS);
}

TEST_F(AosConditionTest, ServicePhone_PlmnChanged_ClearReaconPlmlChanged)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->ServicePhone_PlmnChanged();
}

TEST_F(AosConditionTest, ServicePhone_PowerOff_ListenerIsNull)
{
    m_pAosCondition->SetListener(IMS_NULL);

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->ServicePhone_PowerOff();
}

TEST_F(AosConditionTest, ServicePhone_PowerOff_ListenerIsNotNull)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(1);

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->ServicePhone_PowerOff();
}

TEST_F(AosConditionTest, ServiceSetting_AirplaneChanged_True_MatchedClearReason)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(1);

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(2);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->ServiceSetting_AirplaneChanged(IMS_TRUE);
}

TEST_F(AosConditionTest, ServiceSetting_AirplaneChanged_False)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(0);

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->ServiceSetting_AirplaneChanged(IMS_FALSE);
}

TEST_F(AosConditionTest, ServiceSetting_ServiceChanged_HoldEvent)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->AddHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
    EXPECT_TRUE(m_pAosCondition->IsHolded(TestAosCondition::HOLD_EVENT_IMS_SERVICE));

    m_pAosCondition->ServiceSetting_ServiceChanged(ServiceSetting::ON, 0);
    m_pAosCondition->ServiceSetting_ServiceChanged(ServiceSetting::OFF, 0);
}

TEST_F(AosConditionTest, ServiceSetting_ServiceChanged_On)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(3);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->RemoveHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
    EXPECT_FALSE(m_pAosCondition->IsHolded(TestAosCondition::HOLD_EVENT_IMS_SERVICE));

    m_pAosCondition->ServiceSetting_ServiceChanged(ServiceSetting::ON, 0);
}

TEST_F(AosConditionTest, ServiceSetting_ServiceChanged_Off)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->RemoveHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
    EXPECT_FALSE(m_pAosCondition->IsHolded(TestAosCondition::HOLD_EVENT_IMS_SERVICE));

    m_pAosCondition->ServiceSetting_ServiceChanged(ServiceSetting::OFF, 0);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_On_RttNotSupport)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRttSupported()).WillRepeatedly(Return(IMS_FALSE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->ServiceSetting_TtyChanged(IMS_TRUE);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_True_CombindAttached)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRttSupported()).WillRepeatedly(Return(IMS_TRUE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->m_bIsCombinedAttached = IMS_TRUE;
    m_pAosCondition->ServiceSetting_TtyChanged(IMS_TRUE);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_False)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRttSupported()).WillRepeatedly(Return(IMS_FALSE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->m_bIsCombinedAttached = IMS_TRUE;
    m_pAosCondition->ServiceSetting_TtyChanged(IMS_FALSE);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_TtyNotSupport)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_FALSE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->ServiceSetting_TtyChanged(IMS_TRUE);
    m_pAosCondition->ServiceSetting_TtyChanged(IMS_FALSE);
}

TEST_F(AosConditionTest, DisableNetTrackerListenerWhenConnectionTypeIsWifi)
{
    // GIVEN
    ON_CALL(m_objMockIAosAppContext, GetConnection())
            .WillByDefault(Return(&m_objMockIAosConnection));
    ON_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillByDefault(Return(NetworkPolicy::APN_WIFI));

    // WHEN
    m_pAosCondition->Init();

    // THEN
    EXPECT_FALSE(m_pAosCondition->IsListenerEnabled(AosCondition::LISTENER_NETTRACKER));
}

TEST_F(AosConditionTest, DisableNetTrackerListenerWhenConnectionTypeIsEmergency)
{
    // GIVEN
    ON_CALL(m_objMockIAosAppContext, GetConnection())
            .WillByDefault(Return(&m_objMockIAosConnection));
    ON_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillByDefault(Return(NetworkPolicy::APN_EMERGENCY));

    // WHEN
    m_pAosCondition->Init();

    // THEN
    EXPECT_FALSE(m_pAosCondition->IsListenerEnabled(AosCondition::LISTENER_NETTRACKER));
}

TEST_F(AosConditionTest, EnableNetTrackerListenerWhenConnectionTypeIsIms)
{
    // GIVEN
    ON_CALL(m_objMockIAosAppContext, GetConnection())
            .WillByDefault(Return(&m_objMockIAosConnection));
    ON_CALL(m_objMockIAosConnection, GetConnectionType())
            .WillByDefault(Return(NetworkPolicy::APN_IMS));

    // WHEN
    m_pAosCondition->Init();

    // THEN
    EXPECT_TRUE(m_pAosCondition->IsListenerEnabled(AosCondition::LISTENER_NETTRACKER));
}

TEST_F(AosConditionTest, AddListener)
{
    m_pAosCondition->RemoveListener(AosCondition::LISTENER_ALL);
    EXPECT_EQ(m_pAosCondition->m_nListeners, AosCondition::LISTENER_NONE);

    m_pAosCondition->AddListener(AosCondition::LISTENER_BLOCK);
    EXPECT_EQ(m_pAosCondition->m_nListeners, AosCondition::LISTENER_BLOCK);

    m_pAosCondition->AddListener(AosCondition::LISTENER_NETTRACKER);
    EXPECT_EQ(m_pAosCondition->m_nListeners,
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER);

    m_pAosCondition->AddListener(AosCondition::LISTENER_SUBSCRIBER);
    EXPECT_EQ(m_pAosCondition->m_nListeners,
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER);

    m_pAosCondition->AddListener(AosCondition::LISTENER_CALLTRACKER);
    EXPECT_EQ(m_pAosCondition->m_nListeners,
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER | AosCondition::LISTENER_CALLTRACKER);

    m_pAosCondition->RemoveListener(AosCondition::LISTENER_ALL);
    EXPECT_EQ(m_pAosCondition->m_nListeners, AosCondition::LISTENER_NONE);

    m_pAosCondition->AddListener(AosCondition::LISTENER_ALL);
    EXPECT_EQ(m_pAosCondition->m_nListeners, AosCondition::LISTENER_ALL);
}

TEST_F(AosConditionTest, RemoveListener)
{
    m_pAosCondition->RemoveListener(AosCondition::LISTENER_ALL);

    m_pAosCondition->AddListener(AosCondition::LISTENER_BLOCK);
    m_pAosCondition->AddListener(AosCondition::LISTENER_NETTRACKER);
    m_pAosCondition->AddListener(AosCondition::LISTENER_SUBSCRIBER);
    m_pAosCondition->AddListener(AosCondition::LISTENER_CALLTRACKER);

    EXPECT_EQ(m_pAosCondition->m_nListeners,
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER | AosCondition::LISTENER_CALLTRACKER);

    m_pAosCondition->RemoveListener(AosCondition::LISTENER_CALLTRACKER);
    EXPECT_EQ(m_pAosCondition->m_nListeners,
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER);

    m_pAosCondition->RemoveListener(AosCondition::LISTENER_SUBSCRIBER);
    EXPECT_EQ(m_pAosCondition->m_nListeners,
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER);

    m_pAosCondition->RemoveListener(AosCondition::LISTENER_NETTRACKER);
    EXPECT_EQ(m_pAosCondition->m_nListeners, AosCondition::LISTENER_BLOCK);

    m_pAosCondition->RemoveListener(AosCondition::LISTENER_BLOCK);
    EXPECT_EQ(m_pAosCondition->m_nListeners, AosCondition::LISTENER_NONE);
}

TEST_F(AosConditionTest, IsListenerEnabled)
{
    m_pAosCondition->RemoveListener(AosCondition::LISTENER_ALL);

    m_pAosCondition->AddListener(AosCondition::LISTENER_BLOCK);
    m_pAosCondition->AddListener(AosCondition::LISTENER_NETTRACKER);
    m_pAosCondition->AddListener(AosCondition::LISTENER_SUBSCRIBER);
    m_pAosCondition->AddListener(AosCondition::LISTENER_CALLTRACKER);

    EXPECT_TRUE(m_pAosCondition->IsListenerEnabled(AosCondition::LISTENER_BLOCK));
    EXPECT_TRUE(m_pAosCondition->IsListenerEnabled(AosCondition::LISTENER_NETTRACKER));
    EXPECT_TRUE(m_pAosCondition->IsListenerEnabled(AosCondition::LISTENER_SUBSCRIBER));
    EXPECT_TRUE(m_pAosCondition->IsListenerEnabled(AosCondition::LISTENER_CALLTRACKER));

    m_pAosCondition->RemoveListener(AosCondition::LISTENER_BLOCK);
    EXPECT_FALSE(m_pAosCondition->IsListenerEnabled(AosCondition::LISTENER_BLOCK));
}

TEST_F(AosConditionTest, AddHold_Roaming)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(BLOCK_CELLULAR_ROAMING, _)).Times(1);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->AddHold(TestAosCondition::HOLD_EVENT_ROAMING, IMS_TRUE);
}

TEST_F(AosConditionTest, AddHold_ImsService)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(BLOCK_IMS_DISABLED, _)).Times(1);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->AddHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_TRUE);
}

TEST_F(AosConditionTest, AddHold_IsNotEventReset)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->AddHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
}

TEST_F(AosConditionTest, AddHold_UninterestingEvent)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->AddHold(TestAosCondition::HOLD_EVENT_NONE, IMS_FALSE);
}

TEST_F(AosConditionTest, RemoveHold_Roaming)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(BLOCK_CELLULAR_ROAMING, _)).Times(1);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->RemoveHold(TestAosCondition::HOLD_EVENT_ROAMING, IMS_TRUE);
}

TEST_F(AosConditionTest, RemoveHold_ImsService)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(BLOCK_IMS_DISABLED, _)).Times(1);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->RemoveHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_TRUE);
}

TEST_F(AosConditionTest, RemoveHold_IsNotEventReset)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->RemoveHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
}

TEST_F(AosConditionTest, RemoveHold_UninterestingEvent)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    m_pAosCondition->RemoveHold(TestAosCondition::HOLD_EVENT_NONE, IMS_FALSE);
}

TEST_F(AosConditionTest, RequestCommand_ListenerIsNull)
{
    m_pAosCondition->SetListener(IMS_NULL);

    EXPECT_FALSE(m_pAosCondition->RequestCommand(AosCondition::REQUEST_STOP, 0));
}

TEST_F(AosConditionTest, RequestCommand_ListenerIsNotNull)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(1);

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    EXPECT_TRUE(m_pAosCondition->RequestCommand(AosCondition::REQUEST_STOP, 0));
}

TEST_F(AosConditionTest, UpdateRegistrationMode_ImpuCountIsGreaterThanOne)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_TRUE));
    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosSubscriber));

    EXPECT_CALL(m_objMockIAosSubscriber, IsReady()).WillRepeatedly(Return(IMS_TRUE));

    AStringArray objImpus;
    objImpus.AddElement(AString("PUID1"));
    objImpus.AddElement(AString("PUID2"));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillRepeatedly(ReturnRef(objImpus));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, IAosRegistration::MODE_NORMAL)).Times(1);

    m_pAosCondition->UpdateRegistrationMode();
}

TEST_F(AosConditionTest, UpdateRegistrationMode_ImpuCountIsOne)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_TRUE));
    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosSubscriber));

    EXPECT_CALL(m_objMockIAosSubscriber, IsReady()).WillRepeatedly(Return(IMS_TRUE));

    AStringArray objImpus;
    objImpus.AddElement(AString("PUID1"));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillRepeatedly(ReturnRef(objImpus));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, IAosRegistration::MODE_LIMITED)).Times(1);

    m_pAosCondition->UpdateRegistrationMode();
}

TEST_F(AosConditionTest, UpdateRegistrationMode_NoBlockReason)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_FALSE));
    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosSubscriber));

    EXPECT_CALL(m_objMockIAosSubscriber, IsReady()).WillRepeatedly(Return(IMS_TRUE));

    AStringArray objImpus;
    objImpus.AddElement(AString("PUID1"));
    objImpus.AddElement(AString("PUID2"));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillRepeatedly(ReturnRef(objImpus));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, _)).Times(0);

    m_pAosCondition->UpdateRegistrationMode();
}

TEST_F(AosConditionTest, UpdateRegistrationMode_IsNotReady)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_TRUE));
    m_pAosCondition->SetAosBlock(&objMockIAosBlock);

    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosSubscriber));

    EXPECT_CALL(m_objMockIAosSubscriber, IsReady()).WillRepeatedly(Return(IMS_FALSE));

    AStringArray objImpus;
    objImpus.AddElement(AString("PUID1"));
    objImpus.AddElement(AString("PUID2"));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillRepeatedly(ReturnRef(objImpus));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, _)).Times(0);

    m_pAosCondition->UpdateRegistrationMode();
}
