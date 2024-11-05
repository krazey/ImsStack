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

#include "AStringBuffer.h"
#include "msg/SipViaHeader.h"

namespace android
{

class SipViaHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipViaHeaderTest, Encode)
{
    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE];
    SIP_CHAR* pBuff = &aBuffer[0];

    AStringBuffer objBuffer(256);

    SipViaHeader* pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));

    pHeader->SetProtocolName("SIP");
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pHeader->SetProtocolVer("2.0");
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pHeader->SetTransport("TCP");
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pHeader->SetHost("[2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]");
    pHeader->SetPortNum((SIP_UINT16)39002);
    EXPECT_EQ(SIP_TRUE, pHeader->SetBranchParam("z9hG4bK1422bd448-755bfe94"));

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));

    EXPECT_STREQ("SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
branch=z9hG4bK1422bd448-755bfe94",
            &aBuffer[0]);

    EXPECT_STREQ("SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]\
:39002;branch=z9hG4bK1422bd448-755bfe94",
            objBuffer.GetCharString());

    pHeader->SipDelete();
}

TEST_F(SipViaHeaderTest, Decode)
{
    SipViaHeader* pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("", 0));

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("SIP", 3));
    pHeader->SipDelete();

    pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("SIP/2.0", 7));
    pHeader->SipDelete();

    pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode("SIP/2.0/TCP", 11));
    pHeader->SipDelete();

    pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("SIP/2.0/TCP 192.168.1.2", 23));
    pHeader->SipDelete();

    pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("SIP/2.0/TCP 192.168.1.2:8080", 28));
    pHeader->SipDelete();

    pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("SIP/2.0/TCP [2001::1]", 21));
    pHeader->SipDelete();

    pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("SIP/2.0/TCP [2001::1]:9192", 26));
    pHeader->SipDelete();

    pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("SIP/2.0/TCP 192.168.1.2:8080;branch=abc", 39));
    pHeader->SipDelete();

    pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("SIP/2.0/TCP [2001::1]:9192;branch=xyz", 37));
    EXPECT_STREQ("SIP", pHeader->GetProtocolName());
    EXPECT_STREQ("2.0", pHeader->GetProtocolVer());
    EXPECT_STREQ("TCP", pHeader->GetTransport());
    EXPECT_STREQ("[2001::1]", pHeader->GetHost());
    EXPECT_EQ(9192, pHeader->GetPort());
    const SIP_CHAR* pBranch = pHeader->GetBranch();
    EXPECT_STREQ("xyz", pBranch);
    delete[] pBranch;
    pHeader->SipDelete();

    pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("SIP/2.0/TCP [2001::1];branch=xyz", 32));
    EXPECT_STREQ("SIP", pHeader->GetProtocolName());
    EXPECT_STREQ("2.0", pHeader->GetProtocolVer());
    EXPECT_STREQ("TCP", pHeader->GetTransport());
    EXPECT_STREQ("[2001::1]", pHeader->GetHost());
    EXPECT_EQ(0, pHeader->GetPort());
    pBranch = pHeader->GetBranch();
    EXPECT_STREQ("xyz", pBranch);
    delete[] pBranch;
    pHeader->SipDelete();

    pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("SIP/2.0/TLS ims.sip.com:5061;branch=a1b", 39));
    EXPECT_STREQ("SIP", pHeader->GetProtocolName());
    EXPECT_STREQ("2.0", pHeader->GetProtocolVer());
    EXPECT_STREQ("TLS", pHeader->GetTransport());
    EXPECT_STREQ("ims.sip.com", pHeader->GetHost());
    EXPECT_EQ(5061, pHeader->GetPort());
    pBranch = pHeader->GetBranch();
    EXPECT_STREQ("a1b", pBranch);
    delete[] pBranch;
    pHeader->SipDelete();

    pHeader =
            reinterpret_cast<SipViaHeader*>(SipViaHeader::GetNewObj(SipHeaderBase::VIA, SIP_NULL));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("SIP/2.0/TCP ims.sip.com;branch=xyz", 34));
    EXPECT_STREQ("SIP", pHeader->GetProtocolName());
    EXPECT_STREQ("2.0", pHeader->GetProtocolVer());
    EXPECT_STREQ("TCP", pHeader->GetTransport());
    EXPECT_STREQ("ims.sip.com", pHeader->GetHost());
    EXPECT_EQ(0, pHeader->GetPort());
    pBranch = pHeader->GetBranch();
    EXPECT_STREQ("xyz", pBranch);
    delete[] pBranch;
    pHeader->SipDelete();
}
}  // namespace android
