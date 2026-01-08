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
#include <gmock/gmock.h>

#include "IAosService.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosLocationStarter.h"
#include "interface/IAosRegStateManager.h"
#include "interface/IAosRetryRepository.h"
#include "interface/IAosSubscriberManager.h"
#include "provider/AosDnsQuery.h"
#include "provider/AosKeepAlive.h"
#include "provider/AosLog.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosLocationStarter.h"
#include "interface/MockIAosRegStateManager.h"
#include "interface/MockIAosRetryRepository.h"
#include "interface/MockIAosSubscriberManager.h"
#include "interface/MockIAosTracer.h"

#include "../../interface/aos/MockIAosService.h"

class AosProviderTest : public ::testing::Test
{
public:
    AosProvider* m_pProvider;

protected:
    void SetUp() override
    {
        m_pProvider = new AosProvider();
        ASSERT_TRUE(m_pProvider != nullptr);
    }

    void TearDown() override
    {
        if (m_pProvider)
        {
            delete m_pProvider;
        }
    }
};

TEST_F(AosProviderTest, SucceedsGetLog)
{
    // GIVEN
    // WHEN
    const AosLog* pAosLog = m_pProvider->GetLog();

    // THEN
    EXPECT_TRUE(pAosLog != nullptr);
}

TEST_F(AosProviderTest, SucceedsCreateDnsQuery)
{
    // GIVEN
    // WHEN
    const AosDnsQuery* pAosDnsQuery = m_pProvider->CreateDnsQuery();

    // THEN
    EXPECT_TRUE(pAosDnsQuery != nullptr);
}

TEST_F(AosProviderTest, SucceedsCreateKeepAlive)
{
    // GIVEN
    // WHEN
    const AosKeepAlive* pAosKeepAlive = m_pProvider->CreateKeepAlive();

    // THEN
    EXPECT_TRUE(pAosKeepAlive != nullptr);
}

TEST_F(AosProviderTest, SucceedsGetCallTracker)
{
    // GIVEN
    MockIAosCallTracker objMockIAosCallTracker;
    m_pProvider->SetCallTracker(&objMockIAosCallTracker);

    // WHEN
    const IAosCallTracker* piAosCallTracker = m_pProvider->GetCallTracker();

    // THEN
    EXPECT_TRUE(piAosCallTracker != nullptr);
}

TEST_F(AosProviderTest, SucceedsGetLocationStarter)
{
    // GIVEN
    MockIAosLocationStarter objMockIAosLocationStarter;
    m_pProvider->SetLocationStarter(&objMockIAosLocationStarter);

    // WHEN
    const IAosLocationStarter* piAosLocationStarter = m_pProvider->GetLocationStarter();

    // THEN
    EXPECT_TRUE(piAosLocationStarter != nullptr);
}

TEST_F(AosProviderTest, SucceedsGetRegStateManager)
{
    // GIVEN
    MockIAosRegStateManager objMockIAosRegStateManager;
    m_pProvider->SetRegStateManager(&objMockIAosRegStateManager);

    // WHEN
    const IAosRegStateManager* piAosRegStateManager = m_pProvider->GetRegStateManager();

    // THEN
    EXPECT_TRUE(piAosRegStateManager != nullptr);
}

TEST_F(AosProviderTest, SucceedsGetService)
{
    // GIVEN
    MockIAosService objMockIAosService;
    m_pProvider->SetService(&objMockIAosService);

    // WHEN
    const IAosService* piAosService = m_pProvider->GetService();

    // THEN
    EXPECT_TRUE(piAosService != nullptr);
}

TEST_F(AosProviderTest, SucceedsGetSubscriberManager)
{
    // GIVEN
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    m_pProvider->SetSubscriberManager(&objMockIAosSubscriberManager);

    // WHEN
    const IAosSubscriberManager* piAosSubscriberManager = m_pProvider->GetSubscriberManager();

    // THEN
    EXPECT_TRUE(piAosSubscriberManager != nullptr);
}

TEST_F(AosProviderTest, SucceedsGetTracer)
{
    // GIVEN
    MockIAosTracer objMockIAosTracer;
    m_pProvider->SetTracer(&objMockIAosTracer);

    // WHEN
    const IAosTracer* piAosTracer = m_pProvider->GetTracer();

    // THEN
    EXPECT_TRUE(piAosTracer != nullptr);
}

TEST_F(AosProviderTest, SucceedsGetRetryRepository)
{
    // GIVEN
    MockIAosRetryRepository objMockIAosRetryRepository;
    m_pProvider->SetRetryRepository(&objMockIAosRetryRepository);

    // WHEN
    const IAosRetryRepository* piAosRetryRepository = m_pProvider->GetRetryRepository();

    // THEN
    EXPECT_TRUE(piAosRetryRepository != nullptr);
}
