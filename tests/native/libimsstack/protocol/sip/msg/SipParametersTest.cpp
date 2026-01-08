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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "msg/SipAddrSpec.h"
#include "msg/SipParameters.h"
#include "platform/SipString.h"

namespace android
{

class SipParametersTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipParametersTest, Add_Get_Param)
{
    SipParameters* pParameters = new SipParameters();
    ASSERT_TRUE(pParameters != nullptr);

    /* invalid null input, fail */
    EXPECT_EQ(SIP_FALSE, pParameters->AddParam(nullptr, nullptr));

    EXPECT_EQ(SIP_TRUE, pParameters->AddParam("OnlyName", nullptr));

    const SIP_CHAR* const pParamName = "param-name";
    EXPECT_EQ(SIP_TRUE, pParameters->AddParam(pParamName, "param-value1"));
    EXPECT_EQ(SIP_TRUE, pParameters->AddParam(pParamName, "param-value2"));

    EXPECT_EQ(2, pParameters->GetParamCount());

    EXPECT_EQ(SIP_FALSE, pParameters->IsParamPresent(nullptr));

    EXPECT_EQ(SIP_FALSE, pParameters->IsParamPresent("NameNotExists"));
    EXPECT_EQ(-1, pParameters->GetParamIndex("NameNotExists"));

    EXPECT_EQ(SIP_TRUE, pParameters->IsParamPresent(pParamName));
    EXPECT_EQ(1, pParameters->GetParamIndex(pParamName));

    EXPECT_EQ(nullptr, pParameters->GetParamValue(nullptr));

    EXPECT_EQ(nullptr, pParameters->GetParamValue("NameNotExists"));

    /* no value for param, return null */
    EXPECT_EQ(nullptr, pParameters->GetParamValue("OnlyName"));

    SIP_CHAR* pValue = pParameters->GetParamValue(pParamName, 1);
    EXPECT_STREQ("param-value2", pValue);

    delete[] pValue;

    delete pParameters;
}

TEST_F(SipParametersTest, SetParam)
{
    SipParameters* pParameters = new SipParameters();
    ASSERT_TRUE(pParameters != nullptr);

    /* invalid param name - null , fail */
    EXPECT_EQ(SIP_FALSE, pParameters->SetParam(nullptr, nullptr));

    const SIP_CHAR* const pParamName = "param-name";

    /* New param without value will be added to list, success */
    EXPECT_EQ(SIP_TRUE, pParameters->SetParam(pParamName, nullptr));

    /* Existing param to add value to list as list is empty, success */
    EXPECT_EQ(SIP_TRUE, pParameters->SetParam(pParamName, "param-value"));

    /* Existing param to set value to list at invalid position, fail */
    EXPECT_EQ(SIP_FALSE, pParameters->SetParam(pParamName, "param-value1", 3));

    /* Existing param to set value to list at valid position, success */
    EXPECT_EQ(SIP_TRUE, pParameters->SetParam(pParamName, "param-value1", 0));

    EXPECT_EQ(-1, pParameters->GetParamIndex(nullptr));
    EXPECT_EQ(-1, pParameters->GetParamIndex("NameNotExists"));

    SipNameValue* pNameValue = pParameters->GetParam(0);
    ASSERT_TRUE(pNameValue != nullptr);

    EXPECT_EQ(0, pParameters->GetParamIndex(pParamName));
    EXPECT_EQ(1, pNameValue->m_objValueList.GetSize());

    SIP_CHAR* pValue = pParameters->GetParamValue(pParamName, 0);
    EXPECT_STREQ("param-value1", pValue);
    delete[] pValue;

    /* Existing param to add null value to list - removes value at position, success */
    EXPECT_EQ(SIP_TRUE, pParameters->SetParam(pParamName, nullptr, 0));

    EXPECT_EQ(0, pNameValue->m_objValueList.GetSize());

    delete pParameters;
}

