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
    IMS_BOOL IsNull(IMS_UINT32 manager) const
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
    EXPECT_TRUE(pUceService->IsNull(TestUceService::SUBSCRIBE));
    EXPECT_TRUE(pUceService->IsNull(TestUceService::PUBLISH));
    EXPECT_TRUE(pUceService->IsNull(TestUceService::OPTIONS));
    pUceService->EnableManagers();
    EXPECT_FALSE(pUceService->IsNull(TestUceService::SUBSCRIBE));
    EXPECT_FALSE(pUceService->IsNull(TestUceService::PUBLISH));
    EXPECT_FALSE(pUceService->IsNull(TestUceService::OPTIONS));
}

TEST_F(UceServiceTest, DisableManager)
{
    IMS_TRACE_D("DisableManager", 0, 0, 0);
    EXPECT_FALSE(pUceService->IsNull(TestUceService::SUBSCRIBE));
    EXPECT_FALSE(pUceService->IsNull(TestUceService::PUBLISH));
    EXPECT_FALSE(pUceService->IsNull(TestUceService::OPTIONS));
    pUceService->DisableManagers();
    EXPECT_TRUE(pUceService->IsNull(TestUceService::SUBSCRIBE));
    EXPECT_TRUE(pUceService->IsNull(TestUceService::PUBLISH));
    EXPECT_TRUE(pUceService->IsNull(TestUceService::OPTIONS));
}

TEST_F(UceServiceTest, SendPublishCmd)
{
    IMS_UINT32 key = 1;
    IMS_UINT32 extended = 1;
    IMS_UINT32 capability = 1;
    AString pidfXml = AString::ConstEmpty();
    AString eTag = AString::ConstEmpty();

    IMS_TRACE_D("SendPublishCmd", 0, 0, 0);
    pUceService->DisableManagers();
    EXPECT_FALSE(pUceService->SendPublishCmd(key, extended, capability, pidfXml, eTag));

    pUceService->EnableManagers();
    EXPECT_TRUE(pUceService->SendPublishCmd(key, extended, capability, pidfXml, eTag));
}

TEST_F(UceServiceTest, SendOptionsCmd)
{
    IMS_UINT32 key = 1;
    IMS_UINT32 myCaps = 1;
    AString remoteUri = AString::ConstEmpty();

    IMS_TRACE_D("SendOptionsCmd", 0, 0, 0);
    pUceService->DisableManagers();
    EXPECT_FALSE(pUceService->SendOptionsCmd(key, myCaps, remoteUri));

    pUceService->EnableManagers();
    EXPECT_TRUE(pUceService->SendOptionsCmd(key, myCaps, remoteUri));
}

TEST_F(UceServiceTest, SendOptionsRespCmd)
{
    IMS_UINT32 key = 1;
    IMS_SINT32 responseCode = 200;
    AString reason = AString::ConstEmpty();
    IMS_UINT32 myCaps = 1;

    IMS_TRACE_D("SendOptionsRespCmd", 0, 0, 0);
    pUceService->DisableManagers();
    EXPECT_FALSE(pUceService->SendOptionsRespCmd(key, responseCode, reason, myCaps));

    pUceService->EnableManagers();
    EXPECT_TRUE(pUceService->SendOptionsRespCmd(key, responseCode, reason, myCaps));
}

TEST_F(UceServiceTest, SendSingleSubscribeCmd)
{
    IMS_UINT32 key = 1;
    AString user = AString::ConstEmpty();

    IMS_TRACE_D("SendSingleSubscribeCmd", 0, 0, 0);

    pUceService->DisableManagers();
    EXPECT_FALSE(pUceService->SendSingleSubscribeCmd(key, user));

    pUceService->EnableManagers();
    EXPECT_TRUE(pUceService->SendSingleSubscribeCmd(key, user));
}

TEST_F(UceServiceTest, SendListSubscribeCmd)
{
    IMS_UINT32 key = 1;
    IMSList<AString> userList;

    IMS_TRACE_D("SendListSubscribeCmd", 0, 0, 0);
    pUceService->DisableManagers();
    EXPECT_FALSE(pUceService->SendListSubscribeCmd(key, userList));

    pUceService->EnableManagers();
    EXPECT_TRUE(pUceService->SendListSubscribeCmd(key, userList));
}