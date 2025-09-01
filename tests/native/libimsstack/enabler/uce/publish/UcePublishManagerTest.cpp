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
#include "MockITimer.h"
#include "MockISipClientConnection.h"
#include "TestTimerService.h"
#include "PlatformContext.h"
#include "def/UceDef.h"
#include "config/UceConfig.h"
#include "config/UceAssetItems.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

__IMS_TRACE_TAG_UCE__;

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
    virtual ~TestUcePublishManager() override {}

    IMS_UINT32 GetStateInternal() const { return m_eState; }

    void SetKey(IMS_UINT32 key) { m_nKey = key; }
    IMS_UINT32 GetKey() const { return m_nKey; }

    IMS_RESULT MessageMediatorAdjustMessage(ISipMessage* piSIPMsg)
    {
        return MessageMediator_AdjustMessage(piSIPMsg);
    }

    void SetConectedService(IMS_UINT32 service) { m_nConnectedServices = service; }
    void SetExponentialTimer(ITimer* piTimer) { m_pExponentialTimer = piTimer; }
    void SetRetryTimer(ITimer* piTimer) { m_pRetryTimer = piTimer; }
    void SetRetryAfterTimer(ITimer* piTimer) { m_pRetryAfterTimer = piTimer; }
    void TimerExpired(ITimer* piTimer) { Timer_TimerExpired(piTimer); }

    void Delivered(IPublication* piPub) { PublicationDelivered(piPub); }
    void DeliveryFailed(IPublication* piPub) { PublicationDeliveryFailed(piPub); }
    void Terminated(IPublication* piPub) { PublicationTerminated(piPub); }
    void RefreshStarted(IPublication* piPub) { PublicationRefreshStarted(piPub); }
    void RefreshCompleted(IPublication* piPub) { PublicationRefreshCompleted(piPub); }
    void NotifyCompleted(ISipClientConnection* piScc) { Refresh_NotifyCompleted(piScc); }
    void NotifyTerminated() { Refresh_NotifyTerminated(); }
    void NotifyTimerExpired(IMS_BOOL& bDoRefresh) { Refresh_NotifyTimerExpired(bDoRefresh); }

    IMS_UINT32 GetConectedService() { return m_nConnectedServices; }

    void SetIPublication(IPublication* pPublication) { m_piPublication = pPublication; }

    void SetReceivedUnPublishRequest(IMS_BOOL value) { m_bReceivedUnPublishRequest = value; }

    IMS_BOOL GetReceivedUnPublishRequest() const { return m_bReceivedUnPublishRequest; }

    IPublicationData* GetPublicationData() { return m_pPendingPublicationData; }

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
    inline UcePublishManagerTest() :
            objTimerService(),
            objTimer(objTimerService.GetMockTimer())
    {
        pUcePublishManager = IMS_NULL;
    }
    TestTimerService objTimerService;
    MockITimer& objTimer;
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
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);
    }

    virtual void TearDown() override
    {
        if (pUcePublishManager)
        {
            delete pUcePublishManager;
        }
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }
};

TEST_F(UcePublishManagerTest, MessageMediatorAdjustMessage)
{
    IMS_TRACE_D("MessageMediatorAdjustMessage", 0, 0, 0);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(IMS_NULL), IMS_SUCCESS);

    MockISipMessage objISipMessage;

    SipMethod objSubscribeMethod = SipMethod::SUBSCRIBE;

    ON_CALL(objISipMessage, GetMethod).WillByDefault(ReturnRef(objSubscribeMethod));

    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objISipMessage), IMS_SUCCESS);
}

