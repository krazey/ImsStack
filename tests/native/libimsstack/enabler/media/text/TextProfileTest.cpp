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
const IMS_SINT32 T140_CPS = 30;
const IMS_BOOL T140_CPS_VISIBLE = IMS_TRUE;

class TextProfileTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(TextProfileTest, testT140FmtpCps)
{
    auto pT140Fmtp = std::make_shared<TextProfile::T140Fmtp>();
    EXPECT_EQ(pT140Fmtp->GetCps(), T140_CPS);

    pT140Fmtp->SetCps(T140_CPS + 1);
    EXPECT_EQ(pT140Fmtp->GetCps(), T140_CPS + 1);
}

TEST_F(TextProfileTest, testT140FmtpCpsVisible)
{
    auto pT140Fmtp = std::make_shared<TextProfile::T140Fmtp>();
    EXPECT_EQ(pT140Fmtp->IsCpsVisible(), IMS_FALSE);

    pT140Fmtp->SetVisibleCps(T140_CPS_VISIBLE);
    EXPECT_EQ(pT140Fmtp->IsCpsVisible(), T140_CPS_VISIBLE);
}

TEST_F(TextProfileTest, testT140FmtpCreation)
{
    auto pT140Fmtp1 = std::make_shared<TextProfile::T140Fmtp>();
    EXPECT_EQ(pT140Fmtp1->GetCps(), T140_CPS);
    EXPECT_EQ(pT140Fmtp1->IsCpsVisible(), IMS_FALSE);

    auto pT140Fmtp2 = std::make_shared<TextProfile::T140Fmtp>(T140_CPS + 1, T140_CPS_VISIBLE);
    EXPECT_EQ(pT140Fmtp2->GetCps(), T140_CPS + 1);
    EXPECT_EQ(pT140Fmtp2->IsCpsVisible(), T140_CPS_VISIBLE);

    auto pT140Fmtp3 = std::make_shared<TextProfile::T140Fmtp>(*pT140Fmtp2);
    EXPECT_EQ(pT140Fmtp3->GetCps(), T140_CPS + 1);
    EXPECT_EQ(pT140Fmtp3->IsCpsVisible(), T140_CPS_VISIBLE);
}

TEST_F(TextProfileTest, testT140FmtpAssign)
{
    auto pT140Fmtp1 = std::make_shared<TextProfile::T140Fmtp>();
    auto pT140Fmtp2 = std::make_shared<TextProfile::T140Fmtp>(T140_CPS + 1, T140_CPS_VISIBLE);
    *pT140Fmtp1 = *pT140Fmtp2;
    EXPECT_EQ(*pT140Fmtp1, *pT140Fmtp2);
}

TEST_F(TextProfileTest, testRedFmtpRedLevel)
{
    auto pRedFmtp = std::make_shared<TextProfile::RedFmtp>();
    EXPECT_EQ(pRedFmtp->GetRedLevel(), -1);

    pRedFmtp->SetRedLevel(RED_LEVEL);
    EXPECT_EQ(pRedFmtp->GetRedLevel(), RED_LEVEL);
}

TEST_F(TextProfileTest, testRedFmtpRedPayload)
{
    auto pRedFmtp = std::make_shared<TextProfile::RedFmtp>();
    EXPECT_EQ(pRedFmtp->GetRedPayload(), -1);

    pRedFmtp->SetRedPayload(RED_PAYLOAD);
    EXPECT_EQ(pRedFmtp->GetRedPayload(), RED_PAYLOAD);
}

TEST_F(TextProfileTest, testRedFmtpCreation)
{
    auto pRedFmtp1 = std::make_shared<TextProfile::RedFmtp>();
    EXPECT_EQ(pRedFmtp1->GetRedLevel(), -1);
    EXPECT_EQ(pRedFmtp1->GetRedPayload(), -1);

    auto pRedFmtp2 = std::make_shared<TextProfile::RedFmtp>(RED_LEVEL, RED_PAYLOAD);
    EXPECT_EQ(pRedFmtp2->GetRedLevel(), RED_LEVEL);
    EXPECT_EQ(pRedFmtp2->GetRedPayload(), RED_PAYLOAD);

    auto pRedFmtp3 = std::make_shared<TextProfile::RedFmtp>(*pRedFmtp2);
    EXPECT_EQ(pRedFmtp3->GetRedLevel(), RED_LEVEL);
    EXPECT_EQ(pRedFmtp3->GetRedPayload(), RED_PAYLOAD);
}

