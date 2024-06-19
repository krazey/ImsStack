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

#include "external/AosService.h"
#include "interface/IAosEmergencyListener.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosRegistrationControlListener.h"
#include "interface/IAosServiceSettingListener.h"
#include "interface/IAosServicePhoneListener.h"

#include "interface/MockIAosEmergencyListener.h"
#include "interface/MockIAosRegistrationControlListener.h"
#include "interface/MockIAosServiceSettingListener.h"
#include "interface/MockIAosServicePhoneListener.h"

using ::testing::_;

const IMS_SINT32 SLOT_ID = 0;

#define DECLARE_USING(Base)     \
    using Base::IsTimerRunning; \
    using Base::ProcessPlmnChangeDelayTimerExpired;

class TestAosService : public AosService
{
public:
    DECLARE_USING(AosService)

    inline explicit TestAosService(IN IMS_SINT32 nSlotId) :
            AosService(nSlotId)
    {
    }

    inline ImsList<IAosEmergencyListener*> GetEmergencyListeners()
    {
        return m_objAosEmergencyListeners;
    }

    inline ImsList<IAosRegistrationControlListener*> GetRegistrationControlListener()
    {
        return m_objAosRegistrationControlListeners;
    }

    inline ImsList<IAosServiceSettingListener*> GetServiceSettingListener()
    {
        return m_objAosServiceSettingListeners;
    }

    inline ImsList<IAosServicePhoneListener*> GetsServicePhoneListener()
    {
        return m_objAosServicePhoneListeners;
    }
};

class AosServiceTest : public ::testing::Test
{
public:
    TestAosService* m_pAosService;

    MockIAosEmergencyListener m_objMockIAosEmergencyListener1;
    MockIAosEmergencyListener m_objMockIAosEmergencyListener2;
    MockIAosEmergencyListener m_objMockIAosEmergencyListener3;

protected:
    virtual void SetUp() override
    {
        m_pAosService = new TestAosService(SLOT_ID);
        ASSERT_TRUE(m_pAosService != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pAosService)
        {
            delete m_pAosService;
        }
    }
};

TEST_F(AosServiceTest, SucceedsAddListenerForIAosRegistrationControlListener)
{
    // GIVEN
    MockIAosRegistrationControlListener objMockListener1;
    MockIAosRegistrationControlListener objMockListener2;
    MockIAosRegistrationControlListener objMockListener3;

    // WHEN
    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // THEN
    EXPECT_EQ(m_pAosService->GetRegistrationControlListener().GetSize(), 3);
}

TEST_F(AosServiceTest, FailsAddListenerForIAosRegistrationControlListenerWhenSameListenerIsExist)
{
    // GIVEN
    MockIAosRegistrationControlListener objMockListener1;
    MockIAosRegistrationControlListener objMockListener2;
    MockIAosRegistrationControlListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // WHEN
    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // THEN
    EXPECT_NE(m_pAosService->GetRegistrationControlListener().GetSize(), 6);
}

TEST_F(AosServiceTest, FailsAddListenerForIAosRegistrationControlListenerWhenListenerIsNull)
{
    // GIVEN
    // WHEN
    m_pAosService->AddListener(static_cast<IAosRegistrationControlListener*>(IMS_NULL));

    // THEN
    EXPECT_NE(m_pAosService->GetEmergencyListeners().GetSize(), 1);
}

TEST_F(AosServiceTest, SucceedsRemoveListenerForIAosRegistrationControlListener)
{
    // GIVEN
    MockIAosRegistrationControlListener objMockListener1;
    MockIAosRegistrationControlListener objMockListener2;
    MockIAosRegistrationControlListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // WHEN
    m_pAosService->RemoveListener(&objMockListener3);
    m_pAosService->RemoveListener(&objMockListener2);
    m_pAosService->RemoveListener(&objMockListener1);

    // THEN
    EXPECT_EQ(m_pAosService->GetRegistrationControlListener().GetSize(), 0);
}

TEST_F(AosServiceTest, FailsRemoveListenerForIAosRegistrationControlListenerWhenNoExistListener)
{
    // GIVEN
    MockIAosRegistrationControlListener objMockListener1;
    MockIAosRegistrationControlListener objMockListener2;

    m_pAosService->AddListener(&objMockListener1);

    // WHEN
    m_pAosService->RemoveListener(&objMockListener2);

    // THEN
    EXPECT_NE(m_pAosService->GetRegistrationControlListener().GetSize(), 0);
}

