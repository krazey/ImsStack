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

#include "msg/SipPVisitedNetworkIdHeader.h"
#include "platform/SipString.h"

namespace android
{

class SipPVisitedNetworkIdHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipPVisitedNetworkIdHeaderTest, CopyConstructor)
{
    SipPVisitedNetworkIdHeader* pHeader = reinterpret_cast<SipPVisitedNetworkIdHeader*>(
            SipPVisitedNetworkIdHeader::GetNewObj(SipHeaderBase::P_VISITED_NETWORK_ID, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    SipPVisitedNetworkIdHeader* pCopyHeader = reinterpret_cast<SipPVisitedNetworkIdHeader*>(
            SipPVisitedNetworkIdHeader::GetNewObj(SipHeaderBase::P_VISITED_NETWORK_ID, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    pCopyHeader->SipDelete();
}

TEST_F(SipPVisitedNetworkIdHeaderTest, DecodeHdr)
{
    SipPVisitedNetworkIdHeader* pHeader = reinterpret_cast<SipPVisitedNetworkIdHeader*>(
            SipPVisitedNetworkIdHeader::GetNewObj(SipHeaderBase::P_VISITED_NETWORK_ID, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Empty header not allowed*/
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(SIP_NULL, 0));

    /* Decode ; value */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr(";", 1));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPVisitedNetworkIdHeader*>(
            SipPVisitedNetworkIdHeader::GetNewObj(SipHeaderBase::P_VISITED_NETWORK_ID, nullptr));

    /* any value */
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("any", 3));
    EXPECT_STREQ("any", pHeader->GetValue());
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPVisitedNetworkIdHeader*>(
            SipPVisitedNetworkIdHeader::GetNewObj(SipHeaderBase::P_VISITED_NETWORK_ID, nullptr));
    const SIP_CHAR* pValue = "other.net";
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    EXPECT_STREQ(pValue, pHeader->GetValue());
    EXPECT_EQ(0, pHeader->GetParamCount());
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPVisitedNetworkIdHeader*>(
            SipPVisitedNetworkIdHeader::GetNewObj(SipHeaderBase::P_VISITED_NETWORK_ID, nullptr));

    /* Decode valid value */
    pValue = "Visited network number 1;level=7";
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    EXPECT_STREQ("Visited network number 1", pHeader->GetValue());
    EXPECT_EQ(1, pHeader->GetParamCount());
    SipNameValue* pNameVal = pHeader->GetParam(0);
    EXPECT_STREQ("level", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("7", pNameVal->m_objValueList.GetAt(0));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipPVisitedNetworkIdHeader*>(
            SipPVisitedNetworkIdHeader::GetNewObj(SipHeaderBase::P_VISITED_NETWORK_ID, nullptr));

    /* Decode valid value within quotes */
    pValue = "\"Visited network number 1\"";
    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr(pValue, SipPf_Strlen(pValue)));
    EXPECT_STREQ("Visited network number 1", pHeader->GetValue());
    EXPECT_EQ(0, pHeader->GetParamCount());
    pHeader->SipDelete();
}

}  // namespace android
