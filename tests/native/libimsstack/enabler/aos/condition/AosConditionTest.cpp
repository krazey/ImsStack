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
#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
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
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosRegistration.h"
#include "interface/MockIAosSubscriber.h"

class IAosConditionListener;

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnPointee;
using ::testing::ReturnRef;

enum
{
    HOLD_EVENT_NONE = 0x00,
    HOLD_EVENT_ROAMING = 0x01,
    HOLD_EVENT_IMS_SERVICE = 0x02
};

class AosConditionTest : public ::testing::Test
{
public:
    AosCondition* m_pAosCondition;
    AosBlock* m_pAosBlock;
    IAosNConfiguration* m_piOriginConfiguration;
    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosNetTracker m_objMockIAosNetTracker;
    MockIAosSubscriber m_objMockIAosSubscriber;
    MockIAosRegistration m_objMockIAosRegistration;

protected:
    virtual void SetUp() override
    {
        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosRegistration));

        EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        m_pAosCondition = new AosCondition(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
        ASSERT_TRUE(m_pAosCondition != nullptr);

        m_pAosBlock = new AosBlock(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
        ASSERT_TRUE(m_pAosBlock != nullptr);

        IAosBlock* piAosBlock = static_cast<IAosBlock*>(m_pAosBlock);
        SetAosBlock(piAosBlock);

        m_piOriginConfiguration = AosProvider::GetInstance()->GetNConfiguration();
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piOriginConfiguration, 0);

        if (m_pAosBlock)
        {
            delete m_pAosBlock;
        }

        if (m_pAosCondition)
        {
            delete m_pAosCondition;
        }
    }

    void SetAosBlock(IN IAosBlock* piBlock) { m_pAosCondition->m_piBlock = piBlock; }

    void SetAosBlockToCellur(IN IAosBlock* piBlock)
    {
        m_pAosCondition->m_pAvailableCellular->m_piBlock = piBlock;
    }

    void SetAosBlockToWifi(IN IAosBlock* piBlock)
    {
        m_pAosCondition->m_pAvailableWiFi->m_piBlock = piBlock;
    }

    void SetServiceAvailable(IN SERVICE_TYPE eType, IN IMS_BOOL bIsAvailable)
    {
        switch (eType)
        {
            case SERVICE_CELLULAR:
                m_pAosCondition->m_bCellServiceAvailable = bIsAvailable;
                break;
            case SERVICE_WIFI:
                m_pAosCondition->m_bWiFiServiceAvailable = bIsAvailable;
                break;
            default:
                break;
        }
    }

    AosServiceAvailable* GetServiceAvailable(IN SERVICE_TYPE eType)
    {
        switch (eType)
        {
            case SERVICE_CELLULAR:
                return dynamic_cast<AosServiceAvailable*>(m_pAosCondition->m_pAvailableCellular);
            case SERVICE_WIFI:
                return dynamic_cast<AosServiceAvailable*>(m_pAosCondition->m_pAvailableWiFi);
            default:
                return nullptr;
        }
    }

    void SetServiceType(IN SERVICE_TYPE eType) { m_pAosCondition->m_eServiceType = eType; }

    IAosConditionListener* GetConditionListener() { return m_pAosCondition->m_piListener; }

    IMS_UINT32 GetListener() { return m_pAosCondition->m_nListeners; }

    void Event_NotifyEvent(IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
    {
        m_pAosCondition->Event_NotifyEvent(nEvent, nWParam, nLParam);
    }

    IMS_BOOL GetRoamingState() { return m_pAosCondition->m_pAvailableCellular->m_bRoamingState; }

    IMS_BOOL GetVopsState() { return m_pAosCondition->m_pAvailableCellular->m_bVopsState; }

    IMS_BOOL GetLteInfo() { return m_pAosCondition->m_bIsCombindAttached; }

    void SetLteInfo(IN IMS_BOOL bIsCombindAttached)
    {
        m_pAosCondition->m_bIsCombindAttached = bIsCombindAttached;
    }

    void SetRefreshStarted(IN IMS_BOOL bIsRefreshStarted)
    {
        m_pAosCondition->m_bIsRefreshStarted = bIsRefreshStarted;
    }

    void SetTestLocation(IN ILocationProperties* piTestLocation)
    {
        m_pAosCondition->m_pAvailableWiFi->m_piTestLocation = piTestLocation;
    }

    void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState)
    {
        m_pAosCondition->CallTracker_StateChanged(nType, eState);
    }

    void NetTracker_StatusChanged() { m_pAosCondition->NetTracker_StatusChanged(); }

    void Subscriber_StateChanged(IN IMS_UINT32 nState)
    {
        m_pAosCondition->Subscriber_StateChanged(nState, 0);
    }

    void Block_Changed(IN IMS_UINT32 nType, IN IMS_UINT32 nParam)
    {
        m_pAosCondition->Block_Changed(nType, nParam);
    }

    void ServiceAvailable_RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason)
    {
        m_pAosCondition->ServiceAvailable_RequestCommand(nCommand, nReason);
    }

    void NConfiguration_NotifyConfigChanged()
    {
        m_pAosCondition->NConfiguration_NotifyConfigChanged();
    }

    void ServicePhone_AosStart() { m_pAosCondition->ServicePhone_AosStart(); }

    void ServicePhone_LocationInfoChanged(IN LocationInfo eState)
    {
        m_pAosCondition->ServicePhone_LocationInfoChanged(eState);
    }

    void ServicePhone_PhoneNumberStateChanged(IN PhoneNumberState eState)
    {
        m_pAosCondition->ServicePhone_PhoneNumberStateChanged(IMS_FALSE, eState);
    }

    void ServicePhone_PlmnChanged() { m_pAosCondition->ServicePhone_PlmnChanged(); }

    void ServicePhone_PowerOff() { m_pAosCondition->ServicePhone_PowerOff(); }

    void ServiceSetting_AirplaneChanged(IN IMS_BOOL bIsOn)
    {
        m_pAosCondition->ServiceSetting_AirplaneChanged(bIsOn);
    }

    void ServiceSetting_ServiceChanged(IN ServiceSetting eState)
    {
        m_pAosCondition->ServiceSetting_ServiceChanged(eState, 0);
    }

    void AddHold(IN IMS_UINT32 nEvent, IN IMS_BOOL bIsEventReset)
    {
        m_pAosCondition->AddHold(nEvent, bIsEventReset);
    }
    void RemoveHold(IN IMS_UINT32 nEvent, IN IMS_BOOL bIsEventReset)
    {
        m_pAosCondition->RemoveHold(nEvent, bIsEventReset);
    }

    IMS_BOOL IsHolded(IN IMS_UINT32 nEvent) { return m_pAosCondition->IsHolded(nEvent); }

    IMS_BOOL IsRefreshStarted() const { return m_pAosCondition->m_bIsRefreshStarted; }

    void ServiceSetting_TtyChanged(IN IMS_BOOL bIsOn)
    {
        m_pAosCondition->ServiceSetting_TtyChanged(bIsOn);
    }

    void AddListener(IN IMS_UINT32 nType) { m_pAosCondition->AddListener(nType); }

    void RemoveListener(IN IMS_UINT32 nType) { m_pAosCondition->RemoveListener(nType); }

    IMS_BOOL IsListenerEnabled(IN IMS_UINT32 nType)
    {
        return m_pAosCondition->IsListenerEnabled(nType);
    }

    IMS_BOOL RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason = 0)
    {
        return m_pAosCondition->RequestCommand(nCommand, nReason);
    }

    void UpdateRegistrationMode() { m_pAosCondition->UpdateRegistrationMode(); }
};

