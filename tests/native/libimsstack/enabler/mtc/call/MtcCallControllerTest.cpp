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

#include "Engine.h"
#include "IConfiguration.h"
#include "ImsList.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "PlatformContext.h"
#include "TestConfigService.h"
#include "call/ISilentRedialHelper.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/MtcCallController.h"
#include "call/termination/ByeTransactionHandler.h"
#include "conferencecall/MockIConferenceController.h"
#include "conferencecall/MockIConferenceManager.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "ect/MockIEctManager.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "helper/OperationAsyncRunner.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSessionInterfaceHolder.h"
#include "media/MockIMtcMediaManager.h"
#include <functional>
#include <gtest/gtest.h>
#include <initializer_list>
#include <memory>

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Eq;
using ::testing::Invoke;
using ::testing::Ref;
using ::testing::Return;
using ::testing::ReturnRef;

class MtcCallControllerTest : public ::testing::Test
{
public:
    MtcCallController* pCallController;
    MockIConferenceManager objConferenceManager;
    MockIMtcCallContext objContext;
    MockIMtcCallManager objCallManager;
    MockIEctManager objEctManager;
    MockICallStateProxy objCallStateProxy;
    MockMtcConfigurationProxy objConfigurationProxy;
    MockIMtcMediaManager objMediaManager;
    MockIPassiveTimerHolder objPassiveTimer;
    MockIMtcSession objMtcSession;
    MockISession objISession;
    MockIMtcSipInterfaceFactory objSipInterfaceFactory;
    MockSessionInterfaceHolder* pSessionInterfaceHolder;
    TestConfigService objConfigService;
    MediaInfo objMediaInfo;

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &objConfigService);
        Engine::GetConfiguration()->RefreshConfigs(IMS_SLOT_0);
        pSessionInterfaceHolder = new MockSessionInterfaceHolder();

        ON_CALL(objContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objContext, GetConferenceManager).WillByDefault(ReturnRef(objConferenceManager));
        ON_CALL(objContext, GetEctManager).WillByDefault(ReturnRef(objEctManager));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objISession));
        ON_CALL(objContext, GetSipInterfaceFactory)
                .WillByDefault(ReturnRef(objSipInterfaceFactory));
        ON_CALL(objSipInterfaceFactory, GetISessionHolder)
                .WillByDefault(ReturnRef(*pSessionInterfaceHolder));
        ON_CALL(objMediaManager, GetMediaInfo(Ref(objISession)))
                .WillByDefault(ReturnRef(objMediaInfo));

        pCallController = new MtcCallController(objContext);
    }

    virtual void TearDown() override
    {
        delete pSessionInterfaceHolder;
        delete pCallController;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
    }

    MockIMtcCall* CreateMockIMtcCall(CallKey nKey)
    {
        MockIMtcCall* pCall = new MockIMtcCall();
        ON_CALL(*pCall, GetKey()).WillByDefault(Return(nKey));
        return pCall;
    }
};

TEST_F(MtcCallControllerTest, NotifyJniEnablerSetDoesNothing)
{
    pCallController->NotifyJniEnablerSet();
}

TEST_F(MtcCallControllerTest, OpenCreatesCall)
{
    ServiceType eServiceType = ServiceType::NORMAL;
    CallInfo objCallInfo;

    CallKey nCallKey = 1;
    MockIMtcCall* pCall = CreateMockIMtcCall(nCallKey);

    EXPECT_CALL(objCallManager, CreateCall(eServiceType, Eq(std::ref(objCallInfo)), _))
            .Times(1)
            .WillRepeatedly(Return(pCall));

    pCallController->Open(eServiceType, objCallInfo, AString::ConstNull());

    delete pCall;
}

TEST_F(MtcCallControllerTest, OpenReturnsCreatedCallKey)
{
    CallKey nCallKey = 1;
    MockIMtcCall* pCall = CreateMockIMtcCall(nCallKey);

    ON_CALL(objCallManager, CreateCall(_, _, _)).WillByDefault(Return(pCall));

    ServiceType eServiceType = ServiceType::NORMAL;
    CallInfo objCallInfo;
    EXPECT_EQ(nCallKey, pCallController->Open(eServiceType, objCallInfo, AString::ConstNull()));

    delete pCall;
}

TEST_F(MtcCallControllerTest, AttachAttachesTargetCall)
{
    CallKey nCallKey = 1;
    MockIMtcCall* pCall = CreateMockIMtcCall(nCallKey);
    EXPECT_CALL(*pCall, Attach()).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(pCall));

    pCallController->Attach(nCallKey);

    delete pCall;
}

