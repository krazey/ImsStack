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
#include "IUce.h"
#include "subscribe/UceSubscribe.h"
#include "subscribe/UceNonCapabilityUser.h"
#include "def/UceDef.h"
#include "config/UceConfig.h"
#include "config/UceAssetItems.h"
#include "MockIJniEnabler.h"
#include "MockIUceJniThread.h"
#include "JniEnablerConnector.h"
#include "MockICoreService.h"
#include "MockISubscription.h"
#include "MockIMessage.h"
#include "MockISipMessage.h"
#include "MockISipMessageBodyPart.h"
#include "MockITimer.h"
#include "TestTimerService.h"
#include "PlatformContext.h"
#include "SipAddress.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

__IMS_TRACE_TAG_USER_DECL__("UCE");

class TestUceSubscribe : public UceSubscribe
{
public:
    enum
    {
        SINGLE_REQUESTED = 1,
        LIST_REQUESTED,
        AOS_DISCONNECTED,
        SUBSCRIBE_SUCCEED,
        SUBSCRIBE_FAILED,
        RECEIVE_NOTIFIED,
        SUBSCRIBE_TERMINATED,
    };
    enum
    {
        QUERY_CAPABILITY_TYPE_NONE = 0,
        QUERY_CAPABILITY_TYPE_SINGLE,
        QUERY_CAPABILITY_TYPE_LIST,
    };

public:
    inline explicit TestUceSubscribe(ICoreService* piCoreService) :
            UceSubscribe(piCoreService, AString("AppName"), AString("UceSubscribeManager"), 0, 0)
    {
    }
    virtual ~TestUceSubscribe() {}

    IMS_UINT32 GetKey() const { return m_nKey; }
    void SetKey(IMS_UINT32 key) { m_nKey = key; }
    void SetRemoteUser(const AString& remoteUser) { m_strRemoteUser = remoteUser; }
    AString GetRemoteUser() const { return m_strRemoteUser; }
    void SetRemoteUsers(const ImsList<AString>& objUsers) { m_objRemoteUsers = objUsers; }
    ImsList<AString> GetRemoteUsers() const { return m_objRemoteUsers; }
    IMS_UINT32 GetState() const { return m_eState; }
    void setState(IMS_UINT32 state) { SetState(state); }
    void SetConectedService(IMS_UINT32 service) { m_nConnectedServices = service; }
    void SetWaitNotifyMsgTimer(ITimer* piTimer) { m_pWaitNotifyMsgTimer = piTimer; }
    void SetRetryAfterTimer(ITimer* piTimer) { m_pRetryAfterTimer = piTimer; }
    void SetISubscription(ISubscription* pSubscription) { m_piSubscription = pSubscription; }
    void SetQueryType(IMS_UINT32 type) { m_eQueryType = type; }
    void SetThreadRunningCompleted(IMS_UINT32 count) { m_nThreadRunningCompleted = count; }
    void SetSubscriptionTerminated(IMS_BOOL bTerminated)
    {
        m_bSubscriptionTerminated = bTerminated;
    }

    IMS_RESULT MessageMediatorAdjustMessage(ISipMessage* piSIPMsg)
    {
        return MessageMediator_AdjustMessage(piSIPMsg, 0);
    }

    void TimerExpired(ITimer* piTimer) { Timer_TimerExpired(piTimer); }

    void ForkedNotify(ISubscription* piSubscription, ISubscription* piForkedSubscription)
    {
        SubscriptionForkedNotify(piSubscription, piForkedSubscription);
    }

    void Notify(ISubscription* piSubscription, IMessage* piNotify)
    {
        SubscriptionNotify(piSubscription, piNotify);
    }

    void Started(ISubscription* piSubscription) { SubscriptionStarted(piSubscription); }

    void StartFailed(ISubscription* piSubscription) { SubscriptionStartFailed(piSubscription); }

    void Terminated(ISubscription* piSubscription) { SubscriptionTerminated(piSubscription); }

    IMS_BOOL sendMsg(IN IMSMSG& objMsg) { return OnMessage(objMsg); }

