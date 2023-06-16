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

#include "provider/AosDnsQuery.h"

#include "provider/MockAosDnsQuery.h"
#include "../../../platform/interface/MockINetworkConnection.h"

using ::testing::_;
using ::testing::Return;

enum
{
    MSG_READY = AOSMSG_SERVICE_INTERNAL,
    MSG_REQUEST,
    MSG_DONE,
    MSG_DESTROY,
    MSG_TERMINATED
};

// Dns Query Event
enum
{
    DNS_QUERY_NONE = 0x0000,
    DNS_QUERY_EXEC = 0x0001,
    DNS_QUERY_TERMINATE = 0x8000
};

class TestAosDnsQuery : public AosDnsQuery
{
public:
    TestAosDnsQuery(IN IMS_BOOL bIsTest) :
            AosDnsQuery(bIsTest)
    {
    }

    FRIEND_TEST(AosDnsQueryTest, Request_success);
    FRIEND_TEST(AosDnsQueryTest, Destroy_success);
    FRIEND_TEST(AosDnsQueryTest, DnsQueryPrivate_Done);
    FRIEND_TEST(AosDnsQueryTest, DnsQueryPrivate_Terminated);
    FRIEND_TEST(AosDnsQueryTest, OnMessage_ListenerIsNull);
    FRIEND_TEST(AosDnsQueryTest, OnMessage_MsgReady);
    FRIEND_TEST(AosDnsQueryTest, OnMessage_MsgRequest);
    FRIEND_TEST(AosDnsQueryTest, OnMessage_MsgRequestDuplicated);
    FRIEND_TEST(AosDnsQueryTest, OnMessage_MsgDone);
    FRIEND_TEST(AosDnsQueryTest, OnMessage_MsgDestroy);
    FRIEND_TEST(AosDnsQueryTest, OnMessage_MsgTerminated);
    FRIEND_TEST(AosDnsQueryTest, OnMessage_MsgInvalid);
    FRIEND_TEST(AosDnsQueryTest, AosDnsQueryPrivate_SetEvent);
    FRIEND_TEST(AosDnsQueryTest, AosDnsQueryPrivate_ResetEvent);
    FRIEND_TEST(AosDnsQueryTest, AosDnsQueryPrivate_Start_ThreadIsNull);
    FRIEND_TEST(AosDnsQueryTest, AosDnsQueryPrivate_Terminate_ThreadIsNull);
    FRIEND_TEST(AosDnsQueryTest, AosDnsQueryPrivate_RunImp_QueryFailed);
    FRIEND_TEST(AosDnsQueryTest, AosDnsQueryPrivate_RunImp_QuerySuccess);
    FRIEND_TEST(AosDnsQueryTest, AosDnsQueryPrivate_RunImp_QuerySuccess_Signaled);
    FRIEND_TEST(AosDnsQueryTest, AosDnsQueryPrivate_RunImp_QueryTerminate);
};

class AosDnsQueryTest : public ::testing::Test
{
public:
    TestAosDnsQuery* m_pAosDnsQuery;
    MockINetworkConnection m_objMockINetworkConnection;

protected:
    virtual void SetUp() override
    {
        m_pAosDnsQuery = new TestAosDnsQuery(IMS_TRUE);
        ASSERT_TRUE(m_pAosDnsQuery != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pAosDnsQuery)
        {
            delete m_pAosDnsQuery;
        }
    }
};

TEST_F(AosDnsQueryTest, Request_success)
{
    AString strDomainName = AString("testDomainName");
    EXPECT_TRUE(m_pAosDnsQuery->Request(strDomainName, IMS_NULL));
}

TEST_F(AosDnsQueryTest, Destroy_success)
{
    EXPECT_TRUE(m_pAosDnsQuery->Destroy());
}

TEST_F(AosDnsQueryTest, DnsQueryPrivate_Ready)
{
    EXPECT_TRUE(m_pAosDnsQuery->DnsQueryPrivate_Ready());
}

TEST_F(AosDnsQueryTest, DnsQueryPrivate_Done)
{
    ImsList<IpAddress> Ips;
    EXPECT_TRUE(m_pAosDnsQuery->DnsQueryPrivate_Done(IMS_TRUE, Ips));
    EXPECT_TRUE(m_pAosDnsQuery->DnsQueryPrivate_Done(IMS_FALSE, Ips));
}

TEST_F(AosDnsQueryTest, DnsQueryPrivate_Terminated)
{
    EXPECT_TRUE(m_pAosDnsQuery->DnsQueryPrivate_Terminated());
}

TEST_F(AosDnsQueryTest, OnMessage_ListenerIsNull)
{
    m_pAosDnsQuery->SetListener(IMS_NULL);

    IMSMSG objMsg(MSG_READY, 0, 0);
    EXPECT_FALSE(m_pAosDnsQuery->OnMessage(objMsg));
}

TEST_F(AosDnsQueryTest, OnMessage_MsgReady)
{
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(1);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(static_cast<IAosDnsQueryListener*>(&objMockIAosDnsQueryListener));

    IMSMSG objMsg(MSG_READY, 0, 0);
    EXPECT_TRUE(m_pAosDnsQuery->OnMessage(objMsg));
}