TEST_F(AosServiceTest, FailsRemoveListenerForIAosRegistrationControlListenerWhenListenerIsNull)
{
    // GIVEN
    MockIAosRegistrationControlListener objMockListener;

    m_pAosService->AddListener(&objMockListener);

    // WHEN
    m_pAosService->RemoveListener(static_cast<MockIAosRegistrationControlListener*>(IMS_NULL));

    // THEN
    EXPECT_NE(m_pAosService->GetRegistrationControlListener().GetSize(), 0);
}

TEST_F(AosServiceTest, SucceedsAddListenerForIAosServiceSettingListener)
{
    // GIVEN
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    // WHEN
    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // THEN
    EXPECT_EQ(m_pAosService->GetServiceSettingListener().GetSize(), 3);
}

TEST_F(AosServiceTest, FailsAddListenerForIAosServiceSettingListenerWhenSameListenerIsExist)
{
    // GIVEN
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // WHEN
    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // THEN
    EXPECT_NE(m_pAosService->GetServiceSettingListener().GetSize(), 6);
}

TEST_F(AosServiceTest, FailsAddListenerForIAosServiceSettingListenerWhenListenerIsNull)
{
    // GIVEN
    // WHEN
    m_pAosService->AddListener(static_cast<IAosServiceSettingListener*>(IMS_NULL));

    // THEN
    EXPECT_NE(m_pAosService->GetServiceSettingListener().GetSize(), 1);
}

TEST_F(AosServiceTest, SucceedsRemoveListenerForIAosServiceSettingListener)
{
    // GIVEN
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // WHEN
    m_pAosService->RemoveListener(&objMockListener3);
    m_pAosService->RemoveListener(&objMockListener2);
    m_pAosService->RemoveListener(&objMockListener1);

    // THEN
    EXPECT_EQ(m_pAosService->GetServiceSettingListener().GetSize(), 0);
}

TEST_F(AosServiceTest, FailsRemoveListenerForIAosServiceSettingListenerWhenNoExistListener)
{
    // GIVEN
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;

    m_pAosService->AddListener(&objMockListener1);

    // WHEN
    m_pAosService->RemoveListener(&objMockListener2);

    // THEN
    EXPECT_NE(m_pAosService->GetServiceSettingListener().GetSize(), 0);
}

TEST_F(AosServiceTest, FailsRemoveListenerForIAosServiceSettingListenerWhenListenerIsNull)
{
    // GIVEN
    MockIAosServiceSettingListener objMockListener;

    m_pAosService->AddListener(&objMockListener);

    // WHEN
    m_pAosService->RemoveListener(static_cast<MockIAosServiceSettingListener*>(IMS_NULL));

    // THEN
    EXPECT_NE(m_pAosService->GetServiceSettingListener().GetSize(), 0);
}

TEST_F(AosServiceTest, SucceedsAddListenerForIAosServicePhoneListener)
{
    // GIVEN
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    // WHEN
    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // THEN
    EXPECT_EQ(m_pAosService->GetsServicePhoneListener().GetSize(), 3);
}

TEST_F(AosServiceTest, FailsAddListenerForIAosServicePhoneListenerWhenSameListenerIsExist)
{
    // GIVEN
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // WHEN
    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // THEN
    EXPECT_NE(m_pAosService->GetsServicePhoneListener().GetSize(), 6);
}

TEST_F(AosServiceTest, FailsAddListenerForIAosServicePhoneListenerWhenListenerIsNull)
{
    // GIVEN
    // WHEN
    m_pAosService->AddListener(static_cast<MockIAosServicePhoneListener*>(IMS_NULL));

    // THEN
    EXPECT_NE(m_pAosService->GetsServicePhoneListener().GetSize(), 1);
}

TEST_F(AosServiceTest, SucceedsRemoveListenerForIAosServicePhoneListener)
{
    // GIVEN
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    // WHEN
    m_pAosService->RemoveListener(&objMockListener3);
    m_pAosService->RemoveListener(&objMockListener2);
    m_pAosService->RemoveListener(&objMockListener1);

    // THEN
    EXPECT_EQ(m_pAosService->GetsServicePhoneListener().GetSize(), 0);
}

TEST_F(AosServiceTest, FailsRemoveListenerForIAosServicePhoneListenerWhenNoExistListener)
{
    // GIVEN
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;

    m_pAosService->AddListener(&objMockListener1);

    // WHEN
    m_pAosService->RemoveListener(&objMockListener2);

    // THEN
    EXPECT_NE(m_pAosService->GetsServicePhoneListener().GetSize(), 0);
}

