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
#include "msg/SipHeaderList.h"
#include "SipConfiguration.h"

namespace android
{

class SipHeaderListTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipHeaderListTest, GetListObj)
{
    SipHeaderList* pHeaderList = reinterpret_cast<SipHeaderList*>(
            SipHeaderList::GetNewListObj(SipHeaderBase::ALLOW, nullptr));
    ASSERT_TRUE(pHeaderList != nullptr);

    SipHeaderBase* pHeader1 = pHeaderList->GetListObj();
    ASSERT_TRUE(pHeader1 != nullptr);

    SipHeaderBase* pHeader2 = pHeaderList->GetListObj(pHeader1);
    ASSERT_TRUE(pHeader2 != nullptr);

    pHeader1->SipDelete();
    pHeader2->SipDelete();
    pHeaderList->SipDelete();
}

TEST_F(SipHeaderListTest, AddInsertRemoveHeaders)
{
    SipHeaderList* pHeaderList = reinterpret_cast<SipHeaderList*>(
            SipHeaderList::GetNewListObj(SipHeaderBase::ALLOW, nullptr));
    ASSERT_TRUE(pHeaderList != nullptr);

    SipHeaderBase* pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ALLOW, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->SetValue("INVITE"));

    /* Add header to header list, success */
    EXPECT_EQ(SIP_TRUE, pHeaderList->AddHeader(pHeader));

    pHeader->SipDelete();

    EXPECT_EQ(1, pHeaderList->GetSize());

    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ALLOW, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->SetValue("CANCEL"));

    /* Insert header to header list at invalid index, fail */
    EXPECT_EQ(SIP_FALSE, pHeaderList->InsertHdrAtPos(pHeader, 3));

    /* Insert header to header list at valid index, success */
    EXPECT_EQ(SIP_TRUE, pHeaderList->InsertHdrAtPos(pHeader, 0));

    pHeader->SipDelete();

    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ALLOW, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->SetValue("REFER"));

    /* Add header to header list, success */
    EXPECT_EQ(SIP_TRUE, pHeaderList->AddHeader(pHeader));

    pHeader->SipDelete();

    EXPECT_EQ(3, pHeaderList->GetSize());

    /* Remove one element */
    pHeaderList->RemoveHdr(1);

    EXPECT_EQ(2, pHeaderList->GetSize());

    pHeader = pHeaderList->GetObj(0);
    EXPECT_STREQ("CANCEL", pHeader->GetValue());
    pHeader->SipDelete();

    pHeader = pHeaderList->GetObj(1);
    EXPECT_STREQ("REFER", pHeader->GetValue());
    pHeader->SipDelete();

    EXPECT_TRUE(nullptr == pHeaderList->GetObj(2));

    pHeaderList->SipDelete();
}

TEST_F(SipHeaderListTest, DecodeAndEncodeHdr)
{
    SipHeaderList* pHeaderList = reinterpret_cast<SipHeaderList*>(
            SipHeaderList::GetNewListObj(SipHeaderBase::ALLOW, nullptr));
    ASSERT_TRUE(pHeaderList != nullptr);

    /* Allow empty header allowed, success */
    EXPECT_EQ(SIP_TRUE, pHeaderList->DecodeHdr((char*)"", 0));
    EXPECT_EQ(SIP_FALSE, pHeaderList->EncodeHdr(nullptr));
    AStringBuffer objBuffer(256);
    EXPECT_EQ(SIP_TRUE, pHeaderList->Encode(objBuffer, SIP_TRUE));

    EXPECT_EQ(SIP_TRUE, pHeaderList->DecodeHdr((char*)"INVITE,ACK,UPDATE,REFER", 26));

    EXPECT_EQ(4, pHeaderList->GetSize());

    SipHeaderList* pCopyHeaderList = reinterpret_cast<SipHeaderList*>(
            SipHeaderList::GetNewListObj(SipHeaderBase::ALLOW, pHeaderList));
    ASSERT_TRUE(pCopyHeaderList != nullptr);

    pHeaderList->SipDelete();

    const int BUFFER_SIZE = 256;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    /* Normal Encode - single line, success */
    EXPECT_EQ(SIP_TRUE, pCopyHeaderList->EncodeHdr(&pBuff));

    EXPECT_STREQ("INVITE,ACK,UPDATE,REFER", &(aBuffer[0]));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    /* multi line encode, success */
    EXPECT_EQ(SIP_TRUE,
            pCopyHeaderList->EncodeHdr(
                    &pBuff, SIP_FALSE, SipConfiguration::MSG_OPT_ENCODE_MULTI_LINE));

    char* pData = (char*)"INVITE\r\nAllow: ACK\r\nAllow: UPDATE\r\nAllow: REFER";
    EXPECT_STREQ(pData, &(aBuffer[0]));

    pCopyHeaderList->SipDelete();

    pHeaderList = reinterpret_cast<SipHeaderList*>(
            SipHeaderList::GetNewListObj(SipHeaderBase::AUTHENTICATION_INFO, nullptr));
    ASSERT_TRUE(pHeaderList != nullptr);

    /* Authentication Info header should be considered as one complete header, success */
    EXPECT_EQ(SIP_TRUE,
            pHeaderList->DecodeHdr((char*)"nextnonce=\"abcdefgh\",nonce-count=\"3\"", 37));
    EXPECT_EQ(SIP_TRUE, pHeaderList->DecodeHdr((char*)"nonce-count=\"2\"", 15));

    EXPECT_EQ(2, pHeaderList->GetSize());

    pCopyHeaderList = reinterpret_cast<SipHeaderList*>(
            SipHeaderList::GetNewListObj(SipHeaderBase::AUTHENTICATION_INFO, pHeaderList));
    ASSERT_TRUE(pCopyHeaderList != nullptr);

    pHeaderList->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyHeaderList->EncodeHdr(&pBuff));

    pData = (char*)"nextnonce=\"abcdefgh\",nonce-count=\"3\"\r\n\
Authentication-Info: nonce-count=\"2\"";

    EXPECT_STREQ(pData, &(aBuffer[0]));

    pCopyHeaderList->SipDelete();
}

}  // namespace android