TEST_F(MtcCallControllerTest, DetachInvokesRemoveCallUsingAsyncRunner)
{
    CallKey nCallKey = 1;
    EXPECT_CALL(objContext, RunAsyncOperation(_, _))
            .WillOnce(Invoke(
                    []([[maybe_unused]] void* pOwner, std::function<void()> objOperation)
                    {
                        objOperation();
                    }));
    EXPECT_CALL(objCallManager, RemoveCall(nCallKey)).Times(1);

    pCallController->Detach(nCallKey);
}

TEST_F(MtcCallControllerTest, HandleIncomingCreatesCall)
{
    ServiceType eServiceType = ServiceType::NORMAL;

    MockIMtcService objService;
    ON_CALL(objService, GetServiceType).WillByDefault(Return(eServiceType));

    MockIMtcCall objCall;
    EXPECT_CALL(objCallManager, CreateCall(_, _, _)).Times(1).WillRepeatedly(Return(&objCall));

    pCallController->HandleIncoming(&objService, IMS_NULL);
}

TEST_F(MtcCallControllerTest, HandleIncomingInvokesTargetCall)
{
    MockIMtcService objService;
    ISession* pSession = reinterpret_cast<ISession*>(0x1);

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, HandleIncoming(pSession)).Times(1);

    ON_CALL(objCallManager, CreateCall(_, _, _)).WillByDefault(Return(&objCall));

    pCallController->HandleIncoming(&objService, pSession);
}

TEST_F(MtcCallControllerTest, StartInvokesTargetCall)
{
    CallKey nCallKey = 1;
    CallType eCallType = CallType::VOIP;
    AString strTarget = "target";
    MediaInfo objMediaInfo;
    ImsList<SuppService*> objSuppServices;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Start(eCallType, strTarget, objMediaInfo, _)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->Start(nCallKey, eCallType, strTarget, objMediaInfo, objSuppServices);
}

TEST_F(MtcCallControllerTest, HandleUserAlertInvokesTargetCall)
{
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, HandleUserAlert).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->HandleUserAlert(nCallKey);
}

TEST_F(MtcCallControllerTest, AcceptInvokesTargetCall)
{
    CallKey nCallKey = 1;
    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Accept(eCallType, objMediaInfo)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->Accept(nCallKey, eCallType, objMediaInfo);
}

TEST_F(MtcCallControllerTest, RejectInvokesTargetCall)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Reject(objReason)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->Reject(nCallKey, objReason);
}

TEST_F(MtcCallControllerTest, HoldInvokesTargetCall)
{
    CallKey nCallKey = 1;
    MediaInfo objMediaInfo;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Hold(objMediaInfo)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->Hold(nCallKey, objMediaInfo);
}

TEST_F(MtcCallControllerTest, ResumeInvokesTargetCall)
{
    CallKey nCallKey = 1;
    MediaInfo objMediaInfo;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Resume(objMediaInfo)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->Resume(nCallKey, objMediaInfo);
}

TEST_F(MtcCallControllerTest, AcceptResumeInvokesTargetCall)
{
    CallKey nCallKey = 1;
    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, AcceptResume(eCallType, objMediaInfo)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->AcceptResume(nCallKey, eCallType, objMediaInfo);
}

TEST_F(MtcCallControllerTest, RejectResumeInvokesTargetCall)
{
    CallKey nCallKey = 1;
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, RejectResume(objReason)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->RejectResume(nCallKey, objReason);
}

TEST_F(MtcCallControllerTest, TerminateInvokesTerminateUsingAsyncRunner)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
    CallKey nCallKey = 1;
    MockIMtcCall objCall;
    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    EXPECT_CALL(objContext, RunAsyncOperation(_, _))
            .WillOnce(Invoke(
                    []([[maybe_unused]] void* pOwner, std::function<void()> objOperation)
                    {
                        objOperation();
                    }));

    EXPECT_CALL(objCall, Terminate(objReason)).Times(1);

    pCallController->Terminate(nCallKey, objReason);
}

TEST_F(MtcCallControllerTest, UpdateInvokesTargetCall)
{
    CallKey nCallKey = 1;
    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Update(eCallType, objMediaInfo)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->Update(nCallKey, eCallType, objMediaInfo);
}