    IMS_BOOL onSingleSubscribeRequested(IMSMSG& objMsg)
    {
        return StateON_SingleSubscribeRequested(objMsg);
    }
    IMS_BOOL onListSubscribeRequested(IN IMSMSG& objMsg)
    {
        return StateON_ListSubscribeRequested(objMsg);
    }
    IMS_BOOL subscribingAoSDisConnected(IN IMSMSG& objMsg)
    {
        return StateSUBSCRIBING_AoSDisConnected(objMsg);
    }
    IMS_BOOL subscribingSubscribed(IN IMSMSG& objMsg)
    {
        return StateSUBSCRIBING_Subscribed(objMsg);
    }
    IMS_BOOL subscribingSubscribeFailed(IN IMSMSG& objMsg)
    {
        return StateSUBSCRIBING_SubscribeFailed(objMsg);
    }
    IMS_BOOL subscribingSubscribeTerminated(IN IMSMSG& objMsg)
    {
        return StateSUBSCRIBING_SubscribeTerminated(objMsg);
    }
    IMS_BOOL subscribingNotifyReceived(IN IMSMSG& objMsg)
    {
        return StateSUBSCRIBING_NotifyReceived(objMsg);
    }
    IMS_BOOL subscribedAoSDisConnected(IN IMSMSG& objMsg)
    {
        return StateSUBSCRIBED_AoSDisConnected(objMsg);
    }
    IMS_BOOL subscribedSubscribeTerminated(IN IMSMSG& objMsg)
    {
        return StateSUBSCRIBED_SubscribeTerminated(objMsg);
    }
    IMS_BOOL subscribedNotifyReceived(IN IMSMSG& objMsg)
    {
        return StateSUBSCRIBED_NotifyReceived(objMsg);
    }
};

class UceSubscribeTest : public ::testing::Test
{
public:
    inline UceSubscribeTest() :
            objTimerService(),
            objTimer(objTimerService.GetMockTimer())
    {
        pUceSubscribe = IMS_NULL;
    }
    TestTimerService objTimerService;
    MockITimer& objTimer;
    TestUceSubscribe* pUceSubscribe;
    MockICoreService objMockICoreService;
    MockIJniEnabler objMockJniEnabler;
    MockIUceJniThread objMockIUceJniThread;
    MockISubscription objMockISubscription;

protected:
    virtual void SetUp() override
    {
        pUceSubscribe = new TestUceSubscribe(&objMockICoreService);
        ASSERT_TRUE(pUceSubscribe != nullptr);
        ON_CALL(objMockJniEnabler, GetJniThread()).WillByDefault(Return(&objMockIUceJniThread));
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, &objMockJniEnabler);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);
    }

    virtual void TearDown() override
    {
        if (pUceSubscribe)
        {
            delete pUceSubscribe;
        }
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }
};

TEST_F(UceSubscribeTest, MessageMediator_AdjustMessage)
{
    IMS_TRACE_D("MessageMediator_AdjustMessage", 0, 0, 0);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(IMS_NULL), IMS_SUCCESS);

    MockISipMessage objMockISipMessage;

    SipMethod objSubscribeMethod = SipMethod::PUBLISH;

    ON_CALL(objMockISipMessage, GetMethod).WillByDefault(ReturnRef(objSubscribeMethod));
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    objSubscribeMethod = SipMethod::SUBSCRIBE;
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);
}

