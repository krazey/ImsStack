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

#include "../../interface/aos/MockIAosService.h"

using ::testing::TypedEq;

class AosProviderTest : public ::testing::Test
{
public:
    AosProvider* m_pAosProvider;

protected:
    virtual void SetUp() override
    {
        m_pAosProvider = new AosProvider();
        ASSERT_TRUE(m_pAosProvider != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pAosProvider)
        {
            delete m_pAosProvider;
        }
    }
};

TEST_F(AosProviderTest, GetLog)
{
    m_pAosProvider->~AosProvider();
    AosLog* pAosLog = m_pAosProvider->GetLog();

    EXPECT_TRUE(pAosLog != nullptr);
    EXPECT_THAT(pAosLog, TypedEq<AosLog*>(pAosLog));
}

TEST_F(AosProviderTest, CreateDnsQuery)
{
    AosDnsQuery* pAosDnsQuery = m_pAosProvider->CreateDnsQuery();

    EXPECT_TRUE(pAosDnsQuery != nullptr);
    EXPECT_THAT(pAosDnsQuery, TypedEq<AosDnsQuery*>(pAosDnsQuery));
}

TEST_F(AosProviderTest, CreateKeepAlive)
{
    AosKeepAlive* pAosKeepAlive = m_pAosProvider->CreateKeepAlive(0);

    EXPECT_TRUE(pAosKeepAlive != nullptr);
    EXPECT_THAT(pAosKeepAlive, TypedEq<AosKeepAlive*>(pAosKeepAlive));
}

TEST_F(AosProviderTest, SetCallTracker)
{
    MockIAosCallTracker objMockIAosCallTracker;
    m_pAosProvider->SetCallTracker(static_cast<IAosCallTracker*>(&objMockIAosCallTracker));

    IAosCallTracker* piAosCallTracker = m_pAosProvider->GetCallTracker();

    EXPECT_TRUE(piAosCallTracker != nullptr);
    EXPECT_THAT(piAosCallTracker, TypedEq<IAosCallTracker*>(piAosCallTracker));
}

TEST_F(AosProviderTest, SetLocationStarter)
{
    MockIAosLocationStarter objMockIAosLocationStarter;
    m_pAosProvider->SetLocationStarter(
            static_cast<IAosLocationStarter*>(&objMockIAosLocationStarter));

    IAosLocationStarter* piAosLocationStarter = m_pAosProvider->GetLocationStarter();

    EXPECT_TRUE(piAosLocationStarter != nullptr);
    EXPECT_THAT(piAosLocationStarter, TypedEq<IAosLocationStarter*>(piAosLocationStarter));
}

TEST_F(AosProviderTest, SetRegStateManager)
{
    MockIAosRegStateManager objMockIAosRegStateManager;
    m_pAosProvider->SetRegStateManager(
            static_cast<IAosRegStateManager*>(&objMockIAosRegStateManager));

    IAosRegStateManager* piAosRegStateManager = m_pAosProvider->GetRegStateManager();

    EXPECT_TRUE(piAosRegStateManager != nullptr);
    EXPECT_THAT(piAosRegStateManager, TypedEq<IAosRegStateManager*>(piAosRegStateManager));
}

TEST_F(AosProviderTest, SetService)
{
    MockIAosService objMockIAosService;
    m_pAosProvider->SetService(static_cast<IAosService*>(&objMockIAosService));

    IAosService* piAosService = m_pAosProvider->GetService();

    EXPECT_TRUE(piAosService != nullptr);
    EXPECT_THAT(piAosService, TypedEq<IAosService*>(piAosService));
}

TEST_F(AosProviderTest, SetSubscriberManager)
{
    MockIAosSubscriberManager objMockIAosSubscriberManager;
    m_pAosProvider->SetSubscriberManager(
            static_cast<IAosSubscriberManager*>(&objMockIAosSubscriberManager));

    IAosSubscriberManager* piAosSubscriberManager = m_pAosProvider->GetSubscriberManager();

    EXPECT_TRUE(piAosSubscriberManager != nullptr);
    EXPECT_THAT(piAosSubscriberManager, TypedEq<IAosSubscriberManager*>(piAosSubscriberManager));
}

TEST_F(AosProviderTest, SetRetryRepository)
{
    MockIAosRetryRepository objMockIAosRetryRepository;
    m_pAosProvider->SetRetryRepository(
            static_cast<IAosRetryRepository*>(&objMockIAosRetryRepository));

    IAosRetryRepository* piAosRetryRepository = m_pAosProvider->GetRetryRepository();

    EXPECT_TRUE(piAosRetryRepository != nullptr);
    EXPECT_THAT(piAosRetryRepository, TypedEq<IAosRetryRepository*>(piAosRetryRepository));
}