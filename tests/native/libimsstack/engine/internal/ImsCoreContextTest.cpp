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

#include "CallControlHelper.h"
#include "ImsCoreContext.h"
#include "util/CallerPreferenceManager.h"

#include "MockIConfiguration.h"
#include "MockIImsCoreContext.h"
#include "MockIRegInfoManager.h"
#include "MockIRegistrationManager.h"
#include "MockIServiceManager.h"
#include "TestImsCoreProtocol.h"
#include "TestMutexService.h"
#include "util/MockISipConnectionNotifierManager.h"

using ::testing::Return;

namespace android
{

class ImsCoreContextTest : public ::testing::Test
{
public:
    inline ImsCoreContextTest() :
            m_pImsCoreContext(IMS_NULL)
    {
    }

protected:
    TestMutexService m_objMutexService;

    ImsCoreContext* m_pImsCoreContext;

protected:
    void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_MUTEX, &m_objMutexService);

        m_pImsCoreContext = ImsCoreContext::GetInstance();
    }

    void TearDown() override
    {
        ImsCoreContext::DestroyInstance();

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_MUTEX, IMS_NULL);
    }
};

TEST_F(ImsCoreContextTest, Accessors)
{
    EXPECT_NE(m_pImsCoreContext->GetConfiguration(), nullptr);
    EXPECT_NE(m_pImsCoreContext->GetServiceManager(), nullptr);

    EXPECT_NE(m_pImsCoreContext->GetRegistrationManager(), nullptr);
    EXPECT_NE(m_pImsCoreContext->GetRegInfoManager(), nullptr);

    EXPECT_NE(m_pImsCoreContext->GetImsCoreProtocol(), nullptr);
    EXPECT_NE(m_pImsCoreContext->GetCallControlHelper(), nullptr);
    EXPECT_NE(m_pImsCoreContext->GetCallerPreferenceManager(), nullptr);
    EXPECT_NE(m_pImsCoreContext->GetSipConnectionNotifierManager(), nullptr);
}

TEST_F(ImsCoreContextTest, AccessorsWithExternalImsCoreContextForService)
{
    MockIImsCoreContext objImsCoreContext;
    MockIConfiguration objConfiguration;
    MockIServiceManager objServiceManager;

    EXPECT_CALL(objImsCoreContext, GetConfiguration()).Times(1).WillOnce(Return(&objConfiguration));
    EXPECT_CALL(objImsCoreContext, GetServiceManager())
            .Times(1)
            .WillOnce(Return(&objServiceManager));
    m_pImsCoreContext->SetImsCoreContext(&objImsCoreContext);

    EXPECT_EQ(m_pImsCoreContext->GetConfiguration(), &objConfiguration);
    EXPECT_EQ(m_pImsCoreContext->GetServiceManager(), &objServiceManager);
}

TEST_F(ImsCoreContextTest, AccessorsWithExternalImsCoreContextForRegistration)
{
    MockIImsCoreContext objImsCoreContext;
    MockIRegInfoManager objRegInfoManager;
    MockIRegistrationManager objRegistrationManager;

    EXPECT_CALL(objImsCoreContext, GetRegistrationManager())
            .Times(1)
            .WillOnce(Return(&objRegistrationManager));
    EXPECT_CALL(objImsCoreContext, GetRegInfoManager())
            .Times(1)
            .WillOnce(Return(&objRegInfoManager));
    m_pImsCoreContext->SetImsCoreContext(&objImsCoreContext);

    EXPECT_EQ(m_pImsCoreContext->GetRegistrationManager(), &objRegistrationManager);
    EXPECT_EQ(m_pImsCoreContext->GetRegInfoManager(), &objRegInfoManager);
}

TEST_F(ImsCoreContextTest, AccessorsWithExternalImsCoreContextForCore)
{
    MockIImsCoreContext objImsCoreContext;
    TestImsCoreProtocol objImsCoreProtocol;
    CallControlHelper objCallControlHelper;
    CallerPreferenceManager objCallerPreferenceManager;
    MockISipConnectionNotifierManager objScnManager;

    EXPECT_CALL(objImsCoreContext, GetImsCoreProtocol())
            .Times(1)
            .WillOnce(Return(&objImsCoreProtocol));
    EXPECT_CALL(objImsCoreContext, GetCallControlHelper())
            .Times(1)
            .WillOnce(Return(&objCallControlHelper));
    EXPECT_CALL(objImsCoreContext, GetCallerPreferenceManager())
            .Times(1)
            .WillOnce(Return(&objCallerPreferenceManager));
    EXPECT_CALL(objImsCoreContext, GetSipConnectionNotifierManager())
            .Times(1)
            .WillOnce(Return(&objScnManager));
    m_pImsCoreContext->SetImsCoreContext(&objImsCoreContext);

    EXPECT_EQ(m_pImsCoreContext->GetImsCoreProtocol(), &objImsCoreProtocol);
    EXPECT_EQ(m_pImsCoreContext->GetCallControlHelper(), &objCallControlHelper);
    EXPECT_EQ(m_pImsCoreContext->GetCallerPreferenceManager(), &objCallerPreferenceManager);
    EXPECT_EQ(m_pImsCoreContext->GetSipConnectionNotifierManager(), &objScnManager);
}

}  // namespace android
