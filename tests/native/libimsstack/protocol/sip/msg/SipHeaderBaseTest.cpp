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

class SipHeaderBaseTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipHeaderBaseTest, CopyConstructor)
{
    SipHeaderBase* pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ACCEPT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    pHeader->SetValue("HeaderValue");

    SipHeaderBase* pCopyHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ACCEPT, pHeader);

    pHeader->SipDelete();
    pHeader = nullptr;

    EXPECT_STREQ("HeaderValue", pCopyHeader->GetValue());
    EXPECT_TRUE(pCopyHeader->GetParameters() == nullptr);

    pCopyHeader->InitParameters(SIP_NULL);
    EXPECT_TRUE(pCopyHeader->GetParameters() != nullptr);

    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ACCEPT, pCopyHeader);
    pCopyHeader->SipDelete();

    EXPECT_STREQ("HeaderValue", pHeader->GetValue());
    EXPECT_TRUE(pHeader->GetParameters() != nullptr);

    pHeader->SipDelete();
}

TEST_F(SipHeaderBaseTest, InitParameters)
{
    SipHeaderBase* pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ACCEPT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    pHeader->InitParameters(SIP_NULL);
    EXPECT_TRUE(pHeader->GetParameters() != nullptr);

    SipParameters* pParameters = new SipParameters();
    pHeader->InitParameters(pParameters);

    delete pParameters;

    EXPECT_TRUE(pHeader->GetParameters() != nullptr);
    pHeader->SipDelete();
}

TEST_F(SipHeaderBaseTest, IsValidHeader)
{
    const SIP_INT32 nCount = 23;

    SIP_INT32 arTestData[nCount][2] = {
            {SipHeaderBase::ALLOW,                 SIP_TRUE },
            {SipHeaderBase::CALL_ID,               SIP_FALSE},
            {SipHeaderBase::CONTENT_DISPOSITION,   SIP_FALSE},
            {SipHeaderBase::CONTENT_ENCODING,      SIP_FALSE},
            {SipHeaderBase::MIME_VERSION,          SIP_FALSE},
            {SipHeaderBase::P_ACCESS_NETWORK_INFO, SIP_FALSE},
            {SipHeaderBase::ACCEPT_LANGUAGE,       SIP_TRUE },
            {SipHeaderBase::ANSWER_MODE,           SIP_FALSE},
            {SipHeaderBase::CONTENT_LANGUAGE,      SIP_FALSE},
            {SipHeaderBase::IN_REPLY_TO,           SIP_FALSE},
            {SipHeaderBase::ORGANIZATION,          SIP_TRUE },
            {SipHeaderBase::P_ANSWER_STATE,        SIP_FALSE},
            {SipHeaderBase::P_MEDIA_AUTHORIZATION, SIP_FALSE},
            {SipHeaderBase::PRIORITY,              SIP_FALSE},
            {SipHeaderBase::PRIV_ANSWER_MODE,      SIP_FALSE},
            {SipHeaderBase::PROXY_REQUIRE,         SIP_FALSE},
            {SipHeaderBase::REASON,                SIP_FALSE},
            {SipHeaderBase::SUBJECT,               SIP_TRUE },
            {SipHeaderBase::SUPPRESS_IF_MATCH,     SIP_FALSE},
            {SipHeaderBase::TARGET_DIALOG,         SIP_FALSE},
            {SipHeaderBase::INFO_PACKAGE,          SIP_FALSE},
            {SipHeaderBase::RECV_INFO,             SIP_TRUE },
            {SipHeaderBase::SESSION_ID,            SIP_FALSE}
    };

    for (SIP_INT32 i = 0; i < nCount; i++)
    {
        SipHeaderBase* pHeader = SipHeaderBase::GetNewObj(arTestData[i][0], nullptr);
        ASSERT_TRUE(pHeader != nullptr);

        EXPECT_EQ(arTestData[i][1], pHeader->IsValidHeader());

        pHeader->SetValue("HeaderValue");
        EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

        pHeader->SipDelete();
    }
}

TEST_F(SipHeaderBaseTest, SetValue)
{
    SipHeaderBase* pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ALLOW, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->SetValue("INVITE"));
    EXPECT_STREQ("INVITE", pHeader->GetValue());

    pHeader->SipDelete();

    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ACCEPT_CONTACT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->SetValue("INVITE"));
    EXPECT_EQ(SIP_TRUE, pHeader->SetValue("*"));
    EXPECT_STREQ("*", pHeader->GetValue());

    pHeader->SipDelete();

    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::FEATURE_CAPS, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->SetValue("INVITE"));
    EXPECT_EQ(SIP_TRUE, pHeader->SetValue("*"));
    EXPECT_STREQ("*", pHeader->GetValue());

    pHeader->SipDelete();

    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::REJECT_CONTACT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->SetValue("INVITE"));
    EXPECT_EQ(SIP_TRUE, pHeader->SetValue("*"));
    EXPECT_STREQ("*", pHeader->GetValue());

    pHeader->SipDelete();
}

