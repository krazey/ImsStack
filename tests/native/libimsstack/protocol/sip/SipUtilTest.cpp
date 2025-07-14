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

#include "SipDefNetworkUtil.h"
#include "SipUtil.h"

class ISipTimerUtil;

class SipTransactionListener : public ISipTxnListener
{
public:
    SipTransactionListener() {}
    virtual ~SipTransactionListener() {}

    virtual SIP_BOOL TxnTimeout(ISipUserData* pUserData, SIP_INT32 eTimerType) override
    {
        (void)pUserData;
        (void)eTimerType;
        return SIP_TRUE;
    }

    virtual SIP_BOOL TxnTerminated(ISipUserData* pUserData) override
    {
        (void)pUserData;
        return SIP_TRUE;
    }
};

namespace android
{

class SipUtilTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipUtilTest, UtilityTest)
{
    SipUtil* pUtil = SipUtil::GetInstance();
    ASSERT_TRUE(pUtil != nullptr);

    /* Calling again to verify new object is not created */
    ASSERT_TRUE(pUtil == SipUtil::GetInstance());

    EXPECT_TRUE(pUtil->GetTimer() != nullptr);
    /* Calling Start & Stop Timer will all null values */
    ISipTimerUtil* pSipTimerUtil = pUtil->GetTimer();
    EXPECT_FALSE(pSipTimerUtil->StartTimer(SIP_NULL, 0, 0, SIP_NULL, SIP_NULL));
    ASSERT_TRUE(pSipTimerUtil->StopTimer(SIP_NULL) == nullptr);

    EXPECT_TRUE(pUtil->GetLogger() != nullptr);
    EXPECT_TRUE(pUtil->GetNetwork() != nullptr);
    EXPECT_TRUE(pUtil->GetTransactionListener() == nullptr);

    ISipNetworkUtil* pNetworkUtil = new SipDefNetworkUtil();
    pUtil->SetNetwork(pNetworkUtil);
    EXPECT_TRUE(pNetworkUtil == pUtil->GetNetwork());

    ISipTxnListener* pTxnListener = new SipTransactionListener();
    pUtil->SetTransactionListener(pTxnListener);
    EXPECT_TRUE(pTxnListener == pUtil->GetTransactionListener());

    /* Calling SetTransactionListener again to verify first time set txn listener
    is deleted and then set second time set txn listener */
    pTxnListener = new SipTransactionListener();
    pUtil->SetTransactionListener(pTxnListener);
    EXPECT_TRUE(pTxnListener == pUtil->GetTransactionListener());

    SipUtil::DestroyInstance();
}

}  // namespace android
