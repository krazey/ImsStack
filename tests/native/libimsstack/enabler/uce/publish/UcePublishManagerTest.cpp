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
#include "ISipHeader.h"
#include "IMessage.h"
#include "MockIJniEnabler.h"
#include "MockIUceJniThread.h"
#include "JniEnablerConnector.h"
#include "MockICoreService.h"
#include "MockIPublication.h"
#include "MockIMessage.h"
#include "MockISipMessage.h"
#include "MockISipMessageBodyPart.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

__IMS_TRACE_TAG_USER_DECL__("UCE");

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
    inline explicit TestUcePublishManager(ICoreService* piCoreService) :
            UcePublishManager(piCoreService, AString("UcePublishManager"), 0)
    {
    }
    virtual ~TestUcePublishManager() {}

    IMS_UINT32 GetState() const { return m_eState; }

    void SetKey(IMS_UINT32 key) { m_nKey = key; }
    IMS_UINT32 GetKey() const { return m_nKey; }

    IMS_RESULT MessageMediatorAdjustMessage(ISipMessage* piSIPMsg)
    {
        return MessageMediator_AdjustMessage(piSIPMsg);
    }

    IMS_UINT32 GetConectedService() { return m_nConnectedServices; }

    void SetIPublication(IPublication* pPublication) { m_piPublication = pPublication; }

    void SetReceivedUnPublishRequest(IMS_BOOL value) { m_bReceivedUnPublishRequest = value; }

    IMS_BOOL GetReceivedUnPublishRequest() const { return m_bReceivedUnPublishRequest; }

    IPublicationData* GetPublicationData() { return m_pPendingPublicationData; }

    void SetPublicationData(IPublicationData* pData) { m_pPendingPublicationData = pData; }

    IMS_BOOL IdlePublishRequested(IMSMSG& objMsg) { return StateIDLE_PublishRequested(objMsg); }
    IMS_BOOL IdleAoSConnected(IMSMSG& objMsg) { return StateIDLE_AoSConnected(objMsg); }
    IMS_BOOL OnPublishRequested(IMSMSG& objMsg) { return StateON_PublishRequested(objMsg); }
    IMS_BOOL OnAoSDisConnected(IMSMSG& objMsg) { return StateON_AoSDisConnected(objMsg); }
    IMS_BOOL PublishingPublishRequested(IMSMSG& objMsg)
    {
        return StatePUBLISHING_PublishRequested(objMsg);
    }
    IMS_BOOL PublishingAoSDisConnecting(IMSMSG& objMsg)
    {
        return StatePUBLISHING_AoSDisConnecting(objMsg);
    }
    IMS_BOOL PublishingAoSDisConnected(IMSMSG& objMsg)
    {
        return StatePUBLISHING_AoSDisConnected(objMsg);
    }
    IMS_BOOL PublishingPublished(IMSMSG& objMsg) { return StatePUBLISHING_Published(objMsg); }
    IMS_BOOL PublishingFailed(IMSMSG& objMsg) { return StatePUBLISHING_Failed(objMsg); }
    IMS_BOOL PublishedPublishRequested(IMSMSG& objMsg)
    {
        return StatePUBLISHED_PublishRequested(objMsg);
    }
    IMS_BOOL PublishedAoSDisconnecting(IMSMSG& objMsg)
    {
        return StatePUBLISHED_AoSDisconnecting(objMsg);
    }
    IMS_BOOL PublishedAoSDisconnected(IMSMSG& objMsg)
    {
        return StatePUBLISHED_AoSDisconnected(objMsg);
    }
    IMS_BOOL PublishedRefreshStarted(IMSMSG& objMsg)
    {
        return StatePUBLISHED_RefreshStarted(objMsg);
    }
    IMS_BOOL RefreshingPublishRequested(IMSMSG& objMsg)
    {
        return StateREFRESHING_PublishRequested(objMsg);
    }
    IMS_BOOL RefreshingAoSDisConnecting(IMSMSG& objMsg)
    {
        return StateREFRESHING_AoSDisConnecting(objMsg);
    }
    IMS_BOOL RefreshingAoSDisConnected(IMSMSG& objMsg)
    {
        return StateREFRESHING_AoSDisConnected(objMsg);
    }
    IMS_BOOL RefreshingRefreshed(IMSMSG& objMsg) { return StateREFRESHING_Refreshed(objMsg); }
    IMS_BOOL RefreshingRefreshFailed(IMSMSG& objMsg)
    {
        return StateREFRESHING_RefreshFailed(objMsg);
    }
    IMS_BOOL RefreshingRefreshFailedWithNoResponse(IMSMSG& objMsg)
    {
        return StateREFRESHING_RefreshFailedWithNoResponse(objMsg);
    }
    IMS_BOOL TerminatingPublishRequested(IMSMSG& objMsg)
    {
        return StateTERMINATING_PublishRequested(objMsg);
    }
    IMS_BOOL TerminatingAoSDisconnected(IMSMSG& objMsg)
    {
        return StateTERMINATING_AoSDisconnected(objMsg);
    }
    IMS_BOOL TerminatingPublished(IMSMSG& objMsg) { return StateTERMINATING_Published(objMsg); }
    IMS_BOOL TerminatingFailed(IMSMSG& objMsg) { return StateTERMINATING_Failed(objMsg); }
    IMS_BOOL AllTerminated(IMSMSG& objMsg) { return StateALL_Terminated(objMsg); }
};