TEST_F(AosConditionTest, Constructor)
{
    EXPECT_EQ(GetListener(), AosCondition::LISTENER_ALL);
}

TEST_F(AosConditionTest, Start)
{
    EXPECT_EQ(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_EQ(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_FALSE(m_pAosCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
    EXPECT_FALSE(m_pAosCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));

    m_pAosCondition->Start();
    EXPECT_NE(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_NE(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));
}

TEST_F(AosConditionTest, Stop)
{
    m_pAosCondition->Start();
    EXPECT_NE(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_NE(GetServiceAvailable(SERVICE_WIFI), nullptr);

    m_pAosCondition->Stop();
    EXPECT_EQ(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_EQ(GetServiceAvailable(SERVICE_WIFI), nullptr);
}

TEST_F(AosConditionTest, SetListener)
{
    EXPECT_EQ(GetConditionListener(), nullptr);

    IAosConditionListener* piAosConditionListener = new MockIAosConditionListener();
    m_pAosCondition->SetListener(piAosConditionListener);
    EXPECT_EQ(GetConditionListener(), piAosConditionListener);
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
    m_pAosCondition->SetBlock(BLOCK_WIFI_ROAMING);
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
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_ROAMING));
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
    m_pAosCondition->SetBlock(BLOCK_WIFI_ROAMING);
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
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_ROAMING));
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
    m_pAosCondition->ResetBlock(BLOCK_WIFI_ROAMING);
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
    m_pAosCondition->SetBlock(BLOCK_WIFI_ROAMING);
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
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_ROAMING));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_TEMPORARILY_BLOCKED));
}