TEST_F(UcePublishManagerTest, SetContactHeader)
{
    IMS_TRACE_D("SetContactHeader", 0, 0, 0);
    MockISipMessage objMockISipMessage;

    SipMethod objSubscribeMethod = SipMethod::PUBLISH;
    ON_CALL(objMockISipMessage, GetMethod).WillByDefault(ReturnRef(objSubscribeMethod));
    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString::ConstEmpty()));

    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    UceAssetItems* objNew = new UceAssetItems();
    // KEY_USE_CONTACT_HEADER_IN_PUBLISH
    objNew->m_bUseContactHeaderInPublish = IMS_TRUE;
    UceConfig::GetInstance()->SetConfig(0, objNew);

    pUcePublishManager->SetConectedService(0);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    EXPECT_CALL(objMockISipMessage, SetHeader).Times(AnyNumber());

    pUcePublishManager->SetConectedService(CONNECTED_SERVICE_CPM_SESSION);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUcePublishManager->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CALL_COMPOSER);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUcePublishManager->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CPM_MSG);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUcePublishManager->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CPM_LARGEMSG);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_PRESENCE)));
    pUcePublishManager->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_PRESENCE);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUcePublishManager->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_HTTPFT);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUcePublishManager->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_GEOPUSH);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUcePublishManager->SetConectedService(CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_FTSMS);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUcePublishManager->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_GEOSMS);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader)
            .WillByDefault(Return(AString(UceTag::TAG_CHATBOT_SESSION)));
    pUcePublishManager->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CHATBOT);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUcePublishManager->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    pUcePublishManager->SetConectedService(
            CONNECTED_SERVICE_CHATBOT_V1 | CONNECTED_SERVICE_CPM_SESSION);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    pUcePublishManager->SetConectedService(
            CONNECTED_SERVICE_CHATBOT_V2 | CONNECTED_SERVICE_CPM_SESSION);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    IMS_UINT32 service = CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CALL_COMPOSER |
            CONNECTED_SERVICE_CPM_MSG | CONNECTED_SERVICE_CPM_LARGEMSG |
            CONNECTED_SERVICE_PRESENCE | CONNECTED_SERVICE_HTTPFT | CONNECTED_SERVICE_GEOPUSH |
            CONNECTED_SERVICE_FTSMS | CONNECTED_SERVICE_GEOSMS | CONNECTED_SERVICE_CHATBOT |
            CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG | CONNECTED_SERVICE_CHATBOT_V1 |
            CONNECTED_SERVICE_CHATBOT_V2;
    pUcePublishManager->SetConectedService(service);
    EXPECT_EQ(pUcePublishManager->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);
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
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, AosDisConnected)
{
    IMS_TRACE_D("AosDisConnected", 0, 0, 0);
    EXPECT_EQ(pUcePublishManager->AosDisConnected(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetConectedService(), 0);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::IDLE);

    pUcePublishManager->UpdateState(TestUcePublishManager::ON);
    EXPECT_EQ(pUcePublishManager->AosDisConnected(), IMS_TRUE);
    EXPECT_EQ(pUcePublishManager->GetConectedService(), 0);
}

TEST_F(UcePublishManagerTest, AosDisConnecting)
{
    IMS_TRACE_D("AosDisConnecting", 0, 0, 0);
    EXPECT_EQ(pUcePublishManager->AosDisConnecting(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::IDLE);

    pUcePublishManager->UpdateState(TestUcePublishManager::PUBLISHING);
    EXPECT_EQ(pUcePublishManager->AosDisConnecting(), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, ClosedService)
{
    IMS_TRACE_D("ClosedService", 0, 0, 0);
    EXPECT_EQ(pUcePublishManager->ClosedService(), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, UpdateState)
{
    IMS_TRACE_D("UpdateState", 0, 0, 0);
    pUcePublishManager->UpdateState(TestUcePublishManager::TERMINATING);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::TERMINATING);
}

TEST_F(UcePublishManagerTest, TimerExpired)
{
    IMS_TRACE_D("TimerExpired", 0, 0, 0);
    EXPECT_CALL(objTimer, KillTimer).Times(3);

    pUcePublishManager->SetExponentialTimer(&objTimer);
    pUcePublishManager->TimerExpired(&objTimer);

    pUcePublishManager->SetRetryTimer(&objTimer);
    pUcePublishManager->TimerExpired(&objTimer);

    pUcePublishManager->SetRetryAfterTimer(&objTimer);
    pUcePublishManager->TimerExpired(&objTimer);
}

TEST_F(UcePublishManagerTest, PublicationDelivered)
{
    IMS_TRACE_D("PublicationDelivered", 0, 0, 0);

    pUcePublishManager->Delivered(&objMockIPublication);

    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->Delivered(&objMockIPublication);
}

TEST_F(UcePublishManagerTest, PublicationDeliveryFailed)
{
    IMS_TRACE_D("PublicationDeliveryFailed", 0, 0, 0);

    pUcePublishManager->DeliveryFailed(&objMockIPublication);

    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->DeliveryFailed(&objMockIPublication);
}

TEST_F(UcePublishManagerTest, PublicationTerminated)
{
    IMS_TRACE_D("PublicationTerminated", 0, 0, 0);

    pUcePublishManager->Terminated(&objMockIPublication);

    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->Terminated(&objMockIPublication);
}

TEST_F(UcePublishManagerTest, PublicationRefreshStarted)
{
    IMS_TRACE_D("PublicationRefreshStarted", 0, 0, 0);

    pUcePublishManager->RefreshStarted(&objMockIPublication);

    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->RefreshStarted(&objMockIPublication);
}

TEST_F(UcePublishManagerTest, PublicationRefreshCompleted)
{
    IMS_TRACE_D("PublicationRefreshCompleted", 0, 0, 0);
    pUcePublishManager->RefreshCompleted(&objMockIPublication);

    MockISipMessage objMockISipMessage;
    AString reason = "OK";

    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(200));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));

    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->RefreshCompleted(&objMockIPublication);

    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(403));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));

    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->RefreshCompleted(&objMockIPublication);
}