class UcePublishManagerTest : public ::testing::Test
{
public:
    TestUcePublishManager* pUcePublishManager;
    MockICoreService objMockICoreService;
    MockIJniEnabler objMockJniEnabler;
    MockIUceJniThread objMockIUceJniThread;
    MockIPublication objMockIPublication;
    MockIMessage objMockIMessage;

protected:
    virtual void SetUp() override
    {
        pUcePublishManager = new TestUcePublishManager(&objMockICoreService);
        ASSERT_TRUE(pUcePublishManager != nullptr);
        ON_CALL(objMockJniEnabler, GetJniThread()).WillByDefault(Return(&objMockIUceJniThread));
        ON_CALL(objMockIPublication, GetPreviousResponse(IMessage::PUBLICATION_PUBLISH))
                .WillByDefault(Return(&objMockIMessage));
        ON_CALL(objMockIPublication, GetNextRequest).WillByDefault(Return(&objMockIMessage));
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, &objMockJniEnabler);
    }

    virtual void TearDown() override
    {
        if (pUcePublishManager)
        {
            delete pUcePublishManager;
        }
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, IMS_NULL);
    }
};

TEST_F(UcePublishManagerTest, MessageMediatorAdjustMessage)
{
    IMS_TRACE_D("MessageMediatorAdjustMessage", 0, 0, 0);
    MockISipMessage objISipMessage;

    SipMethod objSubscribeMethod = SipMethod::SUBSCRIBE;

    ON_CALL(objISipMessage, GetMethod).WillByDefault(ReturnRef(objSubscribeMethod));

    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objISipMessage), IMS_SUCCESS);
}

TEST_F(UcePublishManagerTest, SendPublishRequest)
{
    IMS_TRACE_D("SendPublishRequest", 0, 0, 0);
    EXPECT_EQ(pUcePublishManager->SendPublishRequest(1, "pidf", "etag", 0, 0), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, AosConnected)
{
    IMS_TRACE_D("AosConnected", 0, 0, 0);
    IMS_UINT32 conectedService = 10;

    EXPECT_EQ(pUcePublishManager->AosConnected(conectedService), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetConectedService(), conectedService);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, AosDisConnected)
{
    IMS_TRACE_D("AosDisConnected", 0, 0, 0);
    EXPECT_EQ(pUcePublishManager->AosDisConnected(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetConectedService(), 0);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);

    pUcePublishManager->SetState(TestUcePublishManager::ON);
    EXPECT_EQ(pUcePublishManager->AosDisConnected(), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetConectedService(), 0);
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

TEST_F(UcePublishManagerTest, SetState)
{
    IMS_TRACE_D("SetState", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::TERMINATING);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::TERMINATING);
}

TEST_F(UcePublishManagerTest, IdlePublishRequested)
{
    IMS_TRACE_D("IdlePublishRequested", 0, 0, 0);
    IPublicationData* pPublicationData = IMS_NULL;
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));
    EXPECT_FALSE(pUcePublishManager->IdlePublishRequested(objMsg));

    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 0;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 0;
    IMSMSG objSecondMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    EXPECT_TRUE(pUcePublishManager->IdlePublishRequested(objSecondMsg));
    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
}

