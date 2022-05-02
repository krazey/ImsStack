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

#include "interface/MockIAosBlockListener.h"
#include "app/MockAosAppContext.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosBlockListener.h"
#include "condition/AosBlock.h"
#include "provider/AosStaticProfile.h"

using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_UINT32 REASON_MAX_SIZE_WHOLE = 26;
const IMS_UINT32 REASON_MAX_SIZE_CELLULAR = 19;
const IMS_UINT32 REASON_MAX_SIZE_WIFI = 20;

class AosBlockTest : public ::testing::Test {
public:
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

        pAosBlock = new AosBlock(static_cast<IAosAppContext*>(pMockAosAppContext));
        ASSERT_TRUE(pAosBlock != nullptr);
    }

    virtual void TearDown() override {
        if(pAosBlock){
            delete pAosBlock;
        }
    }
};

TEST_F(AosBlockTest, SetListener) {
    IAosBlockListener* piAosBlockListener1 = new MockIAosBlockListener();
    IAosBlockListener* piAosBlockListener2 = new MockIAosBlockListener();
    IAosBlockListener* piAosBlockListener3 = new MockIAosBlockListener();

    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener1));
    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener2));
    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener3));

    pAosBlock->SetListener(piAosBlockListener1);
    pAosBlock->SetListener(piAosBlockListener2);
    pAosBlock->SetListener(piAosBlockListener3);

    EXPECT_TRUE(pAosBlock->IsListened(piAosBlockListener1));
    EXPECT_TRUE(pAosBlock->IsListened(piAosBlockListener2));
    EXPECT_TRUE(pAosBlock->IsListened(piAosBlockListener3));
}

TEST_F(AosBlockTest, RemoveListener) {
    IAosBlockListener* piAosBlockListener1 = new MockIAosBlockListener();
    IAosBlockListener* piAosBlockListener2 = new MockIAosBlockListener();
    IAosBlockListener* piAosBlockListener3 = new MockIAosBlockListener();

    pAosBlock->SetListener(piAosBlockListener1);
    pAosBlock->SetListener(piAosBlockListener2);
    pAosBlock->SetListener(piAosBlockListener3);

    EXPECT_TRUE(pAosBlock->IsListened(piAosBlockListener1));
    EXPECT_TRUE(pAosBlock->IsListened(piAosBlockListener2));
    EXPECT_TRUE(pAosBlock->IsListened(piAosBlockListener3));

    pAosBlock->RemoveListener(piAosBlockListener1);
    pAosBlock->RemoveListener(piAosBlockListener2);
    pAosBlock->RemoveListener(piAosBlockListener3);

    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener1));
    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener2));
    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener3));
}

TEST_F(AosBlockTest, SetBlockReason) {
    ASSERT_TRUE(pAosBlock->IsCleared());

    pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_CSCALL_STARTED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_DATA_FAILED);
    pAosBlock->SetBlockReason(BLOCK_ENABLER_DETACHED);
    pAosBlock->SetBlockReason(BLOCK_IMS_DISABLED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_REG_FAILED);
    pAosBlock->SetBlockReason(BLOCK_POWER_OFF);
    pAosBlock->SetBlockReason(BLOCK_SERVICE_CONNECTING);
    pAosBlock->SetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_TTY_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOLTE_OFF);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOPS_OFF);
    pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_WIFI_NO_WIFI);
    pAosBlock->SetBlockReason(BLOCK_WIFI_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    pAosBlock->SetBlockReason(BLOCK_WIFI_VOWIFI_OFF);

    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_AC_INCOMPLETED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CSCALL_STARTED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_ENABLER_DETACHED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_IMS_DISABLED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_POWER_OFF));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_SERVICE_CONNECTING));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_TTY_MODE_ON));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_TEMPORARY_DATA_DEACTIVATED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_AIRPLANE_MODE_ON));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_NO_NETWORK));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_OUT_OF_SERVICE));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_ROAMING));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_VOLTE_OFF));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_VOPS_OFF));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_BAD_CONNECTION));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_AIRPLANE_MODE_ON));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_NO_WIFI));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_ROAMING));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_TEMPORARILY_BLOCKED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_VOWIFI_OFF));

    IMSList<IMS_UINT32> objReason;
    pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(REASON_MAX_SIZE_WHOLE, objReason.GetSize());

    pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(REASON_MAX_SIZE_CELLULAR, objReason.GetSize());

    pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(REASON_MAX_SIZE_WIFI, objReason.GetSize());
}

