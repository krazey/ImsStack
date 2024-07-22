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
#include "../../interface/aos/MockIAosService.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosConditionListener.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosRegistration.h"
#include "interface/MockIAosSubscriber.h"

using ::testing::_;
using ::testing::An;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;
const AString PROFILE_ID = AString("test");

#define DECLARE_USING(Base)                           \
    using Base::AddAosServiceListener;                \
    using Base::RemoveAosServiceListener;             \
    using Base::Event_NotifyEvent;                    \
    using Base::CallTracker_StateChanged;             \
    using Base::NetTracker_StatusChanged;             \
    using Base::Subscriber_StateChanged;              \
    using Base::Block_Changed;                        \
    using Base::ServiceAvailable_RequestCommand;      \
    using Base::NConfiguration_NotifyConfigChanged;   \
    using Base::ServicePhone_AosStart;                \
    using Base::ServicePhone_LocationInfoChanged;     \
    using Base::ServicePhone_PhoneNumberStateChanged; \
    using Base::ServicePhone_PlmnChanged;             \
    using Base::ServicePhone_PowerOff;                \
    using Base::ServiceSetting_AirplaneChanged;       \
    using Base::ServiceSetting_ServiceChanged;        \
    using Base::ServiceSetting_TtyChanged;            \
    using Base::Init;                                 \
    using Base::AddListener;                          \
    using Base::RemoveListener;                       \
    using Base::IsListenerEnabled;                    \
    using Base::AddHold;                              \
    using Base::RemoveHold;                           \
    using Base::IsHeld;                               \
    using Base::IsRefreshStarted;                     \
    using Base::RequestCommand;                       \
    using Base::UpdateRegistrationMode;

class TestAosCondition : public AosCondition
{
public:
    DECLARE_USING(AosCondition)

    inline explicit TestAosCondition(IN IAosAppContext* piAppContext) :
            AosCondition(piAppContext)
    {
    }

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

    inline AosServiceAvailableCellular* GetAvailableCellular() { return m_pAvailableCellular; }

    inline void SetAvailableCellular(IN AosServiceAvailableCellular* pAvailableCellular)
    {
        m_pAvailableCellular = pAvailableCellular;
    }

    inline AosServiceAvailableWifi* GetAvailableWifi() { return m_pAvailableWifi; }

    inline void SetAvailableWifi(IN AosServiceAvailableWifi* pAvailableWifi)
    {
        m_pAvailableWifi = pAvailableWifi;
    }

    inline IAosConditionListener* GetListener() { return m_piListener; }

    inline void SetServiceType(IN SERVICE_TYPE eType) { m_eServiceType = eType; }

    inline void SetCellServiceAvailable(IN IMS_BOOL bIsAvailable)
    {
        m_bCellServiceAvailable = bIsAvailable;
    }

    inline void SetWifiServiceAvailable(IN IMS_BOOL bIsAvailable)
    {
        m_bWifiServiceAvailable = bIsAvailable;
    }

    inline IMS_BOOL IsCombinedAttached() { return m_bIsCombinedAttached; }

    inline void SetCombinedAttached(IN IMS_BOOL bIsCombined)
    {
        m_bIsCombinedAttached = bIsCombined;
    }

    inline void SetRefreshStarted(IN IMS_BOOL bIsRefreshStarted)
    {
        m_bIsRefreshStarted = bIsRefreshStarted;
    }

    inline IMS_UINT32 GetListeners() { return m_nListeners; }
};

class AosConditionTest : public ::testing::Test
{
public:
    TestAosCondition* m_pAosCondition;

    IAosNConfiguration* m_piAosNConfiguration;
    IAosService* m_piAosService;