TEST_F(MtcCallControllerTest, CancelUpdateInvokesTargetCall)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, CancelUpdate(objReason)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->CancelUpdate(nCallKey, objReason);
}

TEST_F(MtcCallControllerTest, AcceptUpdateInvokesTargetCall)
{
    CallKey nCallKey = 1;
    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, AcceptUpdate(eCallType, objMediaInfo)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->AcceptUpdate(nCallKey, eCallType, objMediaInfo);
}

TEST_F(MtcCallControllerTest, RejectUpdateInvokesTargetCall)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, RejectUpdate(objReason)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->RejectUpdate(nCallKey, objReason);
}

TEST_F(MtcCallControllerTest, SendUssdInvokesTargetCall)
{
    AString strUssd = "ussd";
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, SendUssd(strUssd)).Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey)).WillByDefault(Return(&objCall));

    pCallController->SendUssd(nCallKey, strUssd);
}

TEST_F(MtcCallControllerTest, MergeToConferenceCallsProcessesMerge)
{
    CallKey nCallKey = 1;
    ImsList<ConfUser*> objUsers;

    MockIConferenceController objConferenceController;
    EXPECT_CALL(objConferenceController, ProcessCommand(IConferenceController::MERGE, _)).Times(1);

    EXPECT_CALL(objConferenceManager, GetController(nCallKey))
            .Times(AnyNumber())
            .WillRepeatedly(Return(&objConferenceController));

    pCallController->MergeToConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, MergeToConferenceCreatesControllerIfNull)
{
    CallKey nCallKey = 1;

    ON_CALL(objConferenceManager, GetController(nCallKey)).WillByDefault(Return(nullptr));

    MockIConferenceController objConferenceController;
    EXPECT_CALL(objConferenceManager, CreateController(nCallKey, ConferenceType::MERGE_CALL))
            .Times(1)
            .WillRepeatedly(ReturnRef(objConferenceController));

    ImsList<ConfUser*> objUsers;
    pCallController->MergeToConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, AddToConferenceCallsProcessesAdd)
{
    CallKey nCallKey = 1;
    ImsList<ConfUser*> objUsers;

    MockIConferenceController objConferenceController;
    EXPECT_CALL(objConferenceController, ProcessCommand(IConferenceController::ADD, _)).Times(1);

    EXPECT_CALL(objConferenceManager, GetController(nCallKey))
            .Times(AnyNumber())
            .WillRepeatedly(Return(&objConferenceController));

    pCallController->AddToConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, AddToConferenceDoNothingIfControllerNull)
{
    CallKey nCallKey = 1;

    ON_CALL(objConferenceManager, GetController(nCallKey)).WillByDefault(Return(nullptr));
    EXPECT_CALL(objConferenceManager, CreateController(_, _)).Times(0);

    ImsList<ConfUser*> objUsers;
    pCallController->AddToConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, RemoveFromConferenceCallsProcessesRemove)
{
    CallKey nCallKey = 1;
    ImsList<ConfUser*> objUsers;

    MockIConferenceController objConferenceController;
    EXPECT_CALL(objConferenceController, ProcessCommand(IConferenceController::REMOVE, _)).Times(1);

    EXPECT_CALL(objConferenceManager, GetController(nCallKey))
            .Times(AnyNumber())
            .WillRepeatedly(Return(&objConferenceController));

    pCallController->RemoveFromConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, RemoveFromConferenceDoNothingIfControllerNull)
{
    CallKey nCallKey = 1;

    ON_CALL(objConferenceManager, GetController(nCallKey)).WillByDefault(Return(nullptr));
    EXPECT_CALL(objConferenceManager, CreateController(_, _)).Times(0);

    ImsList<ConfUser*> objUsers;
    pCallController->RemoveFromConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, TransferCallsEctManager)
{
    CallKey nCallKey = 1;
    AString strTarget = "target";

    EXPECT_CALL(objEctManager, Transfer(nCallKey, strTarget));

    pCallController->Transfer(nCallKey, strTarget);
}

TEST_F(MtcCallControllerTest, HandleByeTransactionAddsListenerToSessionHolder)
{
    CallKey nCallKey = 1;
    std::function<void(ISession&)> objOperation = [](ISession&)
    {
    };

    EXPECT_CALL(*pSessionInterfaceHolder, AddListener(_)).Times(1);

    pCallController->HandleByeTransaction(nCallKey, objOperation);

    EXPECT_CALL(*pSessionInterfaceHolder, RemoveListener(_)).Times(0);
}