TEST_F(AosBlockTest, ResetBlockReason) {
    ASSERT_TRUE(pAosBlock->IsCleared());

    pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_CSCALL_STARTED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_DATA_FAILED);
    pAosBlock->SetBlockReason(BLOCK_ENABLER_DETACHED);
    pAosBlock->SetBlockReason(BLOCK_IMS_DISABLED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_REG_FAILED);
    pAosBlock->SetBlockReason(BLOCK_POWER_OFF);
    pAosBlock->SetBlockReason(BLOCK_SERVICE_CONNECTING);
    pAosBlock->SetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_TTY_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOLTE_OFF);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOPS_OFF);
    pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_WIFI_NO_WIFI);
    pAosBlock->SetBlockReason(BLOCK_WIFI_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    pAosBlock->SetBlockReason(BLOCK_WIFI_VOWIFI_OFF);

    IMSList<IMS_UINT32> objReason;
    pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(REASON_MAX_SIZE_WHOLE, objReason.GetSize());

    pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(REASON_MAX_SIZE_CELLULAR, objReason.GetSize());

    pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(REASON_MAX_SIZE_WIFI, objReason.GetSize());

    pAosBlock->ResetBlockReason(BLOCK_AC_INCOMPLETED);
    pAosBlock->ResetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    pAosBlock->ResetBlockReason(BLOCK_AOS_INCOMPLETED);
    pAosBlock->ResetBlockReason(BLOCK_CSCALL_STARTED);
    pAosBlock->ResetBlockReason(BLOCK_PERMANENT_DATA_FAILED);
    pAosBlock->ResetBlockReason(BLOCK_ENABLER_DETACHED);
    pAosBlock->ResetBlockReason(BLOCK_IMS_DISABLED);
    pAosBlock->ResetBlockReason(BLOCK_PERMANENT_REG_FAILED);
    pAosBlock->ResetBlockReason(BLOCK_POWER_OFF);
    pAosBlock->ResetBlockReason(BLOCK_SERVICE_CONNECTING);
    pAosBlock->ResetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED);
    pAosBlock->ResetBlockReason(BLOCK_TTY_MODE_ON);
    pAosBlock->ResetBlockReason(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    pAosBlock->ResetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    pAosBlock->ResetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    pAosBlock->ResetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);
    pAosBlock->ResetBlockReason(BLOCK_CELLULAR_ROAMING);
    pAosBlock->ResetBlockReason(BLOCK_CELLULAR_VOLTE_OFF);
    pAosBlock->ResetBlockReason(BLOCK_CELLULAR_VOPS_OFF);
    pAosBlock->ResetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    pAosBlock->ResetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    pAosBlock->ResetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);
    pAosBlock->ResetBlockReason(BLOCK_WIFI_NO_WIFI);
    pAosBlock->ResetBlockReason(BLOCK_WIFI_ROAMING);
    pAosBlock->ResetBlockReason(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    pAosBlock->ResetBlockReason(BLOCK_WIFI_VOWIFI_OFF);

    EXPECT_TRUE(pAosBlock->IsCleared());
}

TEST_F(AosBlockTest, ClearAllBlockReasons) {
    pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_CSCALL_STARTED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_DATA_FAILED);
    pAosBlock->SetBlockReason(BLOCK_ENABLER_DETACHED);
    pAosBlock->SetBlockReason(BLOCK_IMS_DISABLED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_REG_FAILED);
    pAosBlock->SetBlockReason(BLOCK_POWER_OFF);
    pAosBlock->SetBlockReason(BLOCK_SERVICE_CONNECTING);
    pAosBlock->SetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_TTY_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOLTE_OFF);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOPS_OFF);
    pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_WIFI_NO_WIFI);
    pAosBlock->SetBlockReason(BLOCK_WIFI_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    pAosBlock->SetBlockReason(BLOCK_WIFI_VOWIFI_OFF);

    IMSList<IMS_UINT32> objReason;
    pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(REASON_MAX_SIZE_WHOLE, objReason.GetSize());

    pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(REASON_MAX_SIZE_CELLULAR, objReason.GetSize());

    pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(REASON_MAX_SIZE_WIFI, objReason.GetSize());

    pAosBlock->ClearAllBlockReasons();
    EXPECT_TRUE(pAosBlock->IsCleared());
}

