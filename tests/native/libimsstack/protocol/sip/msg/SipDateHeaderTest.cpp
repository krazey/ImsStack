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

#include "msg/SipDateHeader.h"

namespace android
{

class SipDateHeaderTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipDateHeaderTest, EncodeAndEncodeHdr)
{
    SipDateHeader* pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    const SIP_INT32 BUFFER_SIZE = 256;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objValue(256);

    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));

    pHeader->SetDate(35);
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));

    pHeader->SetDate(25);

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    pHeader->SetMonth(SipDateHeader::UNKNOWN_MONTH);
    objValue = AString::ConstNull();
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));

    pHeader->SetMonth(SipDateHeader::JANUARY);
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    pHeader->SetYear(999);
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_EQ(SIP_FALSE, pHeader->EncodeHdr(&pBuff));

    pHeader->SetYear(2050);

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    pHeader->SetWkDay(SipDateHeader::UNKNOWN_DAY);
    EXPECT_EQ(SIP_FALSE, pHeader->Encode(objValue, SIP_FALSE));

    pHeader->SetWkDay(SipDateHeader::MONDAY);
    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_FALSE));

    EXPECT_STREQ("Mon, 25 Jan 2050 00:00:00 GMT", objValue.GetCharString());
    EXPECT_STREQ("Mon, 25 Jan 2050 00:00:00 GMT", &(aBuffer[0]));

    objValue = AString::ConstNull();
    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    pHeader->SetHour(24);
    pHeader->SetMinute(60);
    pHeader->SetSecond(62);
    objValue = AString::ConstNull();
    EXPECT_EQ(SIP_TRUE, pHeader->Encode(objValue, SIP_FALSE));
    EXPECT_STREQ("Mon, 25 Jan 2050 00:00:00 GMT", objValue.GetCharString());

    pHeader->SetMinute(45);
    pHeader->SetHour(21);
    pHeader->SetSecond(30);

    EXPECT_EQ(SIP_TRUE, pHeader->EncodeHdr(&pBuff));
    EXPECT_STREQ("Mon, 25 Jan 2050 21:45:30 GMT", &(aBuffer[0]));

    pHeader->SipDelete();
}

TEST_F(SipDateHeaderTest, DecodeHdr)
{
    SipDateHeader* pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("", 0));

    /* Only month present, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Mon", 3));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Only month and date present, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Mon, 25", 7));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Only month, date and month present, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Tue, 25 Jan ", 12));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* time missing, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Wed, 25 Feb 2050", 16));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* minutes and seconds missing, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Thu, 25 Mar 2050 21", 19));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* seconds missing, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Fri, 25 Apr 2050 21:45", 22));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* time zone GMT missinf, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Sat, 25 May 2050 21:45:30", 25));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Timezone other than GMT, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Sun, 25 Jun 2050 21:45:30 IST", 29));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Invalid weekday, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Son, 25 Jul 2050 21:45:30 GMT", 29));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Invalid weekday, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Min, 25 Jul 2050 21:45:30 GMT", 29));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Invalid weekday, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Teu, 25 Jul 2050 21:45:30 GMT", 29));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Invalid weekday, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Win, 25 Jul 2050 21:45:30 GMT", 29));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Invalid weekday, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Fir, 25 Jul 2050 21:45:30 GMT", 29));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* invalid month, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Mon, 25 Aal 2050 21:45:30 GMT", 29));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* invalid year, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Tue, 25 Aug 999 21:45:30 GMT", 28));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Invalid hour, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Wed, 25 Sep 2050 25:45:30 GMT", 29));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Invalid minutes, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Thu, 25 Oct 2050 21:64:30 GMT", 29));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    /* Invalid seconds, fail */
    EXPECT_EQ(SIP_FALSE, pHeader->DecodeHdr("Fri, 25 Nov 2050 21:45:70 GMT", 29));
    pHeader->SipDelete();

    pHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, nullptr));
    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(SIP_TRUE, pHeader->DecodeHdr("Sat, 25 Dec 2050 21:45:30 GMT", 29));

    EXPECT_EQ(25, pHeader->GetDate());
    EXPECT_EQ(SipDateHeader::DECEMBER, pHeader->GetMonth());
    EXPECT_EQ(2050, pHeader->GetYear());
    EXPECT_EQ(21, pHeader->GetHour());
    EXPECT_EQ(45, pHeader->GetMinute());
    EXPECT_EQ(30, pHeader->GetSecond());
    EXPECT_EQ(SipDateHeader::SATURDAY, pHeader->GetWkDay());

    SipDateHeader* pCopyHeader = reinterpret_cast<SipDateHeader*>(
            SipDateHeader::GetNewObj(SipHeaderBase::DATE, pHeader));
    ASSERT_TRUE(pCopyHeader != nullptr);

    pHeader->SipDelete();

    AStringBuffer objValue(256);

    EXPECT_EQ(SIP_TRUE, pCopyHeader->Encode(objValue, SIP_FALSE));

    EXPECT_STREQ("Sat, 25 Dec 2050 21:45:30 GMT", objValue.GetCharString());

    pCopyHeader->SipDelete();
}

}  // namespace android