TEST_F(UceSubscribeTest, SetContactHeader)
{
    IMS_TRACE_D("SetContactHeader", 0, 0, 0);
    MockISipMessage objMockISipMessage;

    SipMethod objSubscribeMethod = SipMethod::SUBSCRIBE;
    ON_CALL(objMockISipMessage, GetMethod).WillByDefault(ReturnRef(objSubscribeMethod));
    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString::ConstEmpty()));

    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    UceAssetItems* objNew = new UceAssetItems();
    // KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE
    objNew->m_bUseContactHeaderInSubscribe = IMS_TRUE;
    UceConfig::GetInstance()->SetConfig(0, objNew);

    pUceSubscribe->SetConectedService(0);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    EXPECT_CALL(objMockISipMessage, SetHeader).Times(AnyNumber());

    pUceSubscribe->SetConectedService(CONNECTED_SERVICE_CPM_SESSION);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUceSubscribe->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CALL_COMPOSER);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUceSubscribe->SetConectedService(CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CPM_MSG);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUceSubscribe->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CPM_LARGEMSG);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_PRESENCE)));
    pUceSubscribe->SetConectedService(CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_PRESENCE);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUceSubscribe->SetConectedService(CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_HTTPFT);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUceSubscribe->SetConectedService(CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_GEOPUSH);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUceSubscribe->SetConectedService(CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_FTSMS);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUceSubscribe->SetConectedService(CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_GEOSMS);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUceSubscribe->SetConectedService(CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CHATBOT);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString(UceTag::TAG_CHAT)));
    pUceSubscribe->SetConectedService(
            CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    pUceSubscribe->SetConectedService(CONNECTED_SERVICE_CHATBOT_V1 | CONNECTED_SERVICE_CPM_SESSION);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    pUceSubscribe->SetConectedService(CONNECTED_SERVICE_CHATBOT_V2 | CONNECTED_SERVICE_CPM_SESSION);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);

    IMS_UINT32 service = CONNECTED_SERVICE_CPM_SESSION | CONNECTED_SERVICE_CALL_COMPOSER |
            CONNECTED_SERVICE_CPM_MSG | CONNECTED_SERVICE_CPM_LARGEMSG |
            CONNECTED_SERVICE_PRESENCE | CONNECTED_SERVICE_HTTPFT | CONNECTED_SERVICE_GEOPUSH |
            CONNECTED_SERVICE_FTSMS | CONNECTED_SERVICE_GEOSMS | CONNECTED_SERVICE_CHATBOT |
            CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG | CONNECTED_SERVICE_CHATBOT_V1 |
            CONNECTED_SERVICE_CHATBOT_V2;
    pUceSubscribe->SetConectedService(service);
    EXPECT_EQ(pUceSubscribe->MessageMediatorAdjustMessage(&objMockISipMessage), IMS_SUCCESS);
}

TEST_F(UceSubscribeTest, QuerySingleCapability)
{
    IMS_TRACE_D("QuerySingleCapability", 0, 0, 0);
    AString strUser = AString("tel:+123456789");
    IMS_UINT32 key = 10;
    EXPECT_EQ(pUceSubscribe->QuerySingleCapability(strUser, key), IMS_TRUE);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
    EXPECT_STREQ(pUceSubscribe->GetRemoteUser().GetStr(), strUser.GetStr());
}

TEST_F(UceSubscribeTest, QueryMultiCapability)
{
    IMS_TRACE_D("QueryMultiCapability", 0, 0, 0);
    TestUceSubscribe* pUceSubscribe = new TestUceSubscribe(IMS_NULL);
    AString strUser1 = AString("tel:+123456");
    AString strUser2 = AString("tel:+123457");
    AString strUser3 = AString("tel:+123458");
    AString strUser4 = AString("tel:+123459");

    ImsList<AString> objUsers = ImsList<AString>();
    objUsers.Append(strUser1);
    objUsers.Append(strUser2);
    objUsers.Append(strUser3);
    objUsers.Append(strUser4);

    IMS_UINT32 key = 10;
    EXPECT_EQ(pUceSubscribe->QueryMultiCapability(objUsers, key), IMS_TRUE);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
    EXPECT_EQ(pUceSubscribe->GetRemoteUsers().GetSize(), objUsers.GetSize());
    for (IMS_UINT32 i = 0; i < pUceSubscribe->GetRemoteUsers().GetSize(); i++)
    {
        EXPECT_STREQ(pUceSubscribe->GetRemoteUsers().GetValueAt(i).GetStr(),
                objUsers.GetValueAt(i).GetStr());
    }

    delete pUceSubscribe;
}

