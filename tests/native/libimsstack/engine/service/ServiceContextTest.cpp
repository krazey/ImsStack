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

#include "ServiceContext.h"

#include "MockIConfiguration.h"
#include "MockIServiceContext.h"
#include "MockIServiceManager.h"
#include "TestMutexService.h"

using ::testing::Return;

namespace android
{

class ServiceContextTest : public ::testing::Test
{
public:
    inline ServiceContextTest() :
            m_pServiceContext(IMS_NULL)
    {
    }

protected:
    TestMutexService m_objMutexService;

    ServiceContext* m_pServiceContext;

protected:
    void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_MUTEX, &m_objMutexService);

        m_pServiceContext = ServiceContext::GetInstance();
    }

    void TearDown() override
    {
        ServiceContext::DestroyInstance();

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_MUTEX, IMS_NULL);
    }
};

TEST_F(ServiceContextTest, Accessors)
{
    EXPECT_NE(m_pServiceContext->GetConfiguration(), nullptr);
    EXPECT_NE(m_pServiceContext->GetServiceManager(), nullptr);
}

TEST_F(ServiceContextTest, AccessorsWithExternalServiceContext)
{
    MockIConfiguration objConfiguration;
    MockIServiceManager objServiceManager;
    MockIServiceContext objServiceContext;
    EXPECT_CALL(objServiceContext, GetConfiguration()).Times(1).WillOnce(Return(&objConfiguration));
    EXPECT_CALL(objServiceContext, GetServiceManager())
            .Times(1)
            .WillOnce(Return(&objServiceManager));
    m_pServiceContext->SetServiceContext(&objServiceContext);

    EXPECT_EQ(m_pServiceContext->GetConfiguration(), &objConfiguration);
    EXPECT_EQ(m_pServiceContext->GetServiceManager(), &objServiceManager);
}

}  // namespace android
