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

#include <functional>
#include <initializer_list>
#include <gtest/gtest.h>
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MtcCallController.h"
#include "call/MockIMtcCall.h"
#include "conferencecall/MockIConferenceController.h"
#include "conferencecall/MockIConferenceManager.h"
#include "ect/MockIEctManager.h"
#include "helper/MockICallStateProxy.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Eq;
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

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallManager)
                .WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objContext, GetConferenceManager)
                .WillByDefault(ReturnRef(objConferenceManager));
        ON_CALL(objContext, GetEctManager)
                .WillByDefault(Return(&objEctManager));

        pCallController = new MtcCallController(objContext);
    }

    virtual void TearDown() override
    {
        delete pCallController;
    }

    IMSList<IMtcCall*> CreateCallList(std::initializer_list<IMtcCall*> lstCalls)
    {
        IMSList<IMtcCall*> lstOut;

        for (IMtcCall* pCall : lstCalls)
        {
            lstOut.Append(pCall);
        }

        return lstOut;
    }

    MockIMtcCall* CreateMockIMtcCall(CallKey nKey)
    {
        MockIMtcCall* pCall = new MockIMtcCall();
        ON_CALL(*pCall, GetKey())
                .WillByDefault(Return(nKey));
        return pCall;
    }
};

TEST_F(MtcCallControllerTest, TerminateCallsByNoneKeyTerminatesTargetCalls)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCall objCall1;
    MockIMtcCall objCall2;
    EXPECT_CALL(objCall1, Terminate(objReason))
            .Times(1);
    EXPECT_CALL(objCall2, Terminate(objReason))
            .Times(1);

    IMSList<IMtcCall*> lstCalls = CreateCallList({&objCall1, &objCall2});
    ON_CALL(objCallManager, GetCalls)
            .WillByDefault(Return(lstCalls));

    Key nKey;
    pCallController->TerminateCalls(KeyType::NONE, nKey, CallReasonInfo(objReason));
}

TEST_F(MtcCallControllerTest, TerminateCallsByCallKeyTerminatesTargetCalls)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Terminate(objReason))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    Key nKey;
    nKey.nCallKey = nCallKey;
    pCallController->TerminateCalls(KeyType::CALL_KEY, nKey, CallReasonInfo(objReason));
}

TEST_F(MtcCallControllerTest, TerminateCallsByCallTypeTerminatesTargetCalls)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
    CallType eCallType = CallType::VOIP;

    MockIMtcCall objCall1;
    MockIMtcCall objCall2;
    EXPECT_CALL(objCall1, Terminate(objReason))
            .Times(1);
    EXPECT_CALL(objCall2, Terminate(objReason))
            .Times(1);

    IMSList<IMtcCall*> lstCalls = CreateCallList({&objCall1, &objCall2});
    ON_CALL(objCallManager, GetCallsByType(eCallType))
            .WillByDefault(Return(lstCalls));

    Key nKey;
    nKey.eCallType = eCallType;
    pCallController->TerminateCalls(KeyType::CALL_TYPE, nKey, CallReasonInfo(objReason));
}

TEST_F(MtcCallControllerTest, TerminateCallsByServiceTypeTerminatesTargetCalls)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);

    MockIMtcCall objCall1;
    MockIMtcCall objCall2;
    EXPECT_CALL(objCall1, Terminate(objReason))
            .Times(1);
    EXPECT_CALL(objCall2, Terminate(objReason))
            .Times(1);

    ServiceType eServiceType = ServiceType::NORMAL;

    IMSList<IMtcCall*> lstCalls = CreateCallList({&objCall1, &objCall2});
    ON_CALL(objCallManager, GetCallsByServiceType(eServiceType))
            .WillByDefault(Return(lstCalls));

    Key nKey;
    nKey.eServiceType = eServiceType;
    pCallController->TerminateCalls(KeyType::SERVICE_TYPE, nKey, CallReasonInfo(objReason));
}