TEST_F(UceSubscribeTest, AosDisConnected)
{
    IMS_TRACE_D("AosDisConnected", 0, 0, 0);
    EXPECT_TRUE(pUceSubscribe->AosDisConnected());
}

TEST_F(UceSubscribeTest, TimerExpired)
{
    IMS_TRACE_D("TimerExpired", 0, 0, 0);
    pUceSubscribe->TimerExpired(IMS_NULL);

    EXPECT_CALL(objTimer, KillTimer).Times(4);

    EXPECT_CALL(objMockIUceJniThread, SubscribeResponseInd(_, _, _, _, _)).Times(1);
    EXPECT_CALL(objMockIUceJniThread, SubscribeTerminatedInd(_, _, _)).Times(2);

    pUceSubscribe->SetKey(1);
    pUceSubscribe->setState(UceSubscribe::SUBSCRIBING);
    pUceSubscribe->SetWaitNotifyMsgTimer(&objTimer);
    pUceSubscribe->TimerExpired(&objTimer);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);

    pUceSubscribe->SetKey(1);
    pUceSubscribe->setState(UceSubscribe::SUBSCRIBED);
    pUceSubscribe->SetWaitNotifyMsgTimer(&objTimer);
    pUceSubscribe->TimerExpired(&objTimer);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);

    pUceSubscribe->setState(UceSubscribe::SUBSCRIBED);
    pUceSubscribe->SetQueryType(TestUceSubscribe::QUERY_CAPABILITY_TYPE_SINGLE);
    pUceSubscribe->SetRetryAfterTimer(&objTimer);
    pUceSubscribe->TimerExpired(&objTimer);
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);

    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
    pUceSubscribe->setState(UceSubscribe::SUBSCRIBED);
    pUceSubscribe->SetQueryType(TestUceSubscribe::QUERY_CAPABILITY_TYPE_LIST);
    pUceSubscribe->SetRetryAfterTimer(&objTimer);
    pUceSubscribe->TimerExpired(&objTimer);
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, sendMsg)
{
    IMS_TRACE_D("sendMsg", 0, 0, 0);

    IMSMSG objMsg(TestUceSubscribe::SINGLE_REQUESTED, 0, 0);
    EXPECT_FALSE(pUceSubscribe->sendMsg(objMsg));

    EXPECT_CALL(objMockIUceJniThread, NotifyInd(_, _, _)).Times(1);
    EXPECT_CALL(objMockIUceJniThread, SubscribeResourceTerminatedInd(_, _, _)).Times(1);
    EXPECT_CALL(objMockIUceJniThread, SubscribeTerminatedInd(_, _, _)).Times(1);

    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetThreadRunningCompleted(1);
    pUceSubscribe->SetSubscriptionTerminated(IMS_TRUE);
    UceNonCapabilityUser* pUceNonCapabilityUser = new UceNonCapabilityUser("id", "reason");
    UceNonCapabilityUsers* pUceNonCapabilityUsers = new UceNonCapabilityUsers();
    pUceNonCapabilityUsers->SetNonCapabilityUser(pUceNonCapabilityUser);

    UcePidfXmls* pUcePidfXmls = new UcePidfXmls();
    pUcePidfXmls->SetPidfXml("testPidf");

    IMSMSG objSecondMsg(IUUceService::UCE_XML_PARSE_COMPLETED_IND,
            reinterpret_cast<IMS_UINTP>(pUceNonCapabilityUsers),
            reinterpret_cast<IMS_UINTP>(pUcePidfXmls));
    EXPECT_TRUE(pUceSubscribe->sendMsg(objSecondMsg));
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
}

TEST_F(UceSubscribeTest, SubscriptionForkedNotify)
{
    IMS_TRACE_D("SubscriptionForkedNotify", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    pUceSubscribe->ForkedNotify(&objMockISubscription, &objMockISubscription);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);
}

