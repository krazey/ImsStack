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
#include "SipMethod.h"

namespace android
{

class SipMethodTest : public ::testing::Test
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

TEST_F(SipMethodTest, Constructor)
{
    IMS_SINT32 nInviteMethod = SipMethod::INVITE;
    const IMS_CHAR* pszInviteMethod = SipMethod::ToName(nInviteMethod);
    AString strInviteMethod(pszInviteMethod);
    SipMethod objSm1(nInviteMethod);

    EXPECT_EQ(nInviteMethod, objSm1.ToInt());
    EXPECT_STREQ(pszInviteMethod, objSm1.ToString().GetStr());

    SipMethod objSm2(pszInviteMethod);

    EXPECT_EQ(nInviteMethod, objSm2.ToInt());
    EXPECT_STREQ(pszInviteMethod, objSm2.ToString().GetStr());

    SipMethod objSm3(strInviteMethod);

    EXPECT_EQ(nInviteMethod, objSm3.ToInt());
    EXPECT_STREQ(strInviteMethod.GetStr(), objSm3.ToString().GetStr());

    SipMethod objSm4(nInviteMethod);
    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    SipMethod objSm5(objSm4);

    EXPECT_EQ(objSm4.ToInt(), objSm5.ToInt());
    EXPECT_STREQ(objSm4.ToString().GetStr(), objSm5.ToString().GetStr());

    SipMethod objSm6;

    EXPECT_EQ(SipMethod::INVALID, objSm6.ToInt());
    EXPECT_STREQ("", objSm6.ToString().GetStr());

    const IMS_CHAR* pszUnknownInvite = "Invite";
    SipMethod objSm7(pszUnknownInvite);

    EXPECT_EQ(SipMethod::INVALID, objSm7.ToInt());
    EXPECT_STREQ(pszUnknownInvite, objSm7.ToString().GetStr());
}

TEST_F(SipMethodTest, AssignmentOperator)
{
    IMS_SINT32 nInviteMethod = SipMethod::INVITE;
    const IMS_CHAR* pszInviteMethod = SipMethod::ToName(nInviteMethod);
    AString strInviteMethod(pszInviteMethod);

    IMS_SINT32 nByeMethod = SipMethod::BYE;
    const IMS_CHAR* pszByeMethod = SipMethod::ToName(nByeMethod);
    AString strByeMethod(pszByeMethod);

    SipMethod objSm1;

    objSm1 = nByeMethod;

    EXPECT_EQ(nByeMethod, objSm1.ToInt());
    EXPECT_STREQ(pszByeMethod, objSm1.ToString().GetStr());

    objSm1 = pszInviteMethod;

    EXPECT_EQ(nInviteMethod, objSm1.ToInt());
    EXPECT_STREQ(pszInviteMethod, objSm1.ToString().GetStr());

    objSm1 = strByeMethod;

    EXPECT_EQ(nByeMethod, objSm1.ToInt());
    EXPECT_STREQ(strByeMethod.GetStr(), objSm1.ToString().GetStr());

    SipMethod objSm2(nInviteMethod);

    objSm1 = objSm2;

    EXPECT_EQ(objSm1.ToInt(), objSm2.ToInt());
    EXPECT_STREQ(objSm1.ToString().GetStr(), objSm2.ToString().GetStr());

    EXPECT_EQ(nInviteMethod, objSm1.ToInt());
    EXPECT_STREQ(pszInviteMethod, objSm1.ToString().GetStr());

    const IMS_CHAR* pszUnknownInvite = "Invite";

    objSm1 = pszUnknownInvite;

    EXPECT_EQ(SipMethod::INVALID, objSm1.ToInt());
    EXPECT_STREQ(pszUnknownInvite, objSm1.ToString().GetStr());
}

TEST_F(SipMethodTest, EqualsInt)
{
    SipMethod objSm1;

    for (IMS_SINT32 i = SipMethod::ACK; i < SipMethod::MAX; ++i)
    {
        objSm1 = i;
        EXPECT_EQ(IMS_TRUE, objSm1.Equals(i));
    }
}

TEST_F(SipMethodTest, EqualsCharString)
{
    SipMethod objSm1;

    for (IMS_SINT32 i = SipMethod::ACK; i < SipMethod::MAX; ++i)
    {
        const IMS_CHAR* pszMethod = SipMethod::ToName(i);
        objSm1 = pszMethod;
        EXPECT_EQ(IMS_TRUE, objSm1.Equals(pszMethod));
    }
}

TEST_F(SipMethodTest, EqualsAString)
{
    SipMethod objSm1;

    for (IMS_SINT32 i = SipMethod::ACK; i < SipMethod::MAX; ++i)
    {
        AString strMethod(SipMethod::ToName(i));
        objSm1 = strMethod;
        EXPECT_EQ(IMS_TRUE, objSm1.Equals(strMethod));
    }
}

TEST_F(SipMethodTest, EqualsSipMethod)
{
    SipMethod objSm1;
    SipMethod objSm2;

    for (IMS_SINT32 i = SipMethod::ACK; i < SipMethod::MAX; ++i)
    {
        objSm1 = i;
        objSm2 = i;
        EXPECT_EQ(IMS_TRUE, objSm1.Equals(objSm2));
    }
}

TEST_F(SipMethodTest, ToInt)
{
    SipMethod objSm1;

    for (IMS_SINT32 i = SipMethod::ACK; i < SipMethod::MAX; ++i)
    {
        objSm1 = i;
        EXPECT_EQ(i, objSm1.ToInt());
    }
}

TEST_F(SipMethodTest, ToString)
{
    SipMethod objSm1;

    for (IMS_SINT32 i = SipMethod::ACK; i < SipMethod::MAX; ++i)
    {
        objSm1 = i;
        EXPECT_STREQ(SipMethod::ToName(i), objSm1.ToString().GetStr());
    }
}

TEST_F(SipMethodTest, ToName)
{
    EXPECT_STREQ("INVITE", SipMethod::ToName(SipMethod::INVITE));
    EXPECT_STREQ("BYE", SipMethod::ToName(SipMethod::BYE));
    EXPECT_STREQ("", SipMethod::ToName(SipMethod::INVALID));
    EXPECT_STREQ("", SipMethod::ToName(SipMethod::MAX));
}

} // namespace android