TEST_F(TextProfileTest, testRedFmtpAssign)
{
    auto pRedFmtp1 = std::make_shared<TextProfile::RedFmtp>();
    EXPECT_EQ(pRedFmtp1->GetRedLevel(), -1);
    EXPECT_EQ(pRedFmtp1->GetRedPayload(), -1);

    auto pRedFmtp2 = std::make_shared<TextProfile::RedFmtp>(RED_LEVEL, RED_PAYLOAD);
    EXPECT_EQ(pRedFmtp2->GetRedLevel(), RED_LEVEL);
    EXPECT_EQ(pRedFmtp2->GetRedPayload(), RED_PAYLOAD);

    *pRedFmtp1 = *pRedFmtp2;
    EXPECT_EQ(pRedFmtp1->GetRedLevel(), RED_LEVEL);
    EXPECT_EQ(pRedFmtp1->GetRedPayload(), RED_PAYLOAD);
}

TEST_F(TextProfileTest, testTextPayload)
{
    TextProfile::Payload* pPayload1 = new TextProfile::Payload();
    pPayload1->GetRtpMap().SetPayloadType(RED_PAYLOAD_TYPE);
    EXPECT_EQ(pPayload1->GetFmtp(), nullptr);

    TextProfile::Payload* pPayload2 = new TextProfile::Payload(*pPayload1);
    EXPECT_EQ(pPayload2->GetFmtp(), nullptr);

    auto pRedFmtp = std::make_shared<TextProfile::RedFmtp>();
    pPayload1->SetFmtp(pRedFmtp);

    TextProfile::Payload* pPayload3 = new TextProfile::Payload(*pPayload1);
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    auto pFmtp = std::static_pointer_cast<TextProfile::RedFmtp>(pPayload3->GetFmtp());
    pFmtp->SetRedLevel(RED_LEVEL);
    EXPECT_EQ(pFmtp->GetRedLevel(), RED_LEVEL);

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

    auto pRedFmtp = std::make_shared<TextProfile::RedFmtp>();
    pPayload1->SetFmtp(pRedFmtp);

    TextProfile::Payload* pPayload3 = new TextProfile::Payload(*pPayload1);
    EXPECT_NE(pPayload3->GetFmtp(), nullptr);

    auto pFmtp = std::static_pointer_cast<TextProfile::RedFmtp>(pPayload3->GetFmtp());
    pFmtp->SetRedLevel(RED_LEVEL);
    EXPECT_EQ(pFmtp->GetRedLevel(), RED_LEVEL);

    delete pPayload1;
    delete pPayload2;
    delete pPayload3;
}

TEST_F(TextProfileTest, testTextPayloadEqual)
{
    auto pPayload1 = std::make_unique<TextProfile::Payload>();
    pPayload1->GetRtpMap().SetPayloadType(RED_PAYLOAD_TYPE);
    auto pFmtp1 = std::make_shared<TextProfile::RedFmtp>(RED_LEVEL, RED_PAYLOAD);
    pPayload1->SetFmtp(pFmtp1);

    auto pPayload2 = std::make_unique<TextProfile::Payload>();
    pPayload2->GetRtpMap().SetPayloadType(RED_PAYLOAD_TYPE);
    auto pFmtp2 = std::make_shared<TextProfile::RedFmtp>(RED_LEVEL, RED_PAYLOAD);
    pPayload2->SetFmtp(pFmtp2);

    // Should be equal
    EXPECT_EQ(*pPayload1, *pPayload2);

    // Change Fmtp part
    pFmtp2->SetRedLevel(RED_LEVEL + 1);
    EXPECT_NE(*pPayload1, *pPayload2);
    pFmtp2->SetRedLevel(RED_LEVEL);
    EXPECT_EQ(*pPayload1, *pPayload2);

    // Test with null fmtp
    pPayload1->SetFmtp(nullptr);
    EXPECT_NE(*pPayload1, *pPayload2);

    pPayload2->SetFmtp(nullptr);
    EXPECT_EQ(*pPayload1, *pPayload2);

    // Test one side null
    pPayload1->SetFmtp(pFmtp1);
    EXPECT_NE(*pPayload1, *pPayload2);
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

    TextProfile* pProfile2 = new TextProfile(*pProfile1);
    EXPECT_EQ(pProfile2->GetKeepRedundantLevel(), KEEP_REDUNDANT_LEVEL);

    delete pProfile1;
    delete pProfile2;
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

TEST_F(TextProfileTest, testTextProfileNotEqual)
{
    TextProfile* pProfile1 = new TextProfile();
    TextProfile* pProfile2 = new TextProfile(*pProfile1);
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
    pProfile->AddPayload(pPayload1);

    TextProfile::Payload* pPayload2 = new TextProfile::Payload();
    pPayload2->GetRtpMap().SetPayloadType(RED_PAYLOAD_TYPE);
    pPayload2->GetRtpMap().SetPayloadNumber(nPayload2);
    pProfile->AddPayload(pPayload2);

    EXPECT_EQ(pProfile->GetPayloadList().GetSize(), 2);

    EXPECT_EQ(pProfile->GetPayloadAt(0)->GetRtpMap().GetPayloadNumber(), nPayload1);
    EXPECT_EQ(pProfile->GetPayloadAt(1)->GetRtpMap().GetPayloadNumber(), nPayload2);

    delete pProfile;
}
