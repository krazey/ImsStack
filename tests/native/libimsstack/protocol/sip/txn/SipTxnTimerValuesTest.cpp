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

#include "txn/SipTxn.h"
#include "txn/sip_txn_common.h"
#include "txn/SipTxnTimerValues.h"

namespace android
{

class SipTxnTimerValuesTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipTxnTimerValuesTest, SetGetAndUpdate)
{
    SipTxnTimerValues* pTxnTimers = new SipTxnTimerValues();
    ASSERT_TRUE(pTxnTimers != nullptr);

    pTxnTimers->SetTimerValue(SipTxn::TIMER1, 1);
    pTxnTimers->SetTimerValue(SipTxn::TIMER2, 2);
    pTxnTimers->SetTimerValue(SipTxn::TIMER4, 3);
    pTxnTimers->SetTimerValue(SipTxn::TIMERA, 4);
    pTxnTimers->SetTimerValue(SipTxn::TIMERB, 5);
    pTxnTimers->SetTimerValue(SipTxn::TIMERD, 6);
    pTxnTimers->SetTimerValue(SipTxn::TIMERE, 7);
    pTxnTimers->SetTimerValue(SipTxn::TIMERF, 8);
    pTxnTimers->SetTimerValue(SipTxn::TIMERG, 9);
    pTxnTimers->SetTimerValue(SipTxn::TIMERH, 10);
    pTxnTimers->SetTimerValue(SipTxn::TIMERI, 11);
    pTxnTimers->SetTimerValue(SipTxn::TIMERJ, 12);
    pTxnTimers->SetTimerValue(SipTxn::TIMERK, 13);

    pTxnTimers->SetTimerValue(SipTxn::TIMER_TYPE_INVALID, 14);

    unsigned int nTxnTimerOptions = 0;

    SipTxnTimerValues* pCopyTxnTimers = new SipTxnTimerValues();
    ASSERT_TRUE(pCopyTxnTimers != nullptr);

    /* null sip txn timers, fail */
    EXPECT_EQ(SIP_FALSE, pCopyTxnTimers->UpdateSipTimers(nTxnTimerOptions, nullptr));

    /* no timers set in timer options, fail */
    EXPECT_EQ(SIP_FALSE, pCopyTxnTimers->UpdateSipTimers(nTxnTimerOptions, pTxnTimers));

    nTxnTimerOptions = 0x1FFF;  // SipTxnTimerValues::TV_ALL

    /* txn timer passed along with timer options, success */
    EXPECT_EQ(SIP_TRUE, pCopyTxnTimers->UpdateSipTimers(nTxnTimerOptions, pTxnTimers));

    delete pTxnTimers;

    EXPECT_EQ(1, pCopyTxnTimers->GetTimerValue(SipTxn::TIMER1));
    EXPECT_EQ(2, pCopyTxnTimers->GetTimerValue(SipTxn::TIMER2));
    EXPECT_EQ(3, pCopyTxnTimers->GetTimerValue(SipTxn::TIMER4));
    EXPECT_EQ(4, pCopyTxnTimers->GetTimerValue(SipTxn::TIMERA));
    EXPECT_EQ(5, pCopyTxnTimers->GetTimerValue(SipTxn::TIMERB));
    EXPECT_EQ(6, pCopyTxnTimers->GetTimerValue(SipTxn::TIMERD));
    EXPECT_EQ(7, pCopyTxnTimers->GetTimerValue(SipTxn::TIMERE));
    EXPECT_EQ(8, pCopyTxnTimers->GetTimerValue(SipTxn::TIMERF));
    EXPECT_EQ(9, pCopyTxnTimers->GetTimerValue(SipTxn::TIMERG));
    EXPECT_EQ(10, pCopyTxnTimers->GetTimerValue(SipTxn::TIMERH));
    EXPECT_EQ(11, pCopyTxnTimers->GetTimerValue(SipTxn::TIMERI));
    EXPECT_EQ(12, pCopyTxnTimers->GetTimerValue(SipTxn::TIMERJ));
    EXPECT_EQ(13, pCopyTxnTimers->GetTimerValue(SipTxn::TIMERK));

    EXPECT_EQ(0, pCopyTxnTimers->GetTimerValue(SipTxn::TIMER_TYPE_INVALID));

    delete pCopyTxnTimers;
}

}  // namespace android