    NiceMock<MockIAosAppContext> m_objMockIAosAppContext;
    NiceMock<MockIAosService> m_objMockIAosService;
    NiceMock<MockIAosConnection> m_objMockIAosConnection;
    NiceMock<MockIAosNetTracker> m_objMockIAosNetTracker;
    NiceMock<MockIAosSubscriber> m_objMockIAosSubscriber;
    NiceMock<MockIAosRegistration> m_objMockIAosRegistration;
    NiceMock<MockIAosNConfiguration> m_objMockIAosNConfiguration;
    NiceMock<MockIAosBlock> m_objMockIAosBlock;

protected:
    virtual void SetUp() override
    {
        ReplaceOriginWithMock();

        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));

        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(PROFILE_ID));

        ON_CALL(m_objMockIAosAppContext, GetConnection()).WillByDefault(ReturnNull());

        ON_CALL(m_objMockIAosAppContext, GetRegistration())
                .WillByDefault(Return(&m_objMockIAosRegistration));

        ON_CALL(m_objMockIAosAppContext, GetBlock()).WillByDefault(ReturnNull());

        ON_CALL(m_objMockIAosAppContext, GetNetTracker()).WillByDefault(ReturnNull());

        ON_CALL(m_objMockIAosAppContext, GetSubscriber()).WillByDefault(ReturnNull());

        m_pAosCondition = new TestAosCondition(&m_objMockIAosAppContext);
        ASSERT_TRUE(m_pAosCondition != nullptr);

        m_pAosCondition->SetAosBlock(&m_objMockIAosBlock);
    }

    virtual void TearDown() override
    {
        RestoreOriginInstance();

        if (m_pAosCondition)
        {
            delete m_pAosCondition;
        }
    }

    void ReplaceOriginWithMock()
    {
        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        m_piAosService = AosProvider::GetInstance()->GetService();

        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosNConfiguration);
        AosProvider::GetInstance()->SetService(&m_objMockIAosService);
    }

    void RestoreOriginInstance()
    {
        AosProvider::GetInstance()->SetService(m_piAosService);
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration);
    }
};

TEST_F(AosConditionTest, ShouldCreateAvailableCellularWhenStart)
{
    // GIVEN
    EXPECT_EQ(m_pAosCondition->GetAvailableCellular(), nullptr);

    // WHEN
    m_pAosCondition->Start();

    // THEN
    EXPECT_NE(m_pAosCondition->GetAvailableCellular(), nullptr);
}

TEST_F(AosConditionTest, ShouldCreateAvailableWifiWhenStart)
{
    // GIVEN
    EXPECT_EQ(m_pAosCondition->GetAvailableWifi(), nullptr);

    // WHEN
    m_pAosCondition->Start();

    // THEN
    EXPECT_NE(m_pAosCondition->GetAvailableWifi(), nullptr);
}

TEST_F(AosConditionTest, ShouldSetStartBlockWhenStart)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED, _));
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(BLOCK_AOS_INCOMPLETED, _));

    // WHEN
    m_pAosCondition->Start();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, ShouldDeleteAvailableCellularWhenStop)
{
    // GIVEN
    m_pAosCondition->SetAvailableCellular(new AosServiceAvailableCellular());

    EXPECT_NE(m_pAosCondition->GetAvailableCellular(), nullptr);

    // WHEN
    m_pAosCondition->Stop();

    // THEN
    EXPECT_EQ(m_pAosCondition->GetAvailableCellular(), nullptr);
}

TEST_F(AosConditionTest, ShouldDeleteAvailableWifiWhenStop)
{
    // GIVEN
    m_pAosCondition->SetAvailableWifi(new AosServiceAvailableWifi());

    EXPECT_NE(m_pAosCondition->GetAvailableWifi(), nullptr);

    // WHEN
    m_pAosCondition->Stop();

    // THEN
    EXPECT_EQ(m_pAosCondition->GetAvailableWifi(), nullptr);
}

TEST_F(AosConditionTest, SucceedsAddAosServiceListener)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosService, AddListener(An<IAosServicePhoneListener*>()));

    // WHEN
    m_pAosCondition->AddAosServiceListener();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, FailsAddAosServiceListenerWhenAosServiceIsNull)
{
    // GIVEN
    AosProvider::GetInstance()->SetService(IMS_NULL);

    EXPECT_CALL(m_objMockIAosService, AddListener(An<IAosServicePhoneListener*>())).Times(0);

    // WHEN
    m_pAosCondition->AddAosServiceListener();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, SucceedsRemoveAosServiceListener)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosService, RemoveListener(An<IAosServicePhoneListener*>()));

    // WHEN
    m_pAosCondition->RemoveAosServiceListener();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, FailsRemoveAosServiceListenerWhenAosServiceIsNull)
{
    // GIVEN
    AosProvider::GetInstance()->SetService(IMS_NULL);

    EXPECT_CALL(m_objMockIAosService, RemoveListener(An<IAosServicePhoneListener*>())).Times(0);

    // WHEN
    m_pAosCondition->RemoveAosServiceListener();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, SucceedsSetListener)
{
    // GIVEN
    EXPECT_EQ(m_pAosCondition->GetListener(), nullptr);

    MockIAosConditionListener objMockIAosConditionListener;

    // WHEN
    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    // THEN
    EXPECT_NE(m_pAosCondition->GetListener(), nullptr);
}

