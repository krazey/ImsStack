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

#include "msg/SipAcceptResourcePriorityHeader.h"

namespace android
{

class SipAcceptResourcePriorityHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipAcceptResourcePriorityHeaderTest, CopyConstructor)
{
    SipAcceptResourcePriorityHeader* pHeader = reinterpret_cast<SipAcceptResourcePriorityHeader*>(
            SipAcceptResourcePriorityHeader::GetNewObj(
                    SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    pHeader->SetNameSpace("namespace");
    pHeader->SetRPriority("r-priority");

    SipAcceptResourcePriorityHeader* pCopyHeader =
            reinterpret_cast<SipAcceptResourcePriorityHeader*>(
                    SipAcceptResourcePriorityHeader::GetNewObj(
                            SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_STREQ("namespace", pCopyHeader->GetNameSpace());
    EXPECT_STREQ("r-priority", pCopyHeader->GetResourcePriority());

    pCopyHeader->SipDelete();
}

TEST_F(SipAcceptResourcePriorityHeaderTest, EncodeAndEncodeHdr)
{
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    SipAcceptResourcePriorityHeader* pHeader = reinterpret_cast<SipAcceptResourcePriorityHeader*>(
            SipAcceptResourcePriorityHeader::GetNewObj(
                    SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header allowed */
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));

    /* Only namespace present, fail */
    pHeader->SetNameSpace("namespace");
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptResourcePriorityHeader*>(
            SipAcceptResourcePriorityHeader::GetNewObj(
                    SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    /* Only r-priority present, fail */
    pHeader->SetRPriority("r-priority");
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptResourcePriorityHeader*>(
            SipAcceptResourcePriorityHeader::GetNewObj(
                    SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    /* namespace and r-priority present, success */
    pHeader->SetNameSpace("namespace");
    pHeader->SetRPriority("r-priority");
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));

    EXPECT_STREQ("namespace.r-priority", &(aBuffer[0]));
    EXPECT_STREQ("namespace.r-priority", objBuffer.GetCharString());
    pHeader->SipDelete();
}

TEST_F(SipAcceptResourcePriorityHeaderTest, DecodeHdr)
{
    SipAcceptResourcePriorityHeader* pHeader = reinterpret_cast<SipAcceptResourcePriorityHeader*>(
            SipAcceptResourcePriorityHeader::GetNewObj(
                    SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header allowed */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(nullptr, 0));

    /* Only namespace present, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("namespace", 9));

    /* Only namespace(DOT) without r-priority, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("namespace.", 10));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptResourcePriorityHeader*>(
            SipAcceptResourcePriorityHeader::GetNewObj(
                    SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    /* Only (DOT)r-priority present, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(".r-priority", 11));
    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipAcceptResourcePriorityHeader*>(
            SipAcceptResourcePriorityHeader::GetNewObj(
                    SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    /* Both namespace and r-priority present, success */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("namespace.r-priority", 20));
    EXPECT_STREQ("namespace", pHeader->GetNameSpace());
    EXPECT_STREQ("r-priority", pHeader->GetResourcePriority());
    pHeader->SipDelete();
}

}  // namespace android
