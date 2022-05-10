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

class AosRetryRepositoryTest : public ::testing::Test {
public:
    AosRetryRepository* pAosRetryRepository;
    MockIAosNConfiguration objMockAosConfig;

protected:
    virtual void SetUp() override {
        pAosRetryRepository = new AosRetryRepository(SLOT_ID);
        ASSERT_TRUE(pAosRetryRepository != nullptr);

        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&objMockAosConfig), SLOT_ID);
    }

    virtual void TearDown() override {
        if (pAosRetryRepository) {
            delete pAosRetryRepository;
        }
    }

    void SetRetryCount(IN IMS_SINT32 nRetryCount, IN IMS_SINT32 nEmergencyRetryCount) {
        pAosRetryRepository->m_nRetryCount = nRetryCount;
        pAosRetryRepository->m_nEmergencyRetryCount = nEmergencyRetryCount;
    }
};

TEST_F(AosRetryRepositoryTest, IncreaseCount) {
    EXPECT_CALL(objMockAosConfig, GetSpecificRegistrationErrorMaxCount())
        .WillRepeatedly(Return(5));

    SetRetryCount(0, 0);
    EXPECT_TRUE(pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_NORMAL));
    SetRetryCount(0, 0);
    EXPECT_TRUE(pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_EMERGENCY));

    SetRetryCount(5, 3);
    EXPECT_FALSE(pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_NORMAL));
    EXPECT_TRUE(pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_EMERGENCY));

    EXPECT_TRUE(pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_NORMAL));
    EXPECT_FALSE(pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_EMERGENCY));
    EXPECT_TRUE(pAosRetryRepository->IncreaseRetryCount(AosRetryRepository::TYPE_EMERGENCY));
}