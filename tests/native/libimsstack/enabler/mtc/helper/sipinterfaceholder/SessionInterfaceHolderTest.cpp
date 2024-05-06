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

#include "MockIOsFactory.h"
#include "PlatformContext.h"
#include "core/MockICoreService.h"
#include "core/MockIReference.h"
#include "core/MockISession.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "sipcore/MockISipServerConnection.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

class TestImsTimerForSiHolder : public ImsTimer
{
public:
    inline TestImsTimerForSiHolder() {}
    inline ~TestImsTimerForSiHolder() {}

public:
    IMS_BOOL Equals(IN const ITimer* piTimer) const override
    {
        ImsTimer* pTimer = DYNAMIC_CAST(ImsTimer*, piTimer);
        return pTimer->GetTimerId() == reinterpret_cast<IMS_UINTP>(this);
    }

    IMS_UINTP SetTimer(IN IMS_UINT32 /*nDuration*/, IN ITimerListener* /*piListener*/) override
    {
        return reinterpret_cast<IMS_UINTP>(this);
    }

    void KillTimer() override {}

    IMS_UINTP GetTimerId() const override { return reinterpret_cast<IMS_UINTP>(this); }

    void DispatchServiceMessage(IN IMS_UINTP /*nWparam*/, IN IMS_UINTP /*nLparam*/) override {}
};

class SessionInterfaceHolderTest : public ::testing::Test
{
public:
    MockIOsFactory objMockIOsFactory;
    IOsFactory* piOldOsFactory;
    TestImsTimerForSiHolder* pTestImsTimerForSiHolder;
    SessionInterfaceHolder* pHolder;
    MockIInterfaceHolderListener objListener;
    MockISession objMockISession;

protected:
    virtual void SetUp() override { pHolder = new SessionInterfaceHolder(objListener); }

    virtual void TearDown() override { delete pHolder; }
};

TEST_F(SessionInterfaceHolderTest, HolderDoesNothingForSessionListenerExceptTerminated)
{
    MockIReference objMockIReference;
    MockISipServerConnection objMockServerConnection;

    EXPECT_CALL(objMockISession, Destroy).Times(0);

    pHolder->SessionAlerting(&objMockISession);
    pHolder->SessionReferenceReceived(&objMockISession, &objMockIReference);
    pHolder->SessionStarted(&objMockISession);
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

TEST_F(SessionInterfaceHolderTest, StopsTimerIfSessionTerminated)
{
    EXPECT_CALL(objMockISession, Destroy()).Times(1);

    pHolder->AddISession(&objMockISession);
    pHolder->SessionTerminated(&objMockISession);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 0);
}

TEST_F(SessionInterfaceHolderTest, AddAndReleaseStartsTimer)
{
    ON_CALL(objMockISession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());

    EXPECT_EQ(pHolder->GetSessionCount(), 0);

    pHolder->AddISession(&objMockISession);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 1);

    pHolder->ReleaseISession(&objMockISession, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 1);
}

TEST_F(SessionInterfaceHolderTest, AddAndReleaseWithTerminatedStopsTimer)
{
    ON_CALL(objMockISession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());
    EXPECT_CALL(objMockISession, Destroy()).Times(1);

    EXPECT_EQ(pHolder->GetSessionCount(), 0);

    pHolder->AddISession(&objMockISession);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 1);

    pHolder->ReleaseISession(&objMockISession, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISession));

    pHolder->ReleaseISession(&objMockISession, IMS_TRUE);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 0);
}

TEST_F(SessionInterfaceHolderTest, GetAndReleaseStartsTimer)
{
    ON_CALL(objMockISession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());

    MockICoreService objMockICoreService;
    ON_CALL(objMockICoreService, CreateSession(_, _)).WillByDefault(Return(&objMockISession));

    EXPECT_EQ(pHolder->GetSessionCount(), 0);

    ISession* piSession = pHolder->GetISession(&objMockICoreService, "sip:fromuri", "sip:touri");
    EXPECT_EQ(piSession, &objMockISession);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));

    pHolder->ReleaseISession(&objMockISession, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 1);
}

TEST_F(SessionInterfaceHolderTest, GetAndReleaseWithTimerExpiredStopsTimer)
{
    piOldOsFactory = PlatformContext::GetInstance()->SetOsFactory(&objMockIOsFactory);

    // will be deleted in the TimerService::DestroyTimer.
    pTestImsTimerForSiHolder = new TestImsTimerForSiHolder();
    EXPECT_CALL(objMockIOsFactory, CreateTimer())
            .Times(1)
            .WillOnce(Return(pTestImsTimerForSiHolder));

    ON_CALL(objMockISession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());

    MockICoreService objMockICoreService;
    ON_CALL(objMockICoreService, CreateSession(_, _)).WillByDefault(Return(&objMockISession));

    EXPECT_EQ(pHolder->GetSessionCount(), 0);

    ISession* piSession = pHolder->GetISession(&objMockICoreService, "sip:fromuri", "sip:touri");
    EXPECT_EQ(piSession, &objMockISession);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));

    pHolder->ReleaseISession(&objMockISession, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 1);

    pHolder->Timer_TimerExpired(pTestImsTimerForSiHolder);

    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 0);

    PlatformContext::GetInstance()->SetOsFactory(piOldOsFactory);
}

TEST_F(SessionInterfaceHolderTest, GetAndReleaseWithTerminatedStopsTimer)
{
    ON_CALL(objMockISession, GetState).WillByDefault(Return(ISession::STATE_ESTABLISHED));
    ON_CALL(objMockISession, Destroy()).WillByDefault(Return());
    EXPECT_CALL(objMockISession, Destroy()).Times(1);

    MockICoreService objMockICoreService;
    ON_CALL(objMockICoreService, CreateSession(_, _)).WillByDefault(Return(&objMockISession));

    EXPECT_EQ(pHolder->GetSessionCount(), 0);

    ISession* piSession = pHolder->GetISession(&objMockICoreService, "sip:fromuri", "sip:touri");
    EXPECT_EQ(piSession, &objMockISession);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));

    pHolder->ReleaseISession(&objMockISession, IMS_FALSE);
    EXPECT_TRUE(pHolder->IsTimerExist(&objMockISession));

    pHolder->ReleaseISession(&objMockISession, IMS_TRUE);
    EXPECT_FALSE(pHolder->IsTimerExist(&objMockISession));
    EXPECT_EQ(pHolder->GetSessionCount(), 0);
}

}  // namespace android