TEST_F(UcePublishManagerTest, IdleAoSConnected)
{
    IMS_TRACE_D("IdleAoSConnected", 0, 0, 0);
    IMSMSG objMsg(TestUcePublishManager::AOS_CONNECTED, 0, 0);

    pUcePublishManager->IdleAoSConnected(objMsg);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, OnPublishRequestFailedDuetoFailureCreatePulibcation)
{
    IMS_TRACE_D("OnPublishRequestFailedDuetoFailureCreatePulibcation", 0, 0, 0);

    ON_CALL(objMockICoreService, CreatePublication(_, _, _)).WillByDefault(ReturnNull());

    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 0;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 0;
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    pUcePublishManager->OnPublishRequested(objMsg);
}

TEST_F(UcePublishManagerTest, OnPublishRequested)
{
    IMS_TRACE_D("OnPublishRequested", 0, 0, 0);

    EXPECT_CALL(objMockIPublication, SetListener(_)).Times(1);
    EXPECT_CALL(objMockIPublication, SetRefreshListener(_)).Times(1);
    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    ON_CALL(objMockICoreService, CreatePublication(_, _, _))
            .WillByDefault(Return(&objMockIPublication));

    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 0;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 0;
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    pUcePublishManager->OnPublishRequested(objMsg);
}

TEST_F(UcePublishManagerTest, OnAoSDisConnected)
{
    IMS_TRACE_D("OnAoSDisConnected", 0, 0, 0);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);
    pUcePublishManager->OnAoSDisConnected(objMsg);

    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, PublishingPublishRequested)
{
    IMS_TRACE_D("PublishingPublishRequested", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 0;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 0;
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    pUcePublishManager->PublishingPublishRequested(objMsg);
    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
}

TEST_F(UcePublishManagerTest, PublishingAoSDisConnecting)
{
    IMS_TRACE_D("PublishingAoSDisConnecting", 0, 0, 0);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);

    pUcePublishManager->PublishingAoSDisConnecting(objMsg);
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, PublishingAoSDisConnected)
{
    IMS_TRACE_D("PublishingAoSDisConnected", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);
    pUcePublishManager->PublishingAoSDisConnected(objMsg);

    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, PublishingPublished)
{
    IMS_TRACE_D("PublishingPublished", 0, 0, 0);
    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->SetKey(10);

    EXPECT_CALL(objMockIUceJniThread, PublishResponseInd(_, _, _, _, _, _, _, _)).Times(1);

    MockISipMessage objMockISipMessage;
    IMSList<AString> objReasonHeaders;
    objReasonHeaders.Clear();
    AString reason = "OK";

    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString::ConstEmpty()));
    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(200));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_SUCCEEDED, 0, 0);

    pUcePublishManager->PublishingPublished(objMsg);

    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::PUBLISHED);
}

