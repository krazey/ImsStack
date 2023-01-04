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
#include "subscribe/UceSubscribe.h"
#include "MockIJniEnabler.h"
#include "MockIUceJniThread.h"
#include "JniEnablerConnector.h"
#include "MockICoreService.h"
#include "MockISubscription.h"
#include "MockIMessage.h"
#include "MockISipMessage.h"
#include "MockISipMessageBodyPart.h"
#include "SipAddress.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::Return;
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

    void SetRemoteUsers(const IMSList<AString>& objUsers) { m_objRemoteUsers = objUsers; }

    IMSList<AString> GetRemoteUsers() const { return m_objRemoteUsers; }

    IMS_UINT32 GetState() const { return m_eState; }

    void setState(IMS_UINT32 state) { SetState(state); }

    IMS_BOOL onMessage(IN IMSMSG& objMsg) { return OnMessage(objMsg); }

    void setISubscription(ISubscription* pSubscription) { m_piSubscription = pSubscription; }

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
    }

    virtual void TearDown() override
    {
        if (pUceSubscribe)
        {
            delete pUceSubscribe;
        }
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, IMS_NULL);
    }
};

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

    IMSList<AString> objUsers = IMSList<AString>();
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

TEST_F(UceSubscribeTest, onMessage)
{
    IMS_TRACE_D("onMessage", 0, 0, 0);
    IMSMSG objMsg(TestUceSubscribe::SINGLE_REQUESTED, 0, IMS_NULL);

    EXPECT_FALSE(pUceSubscribe->onMessage(objMsg));
}

TEST_F(UceSubscribeTest, onSingleSubscribeRequested)
{
    IMS_TRACE_D("onSingleSubscribeRequested", 0, 0, 0);
    IMS_UINT32 key = 10;
    AString remoteUri = AString("+123456789");
    pUceSubscribe->SetKey(key);
    pUceSubscribe->SetRemoteUser(remoteUri);

    ON_CALL(objMockICoreService, CreateSubscription(_, _, _))
            .WillByDefault(Return(&objMockISubscription));
    EXPECT_CALL(objMockISubscription, SetListener(_)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::SINGLE_REQUESTED, 0, IMS_NULL);
    pUceSubscribe->onSingleSubscribeRequested(objMsg);

    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
    EXPECT_STREQ(
            pUceSubscribe->GetRemoteUser().GetStr(), AString("tel:").Append(remoteUri).GetStr());
}

