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
#include "PlatformService.h"

#include "Engine.h"
#include "ServiceContext.h"

#include "MockIServiceContext.h"
#include "TestThreadService.h"

using ::testing::Return;

namespace android
{

class ServiceContextTest : public ::testing::Test
{
public:
    inline ServiceContextTest() :
            m_pThreadService(IMS_NULL),
            m_pOldThreadService(IMS_NULL),
            m_pServiceContext(IMS_NULL)
    {
    }

protected:
    TestThreadService* m_pThreadService;
    PlatformService* m_pOldThreadService;

    ServiceContext* m_pServiceContext;

protected:
    void SetUp() override
    {
        m_pThreadService = new TestThreadService();
        m_pOldThreadService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pThreadService);

        m_pServiceContext = ServiceContext::GetInstance();
    }

    void TearDown() override
    {
        ServiceContext::DestroyInstance();

        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pOldThreadService);
        delete m_pThreadService;
    }
};

TEST_F(ServiceContextTest, Accessors)
{
    EXPECT_NE(m_pServiceContext->GetConfiguration(), nullptr);
    EXPECT_NE(m_pServiceContext->GetServiceManager(), nullptr);
}

TEST_F(ServiceContextTest, AccessorsWithExternalServiceContext)
{
    MockIServiceContext objServiceContext;
    m_pServiceContext->SetServiceContext(&objServiceContext);
    EXPECT_CALL(objServiceContext, GetConfiguration()).Times(1).WillOnce(Return(nullptr));
    EXPECT_CALL(objServiceContext, GetServiceManager()).Times(1).WillOnce(Return(nullptr));

    EXPECT_EQ(m_pServiceContext->GetConfiguration(), nullptr);
    EXPECT_EQ(m_pServiceContext->GetServiceManager(), nullptr);
}

}  // namespace android
