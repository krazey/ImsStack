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

using ::testing::_;

enum
{
    MSG_READY = AOSMSG_SERVICE_INTERNAL,
    MSG_REQUEST,
    MSG_DONE,
    MSG_DESTROY,
    MSG_TERMINATED
};

class AosDnsQueryTest : public ::testing::Test
{
public:
    AosDnsQuery* m_pAosDnsQuery;

protected:
    virtual void SetUp() override
    {
        m_pAosDnsQuery = new AosDnsQuery(IMS_TRUE);
        ASSERT_TRUE(m_pAosDnsQuery != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pAosDnsQuery)
        {
            delete m_pAosDnsQuery;
        }
    }

    IMS_BOOL OnMessage(IN IMSMSG& objMsg) { return m_pAosDnsQuery->OnMessage(objMsg); }
};

TEST_F(AosDnsQueryTest, SetListener_ReceiveMessage_MsgReady)
{
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Ready()).Times(1);

    m_pAosDnsQuery->SetListener(static_cast<IAosDnsQueryListener*>(&objMockIAosDnsQueryListener));

    IMSMSG objMsg(MSG_READY, 0, 0);
    EXPECT_TRUE(OnMessage(objMsg));
}

TEST_F(AosDnsQueryTest, SetListener_ReceiveMessage_MsgDone)
{
    MockIAosDnsQueryListener objMockIAosDnsQueryListener;
    EXPECT_CALL(objMockIAosDnsQueryListener, DnsQuery_Done(_, _)).Times(1);

    m_pAosDnsQuery->SetListener(static_cast<IAosDnsQueryListener*>(&objMockIAosDnsQueryListener));

    IMSMSG objMsg(MSG_DONE, 0, 0);
    EXPECT_TRUE(OnMessage(objMsg));
}

TEST_F(AosDnsQueryTest, Request)
{
    AString strDomain = AString("testDomain");
    EXPECT_TRUE(m_pAosDnsQuery->Request(strDomain, IMS_NULL));
}

TEST_F(AosDnsQueryTest, Destroy)
{
    EXPECT_TRUE(m_pAosDnsQuery->Destroy());
}

TEST_F(AosDnsQueryTest, DnsQueryPrivate_Ready)
{
    EXPECT_TRUE(m_pAosDnsQuery->DnsQueryPrivate_Ready());
}

TEST_F(AosDnsQueryTest, DnsQueryPrivate_Done)
{
    IMSList<IPAddress> Ips;
    EXPECT_TRUE(m_pAosDnsQuery->DnsQueryPrivate_Done(IMS_TRUE, Ips));
    EXPECT_TRUE(m_pAosDnsQuery->DnsQueryPrivate_Done(IMS_FALSE, Ips));
}

TEST_F(AosDnsQueryTest, DnsQueryPrivate_Terminated)
{
    EXPECT_TRUE(m_pAosDnsQuery->DnsQueryPrivate_Terminated());
}