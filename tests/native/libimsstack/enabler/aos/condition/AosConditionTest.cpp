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

#include "app/MockAosAppContext.h"
#include "interface/MockIAosConditionListener.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "condition/AosBlock.h"
#include "condition/AosCondition.h"
#include "condition/AosServiceAvailable.h"
#include "condition/AosServiceAvailableCellular.h"
#include "condition/AosServiceAvailableWifi.h"
#include "provider/AosStaticProfile.h"

class IAosConditionListener;

using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_UINT32 REASON_MAX_SIZE_WHOLE = 26;
const IMS_UINT32 REASON_MAX_SIZE_CELLULAR = 19;
const IMS_UINT32 REASON_MAX_SIZE_WIFI = 20;

class AosConditionTest : public ::testing::Test {
public:
    AosCondition* pAosCondition;
    AosBlock* pAosBlock;

protected:
    virtual void SetUp() override {
        AosStaticProfile* pAosStaticProfile = new AosStaticProfile();
        MockAosAppContext* pMockAosAppContext = new MockAosAppContext(pAosStaticProfile);

        EXPECT_CALL(*pMockAosAppContext, GetSlotId())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(*pMockAosAppContext, GetProfileId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(*pMockAosAppContext, GetConnection())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnNull());

        EXPECT_CALL(*pMockAosAppContext, GetBlock())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnNull());

        EXPECT_CALL(*pMockAosAppContext, GetNetTracker())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnNull());

        EXPECT_CALL(*pMockAosAppContext, GetSubscriber())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnNull());

        pAosCondition = new AosCondition(static_cast<IAosAppContext*>(pMockAosAppContext));
        ASSERT_TRUE(pAosCondition != nullptr);

        pAosBlock = new AosBlock(static_cast<IAosAppContext*>(pMockAosAppContext));
        ASSERT_TRUE(pAosBlock != nullptr);

        IAosBlock* piAosBlock = static_cast<IAosBlock*>(pAosBlock);
        SetAosBlock(piAosBlock);
    }

    virtual void TearDown() override {
        if (pAosBlock) {
            delete pAosBlock;
        }
        if (pAosCondition) {
            delete pAosCondition;
        }
    }

    void SetAosBlock(IN IAosBlock* piBlock) {
        pAosCondition->m_piBlock = piBlock;
    }

    void SetServiceAvailable(IN SERVICE_TYPE eType, IN IMS_BOOL bIsAvailable) {
        switch (eType) {
            case SERVICE_CELLULAR:
                pAosCondition->m_bCellServiceAvailable = bIsAvailable;
                break;
            case SERVICE_WIFI:
                pAosCondition->m_bWiFiServiceAvailable = bIsAvailable;
                break;
            default:
                break;
        }
    }

    AosServiceAvailable* GetServiceAvailable(IN SERVICE_TYPE eType) {
        switch (eType) {
            case SERVICE_CELLULAR:
                return dynamic_cast<AosServiceAvailable*>(pAosCondition->m_pAvailableCellular);
            case SERVICE_WIFI:
                return dynamic_cast<AosServiceAvailable*>(pAosCondition->m_pAvailableWiFi);
            default:
                return nullptr;
        }
    }

    void SetServiceType(IN SERVICE_TYPE eType) {
        pAosCondition->m_eServiceType = eType;
    }


    IAosConditionListener* GetConditionListener() {
        return pAosCondition->m_piListener;
    }

    IMS_UINT32 GetListener() {
        return pAosCondition->m_nListeners;
    }
};

TEST_F(AosConditionTest, Constructor) {
    EXPECT_EQ(GetListener(), AosCondition::LISTENER_ALL);
}

