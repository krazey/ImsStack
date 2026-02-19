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

#include "MockIMtcContext.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "SipStatusCode.h"
#include "TestTimerService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcUiNotifier.h"
#include "ect/EctController.h"
#include "ect/EctFactory.h"
#include "ect/MockIEctControllerListener.h"
#include "helper/MockICallStateProxy.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL CallKey TRANSFEREE_KEY = 200;

namespace android
{

class EctControllerTest : public ::testing::Test
{
public:
    inline EctControllerTest() :
            pController(IMS_NULL),
            pTimerService(new TestTimerService()),
            objTimer(pTimerService->GetMockTimer())
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, pTimerService);
    }
    inline virtual ~EctControllerTest()
    {
        delete pController;
        delete pTimerService;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

    EctController* pController;
    TestTimerService* pTimerService;
    EctFactory objFactory;

    MockIMtcContext objContext;
    MockITimer& objTimer;
    MockIEctControllerListener objListener;
    MockIMtcCallManager objCallManager;
    MockIMtcUiNotifier objNotifier;
    MockIMtcCall objTransfereeCall;
    MockIMtcCallContext objTransfereeCallContext;
    MockICallStateProxy objCallStateProxy;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objCallManager, GetCallByCallKey(TRANSFEREE_KEY))
                .WillByDefault(Return(&objTransfereeCall));
        ON_CALL(objTransfereeCall, GetCallContext)
                .WillByDefault(ReturnRef(objTransfereeCallContext));
        ON_CALL(objTransfereeCall, GetKey).WillByDefault(Return(TRANSFEREE_KEY));
        ON_CALL(objTransfereeCallContext, GetUiNotifier).WillByDefault(ReturnRef(objNotifier));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));

        pController = new EctController(objContext, TRANSFEREE_KEY, objListener, objFactory);
    }

    virtual void TearDown() override {}
};

TEST_F(EctControllerTest, OnReferenceStartedDoesNothing)
{
    EXPECT_CALL(objListener, OnEctCompleted).Times(0);
    EXPECT_CALL(objTimer, KillTimer).Times(0);

    pController->OnReferenceStarted();
}

TEST_F(EctControllerTest, OnReferenceStartFailedNotifiesFailure)
{
    EXPECT_CALL(objListener, OnEctCompleted);
    EXPECT_CALL(objNotifier, SendEctCompleted(IMS_FAILURE, CallReasonInfo(CODE_USER_TERMINATED)));

    pController->OnReferenceStartFailed();
}

TEST_F(EctControllerTest, OnReferenceUpdatedSuccessNotifiesSuccess)
{
    EXPECT_CALL(objListener, OnEctCompleted);
    EXPECT_CALL(objNotifier, SendEctCompleted(IMS_SUCCESS, CallReasonInfo(CODE_USER_TERMINATED)));
    EXPECT_CALL(objTransfereeCall,
            Terminate(CallReasonInfo(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_ECT)));

    pController->OnReferenceUpdated(SipStatusCode::SC_202);
}

TEST_F(EctControllerTest, OnReferenceUpdatedFailureNotifiesFailure)
{
    EXPECT_CALL(objListener, OnEctCompleted);
    EXPECT_CALL(objNotifier, SendEctCompleted(IMS_FAILURE, CallReasonInfo(CODE_USER_TERMINATED)));

    pController->OnReferenceUpdated(SipStatusCode::SC_415);
}

TEST_F(EctControllerTest, TimerExpiredDoesNothingBeforeStartTimer)
{
    EXPECT_CALL(objListener, OnEctCompleted).Times(0);
    EXPECT_CALL(objNotifier, SendEctCompleted(_, _)).Times(0);
    EXPECT_CALL(objTimer, KillTimer).Times(0);

    pController->Timer_TimerExpired(&objTimer);
}

TEST_F(EctControllerTest, TransferDoesNothing)
{
    AString strNumber("12345");
    pController->Transfer(strNumber);
    pController->Transfer();
}

TEST_F(EctControllerTest, TimerExpiredNotifiesFailure)
{
    EXPECT_CALL(objListener, OnEctCompleted);
    EXPECT_CALL(objNotifier, SendEctCompleted(IMS_FAILURE, CallReasonInfo(CODE_USER_TERMINATED)));
    pController->Timer_TimerExpired(IMS_NULL);
}

}  // namespace android
