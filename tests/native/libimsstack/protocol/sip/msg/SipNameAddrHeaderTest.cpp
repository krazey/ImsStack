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
#include "msg/SipHeaderBase.h"
#include "sip_abnfUtil.h"

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

TEST_F(SipNameAddrHeaderTest, IsValidComponent)
{
    SipNameAddrHeader* pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::TO, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->IsValidComponent(nullptr));
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidComponent("sip"));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_USER));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_PASSWORD));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_HOST));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_PORT));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_USER_PRM));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_METHOD));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_MADDR_PRM));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_TTL_PRM));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_TRNSPORT_PRM));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_LR_PRM));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_OTHER_PRM));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidComponent(SIP_HEADERS));

    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipNameAddrHeaderTest, IsPercentEncHdr)
{
    const SIP_INT32 nCount = 9;
    SIP_INT32 arPercentEncodeHeaders[nCount][2] = {
            {SipHeaderBase::TO,              SIP_TRUE },
            {SipHeaderBase::FROM,            SIP_TRUE },
            {SipHeaderBase::ROUTE,           SIP_TRUE },
            {SipHeaderBase::RECORD_ROUTE,    SIP_TRUE },
            {SipHeaderBase::CONTACT,         SIP_TRUE },
            {SipHeaderBase::HISTORY_INFO,    SIP_TRUE },
            {SipHeaderBase::TRIGGER_CONSENT, SIP_TRUE },
            {SipHeaderBase::REFERRED_BY,     SIP_FALSE},
            {SipHeaderBase::REFER_TO,        SIP_FALSE}
    };

    for (SIP_INT32 i = 0; i < nCount; i++)
    {
        SipNameAddrHeader* pHeader = reinterpret_cast<SipNameAddrHeader*>(
                SipNameAddrHeader::GetNewObj(arPercentEncodeHeaders[i][0], nullptr));
        ASSERT_TRUE(pHeader != nullptr);

        EXPECT_EQ(arPercentEncodeHeaders[i][1], pHeader->IsPercentEncHdr());

        pHeader->SipDelete();
    }
}

TEST_F(SipNameAddrHeaderTest, EncodeHdr)
{
    SipNameAddrHeader* pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::TO, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(nullptr));

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    SipNameAddr* pNameAddress = pHeader->GetNameAddr();
    pNameAddress->SetDisplayName("DisplayName");
    SipAddrSpec* pAddressSpec = new SipAddrSpec();
    pAddressSpec->SetAbsUri("www.absolute-uri.com/abcd");
    pNameAddress->SetAddrSpec(pAddressSpec);
    pAddressSpec->SipDelete();
    pNameAddress->SipDelete();

    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ("DisplayName <www.absolute-uri.com/abcd>", &(aBuffer[0]));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    /* Encode with parameters */
    pHeader->InitParameters(SIP_NULL);
    SipParameters* pParameters = pHeader->GetParameters();
    pParameters->AddParam("param-name", "param-value");

    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ("DisplayName <www.absolute-uri.com/abcd>;param-name=param-value", &(aBuffer[0]));

    pHeader->SipDelete();
    pHeader = nullptr;
}

TEST_F(SipNameAddrHeaderTest, DecodeHdr)
{
    SipNameAddrHeader* pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::CONTACT, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(nullptr, 0));

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"*", 1));
    EXPECT_STREQ("*", pHeader->GetValue());
    EXPECT_TRUE(nullptr == pHeader->GetParameters());

    pHeader->SipDelete();
    pHeader = nullptr;

    pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    /* Only Value without display name */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"www.absolute-uri.com/abcd", 25));
    SipNameAddr* pNameAddress = pHeader->GetNameAddr();
    EXPECT_TRUE(pNameAddress->GetDisplayName() == nullptr);
    SipAddrSpec* pAddressSpec = pNameAddress->GetAddrSpec();
    EXPECT_STREQ("www.absolute-uri.com/abcd", pAddressSpec->GetAbsUri());
    pNameAddress->SipDelete();
    pAddressSpec->SipDelete();
    pNameAddress = nullptr;
    pAddressSpec = nullptr;

    EXPECT_TRUE(pHeader->GetParameters() == nullptr);

    pHeader->SipDelete();
    pHeader = nullptr;

    /* Only Value with display name */
    pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"DisplayName <www.absolute-uri.com/abcd>", 39));
    pNameAddress = pHeader->GetNameAddr();
    EXPECT_STREQ("DisplayName", pNameAddress->GetDisplayName());
    pAddressSpec = pNameAddress->GetAddrSpec();
    EXPECT_STREQ("www.absolute-uri.com/abcd", pAddressSpec->GetAbsUri());
    pNameAddress->SipDelete();
    pAddressSpec->SipDelete();
    pNameAddress = nullptr;
    pAddressSpec = nullptr;

    EXPECT_TRUE(pHeader->GetParameters() == nullptr);

    pHeader->SipDelete();
    pHeader = nullptr;

    /* Value with display name and header parameters */
    pHeader = reinterpret_cast<SipNameAddrHeader*>(
            SipNameAddrHeader::GetNewObj(SipHeaderBase::FROM, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE,
            pHeader->DecodeHdr((char*)"DisplayName <www.absolute-uri.com/abcd>;tag=tag-value", 62));
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

    EXPECT_EQ(SIP_TRUE,
            pHeader->DecodeHdr((char*)"www.absolute-uri.com/abcd;param-name=param-value", 48));
    pNameAddress = pHeader->GetNameAddr();
    EXPECT_TRUE(pNameAddress->GetDisplayName() == nullptr);
    pAddressSpec = pNameAddress->GetAddrSpec();
    EXPECT_STREQ("www.absolute-uri.com/abcd", pAddressSpec->GetAbsUri());
    pNameAddress->SipDelete();
    pAddressSpec->SipDelete();
    pNameAddress = nullptr;
    pAddressSpec = nullptr;

    SipParameters* pParameters = pHeader->GetParameters();
    ASSERT_TRUE(pParameters != nullptr);
    SipParameterList* pSipParameterList = pParameters->GetParameterList();
    ASSERT_TRUE(pSipParameterList != nullptr);
    EXPECT_EQ(1, pSipParameterList->GetCount());
    SipNameValue* pNameVal = pSipParameterList->GetNameValNode(0);
    EXPECT_STREQ("param-name", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("param-value", pNameVal->m_valueList.GetAt(0));

    pHeader->SipDelete();
    pHeader = nullptr;
}
}  // namespace android
