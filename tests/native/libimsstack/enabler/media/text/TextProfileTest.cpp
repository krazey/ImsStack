/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include <text/TextProfile.h>

const AString RED_PAYLOAD_TYPE = "RED";
const IMS_SINT32 RED_LEVEL = 1;
const IMS_SINT32 RED_PAYLOAD = 111;
const IMS_BOOL KEEP_REDUNDANT_LEVEL = IMS_FALSE;

class TextProfileTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(TextProfileTest, testRedFmtpRedLevel)
{
    TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp();
    EXPECT_EQ(pRedFmtp->GetRedLevel(), -1);

    pRedFmtp->SetRedLevel(RED_LEVEL);
    EXPECT_EQ(pRedFmtp->GetRedLevel(), RED_LEVEL);

    delete pRedFmtp;
}

TEST_F(TextProfileTest, testRedFmtpRedPayload)
{
    TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp();
    EXPECT_EQ(pRedFmtp->GetRedPayload(), -1);

    pRedFmtp->SetRedPayload(RED_PAYLOAD);
    EXPECT_EQ(pRedFmtp->GetRedPayload(), RED_PAYLOAD);

    delete pRedFmtp;
}

TEST_F(TextProfileTest, testRedFmtpCreation)
{
    TextProfile::RedFmtp* pRedFmtp1 = new TextProfile::RedFmtp();
    EXPECT_EQ(pRedFmtp1->GetRedLevel(), -1);
    EXPECT_EQ(pRedFmtp1->GetRedPayload(), -1);

    TextProfile::RedFmtp* pRedFmtp2 = new TextProfile::RedFmtp(RED_LEVEL, RED_PAYLOAD);
    EXPECT_EQ(pRedFmtp2->GetRedLevel(), RED_LEVEL);
    EXPECT_EQ(pRedFmtp2->GetRedPayload(), RED_PAYLOAD);

    TextProfile::RedFmtp* pRedFmtp3 = new TextProfile::RedFmtp(*pRedFmtp2);
    EXPECT_EQ(pRedFmtp3->GetRedLevel(), RED_LEVEL);
    EXPECT_EQ(pRedFmtp3->GetRedPayload(), RED_PAYLOAD);

    delete pRedFmtp1;
    delete pRedFmtp2;
    delete pRedFmtp3;
}

TEST_F(TextProfileTest, testRedFmtpAssign)
{
    TextProfile::RedFmtp* pRedFmtp1 = new TextProfile::RedFmtp();
    EXPECT_EQ(pRedFmtp1->GetRedLevel(), -1);
    EXPECT_EQ(pRedFmtp1->GetRedPayload(), -1);

    TextProfile::RedFmtp* pRedFmtp2 = new TextProfile::RedFmtp(RED_LEVEL, RED_PAYLOAD);
    EXPECT_EQ(pRedFmtp2->GetRedLevel(), RED_LEVEL);
    EXPECT_EQ(pRedFmtp2->GetRedPayload(), RED_PAYLOAD);

    *pRedFmtp1 = *pRedFmtp2;
    EXPECT_EQ(pRedFmtp1->GetRedLevel(), RED_LEVEL);
    EXPECT_EQ(pRedFmtp1->GetRedPayload(), RED_PAYLOAD);

    delete pRedFmtp1;
    delete pRedFmtp2;
}

