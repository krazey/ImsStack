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

#include "interface/MockIAosAppContext.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "condition/AosBlock.h"
#include "condition/AosECondition.h"
#include "provider/AosStaticProfile.h"

class IAosConditionListener;

using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

class AosEConditionTest : public ::testing::Test {
public:
    AosECondition* pAosECondition;
    AosBlock* pAosBlock;

protected:
    virtual void SetUp() override {
        MockIAosAppContext objMockIAosAppContext;

        EXPECT_CALL(objMockIAosAppContext, GetSlotId())
            .Times(AnyNumber())
            .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(objMockIAosAppContext, GetProfileId())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(objMockIAosAppContext, GetConnection())
            .Times(AnyNumber())
            .WillRepeatedly(ReturnNull());

        pAosECondition = new AosECondition(static_cast<IAosAppContext*>(&objMockIAosAppContext));
        ASSERT_TRUE(pAosECondition != nullptr);

        pAosBlock = new AosBlock(static_cast<IAosAppContext*>(&objMockIAosAppContext));
        ASSERT_TRUE(pAosBlock != nullptr);

        IAosBlock* piAosBlock = static_cast<IAosBlock*>(pAosBlock);
        SetAosBlock(piAosBlock);
    }

    virtual void TearDown() override {
        if (pAosBlock) {
            delete pAosBlock;
        }
        if (pAosECondition) {
            delete pAosECondition;
        }
    }

    void SetAosBlock(IN IAosBlock* piBlock) {
        pAosECondition->m_piBlock = piBlock;
    }
};

// Testing is not required.
// TEST_F(AosEConditionTest, Constructor) {
// }

// Testing is not required.
// TEST_F(AosEConditionTest, Destructor) {
// }

TEST_F(AosEConditionTest, IsReady) {
    EXPECT_TRUE(pAosECondition->IsReady());

    pAosECondition->SetBlock(BLOCK_AC_INCOMPLETED);
    EXPECT_FALSE(pAosECondition->IsReady());
    pAosECondition->SetBlock(BLOCK_AUTHENTICATION_FAILED);
    EXPECT_FALSE(pAosECondition->IsReady());
    pAosECondition->SetBlock(BLOCK_AOS_INCOMPLETED);
    EXPECT_FALSE(pAosECondition->IsReady());

    pAosECondition->SetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    EXPECT_FALSE(pAosECondition->IsReady());
    pAosECondition->SetBlock(BLOCK_CELLULAR_NO_NETWORK);
    EXPECT_FALSE(pAosECondition->IsReady());
    pAosECondition->SetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    EXPECT_FALSE(pAosECondition->IsReady());

    pAosECondition->SetBlock(BLOCK_WIFI_BAD_CONNECTION);
    EXPECT_FALSE(pAosECondition->IsReady());
    pAosECondition->SetBlock(BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE);
    EXPECT_FALSE(pAosECondition->IsReady());
    pAosECondition->SetBlock(BLOCK_WIFI_AIRPLANE_MODE_ON);
    EXPECT_FALSE(pAosECondition->IsReady());

    pAosECondition->ResetBlock(BLOCK_AC_INCOMPLETED);
    EXPECT_FALSE(pAosECondition->IsReady());
    pAosECondition->ResetBlock(BLOCK_AUTHENTICATION_FAILED);
    EXPECT_FALSE(pAosECondition->IsReady());
    pAosECondition->ResetBlock(BLOCK_AOS_INCOMPLETED);
    EXPECT_FALSE(pAosECondition->IsReady());

    pAosECondition->ResetBlock(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    EXPECT_FALSE(pAosECondition->IsReady());
    pAosECondition->ResetBlock(BLOCK_CELLULAR_NO_NETWORK);
    EXPECT_FALSE(pAosECondition->IsReady());
    pAosECondition->ResetBlock(BLOCK_CELLULAR_OUT_OF_SERVICE);
    EXPECT_TRUE(pAosECondition->IsReady());

    IMSList<IMS_UINT32> objReason;
    pAosBlock->GetBlockReasons(objReason, SERVICE_WHOLE);
    EXPECT_EQ(objReason.GetSize(), 3);

    EXPECT_TRUE(pAosECondition->IsReady());
}