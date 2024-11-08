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

#include "msg/SipAuthInfoHeader.h"

namespace android
{

class SipAuthInfoHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipAuthInfoHeaderTest, DecodeAndEncodeHdr)
{
    SipAuthInfoHeader* pHeader = reinterpret_cast<SipAuthInfoHeader*>(
            SipAuthInfoHeader::GetNewObj(SipHeaderBase::AUTHENTICATION_INFO, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty buffer, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("", 0));

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(64);

    /* Empty object, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));

    /* Single info, success */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("nextnonce=\"123456789abcde123456789abcde\"", 40));

    const SipNameValue* pNmVl = pHeader->GetAiInfoVal(0);
    ASSERT_TRUE(pNmVl != nullptr);
    EXPECT_STREQ("nextnonce", pNmVl->m_pszName);
    EXPECT_EQ(1, pNmVl->m_objValueList.GetSize());
    EXPECT_STREQ("\"123456789abcde123456789abcde\"", pNmVl->m_objValueList.GetAt(0));

    pNmVl = pHeader->GetAiInfoVal(1);
    ASSERT_TRUE(pNmVl == nullptr);

    SipAuthInfoHeader* pCopyHeader = reinterpret_cast<SipAuthInfoHeader*>(
            SipAuthInfoHeader::GetNewObj(SipHeaderBase::AUTHENTICATION_INFO, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_TRUE, pCopyHeader->EncodeHdr(&pBuff));

    pCopyHeader->SipDelete();

    EXPECT_STREQ("nextnonce=\"123456789abcde123456789abcde\"", &(aBuffer[0]));
    EXPECT_STREQ("nextnonce=\"123456789abcde123456789abcde\"", objBuffer.GetCharString());

    pHeader = reinterpret_cast<SipAuthInfoHeader*>(
            SipAuthInfoHeader::GetNewObj(SipHeaderBase::AUTHENTICATION_INFO, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* multiple info, success */
    EXPECT_EQ(SIP_TRUE,
            pHeader->DecodeHdr("nextnonce=\"123456789abcde123456789abcde\",nonce-count=\"3\"", 56));

    pCopyHeader = reinterpret_cast<SipAuthInfoHeader*>(
            SipAuthInfoHeader::GetNewObj(SipHeaderBase::AUTHENTICATION_INFO, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    objBuffer = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_EQ(SIP_TRUE, pCopyHeader->EncodeHdr(&pBuff));

    pNmVl = pCopyHeader->GetAiInfoVal(0);
    ASSERT_TRUE(pNmVl != nullptr);
    EXPECT_STREQ("nextnonce", pNmVl->m_pszName);
    EXPECT_EQ(1, pNmVl->m_objValueList.GetSize());
    EXPECT_STREQ("\"123456789abcde123456789abcde\"", pNmVl->m_objValueList.GetAt(0));

    pNmVl = pCopyHeader->GetAiInfoVal(1);
    ASSERT_TRUE(pNmVl != nullptr);
    EXPECT_STREQ("nonce-count", pNmVl->m_pszName);
    EXPECT_EQ(1, pNmVl->m_objValueList.GetSize());
    EXPECT_STREQ("\"3\"", pNmVl->m_objValueList.GetAt(0));

    pNmVl = pCopyHeader->GetAiInfoVal(2);
    ASSERT_TRUE(pNmVl == nullptr);

    pCopyHeader->SipDelete();

    EXPECT_STREQ("nextnonce=\"123456789abcde123456789abcde\",nonce-count=\"3\"", &(aBuffer[0]));
    EXPECT_STREQ("nextnonce=\"123456789abcde123456789abcde\",nonce-count=\"3\"",
            objBuffer.GetCharString());
}

}  // namespace android