TEST_F(AosServiceTest, FailsRemoveListenerForIAosServicePhoneListenerWhenListenerIsNull)
{
    // GIVEN
    MockIAosServicePhoneListener objMockListener;

    m_pAosService->AddListener(&objMockListener);

    // WHEN
    m_pAosService->RemoveListener(static_cast<MockIAosServicePhoneListener*>(IMS_NULL));

    // THEN
    EXPECT_NE(m_pAosService->GetsServicePhoneListener().GetSize(), 0);
}

TEST_F(AosServiceTest, SucceedsAddListenerForIAosEmergencyListener)
{
    // GIVEN
    // WHEN
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener1);
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener2);
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener3);

    // THEN
    EXPECT_EQ(m_pAosService->GetEmergencyListeners().GetSize(), 3);
}

TEST_F(AosServiceTest, FailsAddListenerForIAosEmergencyListenerWhenSameListenerIsExist)
{
    // GIVEN
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener1);
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener2);
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener3);

    // WHEN
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener1);
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener2);
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener3);

    // THEN
    EXPECT_NE(m_pAosService->GetEmergencyListeners().GetSize(), 6);
}

TEST_F(AosServiceTest, FailsAddListenerForIAosEmergencyListenerWhenListenerIsNull)
{
    // GIVEN
    // WHEN
    m_pAosService->AddListener(static_cast<IAosEmergencyListener*>(IMS_NULL));

    // THEN
    EXPECT_NE(m_pAosService->GetEmergencyListeners().GetSize(), 1);
}

TEST_F(AosServiceTest, SucceedsRemoveListenerForIAosEmergencyListener)
{
    // GIVEN
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener1);
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener2);
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener3);

    // WHEN
    m_pAosService->RemoveListener(&m_objMockIAosEmergencyListener1);
    m_pAosService->RemoveListener(&m_objMockIAosEmergencyListener2);
    m_pAosService->RemoveListener(&m_objMockIAosEmergencyListener3);

    // THEN
    EXPECT_EQ(m_pAosService->GetEmergencyListeners().GetSize(), 0);
}

TEST_F(AosServiceTest, FailsRemoveListenerForIAosEmergencyListenerWhenNoExistListener)
{
    // GIVEN
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener1);

    // WHEN
    m_pAosService->RemoveListener(&m_objMockIAosEmergencyListener2);

    // THEN
    EXPECT_NE(m_pAosService->GetEmergencyListeners().GetSize(), 0);
}

TEST_F(AosServiceTest, FailsRemoveListenerForIAosEmergencyListenerWhenListenerIsNull)
{
    // GIVEN
    MockIAosEmergencyListener objMockListener;

    m_pAosService->AddListener(&objMockListener);

    // WHEN
    m_pAosService->RemoveListener(static_cast<IAosEmergencyListener*>(IMS_NULL));

    // THEN
    EXPECT_NE(m_pAosService->GetEmergencyListeners().GetSize(), 0);
}

TEST_F(AosServiceTest, SucceedsNotifyEmcCallbackModeChanged)
{
    // GIVEN
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener1);
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener2);
    m_pAosService->AddListener(&m_objMockIAosEmergencyListener3);

    EXPECT_CALL(m_objMockIAosEmergencyListener1, CallbackModeChanged(_, _, _));
    EXPECT_CALL(m_objMockIAosEmergencyListener2, CallbackModeChanged(_, _, _));
    EXPECT_CALL(m_objMockIAosEmergencyListener3, CallbackModeChanged(_, _, _));

    // WHEN
    m_pAosService->NotifyEmcCallbackModeChanged(1, 1, 300);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosServiceTest, FailsNotifyEmcCallbackModeChangedWhenListenerIsNull)
{
    // GIVEN
    EXPECT_CALL(m_objMockIAosEmergencyListener1, CallbackModeChanged(_, _, _)).Times(0);
    EXPECT_CALL(m_objMockIAosEmergencyListener2, CallbackModeChanged(_, _, _)).Times(0);
    EXPECT_CALL(m_objMockIAosEmergencyListener3, CallbackModeChanged(_, _, _)).Times(0);

    // WHEN
    m_pAosService->NotifyEmcCallbackModeChanged(1, 1, 300);

    // THEN : GIVEN conditions should be met.
}

TEST_F(AosServiceTest, UpdateSipDelegateRegistration)
{
    MockIAosRegistrationControlListener objMockListener1;
    MockIAosRegistrationControlListener objMockListener2;
    MockIAosRegistrationControlListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, RegistrationControl_UpdateSipDelegateRegistration());
    EXPECT_CALL(objMockListener2, RegistrationControl_UpdateSipDelegateRegistration());
    EXPECT_CALL(objMockListener3, RegistrationControl_UpdateSipDelegateRegistration());

    m_pAosService->UpdateSipDelegateRegistration();
}