TEST_F(UcePublishManagerTest, RefreshNotifyCompleted)
{
    IMS_TRACE_D("RefreshNotifyCompleted", 0, 0, 0);
    pUcePublishManager->NotifyCompleted(IMS_NULL);

    MockISipClientConnection objMockISipClientConnection;
    ON_CALL(objMockISipClientConnection, GetStatusCode).WillByDefault(Return(0));
    pUcePublishManager->NotifyCompleted(&objMockISipClientConnection);
}

TEST_F(UcePublishManagerTest, RefreshNotifyTerminated)
{
    IMS_TRACE_D("RefreshNotifyTerminated", 0, 0, 0);
    pUcePublishManager->NotifyTerminated();
}

TEST_F(UcePublishManagerTest, RefreshNotifyTimerExpired)
{
    IMS_TRACE_D("RefreshNotifyTimerExpired", 0, 0, 0);
    IMS_BOOL bDoRefresh = IMS_TRUE;
    pUcePublishManager->NotifyTimerExpired(bDoRefresh);
    EXPECT_TRUE(bDoRefresh);
}

TEST_F(UcePublishManagerTest, StateIDLE_PublishRequested)
{
    IMS_TRACE_D("StateIDLE_PublishRequested", 0, 0, 0);
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

TEST_F(UcePublishManagerTest, StateIDLE_AoSConnected)
{
    IMS_TRACE_D("StateIDLE_AoSConnected", 0, 0, 0);
    IMSMSG objMsg(TestUcePublishManager::AOS_CONNECTED, 0, 0);

    pUcePublishManager->IdleAoSConnected(objMsg);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, StateON_PublishRequestedWithNoPulibcation)
{
    IMS_TRACE_D("StateON_PublishRequestedWithNoPulibcation", 0, 0, 0);

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

    EXPECT_FALSE(pUcePublishManager->OnPublishRequested(objMsg));
}

TEST_F(UcePublishManagerTest, StateON_PublishRequested)
{
    IMS_TRACE_D("StateON_PublishRequested", 0, 0, 0);

    IPublicationData* pPublicationData = IMS_NULL;
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    EXPECT_FALSE(pUcePublishManager->OnPublishRequested(objMsg));

    EXPECT_CALL(objMockIPublication, SetListener(_)).Times(1);
    EXPECT_CALL(objMockIPublication, SetRefreshListener(_)).Times(1);
    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    ON_CALL(objMockICoreService, CreatePublication(_, _, _))
            .WillByDefault(Return(&objMockIPublication));

    pPublicationData = new IPublicationData();
    pPublicationData->m_strEtag = "eTag";
    pPublicationData->m_nKey = 10;
    pPublicationData->m_nCapability = 0;
    pPublicationData->m_strPidfXml = "pidfXml";
    pPublicationData->m_nExtended = 0;
    IMSMSG objMsg1(TestUcePublishManager::PUBLISH_REQUESTED, 0,
            reinterpret_cast<IMS_UINTP>(pPublicationData));

    pUcePublishManager->OnPublishRequested(objMsg1);
}

TEST_F(UcePublishManagerTest, StateON_AoSDisConnected)
{
    IMS_TRACE_D("StateON_AoSDisConnected", 0, 0, 0);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);
    EXPECT_TRUE(pUcePublishManager->OnAoSDisConnected(objMsg));

    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, StatePUBLISHING_PublishRequested)
{
    IMS_TRACE_D("StatePUBLISHING_PublishRequested", 0, 0, 0);

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

TEST_F(UcePublishManagerTest, StatePUBLISHING_AoSDisConnecting)
{
    IMS_TRACE_D("StatePUBLISHING_AoSDisConnecting", 0, 0, 0);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);

    EXPECT_TRUE(pUcePublishManager->PublishingAoSDisConnecting(objMsg));
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, PublishingAoSDisConnected)
{
    IMS_TRACE_D("PublishingAoSDisConnected", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);
    pUcePublishManager->PublishingAoSDisConnected(objMsg);

    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, StatePUBLISHING_Published)
{
    IMS_TRACE_D("StatePUBLISHING_Published", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, PublishResponseInd(_, _, _, _, _, _, _, _))
            .Times(AnyNumber());

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_SUCCEEDED, 0, 0);
    EXPECT_TRUE(pUcePublishManager->PublishingPublished(objMsg));

    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->SetKey(10);

    MockISipMessage objMockISipMessage;
    ImsList<AString> objReasonHeaders;
    objReasonHeaders.Clear();
    AString reason = "OK";

    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString::ConstEmpty()));
    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(200));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));

    EXPECT_TRUE(pUcePublishManager->PublishingPublished(objMsg));

    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::PUBLISHED);
}