TEST_F(UceSubscribeTest, onSingleSubscribeRequestedWithoutRemoteUri)
{
    IMS_TRACE_D("onSingleSubscribeRequestedWithoutRemoteUri", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeErrorInd(_, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::SINGLE_REQUESTED, 0, IMS_NULL);
    pUceSubscribe->onSingleSubscribeRequested(objMsg);

    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
}

TEST_F(UceSubscribeTest, onListSubscribeRequested)
{
    IMS_TRACE_D("onListSubscribeRequested", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    IMSList<AString> objUsers = IMSList<AString>();
    AString strUser1 = AString("tel:+123456");
    objUsers.Append(strUser1);
    pUceSubscribe->SetRemoteUsers(objUsers);

    EXPECT_EQ(pUceSubscribe->GetKey(), key);

    SipAddress objUserId("sip:123456@test.ims.com");
    ON_CALL(objMockICoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objUserId));
    ON_CALL(objMockICoreService, CreateSubscription(_, _, _))
            .WillByDefault(Return(&objMockISubscription));

    EXPECT_CALL(objMockISubscription, SetListener(_)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::LIST_REQUESTED, 0, IMS_NULL);
    pUceSubscribe->onListSubscribeRequested(objMsg);

    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
}

TEST_F(UceSubscribeTest, onListSubscribeRequestedWithoutRemoteUris)
{
    IMS_TRACE_D("onListSubscribeRequestedWithoutRemoteUris", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_EQ(pUceSubscribe->GetKey(), key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeErrorInd(_, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::LIST_REQUESTED, 0, IMS_NULL);
    pUceSubscribe->onListSubscribeRequested(objMsg);

    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
}

TEST_F(UceSubscribeTest, subscribingAoSDisConnected)
{
    IMS_TRACE_D("subscribingAoSDisConnected", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeErrorInd(_, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::AOS_DISCONNECTED, 0, IMS_NULL);
    pUceSubscribe->subscribingAoSDisConnected(objMsg);

    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, subscribingSubscribed)
{
    IMS_TRACE_D("subscribingSubscribed", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    pUceSubscribe->setISubscription(&objMockISubscription);

    EXPECT_CALL(objMockIUceJniThread, SubscribeResponseInd(_, _, _, _, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::SUBSCRIBE_SUCCEED, 0, IMS_NULL);
    pUceSubscribe->subscribingSubscribed(objMsg);

    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::SUBSCRIBED);
}

TEST_F(UceSubscribeTest, subscribingSubscribeFailed)
{
    IMS_TRACE_D("subscribingSubscribeFailed", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);
    pUceSubscribe->setISubscription(&objMockISubscription);

    EXPECT_CALL(objMockIUceJniThread, SubscribeResponseInd(_, _, _, _, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::SUBSCRIBE_FAILED, 0, IMS_NULL);
    pUceSubscribe->subscribingSubscribeFailed(objMsg);

    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, subscribingSubscribeTerminated)
{
    IMS_TRACE_D("subscribingSubscribeTerminated", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeResponseInd(_, _, _, _, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::SUBSCRIBE_TERMINATED, 0, IMS_NULL);
    pUceSubscribe->subscribingSubscribeTerminated(objMsg);

    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, subscribingNotifyReceived)
{
    IMS_TRACE_D("subscribingNotifyReceived", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeResponseInd(_, _, _, _, _)).Times(0);

    IMSMSG objMsg(TestUceSubscribe::RECEIVE_NOTIFIED, 0, IMS_NULL);
    pUceSubscribe->subscribingNotifyReceived(objMsg);
}

TEST_F(UceSubscribeTest, subscribingNotifyReceivedWithNotifyBody)
{
    IMS_TRACE_D("subscribingNotifyReceivedWithNotifyBody", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, NotifyInd(_, _, _)).Times(1);

    MockIMessage objMockIMessage;
    MockISipMessage objMockISipMessage;
    MockISipMessageBodyPart objISipMessageBodyPart;
    IMSList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Append(&objISipMessageBodyPart);
    ByteArray objContent = ByteArray("test");

    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetBodyParts).WillByDefault(Return(objBodyParts));
    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return("application/pidf+xml"));
    ON_CALL(objISipMessageBodyPart, GetContent).WillByDefault(ReturnRef(objContent));

    IMSMSG objMsg(TestUceSubscribe::RECEIVE_NOTIFIED, reinterpret_cast<IMS_UINTP>(&objMockIMessage),
            IMS_NULL);
    pUceSubscribe->subscribingNotifyReceived(objMsg);
}

TEST_F(UceSubscribeTest, subscribedAoSDisConnected)
{
    IMS_TRACE_D("subscribedAoSDisConnected", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeTerminatedInd(_, _, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::AOS_DISCONNECTED, 0, IMS_NULL);
    pUceSubscribe->subscribedAoSDisConnected(objMsg);

    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, subscribedSubscribeTerminated)
{
    IMS_TRACE_D("subscribedSubscribeTerminated", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeTerminatedInd(_, _, _)).Times(1);

    IMSMSG objMsg(TestUceSubscribe::SUBSCRIBE_TERMINATED, 0, IMS_NULL);
    pUceSubscribe->subscribedSubscribeTerminated(objMsg);

    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, subscribedNotifyReceived)
{
    IMS_TRACE_D("subscribedNotifyReceived", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, SubscribeResponseInd(_, _, _, _, _)).Times(0);

    IMSMSG objMsg(TestUceSubscribe::RECEIVE_NOTIFIED, 0, IMS_NULL);
    pUceSubscribe->subscribedNotifyReceived(objMsg);
}

TEST_F(UceSubscribeTest, subscribedNotifyReceivedWithNotifyBody)
{
    IMS_TRACE_D("subscribedNotifyReceivedWithNotifyBody", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->SetKey(key);

    EXPECT_CALL(objMockIUceJniThread, NotifyInd(_, _, _)).Times(1);

    MockIMessage objMockIMessage;
    MockISipMessage objMockISipMessage;
    MockISipMessageBodyPart objISipMessageBodyPart;
    IMSList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Append(&objISipMessageBodyPart);
    ByteArray objContent = ByteArray("test");

    ON_CALL(objMockIMessage, GetMessage).WillByDefault(Return(&objMockISipMessage));
    ON_CALL(objMockISipMessage, GetBodyParts).WillByDefault(Return(objBodyParts));
    ON_CALL(objMockISipMessage, GetHeader).WillByDefault(Return("application/pidf+xml"));
    ON_CALL(objISipMessageBodyPart, GetContent).WillByDefault(ReturnRef(objContent));

    IMSMSG objMsg(TestUceSubscribe::RECEIVE_NOTIFIED, reinterpret_cast<IMS_UINTP>(&objMockIMessage),
            IMS_NULL);
    pUceSubscribe->subscribedNotifyReceived(objMsg);
}

TEST_F(UceSubscribeTest, setState)
{
    IMS_TRACE_D("setState", 0, 0, 0);
    pUceSubscribe->setState(UceSubscribe::ON);
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}