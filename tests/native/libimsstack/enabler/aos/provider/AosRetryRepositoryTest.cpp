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

#include "interface/IAosAppContext.h"
#include "provider/AosProvider.h"
#include "provider/AosRetryRepository.h"

#include "interface/MockIAosNConfiguration.h"

using ::testing::Return;

const IMS_SINT32 SLOT_ID = 0;

class TestAosRetryRepository : public AosRetryRepository
{
public:
    inline explicit TestAosRetryRepository(IN IMS_SINT32 nSlotId) :
            AosRetryRepository(nSlotId)
    {
    }

    void SetRetryCount(IN IMS_SINT32 nRetryCount) { m_nRetryCount = nRetryCount; }

    void SetEmcRetryCount(IN IMS_SINT32 nEmcRetryCount) { m_nEmergencyRetryCount = nEmcRetryCount; }
};

class AosRetryRepositoryTest : public ::testing::Test
{
public:
    AosRetryRepositoryTest()
    {
        m_piOriginAosNConfig = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(&m_objMockIAosConfig, SLOT_ID);
    }

    virtual ~AosRetryRepositoryTest()
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piOriginAosNConfig, SLOT_ID);
    }

    TestAosRetryRepository* m_pAosRetryRepository;

    IAosNConfiguration* m_piOriginAosNConfig;
    MockIAosNConfiguration m_objMockIAosConfig;

protected:
    void SetUp() override
    {
        m_pAosRetryRepository = new TestAosRetryRepository(SLOT_ID);
        ASSERT_TRUE(m_pAosRetryRepository != nullptr);

        ON_CALL(m_objMockIAosConfig, GetExtraRegErrMaxCount()).WillByDefault(Return(5));
    }

    void TearDown() override
    {
        if (m_pAosRetryRepository)
        {
            delete m_pAosRetryRepository;
        }
    }
};

TEST_F(AosRetryRepositoryTest, IncreaseCountWhenIncreaseIsRequested)
{
    m_pAosRetryRepository->SetRetryCount(0);
    EXPECT_TRUE(m_pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_NORMAL));
}

TEST_F(AosRetryRepositoryTest, IncreaseCountWhenIncreaseIsRequestedWithinMax)
{
    m_pAosRetryRepository->SetRetryCount(2);
    EXPECT_TRUE(m_pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_NORMAL));
}

TEST_F(AosRetryRepositoryTest, ResetCountWhenCurrentCountIsMax)
{
    m_pAosRetryRepository->SetRetryCount(5);
    EXPECT_FALSE(m_pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_NORMAL));
}

TEST_F(AosRetryRepositoryTest, IncreaseCountWhenIncreaseIsRequestedForEmc)
{
    m_pAosRetryRepository->SetEmcRetryCount(0);
    EXPECT_TRUE(m_pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_EMERGENCY));
}

TEST_F(AosRetryRepositoryTest, IncreaseAndResetCountWhenIncreaseIsRequestedForEmcUntilMax)
{
    m_pAosRetryRepository->SetEmcRetryCount(3);
    EXPECT_TRUE(m_pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_EMERGENCY));
    EXPECT_FALSE(m_pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_EMERGENCY));
}