TEST_F(AosConditionTest, IsReady_CellularServiceAvailable)
{
    // Cellular : Available, WiFi : Not Available
    SetServiceAvailable(SERVICE_CELLULAR, IMS_TRUE);
    SetServiceAvailable(SERVICE_WIFI, IMS_FALSE);

    SetServiceType(SERVICE_CELLULAR);
    EXPECT_TRUE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WIFI);
    EXPECT_FALSE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WHOLE);
    EXPECT_TRUE(m_pAosCondition->IsReady());
}

TEST_F(AosConditionTest, IsReady_WifiServiceAvailable)
{
    // Cellular : Not Available, WiFi : Available
    SetServiceAvailable(SERVICE_CELLULAR, IMS_FALSE);
    SetServiceAvailable(SERVICE_WIFI, IMS_TRUE);

    SetServiceType(SERVICE_CELLULAR);
    EXPECT_FALSE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WIFI);
    EXPECT_TRUE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WHOLE);
    EXPECT_TRUE(m_pAosCondition->IsReady());
}

TEST_F(AosConditionTest, IsReady_WholeServiceAvailable)
{
    // Cellular : Available, WiFi : Available
    SetServiceAvailable(SERVICE_CELLULAR, IMS_TRUE);
    SetServiceAvailable(SERVICE_WIFI, IMS_TRUE);

    SetServiceType(SERVICE_CELLULAR);
    EXPECT_TRUE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WHOLE);
    EXPECT_TRUE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WIFI);
    EXPECT_TRUE(m_pAosCondition->IsReady());
}

TEST_F(AosConditionTest, IsReady_WholeServiceNotAvailable)
{
    // Cellular : Not Available, WiFi : Not Available
    SetServiceAvailable(SERVICE_CELLULAR, IMS_FALSE);
    SetServiceAvailable(SERVICE_WIFI, IMS_FALSE);

    SetServiceType(SERVICE_CELLULAR);
    EXPECT_FALSE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WHOLE);
    EXPECT_FALSE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WIFI);
    EXPECT_FALSE(m_pAosCondition->IsReady());
}

