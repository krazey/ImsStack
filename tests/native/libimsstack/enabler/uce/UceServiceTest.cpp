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
#include "UceService.h"
#include "IUUceService.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

using ::testing::AnyNumber;
using ::testing::Return;

IMS_SINT32 SERVICE_SIM_SLOT = 20;

class TestUceService : public UceService
{
public:
    enum
    {
        SUBSCRIBE = 0,
        PUBLISH,
        OPTIONS,
    };

public:
    TestUceService() :
            UceService(AString("UceApp"), SERVICE_SIM_SLOT)
    {
    }
    virtual ~TestUceService() {}

    void ResetManager()
    {
        m_pUceSubscribeManager = IMS_NULL;
        m_pUcePublishManager = IMS_NULL;
        m_pUceOptionsManager = IMS_NULL;
    }
    void EnableManagers() { EnableManager(); }
    void DisableManagers() { DisableManager(); }
    IMS_BOOL SendMessage(IMSMSG objUIMsg) { return OnMessage(objUIMsg); }
    IMS_BOOL IsNull(IMS_UINT32 manager)
    {
        switch (manager)
        {
            case SUBSCRIBE:
            {
                if (m_pUceSubscribeManager == IMS_NULL)
                {
                    return IMS_TRUE;
                }
            }
            break;
            case PUBLISH:
            {
                if (m_pUcePublishManager == IMS_NULL)
                {
                    return IMS_TRUE;
                }
            }
            break;
            case OPTIONS:
            {
                if (m_pUceOptionsManager == IMS_NULL)
                {
                    return IMS_TRUE;
                }
            }
            break;
            default:
                break;
        }
        return IMS_FALSE;
    }
};

class UceServiceTest : public ::testing::Test
{
public:
    TestUceService* pUceService;

protected:
    virtual void SetUp() override
    {
        pUceService = new TestUceService();
        ASSERT_TRUE(pUceService != nullptr);
    }

    virtual void TearDown() override
    {
        if (pUceService)
        {
            delete pUceService;
        }
    }
};

TEST_F(UceServiceTest, EnableManager)
{
    IMS_TRACE_D("EnableManager", 0, 0, 0);
    pUceService->ResetManager();
    EXPECT_EQ(pUceService->IsNull(TestUceService::SUBSCRIBE), IMS_TRUE);
    EXPECT_EQ(pUceService->IsNull(TestUceService::PUBLISH), IMS_TRUE);
    EXPECT_EQ(pUceService->IsNull(TestUceService::OPTIONS), IMS_TRUE);
    pUceService->EnableManagers();
    EXPECT_EQ(pUceService->IsNull(TestUceService::SUBSCRIBE), IMS_FALSE);
    EXPECT_EQ(pUceService->IsNull(TestUceService::PUBLISH), IMS_FALSE);
    EXPECT_EQ(pUceService->IsNull(TestUceService::OPTIONS), IMS_FALSE);
}

TEST_F(UceServiceTest, DisableManager)
{
    IMS_TRACE_D("DisableManager", 0, 0, 0);
    EXPECT_EQ(pUceService->IsNull(TestUceService::SUBSCRIBE), IMS_FALSE);
    EXPECT_EQ(pUceService->IsNull(TestUceService::PUBLISH), IMS_FALSE);
    EXPECT_EQ(pUceService->IsNull(TestUceService::OPTIONS), IMS_FALSE);
    pUceService->DisableManagers();
    EXPECT_EQ(pUceService->IsNull(TestUceService::SUBSCRIBE), IMS_TRUE);
    EXPECT_EQ(pUceService->IsNull(TestUceService::PUBLISH), IMS_TRUE);
    EXPECT_EQ(pUceService->IsNull(TestUceService::OPTIONS), IMS_TRUE);
}

TEST_F(UceServiceTest, onMessage)
{
    IMS_TRACE_D("onMessage", 0, 0, 0);

    IMSMSG objSendPubMsg(IUUceService::UCE_SEND_PUBLISH_CMD, 0, IMS_NULL);
    EXPECT_EQ(pUceService->SendMessage(objSendPubMsg), IMS_FALSE);

    IUcePubCmdPrm* pPubParam = new IUcePubCmdPrm();
    IMSMSG objSendPubWithParamMsg(
            IUUceService::UCE_SEND_PUBLISH_CMD, 0, reinterpret_cast<IMS_UINTP>(pPubParam));
    EXPECT_EQ(pUceService->SendMessage(objSendPubWithParamMsg), IMS_TRUE);

    IMSMSG objSingleSubMsg(IUUceService::UCE_SEND_SINGLE_SUBSCRIBE_CMD, 0, IMS_NULL);
    EXPECT_EQ(pUceService->SendMessage(objSingleSubMsg), IMS_FALSE);

    IUceSingleSubCmdPrm* pSingleSubParam = new IUceSingleSubCmdPrm();
    IMSMSG objSingleSubWithParamMsg(IUUceService::UCE_SEND_SINGLE_SUBSCRIBE_CMD, 0,
            reinterpret_cast<IMS_UINTP>(pSingleSubParam));
    EXPECT_EQ(pUceService->SendMessage(objSingleSubWithParamMsg), IMS_TRUE);

    IMSMSG objListSubMsg(IUUceService::UCE_SEND_LIST_SUBSCRIBE_CMD, 0, IMS_NULL);
    EXPECT_EQ(pUceService->SendMessage(objListSubMsg), IMS_FALSE);

    IUceListSubCmdPrm* pListSubParam = new IUceListSubCmdPrm();
    IMSMSG objListSubWithParamMsg(IUUceService::UCE_SEND_LIST_SUBSCRIBE_CMD, 0,
            reinterpret_cast<IMS_UINTP>(pListSubParam));
    EXPECT_EQ(pUceService->SendMessage(objListSubWithParamMsg), IMS_TRUE);

    IMSMSG objOptionsMsg(IUUceService::UCE_SEND_OPTIONS_CMD, 0, IMS_NULL);
    EXPECT_EQ(pUceService->SendMessage(objOptionsMsg), IMS_FALSE);

    IUceOptionsCmdPrm* pOptionsParam = new IUceOptionsCmdPrm();
    IMSMSG objOptionsWithParamMsg(
            IUUceService::UCE_SEND_OPTIONS_CMD, 0, reinterpret_cast<IMS_UINTP>(pOptionsParam));
    EXPECT_EQ(pUceService->SendMessage(objOptionsWithParamMsg), IMS_TRUE);

    IMSMSG objOptionsResMsg(IUUceService::UCE_SEND_OPTIONS_RESP_CMD, 0, IMS_NULL);
    EXPECT_EQ(pUceService->SendMessage(objOptionsResMsg), IMS_FALSE);

    IUceOptionsRespCmdPrm* pOptionsResParam = new IUceOptionsRespCmdPrm();
    IMSMSG objOptionsResWithParamMsg(IUUceService::UCE_SEND_OPTIONS_RESP_CMD, 0,
            reinterpret_cast<IMS_UINTP>(pOptionsResParam));
    EXPECT_EQ(pUceService->SendMessage(objOptionsResWithParamMsg), IMS_TRUE);
}