TEST_F(AosServiceTest, TriggerSipDelegateDeregistration)
{
    MockIAosRegistrationControlListener objMockListener1;
    MockIAosRegistrationControlListener objMockListener2;
    MockIAosRegistrationControlListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, RegistrationControl_TriggerSipDelegateDeregistration());
    EXPECT_CALL(objMockListener2, RegistrationControl_TriggerSipDelegateDeregistration());
    EXPECT_CALL(objMockListener3, RegistrationControl_TriggerSipDelegateDeregistration());

    m_pAosService->TriggerSipDelegateDeregistration();
}

TEST_F(AosServiceTest, TriggerFullNetworkRegistration)
{
    MockIAosRegistrationControlListener objMockListener1;
    MockIAosRegistrationControlListener objMockListener2;
    MockIAosRegistrationControlListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, RegistrationControl_TriggerFullNetworkRegistration(_, _));
    EXPECT_CALL(objMockListener2, RegistrationControl_TriggerFullNetworkRegistration(_, _));
    EXPECT_CALL(objMockListener3, RegistrationControl_TriggerFullNetworkRegistration(_, _));

    m_pAosService->TriggerFullNetworkRegistration(1, AString("testReason"));
}

TEST_F(AosServiceTest, NotifyCapabilitiesChanged)
{
    MockIAosRegistrationControlListener objMockListener1;
    MockIAosRegistrationControlListener objMockListener2;
    MockIAosRegistrationControlListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, RegistrationControl_NotifyCapabilitiesChanged(_));
    EXPECT_CALL(objMockListener2, RegistrationControl_NotifyCapabilitiesChanged(_));
    EXPECT_CALL(objMockListener3, RegistrationControl_NotifyCapabilitiesChanged(_));

    const ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    m_pAosService->NotifyCapabilitiesChanged(objCapabilities);
}

TEST_F(AosServiceTest, ControlRegistration)
{
    MockIAosRegistrationControlListener objMockListener1;
    MockIAosRegistrationControlListener objMockListener2;
    MockIAosRegistrationControlListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, RegistrationControl_ControlRegistration(_, _, _));
    EXPECT_CALL(objMockListener2, RegistrationControl_ControlRegistration(_, _, _));
    EXPECT_CALL(objMockListener3, RegistrationControl_ControlRegistration(_, _, _));

    m_pAosService->ControlRegistration(1, 1, 1);
}

TEST_F(AosServiceTest, NotifyAirplaneSetting)
{
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServiceSetting_AirplaneChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener2, ServiceSetting_AirplaneChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener3, ServiceSetting_AirplaneChanged(IMS_TRUE));

    EXPECT_CALL(objMockListener1, ServiceSetting_AirplaneChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener2, ServiceSetting_AirplaneChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener3, ServiceSetting_AirplaneChanged(IMS_FALSE));

    m_pAosService->NotifyAirplaneSetting(IMS_TRUE);
    m_pAosService->NotifyAirplaneSetting(IMS_FALSE);
}

TEST_F(AosServiceTest, NotifyDataRoamingSetting)
{
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServiceSetting_DataRoamingChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener2, ServiceSetting_DataRoamingChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener3, ServiceSetting_DataRoamingChanged(IMS_TRUE));

    EXPECT_CALL(objMockListener1, ServiceSetting_DataRoamingChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener2, ServiceSetting_DataRoamingChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener3, ServiceSetting_DataRoamingChanged(IMS_FALSE));

    m_pAosService->NotifyDataRoamingSetting(IMS_TRUE);
    m_pAosService->NotifyDataRoamingSetting(IMS_FALSE);
}

TEST_F(AosServiceTest, NotifyMobileDataSetting)
{
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServiceSetting_MobileDataChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener2, ServiceSetting_MobileDataChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener3, ServiceSetting_MobileDataChanged(IMS_TRUE));

    EXPECT_CALL(objMockListener1, ServiceSetting_MobileDataChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener2, ServiceSetting_MobileDataChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener3, ServiceSetting_MobileDataChanged(IMS_FALSE));

    m_pAosService->NotifyMobileDataSetting(IMS_TRUE);
    m_pAosService->NotifyMobileDataSetting(IMS_FALSE);
}

TEST_F(AosServiceTest, NotifyRoamingPreferredVoiceNetwork)
{
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServiceSetting_RoamingPreferredVoiceNetworkChanged(_));
    EXPECT_CALL(objMockListener2, ServiceSetting_RoamingPreferredVoiceNetworkChanged(_));
    EXPECT_CALL(objMockListener3, ServiceSetting_RoamingPreferredVoiceNetworkChanged(_));

    m_pAosService->NotifyRoamingPreferredVoiceNetwork(1);
}

