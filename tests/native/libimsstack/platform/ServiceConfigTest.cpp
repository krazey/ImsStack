/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ImsCarrierConfig.h"
#include "MockIOsFactory.h"
#include "PlatformContext.h"
#include "ServiceConfig.h"

using ::testing::_;
using ::testing::Return;

namespace android
{

class TestImsCarrierConfig : public ImsCarrierConfig
{
public:
    inline TestImsCarrierConfig() :
            ImsCarrierConfig(IMS_SLOT_0),
            m_bServiceMsgDispatched(IMS_FALSE),
            m_bConfigLoaded(IMS_FALSE)
    {
    }

public:
    void DispatchServiceMessage(IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/) override
    {
        m_bServiceMsgDispatched = IMS_TRUE;
    }

    void LoadConfig() override { m_bConfigLoaded = IMS_TRUE; }

    IMS_BOOL IsServiceMessageDispatched() { return m_bServiceMsgDispatched; }

    IMS_BOOL IsConfigLoaded() { return m_bConfigLoaded; }

private:
    IMS_BOOL m_bServiceMsgDispatched;
    IMS_BOOL m_bConfigLoaded;
};

class ConfigServiceTest : public ::testing::Test
{
public:
    inline ConfigServiceTest() :
            m_pConfigService(IMS_NULL),
            m_pOldConfigService(IMS_NULL),
            m_pOldIOsFactory(IMS_NULL),
            m_pTestImsCarrierConfig(IMS_NULL)
    {
    }

    inline ~ConfigServiceTest() {}

protected:
    virtual void SetUp() override
    {
        m_pOldIOsFactory = PlatformContext::GetInstance()->SetOsFactory(&m_objMockIOsFactory);

        m_pTestImsCarrierConfig = new TestImsCarrierConfig();

        EXPECT_CALL(m_objMockIOsFactory, CreateCarrierConfig(_))
                .Times(1)
                .WillOnce(Return(m_pTestImsCarrierConfig));

        m_pConfigService = new ConfigService();

        m_pOldConfigService = PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pConfigService);
    }
    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetOsFactory(m_pOldIOsFactory);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, m_pOldConfigService);
        m_pConfigService->Destroy();
    }

protected:
    ConfigService* m_pConfigService;
    PlatformService* m_pOldConfigService;
    MockIOsFactory m_objMockIOsFactory;
    IOsFactory* m_pOldIOsFactory;
    TestImsCarrierConfig* m_pTestImsCarrierConfig;
};

TEST_F(ConfigServiceTest, GetCarrierConfig)
{
    ConfigService* pConfigService = ConfigService::GetConfigService();

    pConfigService->GetCarrierConfig(IMS_SLOT_0);
    pConfigService->GetCarrierConfig(IMS_SLOT_0);
}

TEST_F(ConfigServiceTest, LoadCarrierConfig)
{
    ConfigService* pConfigService = ConfigService::GetConfigService();

    EXPECT_FALSE(m_pTestImsCarrierConfig->IsConfigLoaded());

    pConfigService->LoadCarrierConfig(IMS_SLOT_0);

    EXPECT_TRUE(m_pTestImsCarrierConfig->IsConfigLoaded());
}

TEST_F(ConfigServiceTest, DispatchServiceMessage)
{
    ConfigService* pConfigService = ConfigService::GetConfigService();

    EXPECT_FALSE(m_pTestImsCarrierConfig->IsServiceMessageDispatched());

    ImsMessage objInvalidMessage(IMS_MSG_USIM, 0, 0);

    pConfigService->DispatchServiceMessage(objInvalidMessage);

    EXPECT_FALSE(m_pTestImsCarrierConfig->IsServiceMessageDispatched());

    ImsMessage objMessage(IMS_MSG_CONFIGURATION, 0, 0);

    pConfigService->DispatchServiceMessage(objMessage);

    EXPECT_TRUE(m_pTestImsCarrierConfig->IsServiceMessageDispatched());
}

}  // namespace android
