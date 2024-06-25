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

TEST_F(SipResourcePriorityHeaderTest, EncodeAndEncodeHdr)
{
    SipResourcePriorityHeader* pHeader = reinterpret_cast<SipResourcePriorityHeader*>(
            SipResourcePriorityHeader::GetNewObj(SipHeaderBase::RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    const int BUFFER_SIZE = 256;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objValue(256);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));

    EXPECT_EQ(SIP_TRUE, pHeader->SetNameSpace("Namespace"));

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));

    EXPECT_EQ(SIP_TRUE, pHeader->SetRPriority("ResourcePriority"));

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_TRUE));
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));

    EXPECT_STREQ("Namespace.ResourcePriority", objValue.GetCharString());
    EXPECT_STREQ("Namespace.ResourcePriority", &(aBuffer[0]));

    pHeader->SipDelete();
}

TEST_F(SipResourcePriorityHeaderTest, DecodeHdr)
{
    SipResourcePriorityHeader* pHeader = reinterpret_cast<SipResourcePriorityHeader*>(
            SipResourcePriorityHeader::GetNewObj(SipHeaderBase::RESOURCE_PRIORITY, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("", 0));

    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("OnlyNameSpace", 13));

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("Namespace.ResourcePriority", 26));

    EXPECT_STREQ("Namespace", pHeader->GetNameSpace());
    EXPECT_STREQ("ResourcePriority", pHeader->GetResourcePriority());

    pHeader->SipDelete();
}

}  // namespace android
