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

class TestAosDnsQuery : public AosDnsQuery
{
public:
    TestAosDnsQuery(IN IMS_BOOL bIsTest) :
            AosDnsQuery(bIsTest)
    {
    }

    FRIEND_TEST(AosDnsQueryTest, RequestReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, DestroyReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, DnsQueryPrivateReadyReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, DnsQueryPrivateDoneWithParamTrueReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, DnsQueryPrivateDoneWithParamFalseReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, DnsQueryPrivateTerminatedReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, OnMessageWithoutListenerReturnFalse);
    FRIEND_TEST(AosDnsQueryTest, OnMessageWithMsgReadyReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, OnMessageWithMsgRequestReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, OnMessageWithDuplicatedMsgRequestReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, OnMessageWithMsgDoneAndParamZeroReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, OnMessageWithMsgDoneAndParamNotZeroReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, OnMessageWithMsgDestroyReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, OnMessageWithMsgTerminatedReturnTrue);
    FRIEND_TEST(AosDnsQueryTest, OnMessageWithInvalidMsgReturnFalse);
    FRIEND_TEST(AosDnsQueryTest, SucceedSetEventToDnsQueryPrivate);
    FRIEND_TEST(AosDnsQueryTest, FailSetEventToDnsQueryPrivateWhenDuplicate);
    FRIEND_TEST(AosDnsQueryTest, SucceedResetEventToDnsQueryPrivate);
    FRIEND_TEST(AosDnsQueryTest, FailResetEventToDnsQueryPrivateWhenDuplicate);
    FRIEND_TEST(AosDnsQueryTest, StartDnsQueryPrivateReturnFalseWhenNullThread);
    FRIEND_TEST(AosDnsQueryTest, TerminateDnsQueryPrivateReturnFalseWhenNullThread);
    FRIEND_TEST(AosDnsQueryTest, RunDnsQueryPrivateThenResetEventWhenQueryFail);
    FRIEND_TEST(AosDnsQueryTest, RunDnsQueryPrivateThenResetEventWhenQuerySuccess);
    FRIEND_TEST(AosDnsQueryTest, RunDnsQueryPrivateThenResetEventWhenQuerySuccessWithSignal);
    FRIEND_TEST(AosDnsQueryTest, RunDnsQueryPrivateThenResetTerminateEvent);
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

TEST_F(AosDnsQueryTest, RequestReturnTrue)
{
    // GIVEN
    AString strDomainName = AString("testDomainName");

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->Request(strDomainName, IMS_NULL);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, DestroyReturnTrue)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->Destroy();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, DnsQueryPrivateReadyReturnTrue)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->DnsQueryPrivate_Ready();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, DnsQueryPrivateDoneWithParamTrueReturnTrue)
{
    // GIVEN
    ImsList<IpAddress> Ips;

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->DnsQueryPrivate_Done(IMS_TRUE, Ips);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, DnsQueryPrivateDoneWithParamFalseReturnTrue)
{
    // GIVEN
    ImsList<IpAddress> Ips;

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->DnsQueryPrivate_Done(IMS_FALSE, Ips);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, DnsQueryPrivateTerminatedReturnTrue)
{
    // GIVEN
    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->DnsQueryPrivate_Terminated();

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, OnMessageWithoutListenerReturnFalse)
{
    // GIVEN
    m_pAosDnsQuery->SetListener(IMS_NULL);

    IMSMSG objMsg(TestAosDnsQuery::MSG_READY, 0, 0);

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->OnMessage(objMsg);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosDnsQueryTest, OnMessageWithMsgReadyReturnTrue)
{
    // GIVEN
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(1);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(&objMockIAosDnsQueryListener);

    IMSMSG objMsg(TestAosDnsQuery::MSG_READY, 0, 0);

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->OnMessage(objMsg);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, OnMessageWithMsgRequestReturnTrue)
{
    // GIVEN
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(&objMockIAosDnsQueryListener);

    IMSMSG objMsg(TestAosDnsQuery::MSG_REQUEST, 0, 0);

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->OnMessage(objMsg);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, OnMessageWithDuplicatedMsgRequestReturnTrue)
{
    // GIVEN
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(&objMockIAosDnsQueryListener);

    // DoDnsQuery() returns IMS_FALSE
    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));

    IMSMSG objMsg(TestAosDnsQuery::MSG_REQUEST, 0, 0);

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->OnMessage(objMsg);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, OnMessageWithMsgDoneAndParamZeroReturnTrue)
{
    // GIVEN
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(1);

    m_pAosDnsQuery->SetListener(&objMockIAosDnsQueryListener);

    IMSMSG objMsg(TestAosDnsQuery::MSG_DONE, 0, 0);

    // WHen
    IMS_BOOL bResult = m_pAosDnsQuery->OnMessage(objMsg);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, OnMessageWithMsgDoneAndParamNotZeroReturnTrue)
{
    // GIVEN
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(1);

    m_pAosDnsQuery->SetListener(&objMockIAosDnsQueryListener);

    IMSMSG objMsg(TestAosDnsQuery::MSG_DONE, 1, 0);

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->OnMessage(objMsg);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, OnMessageWithMsgDestroyReturnTrue)
{
    // GIVEN
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(&objMockIAosDnsQueryListener);

    IMSMSG objMsg(TestAosDnsQuery::MSG_DESTROY, 0, 0);

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->OnMessage(objMsg);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, OnMessageWithMsgTerminatedReturnTrue)
{
    // GIVEN
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(&objMockIAosDnsQueryListener);

    IMSMSG objMsg(TestAosDnsQuery::MSG_TERMINATED, 0, 0);

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->OnMessage(objMsg);

    // THEN
    EXPECT_TRUE(bResult);
}

