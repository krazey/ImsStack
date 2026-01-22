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
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ImsTypeDef.h"
#include "MockISystem.h"
#include "MockISystemProperty.h"
#include "OsUtil.h"
#include "PlatformContext.h"
#include "ServiceUtil.h"
#include "TestUtilService.h"

using ::testing::Return;

namespace android
{

class UtilServiceTest : public ::testing::Test
{
public:
    MockISystem m_objMockSystem;

    ISystem* m_piDefaultSystem;
    UtilService* m_pUtilService;

protected:
    virtual void SetUp() override
    {
        m_piDefaultSystem = PlatformContext::GetInstance()->SetSystem(&m_objMockSystem);

        m_pUtilService = new UtilService();
        ASSERT_TRUE(m_pUtilService != nullptr);

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, m_pUtilService);
        ASSERT_TRUE(UtilService::GetUtilService() == m_pUtilService);
    }

    virtual void TearDown() override
    {
        if (m_pUtilService != IMS_NULL)
        {
            m_pUtilService->Destroy();
            m_pUtilService = IMS_NULL;
        }
        PlatformContext::GetInstance()->SetSystem(m_piDefaultSystem);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, IMS_NULL);
    }
};

TEST_F(UtilServiceTest, GetUtilService)
{
    ASSERT_NE(m_pUtilService->GetPrivateProperty(), nullptr);
    ASSERT_NE(m_pUtilService->GetSystemUtil(), nullptr);
    ASSERT_NE(m_pUtilService->GetSystemProperty(), nullptr);
    ASSERT_NE(m_pUtilService->GetZLib(), nullptr);
}

TEST_F(UtilServiceTest, SetDebugOn)
{
    m_pUtilService->SetDebugOn(IMS_TRUE);
    EXPECT_EQ(OsUtil::GetInstance()->IsDebugMode(), IMS_TRUE);
}

TEST_F(UtilServiceTest, GetLogString)
{
    AString strInput;
    AString strOut;
    EXPECT_EQ(UtilService::GetLogString(strInput, strOut, 5), strInput);

    TestUtilService objMockUtilService;
    MockISystemProperty objMockSystemProperty;

    PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, &objMockUtilService);
    objMockUtilService.SetSystemProperty(&objMockSystemProperty);

    EXPECT_CALL(objMockSystemProperty, IsDebugMode()).Times(3).WillRepeatedly(Return(IMS_FALSE));

    AString strOutputValue("zzz");
    EXPECT_EQ(UtilService::GetLogString(strInput, strOut, 5), strOutputValue);
    strInput = "Log String to print";
    strOutputValue = "Log Stringxxx";
    EXPECT_EQ(UtilService::GetLogString(strInput, strOut, 10), strOutputValue);
    strOutputValue = "Log xxx";
    EXPECT_STREQ(
            UtilService::GetLogString(strInput, strOut, 10, 'S').GetStr(), strOutputValue.GetStr());
}

TEST_F(UtilServiceTest, LogSipMessage)
{
    const AString strMessage("SIP/2.0 200 OK");

    EXPECT_CALL(m_objMockSystem,
            LogSipMessage(strMessage.GetStr(), strMessage.GetLength(), IMS_SLOT_0, IMS_TRUE))
            .Times(1);

    m_pUtilService->LogSipMessage(
            strMessage.GetStr(), strMessage.GetLength(), IMS_SLOT_0, IMS_TRUE);
}

}  // namespace android
