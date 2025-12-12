/*
 * Copyright (C) 2025 The Android Open Source Project
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
#include <text/TextSdpGenerator.h>

#include <MockISessionDescriptor.h>
#include "core/media/MockIMediaDescriptor.h"

using ::testing::_;

class TextSdpGeneratorTest : public ::testing::Test
{
public:
    std::unique_ptr<TextSdpGenerator> m_pGenerator;
    std::unique_ptr<MockIMediaDescriptor> m_pMockIMediaDescriptor;
    std::unique_ptr<MockISessionDescriptor> m_pMockISessionDescriptor;
    std::unique_ptr<TextProfile> m_pTextProfile;

protected:
    void SetUp() override
    {
        m_pGenerator = std::make_unique<TextSdpGenerator>();
        m_pMockIMediaDescriptor = std::make_unique<MockIMediaDescriptor>();
        m_pMockISessionDescriptor = std::make_unique<MockISessionDescriptor>();
        m_pTextProfile = std::make_unique<TextProfile>();
    }
};

TEST_F(TextSdpGeneratorTest, TestGenerateRedFmtp)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    auto pRedFmtp = std::make_shared<TextProfile::RedFmtp>(3, 101);
    pPayload->SetRtpMap(102, "red", 1000);
    pPayload->SetFmtp(pRedFmtp);
    m_pTextProfile->AddPayload(pPayload);

    TextProfile::Payload* pSubPayload = new TextProfile::Payload();
    pSubPayload->SetRtpMap(101, "t140", 1000);
    m_pTextProfile->AddPayload(pSubPayload);

    EXPECT_CALL(*m_pMockIMediaDescriptor,
            SetMediaFormat(_, AString("102"), AString("red/1000"), AString("101/101/101")))
            .Times(1);
    EXPECT_CALL(*m_pMockIMediaDescriptor,
            SetMediaFormat(_, AString("101"), AString("t140/1000"), AString::ConstNull()))
            .Times(1);
    EXPECT_TRUE(m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get()));
}

TEST_F(TextSdpGeneratorTest, TestGenerateRedFmtpEmpty)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    auto pRedFmtp = std::make_shared<TextProfile::RedFmtp>();
    pPayload->SetRtpMap(102, "red", 1000);
    pPayload->SetFmtp(pRedFmtp);
    m_pTextProfile->AddPayload(pPayload);

    TextProfile::Payload* pSubPayload = new TextProfile::Payload();
    pSubPayload->SetRtpMap(101, "t140", 1000);
    m_pTextProfile->AddPayload(pSubPayload);

    // T140 payload is only taken
    EXPECT_CALL(*m_pMockIMediaDescriptor,
            SetMediaFormat(_, AString("101"), AString("t140/1000"), AString::ConstNull()))
            .Times(1);
    EXPECT_TRUE(m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get()));
}

TEST_F(TextSdpGeneratorTest, TestGenerateRedFmtpNull)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    pPayload->SetRtpMap(102, "red", 1000);
    pPayload->SetFmtp(IMS_NULL);
    m_pTextProfile->AddPayload(pPayload);

    TextProfile::Payload* pSubPayload = new TextProfile::Payload();
    pSubPayload->SetRtpMap(101, "t140", 1000);
    m_pTextProfile->AddPayload(pSubPayload);

    // SetMediaFormat for RED should not be called because GenerateFmtp fails
    EXPECT_CALL(*m_pMockIMediaDescriptor, SetMediaFormat(_, AString("102"), _, _)).Times(0);
    EXPECT_CALL(*m_pMockIMediaDescriptor,
            SetMediaFormat(_, AString("101"), AString("t140/1000"), AString::ConstNull()))
            .Times(1);
    EXPECT_TRUE(m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get()));
}

TEST_F(TextSdpGeneratorTest, TestGenerateRedFmtpNoSubPayload)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    pPayload->SetRtpMap(102, "red", 1000);
    pPayload->SetFmtp(IMS_NULL);
    m_pTextProfile->AddPayload(pPayload);

    // The RED payload is removed because its sub-payload doesn't exist.
    // So, SetMediaFormat should not be called at all.
    EXPECT_CALL(*m_pMockIMediaDescriptor, SetMediaFormat(_, _, _, _)).Times(0);
    EXPECT_TRUE(m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get()));
}

TEST_F(TextSdpGeneratorTest, TestGenerateNull)
{
    EXPECT_FALSE(
            m_pGenerator->Generate(IMS_NULL, m_pMockIMediaDescriptor.get(), m_pTextProfile.get()));
    EXPECT_FALSE(m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), IMS_NULL, m_pTextProfile.get()));
    EXPECT_FALSE(m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), IMS_NULL));
}

TEST_F(TextSdpGeneratorTest, TestGenerateRtpMapNoPayload)
{
    // Test indirectly via Generate()
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    // Empty RtpMap
    m_pTextProfile->AddPayload(pPayload);

    EXPECT_CALL(*m_pMockIMediaDescriptor, SetMediaFormat(_, _, _, _)).Times(0);
    m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get());
}

TEST_F(TextSdpGeneratorTest, TestGenerateRedFmtpWithZeroLevel)
{
    // Add a RED payload with a RedLevel of 0
    TextProfile::Payload* pRedPayload = new TextProfile::Payload();
    auto pRedFmtp = std::make_shared<TextProfile::RedFmtp>(0, 101);  // Level 0
    pRedPayload->SetRtpMap(102, "red", 1000);
    pRedPayload->SetFmtp(pRedFmtp);
    m_pTextProfile->AddPayload(pRedPayload);

    // Add the corresponding T140 payload so the RED payload is not removed
    TextProfile::Payload* pT140Payload = new TextProfile::Payload();
    pT140Payload->SetRtpMap(101, "t140", 1000);
    m_pTextProfile->AddPayload(pT140Payload);

    // Expect SetMediaFormat to be called for RED, but with an empty fmtp string
    EXPECT_CALL(
            *m_pMockIMediaDescriptor, SetMediaFormat(_, AString("102"), _, AString::ConstNull()))
            .Times(1);
    // Expect SetMediaFormat to be called for T140 as well
    EXPECT_CALL(*m_pMockIMediaDescriptor, SetMediaFormat(_, AString("101"), _, _)).Times(1);

    m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get());
}

TEST_F(TextSdpGeneratorTest, TestCheckRedPayloadSubTypeValiditySubTypeMissing)
{
    // Add a RED payload whose sub-payload (101) is NOT in the profile
    TextProfile::Payload* pRedPayload = new TextProfile::Payload();
    auto pRedFmtp = std::make_shared<TextProfile::RedFmtp>(2, 101);
    pRedPayload->SetRtpMap(102, "red", 1000);
    pRedPayload->SetFmtp(pRedFmtp);
    m_pTextProfile->AddPayload(pRedPayload);

    // Add some other payload that doesn't match
    TextProfile::Payload* pOtherPayload = new TextProfile::Payload();
    pOtherPayload->SetRtpMap(105, "t140", 1000);
    m_pTextProfile->AddPayload(pOtherPayload);

    ASSERT_EQ(m_pTextProfile->GetPayloadListSize(), 2);
    // Because the RED sub-payload is missing, the RED payload itself should be removed.
    // Therefore, SetMediaFormat should only be called for the other payload.
    EXPECT_CALL(*m_pMockIMediaDescriptor,
            SetMediaFormat(_, AString("105"), AString("t140/1000"), AString::ConstNull()))
            .Times(1);

    m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get());
}

TEST_F(TextSdpGeneratorTest, TestGenerateRtpMapInvalidPayload)
{
    // Test indirectly via Generate()
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    pPayload->GetRtpMap().SetPayloadNumber(101);
    pPayload->GetRtpMap().SetPayloadType("");  // Invalid empty type
    pPayload->GetRtpMap().SetSamplingRate(1000);
    m_pTextProfile->AddPayload(pPayload);

    EXPECT_CALL(*m_pMockIMediaDescriptor, SetMediaFormat(_, _, _, _)).Times(0);
    m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get());
}

TEST_F(TextSdpGeneratorTest, TestGenerateT140FmtpCpsNotVisible)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    pPayload->GetRtpMap().SetPayloadType("t140");
    pPayload->GetRtpMap().SetPayloadNumber(100);
    pPayload->GetRtpMap().SetSamplingRate(1000);
    auto t140Fmtp = std::make_shared<TextProfile::T140Fmtp>();
    t140Fmtp->SetVisibleCps(IMS_FALSE);  // CPS is not visible
    pPayload->SetFmtp(t140Fmtp);
    m_pTextProfile->AddPayload(pPayload);

    EXPECT_CALL(*m_pMockIMediaDescriptor, SetMediaFormat(_, _, _, AString::ConstNull())).Times(1);
    m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get());
}

TEST_F(TextSdpGeneratorTest, TestGenerateT140FmtpCpsVisible)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    pPayload->GetRtpMap().SetPayloadType("t140");
    pPayload->GetRtpMap().SetPayloadNumber(100);
    pPayload->GetRtpMap().SetSamplingRate(1000);
    auto t140Fmtp = std::make_shared<TextProfile::T140Fmtp>();
    t140Fmtp->SetCps(40);
    t140Fmtp->SetVisibleCps(IMS_TRUE);  // CPS is visible
    pPayload->SetFmtp(t140Fmtp);
    m_pTextProfile->AddPayload(pPayload);

    EXPECT_CALL(*m_pMockIMediaDescriptor, SetMediaFormat(_, _, _, AString("cps=40"))).Times(1);
    m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get());
}

TEST_F(TextSdpGeneratorTest, TestGenerateT140FmtpNull)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    pPayload->GetRtpMap().SetPayloadType("t140");
    pPayload->GetRtpMap().SetPayloadNumber(100);
    pPayload->GetRtpMap().SetSamplingRate(1000);
    pPayload->SetFmtp(nullptr);  // Null FMTP
    m_pTextProfile->AddPayload(pPayload);

    EXPECT_CALL(*m_pMockIMediaDescriptor, SetMediaFormat(_, _, _, AString::ConstNull())).Times(1);
    m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get());
}

TEST_F(TextSdpGeneratorTest, TestCheckRedPayloadSubTypeValidityNullInList)
{
    // Add a valid RED payload
    TextProfile::Payload* pRedPayload = new TextProfile::Payload();
    auto pRedFmtp = std::make_shared<TextProfile::RedFmtp>(2, 101);
    pRedPayload->SetRtpMap(102, "red", 1000);
    pRedPayload->SetFmtp(pRedFmtp);
    m_pTextProfile->AddPayload(pRedPayload);

    // Add a null payload in the middle
    m_pTextProfile->GetPayloadList().Append(nullptr);

    // Add the corresponding T140 payload
    TextProfile::Payload* pT140Payload = new TextProfile::Payload();
    pT140Payload->SetRtpMap(101, "t140", 1000);
    m_pTextProfile->AddPayload(pT140Payload);

    ASSERT_EQ(m_pTextProfile->GetPayloadListSize(), 3);
    EXPECT_CALL(*m_pMockIMediaDescriptor,
            SetMediaFormat(_, AString("102"), AString("red/1000"), AString("101/101")))
            .Times(1);
    EXPECT_CALL(*m_pMockIMediaDescriptor,
            SetMediaFormat(_, AString("101"), AString("t140/1000"), AString::ConstNull()))
            .Times(1);

    m_pGenerator->Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get());
}