TEST_F(AosConditionTest, SucceedsSetBlock)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(BLOCK_AC_INCOMPLETED, _));
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON, _));
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _));

    // WHEN
    m_pAosCondition->SetBlock(BLOCK_AC_INCOMPLETED);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, SucceedsResetBlock)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_AC_INCOMPLETED, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_WIFI_BAD_CONNECTION, _));

    // WHEN
    m_pAosCondition->ResetBlock(BLOCK_AC_INCOMPLETED);
    m_pAosCondition->ResetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_BAD_CONNECTION);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, ReturnTrueWhenCellularServiceIsReady)
{
    // GIVEN
    m_pAosCondition->SetServiceType(SERVICE_CELLULAR);
    m_pAosCondition->SetCellServiceAvailable(IMS_TRUE);
    m_pAosCondition->SetWifiServiceAvailable(IMS_FALSE);

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosConditionTest, ReturnTrueWhenWifiServiceIsReady)
{
    // GIVEN
    m_pAosCondition->SetServiceType(SERVICE_WIFI);
    m_pAosCondition->SetCellServiceAvailable(IMS_FALSE);
    m_pAosCondition->SetWifiServiceAvailable(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosConditionTest, ReturnTrueWhenWholeServiceIsReady)
{
    // GIVEN
    m_pAosCondition->SetServiceType(SERVICE_WHOLE);
    m_pAosCondition->SetCellServiceAvailable(IMS_TRUE);
    m_pAosCondition->SetWifiServiceAvailable(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosConditionTest, ReturnTrueWhenWholeServiceIsReadyByCellular)
{
    // GIVEN
    m_pAosCondition->SetServiceType(SERVICE_WHOLE);
    m_pAosCondition->SetCellServiceAvailable(IMS_TRUE);
    m_pAosCondition->SetWifiServiceAvailable(IMS_FALSE);

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosConditionTest, ReturnTrueWhenWholeServiceIsReadyByWifi)
{
    // GIVEN
    m_pAosCondition->SetServiceType(SERVICE_WHOLE);
    m_pAosCondition->SetCellServiceAvailable(IMS_FALSE);
    m_pAosCondition->SetWifiServiceAvailable(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosConditionTest, ReturnFalseWhenCellularServiceIsNotReady)
{
    // GIVEN
    m_pAosCondition->SetServiceType(SERVICE_CELLULAR);
    m_pAosCondition->SetCellServiceAvailable(IMS_FALSE);
    m_pAosCondition->SetWifiServiceAvailable(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosConditionTest, ReturnFalseWhenWifiServiceIsNotReady)
{
    // GIVEN
    m_pAosCondition->SetServiceType(SERVICE_WIFI);
    m_pAosCondition->SetCellServiceAvailable(IMS_TRUE);
    m_pAosCondition->SetWifiServiceAvailable(IMS_FALSE);

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosConditionTest, ReturnFalseWhenWholeServiceIsNotReady)
{
    // GIVEN
    m_pAosCondition->SetServiceType(SERVICE_WHOLE);
    m_pAosCondition->SetCellServiceAvailable(IMS_FALSE);
    m_pAosCondition->SetWifiServiceAvailable(IMS_FALSE);

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->IsReady();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosConditionTest, ReturnsNoneWhenNoAvailableServiceCellular)
{
    // GIVEN
    EXPECT_EQ(m_pAosCondition->GetAvailableCellular(), nullptr);

    // WHEN
    IMS_UINT32 nResult = m_pAosCondition->CheckServiceAvailable(SERVICE_CELLULAR);

    // THEN
    EXPECT_EQ(nResult, AosCondition::CHECK_NONE);
}

TEST_F(AosConditionTest, ReturnsNoneWhenNoAvailableServiceWifi)
{
    // GIVEN
    EXPECT_EQ(m_pAosCondition->GetAvailableWifi(), nullptr);

    // WHEN
    IMS_UINT32 nResult = m_pAosCondition->CheckServiceAvailable(SERVICE_WIFI);

    // THEN
    EXPECT_EQ(nResult, AosCondition::CHECK_NONE);
}

TEST_F(AosConditionTest, ReturnsNoneWhenNoAvailableServiceWhole)
{
    // GIVEN
    EXPECT_EQ(m_pAosCondition->GetAvailableCellular(), nullptr);
    EXPECT_EQ(m_pAosCondition->GetAvailableWifi(), nullptr);

    // WHEN
    IMS_UINT32 nResult = m_pAosCondition->CheckServiceAvailable(SERVICE_WHOLE);

    // THEN
    EXPECT_EQ(nResult, AosCondition::CHECK_NONE);
}

TEST_F(AosConditionTest, ReturnsCellularWhenExistAvailableServiceCellular)
{
    // GIVEN
    m_pAosCondition->Start();

    // WHEN
    IMS_UINT32 nResult = m_pAosCondition->CheckServiceAvailable(SERVICE_CELLULAR);

    // THEN
    EXPECT_EQ(nResult, AosCondition::CHECK_CELLULAR);
}

TEST_F(AosConditionTest, ReturnsWifiWhenExistAvailableServiceWifi)
{
    // GIVEN
    m_pAosCondition->Start();

    // WHEN
    IMS_UINT32 nResult = m_pAosCondition->CheckServiceAvailable(SERVICE_WIFI);

    // THEN
    EXPECT_EQ(nResult, AosCondition::CHECK_WIFI);
}

TEST_F(AosConditionTest, ReturnsWholeWhenExistAvailableServiceWhole)
{
    // GIVEN
    m_pAosCondition->Start();

    // WHEN
    IMS_UINT32 nResult = m_pAosCondition->CheckServiceAvailable(SERVICE_WHOLE);

    // THEN
    EXPECT_EQ(nResult, AosCondition::CHECK_CELLULAR | AosCondition::CHECK_WIFI);
}

TEST_F(AosConditionTest, CheckBadNetworkReturnsFalseWhenAvailableWifiIsNull)
{
    // GIVEN
    EXPECT_EQ(m_pAosCondition->GetAvailableWifi(), nullptr);

    // WHEN
    IMS_BOOL bResult1 = m_pAosCondition->CheckBadNetwork(SERVICE_WHOLE);
    IMS_BOOL bResult2 = m_pAosCondition->CheckBadNetwork(SERVICE_CELLULAR);
    IMS_BOOL bResult3 = m_pAosCondition->CheckBadNetwork(SERVICE_WIFI);

    // THEN
    EXPECT_FALSE(bResult1);
    EXPECT_FALSE(bResult2);
    EXPECT_FALSE(bResult3);
}

TEST_F(AosConditionTest, CheckBadNetworkReturnsFalseWhenCellServiceIsNotAvailable)
{
    // GIVEN
    m_pAosCondition->Start();

    EXPECT_NE(m_pAosCondition->GetAvailableCellular(), nullptr);
    EXPECT_NE(m_pAosCondition->GetAvailableWifi(), nullptr);

    m_pAosCondition->SetCellServiceAvailable(IMS_FALSE);

    // WHEN
    IMS_BOOL bResult1 = m_pAosCondition->CheckBadNetwork(SERVICE_WHOLE);
    IMS_BOOL bResult2 = m_pAosCondition->CheckBadNetwork(SERVICE_CELLULAR);
    IMS_BOOL bResult3 = m_pAosCondition->CheckBadNetwork(SERVICE_WIFI);

    // THEN
    EXPECT_FALSE(bResult1);
    EXPECT_FALSE(bResult2);
    EXPECT_FALSE(bResult3);
}

TEST_F(AosConditionTest, CheckBadNetworkReturnsFalseWhenServiceTypeIsNotWifi)
{
    // WHEN
    m_pAosCondition->Start();

    EXPECT_NE(m_pAosCondition->GetAvailableCellular(), nullptr);
    EXPECT_NE(m_pAosCondition->GetAvailableWifi(), nullptr);

    m_pAosCondition->SetCellServiceAvailable(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult1 = m_pAosCondition->CheckBadNetwork(SERVICE_WHOLE);
    IMS_BOOL bResult2 = m_pAosCondition->CheckBadNetwork(SERVICE_CELLULAR);

    // THEN
    EXPECT_FALSE(bResult1);
    EXPECT_FALSE(bResult2);
}

TEST_F(AosConditionTest, CheckBadNetworkReturnsTrueWhenServiceTypeIsWifi)
{
    // WHEN
    m_pAosCondition->Start();

    EXPECT_NE(m_pAosCondition->GetAvailableCellular(), nullptr);
    EXPECT_NE(m_pAosCondition->GetAvailableWifi(), nullptr);

    m_pAosCondition->SetCellServiceAvailable(IMS_TRUE);

    // WHEN
    IMS_BOOL bResult = m_pAosCondition->CheckBadNetwork(SERVICE_WIFI);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosConditionTest, ShouldSetRoamingOffWhenNotifyEventRoamingOff)
{
    // GIVEN
    m_pAosCondition->Start();

    // WHEN
    m_pAosCondition->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_OFF);

    // THEN
    EXPECT_FALSE(m_pAosCondition->IsRoaming());
}

TEST_F(AosConditionTest, ShouldSetRoamingOnWhenNotifyEventRoamingWithPsRoamingOn)
{
    // GIVEN
    m_pAosCondition->Start();

    // WHEN
    m_pAosCondition->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_ON, IMS_ROAMING_STATE_OFF);

    // THWN
    EXPECT_TRUE(m_pAosCondition->IsRoaming());
}

TEST_F(AosConditionTest, ShouldSetRoamingOnWhenNotifyEventRoamingWithCsRoamingOn)
{
    // GIVEN
    m_pAosCondition->Start();

    // WHEN
    m_pAosCondition->Event_NotifyEvent(
            IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_ON);

    // THEN
    EXPECT_TRUE(m_pAosCondition->IsRoaming());
}

TEST_F(AosConditionTest, ShouldSetVopsOnWhenNotifyEventWithVopsSupported)
{
    // GIVEN
    m_pAosCondition->Start();

    // WHEN
    m_pAosCondition->Event_NotifyEvent(
            IMS_EVENT_IMS_VOICE_OVER_PS_STATE, IMS_VOICE_OVER_PS_SUPPORTED, 0);

    // THEN
    EXPECT_TRUE(m_pAosCondition->IsVopsSupported());
}

TEST_F(AosConditionTest, ShouldSetVopsOffWhenNotifyEventWithVopsNotSupported)
{
    // GIVEN
    m_pAosCondition->Start();

    // WHEN
    m_pAosCondition->Event_NotifyEvent(
            IMS_EVENT_IMS_VOICE_OVER_PS_STATE, IMS_VOICE_OVER_PS_NOT_SUPPORTED, 0);

    // WHEN
    EXPECT_FALSE(m_pAosCondition->IsVopsSupported());
}

TEST_F(AosConditionTest, ShouldResetCombinedAttachWhenNotifyEventWithoutCombinedAttached)
{
    // GIVEN
    m_pAosCondition->Start();

    // WHEN
    m_pAosCondition->Event_NotifyEvent(IMS_EVENT_LTE_INFO, IMS_LTE_INFO_EPS_ONLY_ATTACHED, 0);

    // THEN
    EXPECT_FALSE(m_pAosCondition->IsCombinedAttached());
}

TEST_F(AosConditionTest, ShouldSetCombinedAttachWhenNotifyEventWithCombinedAttached)
{
    // GIVEN
    m_pAosCondition->Start();

    // WHEN
    m_pAosCondition->Event_NotifyEvent(IMS_EVENT_LTE_INFO, IMS_LTE_INFO_COMBINED_ATTACHED, 0);

    // THEN
    EXPECT_TRUE(m_pAosCondition->IsCombinedAttached());
}

TEST_F(AosConditionTest, ShouldSetCsCallStartedBlockWhenStateChangedInCaseOfCsAndOffhook)
{
    // GIVEN
    m_pAosCondition->Start();

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pAosCondition->CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, ShouldResetCsCallStartedBlockWhenStateChangedInCaseOfCsAndNotOffhook)
{
    // GIVEN
    m_pAosCondition->Start();

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _));

    // WHEN
    m_pAosCondition->CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::IDLE);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, ShouldNotDoAnythingWhenStateChangedInCaseOfNotCs)
{
    // GIVEN
    m_pAosCondition->Start();

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    // WHEN
    m_pAosCondition->CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, ResetBlockWhenNetTrackerStatusChangedWithServiceIn)
{
    // GIVEN
    ON_CALL(m_objMockIAosAppContext, GetNetTracker())
            .WillByDefault(Return(&m_objMockIAosNetTracker));

    ON_CALL(m_objMockIAosNetTracker, IsServiceIn(_)).WillByDefault(Return(IMS_TRUE));

    m_pAosCondition->Start();

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _));

    m_pAosCondition->SetAosBlockToCellur(&m_objMockIAosBlock);

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

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlockToCellur(&m_objMockIAosBlock);

    // WHEN
    m_pAosCondition->NetTracker_StatusChanged();

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosConditionTest, SetBlockWhenSubscriberStateChangedWithRefreshStarted)
{
    // GIVEN
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _));

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

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
    m_pAosCondition->SetAosBlockToCellur(&m_objMockIAosBlock);

    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosBlock, GetBlockReasons(_, _));
    EXPECT_CALL(m_objMockIAosBlock, PrintBlockReasons());

    m_pAosCondition->Block_Changed(0, 0);
}