TEST_F(AosConditionTest, CheckServiceAvailable)
{
    EXPECT_EQ(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_EQ(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_EQ(m_pAosCondition->CheckServiceAvailable(SERVICE_WHOLE), AosCondition::CHECK_NONE);
    EXPECT_EQ(m_pAosCondition->CheckServiceAvailable(SERVICE_CELLULAR), AosCondition::CHECK_NONE);
    EXPECT_EQ(m_pAosCondition->CheckServiceAvailable(SERVICE_WIFI), AosCondition::CHECK_NONE);

    m_pAosCondition->Start();
    EXPECT_NE(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_NE(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_EQ(m_pAosCondition->CheckServiceAvailable(SERVICE_WHOLE),
            AosCondition::CHECK_CELLULAR | AosCondition::CHECK_WIFI);
    EXPECT_EQ(
            m_pAosCondition->CheckServiceAvailable(SERVICE_CELLULAR), AosCondition::CHECK_CELLULAR);
    EXPECT_EQ(m_pAosCondition->CheckServiceAvailable(SERVICE_WIFI), AosCondition::CHECK_WIFI);
}

TEST_F(AosConditionTest, CheckBadNetwork)
{
    EXPECT_EQ(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_EQ(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WIFI));

    m_pAosCondition->Start();
    EXPECT_NE(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_NE(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WIFI));

    SetServiceAvailable(SERVICE_CELLULAR, IMS_TRUE);
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_TRUE(m_pAosCondition->CheckBadNetwork(SERVICE_WIFI));

    SetServiceAvailable(SERVICE_CELLULAR, IMS_FALSE);
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_FALSE(m_pAosCondition->CheckBadNetwork(SERVICE_WIFI));
}

TEST_F(AosConditionTest, Event_NotifyEvent_RoamingState)
{
    m_pAosCondition->Start();

    Event_NotifyEvent(IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_OFF);
    EXPECT_FALSE(GetRoamingState());

    Event_NotifyEvent(IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_ON, IMS_ROAMING_STATE_OFF);
    EXPECT_TRUE(GetRoamingState());

    Event_NotifyEvent(IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_OFF);
    EXPECT_FALSE(GetRoamingState());

    Event_NotifyEvent(IMS_EVENT_ROAMING_STATE, IMS_ROAMING_STATE_OFF, IMS_ROAMING_STATE_ON);
    EXPECT_TRUE(GetRoamingState());
}

TEST_F(AosConditionTest, Event_NotifyEvent_VopsState)
{
    m_pAosCondition->Start();

    Event_NotifyEvent(IMS_EVENT_IMS_VOICE_OVER_PS_STATE, IMS_VOICE_OVER_PS_SUPPORTED, 0);
    EXPECT_TRUE(GetVopsState());

    Event_NotifyEvent(IMS_EVENT_IMS_VOICE_OVER_PS_STATE, IMS_VOICE_OVER_PS_NOT_SUPPORTED, 0);
    EXPECT_FALSE(GetVopsState());
}

TEST_F(AosConditionTest, Event_NotifyEvent_LteInfo)
{
    m_pAosCondition->Start();

    Event_NotifyEvent(IMS_EVENT_LTE_INFO, IMS_LTE_INFO_EPS_ONLY_ATTACHED, 0);
    EXPECT_TRUE(GetLteInfo());

    Event_NotifyEvent(IMS_EVENT_LTE_INFO, IMS_LTE_INFO_NORMAL_ATTACHED, 0);
    EXPECT_FALSE(GetLteInfo());
}

TEST_F(AosConditionTest, CallTracker_StateChanged_Cs_Offhook)
{
    m_pAosCondition->Start();

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::OFFHOOK);
}

TEST_F(AosConditionTest, CallTracker_StateChanged_Cs_NotOffhook)
{
    m_pAosCondition->Start();

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    CallTracker_StateChanged(IAosCallTracker::TYPE_CS, CallState::IDLE);
}

TEST_F(AosConditionTest, CallTracker_StateChanged_Normal_Offhook)
{
    m_pAosCondition->Start();

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    CallTracker_StateChanged(IAosCallTracker::TYPE_NORMAL, CallState::OFFHOOK);
}

TEST_F(AosConditionTest, NetTracker_StatusChanged_ServiceIn)
{
    EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosNetTracker));

    EXPECT_CALL(m_objMockIAosNetTracker, IsServiceIn(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_TRUE));

    m_pAosCondition->Start();

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    SetAosBlockToCellur(static_cast<IAosBlock*>(&objMockIAosBlock));

    NetTracker_StatusChanged();
}

TEST_F(AosConditionTest, NetTracker_StatusChanged_ServiceOut)
{
    EXPECT_CALL(m_objMockIAosAppContext, GetNetTracker())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosNetTracker));

    EXPECT_CALL(m_objMockIAosNetTracker, IsServiceIn(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pAosCondition->Start();

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlockToCellur(static_cast<IAosBlock*>(&objMockIAosBlock));

    NetTracker_StatusChanged();
}

TEST_F(AosConditionTest, Subscriber_StateChanged_RefreshStarted)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(1);

    m_pAosCondition->SetListener(
            static_cast<IAosConditionListener*>(&objMockIAosConditionListener));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    Subscriber_StateChanged(IAosSubscriber::REFRESH_STARTED);
}

TEST_F(AosConditionTest, Block_Changed)
{
    m_pAosCondition->Start();
    RemoveListener(AosCondition::LISTENER_ALL);

    // Set IAosNConfiguration
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsVoLteAvailable()).WillRepeatedly(Return(IMS_TRUE));

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    SetAosBlockToCellur(static_cast<IAosBlock*>(&objMockIAosBlock));

    EXPECT_CALL(objMockIAosBlock, IsCleared(_)).Times(AnyNumber()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, PrintBlockReasons()).Times(1);

    Block_Changed(0, 0);
}

TEST_F(AosConditionTest, Subscriber_StateChanged_RefreshCompleted_RefreshStartedFalse)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(2);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    SetRefreshStarted(IMS_FALSE);
    EXPECT_FALSE(IsRefreshStarted());

    Subscriber_StateChanged(IAosSubscriber::REFRESH_COMPLETED);
}

TEST_F(AosConditionTest, Subscriber_StateChanged_RefreshCompleted_RefreshStartedTrue)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(3);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    SetRefreshStarted(IMS_TRUE);
    EXPECT_TRUE(IsRefreshStarted());

    Subscriber_StateChanged(IAosSubscriber::REFRESH_COMPLETED);
}

