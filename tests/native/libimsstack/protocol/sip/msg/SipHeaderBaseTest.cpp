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
    EXPECT_EQ(0, pCopyHeader->GetParamCount());

    pCopyHeader->SipDelete();
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

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

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
    pHeader->AddParam("handling", "required");

    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("session;handling=required", &(aBuffer[0]));
    EXPECT_STREQ("session;handling=required", objBuffer.GetCharString());

    pHeader->SipDelete();
}

TEST_F(SipHeaderBaseTest, DecodeHdr)
{
    SipHeaderBase* pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ALLOW, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    /* Decode empty header - allowed for ALLOW header */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(SIP_NULL, 0));

    /* Decode ALLOW with value */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("UPDATE", 6));
    EXPECT_STREQ("UPDATE", pHeader->GetValue());
    pHeader->SipDelete();

    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_DISPOSITION, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header value not allowed in content-disposition */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(SIP_NULL, 0));

    /* Decode content-disposition with only value */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("render", 6));
    EXPECT_STREQ("render", pHeader->GetValue());
    pHeader->SipDelete();

    /* Decode content-disposition with value and parameters */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_DISPOSITION, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("render;handling=optional", 24));
    EXPECT_STREQ("render", pHeader->GetValue());
    EXPECT_EQ(1, pHeader->GetParamCount());
    SipNameValue* pNameVal = pHeader->GetParam(0);
    EXPECT_STREQ("handling", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("optional", pNameVal->m_objValueList.GetAt(0));
    pHeader->SipDelete();

    /* Decode with only value and empty parameters */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_DISPOSITION, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("render;", 7));
    pHeader->SipDelete();

    /* Decode with only parameters and empty value */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_DISPOSITION, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(";handling=optional", 18));
    pHeader->SipDelete();

    /* Decode feature-caps with value not as '*', fail */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::FEATURE_CAPS, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("render;param-name=param-value", 29));
    pHeader->SipDelete();

    /* Decode reject-contact with value not as '*', fail */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::REJECT_CONTACT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("render;param-name=param-value", 29));
    pHeader->SipDelete();

    /* Decode accept-contact with value not as '*', fail */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ACCEPT_CONTACT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("render;param-name=param-value", 29));
    pHeader->SipDelete();

    /* Decode accept-contact with value as'*', success */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ACCEPT_CONTACT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("*;param-name=param-value", 24));
    EXPECT_STREQ("*", pHeader->GetValue());
    EXPECT_EQ(1, pHeader->GetParamCount());
    pNameVal = pHeader->GetParam(0);
    EXPECT_STREQ("param-name", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("param-value", pNameVal->m_objValueList.GetAt(0));
    pHeader->SipDelete();
}

TEST_F(SipHeaderBaseTest, DecodeParameter)
{
    SipHeaderBase* pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    /* Decode content-type with value and parameters */
    const SIP_CHAR* pValue = "multipart/mixed;boundary=b_4043f-000a3b";
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    EXPECT_STREQ("multipart/mixed", pHeader->GetValue());

    EXPECT_TRUE(pHeader->IsParamPresent("boundary"));
    EXPECT_EQ(0, pHeader->GetParamIndex("boundary"));
    EXPECT_STREQ("b_4043f-000a3b", pHeader->GetParamValue("boundary", 0));

    /* remove parameter and access parameter */
    pHeader->RemoveParam("boundary");
    EXPECT_FALSE(pHeader->IsParamPresent("boundary"));
    EXPECT_EQ(-1, pHeader->GetParamIndex("boundary"));
    ASSERT_TRUE(pHeader->GetParamValue("boundary", 0) == nullptr);
    pHeader->SipDelete();

    /* Decode with only value and empty parameters */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    pValue = "multipart/mixed";
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    EXPECT_STREQ("multipart/mixed", pHeader->GetValue());

    EXPECT_FALSE(pHeader->IsParamPresent("boundary"));
    EXPECT_EQ(-1, pHeader->GetParamIndex("boundary"));
    ASSERT_TRUE(pHeader->GetParamValue("boundary", 0) == nullptr);

    /* set parameter and access parameter */
    EXPECT_TRUE(pHeader->SetParam("boundary", "4043f-000a3b"));
    EXPECT_TRUE(pHeader->IsParamPresent("boundary"));
    EXPECT_EQ(0, pHeader->GetParamIndex("boundary"));
    EXPECT_STREQ("4043f-000a3b", pHeader->GetParamValue("boundary", 0));
    ASSERT_TRUE(pHeader->GetParamValue("boundary", 1) == nullptr);
    pHeader->SipDelete();

    /* Decode with only parameters and empty value */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::CONTENT_TYPE, nullptr);
    ASSERT_TRUE(pHeader != nullptr);
    pValue = ";boundary=b_4043f-000a3b";
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    pHeader->SipDelete();

    /* Decode accept-contact with value as'*' and multpile parameters */
    pHeader = SipHeaderBase::GetNewObj(SipHeaderBase::ACCEPT_CONTACT, nullptr);
    ASSERT_TRUE(pHeader != nullptr);

    pValue = "*;param1=value1;param2=value2,value3";
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    EXPECT_STREQ("*", pHeader->GetValue());
    EXPECT_EQ(2, pHeader->GetParamCount());

    EXPECT_TRUE(pHeader->IsParamPresent("param1"));
    EXPECT_EQ(0, pHeader->GetParamIndex("param1"));
    EXPECT_STREQ("value1", pHeader->GetParamValue("param1", 0));

    EXPECT_TRUE(pHeader->IsParamPresent("param2"));
    EXPECT_EQ(1, pHeader->GetParamIndex("param2"));
    EXPECT_STREQ("value3", pHeader->GetParamValue("param2", 1));

    EXPECT_EQ(-1, pHeader->GetParamIndex("param3"));

    pHeader->RemoveParam("param1");
    EXPECT_EQ(1, pHeader->GetParamCount());
    EXPECT_EQ(-1, pHeader->GetParamIndex("param1"));
    EXPECT_EQ(0, pHeader->GetParamIndex("param2"));
    EXPECT_STREQ("value2", pHeader->GetParamValue("param2"));

    EXPECT_TRUE(pHeader->SetParam("param3", "value3"));
    EXPECT_EQ(1, pHeader->GetParamIndex("param3"));
    EXPECT_STREQ("value3", pHeader->GetParamValue("param3"));

    EXPECT_TRUE(pHeader->SetParam("param4", SIP_NULL));
    EXPECT_EQ(2, pHeader->GetParamIndex("param4"));
    ASSERT_TRUE(pHeader->GetParamValue("param4", 0) == nullptr);
    pHeader->SipDelete();
}
}  // namespace android