TEST_F(AosServiceTest, NotifyServiceSetting)
{
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServiceSetting_ServiceChanged(_, _));
    EXPECT_CALL(objMockListener2, ServiceSetting_ServiceChanged(_, _));
    EXPECT_CALL(objMockListener3, ServiceSetting_ServiceChanged(_, _));

    m_pAosService->NotifyServiceSetting(1, 1);
}

TEST_F(AosServiceTest, NotifyTtySetting)
{
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServiceSetting_TtyChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener2, ServiceSetting_TtyChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener3, ServiceSetting_TtyChanged(IMS_TRUE));

    EXPECT_CALL(objMockListener1, ServiceSetting_TtyChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener2, ServiceSetting_TtyChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener3, ServiceSetting_TtyChanged(IMS_FALSE));

    m_pAosService->NotifyTtySetting(IMS_TRUE);
    m_pAosService->NotifyTtySetting(IMS_FALSE);
}

TEST_F(AosServiceTest, NotifyVideoSetting)
{
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServiceSetting_VideoChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener2, ServiceSetting_VideoChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener3, ServiceSetting_VideoChanged(IMS_TRUE));

    EXPECT_CALL(objMockListener1, ServiceSetting_VideoChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener2, ServiceSetting_VideoChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener3, ServiceSetting_VideoChanged(IMS_FALSE));

    m_pAosService->NotifyVideoSetting(IMS_TRUE);
    m_pAosService->NotifyVideoSetting(IMS_FALSE);
}

TEST_F(AosServiceTest, NotifyVolteSetting)
{
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServiceSetting_VolteChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener2, ServiceSetting_VolteChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener3, ServiceSetting_VolteChanged(IMS_TRUE));

    EXPECT_CALL(objMockListener1, ServiceSetting_VolteChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener2, ServiceSetting_VolteChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener3, ServiceSetting_VolteChanged(IMS_FALSE));

    m_pAosService->NotifyVolteSetting(IMS_TRUE);
    m_pAosService->NotifyVolteSetting(IMS_FALSE);
}

TEST_F(AosServiceTest, NotifyWfcSetting)
{
    MockIAosServiceSettingListener objMockListener1;
    MockIAosServiceSettingListener objMockListener2;
    MockIAosServiceSettingListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServiceSetting_WfcChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener2, ServiceSetting_WfcChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener3, ServiceSetting_WfcChanged(IMS_TRUE));

    EXPECT_CALL(objMockListener1, ServiceSetting_WfcChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener2, ServiceSetting_WfcChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener3, ServiceSetting_WfcChanged(IMS_FALSE));

    m_pAosService->NotifyWfcSetting(IMS_TRUE);
    m_pAosService->NotifyWfcSetting(IMS_FALSE);
}

TEST_F(AosServiceTest, NotifyAosStart)
{
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServicePhone_AosStart());
    EXPECT_CALL(objMockListener2, ServicePhone_AosStart());
    EXPECT_CALL(objMockListener3, ServicePhone_AosStart());

    m_pAosService->NotifyAosStart();
}

TEST_F(AosServiceTest, NotifyIpcanHandoverFailure)
{
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServicePhone_notifyIpcanHandoverFailure(_, _));
    EXPECT_CALL(objMockListener2, ServicePhone_notifyIpcanHandoverFailure(_, _));
    EXPECT_CALL(objMockListener3, ServicePhone_notifyIpcanHandoverFailure(_, _));

    m_pAosService->NotifyIpcanHandoverFailure(1, 1);
}

TEST_F(AosServiceTest, NotifyIsimState)
{
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServicePhone_IsimStateChanged(_));
    EXPECT_CALL(objMockListener2, ServicePhone_IsimStateChanged(_));
    EXPECT_CALL(objMockListener3, ServicePhone_IsimStateChanged(_));

    m_pAosService->NotifyIsimState(1);
}

TEST_F(AosServiceTest, NotifyLocationInfo)
{
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServicePhone_LocationInfoChanged(_));
    EXPECT_CALL(objMockListener2, ServicePhone_LocationInfoChanged(_));
    EXPECT_CALL(objMockListener3, ServicePhone_LocationInfoChanged(_));

    m_pAosService->NotifyLocationInfo(1);
}