TEST_F(UcePublishManagerTest, StatePUBLISHING_FailedWithNoIMessage)
{
    IMS_TRACE_D("StatePUBLISHING_FailedWithNoIMessage", 0, 0, 0);
    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->SetKey(10);

    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    ON_CALL(objMockIPublication, GetPreviousResponse(IMessage::PUBLICATION_PUBLISH))
            .WillByDefault(ReturnNull());

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_FAILED, 0, 0);

    pUcePublishManager->PublishingFailed(objMsg);

    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, StatePUBLISHING_Failed)
{
    IMS_TRACE_D("StatePUBLISHING_Failed", 0, 0, 0);
    pUcePublishManager->SetIPublication(&objMockIPublication);
    pUcePublishManager->SetKey(10);

    EXPECT_CALL(objMockIUceJniThread, PublishResponseInd(_, _, _, _, _, _, _, _)).Times(1);

    MockISipMessage objMockISipMessage;
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));

    AString reason = "Not Implemented";
    ImsList<AString> objReasonHeaders;
    objReasonHeaders.Clear();

    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(501));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_FAILED, 0, 0);
    pUcePublishManager->PublishingFailed(objMsg);

    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, StatePUBLISHING_FailedWithNoKey)
{
    IMS_TRACE_D("StatePUBLISHING_FailedWithNoKey", 0, 0, 0);
    pUcePublishManager->SetIPublication(&objMockIPublication);

    EXPECT_CALL(objMockIUceJniThread, PublishUpdatedInd(_, _, _, _, _, _, _)).Times(1);

    MockISipMessage objMockISipMessage;
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));

    AString reason = "Not Implemented";
    ImsList<AString> objReasonHeaders;
    objReasonHeaders.Clear();

    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(501));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_FAILED, 0, 0);
    pUcePublishManager->PublishingFailed(objMsg);

    EXPECT_EQ(pUcePublishManager->GetKey(), 0);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, StatePUBLISHED_PublishRequested)
{
    IMS_TRACE_D("StatePUBLISHED_PublishRequested", 0, 0, 0);
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
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::PUBLISHING);
}

TEST_F(UcePublishManagerTest, StatePUBLISHED_AoSDisconnecting)
{
    IMS_TRACE_D("StatePUBLISHED_AoSDisconnecting", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, UnPublishedInd).Times(1);

    ON_CALL(objMockICoreService, CreatePublication(_, _, _)).WillByDefault(ReturnNull());

    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);
    pUcePublishManager->PublishedAoSDisconnecting(objMsg);

    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, StatePUBLISHED_AoSDisconnected)
{
    IMS_TRACE_D("StatePUBLISHED_AoSDisconnected", 0, 0, 0);
    EXPECT_CALL(objMockIUceJniThread, UnPublishedInd).Times(1);

    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);
    pUcePublishManager->PublishedAoSDisconnected(objMsg);

    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, StatePUBLISHED_RefreshStarted)
{
    IMS_TRACE_D("StatePUBLISHED_RefreshStarted", 0, 0, 0);

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REFRESH_STARTED, 0, 0);
    pUcePublishManager->PublishedRefreshStarted(objMsg);

    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::REFRESHING);
}

TEST_F(UcePublishManagerTest, StateREFRESHING_PublishRequested)
{
    IMS_TRACE_D("StateREFRESHING_PublishRequested", 0, 0, 0);

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

TEST_F(UcePublishManagerTest, StateREFRESHING_AoSDisConnecting)
{
    IMS_TRACE_D("StateREFRESHING_AoSDisConnecting", 0, 0, 0);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);

    pUcePublishManager->RefreshingAoSDisConnecting(objMsg);
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_TRUE);
}