TEST_F(UceSubscribeTest, SubscriptionNotify)
{
    IMS_TRACE_D("SubscriptionNotify", 0, 0, 0);
    MockIMessage obMockIMessage;
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    pUceSubscribe->Notify(&objMockISubscription, &obMockIMessage);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);

    pUceSubscribe->SetISubscription(&objMockISubscription);
    pUceSubscribe->Notify(&objMockISubscription, &obMockIMessage);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);
}

TEST_F(UceSubscribeTest, SubscriptionStarted)
{
    IMS_TRACE_D("SubscriptionStarted", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    pUceSubscribe->Started(&objMockISubscription);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);

    pUceSubscribe->SetISubscription(&objMockISubscription);
    pUceSubscribe->Started(&objMockISubscription);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);
}

TEST_F(UceSubscribeTest, SubscriptionStartFailed)
{
    IMS_TRACE_D("SubscriptionStartFailed", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    pUceSubscribe->StartFailed(&objMockISubscription);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeErrorInd(_, _)).Times(2);

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetQueryType(TestUceSubscribe::QUERY_CAPABILITY_TYPE_SINGLE);
    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(ReturnNull());
    pUceSubscribe->SetISubscription(&objMockISubscription);
    pUceSubscribe->StartFailed(&objMockISubscription);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);

    MockIMessage obMockIMessage;
    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(Return(&obMockIMessage));
    ON_CALL(obMockIMessage, GetMessage()).WillByDefault(ReturnNull());

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetQueryType(TestUceSubscribe::QUERY_CAPABILITY_TYPE_SINGLE);
    pUceSubscribe->SetISubscription(&objMockISubscription);
    pUceSubscribe->StartFailed(&objMockISubscription);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);

    MockISipMessage objMockISipMessage;
    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(Return(&obMockIMessage));
    ON_CALL(obMockIMessage, GetMessage()).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(408));

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetQueryType(TestUceSubscribe::QUERY_CAPABILITY_TYPE_SINGLE);
    pUceSubscribe->SetISubscription(&objMockISubscription);
    pUceSubscribe->StartFailed(&objMockISubscription);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);
}

TEST_F(UceSubscribeTest, StateON_SingleSubscribeRequested)
{
    IMS_TRACE_D("StateON_SingleSubscribeRequested", 0, 0, 0);
    IMS_UINT32 key = 10;
    AString remoteUri = AString("+123456789");
    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetRemoteUser(remoteUri);

    EXPECT_CALL(objMockISubscription, SetListener(_)).Times(3);
    EXPECT_CALL(objMockIUceJniThread, SubscribeErrorInd(_, _)).Times(3);

    ON_CALL(objMockICoreService, CreateSubscription(_, _, _))
            .WillByDefault(Return(&objMockISubscription));
    ON_CALL(objMockISubscription, GetNextRequest()).WillByDefault(ReturnNull());

    IMSMSG objMsg(TestUceSubscribe::SINGLE_REQUESTED, 0, 0);
    pUceSubscribe->onSingleSubscribeRequested(objMsg);

    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
    EXPECT_STREQ(
            pUceSubscribe->GetRemoteUser().GetStr(), AString("tel:").Append(remoteUri).GetStr());

    MockIMessage objMockIMessage;
    ON_CALL(objMockICoreService, CreateSubscription(_, _, _))
            .WillByDefault(Return(&objMockISubscription));
    ON_CALL(objMockISubscription, GetNextRequest()).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(ReturnNull());

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetRemoteUser(remoteUri);
    pUceSubscribe->onSingleSubscribeRequested(objMsg);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
    EXPECT_STREQ(
            pUceSubscribe->GetRemoteUser().GetStr(), AString("tel:").Append(remoteUri).GetStr());

    MockISipMessage objMockISipMessage;
    ON_CALL(objMockICoreService, CreateSubscription(_, _, _))
            .WillByDefault(Return(&objMockISubscription));
    ON_CALL(objMockISubscription, GetNextRequest()).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(Return(&objMockISipMessage));

    EXPECT_CALL(objMockISubscription, SetRefreshPolicy).Times(1);
    EXPECT_CALL(objMockISubscription, Poll).Times(1).WillRepeatedly(Return(IMS_FAILURE));

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetRemoteUser(remoteUri);
    pUceSubscribe->onSingleSubscribeRequested(objMsg);

    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
    EXPECT_STREQ(
            pUceSubscribe->GetRemoteUser().GetStr(), AString("tel:").Append(remoteUri).GetStr());
}

