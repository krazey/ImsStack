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
#include "MockIReasonHeaderSetter.h"
#include "MockISipClientConnection.h"
#include "Session.h"
#include "SessionRefreshHelper.h"
#include "TestCoreBase.h"
#include "TestCoreService.h"

using ::testing::_;
using ::testing::Eq;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class TestSessionRefreshHelper : public SessionRefreshHelper
{
public:
    inline TestSessionRefreshHelper(Service* pService, IRefreshable* piRefreshable) :
            SessionRefreshHelper(pService, piRefreshable)
    {
    }

    inline void RefreshCompleted(ISipClientConnection* piScc, IMS_SINT32 nCode /*= 0*/) override
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

    inline void SetTerminationReasonForTest(IN IMS_SINT32 nReason)
    {
        SetTerminationReason(nReason);
    }
    inline void SetStateForTest(IN IMS_SINT32 nState) { SetState(nState); }
    inline void SendRequestToByeInternalForTest() { SendRequestToByeInternal(); }
    inline void CreateDialog(IN const ISipConnection* piSc) { CheckNCreateDialog(piSc); }

public:
    TestSessionRefreshHelper* m_pTestRefreshHelper;
};

class SessionTest : public TestCoreBase
{
public:
    inline SessionTest() :
            TestCoreBase(),
            m_pSession(IMS_NULL)
    {
    }
    inline ~SessionTest() override
    {
        if (m_pSession != IMS_NULL)
        {
            delete m_pSession;
        }
    }

protected:
    virtual void SetUp() override
    {
        TestCoreBase::SetUp();

        m_pSession = new TestSession(GetCoreService());
        InitMethod(m_pSession);
    }

    virtual void TearDown() override
    {
        if (m_pSession != IMS_NULL)
        {
            delete m_pSession;
            m_pSession = IMS_NULL;
        }

        TestCoreBase::TearDown();
    }

protected:
    TestSession* m_pSession;
};

TEST_F(SessionTest, SetConfiguration)
{
    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE |
                    Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR |
                    Session::CONFIG_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE,
            m_pSession->GetConfiguration());

    m_pSession->SetConfiguration(
            m_pSession->GetConfiguration() | Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED);

    EXPECT_TRUE(m_pSession->IsConfigurationSet(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE));
    EXPECT_TRUE(m_pSession->IsConfigurationSet(Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR));
    EXPECT_TRUE(
            m_pSession->IsConfigurationSet(Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED));
    EXPECT_TRUE(m_pSession->IsConfigurationSet(
            Session::CONFIG_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE));
    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE |
                    Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR |
                    Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED |
                    Session::CONFIG_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE,
            m_pSession->GetConfiguration());

    m_pSession->SetConfiguration(m_pSession->GetConfiguration() &
            (~Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED));

    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE |
                    Session::CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR |
                    Session::CONFIG_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE,
            m_pSession->GetConfiguration());
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

TEST_F(SessionTest, SendRequestToByeInternal)
{
    SetUpClientConnection(IMS_TRUE);
    ON_CALL(GetScc(), SetExtensionTokenForViaBranch(_)).WillByDefault(Return());
    ON_CALL(GetSipMsg(), GetType()).WillByDefault(Return(ISipMessage::TYPE_REQUEST));

    MockIReasonHeaderSetter objReasonHeaderSetter;
    m_pSession->SetReasonHeaderSetter(&objReasonHeaderSetter);
    m_pSession->SetStateForTest(Session::STATE_ESTABLISHED);
    m_pSession->SetTerminationReasonForTest(Session::TERMINATION_REASON_REFRESH_TIMEOUT);
    EXPECT_CALL(objReasonHeaderSetter,
            ReasonHeaderSetter_SetHeader(_, Session::TERMINATION_REASON_REFRESH_TIMEOUT))
            .WillOnce(Return());

    m_pSession->SendRequestToByeInternalForTest();
}

TEST_F(SessionTest, CreateTransaction)
{
    SetUpClientConnection(IMS_TRUE);

    // Create a SIP dialog
    SetUpDialog(ISipDialog::STATE_CONFIRMED);
    m_pSession->CreateDialog(&GetScc());

    AString strServiceRoute("sip:192.168.0.1");
    AStringArray objServiceRoutes;
    objServiceRoutes.AddElement(strServiceRoute);
    ON_CALL(GetCoreService()->GetMockRegBinding(), GetServiceRoutes())
            .WillByDefault(ReturnRef(objServiceRoutes));
    ON_CALL(GetSipMsg(), GetType()).WillByDefault(Return(ISipMessage::TYPE_REQUEST));

    EXPECT_CALL(GetScc(), SetImplicitRouteHeader(Eq(strServiceRoute))).WillOnce(Return());

    SipMethod objMethod(SipMethod::INFO);
    ISipClientConnection* piScc = m_pSession->CreateTransaction(objMethod);

    ASSERT_TRUE(piScc != nullptr);
}

}  // namespace android
