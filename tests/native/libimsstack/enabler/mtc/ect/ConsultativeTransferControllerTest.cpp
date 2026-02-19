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

#include "ImsList.h"
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
#include "ect/ConsultativeTransferController.h"
#include "ect/EctFactory.h"
#include "ect/MockEctReference.h"
#include "ect/MockIEctControllerListener.h"
#include "helper/MockICallStateProxy.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockReferenceInterfaceHolder.h"
#include <gtest/gtest.h>

// EctController::TIME_WAIT_OPERATION_COMPLETE
LOCAL IMS_UINT32 TIME_WAIT_OPERATION_COMPLETE = 32000;

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL CallKey TRANSFEREE_KEY = 200;
LOCAL CallKey TRANSFER_TARGET_KEY = 300;

namespace android
{

class ConsultativeTransferControllerTest : public ::testing::Test
{
public:
    inline ConsultativeTransferControllerTest() :
            pController(IMS_NULL),
            pTimerService(new TestTimerService()),
            objTimer(pTimerService->GetMockTimer()),
            pMockReferenceInterfaceHolder(IMS_NULL)
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, pTimerService);
    }
    inline virtual ~ConsultativeTransferControllerTest()
    {
        delete pController;
        delete pTimerService;
        delete pMockReferenceInterfaceHolder;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

    ConsultativeTransferController* pController;
    TestTimerService* pTimerService;
    EctFactory objFactory;
    std::unique_ptr<MockEctReference> pReference;

    MockIMtcContext objContext;
    MockITimer& objTimer;
    MockIEctControllerListener objListener;
    MockIMtcCallManager objCallManager;
    MockIMtcUiNotifier objNotifier;
    MockIMtcCall objTransfereeCall;
    MockIMtcCall objTransferTargetCall;
    MockIMtcCallContext objTransfereeCallContext;
    MockIMtcSipInterfaceFactory objMockInterfaceFactory;
    MockReferenceInterfaceHolder* pMockReferenceInterfaceHolder;
    MockIInterfaceHolderListener objMockHolderListener;
    ImsList<IMtcCall*> objManagedCalls;
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

        ON_CALL(objCallManager, GetCallByCallKey(TRANSFER_TARGET_KEY))
                .WillByDefault(Return(&objTransferTargetCall));
        ON_CALL(objTransferTargetCall, GetKey).WillByDefault(Return(TRANSFER_TARGET_KEY));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));

        pController = new ConsultativeTransferController(
                objContext, TRANSFEREE_KEY, objListener, objFactory);
    }

    virtual void TearDown() override { objManagedCalls.Clear(); }

    MockEctReference* GetMockReference()
    {
        // For ~EctReference()
        ON_CALL(objContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(objMockInterfaceFactory));
        pMockReferenceInterfaceHolder = new MockReferenceInterfaceHolder(objMockHolderListener);
        ON_CALL(objMockInterfaceFactory, GetIReferenceHolder)
                .WillByDefault(Return(pMockReferenceInterfaceHolder));
        ON_CALL(*pMockReferenceInterfaceHolder, ReleaseIReference(_, _)).WillByDefault(Return());

        pReference = std::make_unique<MockEctReference>(objContext, TRANSFEREE_KEY, *pController);
        MockEctReference* pRawPtr = pReference.get();
        objFactory.SetReference(std::move(pReference));
        return pRawPtr;
    }

    void SetUpCallsForSuccessfulCase()
    {
        objManagedCalls.Append(&objTransfereeCall);
        objManagedCalls.Append(&objTransferTargetCall);
        ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objManagedCalls));
    }
};

TEST_F(ConsultativeTransferControllerTest, TransferFailsIfCallCountIsNotTwo)
{
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objManagedCalls));

    EXPECT_CALL(objNotifier, SendEctCompleted(IMS_FAILURE, CallReasonInfo(CODE_USER_TERMINATED)))
            .Times(1);
    pController->Transfer();

    objManagedCalls.Append(&objTransfereeCall);
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objManagedCalls));
    EXPECT_CALL(objNotifier, SendEctCompleted(IMS_FAILURE, CallReasonInfo(CODE_USER_TERMINATED)))
            .Times(1);
    pController->Transfer();
}

TEST_F(ConsultativeTransferControllerTest, TransferFailsIfSendingReferenceFailed)
{
    SetUpCallsForSuccessfulCase();

    ON_CALL(*GetMockReference(), SendInvite(TRANSFER_TARGET_KEY))
            .WillByDefault(Return(IMS_FAILURE));

    EXPECT_CALL(objNotifier, SendEctCompleted(IMS_FAILURE, CallReasonInfo(CODE_USER_TERMINATED)))
            .Times(1);

    pController->Transfer();
}

TEST_F(ConsultativeTransferControllerTest, SuccessfulTransferStartsTimer)
{
    SetUpCallsForSuccessfulCase();

    ON_CALL(*GetMockReference(), SendInvite(TRANSFER_TARGET_KEY))
            .WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(objTimer, SetTimer(TIME_WAIT_OPERATION_COMPLETE, _)).Times(1);

    pController->Transfer();
}

TEST_F(ConsultativeTransferControllerTest, OnReferenceUpdatedSuccessNotifiesSuccess)
{
    SetUpCallsForSuccessfulCase();
    ON_CALL(*GetMockReference(), SendInvite(TRANSFER_TARGET_KEY))
            .WillByDefault(Return(IMS_SUCCESS));

    pController->Transfer();

    EXPECT_CALL(objListener, OnEctCompleted);
    EXPECT_CALL(objNotifier, SendEctCompleted(IMS_SUCCESS, CallReasonInfo(CODE_USER_TERMINATED)));
    EXPECT_CALL(objTransfereeCall,
            Terminate(CallReasonInfo(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_ECT)));
    EXPECT_CALL(objTransferTargetCall,
            Terminate(CallReasonInfo(CODE_USER_TERMINATED, EXTRA_USER_TERMINATED_ECT)));
    EXPECT_CALL(objTimer, KillTimer).Times(1);

    pController->OnReferenceUpdated(SipStatusCode::SC_202);
}

TEST_F(ConsultativeTransferControllerTest, OnReferenceUpdatedFailureNotifiesFailure)
{
    SetUpCallsForSuccessfulCase();
    ON_CALL(*GetMockReference(), SendInvite(TRANSFER_TARGET_KEY))
            .WillByDefault(Return(IMS_SUCCESS));

    pController->Transfer();

    EXPECT_CALL(objListener, OnEctCompleted);
    EXPECT_CALL(objNotifier, SendEctCompleted(IMS_FAILURE, CallReasonInfo(CODE_USER_TERMINATED)));
    EXPECT_CALL(objTimer, KillTimer).Times(1);

    pController->OnReferenceUpdated(SipStatusCode::SC_415);
}

}  // namespace android