TEST_F(UcePublishManagerTest, PublishingFailedWithNoIMessage)
{
    IMS_TRACE_D("PublishingFailedWithNoIMessage", 0, 0, 0);
    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->SetKey(10);

    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    ON_CALL(objMockIPublication, GetPreviousResponse(IMessage::PUBLICATION_PUBLISH))
            .WillByDefault(ReturnNull());

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_FAILED, 0, 0);

    pUcePublishManager->PublishingFailed(objMsg);

    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, PublishingFailed)
{
    IMS_TRACE_D("PublishingFailed", 0, 0, 0);
    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->SetKey(10);

    EXPECT_CALL(objMockIUceJniThread, PublishResponseInd(_, _, _, _, _, _, _, _)).Times(1);

    MockISipMessage objMockISipMessage;
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));

    AString reason = "Not Implemented";
    IMSList<AString> objReasonHeaders;
    objReasonHeaders.Clear();

    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(501));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_FAILED, 0, 0);
    pUcePublishManager->PublishingFailed(objMsg);

    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, PublishingFailedNoKey)
{
    IMS_TRACE_D("PublishingFailedNoKey", 0, 0, 0);
    pUcePublishManager->SetIPublication(&objMockIPublication);

    EXPECT_CALL(objMockIUceJniThread, PublishUpdatedInd(_, _, _, _, _, _, _)).Times(1);

    MockISipMessage objMockISipMessage;
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));

    AString reason = "Not Implemented";
    IMSList<AString> objReasonHeaders;
    objReasonHeaders.Clear();

    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(501));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_FAILED, 0, 0);
    pUcePublishManager->PublishingFailed(objMsg);

    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, PublishedPublishRequested)
{
    IMS_TRACE_D("PublishedPublishRequested", 0, 0, 0);
    MockISipMessage objMockISipMessage;
    MockISipMessageBodyPart objMockISipMessageBodyPart;

    EXPECT_CALL(objMockIPublication, SetListener(_)).Times(1);
    EXPECT_CALL(objMockIPublication, SetRefreshListener(_)).Times(1);
    EXPECT_CALL(objMockISipMessage, AddHeader(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(objMockISipMessageBodyPart, SetContent(_)).Times(1);

    ON_CALL(objMockICoreService, CreatePublication(_, _, _))
            .WillByDefault(Return(&objMockIPublication));

    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, CreateBodyPart).WillByDefault(Return(&objMockISipMessageBodyPart));
    ON_CALL(objMockIPublication, Publish).WillByDefault(Return(IMS_SUCCESS));

    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 0;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 0;

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    pUcePublishManager->PublishedPublishRequested(objMsg);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::PUBLISHING);
}

TEST_F(UcePublishManagerTest, PublishedAoSDisconnecting)
{
    IMS_TRACE_D("PublishedAoSDisconnecting", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, UnPublishedInd).Times(1);

    ON_CALL(objMockICoreService, CreatePublication(_, _, _)).WillByDefault(ReturnNull());

    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);
    pUcePublishManager->PublishedAoSDisconnecting(objMsg);

    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, PublishedAoSDisconnected)
{
    IMS_TRACE_D("PublishedAoSDisconnected", 0, 0, 0);
    EXPECT_CALL(objMockIUceJniThread, UnPublishedInd).Times(1);

    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);
    pUcePublishManager->PublishedAoSDisconnected(objMsg);

    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, PublishedRefreshStarted)
{
    IMS_TRACE_D("PublishedRefreshStarted", 0, 0, 0);

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REFRESH_STARTED, 0, 0);
    pUcePublishManager->PublishedRefreshStarted(objMsg);

    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::REFRESHING);
}

TEST_F(UcePublishManagerTest, RefreshingPublishRequested)
{
    IMS_TRACE_D("RefreshingPublishRequested", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 10;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 20;
    pUcePublishManager->SetReceivedUnPublishRequest(IMS_TRUE);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    pUcePublishManager->RefreshingPublishRequested(objMsg);
    EXPECT_EQ(pUcePublishManager->GetKey(), 0);

    pUcePublishManager->SetReceivedUnPublishRequest(IMS_FALSE);
    IMSMSG objSecondMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));
    pUcePublishManager->RefreshingPublishRequested(objSecondMsg);

    IPublicationData* pData = pUcePublishManager->GetPublicationData();
    EXPECT_EQ(pData->m_nKey, 10);
    EXPECT_EQ(pData->m_nCapability, 10);
    EXPECT_EQ(pData->m_nExtended, 20);
    EXPECT_STREQ(pData->m_strEtag.GetStr(), "eTag");
    EXPECT_STREQ(pData->m_strPidfXml.GetStr(), "pidfXml");
}

