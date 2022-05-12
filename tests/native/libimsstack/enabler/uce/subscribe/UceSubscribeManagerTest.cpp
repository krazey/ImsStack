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
#include "subscribe/UceSubscribeManager.h"
#include "subscribe/UceSubscribe.h"
#include "IUUceService.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

using ::testing::AnyNumber;
using ::testing::Return;

IMS_SINT32 SUBSCRIBE_MNGR_SIM_SLOT = 20;
IMS_UINT32 SUBSCRIBE_KEY = 20;

class TestUceSubscribeManager : public UceSubscribeManager
{
public:
    TestUceSubscribeManager() :
            UceSubscribeManager(AString("UceSubscribeManager"), IMS_NULL,
                    AString("JniUceServiceThread"), SUBSCRIBE_MNGR_SIM_SLOT)
    {
    }
    virtual ~TestUceSubscribeManager() {}

    IMS_UINT32 GetListCount() { return m_objUceSubscribeList.GetSize(); }

    void AddUceSubscribe(UceSubscribe* pUceSubscribe)
    {
        m_objUceSubscribeList.Append(pUceSubscribe);
    }

    IMS_BOOL SendDeleteIndMessage(UceSubscribe* pUceSubscribe)
    {
        IMSMSG objUIMsg(IUUceService::UCE_SUBSCRIBE_DELETED_IND, 0,
                reinterpret_cast<IMS_UINTP>(pUceSubscribe));
        return OnMessage(objUIMsg);
    }
};

class UceSubscribeManagerTest : public ::testing::Test
{
public:
    TestUceSubscribeManager* pUceSubscribeManager;

protected:
    virtual void SetUp() override
    {
        pUceSubscribeManager = new TestUceSubscribeManager();
        ASSERT_TRUE(pUceSubscribeManager != nullptr);
    }

    virtual void TearDown() override
    {
        if (pUceSubscribeManager)
        {
            delete pUceSubscribeManager;
        }
    }
};

TEST_F(UceSubscribeManagerTest, QuerySingleCapability)
{
    IMS_TRACE_D("QuerySingleCapability", 0, 0, 0);
    EXPECT_EQ(
            pUceSubscribeManager->QuerySingleCapability("tel:+123456789", SUBSCRIBE_KEY), IMS_TRUE);
    EXPECT_EQ(pUceSubscribeManager->GetListCount(), 1);

    EXPECT_EQ(
            pUceSubscribeManager->QuerySingleCapability("tel:+123456789", SUBSCRIBE_KEY), IMS_TRUE);
    EXPECT_EQ(pUceSubscribeManager->GetListCount(), 2);
}

TEST_F(UceSubscribeManagerTest, QueryMultiCapability)
{
    IMS_TRACE_D("QueryMultiCapability", 0, 0, 0);
    IMSList<AString> objUsers = IMSList<AString>();
    objUsers.Append("tel:+123456789");
    EXPECT_EQ(pUceSubscribeManager->QueryMultiCapability(objUsers, SUBSCRIBE_KEY), IMS_TRUE);
    EXPECT_EQ(pUceSubscribeManager->GetListCount(), 1);

    EXPECT_EQ(pUceSubscribeManager->QueryMultiCapability(objUsers, SUBSCRIBE_KEY), IMS_TRUE);
    EXPECT_EQ(pUceSubscribeManager->GetListCount(), 2);
}

TEST_F(UceSubscribeManagerTest, OnMessage)
{
    IMS_TRACE_D("OnMessage", 0, 0, 0);
    UceSubscribe* pUceSubscribe = new UceSubscribe(IMS_NULL, "strAppName", "GetName", 0, 0);
    pUceSubscribeManager->AddUceSubscribe(pUceSubscribe);

    EXPECT_EQ(pUceSubscribeManager->GetListCount(), 1);
    EXPECT_EQ(pUceSubscribeManager->SendDeleteIndMessage(pUceSubscribe), IMS_TRUE);
    EXPECT_EQ(pUceSubscribeManager->GetListCount(), 0);
}
