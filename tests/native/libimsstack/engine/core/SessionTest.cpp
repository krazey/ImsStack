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

#include "CoreService.h"
#include "MockISipClientConnection.h"
#include "Session.h"
#include "SessionRefreshHelper.h"

using ::testing::Return;

namespace android
{

class TestSessionRefreshHelper : public SessionRefreshHelper
{
public:
    inline TestSessionRefreshHelper(Service* pService, IRefreshable* piRefreshable) :
            SessionRefreshHelper(pService, piRefreshable)
    {
    }

    inline void RefreshCompleted(ISipClientConnection* piScc, IMS_SINT32 nCode /*= 0*/)
    {
        SessionRefreshHelper::RefreshCompleted(piScc, nCode);
    }

    inline void SetRefreshConnection(ISipClientConnection* piScc) { SetConnection(piScc); }
};

class TestSession : public Session
{
public:
    inline explicit TestSession(IN Service* pService) :
            Session(pService),
            m_pTestRefreshHelper(IMS_NULL)
    {
    }

    inline SessionRefreshHelper* CreateRefreshHelper() override
    {
        if (m_pTestRefreshHelper == IMS_NULL)
        {
            m_pTestRefreshHelper = new TestSessionRefreshHelper(GetService(), this);
        }

        return m_pTestRefreshHelper;
    }

    inline void RefreshCompleted(ISipClientConnection* piScc, IMS_SINT32 nCode /*= 0*/)
    {
        m_pTestRefreshHelper->RefreshCompleted(piScc, nCode);
    }

    inline void SetRefreshConnection(ISipClientConnection* piScc)
    {
        m_pTestRefreshHelper->SetRefreshConnection(piScc);
    }

public:
    TestSessionRefreshHelper* m_pTestRefreshHelper;
};

class SessionTest : public ::testing::Test
{
public:
    inline SessionTest() :
            m_pCoreService(new CoreService("test.app", "test.service")),
            m_pSession(IMS_NULL)
    {
    }
    inline virtual ~SessionTest()
    {
        if (m_pSession != IMS_NULL)
        {
            delete m_pSession;
        }

        if (m_pCoreService != IMS_NULL)
        {
            delete m_pCoreService;
        }
    }

protected:
    virtual void SetUp() override
    {
        m_pSession = new TestSession(m_pCoreService);

        SipAddress objUserId("sip:1234@test.ims.com");
        m_pSession->InitMethod("sip:1234@test.ims.com", "sip:5678@test.ims.com", objUserId);
    }

    virtual void TearDown() override
    {
        if (m_pSession != IMS_NULL)
        {
            delete m_pSession;
            m_pSession = IMS_NULL;
        }
    }

protected:
    CoreService* m_pCoreService;
    TestSession* m_pSession;
};

TEST_F(SessionTest, SetConfiguration)
{
    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE, m_pSession->GetConfiguration());

    m_pSession->SetConfiguration(m_pSession->GetConfiguration() |
            Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR |
            Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED);

    EXPECT_TRUE(m_pSession->IsConfigurationSet(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE));
    EXPECT_TRUE(m_pSession->IsConfigurationSet(Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR));
    EXPECT_TRUE(
            m_pSession->IsConfigurationSet(Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED));
    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE |
                    Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR |
                    Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED,
            m_pSession->GetConfiguration());

    m_pSession->SetConfiguration(m_pSession->GetConfiguration() &
            (~Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR) &
            (~Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED));

    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE, m_pSession->GetConfiguration());
}

TEST_F(SessionTest, IsSessionRefreshInProgress)
{
    EXPECT_FALSE(m_pSession->IsSessionRefreshInProgress());

    MockISipClientConnection* pScc = new MockISipClientConnection();
    m_pSession->SetRefreshConnection(pScc);

    EXPECT_TRUE(m_pSession->IsSessionRefreshInProgress());

    m_pSession->RefreshCompleted(pScc, 1);
    m_pSession->SetRefreshConnection(IMS_NULL);

    EXPECT_FALSE(m_pSession->IsSessionRefreshInProgress());

    delete pScc;
}

}  // namespace android
