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

#include "MockICoreService.h"
#include "MockIReference.h"
#include "MockISession.h"
#include "MockISipServerConnection.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "call/IMtcCall.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Mock;
using ::testing::Return;

LOCAL const CallKey CALL_KEY_1 = 1;
LOCAL const CallKey CALL_KEY_2 = 2;

namespace android
{

class SessionInterfaceHolderTest : public ::testing::Test
{
public:
    SessionInterfaceHolder* pHolder;
    MockIInterfaceHolderListener objListener;
    MockISession objMockISession;
    TestTimerService objTimerService;
    MockITimer objMockITimer;

protected:
    virtual void SetUp() override
    {
        objTimerService.SetTimer(&objMockITimer);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);
        pHolder = new SessionInterfaceHolder();
    }

    virtual void TearDown() override
    {
        delete pHolder;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }
};

TEST_F(SessionInterfaceHolderTest, HolderDoesNothingForUnexpectedSessionListeners)
{
    MockIReference objMockIReference;
    MockISipServerConnection objMockServerConnection;

    EXPECT_CALL(objMockISession, Destroy).Times(0);

    pHolder->SessionAlerting(&objMockISession);
    pHolder->SessionReferenceReceived(&objMockISession, &objMockIReference);
    pHolder->SessionStartFailed(&objMockISession);
    pHolder->SessionUpdated(&objMockISession);
    pHolder->SessionUpdateFailed(&objMockISession);
    pHolder->SessionUpdateReceived(&objMockISession);
    pHolder->SessionCancelDelivered(&objMockISession);
    pHolder->SessionCancelDeliveryFailed(&objMockISession);
    pHolder->SessionEarlyMediaUpdated(&objMockISession);
    pHolder->SessionEarlyMediaUpdateFailed(&objMockISession);
    pHolder->SessionEarlyMediaUpdateReceived(&objMockISession);
    pHolder->SessionForkedResponseReceived(&objMockISession, &objMockISession);
    pHolder->SessionPrackDelivered(&objMockISession);
    pHolder->SessionPrackDeliveryFailed(&objMockISession);
    pHolder->SessionPrackReceived(&objMockISession);
    pHolder->SessionProvisionalResponseReceived(&objMockISession, 0);
    pHolder->SessionRprDeliveryFailed(&objMockISession);
    pHolder->SessionRprReceived(&objMockISession, 0);
    pHolder->SessionTransactionReceived(&objMockISession, &objMockServerConnection);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
}

TEST_F(SessionInterfaceHolderTest, SendsAckAndTerminatesIfSessionStarted)
{
    EXPECT_CALL(objMockISession, SendAck).Times(1);
    EXPECT_CALL(objMockISession, Terminate).Times(1);

    pHolder->SessionStarted(&objMockISession);
}

TEST_F(SessionInterfaceHolderTest, StopsTimerIfSessionStartFailed)
{
    EXPECT_CALL(objMockISession, Destroy()).Times(1);

    pHolder->AddISession(CALL_KEY_1, &objMockISession);
    pHolder->SessionStartFailed(&objMockISession);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 0);
}

TEST_F(SessionInterfaceHolderTest, StopsTimerIfSessionTerminated)
{
    EXPECT_CALL(objMockISession, Destroy()).Times(1);

    pHolder->AddISession(CALL_KEY_1, &objMockISession);
    pHolder->SessionTerminated(&objMockISession);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 0);
}

TEST_F(SessionInterfaceHolderTest, AddAndReleaseStartsTimer)
{
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());

    EXPECT_EQ(pHolder->GetSessionCount(), 0);

    pHolder->AddISession(CALL_KEY_1, &objMockISession);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 1);

    pHolder->ReleaseISession(&objMockISession);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 1);
}

TEST_F(SessionInterfaceHolderTest, AddAndReleaseWithTerminatedStopsTimer)
{
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());
    EXPECT_CALL(objMockISession, Destroy()).Times(1);

    EXPECT_EQ(pHolder->GetSessionCount(), 0);

    pHolder->AddISession(CALL_KEY_1, &objMockISession);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 1);

    pHolder->ReleaseISession(&objMockISession);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISession));

    pHolder->ReleaseISession(&objMockISession, IMS_TRUE, IMS_FALSE);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 0);
}

TEST_F(SessionInterfaceHolderTest, GetAndReleaseStartsTimer)
{
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());

    MockICoreService objMockICoreService;
    ON_CALL(objMockICoreService, CreateSession(_, _)).WillByDefault(Return(&objMockISession));

    EXPECT_EQ(pHolder->GetSessionCount(), 0);

    ISession* piSession =
            pHolder->GetISession(CALL_KEY_1, &objMockICoreService, "sip:fromuri", "sip:touri");
    EXPECT_EQ(piSession, &objMockISession);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));

    pHolder->ReleaseISession(&objMockISession);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 1);
}

TEST_F(SessionInterfaceHolderTest, GetAndReleaseWithTimerExpiredStopsTimer)
{
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());

    MockICoreService objMockICoreService;
    ON_CALL(objMockICoreService, CreateSession(_, _)).WillByDefault(Return(&objMockISession));

    EXPECT_EQ(pHolder->GetSessionCount(), 0);

    ISession* piSession =
            pHolder->GetISession(CALL_KEY_1, &objMockICoreService, "sip:fromuri", "sip:touri");
    EXPECT_EQ(piSession, &objMockISession);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));

    pHolder->ReleaseISession(&objMockISession);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 1);

    pHolder->Timer_TimerExpired(&objMockITimer);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 0);
}

