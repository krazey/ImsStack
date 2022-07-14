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
#include "condition/AosBlock.h"
#include "condition/AosCondition.h"
#include "condition/AosServiceAvailable.h"
#include "condition/AosServiceAvailableCellular.h"
#include "condition/AosServiceAvailableWifi.h"
#include "provider/AosStaticProfile.h"

#include "app/MockAosAppContext.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosConditionListener.h"

class IAosConditionListener;

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_UINT32 REASON_MAX_SIZE_WHOLE = 26;
const IMS_UINT32 REASON_MAX_SIZE_CELLULAR = 19;
const IMS_UINT32 REASON_MAX_SIZE_WIFI = 20;

class AosConditionTest : public ::testing::Test
{
public:
    AosCondition* m_pAosCondition;
    AosBlock* m_pAosBlock;
    AosStaticProfile* m_pAosStaticProfile;
    MockAosAppContext* m_pMockAosAppContext;

protected:
    virtual void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pMockAosAppContext = new MockAosAppContext(m_pAosStaticProfile);

        EXPECT_CALL(*m_pMockAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(*m_pMockAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(*m_pMockAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        EXPECT_CALL(*m_pMockAosAppContext, GetRegistration())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        EXPECT_CALL(*m_pMockAosAppContext, GetBlock())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        EXPECT_CALL(*m_pMockAosAppContext, GetNetTracker())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        EXPECT_CALL(*m_pMockAosAppContext, GetSubscriber())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnNull());

        m_pAosCondition = new AosCondition(static_cast<IAosAppContext*>(m_pMockAosAppContext));
        ASSERT_TRUE(m_pAosCondition != nullptr);

        m_pAosBlock = new AosBlock(static_cast<IAosAppContext*>(m_pMockAosAppContext));
        ASSERT_TRUE(m_pAosBlock != nullptr);

        IAosBlock* piAosBlock = static_cast<IAosBlock*>(m_pAosBlock);
        SetAosBlock(piAosBlock);
    }

    virtual void TearDown() override
    {
        if (m_pMockAosAppContext)
        {
            delete m_pMockAosAppContext;
        }

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }

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

    void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState)
    {
        m_pAosCondition->CallTracker_StateChanged(nType, eState);
    }
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
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_VOLTE_OFF);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_VOPS_OFF);
    m_pAosCondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosCondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosCondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_WIFI_NO_WIFI);
    m_pAosCondition->SetBlock(BLOCK_WIFI_ROAMING);
    m_pAosCondition->SetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    m_pAosCondition->SetBlock(BLOCK_WIFI_VOWIFI_OFF);

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
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_VOLTE_OFF));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_VOPS_OFF));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_BAD_CONNECTION));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_AIRPLANE_MODE_ON));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_NO_WIFI));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_ROAMING));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_TEMPORARILY_BLOCKED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_VOWIFI_OFF));

    IMSList<IMS_UINT32> objReason;
    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_WHOLE);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_CELLULAR);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_WIFI);
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
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_VOLTE_OFF);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_VOPS_OFF);
    m_pAosCondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosCondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosCondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_WIFI_NO_WIFI);
    m_pAosCondition->SetBlock(BLOCK_WIFI_ROAMING);
    m_pAosCondition->SetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    m_pAosCondition->SetBlock(BLOCK_WIFI_VOWIFI_OFF);

    IMSList<IMS_UINT32> objReason;
    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_WHOLE);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_CELLULAR);

    m_pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_WIFI);

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
    m_pAosCondition->ResetBlock(BLOCK_CELLULAR_VOLTE_OFF);
    m_pAosCondition->ResetBlock(BLOCK_CELLULAR_VOPS_OFF);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_NO_WIFI);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_ROAMING);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    m_pAosCondition->ResetBlock(BLOCK_WIFI_VOWIFI_OFF);

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
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_VOLTE_OFF);
    m_pAosCondition->SetBlock(BLOCK_CELLULAR_VOPS_OFF);
    m_pAosCondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    m_pAosCondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    m_pAosCondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    m_pAosCondition->SetBlock(BLOCK_WIFI_NO_WIFI);
    m_pAosCondition->SetBlock(BLOCK_WIFI_ROAMING);
    m_pAosCondition->SetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    m_pAosCondition->SetBlock(BLOCK_WIFI_VOWIFI_OFF);

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
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_VOLTE_OFF));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_VOPS_OFF));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_BAD_CONNECTION));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_AIRPLANE_MODE_ON));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_NO_WIFI));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_ROAMING));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_TEMPORARILY_BLOCKED));
    EXPECT_TRUE(m_pAosCondition->IsReasonBlocked(BLOCK_WIFI_VOWIFI_OFF));
}

TEST_F(AosConditionTest, IsReady)
{
    // Cellular : Available, WiFi : Not Available
    SetServiceAvailable(SERVICE_CELLULAR, IMS_TRUE);

    SetServiceType(SERVICE_CELLULAR);
    EXPECT_TRUE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WHOLE);
    EXPECT_TRUE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WIFI);
    EXPECT_FALSE(m_pAosCondition->IsReady());

    // Cellular : Available, WiFi : Available
    SetServiceAvailable(SERVICE_WIFI, IMS_TRUE);

    SetServiceType(SERVICE_CELLULAR);
    EXPECT_TRUE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WHOLE);
    EXPECT_TRUE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WIFI);
    EXPECT_TRUE(m_pAosCondition->IsReady());

    // Cellular : Not Available, WiFi : Available
    SetServiceAvailable(SERVICE_CELLULAR, IMS_FALSE);

    SetServiceType(SERVICE_CELLULAR);
    EXPECT_FALSE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WHOLE);
    EXPECT_TRUE(m_pAosCondition->IsReady());

    SetServiceType(SERVICE_WIFI);
    EXPECT_TRUE(m_pAosCondition->IsReady());

    // Cellular : Not Available, WiFi : Not Available
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