TEST_F(AosServiceTest, NotifyMobileDataLimit)
{
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServicePhone_MobileDataLimitChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener2, ServicePhone_MobileDataLimitChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener3, ServicePhone_MobileDataLimitChanged(IMS_TRUE));

    EXPECT_CALL(objMockListener1, ServicePhone_MobileDataLimitChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener2, ServicePhone_MobileDataLimitChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener3, ServicePhone_MobileDataLimitChanged(IMS_FALSE));

    m_pAosService->NotifyMobileDataLimit(IMS_TRUE);
    m_pAosService->NotifyMobileDataLimit(IMS_FALSE);
}

TEST_F(AosServiceTest, NotifyNetworkVideoCapability)
{
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServicePhone_NetworkVideoCapabilityChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener2, ServicePhone_NetworkVideoCapabilityChanged(IMS_TRUE));
    EXPECT_CALL(objMockListener3, ServicePhone_NetworkVideoCapabilityChanged(IMS_TRUE));

    EXPECT_CALL(objMockListener1, ServicePhone_NetworkVideoCapabilityChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener2, ServicePhone_NetworkVideoCapabilityChanged(IMS_FALSE));
    EXPECT_CALL(objMockListener3, ServicePhone_NetworkVideoCapabilityChanged(IMS_FALSE));

    m_pAosService->NotifyNetworkVideoCapability(IMS_TRUE);
    m_pAosService->NotifyNetworkVideoCapability(IMS_FALSE);
}

TEST_F(AosServiceTest, NotifyPhoneNumberState)
{
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServicePhone_PhoneNumberStateChanged(IMS_TRUE, _));
    EXPECT_CALL(objMockListener2, ServicePhone_PhoneNumberStateChanged(IMS_TRUE, _));
    EXPECT_CALL(objMockListener3, ServicePhone_PhoneNumberStateChanged(IMS_TRUE, _));

    EXPECT_CALL(objMockListener1, ServicePhone_PhoneNumberStateChanged(IMS_FALSE, _));
    EXPECT_CALL(objMockListener2, ServicePhone_PhoneNumberStateChanged(IMS_FALSE, _));
    EXPECT_CALL(objMockListener3, ServicePhone_PhoneNumberStateChanged(IMS_FALSE, _));

    m_pAosService->NotifyPhoneNumberState(IMS_TRUE, 1);
    m_pAosService->NotifyPhoneNumberState(IMS_FALSE, 1);
}

TEST_F(AosServiceTest, NotifyPlmnChanged)
{
    TestAosService* pTestAosService = new TestAosService(SLOT_ID);

    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    pTestAosService->AddListener(&objMockListener1);
    pTestAosService->AddListener(&objMockListener2);
    pTestAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServicePhone_PlmnChanged());
    EXPECT_CALL(objMockListener2, ServicePhone_PlmnChanged());
    EXPECT_CALL(objMockListener3, ServicePhone_PlmnChanged());

    pTestAosService->NotifyPlmnChanged();
    EXPECT_TRUE(pTestAosService->IsTimerRunning(TestAosService::TIMER_PLMN_CHANGE_DELAY));
    pTestAosService->ProcessPlmnChangeDelayTimerExpired();
}

TEST_F(AosServiceTest, NotifyPowerOff)
{
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServicePhone_PowerOff());
    EXPECT_CALL(objMockListener2, ServicePhone_PowerOff());
    EXPECT_CALL(objMockListener3, ServicePhone_PowerOff());

    m_pAosService->NotifyPowerOff();
}

TEST_F(AosServiceTest, NotifyPreciseCallState)
{
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServicePhone_PreciseCallStateChanged(_));
    EXPECT_CALL(objMockListener2, ServicePhone_PreciseCallStateChanged(_));
    EXPECT_CALL(objMockListener3, ServicePhone_PreciseCallStateChanged(_));

    m_pAosService->NotifyPreciseCallState(1);
}

TEST_F(AosServiceTest, NotifyCarrierSignalPcoValueChanged)
{
    MockIAosServicePhoneListener objMockListener1;
    MockIAosServicePhoneListener objMockListener2;
    MockIAosServicePhoneListener objMockListener3;

    m_pAosService->AddListener(&objMockListener1);
    m_pAosService->AddListener(&objMockListener2);
    m_pAosService->AddListener(&objMockListener3);

    EXPECT_CALL(objMockListener1, ServicePhone_PcoValueChanged(_));
    EXPECT_CALL(objMockListener2, ServicePhone_PcoValueChanged(_));
    EXPECT_CALL(objMockListener3, ServicePhone_PcoValueChanged(_));

    m_pAosService->NotifyCarrierSignalPcoValueChanged(5);
}

