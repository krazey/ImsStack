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
#include "options/UceOptionsManager.h"
#include "options/UceOptions.h"
#include "IUce.h"
#include "MockIJniEnabler.h"
#include "MockIUceJniThread.h"
#include "JniEnablerConnector.h"
#include "MockICoreService.h"
#include "MockICapabilities.h"
#include "MockIMessage.h"
#include "MockISipMessage.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnNull;

__IMS_TRACE_TAG_USER_DECL__("UCE");

class TestUceOptionsManager : public UceOptionsManager
{
public:
    inline explicit TestUceOptionsManager(ICoreService* piCoreService) :
            UceOptionsManager(AString("UceOptionsManager"), piCoreService, 0)
    {
    }
    virtual ~TestUceOptionsManager() {}

    void setAosConnected(IMS_BOOL connected) { m_bAoSConnected = connected; }
    IMS_BOOL getAosConnected() { return m_bAoSConnected; }
    void addOptions(IMS_BOOL isSent, IMS_UINT32 key, UceOptions* pOptions)
    {
        if (isSent)
        {
            m_objSentUceOptionsMap.Add(key, pOptions);
        }
        else
        {
            m_objReceivedUceOptionsMap.Add(key, pOptions);
        }
    }
    IMS_UINT32 getListCount() const { return m_objSentUceOptionsMap.GetSize(); }
    IMS_BOOL sendMsg(IMSMSG& objMsg) { return OnMessage(objMsg); }
};

class UceOptionsManagerTest : public ::testing::Test
{
public:
    TestUceOptionsManager* pUceOptionsManager;
    MockICoreService objMockICoreService;
    MockIJniEnabler objMockJniEnabler;
    MockIUceJniThread objMockIUceJniThread;

protected:
    virtual void SetUp() override
    {
        pUceOptionsManager = new TestUceOptionsManager(&objMockICoreService);
        ASSERT_TRUE(pUceOptionsManager != nullptr);
        ON_CALL(objMockJniEnabler, GetJniThread()).WillByDefault(Return(&objMockIUceJniThread));
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, &objMockJniEnabler);
    }

    virtual void TearDown() override
    {
        if (pUceOptionsManager)
        {
            delete pUceOptionsManager;
        }
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, IMS_NULL);
    }
};

TEST_F(UceOptionsManagerTest, SendOptionsRequestInAosDisconnected)
{
    IMS_TRACE_D("SendOptionsRequestInAosDisconnected", 0, 0, 0);

    pUceOptionsManager->setAosConnected(IMS_FALSE);
    EXPECT_CALL(objMockIUceJniThread, OptionsErrorInd(_, _)).Times(1);

    pUceOptionsManager->SendOptionsRequest(0, "RemoteURI", 0);
}

TEST_F(UceOptionsManagerTest, SendOptionsRequestInAosConnected)
{
    IMS_TRACE_D("SendOptionsRequestInAosConnected", 0, 0, 0);
    TestUceOptionsManager* pTestUceOptionsManager;
    pTestUceOptionsManager = new TestUceOptionsManager(IMS_NULL);
    pTestUceOptionsManager->setAosConnected(IMS_TRUE);

    EXPECT_TRUE(pTestUceOptionsManager->SendOptionsRequest(0, "RemoteURI", 0));
    delete pTestUceOptionsManager;
}

TEST_F(UceOptionsManagerTest, SendOptionsResponse)
{
    IMS_TRACE_D("SendOptionsResponse", 0, 0, 0);
    EXPECT_FALSE(pUceOptionsManager->SendOptionsResponse(0, 0, "reason", 0));

    IMS_UINT32 key = 3;
    pUceOptionsManager->addOptions(IMS_FALSE, key, IMS_NULL);
    EXPECT_FALSE(pUceOptionsManager->SendOptionsResponse(key, 0, "reason", 0));

    UceOptions* pOptions = new UceOptions("Options", IMS_NULL, IMS_NULL, 0, IMS_TRUE, 0);
    pUceOptionsManager->addOptions(IMS_FALSE, key, pOptions);
    EXPECT_TRUE(pUceOptionsManager->SendOptionsResponse(key, 0, "reason", 0));
}

