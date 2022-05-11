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
#include "msg/SipAddrSpec.h"

namespace android
{

class SipNameAddrTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipNameAddrTest, CopyConstructor)
{
    SipNameAddr* pSipNameAddr = new SipNameAddr();
    ASSERT_TRUE(pSipNameAddr != nullptr);
    ASSERT_TRUE(pSipNameAddr->GetAddrSpec() == nullptr);

    SipAddrSpec* pSipAddrSpec = new SipAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);

    pSipNameAddr->SetAddrSpec(pSipAddrSpec);

    SipNameAddr* pCopySipNameAddr = new SipNameAddr(*pSipNameAddr);
    ASSERT_TRUE(pCopySipNameAddr != nullptr);

    pSipNameAddr->SipDelete();

    pSipAddrSpec = pCopySipNameAddr->GetAddrSpec();
    ASSERT_TRUE(pSipAddrSpec != nullptr);
    pSipAddrSpec->SipDelete();

    pCopySipNameAddr->SipDelete();
}

TEST_F(SipNameAddrTest, EncodeAndDecodeNameAddr)
{
    SipNameAddr* pSipNameAddr = new SipNameAddr();
    ASSERT_TRUE(pSipNameAddr != nullptr);

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_FALSE, pSipNameAddr->EncodeNameAddr(&pBuff));

    char* pNameAddress = (char*)"";
    char* pEnd = pNameAddress;
    EXPECT_EQ(SIP_FALSE, pSipNameAddr->DecodeNameAddr(nullptr, nullptr));

    pNameAddress = (char*)"DisplayName <http://absoluteuri.addrspec";
    pEnd = pNameAddress + strlen(pNameAddress) - SIP_ONE;

    EXPECT_EQ(SIP_TRUE, pSipNameAddr->DecodeNameAddr(pNameAddress, pEnd));

    EXPECT_STREQ("DisplayName", pSipNameAddr->GetDisplayName());

    EXPECT_EQ(SIP_TRUE, pSipNameAddr->EncodeNameAddr(&pBuff));
    EXPECT_STREQ("DisplayName <http://absoluteuri.addrspec>", &(aBuffer[0]));

    pSipNameAddr->SipDelete();

    pSipNameAddr = new SipNameAddr();
    ASSERT_TRUE(pSipNameAddr != nullptr);

    pNameAddress = (char*)"<http://absoluteuri.addrspec";
    pEnd = pNameAddress + strlen(pNameAddress) - SIP_ONE;
    EXPECT_EQ(SIP_TRUE, pSipNameAddr->DecodeNameAddr(pNameAddress, pEnd));

    EXPECT_STREQ(nullptr, pSipNameAddr->GetDisplayName());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pSipNameAddr->EncodeNameAddr(&pBuff));
    EXPECT_STREQ("<http://absoluteuri.addrspec>", &(aBuffer[0]));

    pSipNameAddr->SipDelete();

    pSipNameAddr = new SipNameAddr();
    ASSERT_TRUE(pSipNameAddr != nullptr);

    pNameAddress = (char*)"\"QuotedDispName\"<http://absoluteuri.addrspec";
    pEnd = pNameAddress + strlen(pNameAddress) - SIP_ONE;
    EXPECT_EQ(SIP_TRUE, pSipNameAddr->DecodeNameAddr(pNameAddress, pEnd));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pSipNameAddr->EncodeNameAddr(&pBuff));
    EXPECT_STREQ("\"QuotedDispName\" <http://absoluteuri.addrspec>", &(aBuffer[0]));

    pSipNameAddr->SipDelete();
}

}  // namespace android
