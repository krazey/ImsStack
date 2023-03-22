/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "ImsList.h"
#include "MockIMessage.h"
#include "MtcDef.h"
#include "SipHeaderName.h"
#include "call/MockIMtcCallContext.h"
#include "utility/CallComposerUtil.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

class CallComposerUtilTest : public ::testing::Test
{
public:
    MockIMessage objMessage;
    MockIMtcCallContext objContext;
};

TEST_F(CallComposerUtilTest, GetPriorityNotExist)
{
    ImsList<AString> lstHeaders;
    ON_CALL(objMessage, GetHeaders(AString(SipHeaderName::PRIORITY)))
            .WillByDefault(Return(lstHeaders));

    EXPECT_LT(CallComposerUtil::GetPriority(objMessage), 0);
}

TEST_F(CallComposerUtilTest, GetPriorityWhenNone)
{
    ImsList<AString> lstHeaders;
    lstHeaders.Append("none");
    ON_CALL(objMessage, GetHeaders(AString(SipHeaderName::PRIORITY)))
            .WillByDefault(Return(lstHeaders));

    EXPECT_EQ(CallComposerUtil::GetPriority(objMessage), CALL_COMPOSER_PRIORITY_NONE);
}

TEST_F(CallComposerUtilTest, GetPriorityWhenUrgent)
{
    ImsList<AString> lstHeaders;
    lstHeaders.Append("urgent");
    ON_CALL(objMessage, GetHeaders(AString(SipHeaderName::PRIORITY)))
            .WillByDefault(Return(lstHeaders));

    EXPECT_EQ(CallComposerUtil::GetPriority(objMessage), CALL_COMPOSER_PRIORITY_URGENT);
}

TEST_F(CallComposerUtilTest, GetSubjectNotExist)
{
    ImsList<AString> lstHeaders;
    ON_CALL(objMessage, GetHeaders(AString(SipHeaderName::SUBJECT)))
            .WillByDefault(Return(lstHeaders));

    EXPECT_EQ(CallComposerUtil::GetSubject(objMessage), "");
}

TEST_F(CallComposerUtilTest, GetSubject)
{
    ImsList<AString> lstHeaders;
    lstHeaders.Append("subject");
    ON_CALL(objMessage, GetHeaders(AString(SipHeaderName::SUBJECT)))
            .WillByDefault(Return(lstHeaders));

    EXPECT_EQ(CallComposerUtil::GetSubject(objMessage), "subject");
}

TEST_F(CallComposerUtilTest, GetPictureNotExist)
{
    ImsList<AString> lstHeaders;
    lstHeaders.Append("not-a-picture");
    ON_CALL(objMessage, GetHeaders(AString(SipHeaderName::CALL_INFO)))
            .WillByDefault(Return(lstHeaders));

    EXPECT_EQ(CallComposerUtil::GetPicture(objMessage), "");
}

TEST_F(CallComposerUtilTest, GetPicture)
{
    ImsList<AString> lstHeaders;
    lstHeaders.Append("not-a-picture");
    lstHeaders.Append("<https://it-is-a/picture.jpg>;purpose=icon");
    ON_CALL(objMessage, GetHeaders(AString(SipHeaderName::CALL_INFO)))
            .WillByDefault(Return(lstHeaders));

    EXPECT_EQ(CallComposerUtil::GetPicture(objMessage), "https://it-is-a/picture.jpg");
}

TEST_F(CallComposerUtilTest, GetLocationNotExist)
{
    EXPECT_EQ(CallComposerUtil::GetLocation(objMessage),
            std::make_pair(AString::ConstNull(), AString::ConstNull()));
}

TEST_F(CallComposerUtilTest, SetPriority)
{
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::PRIORITY), AString("none"))).Times(1);
    CallComposerUtil::SetPriority(CALL_COMPOSER_PRIORITY_NONE, objMessage);

    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::PRIORITY), AString("urgent")))
            .Times(1);
    CallComposerUtil::SetPriority(CALL_COMPOSER_PRIORITY_URGENT, objMessage);

    const IMS_SINT32 nInvalidPriority = 9999;
    EXPECT_CALL(objMessage, AddHeader(_, _)).Times(0);
    CallComposerUtil::SetPriority(nInvalidPriority, objMessage);
}

TEST_F(CallComposerUtilTest, SetSubject)
{
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::SUBJECT), _)).Times(0);
    CallComposerUtil::SetSubject("", objMessage);

    const AString strSubject = "subject";
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::SUBJECT), strSubject)).Times(1);
    CallComposerUtil::SetSubject(strSubject, objMessage);
}

TEST_F(CallComposerUtilTest, SetPicture)
{
    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::CALL_INFO), _)).Times(0);
    CallComposerUtil::SetPicture("", objMessage);

    EXPECT_CALL(
            objMessage, AddHeader(AString(SipHeaderName::CALL_INFO), AString("<url>;purpose=icon")))
            .Times(1);
    CallComposerUtil::SetPicture("url", objMessage);
}

TEST_F(CallComposerUtilTest, SetLocation)
{
    const AString strLatitude = "1";
    const AString strLongitude = "2";

    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::GEOLOCATION), _)).Times(0);
    CallComposerUtil::SetLocation("", "", objContext, objMessage);

    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::GEOLOCATION), _)).Times(0);
    CallComposerUtil::SetLocation(strLatitude, "", objContext, objMessage);

    EXPECT_CALL(objMessage, AddHeader(AString(SipHeaderName::GEOLOCATION), _)).Times(0);
    CallComposerUtil::SetLocation("", strLongitude, objContext, objMessage);

    // TODO: Location is hard to test now
    // CallComposerUtil::SetLocation(strLatitude, strLongitude, objContext, objMessage);
}
