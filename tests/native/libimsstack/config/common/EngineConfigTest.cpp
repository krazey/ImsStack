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

#include "ITraceOption.h"
#include "MockIImsPrivateProperty.h"
#include "PlatformContext.h"
#include "ServiceTrace.h"
#include "TestUtilService.h"

#include "private/EngineConfig.h"

using ::testing::Eq;
using ::testing::Return;

namespace android
{

class EngineConfigTest : public ::testing::Test
{
public:
    EngineConfigTest() :
            m_pUtilService(IMS_NULL),
            m_strTestLogOptions(ImsPrivateProperties::Persistent::KEY_TEST_LOG_OPTIONS)
    {
    }
    virtual ~EngineConfigTest() {}

protected:
    virtual void SetUp() override
    {
        m_pUtilService = new TestUtilService();

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, m_pUtilService);
    }
    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, IMS_NULL);

        if (m_pUtilService != IMS_NULL)
        {
            delete m_pUtilService;
            m_pUtilService = IMS_NULL;
        }
    }

protected:
    TestUtilService* m_pUtilService;
    AString m_strTestLogOptions;
};

TEST_F(EngineConfigTest, Constructor)
{
    EngineConfig objEngineConfig(IMS_SLOT_0);

    EXPECT_EQ(objEngineConfig.GetTraceOption(), ITraceOption::OPT_DEFAULT);
    EXPECT_EQ(objEngineConfig.GetTraceModule(), IMS_TRACE_MODULE_ALL);
}

TEST_F(EngineConfigTest, RefreshWithNoCategories)
{
    IMS_UINT32 nTraceOptions = ITraceOption::OPT_MEDIUM_SERIAL;
    ON_CALL(m_pUtilService->GetMockPrivateProperty(),
            GetPersistent(Eq(m_strTestLogOptions), Eq(IMS_SLOT_0)))
            .WillByDefault(Return(AString("0x00010000")));

    EngineConfig objEngineConfig(IMS_SLOT_0);
    objEngineConfig.Refresh();

    EXPECT_EQ(objEngineConfig.GetTraceOption(), nTraceOptions);
    EXPECT_EQ(TraceService::GetTraceService()->GetOption(), nTraceOptions);
}

TEST_F(EngineConfigTest, RefreshWithAllCategories)
{
    IMS_UINT32 nTraceOptions = ITraceOption::OPT_MEDIUM_SERIAL | ITraceOption::OPT_CAT_ALL;
    ON_CALL(m_pUtilService->GetMockPrivateProperty(),
            GetPersistent(Eq(m_strTestLogOptions), Eq(IMS_SLOT_0)))
            .WillByDefault(Return(AString("0x0001000F")));

    EngineConfig objEngineConfig(IMS_SLOT_0);
    objEngineConfig.Refresh();

    EXPECT_EQ(objEngineConfig.GetTraceOption(), nTraceOptions);
    EXPECT_EQ(TraceService::GetTraceService()->GetOption(), nTraceOptions);
}

TEST_F(EngineConfigTest, RefreshWithDebug)
{
    IMS_UINT32 nTraceOptions = ITraceOption::OPT_MEDIUM_SERIAL | ITraceOption::OPT_CAT_D;
    ON_CALL(m_pUtilService->GetMockPrivateProperty(),
            GetPersistent(Eq(m_strTestLogOptions), Eq(IMS_SLOT_0)))
            .WillByDefault(Return(AString("0x00010001")));

    EngineConfig objEngineConfig(IMS_SLOT_0);
    objEngineConfig.Refresh();

    EXPECT_EQ(objEngineConfig.GetTraceOption(), nTraceOptions);
    EXPECT_EQ(TraceService::GetTraceService()->GetOption(), nTraceOptions);
}

TEST_F(EngineConfigTest, RefreshWithInfo)
{
    IMS_UINT32 nTraceOptions = ITraceOption::OPT_MEDIUM_SERIAL | ITraceOption::OPT_CAT_I;
    ON_CALL(m_pUtilService->GetMockPrivateProperty(),
            GetPersistent(Eq(m_strTestLogOptions), Eq(IMS_SLOT_0)))
            .WillByDefault(Return(AString("0x00010004")));

    EngineConfig objEngineConfig(IMS_SLOT_0);
    objEngineConfig.Refresh();

    EXPECT_EQ(objEngineConfig.GetTraceOption(), nTraceOptions);
    EXPECT_EQ(TraceService::GetTraceService()->GetOption(), nTraceOptions);
}

TEST_F(EngineConfigTest, RefreshWithError)
{
    IMS_UINT32 nTraceOptions = ITraceOption::OPT_MEDIUM_SERIAL | ITraceOption::OPT_CAT_E;
    ON_CALL(m_pUtilService->GetMockPrivateProperty(),
            GetPersistent(Eq(m_strTestLogOptions), Eq(IMS_SLOT_0)))
            .WillByDefault(Return(AString("0x00010002")));

    EngineConfig objEngineConfig(IMS_SLOT_0);
    objEngineConfig.Refresh();

    EXPECT_EQ(objEngineConfig.GetTraceOption(), nTraceOptions);
    EXPECT_EQ(TraceService::GetTraceService()->GetOption(), nTraceOptions);
}

TEST_F(EngineConfigTest, RefreshWithText)
{
    IMS_UINT32 nTraceOptions = ITraceOption::OPT_MEDIUM_SERIAL | ITraceOption::OPT_CAT_TEXT;
    ON_CALL(m_pUtilService->GetMockPrivateProperty(),
            GetPersistent(Eq(m_strTestLogOptions), Eq(IMS_SLOT_0)))
            .WillByDefault(Return(AString("0x00010008")));

    EngineConfig objEngineConfig(IMS_SLOT_0);
    objEngineConfig.Refresh();

    EXPECT_EQ(objEngineConfig.GetTraceOption(), nTraceOptions);
    EXPECT_EQ(TraceService::GetTraceService()->GetOption(), nTraceOptions);
}

}  // namespace android
