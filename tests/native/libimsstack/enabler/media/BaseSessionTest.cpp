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
#include <BaseSession.h>

class BaseSessionTest : public ::testing::Test
{
public:
    std::unique_ptr<BaseSession> m_pSession;

protected:
    virtual void SetUp() override { m_pSession = std::unique_ptr<BaseSession>(new BaseSession()); }

    virtual void TearDown() override {}
};

TEST_F(BaseSessionTest, testConstruction)
{
    EXPECT_EQ(m_pSession->GetLocalIpAddress(), IpAddress::IPv6NONE);
    EXPECT_EQ(m_pSession->GetLocalPort(), 0);
    EXPECT_EQ(m_pSession->GetRemotePort(), 0);
    EXPECT_EQ(m_pSession->GetRtpConfig(), IMS_NULL);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(m_pSession->GetPrevDirection(), MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(m_pSession->GetState(), 0);
}

TEST_F(BaseSessionTest, testSetServiceType)
{
    m_pSession->SetServiceType(MEDIA_SERVICE_DEFAULT);
    EXPECT_EQ(m_pSession->GetServiceType(), MEDIA_SERVICE_DEFAULT);

    m_pSession->SetServiceType(MEDIA_SERVICE_EMERGENCY);
    EXPECT_EQ(m_pSession->GetServiceType(), MEDIA_SERVICE_EMERGENCY);
}

TEST_F(BaseSessionTest, testSetDirection)
{
    m_pSession->SetDirection(MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(m_pSession->GetDirection(), MEDIA_DIRECTION_INVALID);

    m_pSession->SetPrevDirection(MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(m_pSession->GetPrevDirection(), MEDIA_DIRECTION_INVALID);
}

TEST_F(BaseSessionTest, testSetState)
{
    m_pSession->SetState(1);
    EXPECT_EQ(m_pSession->GetState(), 1);
}

TEST_F(BaseSessionTest, testSetLocalEndPoint)
{
    IpAddress objLocalAddr = IpAddress("127.0.0.1");
    IMS_UINT32 nPort = 10000;

    m_pSession->SetLocalEndPoint(objLocalAddr, nPort);
    EXPECT_EQ(m_pSession->GetLocalIpAddress(), objLocalAddr);
    EXPECT_EQ(m_pSession->GetLocalPort(), nPort);
}