TEST_F(SipParametersTest, DecodeAndEncode)
{
    SipParameters* pParameters = new SipParameters();
    ASSERT_TRUE(pParameters != nullptr);

    const SIP_INT32 BUFFER_SIZE = 256;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objValue(256);

    /* Empty list - no encoding, success */
    EXPECT_EQ(SIP_TRUE, pParameters->Encode(objValue, ' '));
    EXPECT_EQ(SIP_TRUE, pParameters->Encode(&pBuff, ' '));

    /* Decode with empty data, fail */
    const SIP_CHAR* pData = "";
    EXPECT_EQ(SIP_FALSE, pParameters->Decode(pData, pData, ' '));

    pData = "OnlyName;param-name=param-value,param-value1";
    const SIP_CHAR* pDataEnd = pData + SipPf_Strlen(pData) - 1;

    /* Decode with valid input, success */
    EXPECT_EQ(SIP_TRUE, pParameters->Decode(pData, pDataEnd, ';'));

    EXPECT_EQ(2, pParameters->GetParamCount());

    SipParameters* pCopyParameters = new SipParameters(*pParameters);
    ASSERT_TRUE(pCopyParameters != nullptr);

    delete pParameters;

    EXPECT_EQ(SIP_TRUE, pCopyParameters->Encode(objValue, ';'));
    EXPECT_EQ(SIP_TRUE, pCopyParameters->Encode(&pBuff, ';'));

    EXPECT_STREQ(";OnlyName;param-name=param-value,param-value1", objValue.GetCharString());
    EXPECT_STREQ(";OnlyName;param-name=param-value,param-value1", &(aBuffer[0]));

    delete pCopyParameters;

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    /* Normal parameters Decode and Encode with no percent coding, success */
    pParameters = new SipParameters();
    ASSERT_TRUE(pParameters != nullptr);

    pData = "OnlyName;param-name=param%20value,param-value1";
    pDataEnd = pData + SipPf_Strlen(pData) - 1;

    SipUri* pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    EXPECT_EQ(SIP_TRUE, pParameters->Decode(pData, pDataEnd, ';', pSipUri));
    EXPECT_EQ(2, pParameters->GetParamCount());

    pCopyParameters = new SipParameters(*pParameters);
    ASSERT_TRUE(pCopyParameters != nullptr);

    delete pParameters;

    EXPECT_EQ(SIP_TRUE, pCopyParameters->Encode(objValue, ';', pSipUri));
    EXPECT_EQ(SIP_TRUE, pCopyParameters->Encode(&pBuff, ';', pSipUri));

    EXPECT_STREQ(";OnlyName;param-name=param%20value,param-value1", objValue.GetCharString());
    EXPECT_STREQ(";OnlyName;param-name=param%20value,param-value1", &(aBuffer[0]));

    delete pCopyParameters;

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    /* Header parameters Decode and Encode with no percent coding, success */
    pParameters = new SipParameters();
    ASSERT_TRUE(pParameters != nullptr);

    pSipUri->SetComponentType(IParameterComponent::HEADER);

    pData = "OnlyName;param-name=param%20value";
    pDataEnd = pData + SipPf_Strlen(pData) - 1;

    EXPECT_EQ(SIP_TRUE, pParameters->Decode(pData, pDataEnd, ';', pSipUri));

    EXPECT_EQ(2, pParameters->GetParamCount());

    SipNameValue* pNameVal = pParameters->GetParam(1);
    EXPECT_STREQ("param-name", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("param value", pNameVal->m_objValueList.GetAt(0));

    pCopyParameters = new SipParameters(*pParameters);
    ASSERT_TRUE(pCopyParameters != nullptr);

    delete pParameters;

    EXPECT_EQ(SIP_TRUE, pCopyParameters->Encode(objValue, ';', pSipUri));
    EXPECT_EQ(SIP_TRUE, pCopyParameters->Encode(&pBuff, ';', pSipUri));

    EXPECT_STREQ(";OnlyName;param-name=param%20value", objValue.GetCharString());
    EXPECT_STREQ(";OnlyName;param-name=param%20value", &(aBuffer[0]));

    delete pCopyParameters;

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    /* Uri parameters Decode and Encode with no percent coding, success */
    pParameters = new SipParameters();
    ASSERT_TRUE(pParameters != nullptr);

    pData = "OnlyName;transport=%24!%26";
    pDataEnd = pData + SipPf_Strlen(pData) - 1;

    pSipUri->SetComponentType(IParameterComponent::URI);

    EXPECT_EQ(SIP_TRUE, pParameters->Decode(pData, pDataEnd, ';', pSipUri));

    EXPECT_EQ(2, pParameters->GetParamCount());

    pNameVal = pParameters->GetParam(1);
    EXPECT_STREQ("transport", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("$!&", pNameVal->m_objValueList.GetAt(0));

    pCopyParameters = new SipParameters(*pParameters);
    ASSERT_TRUE(pCopyParameters != nullptr);

    delete pParameters;

    EXPECT_EQ(SIP_TRUE, pCopyParameters->Encode(objValue, ';', pSipUri));
    EXPECT_EQ(SIP_TRUE, pCopyParameters->Encode(&pBuff, ';', pSipUri));

    EXPECT_STREQ(";OnlyName;transport=%24!%26", objValue.GetCharString());
    EXPECT_STREQ(";OnlyName;transport=%24!%26", &(aBuffer[0]));

    delete pCopyParameters;
}

}  // namespace android
