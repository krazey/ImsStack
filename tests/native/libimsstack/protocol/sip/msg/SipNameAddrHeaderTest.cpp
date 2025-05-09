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

#include "SipAbnfUtil.h"
#include "msg/SipHeaderBase.h"
#include "platform/SipString.h"

namespace android
{

class SipNameAddrHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipNameAddrHeaderTest, CopyConstructor)
{
    SipNameAddrHeader* pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    SipNameAddr* pNameAddress = pHeader->GetNameAddr();
    ASSERT_TRUE(pNameAddress != nullptr);

    pNameAddress->SetDisplayName("DisplayName");
    pNameAddress->SipDelete();

    SipNameAddrHeader* pCopyHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pNameAddress = pCopyHeader->GetNameAddr();
    ASSERT_TRUE(pNameAddress != nullptr);

    EXPECT_STREQ("DisplayName", pNameAddress->GetDisplayName());

    pNameAddress->SipDelete();
    pCopyHeader->SipDelete();
}

TEST_F(SipNameAddrHeaderTest, SetAddrSpec)
{
    SipNameAddrHeader* pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->SetAddrSpec(nullptr));

    SipAddrSpec* pAddrSpec = new SipAddrSpec();
    EXPECT_EQ(SIP_TRUE, pHeader->SetAddrSpec(pAddrSpec));

    pAddrSpec->SipDelete();

    SipAddrSpec* pNewAddrSpec = new SipAddrSpec();
    EXPECT_EQ(SIP_TRUE, pHeader->SetAddrSpec(pNewAddrSpec));

    pNewAddrSpec->SipDelete();
    pHeader->SipDelete();
}

TEST_F(SipNameAddrHeaderTest, GetNameAddr)
{
    SipNameAddrHeader* pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::CONTACT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    SipNameAddr* pNameAddress = pHeader->GetNameAddr();
    EXPECT_TRUE(pNameAddress != nullptr);

    pNameAddress->SipDelete();
    pHeader->SipDelete();
}

TEST_F(SipNameAddrHeaderTest, Encode)
{
    SipNameAddrHeader* pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::TO, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(nullptr));

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    SipNameAddr* pNameAddress = pHeader->GetNameAddr();
    pNameAddress->SetDisplayName("DisplayName");
    SipAddrSpec* pAddressSpec = new SipAddrSpec();
    pAddressSpec->SetAbsUri("www.absolute-uri.com/abcd");
    pNameAddress->SetAddrSpec(pAddressSpec);
    pNameAddress->SipDelete();

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("DisplayName <www.absolute-uri.com/abcd>", &(aBuffer[0]));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    /* Encode with parameters */
    pHeader->AddParam("param-name", "param-value");

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("DisplayName <www.absolute-uri.com/abcd>;param-name=param-value", &(aBuffer[0]));

    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipNameAddrHeaderTest, Decode)
{
    SipNameAddrHeader* pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::CONTACT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->Decode(nullptr, 0));

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("*", 1));
    EXPECT_STREQ("*", pHeader->GetValue());
    EXPECT_EQ(0, pHeader->GetParamCount());

    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    /* Only Value without display name */
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("www.absolute-uri.com/abcd", 25));
    SipNameAddr* pNameAddress = pHeader->GetNameAddr();
    EXPECT_TRUE(pNameAddress->GetDisplayName() == nullptr);
    SipAddrSpec* pAddressSpec = pNameAddress->GetAddrSpec();
    EXPECT_STREQ("www.absolute-uri.com/abcd", pAddressSpec->GetAbsUri());
    pNameAddress->SipDelete();
    pAddressSpec->SipDelete();
    pNameAddress = nullptr;
    pAddressSpec = nullptr;

    EXPECT_EQ(0, pHeader->GetParamCount());

    pHeader->SipDelete();
    pHeader = nullptr;

    /* Only Value with display name */
    pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pHeader->Decode("DisplayName <www.absolute-uri.com/abcd>", 39));
    pNameAddress = pHeader->GetNameAddr();
    EXPECT_STREQ("DisplayName", pNameAddress->GetDisplayName());
    pAddressSpec = pNameAddress->GetAddrSpec();
    EXPECT_STREQ("www.absolute-uri.com/abcd", pAddressSpec->GetAbsUri());
    pNameAddress->SipDelete();
    pAddressSpec->SipDelete();
    pNameAddress = nullptr;
    pAddressSpec = nullptr;

    EXPECT_EQ(0, pHeader->GetParamCount());

    pHeader->SipDelete();
    pHeader = nullptr;

    /* Value with display name and header parameters */
    pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(
            SIP_TRUE, pHeader->Decode("DisplayName <www.absolute-uri.com/abcd>;tag=tag-value", 53));
    pNameAddress = pHeader->GetNameAddr();
    EXPECT_STREQ("DisplayName", pNameAddress->GetDisplayName());
    pAddressSpec = pNameAddress->GetAddrSpec();
    EXPECT_STREQ("www.absolute-uri.com/abcd", pAddressSpec->GetAbsUri());
    pNameAddress->SipDelete();
    pAddressSpec->SipDelete();
    pNameAddress = nullptr;
    pAddressSpec = nullptr;

    SIP_CHAR* pTagValue = pHeader->GetTag();
    EXPECT_STREQ("tag-value", pTagValue);
    delete[] pTagValue;
    pHeader->SipDelete();
    pHeader = nullptr;

    /* Value without display name and with header parameters */
    pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->Decode("www.absolute-uri.com/abcd;param-name=param-value", 48));
    pNameAddress = pHeader->GetNameAddr();
    EXPECT_TRUE(pNameAddress->GetDisplayName() == nullptr);
    pAddressSpec = pNameAddress->GetAddrSpec();
    EXPECT_STREQ("www.absolute-uri.com/abcd", pAddressSpec->GetAbsUri());
    pNameAddress->SipDelete();
    pAddressSpec->SipDelete();
    pNameAddress = nullptr;
    pAddressSpec = nullptr;

    EXPECT_EQ(1, pHeader->GetParamCount());
    SipNameValue* pNameVal = pHeader->GetParam(0);
    EXPECT_STREQ("param-name", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("param-value", pNameVal->m_objValueList.GetAt(0));

    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipNameAddrHeaderTest, DecodeEncodeSipUri)
{
    SipNameAddrHeader* pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::TO, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    const SIP_CHAR* pUri = "<sip:user%3Binfo@host>";

    EXPECT_EQ(SIP_TRUE, pHeader->Decode(pUri, SipPf_Strlen(pUri)));

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("<sip:user;info@host>", &(aBuffer[0]));

    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::TO, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    pUri = "sip:user;info@host";

    EXPECT_EQ(SIP_TRUE, pHeader->Decode(pUri, SipPf_Strlen(pUri)));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("<sip:user;info@host>", &(aBuffer[0]));

    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    pUri = "sip:2222;222@ims.mnc001.mcc001.3gppnetwork.org;tag=565656";

    EXPECT_EQ(SIP_TRUE, pHeader->Decode(pUri, SipPf_Strlen(pUri)));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ("<sip:2222;222@ims.mnc001.mcc001.3gppnetwork.org>;tag=565656", &(aBuffer[0]));

    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    pUri = "<sip:2222;222@ims.mnc001.mcc001.3gppnetwork.org>;tag=2899e67";

    EXPECT_EQ(SIP_TRUE, pHeader->Decode(pUri, SipPf_Strlen(pUri)));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_TRUE, pHeader->Encode(&pBuff));
    EXPECT_STREQ(pUri, &(aBuffer[0]));

    pHeader->SipDelete();
    pHeader = nullptr;
}
}  // namespace android
