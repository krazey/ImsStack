/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "ImsStrLib.h"
#include "SipStack.h"

namespace android
{

class SipStackTest : public ::testing::Test
{
protected:
    virtual void SetUp() override { SipMsgUtil::Init(); }

    virtual void TearDown() override {}
};

TEST_F(SipStackTest, DisplayBadHeaders)
{
    const SIP_CHAR acMsg[] = {"INVITE sip:user@example.com SIP/2.0\r\n"
                              "Max-Forwards: 254\r\n"
                              "To: sip:j.user@example.com\r\n"
                              "From: sip:caller@example.net;tag=32394234\r\n"
                              "Call-ID: ncl.0ha0isndaksdj2193423r542w35\r\n"
                              "CSeq: 0 INVITE\r\n"
                              "Via: SIP/2.0/UDP 192.0.2.53;branch=z9hG4bKkdjuw\r\n"
                              "Contact: sip:caller@example53.example.net\r\n"
                              "Date: Fri, 01 Jan 2010 16:00:00 EST\r\n"
                              "Content-Length: 0\r\n"
                              "\r\n"};

    ::SipMessage* pSipMsg = new ::SipMessage();
    SIP_BOOL bResult = pSipMsg->Decode(acMsg, IMS_StrLen(acMsg));

    EXPECT_EQ(SIP_TRUE, bResult);
    EXPECT_EQ(1, pSipMsg->GetBadHeaderCount());  // Date is a bad header.

    SipStack::DisplayBadHeaders(pSipMsg);

    // Checks whether the bad header list is kept or not.
    EXPECT_EQ(1, pSipMsg->GetBadHeaderCount());

    pSipMsg->SipDelete();
}

TEST_F(SipStackTest, SetTimerValues)
{
    SipTimerValues objTv;
    IMS_UINT32 nT1 = 3000;
    IMS_UINT32 nT2 = 16000;
    IMS_UINT32 nT4 = nT2 + 1000;
    IMS_UINT32 nTB = 32000;
    IMS_UINT32 nTD = 64000;
    IMS_UINT32 nTF = 8000;
    IMS_UINT32 nTH = 192000;
    IMS_UINT32 nTI = 64000;
    IMS_UINT32 nTJ = 64000;
    IMS_UINT32 nTK = 32000;

    objTv.SetValue(SipTimerValues::TIMER_T1, nT1);
    objTv.SetValue(SipTimerValues::TIMER_T2, nT2);
    objTv.SetValue(SipTimerValues::TIMER_B, nTB);
    objTv.SetValue(SipTimerValues::TIMER_D, nTD);
    objTv.SetValue(SipTimerValues::TIMER_F, nTF);
    objTv.SetValue(SipTimerValues::TIMER_H, nTH);
    objTv.SetValue(SipTimerValues::TIMER_I, nTI);
    objTv.SetValue(SipTimerValues::TIMER_J, nTJ);
    objTv.SetValue(SipTimerValues::TIMER_K, nTK);

    SipTxnContext objTxnContext;
    SipTxnTimerValues* pTxnTimerValues = objTxnContext.m_pSipTimerContext->m_pTxnSipTxnTimers;
    SipTxnContext* pTxnContext = &objTxnContext;
    SipStack::SetTimerValues(&objTv, pTxnContext);

    EXPECT_EQ(nT1, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_T1));
    EXPECT_EQ(nT1, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_A));
    EXPECT_EQ(nT1, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_E));
    EXPECT_EQ(nT1, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_G));
    EXPECT_EQ(nT1 * 64, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_M));

    EXPECT_EQ(nT2, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_T2));
    EXPECT_EQ(nT2 + 1000, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_T4));

    EXPECT_EQ(nTB, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_B));
    EXPECT_EQ(nTD, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_D));
    EXPECT_EQ(nTF, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_F));
    EXPECT_EQ(nTH, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_H));
    EXPECT_EQ(nTI, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_I));
    EXPECT_EQ(nTJ, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_J));
    EXPECT_EQ(nTK, pTxnTimerValues->GetTimerValue(SipTxn::TIMER_K));
}

}  // namespace android