TEST_F(UceSubscribeTest, StateON_SingleSubscribeRequestedWithoutRemoteUri)
{
    IMS_TRACE_D("StateON_SingleSubscribeRequestedWithoutRemoteUri", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeErrorInd(_, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::SINGLE_REQUESTED, 0, 0);
    pUceSubscribe->onSingleSubscribeRequested(objMsg);

    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
}

TEST_F(UceSubscribeTest, StateON_ListSubscribeRequested)
{
    IMS_TRACE_D("StateON_ListSubscribeRequested", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, SubscribeErrorInd(_, _)).Times(4);
    EXPECT_CALL(objMockISubscription, SetListener(_)).Times(4);

    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    IMSMSG objMsg(TestUceSubscribe::LIST_REQUESTED, 0, 0);
    pUceSubscribe->onListSubscribeRequested(objMsg);

    pUceSubscribe->SetKey(key);
    ImsList<AString> objUsers = ImsList<AString>();
    AString strUser1 = AString("tel:+123456");
    objUsers.Append(strUser1);
    pUceSubscribe->SetRemoteUsers(objUsers);

    SipAddress objUserId("sip:123456@test.ims.com");
    MockIMessage objMockIMessage;

    ON_CALL(objMockICoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objUserId));
    ON_CALL(objMockICoreService, CreateSubscription(_, _, _))
            .WillByDefault(Return(&objMockISubscription));
    ON_CALL(objMockISubscription, GetNextRequest()).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(ReturnNull());
    pUceSubscribe->onListSubscribeRequested(objMsg);

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetRemoteUsers(objUsers);
    MockISipMessage objMockISipMessage;
    ON_CALL(objMockICoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objUserId));
    ON_CALL(objMockICoreService, CreateSubscription(_, _, _))
            .WillByDefault(Return(&objMockISubscription));
    ON_CALL(objMockISubscription, GetNextRequest()).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, CreateBodyPart()).WillByDefault(ReturnNull());

    EXPECT_CALL(objMockISipMessage, SetHeader).Times(AnyNumber());
    EXPECT_CALL(objMockISipMessage, AddHeader).Times(AnyNumber());
    pUceSubscribe->onListSubscribeRequested(objMsg);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);

    ON_CALL(objMockICoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objUserId));
    ON_CALL(objMockICoreService, CreateSubscription(_, _, _))
            .WillByDefault(Return(&objMockISubscription));
    ON_CALL(objMockISubscription, GetNextRequest()).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(ReturnNull());
    pUceSubscribe->onListSubscribeRequested(objMsg);

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetRemoteUsers(objUsers);
    MockISipMessageBodyPart objMockISipMessageBodyPart;
    ON_CALL(objMockICoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objUserId));
    ON_CALL(objMockICoreService, CreateSubscription(_, _, _))
            .WillByDefault(Return(&objMockISubscription));
    ON_CALL(objMockISubscription, GetNextRequest()).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage()).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, CreateBodyPart())
            .WillByDefault(Return(&objMockISipMessageBodyPart));

    EXPECT_CALL(objMockISipMessageBodyPart, SetContent).Times(AnyNumber());
    EXPECT_CALL(objMockISubscription, SetRefreshPolicy).Times(1);
    EXPECT_CALL(objMockISubscription, Subscribe).Times(1).WillRepeatedly(Return(IMS_FAILURE));

    pUceSubscribe->onListSubscribeRequested(objMsg);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
}