TEST_F(AosServiceTest, NotifyRegistered)
{
    const ImsList<AString> objFeatureTags;
    EXPECT_TRUE(m_pAosService->NotifyRegistered(
            IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::LTE, 1, objFeatureTags));
}

TEST_F(AosServiceTest, NotifyRegistering)
{
    const ImsList<AString> objFeatureTags;
    EXPECT_TRUE(m_pAosService->NotifyRegistering(
            IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::LTE, 1, objFeatureTags));
}

TEST_F(AosServiceTest, NotifyDeregistered)
{
    EXPECT_TRUE(m_pAosService->NotifyDeregistered(AosNetworkType::LTE, AosReasonCode::UNSPECIFIED));
}

TEST_F(AosServiceTest, NotifyTechnologyChangeFailed)
{
    EXPECT_TRUE(m_pAosService->NotifyTechnologyChangeFailed(
            IAosRegistration::IMS_REG_TYPE_NORMAL, AosNetworkType::LTE, 1));
}

TEST_F(AosServiceTest, NotifyAssociatedUriChanged)
{
    const ImsList<AString> objUris;
    EXPECT_TRUE(m_pAosService->NotifyAssociatedUriChanged(objUris));
}

TEST_F(AosServiceTest, NotifyCapabilitiesUpdateFailed)
{
    EXPECT_TRUE(m_pAosService->NotifyCapabilitiesUpdateFailed(
            AosCapability::VOICE, AosNetworkType::LTE, AosReasonCode::UNSPECIFIED));
}

TEST_F(AosServiceTest, NotifyAosIsimState)
{
    EXPECT_TRUE(m_pAosService->NotifyAosIsimState(AosIsimState::VALID));
}

TEST_F(AosServiceTest, NotifyRegEventState)
{
    ImsList<AString> objImpus;
    EXPECT_TRUE(m_pAosService->NotifyRegEventState(200, objImpus));
}

TEST_F(AosServiceTest, RequestPhoneNumberRetry)
{
    EXPECT_TRUE(m_pAosService->RequestPhoneNumberRetry(AosPhoneNumberRetryCommand::INITIAL));
}

TEST_F(AosServiceTest, RequestWifiService)
{
    EXPECT_TRUE(m_pAosService->RequestWifiService(IMS_TRUE));
}

TEST_F(AosServiceTest, GetCapabilities)
{
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::UT) |
                    static_cast<IMS_UINT32>(AosCapability::SMS) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::OPTIONS_UCE) |
                    static_cast<IMS_UINT32>(AosCapability::PRESENCE_UCE));

    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    m_pAosService->NotifyCapabilitiesChanged(objCapabilities);
    EXPECT_EQ(
            m_pAosService->GetCapabilities().GetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE)),
            objCapabilities.GetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE)));

    EXPECT_EQ(m_pAosService->GetCapabilities().GetValue(
                      static_cast<IMS_UINT32>(AosNetworkType::IWLAN)),
            objCapabilities.GetValue(static_cast<IMS_UINT32>(AosNetworkType::IWLAN)));
}

TEST_F(AosServiceTest, GetCapabilitiesForNetwork_ReturnValue)
{
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::UT) |
                    static_cast<IMS_UINT32>(AosCapability::SMS) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::OPTIONS_UCE) |
                    static_cast<IMS_UINT32>(AosCapability::PRESENCE_UCE));

    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    m_pAosService->NotifyCapabilitiesChanged(objCapabilities);

    EXPECT_EQ(m_pAosService->GetCapabilitiesForNetwork(AosNetworkType::LTE),
            objCapabilities.GetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE)));
    EXPECT_EQ(m_pAosService->GetCapabilitiesForNetwork(AosNetworkType::IWLAN),
            objCapabilities.GetValue(static_cast<IMS_UINT32>(AosNetworkType::IWLAN)));
}

TEST_F(AosServiceTest, GetCapabilitiesForNetwork_ReturnZero)
{
    EXPECT_EQ(m_pAosService->GetCapabilitiesForNetwork(AosNetworkType::LTE), 0);
    EXPECT_EQ(m_pAosService->GetCapabilitiesForNetwork(AosNetworkType::IWLAN), 0);
}