TEST_F(MtcCallControllerTest, RemoveCallsByNoneKeyRemovesTargetCalls)
{
    CallKey nCallKey1 = 1;
    MockIMtcCall* pCall1 = CreateMockIMtcCall(nCallKey1);

    CallKey nCallKey2 = 2;
    MockIMtcCall* pCall2 = CreateMockIMtcCall(nCallKey2);

    IMSList<IMtcCall*> lstCalls = CreateCallList({pCall1, pCall2});
    ON_CALL(objCallManager, GetCalls)
            .WillByDefault(Return(lstCalls));
    EXPECT_CALL(objCallManager, RemoveCall(pCall1->GetKey()))
            .Times(1);
    EXPECT_CALL(objCallManager, RemoveCall(pCall2->GetKey()))
            .Times(1);

    Key nKey;
    pCallController->RemoveCalls(KeyType::NONE, nKey);

    delete pCall1;
    delete pCall2;
}

TEST_F(MtcCallControllerTest, RemoveCallsByCallKeyRemovesTargetCalls)
{
    CallKey nCallKey = 1;
    MockIMtcCall* pCall = CreateMockIMtcCall(nCallKey);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(pCall));
    EXPECT_CALL(objCallManager, RemoveCall(pCall->GetKey()))
            .Times(1);

    Key nKey;
    nKey.nCallKey = nCallKey;
    pCallController->RemoveCalls(KeyType::CALL_KEY, nKey);

    delete pCall;
}

TEST_F(MtcCallControllerTest, RemoveCallsByCallTypeRemovesTargetCalls)
{
    CallKey nCallKey1 = 1;
    MockIMtcCall* pCall1 = CreateMockIMtcCall(nCallKey1);

    CallKey nCallKey2 = 2;
    MockIMtcCall* pCall2 = CreateMockIMtcCall(nCallKey2);

    CallType eCallType = CallType::VOIP;

    IMSList<IMtcCall*> lstCalls = CreateCallList({pCall1, pCall2});
    ON_CALL(objCallManager, GetCallsByType(eCallType))
            .WillByDefault(Return(lstCalls));
    EXPECT_CALL(objCallManager, RemoveCall(pCall1->GetKey()))
            .Times(1);
    EXPECT_CALL(objCallManager, RemoveCall(pCall2->GetKey()))
            .Times(1);

    Key nKey;
    nKey.eCallType = eCallType;
    pCallController->RemoveCalls(KeyType::CALL_TYPE, nKey);

    delete pCall1;
    delete pCall2;
}

TEST_F(MtcCallControllerTest, RemoveCallsByServiceTypeRemovesTargetCalls)
{
    CallKey nCallKey1 = 1;
    MockIMtcCall* pCall1 = CreateMockIMtcCall(nCallKey1);

    CallKey nCallKey2 = 2;
    MockIMtcCall* pCall2 = CreateMockIMtcCall(nCallKey2);

    ServiceType eServiceType = ServiceType::NORMAL;

    IMSList<IMtcCall*> lstCalls = CreateCallList({pCall1, pCall2});
    ON_CALL(objCallManager, GetCallsByServiceType(eServiceType))
            .WillByDefault(Return(lstCalls));
    EXPECT_CALL(objCallManager, RemoveCall(pCall1->GetKey()))
            .Times(1);
    EXPECT_CALL(objCallManager, RemoveCall(pCall2->GetKey()))
            .Times(1);

    Key nKey;
    nKey.eServiceType = eServiceType;
    pCallController->RemoveCalls(KeyType::SERVICE_TYPE, nKey);

    delete pCall1;
    delete pCall2;
}

TEST_F(MtcCallControllerTest, OpenCreatesCall)
{
    ServiceType eServiceType = ServiceType::NORMAL;
    CallInfo objCallInfo;

    CallKey nCallKey = 1;
    MockIMtcCall* pCall = CreateMockIMtcCall(nCallKey);

    EXPECT_CALL(objCallManager, CreateCall(eServiceType, Eq(std::ref(objCallInfo))))
            .Times(1)
            .WillRepeatedly(Return(pCall));

    pCallController->Open(eServiceType, objCallInfo);

    delete pCall;
}

TEST_F(MtcCallControllerTest, OpenReturnsCreatedCallKey)
{
    CallKey nCallKey = 1;
    MockIMtcCall* pCall = CreateMockIMtcCall(nCallKey);

    ON_CALL(objCallManager, CreateCall(_, _))
            .WillByDefault(Return(pCall));

    ServiceType eServiceType = ServiceType::NORMAL;
    CallInfo objCallInfo;
    EXPECT_EQ(nCallKey, pCallController->Open(eServiceType, objCallInfo));

    delete pCall;
}

