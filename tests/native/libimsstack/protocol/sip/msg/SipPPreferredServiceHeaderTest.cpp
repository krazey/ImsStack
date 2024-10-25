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

#include "msg/SipPPreferredServiceHeader.h"
#include "platform/SipString.h"

namespace android
{

class SipPPreferredServiceHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipPPreferredServiceHeaderTest, CopyConstructor)
{
    SipPPreferredServiceHeader* pHeader = reinterpret_cast<SipPPreferredServiceHeader*>(
            SipPPreferredServiceHeader::GetNewObj(SipHeaderBase::P_PREFERRED_SERVICE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    SipPPreferredServiceHeader* pCopyHeader = reinterpret_cast<SipPPreferredServiceHeader*>(
            SipPPreferredServiceHeader::GetNewObj(SipHeaderBase::P_PREFERRED_SERVICE, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pCopyHeader->SipDelete();
}

TEST_F(SipPPreferredServiceHeaderTest, DecodeHdr)
{
    SipPPreferredServiceHeader* pHeader = reinterpret_cast<SipPPreferredServiceHeader*>(
            SipPPreferredServiceHeader::GetNewObj(SipHeaderBase::P_PREFERRED_SERVICE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header not allowed*/
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(SIP_NULL, 0));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPPreferredServiceHeader*>(
            SipPPreferredServiceHeader::GetNewObj(SipHeaderBase::P_PREFERRED_SERVICE, nullptr));

    /* Decode in complete value */
    const SIP_CHAR* pValue = "urn:urn-8";
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ(pValue, &(aBuffer[0]));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPPreferredServiceHeader*>(
            SipPPreferredServiceHeader::GetNewObj(SipHeaderBase::P_PREFERRED_SERVICE, nullptr));
    pValue = "urn:urn-7:gpp-serv--ice.";
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ(pValue, &(aBuffer[0]));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPPreferredServiceHeader*>(
            SipPPreferredServiceHeader::GetNewObj(SipHeaderBase::P_PREFERRED_SERVICE, nullptr));

    /* Decode incomplete subservice value */
    pValue = "urn:urn-7:3gpp-service.ims.icsi.mmtel:.";
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ(pValue, &(aBuffer[0]));
    pHeader->SipDelete();

    /* Decode complete value */
    pHeader = reinterpret_cast<SipPPreferredServiceHeader*>(
            SipPPreferredServiceHeader::GetNewObj(SipHeaderBase::P_PREFERRED_SERVICE, nullptr));
    pValue = "urn:urn-7:3gpp-service.ims.icsi.mmtel";
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ(pValue, &(aBuffer[0]));
    pHeader->SipDelete();

    /* Decode complete value of SipHeaderBase::P_ASSERTED_SERVICE header */
    pHeader = reinterpret_cast<SipPPreferredServiceHeader*>(
            SipPPreferredServiceHeader::GetNewObj(SipHeaderBase::P_ASSERTED_SERVICE, nullptr));
    pValue = "urn:urn-7:3gpp-service.ims.icsi.mmtel";
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ(pValue, &(aBuffer[0]));
    pHeader->SipDelete();
}

}  // namespace android
