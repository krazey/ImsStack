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
#include "SipStatusCode.h"

namespace android
{

class SipStatusCodeTest : public ::testing::Test
{
public:

protected:
    virtual void SetUp() override
    {
    }

    virtual void TearDown() override
    {
    }
};

TEST_F(SipStatusCodeTest, Constructor)
{
    SipStatusCode objSsc1(SipStatusCode::SC_200);

    EXPECT_EQ(SipStatusCode::SC_200, objSsc1.ToInt());
    EXPECT_STREQ("", objSsc1.GetReasonPhrase().GetStr());

    SipStatusCode objSsc2(SipStatusCode::SC_183, "Session Progress");

    EXPECT_EQ(SipStatusCode::SC_183, objSsc2.ToInt());
    EXPECT_STREQ("Session Progress", objSsc2.GetReasonPhrase().GetStr());

    SipStatusCode objSsc3(SipStatusCode::SC_400, "Bad Request");
    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    SipStatusCode objSsc4(objSsc3);

    EXPECT_EQ(objSsc3.ToInt(), objSsc4.ToInt());
    EXPECT_STREQ(objSsc3.GetReasonPhrase().GetStr(), objSsc4.GetReasonPhrase().GetStr());
}

TEST_F(SipStatusCodeTest, AssignmentOperator)
{
    SipStatusCode objSsc1;

    objSsc1 = SipStatusCode::SC_200;

    EXPECT_EQ(SipStatusCode::SC_200, objSsc1.ToInt());
    EXPECT_STREQ("", objSsc1.GetReasonPhrase().GetStr());

    SipStatusCode objSsc2;
    const IMS_CHAR* pszReasonPhrase = "Session Progress";

    objSsc2 = pszReasonPhrase;

    EXPECT_EQ(SipStatusCode::SC_INVALID, objSsc2.ToInt());
    EXPECT_STREQ(pszReasonPhrase, objSsc2.GetReasonPhrase().GetStr());

    objSsc2 = AString(pszReasonPhrase);

    EXPECT_EQ(SipStatusCode::SC_INVALID, objSsc2.ToInt());
    EXPECT_STREQ(pszReasonPhrase, objSsc2.GetReasonPhrase().GetStr());

    SipStatusCode objSsc3(SipStatusCode::SC_400, "Bad Request");

    objSsc2 = objSsc3;

    EXPECT_EQ(objSsc3.ToInt(), objSsc2.ToInt());
    EXPECT_STREQ(objSsc3.GetReasonPhrase().GetStr(), objSsc2.GetReasonPhrase().GetStr());
}

TEST_F(SipStatusCodeTest, Compare)
{
    SipStatusCode objSsc(SipStatusCode::SC_200);

    EXPECT_LT(0, objSsc.Compare(SipStatusCode::SC_100));
    EXPECT_GT(0, objSsc.Compare(SipStatusCode::SC_300));
    EXPECT_EQ(0, objSsc.Compare(SipStatusCode::SC_200));
}

TEST_F(SipStatusCodeTest, GetReasonPhrase)
{
    SipStatusCode objSsc1(SipStatusCode::SC_200);

    EXPECT_STREQ("", objSsc1.GetReasonPhrase().GetStr());

    SipStatusCode objSsc2(SipStatusCode::SC_200, "SUCCESS");

    EXPECT_STREQ("SUCCESS", objSsc2.GetReasonPhrase().GetStr());

    SipStatusCode objSsc3(SipStatusCode::SC_400,
            SipStatusCode::GetReasonPhrase(SipStatusCode::SC_400));

    EXPECT_STREQ("Bad Request", objSsc3.GetReasonPhrase().GetStr());
}

TEST_F(SipStatusCodeTest, ToInt)
{
    SipStatusCode objSsc1(SipStatusCode::SC_200);

    EXPECT_EQ(SipStatusCode::SC_200, objSsc1.ToInt());

    SipStatusCode objSsc2(SipStatusCode::SC_400, "Bad Request");

    EXPECT_EQ(SipStatusCode::SC_400, objSsc2.ToInt());
}

TEST_F(SipStatusCodeTest, GetReasonPhraseStatic)
{
    EXPECT_STREQ("OK", SipStatusCode::GetReasonPhrase(SipStatusCode::SC_200));

    EXPECT_STREQ("Bad Request", SipStatusCode::GetReasonPhrase(SipStatusCode::SC_400));

    EXPECT_STREQ("", SipStatusCode::GetReasonPhrase(SipStatusCode::SC_MAX));
}

TEST_F(SipStatusCodeTest, Is1XX)
{
    EXPECT_EQ(IMS_TRUE, SipStatusCode::Is1XX(SipStatusCode::SC_100));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::Is1XX(SipStatusCode::SC_183));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::Is1XX(SipStatusCode::SC_199));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::Is1XX(SipStatusCode::SC_200));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::Is1XX(SipStatusCode::SC_300));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::Is1XX(SipStatusCode::SC_400));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::Is1XX(SipStatusCode::SC_500));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::Is1XX(SipStatusCode::SC_600));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::Is1XX(SipStatusCode::SC_MAX));
}

TEST_F(SipStatusCodeTest, IsProvisional)
{
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsProvisional(SipStatusCode::SC_100));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsProvisional(SipStatusCode::SC_183));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsProvisional(SipStatusCode::SC_199));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsProvisional(SipStatusCode::SC_200));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsProvisional(SipStatusCode::SC_300));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsProvisional(SipStatusCode::SC_400));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsProvisional(SipStatusCode::SC_500));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsProvisional(SipStatusCode::SC_600));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsProvisional(SipStatusCode::SC_MAX));
}

TEST_F(SipStatusCodeTest, IsFinal)
{
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinal(SipStatusCode::SC_100));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinal(SipStatusCode::SC_199));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinal(SipStatusCode::SC_200));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinal(SipStatusCode::SC_300));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinal(SipStatusCode::SC_400));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinal(SipStatusCode::SC_500));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinal(SipStatusCode::SC_600));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinal(SipStatusCode::SC_MAX));
}

TEST_F(SipStatusCodeTest, IsFinalSuccess)
{
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinalSuccess(SipStatusCode::SC_100));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinalSuccess(SipStatusCode::SC_199));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinalSuccess(SipStatusCode::SC_200));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinalSuccess(SipStatusCode::SC_202));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinalSuccess(SipStatusCode::SC_300));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinalSuccess(SipStatusCode::SC_400));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinalSuccess(SipStatusCode::SC_500));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinalSuccess(SipStatusCode::SC_600));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinalSuccess(SipStatusCode::SC_MAX));
}

TEST_F(SipStatusCodeTest, IsFinalFailure)
{
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinalFailure(SipStatusCode::SC_100));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinalFailure(SipStatusCode::SC_199));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinalFailure(SipStatusCode::SC_200));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinalFailure(SipStatusCode::SC_300));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinalFailure(SipStatusCode::SC_400));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinalFailure(SipStatusCode::SC_500));
    EXPECT_EQ(IMS_TRUE, SipStatusCode::IsFinalFailure(SipStatusCode::SC_600));
    EXPECT_EQ(IMS_FALSE, SipStatusCode::IsFinalFailure(SipStatusCode::SC_MAX));
}

} // namespace android
