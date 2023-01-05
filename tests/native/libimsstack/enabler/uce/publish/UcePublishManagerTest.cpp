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
#include "publish/UcePublishManager.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

IMS_SINT32 SIM_SLOT = 20;

class TestUcePublishManager : public UcePublishManager
{
public:
    enum
    {
        IDLE,
        ON,
        PUBLISHING,
        PUBLISHED,
        REFRESHING,
        TERMINATING,
    };
    enum
    {
        PUBLISH_SUCCEEDED = 1,
        PUBLISH_FAILED,
        PUBLISH_TERMINATED,
        PUBLISH_REFRESH_STARTED,
        PUBLISH_REFRESHED,
        PUBLISH_REFRESH_FAILED,
        PUBLISH_REFRESH_NO_RESPONSE,
        TIMER_EXPIRED,
        AOS_CONNECTED,
        AOS_DISCONNECTED,
        AOS_DISCONNECTING,
        PUBLISH_REQUESTED,
        SERVICE_CLOSED,
    };

public:
    TestUcePublishManager() :
            UcePublishManager(IMS_NULL, AString("UcePublishManager"), SIM_SLOT)
    {
    }
    virtual ~TestUcePublishManager() {}

    IMS_UINT32 GetState() const { return m_eState; }

    IMS_UINT32 GetKey() const { return m_nKey; }

    void SetReceivedUnPublishRequest(IMS_BOOL value) { m_bReceivedUnPublishRequest = value; }

    IMS_BOOL GetReceivedUnPublishRequest() const { return m_bReceivedUnPublishRequest; }

    IPublicationData* GetPublicationData() { return m_pPendingPublicationData; }

    void SetPublicationData(IPublicationData* pData) { m_pPendingPublicationData = pData; }
};

class UcePublishManagerTest : public ::testing::Test
{
public:
    TestUcePublishManager* pUcePublishManager;

protected:
    virtual void SetUp() override
    {
        pUcePublishManager = new TestUcePublishManager();
        ASSERT_TRUE(pUcePublishManager != nullptr);
    }

    virtual void TearDown() override
    {
        if (pUcePublishManager)
        {
            delete pUcePublishManager;
        }
    }
};

TEST_F(UcePublishManagerTest, SendPublishRequest)
{
    IMS_TRACE_D("SendPublishRequest", 0, 0, 0);
    EXPECT_EQ(pUcePublishManager->SendPublishRequest(1, "pidf", "etag", 0, 0), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, AosConnected)
{
    IMS_TRACE_D("AosConnected", 0, 0, 0);
    EXPECT_EQ(pUcePublishManager->AosConnected(1), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, AosDisConnected)
{
    IMS_TRACE_D("AosDisConnected", 0, 0, 0);
    EXPECT_EQ(pUcePublishManager->AosDisConnected(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);

    pUcePublishManager->SetState(TestUcePublishManager::ON);
    EXPECT_EQ(pUcePublishManager->AosDisConnected(), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, AosDisConnecting)
{
    IMS_TRACE_D("AosDisConnecting", 0, 0, 0);
    EXPECT_EQ(pUcePublishManager->AosDisConnecting(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);

    pUcePublishManager->SetState(TestUcePublishManager::PUBLISHING);
    EXPECT_EQ(pUcePublishManager->AosDisConnecting(), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, ClosedService)
{
    IMS_TRACE_D("ClosedService", 0, 0, 0);
    EXPECT_EQ(pUcePublishManager->ClosedService(), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, IDLE_PublishRequested)
{
    IMS_TRACE_D("IDLE_PublishRequested", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::IDLE);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0, IMS_NULL);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_FALSE);

    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 0;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 0;
    IMSMSG objSecondMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objSecondMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
}

TEST_F(UcePublishManagerTest, IDLE_AoSConnected)
{
    IMS_TRACE_D("IDLE_AoSConnected", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::IDLE);
    IMSMSG objMsg(TestUcePublishManager::AOS_CONNECTED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, ON_AoSDisConnected)
{
    IMS_TRACE_D("ON_AoSDisConnected", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::ON);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, PUBLISHING_PublishRequested)
{
    IMS_TRACE_D("PUBLISHING_PublishRequested", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::PUBLISHING);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0, IMS_NULL);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_FALSE);

    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 0;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 0;
    IMSMSG objSecondMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objSecondMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
}

TEST_F(UcePublishManagerTest, PUBLISHING_AoSDisConnecting)
{
    IMS_TRACE_D("PUBLISHING_AoSDisConnecting", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::PUBLISHING);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, PUBLISHING_AoSDisConnected)
{
    IMS_TRACE_D("PUBLISHING_AoSDisConnected", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::PUBLISHING);
    pUcePublishManager->SetReceivedUnPublishRequest(IMS_TRUE);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, PUBLISHING_Failed)
{
    IMS_TRACE_D("PUBLISHING_Failed", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::PUBLISHING);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_FAILED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, PUBLISHED_PublishRequested)
{
    IMS_TRACE_D("PUBLISHED_PublishRequested", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::PUBLISHED);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0, IMS_NULL);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_FALSE);
}

TEST_F(UcePublishManagerTest, PUBLISHED_AoSDisconnecting)
{
    IMS_TRACE_D("PUBLISHED_AoSDisconnecting", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::PUBLISHED);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, PUBLISHED_AoSDisconnected)
{
    IMS_TRACE_D("PUBLISHED_AoSDisconnected", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::PUBLISHED);
    pUcePublishManager->SetReceivedUnPublishRequest(IMS_TRUE);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, PUBLISHED_RefreshStarted)
{
    IMS_TRACE_D("PUBLISHED_RefreshStarted", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::PUBLISHED);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REFRESH_STARTED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::REFRESHING);
}

TEST_F(UcePublishManagerTest, REFRESHING_PublishRequested)
{
    IMS_TRACE_D("REFRESHING_PublishRequested", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::REFRESHING);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0, IMS_NULL);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_FALSE);

    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 10;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 20;
    pUcePublishManager->SetReceivedUnPublishRequest(IMS_TRUE);
    IMSMSG objSecondMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objSecondMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetKey(), 0);

    pUcePublishManager->SetReceivedUnPublishRequest(IMS_FALSE);
    IMSMSG objThirdMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));
    EXPECT_EQ(pUcePublishManager->OnStateMessage(objThirdMsg), IMS_TRUE);
    IPublicationData* pData = pUcePublishManager->GetPublicationData();
    EXPECT_EQ(pData->m_nKey, 10);
    EXPECT_EQ(pData->m_nCapability, 10);
    EXPECT_EQ(pData->m_nExtended, 20);
    EXPECT_STREQ(pData->m_strEtag.GetStr(), "eTag");
    EXPECT_STREQ(pData->m_strPidfXml.GetStr(), "pidfXml");
}

