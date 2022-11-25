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
#include <gmock/gmock.h>

#include "msg/SipAddrSpec.h"
#include "msg/SipParameters.h"

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
    EXPECT_EQ(SIP_TRUE, pParameters->AddParam("param-name", "param-value1"));
    EXPECT_EQ(SIP_TRUE, pParameters->AddParam("param-name", "param-value2"));

    EXPECT_EQ(2, pParameters->GetParamCount());

    EXPECT_EQ(SIP_FALSE, pParameters->IsParamExists(nullptr, nullptr));

    unsigned int nPosition = ~(0);
    EXPECT_EQ(SIP_FALSE, pParameters->IsParamExists("NameNotExists", &nPosition));
    EXPECT_EQ(~(0), nPosition);

    EXPECT_EQ(SIP_TRUE, pParameters->IsParamExists("param-name", &nPosition));
    EXPECT_EQ(1, nPosition);

    EXPECT_EQ(nullptr, pParameters->GetParamValue(nullptr));

    EXPECT_EQ(nullptr, pParameters->GetParamValue("NameNotExists"));

    /* no value for param, return null */
    EXPECT_EQ(nullptr, pParameters->GetParamValue("OnlyName"));

    char* pValue = pParameters->GetParamValue("param-name", 1);
    EXPECT_STREQ("param-value2", pValue);

    delete[] pValue;

    delete pParameters;
}

TEST_F(SipParametersTest, SetParamValue)
{
    SipParameters* pParameters = new SipParameters();
    ASSERT_TRUE(pParameters != nullptr);

    /* invalid param name - null , fail */
    EXPECT_EQ(SIP_FALSE, pParameters->SetParamValue(nullptr, nullptr));

    /* New param without value will be added to list, success */
    EXPECT_EQ(SIP_TRUE, pParameters->SetParamValue("param-name", nullptr));

    /* Existing param to add value to list as list is empty, success */
    EXPECT_EQ(SIP_TRUE, pParameters->SetParamValue("param-name", "param-value"));

    /* Existing param to set value to list at invalid position, fail */
    EXPECT_EQ(SIP_FALSE, pParameters->SetParamValue("param-name", "param-value1", 3));

    /* Existing param to set value to list at valid position, success */
    EXPECT_EQ(SIP_TRUE, pParameters->SetParamValue("param-name", "param-value1", 0));

    unsigned int nPosition = ~(0);
    EXPECT_EQ(nullptr, pParameters->GetParamNode(nullptr, &nPosition));
    EXPECT_EQ(nullptr, pParameters->GetParamNode("NameNotExists", &nPosition));

    SipNameValue* pNameValue = pParameters->GetParamNode("param-name", &nPosition);
    ASSERT_TRUE(pNameValue != nullptr);

    EXPECT_EQ(0, nPosition);
    EXPECT_EQ(1, pNameValue->m_valueList.GetSize());

    char* pValue = pParameters->GetParamValue("param-name", 0);
    EXPECT_STREQ("param-value1", pValue);
    delete[] pValue;

    /* Existing param to add null value to list - removes value at position, success */
    EXPECT_EQ(SIP_TRUE, pParameters->SetParamValue("param-name", nullptr, 0));

    EXPECT_EQ(0, pNameValue->m_valueList.GetSize());

    delete pParameters;
}