TEST_F(AosConditionTest, ShouldResetBlockSubscriberIncompletedWhenReceiveRefreshCompleted)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED, _));

    // WHEN
    m_pAosCondition->Subscriber_StateChanged(IAosSubscriber::REFRESH_COMPLETED);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosConditionTest, ShouldResetBlockSubscriberIncompletedWhenReceiveReady)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED, _));

    // WHEN
    m_pAosCondition->Subscriber_StateChanged(IAosSubscriber::READY);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosConditionTest, ShouldResetBlocksWhenReceiveRefreshCompletedAfterRefreshStarted)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_PERMANENT_REG_FAILED, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_AUTHENTICATION_FAILED, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_PERMANENT_DATA_FAILED, _));

    m_pAosCondition->SetRefreshStarted(IMS_TRUE);

    // WHEN
    m_pAosCondition->Subscriber_StateChanged(IAosSubscriber::REFRESH_COMPLETED);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosConditionTest, ShouldResetBlocksWhenReceiveReadyAfterRefreshStarted)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_PERMANENT_REG_FAILED, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_AUTHENTICATION_FAILED, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_PERMANENT_DATA_FAILED, _));

    m_pAosCondition->SetRefreshStarted(IMS_TRUE);

    // WHEN
    m_pAosCondition->Subscriber_StateChanged(IAosSubscriber::REFRESH_COMPLETED);

    // THEN: The GIVEN condition should be met.
}