TEST_F(AosBlockTest, PrintBlockReasons) {
    // No testing is required.
}

TEST_F(AosBlockTest, GetBlockReasons) {
    pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_CSCALL_STARTED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_DATA_FAILED);
    pAosBlock->SetBlockReason(BLOCK_ENABLER_DETACHED);
    pAosBlock->SetBlockReason(BLOCK_IMS_DISABLED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_REG_FAILED);
    pAosBlock->SetBlockReason(BLOCK_POWER_OFF);
    pAosBlock->SetBlockReason(BLOCK_SERVICE_CONNECTING);
    pAosBlock->SetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_TTY_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOLTE_OFF);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOPS_OFF);
    pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_WIFI_NO_WIFI);
    pAosBlock->SetBlockReason(BLOCK_WIFI_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    pAosBlock->SetBlockReason(BLOCK_WIFI_VOWIFI_OFF);

    IMSList<IMS_UINT32> objReason;
    pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(REASON_MAX_SIZE_WHOLE, objReason.GetSize());

    pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(REASON_MAX_SIZE_CELLULAR, objReason.GetSize());

    pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(REASON_MAX_SIZE_WIFI, objReason.GetSize());
}

TEST_F(AosBlockTest, IsReasonBlocked) {
    ASSERT_TRUE(pAosBlock->IsCleared());

    pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_CSCALL_STARTED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_DATA_FAILED);
    pAosBlock->SetBlockReason(BLOCK_ENABLER_DETACHED);
    pAosBlock->SetBlockReason(BLOCK_IMS_DISABLED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_REG_FAILED);
    pAosBlock->SetBlockReason(BLOCK_POWER_OFF);
    pAosBlock->SetBlockReason(BLOCK_SERVICE_CONNECTING);
    pAosBlock->SetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_TTY_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOLTE_OFF);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOPS_OFF);
    pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_WIFI_NO_WIFI);
    pAosBlock->SetBlockReason(BLOCK_WIFI_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    pAosBlock->SetBlockReason(BLOCK_WIFI_VOWIFI_OFF);

    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_AC_INCOMPLETED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_AOS_INCOMPLETED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CSCALL_STARTED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_PERMANENT_DATA_FAILED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_ENABLER_DETACHED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_IMS_DISABLED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_POWER_OFF));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_SERVICE_CONNECTING));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_TTY_MODE_ON));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_TEMPORARY_DATA_DEACTIVATED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_AIRPLANE_MODE_ON));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_NO_NETWORK));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_OUT_OF_SERVICE));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_ROAMING));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_VOLTE_OFF));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_CELLULAR_VOPS_OFF));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_BAD_CONNECTION));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_AIRPLANE_MODE_ON));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_NO_WIFI));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_ROAMING));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_TEMPORARILY_BLOCKED));
    EXPECT_TRUE(pAosBlock->IsReasonBlocked(BLOCK_WIFI_VOWIFI_OFF));
}

TEST_F(AosBlockTest, IsCleared) {
    ASSERT_TRUE(pAosBlock->IsCleared());

    pAosBlock->SetBlockReason(BLOCK_AC_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    pAosBlock->SetBlockReason(BLOCK_AOS_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_CSCALL_STARTED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_DATA_FAILED);
    pAosBlock->SetBlockReason(BLOCK_ENABLER_DETACHED);
    pAosBlock->SetBlockReason(BLOCK_IMS_DISABLED);
    pAosBlock->SetBlockReason(BLOCK_PERMANENT_REG_FAILED);
    pAosBlock->SetBlockReason(BLOCK_POWER_OFF);
    pAosBlock->SetBlockReason(BLOCK_SERVICE_CONNECTING);
    pAosBlock->SetBlockReason(BLOCK_SUBSCRIBER_INCOMPLETED);
    pAosBlock->SetBlockReason(BLOCK_TTY_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_TEMPORARY_DATA_DEACTIVATED);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_NO_NETWORK);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOLTE_OFF);
    pAosBlock->SetBlockReason(BLOCK_CELLULAR_VOPS_OFF);
    pAosBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    pAosBlock->SetBlockReason(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    pAosBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);
    pAosBlock->SetBlockReason(BLOCK_WIFI_NO_WIFI);
    pAosBlock->SetBlockReason(BLOCK_WIFI_ROAMING);
    pAosBlock->SetBlockReason(BLOCK_WIFI_TEMPORARILY_BLOCKED);
    pAosBlock->SetBlockReason(BLOCK_WIFI_VOWIFI_OFF);

    IMSList<IMS_UINT32> objReason;
    pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(REASON_MAX_SIZE_WHOLE, objReason.GetSize());

    pAosBlock->GetBlockReasons(objReason, SERVICE_CELLULAR);
    EXPECT_EQ(REASON_MAX_SIZE_CELLULAR, objReason.GetSize());

    pAosBlock->GetBlockReasons(objReason, SERVICE_WIFI);
    EXPECT_EQ(REASON_MAX_SIZE_WIFI, objReason.GetSize());

    EXPECT_FALSE(pAosBlock->IsCleared());

    pAosBlock->ClearAllBlockReasons();
    EXPECT_TRUE(pAosBlock->IsCleared());
}