TEST_F(MtcCallControllerTest, AttachAttachesTargetCall)
{
    CallKey nCallKey = 1;
    MockIMtcCall* pCall = CreateMockIMtcCall(nCallKey);
    EXPECT_CALL(*pCall, Attach())
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(pCall));

    pCallController->Attach(nCallKey);

    delete pCall;
}

TEST_F(MtcCallControllerTest, HandleIncomingCreatesCall)
{
    ServiceType eServiceType = ServiceType::NORMAL;

    MockIMtcService objService;
    ON_CALL(objService, GetServiceType)
            .WillByDefault(Return(eServiceType));

    MockIMtcCall objCall;
    EXPECT_CALL(objCallManager, CreateCall(_, _))
            .Times(1)
            .WillRepeatedly(Return(&objCall));

    pCallController->HandleIncoming(&objService, IMS_NULL);
}

TEST_F(MtcCallControllerTest, HandleIncomingCallsTargetCall)
{
    MockIMtcService objService;
    ISession* pSession = reinterpret_cast<ISession*>(0x1);

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, HandleIncoming(pSession))
            .Times(1);

    ON_CALL(objCallManager, CreateCall(_, _))
            .WillByDefault(Return(&objCall));

    pCallController->HandleIncoming(&objService, pSession);
}

TEST_F(MtcCallControllerTest, StartCallsTargetCall)
{
    CallKey nCallKey = 1;
    CallType eCallType = CallType::VOIP;
    AString strTarget = "target";
    MediaInfo objMediaInfo;
    IMSMap<SuppType, SuppService*> objSuppServices;

    // TODO: Make a matcher for IMSMap<SuppType, SuppService*>
    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Start(eCallType, strTarget, &objMediaInfo, _))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->Start(
            nCallKey, eCallType, strTarget, &objMediaInfo, objSuppServices, IMS_NULL);
}

TEST_F(MtcCallControllerTest, HandleUserAlertCallsTargetCall)
{
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, HandleUserAlert)
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->HandleUserAlert(nCallKey);
}

TEST_F(MtcCallControllerTest, AcceptCallsTargetCall)
{
    CallKey nCallKey = 1;
    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Accept(eCallType, &objMediaInfo))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->Accept(nCallKey, eCallType, &objMediaInfo);
}

TEST_F(MtcCallControllerTest, RejectCallsTargetCall)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Reject(objReason))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->Reject(nCallKey, objReason);
}

TEST_F(MtcCallControllerTest, HoldCallsTargetCall)
{
    CallKey nCallKey = 1;
    MediaInfo objMediaInfo;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Hold(&objMediaInfo))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->Hold(nCallKey, &objMediaInfo);
}

TEST_F(MtcCallControllerTest, ResumeCallsTargetCall)
{
    CallKey nCallKey = 1;
    MediaInfo objMediaInfo;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Resume(&objMediaInfo))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->Resume(nCallKey, &objMediaInfo);
}

TEST_F(MtcCallControllerTest, TerminateCallsTargetCall)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Terminate(objReason))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->Terminate(nCallKey, objReason);
}

TEST_F(MtcCallControllerTest, UpdateCallsTargetCall)
{
    CallKey nCallKey = 1;
    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, Update(eCallType, &objMediaInfo))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->Update(nCallKey, eCallType, &objMediaInfo);
}

TEST_F(MtcCallControllerTest, CancelUpdateCallsTargetCall)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, CancelUpdate(objReason))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->CancelUpdate(nCallKey, objReason);
}

TEST_F(MtcCallControllerTest, AcceptUpdateCallsTargetCall)
{
    CallKey nCallKey = 1;
    CallType eCallType = CallType::VOIP;
    MediaInfo objMediaInfo;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, AcceptUpdate(eCallType, &objMediaInfo))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->AcceptUpdate(nCallKey, eCallType, &objMediaInfo);
}

TEST_F(MtcCallControllerTest, RejectUpdateCallsTargetCall)
{
    CallReasonInfo objReason(CODE_LOCAL_SERVICE_UNAVAILABLE);
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, RejectUpdate(objReason))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->RejectUpdate(nCallKey, objReason);
}