TEST_F(AosConditionTest, Subscriber_StateChanged_RefreshFailed)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    Subscriber_StateChanged(IAosSubscriber::REFRESH_FAILED);
}

TEST_F(AosConditionTest, ServiceAvailable_RequestCommand_ListenerIsNotNull)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(1);

    m_pAosCondition->SetListener(
            static_cast<IAosConditionListener*>(&objMockIAosConditionListener));

    ServiceAvailable_RequestCommand(AosCondition::REQUEST_PDN_DISCONNECT, 0);
}

TEST_F(AosConditionTest, NConfiguration_NotifyConfigChanged_NConfigIsNotNull)
{
    // Set IAosNConfiguration
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsVoLteRoamingAvailable())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(objMockIAosNConfiguration, IsWfcRoamingEnabled())
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    NConfiguration_NotifyConfigChanged();
}

TEST_F(AosConditionTest, NConfiguration_NotifyConfigChanged_NConfigIsNull)
{
    // Set IAosNConfiguration
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(IMS_NULL, 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsVoLteRoamingAvailable()).Times(0);

    EXPECT_CALL(objMockIAosNConfiguration, IsWfcRoamingEnabled()).Times(0);

    NConfiguration_NotifyConfigChanged();
}

TEST_F(AosConditionTest, ServicePhone_AosStart)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServicePhone_AosStart();
}

TEST_F(AosConditionTest, ServicePhone_LocationInfoChanged_Changed)
{
    m_pAosCondition->Start();

    // Set IAosNConfiguration
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsVoLteAvailable()).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosNConfiguration, IsWfcImsAvailable()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(objMockIAosNConfiguration, UseWfcCountryCodeAvailabilityCheck())
            .WillRepeatedly(Return(IMS_TRUE));

    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country_new");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    SetTestLocation(static_cast<ILocationProperties*>(&objMockILocationProperties));

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(AnyNumber());

    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(2);

    SetAosBlockToWifi(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServicePhone_LocationInfoChanged(LocationInfo::CHANGED);
}

TEST_F(AosConditionTest, ServicePhone_LocationInfoChanged_NotChanged)
{
    m_pAosCondition->Start();

    // Set IAosNConfiguration
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsVoLteAvailable()).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosNConfiguration, IsWfcImsAvailable()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(objMockIAosNConfiguration, UseWfcCountryCodeAvailabilityCheck())
            .WillRepeatedly(Return(IMS_TRUE));

    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country_new");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    SetTestLocation(static_cast<ILocationProperties*>(&objMockILocationProperties));

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(AnyNumber());

    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlockToWifi(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServicePhone_LocationInfoChanged(LocationInfo::AVAILABLE);
}

TEST_F(AosConditionTest, ServicePhone_LocationInfoChanged_ReturnByConfig)
{
    m_pAosCondition->Start();

    // Set IAosNConfiguration
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsVoLteAvailable()).WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosNConfiguration, IsWfcImsAvailable()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(objMockIAosNConfiguration, UseWfcCountryCodeAvailabilityCheck())
            .WillRepeatedly(Return(IMS_FALSE));

    // Set ILocationProperties
    MockILocationProperties objMockILocationProperties;

    AString strCountryNew = AString("test_country_new");
    EXPECT_CALL(objMockILocationProperties, GetCountry())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strCountryNew));

    SetTestLocation(static_cast<ILocationProperties*>(&objMockILocationProperties));

    // Set IAosBlock
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsCleared(_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosBlock, GetBlockReasons(_, _)).Times(AnyNumber());

    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlockToWifi(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServicePhone_LocationInfoChanged(LocationInfo::CHANGED);
}

TEST_F(AosConditionTest, ServicePhone_PhoneNumberStateChanged_RetryFailure)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServicePhone_PhoneNumberStateChanged(PhoneNumberState::RETRY_FAILURE);
}

TEST_F(AosConditionTest, ServicePhone_PhoneNumberStateChanged_ClearReasonSimState)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, GetClearReasonForPermanentPdnFailure())
            .WillRepeatedly(
                    Return(static_cast<IMS_UINT32>(IAosNConfiguration::ClearReason::SIM_STATE)));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServicePhone_PhoneNumberStateChanged(PhoneNumberState::RETRY_SUCCESS);
}