TEST_F(AosConditionTest, ShouldResetRefreshStartedValueWhenReceiveRefreshCompleted)
{
    // GIVEN
    m_pAosCondition->SetRefreshStarted(IMS_TRUE);

    // WHEN
    m_pAosCondition->Subscriber_StateChanged(IAosSubscriber::REFRESH_COMPLETED);

    // THEN
    EXPECT_FALSE(m_pAosCondition->IsRefreshStarted());
}

TEST_F(AosConditionTest, Subscriber_StateChanged_RefreshFailed)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->Subscriber_StateChanged(IAosSubscriber::REFRESH_FAILED);
}

TEST_F(AosConditionTest, ServiceAvailable_RequestCommand_ListenerIsNotNull)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _));

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    m_pAosCondition->ServiceAvailable_RequestCommand(AosCondition::REQUEST_PDN_DISCONNECT, 0);
}

TEST_F(AosConditionTest, NConfiguration_NotifyConfigChanged_NConfigIsNotNull)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsVoLteRoamingAvailable())
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
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _));

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
    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosBlock, GetBlockReasons(_, _)).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(2);

    m_pAosCondition->SetAosBlockToWifi(&m_objMockIAosBlock);

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
    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosBlock, GetBlockReasons(_, _)).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlockToWifi(&m_objMockIAosBlock);

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
    EXPECT_CALL(m_objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosBlock, GetBlockReasons(_, _)).Times(AnyNumber());

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetAosBlockToWifi(&m_objMockIAosBlock);

    m_pAosCondition->ServicePhone_LocationInfoChanged(LocationInfo::CHANGED);
}