TEST_F(UcePublishManagerTest, REFRESHING_AoSDisConnecting)
{
    IMS_TRACE_D("REFRESHING_AoSDisConnecting", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::REFRESHING);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, REFRESHING_AoSDisConnected)
{
    IMS_TRACE_D("REFRESHING_AoSDisConnected", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::REFRESHING);
    pUcePublishManager->SetReceivedUnPublishRequest(IMS_TRUE);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, REFRESHING_Refreshed)
{
    IMS_TRACE_D("REFRESHING_Refreshed", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::REFRESHING);
    pUcePublishManager->SetReceivedUnPublishRequest(IMS_TRUE);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REFRESHED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);

    pUcePublishManager->SetState(TestUcePublishManager::REFRESHING);
    pUcePublishManager->SetReceivedUnPublishRequest(IMS_FALSE);
    pUcePublishManager->SetPublicationData(IMS_NULL);
    IMSMSG obSecondjMsg(TestUcePublishManager::PUBLISH_REFRESHED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(obSecondjMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::PUBLISHED);

    pUcePublishManager->SetState(TestUcePublishManager::REFRESHING);
    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 10;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 20;
    pUcePublishManager->SetPublicationData(pPublicationData);
    IMSMSG obThirdjMsg(TestUcePublishManager::PUBLISH_REFRESHED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(obThirdjMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, TERMINATING_PublishRequested)
{
    IMS_TRACE_D("TERMINATING_PublishRequested", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::TERMINATING);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0, IMS_NULL);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_FALSE);

    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 0;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 0;
    IMSMSG objSecondMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objSecondMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
}

TEST_F(UcePublishManagerTest, TERMINATING_AoSDisConnecting)
{
    IMS_TRACE_D("TERMINATING_AoSDisConnecting", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::TERMINATING);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_FALSE);
}

TEST_F(UcePublishManagerTest, TERMINATING_AoSDisconnected)
{
    IMS_TRACE_D("TERMINATING_AoSDisconnected", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::TERMINATING);
    pUcePublishManager->SetReceivedUnPublishRequest(IMS_TRUE);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, TERMINATING_Publish)
{
    IMS_TRACE_D("TERMINATING_Published", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::TERMINATING);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_SUCCEEDED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);

    pUcePublishManager->SetState(TestUcePublishManager::TERMINATING);
    IMSMSG objSecondMsg(TestUcePublishManager::PUBLISH_FAILED, 0, 0);

    EXPECT_EQ(pUcePublishManager->OnStateMessage(objSecondMsg), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}