TEST_F(MtcCallControllerTest, OnByeTransactionCompletedRemovesListenerAndDeletesHandler)
{
    CallKey nCallKey = 1;
    EXPECT_CALL(*pSessionInterfaceHolder, RemoveListener(_));

    std::function<void(ISession&)> objOperation = [](ISession&)
    {
    };

    pCallController->HandleByeTransaction(nCallKey, objOperation);

    // Pass a dummy handler to trigger ClearByeTransactionHandlers.
    // The handler itself is not managed by the controller, so it must be deleted manually.
    std::unique_ptr<ByeTransactionHandler> pHandler =
            std::make_unique<ByeTransactionHandler>(nCallKey, *pCallController,
                    [](ISession&)
                    {
                    });
    pCallController->OnByeTransactionCompleted(pHandler.get());
}

TEST_F(MtcCallControllerTest, OnByeTransactionCompletedRemovesListenerAndDeletesAllHandlers)
{
    CallKey nCallKey = 1;
    std::function<void(ISession&)> objOperation = [](ISession&)
    {
    };

    EXPECT_CALL(*pSessionInterfaceHolder, AddListener(_)).Times(2);

    pCallController->HandleByeTransaction(nCallKey, objOperation);
    pCallController->HandleByeTransaction(nCallKey, objOperation);

    EXPECT_CALL(*pSessionInterfaceHolder, RemoveListener(_)).Times(2);

    std::unique_ptr<ByeTransactionHandler> pHandler =
            std::make_unique<ByeTransactionHandler>(nCallKey, *pCallController,
                    [](ISession&)
                    {
                    });
    pCallController->OnByeTransactionCompleted(pHandler.get());
}

TEST_F(MtcCallControllerTest, GetRedialHelperCreatesSilentRedialHelper)
{
    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE);
    const ISilentRedialHelper& objRedialHelper =
            pCallController->GetRedialHelper(objContext, objReason);
    EXPECT_NE(&objRedialHelper, nullptr);
}

TEST_F(MtcCallControllerTest, GetRedialHelperWithSameReasonDoesNotCreatesSilentRedialHelper)
{
    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE);
    const ISilentRedialHelper& objRedialHelper1 =
            pCallController->GetRedialHelper(objContext, objReason);
    const ISilentRedialHelper& objRedialHelper2 =
            pCallController->GetRedialHelper(objContext, objReason);
    EXPECT_EQ(&objRedialHelper1, &objRedialHelper2);
}

TEST_F(MtcCallControllerTest, GetRedialHelperWithDifferentTypeCreatesSilentRedialHelper)
{
    const CallReasonInfo objReason1(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE);
    const CallReasonInfo objReason2(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_BY_RETRY_AFTER);
    const ISilentRedialHelper& objRedialHelper1 =
            pCallController->GetRedialHelper(objContext, objReason1);
    IMS_UINT32 nType1 = objRedialHelper1.GetType();
    const ISilentRedialHelper& objRedialHelper2 =
            pCallController->GetRedialHelper(objContext, objReason2);
    EXPECT_NE(nType1, objRedialHelper2.GetType());
}

TEST_F(MtcCallControllerTest, ReleaseRedialHelperDeletesSilentRedialHelper)
{
    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE);
    const ISilentRedialHelper& objRedialHelper1 =
            pCallController->GetRedialHelper(objContext, objReason);
    IMS_UINT32 nType1 = objRedialHelper1.GetType();
    pCallController->ReleaseRedialHelper();
    const ISilentRedialHelper& objRedialHelper2 =
            pCallController->GetRedialHelper(objContext, objReason);
    // nothing to check : cannot check address
    EXPECT_EQ(nType1, objRedialHelper2.GetType());
}

TEST_F(MtcCallControllerTest, GetActiveRedialHelper)
{
    pCallController->ReleaseRedialHelper();
    const ISilentRedialHelper* pNullRedialHelper = pCallController->GetActiveRedialHelper();
    EXPECT_EQ(pNullRedialHelper, nullptr);

    const CallReasonInfo objReason(CODE_INTERNAL_REDIAL, EXTRA_CODE_REDIAL_FOR_SDP_CHANGE);
    const ISilentRedialHelper& objRedialHelper =
            pCallController->GetRedialHelper(objContext, objReason);
    const ISilentRedialHelper* pActiveRedialHelper = pCallController->GetActiveRedialHelper();
    EXPECT_EQ(&objRedialHelper, pActiveRedialHelper);
}