TEST_F(AosConditionTest, ServicePhone_PhoneNumberStateChanged_RetryFailure)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->ServicePhone_PhoneNumberStateChanged(
            IMS_FALSE, PhoneNumberState::RETRY_FAILURE);
}

TEST_F(AosConditionTest, ServicePhone_PhoneNumberStateChanged_ClearReasonSimState)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _));

    m_pAosCondition->ServicePhone_PhoneNumberStateChanged(
            IMS_FALSE, PhoneNumberState::RETRY_SUCCESS);
}

TEST_F(AosConditionTest, ServicePhone_PlmnChanged_ClearReaconPlmlChanged)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _));

    m_pAosCondition->ServicePhone_PlmnChanged();
}

TEST_F(AosConditionTest, ServicePhone_PowerOff_ListenerIsNull)
{
    m_pAosCondition->SetListener(IMS_NULL);

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->ServicePhone_PowerOff();
}

TEST_F(AosConditionTest, ServicePhone_PowerOff_ListenerIsNotNull)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _));

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->ServicePhone_PowerOff();
}

TEST_F(AosConditionTest, ServiceSetting_AirplaneChanged_True_MatchedClearReason)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _));

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(3);

    m_pAosCondition->ServiceSetting_AirplaneChanged(IMS_TRUE);
}