TEST_F(UcePublishManagerTest, StateREFRESHING_AoSDisConnected)
{
    IMS_TRACE_D("StateREFRESHING_AoSDisConnected", 0, 0, 0);
    pUcePublishManager->UpdateState(TestUcePublishManager::REFRESHING);
    pUcePublishManager->SetReceivedUnPublishRequest(IMS_TRUE);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTED, 0, 0);

    pUcePublishManager->RefreshingAoSDisConnected(objMsg);
    EXPECT_EQ(pUcePublishManager->GetReceivedUnPublishRequest(), IMS_FALSE);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, StateREFRESHING_RefreshedFromPreviouslyReceivedUnpublish)
{
    IMS_TRACE_D("StateREFRESHING_RefreshedFromPreviouslyReceivedUnpublish", 0, 0, 0);
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
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::TERMINATING);
}

TEST_F(UcePublishManagerTest, StateREFRESHING_Refreshed)
{
    IMS_TRACE_D("StateREFRESHING_Refreshed", 0, 0, 0);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REFRESHED, 0, 0);

    pUcePublishManager->RefreshingRefreshed(objMsg);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::PUBLISHED);
}

TEST_F(UcePublishManagerTest, StateREFRESHING_RefreshFailed)
{
    IMS_TRACE_D("StateREFRESHING_RefreshFailed", 0, 0, 0);
    pUcePublishManager->SetIPublication(&objMockIPublication);

    EXPECT_CALL(objMockIUceJniThread, PublishUpdatedInd(_, _, _, _, _, _, _)).Times(1);

    MockISipMessage objMockISipMessage;
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));

    AString reason = "Not Implemented";
    ImsList<AString> objReasonHeaders;
    objReasonHeaders.Clear();

    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(501));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REFRESH_FAILED, 0, 0);

    pUcePublishManager->RefreshingRefreshFailed(objMsg);
}

TEST_F(UcePublishManagerTest, StateREFRESHING_RefreshFailedWithNoResponse)
{
    IMS_TRACE_D("StateREFRESHING_RefreshFailedWithNoResponse", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, PublishUpdatedInd(_, _, _, _, _, _, _)).Times(1);

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_REFRESH_NO_RESPONSE, 0, 0);

    pUcePublishManager->RefreshingRefreshFailedWithNoResponse(objMsg);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, StateTERMINATING_PublishRequested)
{
    IMS_TRACE_D("StateTERMINATING_PublishRequested", 0, 0, 0);
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

TEST_F(UcePublishManagerTest, StateTERMINATING_AoSDisconnected)
{
    IMS_TRACE_D("StateTERMINATING_AoSDisconnected", 0, 0, 0);
    pUcePublishManager->UpdateState(TestUcePublishManager::TERMINATING);
    IMSMSG objMsg(TestUcePublishManager::AOS_DISCONNECTING, 0, 0);

    pUcePublishManager->TerminatingAoSDisconnected(objMsg);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::IDLE);
}

TEST_F(UcePublishManagerTest, StateTERMINATING_Published)
{
    IMS_TRACE_D("StateTERMINATING_Published", 0, 0, 0);
    pUcePublishManager->UpdateState(TestUcePublishManager::TERMINATING);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_SUCCEEDED, 0, 0);

    pUcePublishManager->TerminatingPublished(objMsg);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, StateTERMINATING_Failed)
{
    IMS_TRACE_D("StateTERMINATING_Failed", 0, 0, 0);
    pUcePublishManager->UpdateState(TestUcePublishManager::TERMINATING);
    IMSMSG objMsg(TestUcePublishManager::PUBLISH_FAILED, 0, 0);

    pUcePublishManager->TerminatingFailed(objMsg);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::ON);
}

TEST_F(UcePublishManagerTest, StateALL_Terminated)
{
    IMS_TRACE_D("StateALL_Terminated", 0, 0, 0);
    pUcePublishManager->UpdateState(TestUcePublishManager::PUBLISHING);

    EXPECT_CALL(objMockIUceJniThread, PublishErrorInd(_, _)).Times(1);

    IMSMSG objMsg(TestUcePublishManager::PUBLISH_TERMINATED, 0, 0);

    pUcePublishManager->AllTerminated(objMsg);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::ON);

    pUcePublishManager->UpdateState(TestUcePublishManager::TERMINATING);

    EXPECT_CALL(objMockIUceJniThread, PublishUpdatedInd(_, _, _, _, _, _, _)).Times(1);

    IMSMSG objSecondMsg(TestUcePublishManager::PUBLISH_TERMINATED, 0, 0);

    pUcePublishManager->AllTerminated(objSecondMsg);
    EXPECT_EQ(pUcePublishManager->GetStateInternal(), TestUcePublishManager::ON);
}
