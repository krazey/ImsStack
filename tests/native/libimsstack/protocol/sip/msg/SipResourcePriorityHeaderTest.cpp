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

#include "msg/SipResourcePriorityHeader.h"

namespace android
{

class SipResourcePriorityHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipResourcePriorityHeaderTest, CopyConstructor)
{
    SipResourcePriorityHeader* pHeader = reinterpret_cast<SipResourcePriorityHeader*>(
            SipResourcePriorityHeader::GetNewObj(SipHeaderBase::RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    SipResourcePriorityHeader* pCopyHeader = reinterpret_cast<SipResourcePriorityHeader*>(
            SipResourcePriorityHeader::GetNewObj(SipHeaderBase::RESOURCE_PRIORITY, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();
    pCopyHeader->SipDelete();
}

TEST_F(SipResourcePriorityHeaderTest, Encode)
{
    SipResourcePriorityHeader* pHeader = reinterpret_cast<SipResourcePriorityHeader*>(
            SipResourcePriorityHeader::GetNewObj(SipHeaderBase::RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    const SIP_INT32 BUFFER_SIZE = 256;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SetNameSpace("Namespace");

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));

    pHeader->SetRPriority("ResourcePriority");

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));

    EXPECT_STREQ("Namespace.ResourcePriority", objBuffer.GetCharString());
    EXPECT_STREQ("Namespace.ResourcePriority", &(aBuffer[0]));

    pHeader->SipDelete();

    // ACCEPT_RESOURCE_PRIORITY header tests
    SipResourcePriorityHeader* pAcceptResourcePriorityHeader =
            reinterpret_cast<SipResourcePriorityHeader*>(SipResourcePriorityHeader::GetNewObj(
                    SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pAcceptResourcePriorityHeader != nullptr);

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    objBuffer = AString::ConstNull();

    /* Empty header allowed */
    EXPECT_EQ(SIP_TRUE, pAcceptResourcePriorityHeader->IsValidHeader());
    EXPECT_EQ(SIP_TRUE, pAcceptResourcePriorityHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pAcceptResourcePriorityHeader->Encode(objBuffer, SIP_FALSE));

    /* Only namespace present, fail */
    pAcceptResourcePriorityHeader->SetNameSpace("namespace");
    EXPECT_EQ(SIP_FALSE, pAcceptResourcePriorityHeader->IsValidHeader());
    EXPECT_EQ(SIP_FALSE, pAcceptResourcePriorityHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pAcceptResourcePriorityHeader->Encode(objBuffer, SIP_FALSE));
    pAcceptResourcePriorityHeader->SipDelete();
    pAcceptResourcePriorityHeader = nullptr;

    pAcceptResourcePriorityHeader = reinterpret_cast<SipResourcePriorityHeader*>(
            SipResourcePriorityHeader::GetNewObj(SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pAcceptResourcePriorityHeader != nullptr);
    /* Only r-priority present, fail */
    pAcceptResourcePriorityHeader->SetRPriority("r-priority");
    EXPECT_EQ(SIP_FALSE, pAcceptResourcePriorityHeader->IsValidHeader());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    objBuffer = AString::ConstNull();
    EXPECT_EQ(SIP_FALSE, pAcceptResourcePriorityHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pAcceptResourcePriorityHeader->Encode(objBuffer, SIP_FALSE));
    pAcceptResourcePriorityHeader->SipDelete();
    pAcceptResourcePriorityHeader = nullptr;

    pAcceptResourcePriorityHeader = reinterpret_cast<SipResourcePriorityHeader*>(
            SipResourcePriorityHeader::GetNewObj(SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pAcceptResourcePriorityHeader != nullptr);
    /* namespace and r-priority present, success */
    pAcceptResourcePriorityHeader->SetNameSpace("namespace");
    pAcceptResourcePriorityHeader->SetRPriority("r-priority");
    EXPECT_EQ(SIP_TRUE, pAcceptResourcePriorityHeader->IsValidHeader());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    objBuffer = AString::ConstNull();
    EXPECT_EQ(SIP_TRUE, pAcceptResourcePriorityHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pAcceptResourcePriorityHeader->Encode(objBuffer, SIP_FALSE));

    EXPECT_STREQ("namespace.r-priority", &(aBuffer[0]));
    EXPECT_STREQ("namespace.r-priority", objBuffer.GetCharString());
    pAcceptResourcePriorityHeader->SipDelete();
}

TEST_F(SipResourcePriorityHeaderTest, Decode)
{
    SipResourcePriorityHeader* pHeader = reinterpret_cast<SipResourcePriorityHeader*>(
            SipResourcePriorityHeader::GetNewObj(SipHeaderBase::RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("OnlyNameSpace", 13));

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("Namespace.ResourcePriority", 26));

    EXPECT_STREQ("Namespace", pHeader->GetNameSpace());
    EXPECT_STREQ("ResourcePriority", pHeader->GetResourcePriority());

    pHeader->SipDelete();

    // ACCEPT_RESOURCE_PRIORITY header tests
    SipResourcePriorityHeader* pAcceptResourcePriorityHeader =
            reinterpret_cast<SipResourcePriorityHeader*>(SipResourcePriorityHeader::GetNewObj(
                    SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pAcceptResourcePriorityHeader != nullptr);

    /* Empty header allowed */
    EXPECT_EQ(SIP_TRUE, pAcceptResourcePriorityHeader->Decode(nullptr, 0));

    /* Only namespace present, fail */
    EXPECT_EQ(SIP_FALSE, pAcceptResourcePriorityHeader->Decode("namespace", 9));

    /* Only namespace(DOT) without r-priority, fail */
    EXPECT_EQ(SIP_FALSE, pAcceptResourcePriorityHeader->Decode("namespace.", 10));
    pAcceptResourcePriorityHeader->SipDelete();
    pAcceptResourcePriorityHeader = nullptr;

    pAcceptResourcePriorityHeader = reinterpret_cast<SipResourcePriorityHeader*>(
            SipResourcePriorityHeader::GetNewObj(SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pAcceptResourcePriorityHeader != nullptr);
    /* Only (DOT)r-priority present, fail */
    EXPECT_EQ(SIP_FALSE, pAcceptResourcePriorityHeader->Decode(".r-priority", 11));
    pAcceptResourcePriorityHeader->SipDelete();
    pAcceptResourcePriorityHeader = nullptr;

    pAcceptResourcePriorityHeader = reinterpret_cast<SipResourcePriorityHeader*>(
            SipResourcePriorityHeader::GetNewObj(SipHeaderBase::ACCEPT_RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pAcceptResourcePriorityHeader != nullptr);
    /* Both namespace and r-priority present, success */
    EXPECT_EQ(SIP_TRUE, pAcceptResourcePriorityHeader->Decode("namespace.r-priority", 20));
    EXPECT_STREQ("namespace", pAcceptResourcePriorityHeader->GetNameSpace());
    EXPECT_STREQ("r-priority", pAcceptResourcePriorityHeader->GetResourcePriority());
    pAcceptResourcePriorityHeader->SipDelete();
}

}  // namespace android