TEST_F(AosConditionTest, Start) {
    EXPECT_EQ(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_EQ(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_FALSE(pAosCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
    EXPECT_FALSE(pAosCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));

    pAosCondition->Start();
    EXPECT_NE(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_NE(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));
}

TEST_F(AosConditionTest, Stop) {
    pAosCondition->Start();
    EXPECT_NE(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_NE(GetServiceAvailable(SERVICE_WIFI), nullptr);

    pAosCondition->Stop();
    EXPECT_EQ(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_EQ(GetServiceAvailable(SERVICE_WIFI), nullptr);
}

TEST_F(AosConditionTest, SetListener) {
    EXPECT_EQ(GetConditionListener(), nullptr);

    IAosConditionListener* piAosConditionListener = new MockIAosConditionListener();
    pAosCondition->SetListener(piAosConditionListener);
    EXPECT_EQ(GetConditionListener(), piAosConditionListener);
}

TEST_F(AosConditionTest, SetBlock) {
    ASSERT_TRUE(pAosBlock->IsCleared());

    pAosCondition->SetBlock(BLOCK_AC_INCOMPLETED);
    pAosCondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);
    pAosCondition->SetBlock(BLOCK_AOS_INCOMPLETED);
    pAosCondition->SetBlock(BLOCK_CSCALL_STARTED);
    pAosCondition->SetBlock(BLOCK_PERMANENT_DATA_FAILED);
    pAosCondition->SetBlock(BLOCK_ENABLER_DETACHED);
    pAosCondition->SetBlock(BLOCK_IMS_DISABLED);
    pAosCondition->SetBlock(BLOCK_PERMANENT_REG_FAILED);
    pAosCondition->SetBlock(BLOCK_POWER_OFF);
    pAosCondition->SetBlock(BLOCK_SERVICE_CONNECTING);
    pAosCondition->SetBlock(BLOCK_SUBSCRIBER_INCOMPLETED);
    pAosCondition->SetBlock(BLOCK_TTY_MODE_ON);
    pAosCondition->SetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    pAosCondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    pAosCondition->SetBlock(BLOCK_CELLULAR_NO_NETWORK);
    pAosCondition->SetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    pAosCondition->SetBlock(BLOCK_CELLULAR_ROAMING);
    pAosCondition->SetBlock(BLOCK_CELLULAR_VOLTE_OFF);
    pAosCondition->SetBlock(BLOCK_CELLULAR_VOPS_OFF);
    pAosCondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    pAosCondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    pAosCondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    pAosCondition->SetBlock(BLOCK_WIFI_NO_WIFI);
    pAosCondition->SetBlock(BLOCK_WIFI_ROAMING);
    pAosCondition->SetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    pAosCondition->SetBlock(BLOCK_WIFI_VOWIFI_OFF);

    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_AC_INCOMPLETED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CSCALL_STARTED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_ENABLER_DETACHED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_IMS_DISABLED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_POWER_OFF));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_SERVICE_CONNECTING));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_TTY_MODE_ON));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_TEMPORARY_DATA_DEACTIVATED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_AIRPLANE_MODE_ON));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_NO_NETWORK));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_OUT_OF_SERVICE));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_ROAMING));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_VOLTE_OFF));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_VOPS_OFF));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_BAD_CONNECTION));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_AIRPLANE_MODE_ON));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_NO_WIFI));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_ROAMING));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_TEMPORARILY_BLOCKED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_VOWIFI_OFF));

    IMSList<IMS_UINT32> objReason;
    pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_WHOLE);

    pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_CELLULAR);

    pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_WIFI);
}