TEST_F(AosDnsQueryTest, OnMessageWithInvalidMsgReturnFalse)
{
    // GIVEN
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(0);
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(0);

    m_pAosDnsQuery->SetListener(&objMockIAosDnsQueryListener);

    IMSMSG objMsg(TestAosDnsQuery::MSG_TERMINATED + 999, 0, 0);

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->OnMessage(objMsg);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosDnsQueryTest, SucceedSetEventToDnsQueryPrivate)
{
    // GIVEN
    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(TestAosDnsQuery::DNS_QUERY_NONE));
    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));

    // WHEN
    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(TestAosDnsQuery::DNS_QUERY_EXEC));

    // THEN
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
}

TEST_F(AosDnsQueryTest, FailSetEventToDnsQueryPrivateWhenDuplicate)
{
    // GIVEN
    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->SetEvent(TestAosDnsQuery::DNS_QUERY_EXEC);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosDnsQueryTest, SucceedResetEventToDnsQueryPrivate)
{
    // GIVEN
    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));

    // WHEN
    EXPECT_TRUE(m_pAosDnsQuery->ResetEvent(TestAosDnsQuery::DNS_QUERY_EXEC));

    // THEN
    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
}

TEST_F(AosDnsQueryTest, FailResetEventToDnsQueryPrivateWhenDuplicate)
{
    // GIVEN
    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));

    EXPECT_TRUE(m_pAosDnsQuery->ResetEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->ResetEvent(TestAosDnsQuery::DNS_QUERY_EXEC);

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosDnsQueryTest, StartDnsQueryPrivateReturnFalseWhenNullThread)
{
    // GIVEN
    m_pAosDnsQuery->SetThread(IMS_NULL);

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->Start();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosDnsQueryTest, TerminateDnsQueryPrivateReturnFalseWhenNullThread)
{
    // GIVEN
    m_pAosDnsQuery->SetThread(IMS_NULL);

    // WHEN
    IMS_BOOL bResult = m_pAosDnsQuery->Terminate();

    // THEN
    EXPECT_FALSE(bResult);
}

TEST_F(AosDnsQueryTest, RunDnsQueryPrivateThenResetEventWhenQueryFail)
{
    // GIVEN
    EXPECT_CALL(m_objMockINetworkConnection, GetHostByName(_, _, _)).Times(1).WillOnce(Return(-1));

    m_pAosDnsQuery->SetConnection(&m_objMockINetworkConnection);

    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));

    // WHEN
    m_pAosDnsQuery->RunImp();

    // THEN
    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
}

TEST_F(AosDnsQueryTest, RunDnsQueryPrivateThenResetEventWhenQuerySuccess)
{
    // GIVEN
    EXPECT_CALL(m_objMockINetworkConnection, GetHostByName(_, _, _)).Times(1).WillOnce(Return(1));

    m_pAosDnsQuery->SetConnection(&m_objMockINetworkConnection);

    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));

    // WHEN
    m_pAosDnsQuery->RunImp();

    // THEN
    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
}

TEST_F(AosDnsQueryTest, RunDnsQueryPrivateThenResetEventWhenQuerySuccessWithSignal)
{
    // GIVEN
    EXPECT_CALL(m_objMockINetworkConnection, GetHostByName(_, _, _)).Times(1).WillOnce(Return(1));

    m_pAosDnsQuery->SetConnection(&m_objMockINetworkConnection);
    m_pAosDnsQuery->SetSignaled(IMS_TRUE);

    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));

    // WHEN
    m_pAosDnsQuery->RunImp();

    // THEN
    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_EXEC));
}

TEST_F(AosDnsQueryTest, RunDnsQueryPrivateThenResetTerminateEvent)
{
    // GIVEN
    EXPECT_TRUE(m_pAosDnsQuery->SetEvent(TestAosDnsQuery::DNS_QUERY_TERMINATE));
    EXPECT_TRUE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_TERMINATE));

    // WHEN
    m_pAosDnsQuery->RunImp();

    // THEN
    EXPECT_FALSE(m_pAosDnsQuery->HasEvent(TestAosDnsQuery::DNS_QUERY_TERMINATE));
}