TEST_F(SessionInterfaceHolderTest, GetAndReleaseWithTerminatedStopsTimer)
{
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());
    EXPECT_CALL(objMockISession, Destroy()).Times(1);

    MockICoreService objMockICoreService;
    ON_CALL(objMockICoreService, CreateSession(_, _)).WillByDefault(Return(&objMockISession));

    EXPECT_EQ(pHolder->GetSessionCount(), 0);

    ISession* piSession =
            pHolder->GetISession(CALL_KEY_1, &objMockICoreService, "sip:fromuri", "sip:touri");
    EXPECT_EQ(piSession, &objMockISession);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));

    pHolder->ReleaseISession(&objMockISession);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISession));

    pHolder->ReleaseISession(&objMockISession, IMS_TRUE, IMS_FALSE);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 0);
}

TEST_F(SessionInterfaceHolderTest, GetAndReleaseWithSessionStartFailedDestroysSession)
{
    MockICoreService objMockICoreService;
    ON_CALL(objMockICoreService, CreateSession(_, _)).WillByDefault(Return(&objMockISession));

    const IMS_SINT32 objNonTerminatingStates[] = {ISession::STATE_CREATED,
            ISession::STATE_INITIATED, ISession::STATE_NEGOTIATING, ISession::STATE_ESTABLISHING,
            ISession::STATE_ESTABLISHED, ISession::STATE_RENEGOTIATING,
            ISession::STATE_REESTABLISHING, ISession::STATE_TERMINATED};

    for (const auto& state : objNonTerminatingStates)
    {
        EXPECT_CALL(objMockISession, Destroy()).Times(1);

        ISession* piSession =
                pHolder->GetISession(CALL_KEY_1, &objMockICoreService, "sip:fromuri", "sip:touri");

        ON_CALL(objMockISession, GetState).WillByDefault(Return(state));
        pHolder->ReleaseISession(&objMockISession, IMS_FALSE, IMS_TRUE);
    }
}

TEST_F(SessionInterfaceHolderTest,
        GetAndReleaseWithSessionStartFailedDoesNotDestroySessionIfTerminating)
{
    EXPECT_CALL(objMockISession, Destroy()).Times(0);

    MockICoreService objMockICoreService;
    ON_CALL(objMockICoreService, CreateSession(_, _)).WillByDefault(Return(&objMockISession));

    ISession* piSession =
            pHolder->GetISession(CALL_KEY_1, &objMockICoreService, "sip:fromuri", "sip:touri");

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    ON_CALL(objMockISession, GetState).WillByDefault(Return(ISession::STATE_TERMINATING));
    pHolder->ReleaseISession(&objMockISession, IMS_FALSE, IMS_TRUE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISession));

    // ISession#Destroy() is invoked by the Destructor so clears the expectations.
    Mock::VerifyAndClearExpectations(&objMockISession);
}

TEST_F(SessionInterfaceHolderTest, AddAndReleaseWithTerminatedNotifiesListener)
{
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());

    pHolder->AddListener(&objListener);

    EXPECT_CALL(objListener, OnSessionInterfaceReleased(CALL_KEY_1));

    pHolder->AddISession(CALL_KEY_1, &objMockISession);
    pHolder->ReleaseISession(&objMockISession, IMS_TRUE, IMS_FALSE);
}

TEST_F(SessionInterfaceHolderTest, AddAndRemoveListenerRemovesGivenListener)
{
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());

    MockIInterfaceHolderListener objAnotherListener;
    pHolder->AddListener(&objAnotherListener);
    pHolder->AddListener(&objListener);
    pHolder->RemoveListener(&objListener);

    EXPECT_CALL(objListener, OnSessionInterfaceReleased(_)).Times(0);
    EXPECT_CALL(objAnotherListener, OnSessionInterfaceReleased(_));

    pHolder->AddISession(CALL_KEY_1, &objMockISession);
    pHolder->ReleaseISession(&objMockISession, IMS_TRUE, IMS_FALSE);
}

TEST_F(SessionInterfaceHolderTest, ListnerIsNotifiedOnlyIfAllInterfacesInSameCallKeyAreReleased)
{
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());

    pHolder->AddListener(&objListener);

    MockISession objISession1OfCall1;
    MockISession objISession2OfCall1;
    MockISession objISession1OfCall2;
    MockISession objISession2OfCall2;

    pHolder->AddISession(CALL_KEY_1, &objISession1OfCall1);
    pHolder->AddISession(CALL_KEY_1, &objISession2OfCall1);
    pHolder->AddISession(CALL_KEY_2, &objISession1OfCall2);
    pHolder->AddISession(CALL_KEY_2, &objISession2OfCall2);

    {
        EXPECT_CALL(objListener, OnSessionInterfaceReleased(_)).Times(0);

        pHolder->ReleaseISession(&objISession1OfCall1, IMS_TRUE, IMS_FALSE);
        pHolder->ReleaseISession(&objISession1OfCall2, IMS_TRUE, IMS_FALSE);
    }

    {
        EXPECT_CALL(objListener, OnSessionInterfaceReleased(CALL_KEY_1));
        EXPECT_CALL(objListener, OnSessionInterfaceReleased(CALL_KEY_2)).Times(0);

        pHolder->ReleaseISession(&objISession2OfCall1, IMS_TRUE, IMS_FALSE);
    }

    {
        EXPECT_CALL(objListener, OnSessionInterfaceReleased(CALL_KEY_1)).Times(0);
        EXPECT_CALL(objListener, OnSessionInterfaceReleased(CALL_KEY_2));

        pHolder->ReleaseISession(&objISession2OfCall2, IMS_TRUE, IMS_FALSE);
    }
}

}  // namespace android