TEST_F(SipHeaderBaseTest, EncodeAndEncodeHdr)
{
    SipHeaderBase* pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ALLOW, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    /* Encode empty ALLOW allowed */
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));

    pHeader->SetValue("INVITE");

    /* Encode ALLOW with value */
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("INVITE", &(aBuffer[0]));
    EXPECT_STREQ("INVITE", objBuffer.GetCharString());
    pHeader->SipDelete();
    pHeader = SIP_NULL;

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_DISPOSITION, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    /* Encode empty content-disposition not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_FALSE));

    pHeader->SetValue("session");

    /* Encode content-disposition with only value */
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("session", &(aBuffer[0]));
    EXPECT_STREQ("session", objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    /* Encode content-disposition with value and parameters */
    pHeader->InitParameters(SIP_NULL);
    SipParameters* pParameters = pHeader->GetParameters();
    pParameters->AddParam("handling", "required");

    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("session;handling=required", &(aBuffer[0]));
    EXPECT_STREQ("session;handling=required", objBuffer.GetCharString());

    pHeader->SipDelete();
    pHeader = SIP_NULL;
}

TEST_F(SipHeaderBaseTest, DecodeHdr)
{
    SipHeaderBase* pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ALLOW, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    /* Decode empty header - allowed for ALLOW header */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(SIP_NULL, 0));

    /* Decode ALLOW with value */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"UPDATE", 6));
    EXPECT_STREQ("UPDATE", pHeader->GetValue());
    pHeader->SipDelete();
    pHeader = SIP_NULL;

    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_DISPOSITION, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header value not allowed in content-disposition */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(SIP_NULL, 0));

    /* Decode content-disposition with only value */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"render", 6));
    EXPECT_STREQ("render", pHeader->GetValue());
    pHeader->SipDelete();
    pHeader = SIP_NULL;

    /* Decode content-disposition with value and parameters */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_DISPOSITION, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"render;handling=optional", 24));
    EXPECT_STREQ("render", pHeader->GetValue());
    SipParameters* pParameters = pHeader->GetParameters();
    ASSERT_TRUE(pParameters != nullptr);
    SipParameterList& objParameterList = pParameters->GetParameterList();
    EXPECT_EQ(1, objParameterList.GetCount());
    SipNameValue* pNameVal = objParameterList.GetNameValNode(0);
    EXPECT_STREQ("handling", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("optional", pNameVal->m_valueList.GetAt(0));
    pHeader->SipDelete();
    pHeader = SIP_NULL;

    /* Decode with only value and empty parameters */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_DISPOSITION, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr((char*)"render;", 7));
    pHeader->SipDelete();
    pHeader = SIP_NULL;

    /* Decode with only parameters and empty value */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_DISPOSITION, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr((char*)";handling=optional", 18));
    pHeader->SipDelete();
    pHeader = SIP_NULL;

    /* Decode feature-caps with value not as '*', fail */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::FEATURE_CAPS, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr((char*)"render;param-name=param-value", 29));
    pHeader->SipDelete();

    /* Decode reject-contact with value not as '*', fail */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::REJECT_CONTACT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr((char*)"render;param-name=param-value", 29));
    pHeader->SipDelete();

    /* Decode accept-contact with value not as '*', fail */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ACCEPT_CONTACT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr((char*)"render;param-name=param-value", 29));
    pHeader->SipDelete();

    /* Decode accept-contact with value as'*', success */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ACCEPT_CONTACT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr((char*)"*;param-name=param-value", 24));
    EXPECT_STREQ("*", pHeader->GetValue());
    pParameters = pHeader->GetParameters();
    ASSERT_TRUE(pParameters != nullptr);
    SipParameterList& objParameterList1 = pParameters->GetParameterList();
    EXPECT_EQ(1, objParameterList1.GetCount());
    pNameVal = objParameterList1.GetNameValNode(0);
    EXPECT_STREQ("param-name", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("param-value", pNameVal->m_valueList.GetAt(0));
    pHeader->SipDelete();
    pHeader = SIP_NULL;
}
}  // namespace android