TEST_F(AosConditionTest, ServicePhone_PhoneNumberStateChanged_ClearReasonNotSimState)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, GetClearReasonForPermanentPdnFailure())
            .WillRepeatedly(
                    Return(static_cast<IMS_UINT32>(IAosNConfiguration::ClearReason::AIRPLANE)));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServicePhone_PhoneNumberStateChanged(PhoneNumberState::RETRY_SUCCESS);
}

TEST_F(AosConditionTest, ServicePhone_PlmnChanged_ClearReaconPlmlChanged)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, GetClearReasonForPermanentPdnFailure())
            .WillRepeatedly(
                    Return(static_cast<IMS_UINT32>(IAosNConfiguration::ClearReason::PLMN_CHANGED)));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServicePhone_PlmnChanged();
}

TEST_F(AosConditionTest, ServicePhone_PlmnChanged_ClearReaconNotPlmlChanged)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, GetClearReasonForPermanentPdnFailure())
            .WillRepeatedly(Return(0));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServicePhone_PlmnChanged();
}

TEST_F(AosConditionTest, ServicePhone_PowerOff_ListenerIsNull)
{
    m_pAosCondition->SetListener(IMS_NULL);

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServicePhone_PowerOff();
}

TEST_F(AosConditionTest, ServicePhone_PowerOff_ListenerIsNotNull)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(1);

    m_pAosCondition->SetListener(
            static_cast<IAosConditionListener*>(&objMockIAosConditionListener));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServicePhone_PowerOff();
}

TEST_F(AosConditionTest, ServiceSetting_AirplaneChanged_True_MatchedClearReason)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(1);

    m_pAosCondition->SetListener(
            static_cast<IAosConditionListener*>(&objMockIAosConditionListener));

    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, GetClearReasonForPermanentPdnFailure())
            .WillRepeatedly(
                    Return(static_cast<IMS_UINT32>(IAosNConfiguration::ClearReason::AIRPLANE)));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(2);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServiceSetting_AirplaneChanged(IMS_TRUE);
}

TEST_F(AosConditionTest, ServiceSetting_AirplaneChanged_True_NotMatchedClearReason)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(1);

    m_pAosCondition->SetListener(
            static_cast<IAosConditionListener*>(&objMockIAosConditionListener));

    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, GetClearReasonForPermanentPdnFailure())
            .WillRepeatedly(Return(0));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServiceSetting_AirplaneChanged(IMS_TRUE);
}

TEST_F(AosConditionTest, ServiceSetting_AirplaneChanged_False)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(0);

    m_pAosCondition->SetListener(
            static_cast<IAosConditionListener*>(&objMockIAosConditionListener));

    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, GetClearReasonForPermanentPdnFailure())
            .WillRepeatedly(
                    Return(static_cast<IMS_UINT32>(IAosNConfiguration::ClearReason::AIRPLANE)));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServiceSetting_AirplaneChanged(IMS_FALSE);
}

TEST_F(AosConditionTest, ServiceSetting_ServiceChanged_HoldEvent)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    AddHold(HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
    EXPECT_TRUE(IsHolded(HOLD_EVENT_IMS_SERVICE));

    ServiceSetting_ServiceChanged(ServiceSetting::ON);
    ServiceSetting_ServiceChanged(ServiceSetting::OFF);
}

TEST_F(AosConditionTest, ServiceSetting_ServiceChanged_On)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(3);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    RemoveHold(HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
    EXPECT_FALSE(IsHolded(HOLD_EVENT_IMS_SERVICE));

    ServiceSetting_ServiceChanged(ServiceSetting::ON);
}