TEST_F(AosDnsQueryTest, OnMessage_MsgRequest)
{
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(static_cast<IAosDnsQueryListener*>(&objMockIAosDnsQueryListener));

    IMSMSG objMsg(MSG_REQUEST, 0, 0);
    EXPECT_TRUE(m_pAosDnsQuery->OnMessage(objMsg));
}

TEST_F(AosDnsQueryTest, OnMessage_MsgRequestDuplicated)
{
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(static_cast<IAosDnsQueryListener*>(&objMockIAosDnsQueryListener));

    // DoDnsQuery() returns IMS_FALSE
    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));

    IMSMSG objMsg(MSG_REQUEST, 0, 0);
    EXPECT_TRUE(m_pAosDnsQuery->OnMessage(objMsg));
}

TEST_F(AosDnsQueryTest, OnMessage_MsgDone)
{
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(2);

    m_pAosDnsQuery->SetListener(static_cast<IAosDnsQueryListener*>(&objMockIAosDnsQueryListener));

    IMSMSG objMsg1(MSG_DONE, 0, 0);
    EXPECT_TRUE(m_pAosDnsQuery->OnMessage(objMsg1));

    IMSMSG objMsg2(MSG_DONE, 1, 0);
    EXPECT_TRUE(m_pAosDnsQuery->OnMessage(objMsg2));
}

TEST_F(AosDnsQueryTest, OnMessage_MsgDestroy)
{
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(static_cast<IAosDnsQueryListener*>(&objMockIAosDnsQueryListener));

    IMSMSG objMsg(MSG_DESTROY, 0, 0);
    EXPECT_TRUE(m_pAosDnsQuery->OnMessage(objMsg));
}

TEST_F(AosDnsQueryTest, OnMessage_MsgTerminated)
{
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(static_cast<IAosDnsQueryListener*>(&objMockIAosDnsQueryListener));

    IMSMSG objMsg(MSG_TERMINATED, 0, 0);
    EXPECT_TRUE(m_pAosDnsQuery->OnMessage(objMsg));
}

TEST_F(AosDnsQueryTest, OnMessage_MsgInvalid)
{
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(static_cast<IAosDnsQueryListener*>(&objMockIAosDnsQueryListener));

    IMSMSG objMsg(MSG_TERMINATED + 999, 0, 0);
    EXPECT_FALSE(m_pAosDnsQuery->OnMessage(objMsg));
}

TEST_F(AosDnsQueryTest, AosDnsQueryPrivate_SetEvent)
{
    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(DNS_QUERY_NONE));
    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));

    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));

    // Duplicated event
    EXPECT_FALSE(m_pAosDnsQuery->SetEvent(DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));
}

TEST_F(AosDnsQueryTest, AosDnsQueryPrivate_ResetEvent)
{
    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));

    EXPECT_TRUE(m_pAosDnsQuery->ResetEvent(DNS_QUERY_EXEC));
    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));

    // Duplicated event
    EXPECT_FALSE(m_pAosDnsQuery->ResetEvent(DNS_QUERY_EXEC));
    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));
}

TEST_F(AosDnsQueryTest, AosDnsQueryPrivate_Start_ThreadIsNull)
{
    m_pAosDnsQuery->SetThread(IMS_NULL);
    EXPECT_FALSE(m_pAosDnsQuery->Start());
}

TEST_F(AosDnsQueryTest, AosDnsQueryPrivate_Terminate_ThreadIsNull)
{
    m_pAosDnsQuery->SetThread(IMS_NULL);
    EXPECT_FALSE(m_pAosDnsQuery->Terminate());
}

TEST_F(AosDnsQueryTest, AosDnsQueryPrivate_RunImp_QueryFailed)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetHostByName(_, _, _)).Times(1).WillOnce(Return(-1));

    m_pAosDnsQuery->SetConnection(&m_objMockINetworkConnection);

    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));

    m_pAosDnsQuery->RunImp();

    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));
}

TEST_F(AosDnsQueryTest, AosDnsQueryPrivate_RunImp_QuerySuccess)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetHostByName(_, _, _)).Times(1).WillOnce(Return(1));

    m_pAosDnsQuery->SetConnection(&m_objMockINetworkConnection);

    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));

    m_pAosDnsQuery->RunImp();

    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));
}

TEST_F(AosDnsQueryTest, AosDnsQueryPrivate_RunImp_QuerySuccess_Signaled)
{
    EXPECT_CALL(m_objMockINetworkConnection, GetHostByName(_, _, _)).Times(1).WillOnce(Return(1));

    m_pAosDnsQuery->SetConnection(&m_objMockINetworkConnection);
    m_pAosDnsQuery->SetSignaled(IMS_TRUE);

    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));

    m_pAosDnsQuery->RunImp();

    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(DNS_QUERY_EXEC));
}

TEST_F(AosDnsQueryTest, AosDnsQueryPrivate_RunImp_QueryTerminate)
{
    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(DNS_QUERY_TERMINATE));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(DNS_QUERY_TERMINATE));
    m_pAosDnsQuery->RunImp();

    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(DNS_QUERY_TERMINATE));
}
