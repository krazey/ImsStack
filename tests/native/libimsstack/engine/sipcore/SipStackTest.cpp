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
    virtual void SetUp() override { SIPHdrAccess::Init(); }

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

}  // namespace android