TEST_F(UcePublishManagerTest, RefreshingAoSDisConnecting)
{
    IMS_TRACE_D("RefreshingAoSDisConnecting", 0, 0, 0);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);

    pUcePublishManager->RefreshingAoSDisConnecting(objMsg);
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, RefreshingAoSDisConnected)
{
    IMS_TRACE_D("RefreshingAoSDisConnected", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::REFRESHING);
    pUcePublishManager->SetReceivedUnPublishRequest(IMS_TRUE);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);

    pUcePublishManager->RefreshingAoSDisConnected(objMsg);
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, RefreshingRefreshedFromPreviouslyReceivedUnpublish)
{
    IMS_TRACE_D("RefreshingRefreshedFromPreviouslyReceivedUnpublish", 0, 0, 0);
    pUcePublishManager->SetReceivedUnPublishRequest(IMS_TRUE);

    MockISipMessage objMockISipMessage;

    EXPECT_CALL(objMockIPublication, SetListener(_)).Times(1);
    EXPECT_CALL(objMockIPublication, SetRefreshListener(_)).Times(1);
    EXPECT_CALL(objMockIUceJniThread, UnPublishedInd).Times(1);
    EXPECT_CALL(objMockISipMessage, AddHeader(_, _, _)).Times(AnyNumber());

    ON_CALL(objMockICoreService, CreatePublication(_, _, _))
            .WillByDefault(Return(&objMockIPublication));
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockIPublication, Unpublish).WillByDefault(Return(IMS_SUCCESS));

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REFRESHED, 0, 0);
    pUcePublishManager->RefreshingRefreshed(objMsg);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::TERMINATING);
}

TEST_F(UcePublishManagerTest, RefreshingRefreshed)
{
    IMS_TRACE_D("RefreshingRefreshed", 0, 0, 0);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REFRESHED, 0, 0);

    pUcePublishManager->RefreshingRefreshed(objMsg);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::PUBLISHED);
}

TEST_F(UcePublishManagerTest, RefreshingRefreshFailed)
{
    IMS_TRACE_D("RefreshingRefreshFailed", 0, 0, 0);
    pUcePublishManager->SetIPublication(&objMockIPublication);

    EXPECT_CALL(objMockIUceJniThread, PublishUpdatedInd(_, _, _, _, _, _, _)).Times(1);

    MockISipMessage objMockISipMessage;
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));

    AString reason = "Not Implemented";
    IMSList<AString> objReasonHeaders;
    objReasonHeaders.Clear();

    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(501));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REFRESH_FAILED, 0, 0);

    pUcePublishManager->RefreshingRefreshFailed(objMsg);
}

TEST_F(UcePublishManagerTest, RefreshingRefreshFailedWithNoResponse)
{
    IMS_TRACE_D("RefreshingRefreshFailedWithNoResponse", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, PublishUpdatedInd(_, _, _, _, _, _, _)).Times(1);

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REFRESH_NO_RESPONSE, 0, 0);

    pUcePublishManager->RefreshingRefreshFailedWithNoResponse(objMsg);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, TerminatingPublishRequested)
{
    IMS_TRACE_D("TerminatingPublishRequested", 0, 0, 0);
    pUcePublishManager->SetKey(10);

    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    IPublicationData* pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 0;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 0;
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    pUcePublishManager->TerminatingPublishRequested(objMsg);
    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
}

TEST_F(UcePublishManagerTest, TerminatingAoSDisconnected)
{
    IMS_TRACE_D("TerminatingAoSDisconnected", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::TERMINATING);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);

    pUcePublishManager->TerminatingAoSDisconnected(objMsg);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, TerminatingPublished)
{
    IMS_TRACE_D("TerminatingPublished", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::TERMINATING);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_SUCCEEDED, 0, 0);

    pUcePublishManager->TerminatingPublished(objMsg);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, TerminatingFailed)
{
    IMS_TRACE_D("TerminatingFailed", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::TERMINATING);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_FAILED, 0, 0);

    pUcePublishManager->TerminatingFailed(objMsg);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, AllTerminated)
{
    IMS_TRACE_D("AllTerminated", 0, 0, 0);
    pUcePublishManager->SetState(TestUcePublishManager::PUBLISHING);

    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_TERMINATED, 0, 0);

    pUcePublishManager->AllTerminated(objMsg);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);

    pUcePublishManager->SetState(TestUcePublishManager::TERMINATING);

    EXPECT_CALL(objMockIUceJniThread, PublishUpdatedInd(_, _, _, _, _, _, _)).Times(1);

    IMSMSG objSecondMsg(TestUcePublishManager::PUBLISH_TERMINATED, 0, 0);

    pUcePublishManager->AllTerminated(objSecondMsg);
    EXPECT_EQ(pUcePublishManager->GetState(), TestUcePublishManager::ON);
}
