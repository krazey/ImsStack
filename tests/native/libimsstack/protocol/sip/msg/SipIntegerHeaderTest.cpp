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
#include "msg/SipHeaders.h"
#include "msg/SipIntegerHeader.h"
#include "msg/SipParameters.h"
#include "msg/SipMsgUtil.h"

namespace android
{

class SipIntegerHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipIntegerHeaderTest, SetValueInt)
{
    SipIntegerHeader* pContentLengthHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::CONTENT_LENGTH));
    ASSERT_TRUE(pContentLengthHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->SetValueInt(20));
    EXPECT_EQ(20, pContentLengthHeader->GetValueInt());
    pContentLengthHeader->SipDelete();

    SipIntegerHeader* pExpiresHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::EXPIRES_SEC));
    ASSERT_TRUE(pExpiresHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pExpiresHeader->SetValueInt(20));
    EXPECT_EQ(20, pExpiresHeader->GetValueInt());
    pExpiresHeader->SipDelete();

    SipIntegerHeader* pMinExpiresHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MIN_EXPIRES));
    ASSERT_TRUE(pMinExpiresHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMinExpiresHeader->SetValueInt(20));
    EXPECT_EQ(20, pMinExpiresHeader->GetValueInt());
    pMinExpiresHeader->SipDelete();

    SipIntegerHeader* pMaxForwardsHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS));
    ASSERT_TRUE(pMaxForwardsHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMaxForwardsHeader->SetValueInt(20));
    EXPECT_EQ(20, pMaxForwardsHeader->GetValueInt());
    EXPECT_EQ(SIP_FALSE, pMaxForwardsHeader->SetValueInt(MAX_MAXFD + 1));
    pMaxForwardsHeader->SipDelete();

    SipIntegerHeader* pGeoLocationErrorHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::GEOLOCATION_ERROR));
    ASSERT_TRUE(pGeoLocationErrorHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pGeoLocationErrorHeader->SetValueInt(20));
    EXPECT_EQ(20, pGeoLocationErrorHeader->GetValueInt());
    EXPECT_EQ(SIP_FALSE, pGeoLocationErrorHeader->SetValueInt(MAX_ERROR_CODE + 1));
    pGeoLocationErrorHeader->SipDelete();

    SipIntegerHeader* pMinSeHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MIN_SE));
    ASSERT_TRUE(pMinSeHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMinSeHeader->SetValueInt(20));
    EXPECT_EQ(20, pMinSeHeader->GetValueInt());
    pMinSeHeader->SipDelete();

    SipIntegerHeader* pSessionExpiresHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::SESSION_EXPIRES));
    ASSERT_TRUE(pSessionExpiresHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pSessionExpiresHeader->SetValueInt(20));
    EXPECT_EQ(20, pSessionExpiresHeader->GetValueInt());
    pSessionExpiresHeader->SipDelete();

    SipIntegerHeader* pFlowTimerHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::FLOW_TIMER));
    ASSERT_TRUE(pFlowTimerHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pFlowTimerHeader->SetValueInt(20));
    EXPECT_EQ(20, pFlowTimerHeader->GetValueInt());
    pFlowTimerHeader->SipDelete();

    SipIntegerHeader* pMaxBreadthHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_BREADTH));
    ASSERT_TRUE(pMaxBreadthHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMaxBreadthHeader->SetValueInt(20));
    EXPECT_EQ(20, pMaxBreadthHeader->GetValueInt());
    pMaxBreadthHeader->SipDelete();

    SipIntegerHeader* pRSeqHeader =
            reinterpret_cast<SipIntegerHeader*>(SipHeaders::CreateCoreHdrObj(SipHeaderBase::RSEQ));
    ASSERT_TRUE(pRSeqHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pRSeqHeader->SetValueInt(20));
    EXPECT_EQ(20, pRSeqHeader->GetValueInt());
    pRSeqHeader->SipDelete();
}

