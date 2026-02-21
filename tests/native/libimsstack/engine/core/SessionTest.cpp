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
#include "IMessage.h"
#include "MockISipDialog.h"
#include "MockIReasonHeaderSetter.h"
#include "MockISipClientConnection.h"
#include "Session.h"
#include "SessionRefreshHelper.h"
#include "SipError.h"
#include "SipMessage.h"
#include "SipMethod.h"
#include "SipPrivate.h"

#include "TestCoreBase.h"
#include "TestCoreService.h"

using ::testing::_;
using ::testing::An;
using ::testing::Eq;
using ::testing::Invoke;
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
            m_pTestRefreshHelper(IMS_NULL),
            m_pForkedSession(IMS_NULL)
    {
    }
    inline ~TestSession() override
    {
        if (m_pForkedSession != IMS_NULL)
        {
            delete m_pForkedSession;
        }
    }

    inline Session* GetForkedSession() const { return m_pForkedSession; }
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
    inline void NotifySipForkedResponseForTest(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc)
    {
        UpdateRequestOnSent(IMessage::SESSION_START, piScc);
        NotifySipForkedResponse(piScc, piForkedScc);
    }

    inline void DispatchMessageForTest(IN const ImsMessage& objMsg)
    {
        switch (objMsg.GetName())
        {
            case AMSG_SESSION_FORKED_RESPONSE_RECEIVED:
                m_pForkedSession = reinterpret_cast<Session*>(objMsg.nLparam);
                break;
            default:
                break;
        }
    }

public:
    TestSessionRefreshHelper* m_pTestRefreshHelper;
    Session* m_pForkedSession;
};

class SessionTest : public TestCoreBase
{
public:
    inline SessionTest() :
            TestCoreBase(),
            m_nMsgCount(1),
            m_pForkedSipMsg(IMS_NULL),
            m_pSession(IMS_NULL)
    {
    }
    inline ~SessionTest() override
    {
        if (m_pForkedSipMsg != IMS_NULL)
        {
            delete m_pForkedSipMsg;
        }

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

        ON_CALL(m_pThreadService->GetMockThread(), PostMessageI(An<ImsMessage&>()))
                .WillByDefault(Invoke(
                        [&](IN ImsMessage& objMsg)
                        {
                            m_pSession->DispatchMessageForTest(objMsg);
                            return IMS_TRUE;
                        }));

        InitMethod(m_pSession);
    }

    virtual void TearDown() override
    {
        if (m_pSession != IMS_NULL)
        {
            delete m_pSession;
            m_pSession = IMS_NULL;
        }

        SipPrivate::SetLastError(SipError::NO_ERROR);
        TestCoreBase::TearDown();
    }

    void SetUpForkedClientConnection()
    {
        if (m_pForkedSipMsg == IMS_NULL)
        {
            m_pForkedSipMsg = new sipcore::SipMessage(ISipMessage::TYPE_RESPONSE);
            m_pForkedSipMsg->SetMethod(SipMethod(SipMethod::INVITE));
            m_pForkedSipMsg->SetStatusCode(183);
        }
        ON_CALL(m_objForkedScc, Close()).WillByDefault(Return());
        ON_CALL(m_objForkedScc, SetErrorListener(_)).WillByDefault(Return());
        ON_CALL(m_objForkedScc, SetListener(_)).WillByDefault(Return());
        ON_CALL(m_objForkedScc, GetDialog()).WillByDefault(Return(&m_objForkedDialog));
        ON_CALL(m_objForkedScc, GetMessage()).WillByDefault(Return(m_pForkedSipMsg));
        ON_CALL(m_objForkedScc, GetMethod()).WillByDefault(ReturnRef(m_pForkedSipMsg->GetMethod()));
        ON_CALL(m_objForkedScc, Receive)
                .WillByDefault(Invoke(
                        [&]()
                        {
                            if (m_nMsgCount > 0)
                            {
                                m_nMsgCount--;
                                return IMS_SUCCESS;
                            }
                            SipPrivate::SetLastError(SipError::NO_MESSAGE);
                            return IMS_FAILURE;
                        }));

        ON_CALL(m_objForkedDialog, Clone()).WillByDefault(Return(&m_objForkedDialog));
        ON_CALL(m_objForkedDialog, GetState()).WillByDefault(Return(ISipDialog::STATE_EARLY));
        ON_CALL(m_objForkedDialog, Destroy()).WillByDefault(Return());
    }

protected:
    IMS_SINT32 m_nMsgCount;
    sipcore::SipMessage* m_pForkedSipMsg;
    MockISipDialog m_objForkedDialog;
    MockISipClientConnection m_objForkedScc;
    TestSession* m_pSession;
};

TEST_F(SessionTest, SetConfiguration)
{
    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE |
                    Session::CONFIG_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE,
            m_pSession->GetConfiguration());

    m_pSession->SetConfiguration(
            m_pSession->GetConfiguration() | Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED);

    EXPECT_TRUE(m_pSession->IsConfigurationSet(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE));
    EXPECT_TRUE(
            m_pSession->IsConfigurationSet(Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED));
    EXPECT_TRUE(m_pSession->IsConfigurationSet(
            Session::CONFIG_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE));
    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE |
                    Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED |
                    Session::CONFIG_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE,
            m_pSession->GetConfiguration());

    m_pSession->SetConfiguration(m_pSession->GetConfiguration() &
            (~Session::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED));

    EXPECT_EQ(Session::CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE |
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
    const ISipClientConnection* piScc = m_pSession->CreateTransaction(objMethod);

    ASSERT_TRUE(piScc != nullptr);
}

TEST_F(SessionTest, NotifySipForkedResponse)
{
    SipMethod objMethod(SipMethod::INVITE);
    SetMethodForSipConnection(objMethod);
    SetUpClientConnection(IMS_TRUE);
    SetUpForkedClientConnection();

    m_pSession->SetConfiguration(m_pSession->GetConfiguration() | Session::CONFIG_SUPPORT_PREVIEW);
    m_pSession->NotifySipForkedResponseForTest(&GetScc(), &m_objForkedScc);

    const Session* pForkedSession = m_pSession->GetForkedSession();
    ASSERT_TRUE(pForkedSession != IMS_NULL);
    ASSERT_EQ(m_pSession->GetConfiguration(), pForkedSession->GetConfiguration());
}

}  // namespace android