TEST_F(MtcCallControllerTest, SendUssdCallsTargetCall)
{
    AString strUssd = "ussd";
    CallKey nCallKey = 1;

    MockIMtcCall objCall;
    EXPECT_CALL(objCall, SendUssd(strUssd))
            .Times(1);

    ON_CALL(objCallManager, GetCallByCallKey(nCallKey))
            .WillByDefault(Return(&objCall));

    pCallController->SendUssd(nCallKey, strUssd);
}

TEST_F(MtcCallControllerTest, MergeToConferenceCallsProcessesMerge)
{
    CallKey nCallKey = 1;
    IMSList<ConfUser*> objUsers;

    // TODO: Make a matcher for IMSList<ConfUser*>
    MockIConferenceController objConferenceController;
    EXPECT_CALL(objConferenceController, ProcessCommand(IConferenceController::MERGE, _))
            .Times(1);

    EXPECT_CALL(objConferenceManager, GetController(nCallKey))
            .Times(AnyNumber())
            .WillRepeatedly(Return(&objConferenceController));

    pCallController->MergeToConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, MergeToConferenceCreatesControllerIfNull)
{
    CallKey nCallKey = 1;

    ON_CALL(objConferenceManager, GetController(nCallKey))
            .WillByDefault(Return(nullptr));

    MockIConferenceController objConferenceController;
    EXPECT_CALL(objConferenceManager, CreateController(nCallKey, ConferenceType::MERGE_CALL))
            .Times(1)
            .WillRepeatedly(ReturnRef(objConferenceController));

    IMSList<ConfUser*> objUsers;
    pCallController->MergeToConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, AddToConferenceCallsProcessesAdd)
{
    CallKey nCallKey = 1;
    IMSList<ConfUser*> objUsers;

    // TODO: Make a matcher for IMSList<ConfUser*>
    MockIConferenceController objConferenceController;
    EXPECT_CALL(objConferenceController, ProcessCommand(IConferenceController::ADD, _))
            .Times(1);

    EXPECT_CALL(objConferenceManager, GetController(nCallKey))
            .Times(AnyNumber())
            .WillRepeatedly(Return(&objConferenceController));

    pCallController->AddToConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, AddToConferenceDoNothingIfControllerNull)
{
    CallKey nCallKey = 1;

    ON_CALL(objConferenceManager, GetController(nCallKey))
            .WillByDefault(Return(nullptr));
    EXPECT_CALL(objConferenceManager, CreateController(_, _))
            .Times(0);

    IMSList<ConfUser*> objUsers;
    pCallController->AddToConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, RemoveFromConferenceCallsProcessesRemove)
{
    CallKey nCallKey = 1;
    IMSList<ConfUser*> objUsers;

    // TODO: Make a matcher for IMSList<ConfUser*>
    MockIConferenceController objConferenceController;
    EXPECT_CALL(objConferenceController, ProcessCommand(IConferenceController::REMOVE, _))
            .Times(1);

    EXPECT_CALL(objConferenceManager, GetController(nCallKey))
            .Times(AnyNumber())
            .WillRepeatedly(Return(&objConferenceController));

    pCallController->RemoveFromConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, RemoveFromConferenceDoNothingIfControllerNull)
{
    CallKey nCallKey = 1;

    ON_CALL(objConferenceManager, GetController(nCallKey))
            .WillByDefault(Return(nullptr));
    EXPECT_CALL(objConferenceManager, CreateController(_, _))
            .Times(0);

    IMSList<ConfUser*> objUsers;
    pCallController->RemoveFromConference(nCallKey, objUsers);
}

TEST_F(MtcCallControllerTest, TransferCallsEctManager)
{
    CallKey nCallKey = 1;
    AString strTarget = "target";

    EXPECT_CALL(objEctManager, Transfer(nCallKey, strTarget));

    pCallController->Transfer(nCallKey, strTarget);
}

TEST_F(MtcCallControllerTest, HandleIpcanChangedCallsAllCalls)
{
    MockIMtcCall objCall1;
    MockIMtcCall objCall2;
    EXPECT_CALL(objCall1, HandleIpcanChanged)
            .Times(1);
    EXPECT_CALL(objCall2, HandleIpcanChanged)
            .Times(1);

    IMSList<IMtcCall*> lstCalls = CreateCallList({&objCall1, &objCall2});
    ON_CALL(objCallManager, GetCalls)
            .WillByDefault(Return(lstCalls));

    pCallController->HandleIpcanChanged();
}
