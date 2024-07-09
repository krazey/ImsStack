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

#include "PlatformContext.h"

#include "RegistrationContext.h"

#include "MockIRegInfoManager.h"
#include "MockIRegistrationContext.h"
#include "MockIRegistrationManager.h"
#include "TestMutexService.h"
#include "util/MockISipConnectionNotifierManager.h"

using ::testing::Return;

namespace android
{

class RegistrationContextTest : public ::testing::Test
{
public:
    inline RegistrationContextTest() :
            m_pRegistrationContext(IMS_NULL)
    {
    }

protected:
    TestMutexService m_objMutexService;
    RegistrationContext* m_pRegistrationContext;

protected:
    void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_MUTEX, &m_objMutexService);

        m_pRegistrationContext = RegistrationContext::GetInstance();
    }

    void TearDown() override
    {
        RegistrationContext::DestroyInstance();

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_MUTEX, IMS_NULL);
    }
};

TEST_F(RegistrationContextTest, Accessors)
{
    EXPECT_NE(m_pRegistrationContext->GetRegistrationManager(), nullptr);
    EXPECT_NE(m_pRegistrationContext->GetRegInfoManager(), nullptr);
    EXPECT_NE(m_pRegistrationContext->GetSipConnectionNotifierManager(), nullptr);
}

TEST_F(RegistrationContextTest, AccessorsWithExternalRegistrationContext)
{
    MockIRegInfoManager objRegInfoManager;
    MockIRegistrationManager objRegistrationManager;
    MockIRegistrationContext objRegistrationContext;
    MockISipConnectionNotifierManager objScnManager;
    EXPECT_CALL(objRegistrationContext, GetRegistrationManager())
            .Times(1)
            .WillOnce(Return(&objRegistrationManager));
    EXPECT_CALL(objRegistrationContext, GetRegInfoManager())
            .Times(1)
            .WillOnce(Return(&objRegInfoManager));
    EXPECT_CALL(objRegistrationContext, GetSipConnectionNotifierManager())
            .Times(1)
            .WillOnce(Return(&objScnManager));
    m_pRegistrationContext->SetRegistrationContext(&objRegistrationContext);

    EXPECT_EQ(m_pRegistrationContext->GetRegistrationManager(), &objRegistrationManager);
    EXPECT_EQ(m_pRegistrationContext->GetRegInfoManager(), &objRegInfoManager);
    EXPECT_EQ(m_pRegistrationContext->GetSipConnectionNotifierManager(), &objScnManager);
}

}  // namespace android
