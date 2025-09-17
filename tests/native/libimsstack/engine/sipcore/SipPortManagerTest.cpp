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

#include "SipPortManager.h"

#include "PlatformContext.h"
#include "TestNetworkService.h"

using ::testing::Return;

namespace android
{

class SipPortManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_pPortManager = SipPortManager::GetInstance();
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_NETWORK, &m_objNetworkService);
    }

    void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_NETWORK, IMS_NULL);
        m_pPortManager->Clear();
    }

protected:
    TestNetworkService m_objNetworkService;
    SipPortManager* m_pPortManager;
};

TEST_F(SipPortManagerTest, GetPortC)
{
    IMS_SINT32 nPortStart = 40000;
    IMS_SINT32 nPortEnd = 40100;
    m_pPortManager->SetPortC(nPortStart, nPortEnd);

    IpAddress objIp("192.168.0.1");
    IMS_SINT32 nStartPortC = m_pPortManager->GetPortC(objIp);

    for (IMS_SINT32 i = nPortStart + 1; i < nPortEnd; ++i)
    {
        IMS_SINT32 nPortC = m_pPortManager->GetPortC(objIp);

        EXPECT_TRUE(nPortC >= nPortStart && nPortC < nPortEnd);
    }

    IMS_SINT32 nEndPortC = m_pPortManager->GetPortC(objIp);

    // Round robin
    EXPECT_EQ(nStartPortC, nEndPortC);
}

TEST_F(SipPortManagerTest, IsPortCProvisioned)
{
    // Normal case
    m_pPortManager->SetPortC(40000, 50000);

    EXPECT_TRUE(m_pPortManager->IsPortCProvisioned());

    // Invalid Port-End: use max port number.
    m_pPortManager->SetPortC(50000, -1);

    EXPECT_TRUE(m_pPortManager->IsPortCProvisioned());

    // Invalid Port-End: use max port number.
    m_pPortManager->SetPortC(50000, 65536);

    EXPECT_TRUE(m_pPortManager->IsPortCProvisioned());

    // Port-End is greater than Port-Start.
    m_pPortManager->SetPortC(50000, 40000);

    EXPECT_FALSE(m_pPortManager->IsPortCProvisioned());

    // Well-known port
    m_pPortManager->SetPortC(1000, 40000);

    EXPECT_FALSE(m_pPortManager->IsPortCProvisioned());
}

}  // namespace android