TEST_F(AosConditionTest, ServiceSetting_AirplaneChanged_False)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(0);

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->ServiceSetting_AirplaneChanged(IMS_FALSE);
}

TEST_F(AosConditionTest, ServiceSetting_ServiceChanged_HoldEvent)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->AddHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
    EXPECT_TRUE(m_pAosCondition->IsHeld(TestAosCondition::HOLD_EVENT_IMS_SERVICE));

    m_pAosCondition->ServiceSetting_ServiceChanged(ServiceSetting::ON, 0);
    m_pAosCondition->ServiceSetting_ServiceChanged(ServiceSetting::OFF, 0);
}

TEST_F(AosConditionTest, ServiceSetting_ServiceChanged_On)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(4);

    m_pAosCondition->RemoveHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
    EXPECT_FALSE(m_pAosCondition->IsHeld(TestAosCondition::HOLD_EVENT_IMS_SERVICE));

    m_pAosCondition->ServiceSetting_ServiceChanged(ServiceSetting::ON, 0);
}

TEST_F(AosConditionTest, ServiceSetting_ServiceChanged_Off)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->RemoveHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
    EXPECT_FALSE(m_pAosCondition->IsHeld(TestAosCondition::HOLD_EVENT_IMS_SERVICE));

    m_pAosCondition->ServiceSetting_ServiceChanged(ServiceSetting::OFF, 0);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_On_RttNotSupport)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRttSupported()).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->ServiceSetting_TtyChanged(IMS_TRUE);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_True_CombindAttached)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRttSupported()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _));
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->SetCombinedAttached(IMS_TRUE);
    m_pAosCondition->ServiceSetting_TtyChanged(IMS_TRUE);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_False)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosNConfiguration, IsRttSupported()).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _));

    m_pAosCondition->SetCombinedAttached(IMS_TRUE);
    m_pAosCondition->ServiceSetting_TtyChanged(IMS_FALSE);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_TtyNotSupport)
{
    EXPECT_CALL(m_objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

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
    EXPECT_EQ(m_pAosCondition->GetListeners(), AosCondition::LISTENER_NONE);

    m_pAosCondition->AddListener(AosCondition::LISTENER_BLOCK);
    EXPECT_EQ(m_pAosCondition->GetListeners(), AosCondition::LISTENER_BLOCK);

    m_pAosCondition->AddListener(AosCondition::LISTENER_NETTRACKER);
    EXPECT_EQ(m_pAosCondition->GetListeners(),
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER);

    m_pAosCondition->AddListener(AosCondition::LISTENER_SUBSCRIBER);
    EXPECT_EQ(m_pAosCondition->GetListeners(),
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER);

    m_pAosCondition->AddListener(AosCondition::LISTENER_CALLTRACKER);
    EXPECT_EQ(m_pAosCondition->GetListeners(),
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER | AosCondition::LISTENER_CALLTRACKER);

    m_pAosCondition->RemoveListener(AosCondition::LISTENER_ALL);
    EXPECT_EQ(m_pAosCondition->GetListeners(), AosCondition::LISTENER_NONE);

    m_pAosCondition->AddListener(AosCondition::LISTENER_ALL);
    EXPECT_EQ(m_pAosCondition->GetListeners(), AosCondition::LISTENER_ALL);
}

TEST_F(AosConditionTest, RemoveListener)
{
    m_pAosCondition->RemoveListener(AosCondition::LISTENER_ALL);

    m_pAosCondition->AddListener(AosCondition::LISTENER_BLOCK);
    m_pAosCondition->AddListener(AosCondition::LISTENER_NETTRACKER);
    m_pAosCondition->AddListener(AosCondition::LISTENER_SUBSCRIBER);
    m_pAosCondition->AddListener(AosCondition::LISTENER_CALLTRACKER);

    EXPECT_EQ(m_pAosCondition->GetListeners(),
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER | AosCondition::LISTENER_CALLTRACKER);

    m_pAosCondition->RemoveListener(AosCondition::LISTENER_CALLTRACKER);
    EXPECT_EQ(m_pAosCondition->GetListeners(),
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER);

    m_pAosCondition->RemoveListener(AosCondition::LISTENER_SUBSCRIBER);
    EXPECT_EQ(m_pAosCondition->GetListeners(),
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER);

    m_pAosCondition->RemoveListener(AosCondition::LISTENER_NETTRACKER);
    EXPECT_EQ(m_pAosCondition->GetListeners(), AosCondition::LISTENER_BLOCK);

    m_pAosCondition->RemoveListener(AosCondition::LISTENER_BLOCK);
    EXPECT_EQ(m_pAosCondition->GetListeners(), AosCondition::LISTENER_NONE);
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
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_CELLULAR_ROAMING, _));

    m_pAosCondition->AddHold(TestAosCondition::HOLD_EVENT_ROAMING, IMS_TRUE);
}