TEST_F(AosConditionTest, ResetBlock) {
    ASSERT_TRUE(pAosBlock->IsCleared());

    pAosCondition->SetBlock(BLOCK_AC_INCOMPLETED);
    pAosCondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);
    pAosCondition->SetBlock(BLOCK_AOS_INCOMPLETED);
    pAosCondition->SetBlock(BLOCK_CSCALL_STARTED);
    pAosCondition->SetBlock(BLOCK_PERMANENT_DATA_FAILED);
    pAosCondition->SetBlock(BLOCK_ENABLER_DETACHED);
    pAosCondition->SetBlock(BLOCK_IMS_DISABLED);
    pAosCondition->SetBlock(BLOCK_PERMANENT_REG_FAILED);
    pAosCondition->SetBlock(BLOCK_POWER_OFF);
    pAosCondition->SetBlock(BLOCK_SERVICE_CONNECTING);
    pAosCondition->SetBlock(BLOCK_SUBSCRIBER_INCOMPLETED);
    pAosCondition->SetBlock(BLOCK_TTY_MODE_ON);
    pAosCondition->SetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    pAosCondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    pAosCondition->SetBlock(BLOCK_CELLULAR_NO_NETWORK);
    pAosCondition->SetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    pAosCondition->SetBlock(BLOCK_CELLULAR_ROAMING);
    pAosCondition->SetBlock(BLOCK_CELLULAR_VOLTE_OFF);
    pAosCondition->SetBlock(BLOCK_CELLULAR_VOPS_OFF);
    pAosCondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    pAosCondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    pAosCondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    pAosCondition->SetBlock(BLOCK_WIFI_NO_WIFI);
    pAosCondition->SetBlock(BLOCK_WIFI_ROAMING);
    pAosCondition->SetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    pAosCondition->SetBlock(BLOCK_WIFI_VOWIFI_OFF);

    IMSList<IMS_UINT32> objReason;
    pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_WHOLE);

    pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_CELLULAR);

    pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(objReason.GetSize(), REASON_MAX_SIZE_WIFI);

    pAosCondition->ResetBlock(BLOCK_AC_INCOMPLETED);
    pAosCondition->ResetBlock(BLOCK_AUTHENTICATION_FAILED);
    pAosCondition->ResetBlock(BLOCK_AOS_INCOMPLETED);
    pAosCondition->ResetBlock(BLOCK_CSCALL_STARTED);
    pAosCondition->ResetBlock(BLOCK_PERMANENT_DATA_FAILED);
    pAosCondition->ResetBlock(BLOCK_ENABLER_DETACHED);
    pAosCondition->ResetBlock(BLOCK_IMS_DISABLED);
    pAosCondition->ResetBlock(BLOCK_PERMANENT_REG_FAILED);
    pAosCondition->ResetBlock(BLOCK_POWER_OFF);
    pAosCondition->ResetBlock(BLOCK_SERVICE_CONNECTING);
    pAosCondition->ResetBlock(BLOCK_SUBSCRIBER_INCOMPLETED);
    pAosCondition->ResetBlock(BLOCK_TTY_MODE_ON);
    pAosCondition->ResetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    pAosCondition->ResetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    pAosCondition->ResetBlock(BLOCK_CELLULAR_NO_NETWORK);
    pAosCondition->ResetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    pAosCondition->ResetBlock(BLOCK_CELLULAR_ROAMING);
    pAosCondition->ResetBlock(BLOCK_CELLULAR_VOLTE_OFF);
    pAosCondition->ResetBlock(BLOCK_CELLULAR_VOPS_OFF);
    pAosCondition->ResetBlock(BLOCK_WIFI_BAD_CONNECTION);
    pAosCondition->ResetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    pAosCondition->ResetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    pAosCondition->ResetBlock(BLOCK_WIFI_NO_WIFI);
    pAosCondition->ResetBlock(BLOCK_WIFI_ROAMING);
    pAosCondition->ResetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    pAosCondition->ResetBlock(BLOCK_WIFI_VOWIFI_OFF);

    EXPECT_TRUE(pAosBlock->IsCleared());
}

TEST_F(AosConditionTest, IsReasonBlocked) {
    ASSERT_TRUE(pAosBlock->IsCleared());

    pAosCondition->SetBlock(BLOCK_AC_INCOMPLETED);
    pAosCondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);
    pAosCondition->SetBlock(BLOCK_AOS_INCOMPLETED);
    pAosCondition->SetBlock(BLOCK_CSCALL_STARTED);
    pAosCondition->SetBlock(BLOCK_PERMANENT_DATA_FAILED);
    pAosCondition->SetBlock(BLOCK_ENABLER_DETACHED);
    pAosCondition->SetBlock(BLOCK_IMS_DISABLED);
    pAosCondition->SetBlock(BLOCK_PERMANENT_REG_FAILED);
    pAosCondition->SetBlock(BLOCK_POWER_OFF);
    pAosCondition->SetBlock(BLOCK_SERVICE_CONNECTING);
    pAosCondition->SetBlock(BLOCK_SUBSCRIBER_INCOMPLETED);
    pAosCondition->SetBlock(BLOCK_TTY_MODE_ON);
    pAosCondition->SetBlock(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    pAosCondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    pAosCondition->SetBlock(BLOCK_CELLULAR_NO_NETWORK);
    pAosCondition->SetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    pAosCondition->SetBlock(BLOCK_CELLULAR_ROAMING);
    pAosCondition->SetBlock(BLOCK_CELLULAR_VOLTE_OFF);
    pAosCondition->SetBlock(BLOCK_CELLULAR_VOPS_OFF);
    pAosCondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    pAosCondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    pAosCondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    pAosCondition->SetBlock(BLOCK_WIFI_NO_WIFI);
    pAosCondition->SetBlock(BLOCK_WIFI_ROAMING);
    pAosCondition->SetBlock(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    pAosCondition->SetBlock(BLOCK_WIFI_VOWIFI_OFF);

    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_AC_INCOMPLETED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CSCALL_STARTED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_ENABLER_DETACHED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_IMS_DISABLED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_POWER_OFF));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_SERVICE_CONNECTING));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_TTY_MODE_ON));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_TEMPORARY_DATA_DEACTIVATED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_AIRPLANE_MODE_ON));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_NO_NETWORK));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_OUT_OF_SERVICE));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_ROAMING));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_VOLTE_OFF));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_CELLULAR_VOPS_OFF));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_BAD_CONNECTION));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_AIRPLANE_MODE_ON));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_NO_WIFI));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_ROAMING));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_TEMPORARILY_BLOCKED));
    EXPECT_TRUE(pAosCondition->IsReasonBlocked(BLOCK_WIFI_VOWIFI_OFF));
}

