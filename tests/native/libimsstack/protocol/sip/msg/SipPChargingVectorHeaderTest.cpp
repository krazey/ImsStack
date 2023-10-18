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
#include "msg/SipPChargingVectorHeader.h"

namespace android
{

class SipPChargingVectorHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipPChargingVectorHeaderTest, CopyConstructor)
{
    SipPChargingVectorHeader* pHeader = reinterpret_cast<SipPChargingVectorHeader*>(
            SipPChargingVectorHeader::GetNewObj(SipHeaderBase::P_CHARGING_VECTOR, nullptr));
    ASSERT_TRUE(pHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(const_cast<char*>("icid-value=1234"), 15));

    SipPChargingVectorHeader* pCopyHeader = reinterpret_cast<SipPChargingVectorHeader*>(
            SipPChargingVectorHeader::GetNewObj(SipHeaderBase::P_CHARGING_VECTOR, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pCopyHeader->SipDelete();
}

TEST_F(SipPChargingVectorHeaderTest, EncodeHdr_Null)
{
    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    SipPChargingVectorHeader* pHeader = reinterpret_cast<SipPChargingVectorHeader*>(
            SipPChargingVectorHeader::GetNewObj(SipHeaderBase::P_CHARGING_VECTOR, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty value not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));
    pHeader->SipDelete();
}

TEST_F(SipPChargingVectorHeaderTest, Encode)
{
    AStringBuffer objBuffer(512);

    SipPChargingVectorHeader* pHeader = reinterpret_cast<SipPChargingVectorHeader*>(
            SipPChargingVectorHeader::GetNewObj(SipHeaderBase::P_CHARGING_VECTOR, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty value not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_TRUE));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingVectorHeader*>(
            SipPChargingVectorHeader::GetNewObj(SipHeaderBase::P_CHARGING_VECTOR, nullptr));

    /* Invalid value */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(const_cast<char*>("=value"), strlen("=value")));
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objBuffer, SIP_TRUE));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingVectorHeader*>(
            SipPChargingVectorHeader::GetNewObj(SipHeaderBase::P_CHARGING_VECTOR, nullptr));

    /* Valid value */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(const_cast<char*>("icid-value=1234"), 15));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("icid-value=1234", objBuffer.GetCharString());

    pHeader->SipDelete();
}

TEST_F(SipPChargingVectorHeaderTest, Encode_DecodeHdr)
{
    SipPChargingVectorHeader* pHeader = reinterpret_cast<SipPChargingVectorHeader*>(
            SipPChargingVectorHeader::GetNewObj(SipHeaderBase::P_CHARGING_VECTOR, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header not allowed */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(SIP_NULL, 0));
    EXPECT_EQ(SIP_FALSE, pHeader->IsValidHeader());
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingVectorHeader*>(
            SipPChargingVectorHeader::GetNewObj(SipHeaderBase::P_CHARGING_VECTOR, nullptr));

    /* Invalid value*/
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(const_cast<char*>("=value"), strlen("=value")));

    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));

    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingVectorHeader*>(
            SipPChargingVectorHeader::GetNewObj(SipHeaderBase::P_CHARGING_VECTOR, nullptr));
    /* Decode invalid value */
    SIP_CHAR* pValue = const_cast<char*>("icid;");
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(pValue, strlen(pValue)));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingVectorHeader*>(
            SipPChargingVectorHeader::GetNewObj(SipHeaderBase::P_CHARGING_VECTOR, nullptr));

    /* Decode valid value */
    pValue = const_cast<char*>("icid-value=1234bc9876e");
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, strlen(pValue)));

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ(pValue, &(aBuffer[0]));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPChargingVectorHeader*>(
            SipPChargingVectorHeader::GetNewObj(SipHeaderBase::P_CHARGING_VECTOR, nullptr));
    /* Decode valid value */
    pValue = const_cast<char*>(
            "icid-value=1234bc9876e;icid-generated-at=192.0.6.8;orig-ioi=home1.net");
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, strlen(pValue)));
    EXPECT_EQ(SIP_TRUE, pHeader->IsValidHeader());

    SipParameters* pParameters = pHeader->GetParameters();
    ASSERT_TRUE(pParameters != nullptr);
    SipParameterList& objParameterList = pParameters->GetParameterList();
    EXPECT_EQ(2, objParameterList.GetCount());
    SipNameValue* pNameVal = objParameterList.GetNameValNode(0);
    EXPECT_STREQ("icid-generated-at", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("192.0.6.8", pNameVal->m_valueList.GetAt(0));
    pNameVal = objParameterList.GetNameValNode(1);
    EXPECT_STREQ("orig-ioi", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("home1.net", pNameVal->m_valueList.GetAt(0));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ(pValue, &(aBuffer[0]));
    pHeader->SipDelete();
}

}  // namespace android
