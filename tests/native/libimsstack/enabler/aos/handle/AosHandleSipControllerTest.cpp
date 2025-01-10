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

#include "handle/AosHandleSipController.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "provider/AosProvider.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosNConfiguration.h"

using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

class AosHandleSipControllerTest : public ::testing::Test
{
public:
    AosHandleSipController* m_pAosHandleSipController;

    MockIAosAppContext m_objMockIAosAppContext;

    IAosNConfiguration* m_piAosNConfiguration;
    MockIAosNConfiguration m_objMockIAosNConfiguration;

protected:
    void SetUp() override
    {
        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        const AString strAppId = AString("ims.app.sipcontroller.test");
        const AString strServiceId = AString("ims.service.sipcontroller.test");
        const IMS_UINT32 nServiceType = -1;

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockIAosNConfiguration));

        m_pAosHandleSipController =
                new AosHandleSipController(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        strAppId, strServiceId, nServiceType);

        ASSERT_TRUE(m_pAosHandleSipController != nullptr);
    }

    void TearDown() override
    {
        if (m_pAosHandleSipController != nullptr)
        {
            delete m_pAosHandleSipController;
            m_pAosHandleSipController = nullptr;
        }

        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration);
    }

    void InitializeServiceBlock() { m_pAosHandleSipController->InitializeServiceBlock(); }

    IMS_BOOL IsBlocked() { return m_pAosHandleSipController->AosHandle::IsBlocked(); }
};

TEST_F(AosHandleSipControllerTest, InitializeServiceBlock_Test1)
{
    // Test1: single reg not required
    // Expectation: blocked

    EXPECT_CALL(m_objMockIAosNConfiguration, IsImsSingleRegistrationRequired())
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    InitializeServiceBlock();

    EXPECT_TRUE(IsBlocked());
}

TEST_F(AosHandleSipControllerTest, InitializeServiceBlock_Test2)
{
    // Test2: single reg required
    // Expectation: not blocked

    EXPECT_CALL(m_objMockIAosNConfiguration, IsImsSingleRegistrationRequired())
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    InitializeServiceBlock();

    EXPECT_FALSE(IsBlocked());
}