TEST_F(SipParametersTest, DecodeAndEncodeHdr)
{
    SipParameters* pParameters = new SipParameters();
    ASSERT_TRUE(pParameters != nullptr);

    const int BUFFER_SIZE = 256;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    AStringBuffer objValue(256);

    SipParameterList& objParameterList = pParameters->GetParameterList();

    /* Empty list - no encoding, success */
    EXPECT_EQ(SIP_TRUE, objParameterList.Encode(objValue, ' '));
    EXPECT_EQ(SIP_TRUE, objParameterList.Encode(&pBuff, ' '));

    /* Decode with empty data, fail */
    char* pData = const_cast<char*>("");
    EXPECT_EQ(SIP_FALSE, objParameterList.Decode(pData, pData, ' '));

    pData = const_cast<char*>("OnlyName;param-name=param-value,param-value1");
    char* pDataEnd = pData + strlen(pData) - 1;

    /* Decode with valid input, success */
    EXPECT_EQ(SIP_TRUE, objParameterList.Decode(pData, pDataEnd, ';'));

    SipVector<SipNameValue*>& objParameters = objParameterList.GetList();
    EXPECT_EQ(2, objParameters.GetSize());

    SipParameters* pCopyParameters = new SipParameters(*pParameters);
    ASSERT_TRUE(pCopyParameters != nullptr);

    delete pParameters;

    SipParameterList& objCopyParameterList = pCopyParameters->GetParameterList();

    EXPECT_EQ(SIP_TRUE, objCopyParameterList.Encode(objValue, ';'));
    EXPECT_EQ(SIP_TRUE, objCopyParameterList.Encode(&pBuff, ';'));

    EXPECT_STREQ(";OnlyName;param-name=param-value,param-value1", objValue.GetCharString());
    EXPECT_STREQ(";OnlyName;param-name=param-value,param-value1", &(aBuffer[0]));

    delete pCopyParameters;

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    /* Normal parameters Decode and Encode with no percent coding, success */
    pParameters = new SipParameters();
    ASSERT_TRUE(pParameters != nullptr);

    SipParameterList& objNormalParameterList = pParameters->GetParameterList();

    pData = const_cast<char*>("OnlyName;param-name=param%20value,param-value1");
    pDataEnd = pData + strlen(pData) - 1;

    SipUri* pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    EXPECT_EQ(SIP_TRUE, objNormalParameterList.Decode(pData, pDataEnd, ';', pSipUri));

    SipVector<SipNameValue*>& objNormalParameters = objNormalParameterList.GetList();
    EXPECT_EQ(2, objNormalParameters.GetSize());

    pCopyParameters = new SipParameters(*pParameters);
    ASSERT_TRUE(pCopyParameters != nullptr);

    delete pParameters;

    SipParameterList& objNormalCopyParameterList = pCopyParameters->GetParameterList();

    EXPECT_EQ(SIP_TRUE, objNormalCopyParameterList.Encode(objValue, ';', pSipUri));
    EXPECT_EQ(SIP_TRUE, objNormalCopyParameterList.Encode(&pBuff, ';', pSipUri));

    EXPECT_STREQ(";OnlyName;param-name=param%20value,param-value1", objValue.GetCharString());
    EXPECT_STREQ(";OnlyName;param-name=param%20value,param-value1", &(aBuffer[0]));

    delete pCopyParameters;

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    /* Header parameters Decode and Encode with no percent coding, success */
    pParameters = new SipParameters();
    ASSERT_TRUE(pParameters != nullptr);

    SipParameterList& objHeaderParameterList = pParameters->GetParameterList();

    pSipUri->SetComponentType(IParameterComponent::HEADER);

    pData = const_cast<char*>("OnlyName;param-name=param%20value");
    pDataEnd = pData + strlen(pData) - 1;

    EXPECT_EQ(SIP_TRUE, objHeaderParameterList.Decode(pData, pDataEnd, ';', pSipUri));

    SipVector<SipNameValue*>& objHeaderParameters = objHeaderParameterList.GetList();
    EXPECT_EQ(2, objHeaderParameters.GetSize());

    SipNameValue* pNameVal = objHeaderParameters.GetAt(1);
    EXPECT_STREQ("param-name", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("param value", pNameVal->m_valueList.GetAt(0));

    pCopyParameters = new SipParameters(*pParameters);
    ASSERT_TRUE(pCopyParameters != nullptr);

    delete pParameters;

    SipParameterList& objHeaderCopyParameterList = pCopyParameters->GetParameterList();

    EXPECT_EQ(SIP_TRUE, objHeaderCopyParameterList.Encode(objValue, ';', pSipUri));
    EXPECT_EQ(SIP_TRUE, objHeaderCopyParameterList.Encode(&pBuff, ';', pSipUri));

    EXPECT_STREQ(";OnlyName;param-name=param%20value", objValue.GetCharString());
    EXPECT_STREQ(";OnlyName;param-name=param%20value", &(aBuffer[0]));

    delete pCopyParameters;

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    /* Uri parameters Decode and Encode with no percent coding, success */
    pParameters = new SipParameters();
    ASSERT_TRUE(pParameters != nullptr);

    SipParameterList& objUriParameterList = pParameters->GetParameterList();

    pData = const_cast<char*>("OnlyName;transport=%24!%26");
    pDataEnd = pData + strlen(pData) - 1;

    pSipUri->SetComponentType(IParameterComponent::URI);

    EXPECT_EQ(SIP_TRUE, objUriParameterList.Decode(pData, pDataEnd, ';', pSipUri));

    SipVector<SipNameValue*>& objUriParameters = objUriParameterList.GetList();
    EXPECT_EQ(2, objUriParameters.GetSize());

    pNameVal = objUriParameters.GetAt(1);
    EXPECT_STREQ("transport", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("$!&", pNameVal->m_valueList.GetAt(0));

    pCopyParameters = new SipParameters(*pParameters);
    ASSERT_TRUE(pCopyParameters != nullptr);

    delete pParameters;

    SipParameterList& objUriCopyParameterList = pCopyParameters->GetParameterList();

    EXPECT_EQ(SIP_TRUE, objUriCopyParameterList.Encode(objValue, ';', pSipUri));
    EXPECT_EQ(SIP_TRUE, objUriCopyParameterList.Encode(&pBuff, ';', pSipUri));

    EXPECT_STREQ(";OnlyName;transport=%24!%26", objValue.GetCharString());
    EXPECT_STREQ(";OnlyName;transport=%24!%26", &(aBuffer[0]));

    delete pCopyParameters;
}

}  // namespace android
