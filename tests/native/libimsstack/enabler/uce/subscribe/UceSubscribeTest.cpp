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

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

using ::testing::AnyNumber;
using ::testing::Return;

IMS_SINT32 SUB_SIM_SLOT = 20;

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
    TestUceSubscribe() :
            UceSubscribe(
                    IMS_NULL, AString("AppName"), AString("UceSubscribeManager"), 0, SUB_SIM_SLOT)
    {
    }
    virtual ~TestUceSubscribe() {}

    IMS_UINT32 GetKey() const { return m_nKey; }

    void SetKey(IMS_UINT32 key) { m_nKey = key; }

    AString GetRemoteUser() const { return m_strRemoteUser; }

    IMSList<AString> GetRemoteUsers() const { return m_objRemoteUsers; }

    IMS_UINT32 GetState() const { return m_eState; }

    void setState(IMS_UINT32 state) { SetState(state); }
};

class UceSubscribeTest : public ::testing::Test
{
public:
    TestUceSubscribe* pUceSubscribe;

protected:
    virtual void SetUp() override
    {
        pUceSubscribe = new TestUceSubscribe();
        ASSERT_TRUE(pUceSubscribe != nullptr);
    }

    virtual void TearDown() override
    {
        if (pUceSubscribe)
        {
            delete pUceSubscribe;
        }
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
}

TEST_F(UceSubscribeTest, ON_SingleSubscribeRequested)
{
    IMS_TRACE_D("ON_SingleSubscribeRequested", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->setState(UceSubscribe::ON);
    pUceSubscribe->SetKey(key);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);
    IMSMSG objMsg(TestUceSubscribe::SINGLE_REQUESTED, 0, IMS_NULL);

    EXPECT_EQ(pUceSubscribe->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
}

TEST_F(UceSubscribeTest, ON_ListSubscribeRequested)
{
    IMS_TRACE_D("ON_ListSubscribeRequested", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->setState(UceSubscribe::ON);
    pUceSubscribe->SetKey(key);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);
    IMSMSG objMsg(TestUceSubscribe::LIST_REQUESTED, 0, IMS_NULL);

    EXPECT_EQ(pUceSubscribe->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
}

TEST_F(UceSubscribeTest, SUBSCRIBING_AoSDisConnected)
{
    IMS_TRACE_D("SUBSCRIBING_AoSDisConnected", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->setState(UceSubscribe::SUBSCRIBING);
    pUceSubscribe->SetKey(key);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);
    IMSMSG objMsg(TestUceSubscribe::AOS_DISCONNECTED, 0, IMS_NULL);

    EXPECT_EQ(pUceSubscribe->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, SUBSCRIBING_SubscribeTerminated)
{
    IMS_TRACE_D("SUBSCRIBING_SubscribeTerminated", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->setState(UceSubscribe::SUBSCRIBING);
    pUceSubscribe->SetKey(key);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);
    IMSMSG objMsg(TestUceSubscribe::SUBSCRIBE_TERMINATED, 0, IMS_NULL);

    EXPECT_EQ(pUceSubscribe->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, SUBSCRIBED_AoSDisConnected)
{
    IMS_TRACE_D("SUBSCRIBED_AoSDisConnected", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->setState(UceSubscribe::SUBSCRIBED);
    pUceSubscribe->SetKey(key);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);
    IMSMSG objMsg(TestUceSubscribe::AOS_DISCONNECTED, 0, IMS_NULL);

    EXPECT_EQ(pUceSubscribe->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}

TEST_F(UceSubscribeTest, SUBSCRIBED_SubscribeTerminated)
{
    IMS_TRACE_D("SUBSCRIBED_SubscribeTerminated", 0, 0, 0);
    IMS_UINT32 key = 10;
    pUceSubscribe->setState(UceSubscribe::SUBSCRIBED);
    pUceSubscribe->SetKey(key);
    EXPECT_EQ(pUceSubscribe->GetKey(), key);
    IMSMSG objMsg(TestUceSubscribe::SUBSCRIBE_TERMINATED, 0, IMS_NULL);

    EXPECT_EQ(pUceSubscribe->OnStateMessage(objMsg), IMS_TRUE);
    EXPECT_EQ(pUceSubscribe->GetKey(), 0);
    EXPECT_EQ(pUceSubscribe->GetState(), UceSubscribe::ON);
}