TEST_F(AosBlockTest, IsListened) {
    IAosBlockListener* piAosBlockListener1 = new MockIAosBlockListener();
    IAosBlockListener* piAosBlockListener2 = new MockIAosBlockListener();
    IAosBlockListener* piAosBlockListener3 = new MockIAosBlockListener();

    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener1));
    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener2));
    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener3));

    pAosBlock->SetListener(piAosBlockListener1);
    pAosBlock->SetListener(piAosBlockListener2);
    pAosBlock->SetListener(piAosBlockListener3);

    EXPECT_TRUE(pAosBlock->IsListened(piAosBlockListener1));
    EXPECT_TRUE(pAosBlock->IsListened(piAosBlockListener2));
    EXPECT_TRUE(pAosBlock->IsListened(piAosBlockListener3));

    pAosBlock->RemoveListener(piAosBlockListener1);
    pAosBlock->RemoveListener(piAosBlockListener2);
    pAosBlock->RemoveListener(piAosBlockListener3);

    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener1));
    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener2));
    EXPECT_FALSE(pAosBlock->IsListened(piAosBlockListener3));
}

TEST_F(AosBlockTest, BlockReasonToString) {
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_AC_INCOMPLETED),
            "AC_INCOMPLETED");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_AUTHENTICATION_FAILED),
            "AUTHENTICATION_FAILED");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_AOS_INCOMPLETED),
            "AOS_INCOMPLETED");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_CSCALL_STARTED),
            "CSCALL_STARTED");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_PERMANENT_DATA_FAILED),
            "PERMANENT_DATA_FAILED");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_ENABLER_DETACHED),
            "ENABLER_DETACHED");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_IMS_DISABLED),
            "IMS_DISABLED");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_PERMANENT_REG_FAILED),
            "PERMANENT_REG_FAILED");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_POWER_OFF),
            "POWER_OFF");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_SERVICE_CONNECTING),
            "SERVICE_CONNECTING");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_SUBSCRIBER_INCOMPLETED),
            "SUBSCRIBER_INCOMPLETED");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_TTY_MODE_ON),
            "TTY_MODE_ON");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_TEMPORARY_DATA_DEACTIVATED),
            "TEMPORARY_DATA_DEACTIVATED");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_CELLULAR_AIRPLANE_MODE_ON),
            "CELLULAR_AIRPLANE_MODE_ON");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_CELLULAR_NO_NETWORK),
            "CELLULAR_NO_NETWORK");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_CELLULAR_OUT_OF_SERVICE),
            "CELLULAR_OUT_OF_SERVICE");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_CELLULAR_ROAMING),
            "CELLULAR_ROAMING");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_CELLULAR_VOLTE_OFF),
            "CELLULAR_VOLTE_OFF");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_CELLULAR_VOPS_OFF),
            "CELLULAR_VOPS_OFF");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_WIFI_BAD_CONNECTION),
            "WIFI_BAD_CONNECTION");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE),
            "WIFI_COUNTRY_CODE_UNAVAILABLE");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_WIFI_AIRPLANE_MODE_ON),
            "WIFI_AIRPLANE_MODE_ON");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_WIFI_NO_WIFI),
            "WIFI_NO_WIFI");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_WIFI_ROAMING),
            "WIFI_ROAMING");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_WIFI_TEMPORARILY_BLOCKED),
            "WIFI_TEMPORARILY_BLOCKED");
    EXPECT_EQ(pAosBlock->BlockReasonToString(BLOCK_WIFI_VOWIFI_OFF),
            "WIFI_VOWIFI_OFF");
}