TEST_F(AosConditionTest, ServiceSetting_ServiceChanged_Off)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    RemoveHold(HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
    EXPECT_FALSE(IsHolded(HOLD_EVENT_IMS_SERVICE));

    ServiceSetting_ServiceChanged(ServiceSetting::OFF);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_On_RttNotSupport)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(objMockIAosNConfiguration, IsRttSupported()).WillRepeatedly(Return(IMS_FALSE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServiceSetting_TtyChanged(IMS_TRUE);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_True_CombindAttached)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(objMockIAosNConfiguration, IsRttSupported()).WillRepeatedly(Return(IMS_TRUE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(1);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    SetLteInfo(IMS_TRUE);
    ServiceSetting_TtyChanged(IMS_TRUE);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_False)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_TRUE));

    EXPECT_CALL(objMockIAosNConfiguration, IsRttSupported()).WillRepeatedly(Return(IMS_FALSE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(1);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    SetLteInfo(IMS_TRUE);
    ServiceSetting_TtyChanged(IMS_FALSE);
}

TEST_F(AosConditionTest, ServiceSetting_TtyChanged_TtyNotSupport)
{
    MockIAosNConfiguration objMockIAosNConfiguration;
    AosProvider::GetInstance()->SetNConfiguration(
            static_cast<IAosNConfiguration*>(&objMockIAosNConfiguration), 0);

    EXPECT_CALL(objMockIAosNConfiguration, IsTtySupported()).WillRepeatedly(Return(IMS_FALSE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    ServiceSetting_TtyChanged(IMS_TRUE);
    ServiceSetting_TtyChanged(IMS_FALSE);
}

TEST_F(AosConditionTest, AddListener)
{
    RemoveListener(AosCondition::LISTENER_ALL);
    EXPECT_EQ(GetListener(), AosCondition::LISTENER_NONE);

    AddListener(AosCondition::LISTENER_BLOCK);
    EXPECT_EQ(GetListener(), AosCondition::LISTENER_BLOCK);

    AddListener(AosCondition::LISTENER_NETTRACKER);
    EXPECT_EQ(GetListener(), AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER);

    AddListener(AosCondition::LISTENER_SUBSCRIBER);
    EXPECT_EQ(GetListener(),
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER);

    AddListener(AosCondition::LISTENER_CALLTRACKER);
    EXPECT_EQ(GetListener(),
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER | AosCondition::LISTENER_CALLTRACKER);

    RemoveListener(AosCondition::LISTENER_ALL);
    EXPECT_EQ(GetListener(), AosCondition::LISTENER_NONE);

    AddListener(AosCondition::LISTENER_ALL);
    EXPECT_EQ(GetListener(), AosCondition::LISTENER_ALL);
}

TEST_F(AosConditionTest, RemoveListener)
{
    RemoveListener(AosCondition::LISTENER_ALL);

    AddListener(AosCondition::LISTENER_BLOCK);
    AddListener(AosCondition::LISTENER_NETTRACKER);
    AddListener(AosCondition::LISTENER_SUBSCRIBER);
    AddListener(AosCondition::LISTENER_CALLTRACKER);

    EXPECT_EQ(GetListener(),
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER | AosCondition::LISTENER_CALLTRACKER);

    RemoveListener(AosCondition::LISTENER_CALLTRACKER);
    EXPECT_EQ(GetListener(),
            AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER |
                    AosCondition::LISTENER_SUBSCRIBER);

    RemoveListener(AosCondition::LISTENER_SUBSCRIBER);
    EXPECT_EQ(GetListener(), AosCondition::LISTENER_BLOCK | AosCondition::LISTENER_NETTRACKER);

    RemoveListener(AosCondition::LISTENER_NETTRACKER);
    EXPECT_EQ(GetListener(), AosCondition::LISTENER_BLOCK);

    RemoveListener(AosCondition::LISTENER_BLOCK);
    EXPECT_EQ(GetListener(), AosCondition::LISTENER_NONE);
}

TEST_F(AosConditionTest, IsListenerEnabled)
{
    RemoveListener(AosCondition::LISTENER_ALL);

    AddListener(AosCondition::LISTENER_BLOCK);
    AddListener(AosCondition::LISTENER_NETTRACKER);
    AddListener(AosCondition::LISTENER_SUBSCRIBER);
    AddListener(AosCondition::LISTENER_CALLTRACKER);

    EXPECT_TRUE(IsListenerEnabled(AosCondition::LISTENER_BLOCK));
    EXPECT_TRUE(IsListenerEnabled(AosCondition::LISTENER_NETTRACKER));
    EXPECT_TRUE(IsListenerEnabled(AosCondition::LISTENER_SUBSCRIBER));
    EXPECT_TRUE(IsListenerEnabled(AosCondition::LISTENER_CALLTRACKER));

    RemoveListener(AosCondition::LISTENER_BLOCK);
    EXPECT_FALSE(IsListenerEnabled(AosCondition::LISTENER_BLOCK));
}

TEST_F(AosConditionTest, AddHold_Roaming)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(BLOCK_CELLULAR_ROAMING, _)).Times(1);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    AddHold(HOLD_EVENT_ROAMING, IMS_TRUE);
}

TEST_F(AosConditionTest, AddHold_ImsService)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(BLOCK_IMS_DISABLED, _)).Times(1);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    AddHold(HOLD_EVENT_IMS_SERVICE, IMS_TRUE);
}

TEST_F(AosConditionTest, AddHold_IsNotEventReset)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    AddHold(HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
}