TEST_F(SipIntegerHeaderTest, EncodeAndEncodeHdr)
{
    const int BUFFER_SIZE = 4096;
    char aBuffer[BUFFER_SIZE] = {
            0,
    };
    char* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    SipIntegerHeader* pContentLengthHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::CONTENT_LENGTH));
    ASSERT_TRUE(pContentLengthHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("0", &(aBuffer[0]));
    EXPECT_STREQ("0", objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->SetValueInt(20));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("20", &(aBuffer[0]));
    EXPECT_STREQ("20", objBuffer.GetCharString());
    pContentLengthHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    SipIntegerHeader* pExpiresHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::EXPIRES_SEC));
    ASSERT_TRUE(pExpiresHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pExpiresHeader->SetValueInt(60000));
    EXPECT_EQ(SIP_TRUE, pExpiresHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("60000", &(aBuffer[0]));
    EXPECT_STREQ("60000", objBuffer.GetCharString());
    pExpiresHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    SipIntegerHeader* pMinExpiresHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MIN_EXPIRES));
    ASSERT_TRUE(pMinExpiresHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMinExpiresHeader->SetValueInt(1200));
    EXPECT_EQ(SIP_TRUE, pMinExpiresHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("1200", &(aBuffer[0]));
    EXPECT_STREQ("1200", objBuffer.GetCharString());
    pMinExpiresHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    SipIntegerHeader* pMaxForwardsHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS));
    ASSERT_TRUE(pMaxForwardsHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMaxForwardsHeader->SetValueInt(8));
    EXPECT_EQ(SIP_TRUE, pMaxForwardsHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("8", &(aBuffer[0]));
    EXPECT_STREQ("8", objBuffer.GetCharString());
    pMaxForwardsHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    SipIntegerHeader* pMinSeHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MIN_SE));
    ASSERT_TRUE(pMinSeHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMinSeHeader->SetValueInt(360));
    EXPECT_EQ(SIP_TRUE, pMinSeHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("360", &(aBuffer[0]));
    EXPECT_STREQ("360", objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    SipParameters* pParameters = pMinSeHeader->GetParameters();

    if (pParameters == SIP_NULL)
    {
        pMinSeHeader->InitParameters(SIP_NULL);
        pParameters = pMinSeHeader->GetParameters();
    }
    pParameters->AddParam("generic-param", "generic-value");
    EXPECT_EQ(SIP_TRUE, pMinSeHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("360;generic-param=generic-value", &(aBuffer[0]));
    EXPECT_STREQ("360;generic-param=generic-value", objBuffer.GetCharString());
    pMinSeHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    SipIntegerHeader* pSessionExpiresHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::SESSION_EXPIRES));
    ASSERT_TRUE(pSessionExpiresHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pSessionExpiresHeader->SetValueInt(2400));
    EXPECT_EQ(SIP_TRUE, pSessionExpiresHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("2400", &(aBuffer[0]));
    EXPECT_STREQ("2400", objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pParameters = pSessionExpiresHeader->GetParameters();

    if (pParameters == SIP_NULL)
    {
        pSessionExpiresHeader->InitParameters(SIP_NULL);
        pParameters = pSessionExpiresHeader->GetParameters();
    }
    pParameters->AddParam("refresher", "uac");
    EXPECT_EQ(SIP_TRUE, pSessionExpiresHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("2400;refresher=uac", &(aBuffer[0]));
    EXPECT_STREQ("2400;refresher=uac", objBuffer.GetCharString());
    pSessionExpiresHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    SipIntegerHeader* pFlowTimerHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::FLOW_TIMER));
    ASSERT_TRUE(pFlowTimerHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pFlowTimerHeader->SetValueInt(20));
    EXPECT_EQ(SIP_TRUE, pFlowTimerHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("20", &(aBuffer[0]));
    EXPECT_STREQ("20", objBuffer.GetCharString());
    pFlowTimerHeader->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    SipIntegerHeader* pMaxBreadthHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_BREADTH));
    ASSERT_TRUE(pMaxBreadthHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pMaxBreadthHeader->SetValueInt(120));
    EXPECT_EQ(SIP_TRUE, pMaxBreadthHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("120", &(aBuffer[0]));
    EXPECT_STREQ("120", objBuffer.GetCharString());
    pMaxBreadthHeader->SipDelete();
}

TEST_F(SipIntegerHeaderTest, DecodeHdr)
{
    SipIntegerHeader* pContentLengthHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::CONTENT_LENGTH));
    ASSERT_TRUE(pContentLengthHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pContentLengthHeader->DecodeHdr(const_cast<char*>(""), 0));
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->DecodeHdr(const_cast<char*>("20"), 2));
    EXPECT_EQ(20, pContentLengthHeader->GetValueInt());
    pContentLengthHeader->SipDelete();
    pContentLengthHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::CONTENT_LENGTH));
    ASSERT_TRUE(pContentLengthHeader != nullptr);
    EXPECT_EQ(SIP_TRUE, pContentLengthHeader->DecodeHdr(const_cast<char*>("0"), 1));
    EXPECT_EQ(0, pContentLengthHeader->GetValueInt());

    SipIntegerHeader* pExpiresHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::EXPIRES_SEC));
    ASSERT_TRUE(pExpiresHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pExpiresHeader->DecodeHdr(const_cast<char*>(""), 0));
    EXPECT_EQ(SIP_TRUE, pExpiresHeader->DecodeHdr(const_cast<char*>("120"), 3));
    EXPECT_EQ(120, pExpiresHeader->GetValueInt());
    pExpiresHeader->SipDelete();

    SipIntegerHeader* pMinExpiresHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MIN_EXPIRES));
    ASSERT_TRUE(pMinExpiresHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pMinExpiresHeader->DecodeHdr(const_cast<char*>(""), 0));
    EXPECT_EQ(SIP_TRUE, pMinExpiresHeader->DecodeHdr(const_cast<char*>("90"), 2));
    EXPECT_EQ(90, pMinExpiresHeader->GetValueInt());
    pMinExpiresHeader->SipDelete();

    SipIntegerHeader* pMaxForwardsHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS));
    ASSERT_TRUE(pMaxForwardsHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pMaxForwardsHeader->DecodeHdr(const_cast<char*>(""), 0));
    EXPECT_EQ(SIP_TRUE, pMaxForwardsHeader->DecodeHdr(const_cast<char*>("250"), 3));
    EXPECT_EQ(250, pMaxForwardsHeader->GetValueInt());
    pMaxForwardsHeader->SipDelete();
    pMaxForwardsHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS));
    ASSERT_TRUE(pMaxForwardsHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pMaxForwardsHeader->DecodeHdr(const_cast<char*>("256"), 3));
    pMaxForwardsHeader->SipDelete();

    SipIntegerHeader* pGeoLocationErrorHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::GEOLOCATION_ERROR));
    ASSERT_TRUE(pGeoLocationErrorHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pGeoLocationErrorHeader->DecodeHdr(const_cast<char*>(""), 0));
    EXPECT_EQ(SIP_TRUE, pGeoLocationErrorHeader->DecodeHdr(const_cast<char*>("2505"), 4));
    EXPECT_EQ(2505, pGeoLocationErrorHeader->GetValueInt());
    pGeoLocationErrorHeader->SipDelete();
    pGeoLocationErrorHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::GEOLOCATION_ERROR));
    ASSERT_TRUE(pGeoLocationErrorHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pGeoLocationErrorHeader->DecodeHdr(const_cast<char*>("10000"), 5));
    pGeoLocationErrorHeader->SipDelete();

    SipIntegerHeader* pMinSeHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MIN_SE));
    ASSERT_TRUE(pMinSeHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pMinSeHeader->DecodeHdr(const_cast<char*>(""), 0));
    EXPECT_EQ(SIP_TRUE, pMinSeHeader->DecodeHdr(const_cast<char*>("1400"), 4));
    EXPECT_EQ(1400, pMinSeHeader->GetValueInt());
    pMinSeHeader->SipDelete();
    pMinSeHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MIN_SE));
    ASSERT_TRUE(pMinSeHeader != nullptr);
    EXPECT_EQ(SIP_TRUE,
            pMinSeHeader->DecodeHdr(const_cast<char*>("1400;generic-param=generic-value"), 32));
    SipParameters* pParameters = pMinSeHeader->GetParameters();
    ASSERT_TRUE(pParameters != nullptr);
    SipParameterList& objParameterList = pParameters->GetParameterList();
    EXPECT_EQ(1, objParameterList.GetCount());
    SipNameValue* pNameVal = objParameterList.GetNameValNode(0);
    EXPECT_STREQ("generic-param", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("generic-value", pNameVal->m_valueList.GetAt(0));
    pMinSeHeader->SipDelete();

    SipIntegerHeader* pSessionExpiresHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::SESSION_EXPIRES));
    ASSERT_TRUE(pSessionExpiresHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pSessionExpiresHeader->DecodeHdr(const_cast<char*>(""), 0));
    EXPECT_EQ(SIP_TRUE, pSessionExpiresHeader->DecodeHdr(const_cast<char*>("3600"), 4));
    EXPECT_EQ(3600, pSessionExpiresHeader->GetValueInt());
    pSessionExpiresHeader->SipDelete();
    pMinSeHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MIN_SE));
    ASSERT_TRUE(pMinSeHeader != nullptr);
    EXPECT_EQ(SIP_TRUE,
            pSessionExpiresHeader->DecodeHdr(const_cast<char*>("1600;refresher=uas"), 18));
    pParameters = pSessionExpiresHeader->GetParameters();
    ASSERT_TRUE(pParameters != nullptr);
    SipParameterList& objParameterList1 = pParameters->GetParameterList();
    EXPECT_EQ(1, objParameterList1.GetCount());
    pNameVal = objParameterList1.GetNameValNode(0);
    EXPECT_STREQ("refresher", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_valueList.GetSize());
    EXPECT_STREQ("uas", pNameVal->m_valueList.GetAt(0));
    pSessionExpiresHeader->SipDelete();

    SipIntegerHeader* pFlowTimerHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::FLOW_TIMER));
    ASSERT_TRUE(pFlowTimerHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pFlowTimerHeader->DecodeHdr(const_cast<char*>(""), 0));
    EXPECT_EQ(SIP_TRUE, pFlowTimerHeader->DecodeHdr(const_cast<char*>("8"), 1));
    EXPECT_EQ(8, pFlowTimerHeader->GetValueInt());
    pFlowTimerHeader->SipDelete();

    SipIntegerHeader* pMaxBreadthHeader = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_BREADTH));
    ASSERT_TRUE(pMaxBreadthHeader != nullptr);
    EXPECT_EQ(SIP_FALSE, pMaxBreadthHeader->DecodeHdr(const_cast<char*>(""), 0));
    EXPECT_EQ(SIP_TRUE, pMaxBreadthHeader->DecodeHdr(const_cast<char*>("13"), 2));
    EXPECT_EQ(13, pMaxBreadthHeader->GetValueInt());
    pMaxBreadthHeader->SipDelete();
}
}  // namespace android