TEST_F(AosConditionTest, IsReady) {
    // Cellular : Available, WiFi : Not Available
    SetServiceAvailable(SERVICE_CELLULAR, IMS_TRUE);

    SetServiceType(SERVICE_CELLULAR);
    EXPECT_TRUE(pAosCondition->IsReady());

    SetServiceType(SERVICE_WHOLE);
    EXPECT_TRUE(pAosCondition->IsReady());

    SetServiceType(SERVICE_WIFI);
    EXPECT_FALSE(pAosCondition->IsReady());

    // Cellular : Available, WiFi : Available
    SetServiceAvailable(SERVICE_WIFI, IMS_TRUE);

    SetServiceType(SERVICE_CELLULAR);
    EXPECT_TRUE(pAosCondition->IsReady());

    SetServiceType(SERVICE_WHOLE);
    EXPECT_TRUE(pAosCondition->IsReady());

    SetServiceType(SERVICE_WIFI);
    EXPECT_TRUE(pAosCondition->IsReady());

    // Cellular : Not Available, WiFi : Available
    SetServiceAvailable(SERVICE_CELLULAR, IMS_FALSE);

    SetServiceType(SERVICE_CELLULAR);
    EXPECT_FALSE(pAosCondition->IsReady());

    SetServiceType(SERVICE_WHOLE);
    EXPECT_TRUE(pAosCondition->IsReady());

    SetServiceType(SERVICE_WIFI);
    EXPECT_TRUE(pAosCondition->IsReady());

    // Cellular : Not Available, WiFi : Not Available
    SetServiceAvailable(SERVICE_WIFI, IMS_FALSE);

    SetServiceType(SERVICE_CELLULAR);
    EXPECT_FALSE(pAosCondition->IsReady());

    SetServiceType(SERVICE_WHOLE);
    EXPECT_FALSE(pAosCondition->IsReady());

    SetServiceType(SERVICE_WIFI);
    EXPECT_FALSE(pAosCondition->IsReady());
}

TEST_F(AosConditionTest, CheckServiceAvailable) {
    EXPECT_EQ(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_EQ(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_EQ(pAosCondition->CheckServiceAvailable(SERVICE_WHOLE), AosCondition::CHECK_NONE);
    EXPECT_EQ(pAosCondition->CheckServiceAvailable(SERVICE_CELLULAR), AosCondition::CHECK_NONE);
    EXPECT_EQ(pAosCondition->CheckServiceAvailable(SERVICE_WIFI), AosCondition::CHECK_NONE);

    pAosCondition->Start();
    EXPECT_NE(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_NE(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_EQ(pAosCondition->CheckServiceAvailable(SERVICE_WHOLE),
            AosCondition::CHECK_CELLULAR | AosCondition::CHECK_WIFI);
    EXPECT_EQ(pAosCondition->CheckServiceAvailable(SERVICE_CELLULAR),
            AosCondition::CHECK_CELLULAR);
    EXPECT_EQ(pAosCondition->CheckServiceAvailable(SERVICE_WIFI), AosCondition::CHECK_WIFI);
}

TEST_F(AosConditionTest, CheckBadNetwork) {
    EXPECT_EQ(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_EQ(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_FALSE(pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_FALSE(pAosCondition->CheckBadNetwork(SERVICE_WIFI));

    pAosCondition->Start();
    EXPECT_NE(GetServiceAvailable(SERVICE_CELLULAR), nullptr);
    EXPECT_NE(GetServiceAvailable(SERVICE_WIFI), nullptr);
    EXPECT_FALSE(pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_FALSE(pAosCondition->CheckBadNetwork(SERVICE_WIFI));

    SetServiceAvailable(SERVICE_CELLULAR, IMS_TRUE);
    EXPECT_FALSE(pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_TRUE(pAosCondition->CheckBadNetwork(SERVICE_WIFI));

    SetServiceAvailable(SERVICE_CELLULAR, IMS_FALSE);
    EXPECT_FALSE(pAosCondition->CheckBadNetwork(SERVICE_WHOLE));
    EXPECT_FALSE(pAosCondition->CheckBadNetwork(SERVICE_CELLULAR));
    EXPECT_FALSE(pAosCondition->CheckBadNetwork(SERVICE_WIFI));
}

// Testing is not required.
// TEST_F(AosConditionTest, PrintBlockReasons) {
// }