TEST_F(TextProfileTest, testTextPayload)
{
    TextProfile::Payload* pPayload1 = new TextProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(RED_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    TextProfile::Payload* pPayload2 = new TextProfile::Payload(*pPayload1);
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp();
    pPayload1->SetFmtp(pRedFmtp);

    TextProfile::Payload* pPayload3 = new TextProfile::Payload(*pPayload1);
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    static_cast<TextProfile::RedFmtp*>(pPayload3->GetFmtp())->SetRedLevel(RED_LEVEL);
    EXPECT_EQ(static_cast<TextProfile::RedFmtp*>(pPayload3->GetFmtp())->GetRedLevel(), RED_LEVEL);

    delete pPayload1;
    delete pPayload2;
    delete pPayload3;
}

TEST_F(TextProfileTest, testTextPayloadAssign)
{
    TextProfile::Payload* pPayload1 = new TextProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(RED_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    TextProfile::Payload* pPayload2 = new TextProfile::Payload();
    *pPayload2 = *pPayload1;
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp();
    pPayload1->SetFmtp(pRedFmtp);

    TextProfile::Payload* pPayload3 = new TextProfile::Payload(*pPayload1);
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    static_cast<TextProfile::RedFmtp*>(pPayload3->GetFmtp())->SetRedLevel(RED_LEVEL);
    EXPECT_EQ(static_cast<TextProfile::RedFmtp*>(pPayload3->GetFmtp())->GetRedLevel(), RED_LEVEL);

    delete pPayload1;
    delete pPayload2;
    delete pPayload3;
}

TEST_F(TextProfileTest, testTextProfileKeepRedundantLevel)
{
    TextProfile* pProfile = new TextProfile();
    EXPECT_EQ(pProfile->GetKeepRedundantLevel(), IMS_TRUE);

    pProfile->SetKeepRedundantLevel(KEEP_REDUNDANT_LEVEL);
    EXPECT_EQ(pProfile->GetKeepRedundantLevel(), KEEP_REDUNDANT_LEVEL);

    delete pProfile;
}

TEST_F(TextProfileTest, testTextProfileCreation)
{
    TextProfile* pProfile1 = new TextProfile();
    pProfile1->SetKeepRedundantLevel(KEEP_REDUNDANT_LEVEL);

    TextProfile* pProfile2 = new TextProfile(pProfile1);
    EXPECT_EQ(pProfile2->GetKeepRedundantLevel(), KEEP_REDUNDANT_LEVEL);

    TextProfile* pProfile3 = new TextProfile(*pProfile1);
    EXPECT_EQ(pProfile3->GetKeepRedundantLevel(), KEEP_REDUNDANT_LEVEL);

    delete pProfile1;
    delete pProfile2;
    delete pProfile3;
}

TEST_F(TextProfileTest, testTextProfileAssign)
{
    TextProfile* pProfile1 = new TextProfile();
    pProfile1->SetKeepRedundantLevel(KEEP_REDUNDANT_LEVEL);

    TextProfile* pProfile2 = new TextProfile();
    *pProfile2 = *pProfile1;

    EXPECT_EQ(*pProfile1, *pProfile2);

    delete pProfile1;
    delete pProfile2;
}

TEST_F(TextProfileTest, testTextProfileEqual)
{
    TextProfile* pProfile1 = new TextProfile();
    pProfile1->SetKeepRedundantLevel(KEEP_REDUNDANT_LEVEL);

    TextProfile* pProfile2 = new TextProfile(pProfile1);
    EXPECT_EQ(*pProfile2, *pProfile1);

    delete pProfile1;
    delete pProfile2;
}

TEST_F(TextProfileTest, testTextProfileNotEqual)
{
    TextProfile* pProfile1 = new TextProfile();
    TextProfile* pProfile2 = new TextProfile(pProfile1);
    pProfile1->SetKeepRedundantLevel(KEEP_REDUNDANT_LEVEL);

    EXPECT_NE(*pProfile2, *pProfile1);

    delete pProfile1;
    delete pProfile2;
}

TEST_F(TextProfileTest, testTextProfileGetPayloadAt)
{
    const IMS_SINT32 nPayload1 = 111;
    const IMS_SINT32 nPayload2 = 112;

    TextProfile* pProfile = new TextProfile();
    TextProfile::Payload* pPayload1 = new TextProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(RED_PAYLOAD_TYPE);
    pPayload1->GetRtpMap().SetPayloadNumber(nPayload1);
    pProfile->GetPayloadList().Append(pPayload1);

    TextProfile::Payload* pPayload2 = new TextProfile::Payload();
    pPayload2->GetRtpMap().SetPayloadType(RED_PAYLOAD_TYPE);
    pPayload2->GetRtpMap().SetPayloadNumber(nPayload2);
    pProfile->GetPayloadList().Append(pPayload2);

    EXPECT_EQ(pProfile->GetPayloadList().GetSize(), 2);

    EXPECT_EQ(pProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), nPayload1);
    EXPECT_EQ(pProfile->GetPayloadAt(1)->GetRtpMap().GetPayloadNumber(), nPayload2);

    delete pProfile;
}
