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
#include "RcsMessageService.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

#include "IURcsMessageService.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

IMS_SINT32 SLOT_ID = 0;

class RcsMessageServiceTest : public ::testing::Test
{
public:
    RcsMessageService* pRcsService;
    AString sThread = "JniSipControllerServiceThread";

protected:
    virtual void SetUp() override
    {
        IMS_TRACE_D("SetUp()", 0, 0, 0);
        pRcsService = new RcsMessageService("RcsMessageApp", SLOT_ID);
        ASSERT_TRUE(pRcsService != nullptr);

        IUSncOpenCmdParam* pParam = new IUSncOpenCmdParam();
        IMS_StrCpy(pParam->szThread, IMS_SOLUTION_MSG_SOURCE_LEN, sThread.GetStr());

        IMSMSG objOpenCmdMsg(IUSncService::OPENMESSAGE_CMD, 0, reinterpret_cast<IMS_UINTP>(pParam));
        ASSERT_TRUE(pRcsService->HandleOpenMSG(objOpenCmdMsg) == IMS_TRUE);
    }

    virtual void TearDown() override
    {
        IMS_TRACE_D("TearDown()", 0, 0, 0);
        if (pRcsService)
        {
            delete pRcsService;
        }
    }
};

TEST_F(RcsMessageServiceTest, HandleSessionMSG)
{
    IMS_TRACE_D("HandleSessionMSG", 0, 0, 0);

    IUSncSendMessageParam* pParam = new IUSncSendMessageParam();
    IMS_StrCpy(pParam->szThread, IMS_SOLUTION_MSG_SOURCE_LEN, sThread.GetStr());

    IMSMSG objSessionCmdMsg(IUSncService::SENDMESSAGE_CMD, 0, reinterpret_cast<IMS_UINTP>(pParam));
    // to avoid test
    // EXPECT_EQ(pRcsService->HandleSessionMSG(objSessionCmdMsg), IMS_TRUE);
}

TEST_F(RcsMessageServiceTest, HandleNotifyReceiveErrorMSG)
{
    IMS_TRACE_D("HandleNotifyReceiveErrorMSG", 0, 0, 0);

    IUSncNotifyErrorCmdParam* pParam = new IUSncNotifyErrorCmdParam();
    IMS_StrCpy(pParam->szThread, IMS_SOLUTION_MSG_SOURCE_LEN, sThread.GetStr());

    IMSMSG objNotifyCmdMsg(
            IUSncService::NOTIFYMESSAGERECEIVEERROR_CMD, 0, reinterpret_cast<IMS_UINTP>(pParam));
    // to avoid test
    // EXPECT_EQ(pRcsService->HandleSessionMSG(objNotifyCmdMsg), IMS_TRUE);
}

TEST_F(RcsMessageServiceTest, HandleCloseSessionMSG)
{
    IMS_TRACE_D("HandleCloseSessionMSG", 0, 0, 0);

    IUSncCloseSessionCmdParam* pParam = new IUSncCloseSessionCmdParam();
    IMS_StrCpy(pParam->szThread, IMS_SOLUTION_MSG_SOURCE_LEN, sThread.GetStr());

    IMSMSG objCloseCmdMsg(IUSncService::CLOSESESSION_CMD, 0, reinterpret_cast<IMS_UINTP>(pParam));
    // to avoid test
    // EXPECT_EQ(pRcsService->HandleCloseSessionMSG(objCloseCmdMsg), IMS_TRUE);
}