TEST_F(AosServiceTest, IsSupportCapabilitiesForNetwork)
{
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::UT) |
                    static_cast<IMS_UINT32>(AosCapability::SMS) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::OPTIONS_UCE) |
                    static_cast<IMS_UINT32>(AosCapability::PRESENCE_UCE));

    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    m_pAosService->NotifyCapabilitiesChanged(objCapabilities);

    EXPECT_TRUE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::LTE, AosCapability::VOICE));
    EXPECT_TRUE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::LTE, AosCapability::VIDEO));
    EXPECT_TRUE(
            m_pAosService->IsSupportCapabilitiesForNetwork(AosNetworkType::LTE, AosCapability::UT));
    EXPECT_TRUE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::LTE, AosCapability::SMS));
    EXPECT_TRUE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::LTE, AosCapability::CALL_COMPOSER));
    EXPECT_TRUE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::LTE, AosCapability::OPTIONS_UCE));
    EXPECT_TRUE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::LTE, AosCapability::PRESENCE_UCE));

    EXPECT_TRUE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::IWLAN, AosCapability::VOICE));
    EXPECT_TRUE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::IWLAN, AosCapability::VIDEO));
    EXPECT_FALSE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::IWLAN, AosCapability::UT));
    EXPECT_FALSE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::IWLAN, AosCapability::SMS));
    EXPECT_FALSE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::IWLAN, AosCapability::CALL_COMPOSER));
    EXPECT_FALSE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::IWLAN, AosCapability::OPTIONS_UCE));
    EXPECT_FALSE(m_pAosService->IsSupportCapabilitiesForNetwork(
            AosNetworkType::IWLAN, AosCapability::PRESENCE_UCE));
}

TEST_F(AosServiceTest, PrintCapabilities)
{
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;
    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::UT) |
                    static_cast<IMS_UINT32>(AosCapability::SMS) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::OPTIONS_UCE) |
                    static_cast<IMS_UINT32>(AosCapability::PRESENCE_UCE));

    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::UT) |
                    static_cast<IMS_UINT32>(AosCapability::SMS) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::OPTIONS_UCE) |
                    static_cast<IMS_UINT32>(AosCapability::PRESENCE_UCE));

    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::CROSS_SIM),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::UT) |
                    static_cast<IMS_UINT32>(AosCapability::SMS) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::OPTIONS_UCE) |
                    static_cast<IMS_UINT32>(AosCapability::PRESENCE_UCE));

    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::UT) |
                    static_cast<IMS_UINT32>(AosCapability::SMS) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::OPTIONS_UCE) |
                    static_cast<IMS_UINT32>(AosCapability::PRESENCE_UCE));

    objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::UTRAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::UT) |
                    static_cast<IMS_UINT32>(AosCapability::SMS) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER) |
                    static_cast<IMS_UINT32>(AosCapability::OPTIONS_UCE) |
                    static_cast<IMS_UINT32>(AosCapability::PRESENCE_UCE));

    AString strExpect =
            AString("LTE : [ VOICE VIDEO UT SMS CALL_COMPOSER OPTIONS_UCE PRESENCE_UCE ] ");
    strExpect.Append("IWLAN : [ VOICE VIDEO UT SMS CALL_COMPOSER OPTIONS_UCE PRESENCE_UCE ] ");
    strExpect.Append("CROSS_SIM : [ VOICE VIDEO UT SMS CALL_COMPOSER OPTIONS_UCE PRESENCE_UCE ] ");
    strExpect.Append("NR : [ VOICE VIDEO UT SMS CALL_COMPOSER OPTIONS_UCE PRESENCE_UCE ] ");
    strExpect.Append("UTRAN : [ VOICE VIDEO UT SMS CALL_COMPOSER OPTIONS_UCE PRESENCE_UCE ] ");

    EXPECT_TRUE(m_pAosService->PrintCapabilities(objCapabilities).Equals(strExpect));
}

TEST_F(AosServiceTest, NetworkTypeToString)
{
    EXPECT_STREQ(m_pAosService->NetworkTypeToString(static_cast<IMS_SINT32>(AosNetworkType::NONE)),
            "NONE");
    EXPECT_STREQ(m_pAosService->NetworkTypeToString(static_cast<IMS_SINT32>(AosNetworkType::LTE)),
            "LTE");
    EXPECT_STREQ(m_pAosService->NetworkTypeToString(static_cast<IMS_SINT32>(AosNetworkType::IWLAN)),
            "IWLAN");
    EXPECT_STREQ(
            m_pAosService->NetworkTypeToString(static_cast<IMS_SINT32>(AosNetworkType::CROSS_SIM)),
            "CROSS_SIM");
    EXPECT_STREQ(
            m_pAosService->NetworkTypeToString(static_cast<IMS_SINT32>(AosNetworkType::NR)), "NR");
    EXPECT_STREQ(m_pAosService->NetworkTypeToString(static_cast<IMS_SINT32>(AosNetworkType::UTRAN)),
            "UTRAN");
    EXPECT_STREQ(m_pAosService->NetworkTypeToString(999), "INVALID");
}