TEST_F(AosConditionTest, AddHold_UninterestingEvent)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    AddHold(HOLD_EVENT_NONE, IMS_FALSE);
}

TEST_F(AosConditionTest, RemoveHold_Roaming)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(BLOCK_CELLULAR_ROAMING, _)).Times(1);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    RemoveHold(HOLD_EVENT_ROAMING, IMS_TRUE);
}

TEST_F(AosConditionTest, RemoveHold_ImsService)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(BLOCK_IMS_DISABLED, _)).Times(1);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    RemoveHold(HOLD_EVENT_IMS_SERVICE, IMS_TRUE);
}

TEST_F(AosConditionTest, RemoveHold_IsNotEventReset)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    RemoveHold(HOLD_EVENT_IMS_SERVICE, IMS_FALSE);
}

TEST_F(AosConditionTest, RemoveHold_UninterestingEvent)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, SetBlockReason(_, _)).Times(0);
    EXPECT_CALL(objMockIAosBlock, ResetBlockReason(_, _)).Times(0);

    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    RemoveHold(HOLD_EVENT_NONE, IMS_FALSE);
}

TEST_F(AosConditionTest, RequestCommand_ListenerIsNull)
{
    m_pAosCondition->SetListener(IMS_NULL);

    EXPECT_FALSE(RequestCommand(AosCondition::REQUEST_STOP, 0));
}

TEST_F(AosConditionTest, RequestCommand_ListenerIsNotNull)
{
    MockIAosConditionListener objMockIAosConditionListener;
    EXPECT_CALL(objMockIAosConditionListener, Condition_RequestCommand(_, _)).Times(1);

    m_pAosCondition->SetListener(
            static_cast<IAosConditionListener*>(&objMockIAosConditionListener));

    EXPECT_TRUE(RequestCommand(AosCondition::REQUEST_STOP, 0));
}

TEST_F(AosConditionTest, UpdateRegistrationMode_ImpuCountIsGreaterThanOne)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_TRUE));
    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosSubscriber));

    EXPECT_CALL(m_objMockIAosSubscriber, IsReady()).WillRepeatedly(Return(IMS_TRUE));

    AStringArray objImpus;
    objImpus.AddElement(AString("PUID1"));
    objImpus.AddElement(AString("PUID2"));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillRepeatedly(ReturnRef(objImpus));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, IAosRegistration::MODE_NORMAL)).Times(1);

    UpdateRegistrationMode();
}

TEST_F(AosConditionTest, UpdateRegistrationMode_ImpuCountIsOne)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_TRUE));
    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosSubscriber));

    EXPECT_CALL(m_objMockIAosSubscriber, IsReady()).WillRepeatedly(Return(IMS_TRUE));

    AStringArray objImpus;
    objImpus.AddElement(AString("PUID1"));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillRepeatedly(ReturnRef(objImpus));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, IAosRegistration::MODE_LIMITED)).Times(1);

    UpdateRegistrationMode();
}

TEST_F(AosConditionTest, UpdateRegistrationMode_NoBlockReason)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_FALSE));
    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosSubscriber));

    EXPECT_CALL(m_objMockIAosSubscriber, IsReady()).WillRepeatedly(Return(IMS_TRUE));

    AStringArray objImpus;
    objImpus.AddElement(AString("PUID1"));
    objImpus.AddElement(AString("PUID2"));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillRepeatedly(ReturnRef(objImpus));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, _)).Times(0);

    UpdateRegistrationMode();
}

TEST_F(AosConditionTest, UpdateRegistrationMode_IsNotReady)
{
    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(_, _, _)).WillRepeatedly(Return(IMS_TRUE));
    SetAosBlock(static_cast<IAosBlock*>(&objMockIAosBlock));

    EXPECT_CALL(m_objMockIAosAppContext, GetSubscriber())
            .Times(AnyNumber())
            .WillRepeatedly(Return(&m_objMockIAosSubscriber));

    EXPECT_CALL(m_objMockIAosSubscriber, IsReady()).WillRepeatedly(Return(IMS_FALSE));

    AStringArray objImpus;
    objImpus.AddElement(AString("PUID1"));
    objImpus.AddElement(AString("PUID2"));

    EXPECT_CALL(m_objMockIAosSubscriber, GetConfiguredImpus()).WillRepeatedly(ReturnRef(objImpus));

    EXPECT_CALL(m_objMockIAosRegistration, RequestCmd(_, _)).Times(0);

    UpdateRegistrationMode();
}