TEST_F(UceOptionsManagerTest, ReceivedOptionsWithNoMatchService)
{
    IMS_TRACE_D("ReceivedOptionsWithNoMatchService", 0, 0, 0);

    EXPECT_FALSE(pUceOptionsManager->ReceivedOptions(IMS_NULL, IMS_NULL));
}

TEST_F(UceOptionsManagerTest, ReceivedOptionsWithNoMessage)
{
    IMS_TRACE_D("ReceivedOptionsWithNoMessage", 0, 0, 0);
    MockICapabilities objMockICapabilities;
    ON_CALL(objMockICapabilities, GetPreviousRequest).WillByDefault(ReturnNull());
    EXPECT_FALSE(pUceOptionsManager->ReceivedOptions(&objMockICoreService, &objMockICapabilities));
}

TEST_F(UceOptionsManagerTest, ReceivedOptionsWithNoSipMessage)
{
    IMS_TRACE_D("ReceivedOptionsWithNoSipMessage", 0, 0, 0);
    MockICapabilities objMockICapabilities;
    MockIMessage objMockIMessage;
    ON_CALL(objMockICapabilities, GetPreviousRequest(IMessage::CAPABILITIES_QUERY))
            .WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(ReturnNull());

    EXPECT_FALSE(pUceOptionsManager->ReceivedOptions(&objMockICoreService, &objMockICapabilities));
}

TEST_F(UceOptionsManagerTest, AoSConnected)
{
    IMS_TRACE_D("AoSConnected", 0, 0, 0);

    pUceOptionsManager->AoSConnected();
    EXPECT_TRUE(pUceOptionsManager->getAosConnected());
}

TEST_F(UceOptionsManagerTest, AoSDisconnected)
{
    IMS_TRACE_D("AoSDisconnected", 0, 0, 0);

    pUceOptionsManager->AoSDisconnected();
    EXPECT_FALSE(pUceOptionsManager->getAosConnected());
}

TEST_F(UceOptionsManagerTest, ClosedService)
{
    IMS_TRACE_D("ClosedService", 0, 0, 0);

    IMS_UINT32 key = 3;
    UceOptions* pOptions = new UceOptions("Options", IMS_NULL, IMS_NULL, 0, IMS_TRUE, 0);
    pUceOptionsManager->addOptions(IMS_TRUE, key, pOptions);

    EXPECT_TRUE(pUceOptionsManager->ClosedService());
}

TEST_F(UceOptionsManagerTest, sendMsg)
{
    IMS_TRACE_D("sendMsg", 0, 0, 0);
    IMS_UINT32 key = 3;
    IMSMSG objMsg(IUUceService::UCE_SUBSCRIBE_DELETED_IND, 0, key);
    EXPECT_FALSE(pUceOptionsManager->sendMsg(objMsg));

    IMSMSG objMsg2(IUUceService::UCE_OPTIONS_DELETED_IND, 0, key);
    EXPECT_FALSE(pUceOptionsManager->sendMsg(objMsg2));

    pUceOptionsManager->addOptions(IMS_TRUE, key, IMS_NULL);
    IMSMSG objMsg3(IUUceService::UCE_OPTIONS_DELETED_IND, 0, key);
    EXPECT_FALSE(pUceOptionsManager->sendMsg(objMsg3));

    UceOptions* pOptions = new UceOptions("Options", IMS_NULL, IMS_NULL, 0, IMS_TRUE, 0);
    pUceOptionsManager->addOptions(IMS_TRUE, key, pOptions);

    IMSMSG objMsg4(IUUceService::UCE_OPTIONS_DELETED_IND, 0, key);
    EXPECT_EQ(pUceOptionsManager->getListCount(), 1);
    EXPECT_TRUE(pUceOptionsManager->sendMsg(objMsg4));

    EXPECT_EQ(pUceOptionsManager->getListCount(), 0);
}