TEST_F(UceSubscribeTest, StateSUBSCRIBING_AoSDisConnected)
{
    IMS_TRACE_D("StateSUBSCRIBING_AoSDisConnected", 0, 0, 0);

    EXPECT_CALL(objMockIUceJniThread, SubscribeErrorInd(_, _)).Times(1);

    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    IMSMSG objMsg(TestUceSubscribe::AOS_DISCONNECTED, 0, 0);
    EXPECT_TRUE(pUceSubscribe->subscribingAoSDisConnected(objMsg));

    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, StateSUBSCRIBING_Subscribed)
{
    IMS_TRACE_D("StateSUBSCRIBING_Subscribed", 0, 0, 0);
    EXPECT_CALL(objMockIUceJniThread, SubscribeResponseInd(_, _, _, _, _)).Times(3);

    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetISubscription(&objMockISubscription);

    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(ReturnNull());

    IMSMSG objMsg(TestUceSubscribe::SUBSCRIBE_SUCCEED, 0, 0);
    pUceSubscribe->subscribingSubscribed(objMsg);
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::SUBSCRIBED);

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetISubscription(&objMockISubscription);
    MockIMessage objMockIMessage;
    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(ReturnNull());

    pUceSubscribe->subscribingSubscribed(objMsg);

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetISubscription(&objMockISubscription);
    MockISipMessage objMockISipMessage;
    AString reason = "OK";
    ImsList<AString> objReasonHeaders;
    objReasonHeaders.Clear();
    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return(AString::ConstEmpty()));
    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(200));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));
    pUceSubscribe->subscribingSubscribed(objMsg);
}

TEST_F(UceSubscribeTest, StateSUBSCRIBING_SubscribeFailed)
{
    IMS_TRACE_D("StateSUBSCRIBING_SubscribeFailed", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetISubscription(&objMockISubscription);

    EXPECT_CALL(objMockIUceJniThread, SubscribeResponseInd(_, _, _, _, _)).Times(5);

    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(ReturnNull());

    IMSMSG objMsg(TestUceSubscribe::SUBSCRIBE_FAILED, 404, 0);
    EXPECT_TRUE(pUceSubscribe->subscribingSubscribeFailed(objMsg));
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetISubscription(&objMockISubscription);
    MockIMessage objMockIMessage;
    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(ReturnNull());
    EXPECT_TRUE(pUceSubscribe->subscribingSubscribeFailed(objMsg));

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetISubscription(&objMockISubscription);
    MockISipMessage objMockISipMessage;
    AString reason = "OK";
    ImsList<AString> objReasonHeaders;
    objReasonHeaders.Clear();
    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return("0"));
    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(404));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));
    EXPECT_TRUE(pUceSubscribe->subscribingSubscribeFailed(objMsg));

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetISubscription(&objMockISubscription);
    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return("0"));
    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(423));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));
    IMSMSG objMsg1(TestUceSubscribe::SUBSCRIBE_FAILED, 423, 0);
    EXPECT_TRUE(pUceSubscribe->subscribingSubscribeFailed(objMsg1));

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetISubscription(&objMockISubscription);
    reason = "NOT AUTHORIZED FOR PRESENCE";
    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(403));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));
    IMSMSG objMsg2(TestUceSubscribe::SUBSCRIBE_FAILED, 403, 0);
    EXPECT_TRUE(pUceSubscribe->subscribingSubscribeFailed(objMsg2));

    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetISubscription(&objMockISubscription);
    reason = "OK";
    ON_CALL(objMockISubscription, GetPreviousResponse).WillByDefault(Return(&objMockIMessage));
    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetStatusCode).WillByDefault(Return(403));
    ON_CALL(objMockISipMessage, GetReasonPhrase).WillByDefault(ReturnRef(reason));
    ON_CALL(objMockISipMessage, GetHeaders).WillByDefault(Return(objReasonHeaders));
    IMSMSG objMsg3(TestUceSubscribe::SUBSCRIBE_FAILED, 403, 0);
    EXPECT_TRUE(pUceSubscribe->subscribingSubscribeFailed(objMsg3));
}

