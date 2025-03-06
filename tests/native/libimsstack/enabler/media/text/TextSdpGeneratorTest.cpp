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

class TextSdpGeneratorTest : public TextSdpGenerator, public ::testing::Test
{
public:
    std::unique_ptr<MockIMediaDescriptor> m_pMockIMediaDescriptor;
    std::unique_ptr<MockISessionDescriptor> m_pMockISessionDescriptor;
    std::unique_ptr<TextProfile> m_pTextProfile;

protected:
    virtual void SetUp() override
    {
        m_pMockIMediaDescriptor = std::unique_ptr<MockIMediaDescriptor>(new MockIMediaDescriptor());
        m_pMockISessionDescriptor =
                std::unique_ptr<MockISessionDescriptor>(new MockISessionDescriptor());
        m_pTextProfile = std::unique_ptr<TextProfile>(new TextProfile());
    }

    virtual void TearDown() override {}
};

TEST_F(TextSdpGeneratorTest, TestGenerateRedFmtp)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp(3, 101);
    pPayload->SetRtpMap(102, "red", 1000);
    pPayload->SetFmtp(pRedFmtp);
    m_pTextProfile->GetPayloadList().Append(pPayload);

    TextProfile::Payload* pSubPayload = new TextProfile::Payload();
    pSubPayload->SetRtpMap(101, "t140", 1000);
    m_pTextProfile->GetPayloadList().Append(pSubPayload);

    EXPECT_TRUE(Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get()));
}

TEST_F(TextSdpGeneratorTest, TestGenerateRedFmtpEmpty)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    TextProfile::RedFmtp* pRedFmtp = new TextProfile::RedFmtp();
    pPayload->SetRtpMap(102, "red", 1000);
    pPayload->SetFmtp(pRedFmtp);
    m_pTextProfile->GetPayloadList().Append(pPayload);

    TextProfile::Payload* pSubPayload = new TextProfile::Payload();
    pSubPayload->SetRtpMap(101, "t140", 1000);
    m_pTextProfile->GetPayloadList().Append(pSubPayload);

    EXPECT_TRUE(Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get()));
}

TEST_F(TextSdpGeneratorTest, TestGenerateRedFmtpNull)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    pPayload->SetRtpMap(102, "red", 1000);
    pPayload->SetFmtp(IMS_NULL);
    m_pTextProfile->GetPayloadList().Append(pPayload);

    TextProfile::Payload* pSubPayload = new TextProfile::Payload();
    pSubPayload->SetRtpMap(101, "t140", 1000);
    m_pTextProfile->GetPayloadList().Append(pSubPayload);

    EXPECT_TRUE(Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get()));
}

TEST_F(TextSdpGeneratorTest, TestGenerateRedFmtpNoSubPayload)
{
    TextProfile::Payload* pPayload = new TextProfile::Payload();
    pPayload->SetRtpMap(102, "red", 1000);
    pPayload->SetFmtp(IMS_NULL);
    m_pTextProfile->GetPayloadList().Append(pPayload);

    EXPECT_TRUE(Generate(
            m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), m_pTextProfile.get()));
}

TEST_F(TextSdpGeneratorTest, TestGenerateNull)
{
    EXPECT_FALSE(Generate(IMS_NULL, m_pMockIMediaDescriptor.get(), m_pTextProfile.get()));
    EXPECT_FALSE(Generate(m_pMockISessionDescriptor.get(), IMS_NULL, m_pTextProfile.get()));
    EXPECT_FALSE(
            Generate(m_pMockISessionDescriptor.get(), m_pMockIMediaDescriptor.get(), IMS_NULL));
}