TEST_F(AosConditionTest, AddHold_ImsService)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_IMS_DISABLED, _));

    m_pAosCondition->AddHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_TRUE);
}

TEST_F(AosConditionTest, AddHold_IsNotEventReset)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->AddHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
}

TEST_F(AosConditionTest, AddHold_UninterestingEvent)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->AddHold(TestAosCondition::HOLD_EVENT_NONE, IMS_FALSE);
}

TEST_F(AosConditionTest, RemoveHold_Roaming)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_CELLULAR_ROAMING, _));

    m_pAosCondition->RemoveHold(TestAosCondition::HOLD_EVENT_ROAMING, IMS_TRUE);
}

TEST_F(AosConditionTest, RemoveHold_ImsService)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(BLOCK_IMS_DISABLED, _));

    m_pAosCondition->RemoveHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_TRUE);
}

TEST_F(AosConditionTest, RemoveHold_IsNotEventReset)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    m_pAosCondition->RemoveHold(TestAosCondition::HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
}

TEST_F(AosConditionTest, RemoveHold_UninterestingEvent)
{
    EXPECT_CALL(m_objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(m_objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

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
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _));

    m_pAosCondition->SetListener(&objMockIAosConditionListener);

    EXPECT_TRUE(m_pAosCondition->RequestCommand(AosCondition::REQUEST_STOP, 0));
}

TEST_F(AosConditionTest, UpdateRegistrationMode_ImpuCountIsGreaterThanOne)
{
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosSubscriber));

    EXPECT_CALL(m_objMockIAosSubscriber, IsReady()).WillRepeatedly(Return(IMS_TRUE));

    AStringArray objImpus;
    objImpus.AddElement(AString("PUID1"));
    objImpus.AddElement(AString("PUID2"));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillRepeatedly(ReturnRef(objImpus));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, IAosRegistration::MODE_NORMAL));

    m_pAosCondition->UpdateRegistrationMode();
}

TEST_F(AosConditionTest, UpdateRegistrationMode_ImpuCountIsOne)
{
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosSubscriber));

    EXPECT_CALL(m_objMockIAosSubscriber, IsReady()).WillRepeatedly(Return(IMS_TRUE));

    AStringArray objImpus;
    objImpus.AddElement(AString("PUID1"));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillRepeatedly(ReturnRef(objImpus));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, IAosRegistration::MODE_LIMITED));

    m_pAosCondition->UpdateRegistrationMode();
}

TEST_F(AosConditionTest, UpdateRegistrationMode_NoBlockReason)
{
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_FALSE));

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
    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_TRUE));

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