TEST_F(UceSubscribeTest, StateSUBSCRIBING_SubscribeTerminated)
{
    IMS_TRACE_D("StateSUBSCRIBING_SubscribeTerminated", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeResponseInd(_, _, _, _, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::SUBSCRIBE_TERMINATED, 0, 0);
    pUceSubscribe->subscribingSubscribeTerminated(objMsg);

    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, StateSUBSCRIBING_NotifyReceived)
{
    IMS_TRACE_D("StateSUBSCRIBING_NotifyReceived", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    IMSMSG objMsg(TestUceSubscribe::RECEIVE_NOTIFIED, 0, 0);
    EXPECT_TRUE(pUceSubscribe->subscribingNotifyReceived(objMsg));
}

TEST_F(UceSubscribeTest, StateSUBSCRIBING_NotifyReceivedWithNotifyBody)
{
    IMS_TRACE_D("StateSUBSCRIBING_NotifyReceivedWithNotifyBody", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, NotifyInd(_, _, _)).Times(1);

    MockIMessage objMockIMessage;
    MockISipMessage objMockISipMessage;
    MockISipMessageBodyPart objISipMessageBodyPart;
    ImsList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Append(&objISipMessageBodyPart);
    ByteArray objContent = ByteArray("test");

    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetBodyParts).WillByDefault(Return(objBodyParts));
    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return("application/pidf+xml"));
    ON_CALL(objISipMessageBodyPart, GetContent).WillByDefault(ReturnRef(objContent));

    IMSMSG objMsg(
            TestUceSubscribe::RECEIVE_NOTIFIED, reinterpret_cast<IMS_UINTP>(&objMockIMessage), 0);
    pUceSubscribe->subscribingNotifyReceived(objMsg);
}

TEST_F(UceSubscribeTest, StateSUBSCRIBED_AoSDisConnected)
{
    IMS_TRACE_D("StateSUBSCRIBED_AoSDisConnected", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeTerminatedInd(_, _, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::AOS_DISCONNECTED, 0, 0);
    pUceSubscribe->subscribedAoSDisConnected(objMsg);

    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, StateSUBSCRIBED_SubscribeTerminated)
{
    IMS_TRACE_D("StateSUBSCRIBED_SubscribeTerminated", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeTerminatedInd(_, _, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::SUBSCRIBE_TERMINATED, 0, 0);
    pUceSubscribe->subscribedSubscribeTerminated(objMsg);

    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, StateSUBSCRIBED_NotifyReceived)
{
    IMS_TRACE_D("subscribedNotifyReceived", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeResponseInd(_, _, _, _, _)).Times(0);

    IMSMSG objMsg(TestUceSubscribe::RECEIVE_NOTIFIED, 0, 0);
    EXPECT_TRUE(pUceSubscribe->subscribedNotifyReceived(objMsg));
}

TEST_F(UceSubscribeTest, StateSUBSCRIBED_NotifyReceivedWithNotifyBody)
{
    IMS_TRACE_D("StateSUBSCRIBED_NotifyReceivedWithNotifyBody", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, NotifyInd(_, _, _)).Times(1);

    MockIMessage objMockIMessage;
    MockISipMessage objMockISipMessage;
    MockISipMessageBodyPart objISipMessageBodyPart;
    ImsList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Append(&objISipMessageBodyPart);
    ByteArray objContent = ByteArray("test");

    ON_CALL(objMockIMessage, GetMessage).WillByDefault(ReturnNull());
    IMSMSG objMsg(
            TestUceSubscribe::RECEIVE_NOTIFIED, reinterpret_cast<IMS_UINTP>(&objMockIMessage), 0);
    EXPECT_TRUE(pUceSubscribe->subscribedNotifyReceived(objMsg));

    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetBodyParts).WillByDefault(Return(objBodyParts));
    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return("application/pidf+xml"));
    ON_CALL(objISipMessageBodyPart, GetContent).WillByDefault(ReturnRef(objContent));

    pUceSubscribe->subscribedNotifyReceived(objMsg);
}

TEST_F(UceSubscribeTest, setState)
{
    IMS_TRACE_D("setState", 0, 0, 0);
    pUceSubscribe->setState(UceSubscribe::ON);
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}