
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

#include "CallReasonInfo.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "JniCallInfo.h"
#include "JniEnablerConnector.h"
#include "MockIMtcService.h"
#include "MockIJniEnabler.h"
#include "MockIJniMtcCallThread.h"
#include "MockIJniMtcServiceThread.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "call/UpdatingInfo.h"
#include "conferencecall/ConferenceDef.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

LOCAL IMS_SINT32 SLOT_ID = 0;
LOCAL IMS_SINT32 CALL_KEY = 1;

class MtcUiNotifierTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    JniCallInfo objJniCallInfo;
    MockIMtcMediaManager objMediaManager;
    MockIJniEnabler objMockJniEnabler;
    MockIJniMtcCallThread objMockCallThread;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;

    JniEnablerConnector* pConnector;
    MtcUiNotifier* pNotifier;

    CallInfo objCallInfo;
    MediaInfo objMediaInfo;
    UpdatingInfo* pUpdatingInfo;
    ParticipantInfo* pParticipantInfo;
    CallReasonInfo* pReason;

    MtcSupplementaryService* pSupplementaryService;
    ImsMap<SuppType, SuppService*> objSuppServices;

protected:
    virtual void SetUp() override
    {
        pReason = new CallReasonInfo(CODE_UNSPECIFIED);
        pParticipantInfo = new ParticipantInfo(objContext);
        pNotifier = new MtcUiNotifier(objContext);
        pUpdatingInfo = new UpdatingInfo(objContext);
        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        pSupplementaryService = new MtcSupplementaryService(objContext, *pConfigurationProxy);

        pConnector = &JniEnablerConnector::GetInstance();
        pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, &objMockJniEnabler, CALL_KEY);

        ON_CALL(objContext, CreateJniCallInfo).WillByDefault(Return(objJniCallInfo));
        ON_CALL(objContext, GetCallKey).WillByDefault(Return(CALL_KEY));
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));
        ON_CALL(objContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objContext, GetParticipantInfo).WillByDefault(ReturnRef(*pParticipantInfo));
        ON_CALL(objContext, GetSupplementaryService)
                .WillByDefault(ReturnRef(*pSupplementaryService));
        ON_CALL(objContext, GetUpdatingInfo).WillByDefault(ReturnRef(*pUpdatingInfo));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));

        ON_CALL(objMediaManager, GetMediaInfo).WillByDefault(ReturnRef(objMediaInfo));

        ON_CALL(objMockJniEnabler, GetJniThread).WillByDefault(Return(&objMockCallThread));
    }

    virtual void TearDown() override
    {
        objSuppServices.Clear();
        delete pSupplementaryService;
        delete pConfigurationProxy;
        delete pConnector;
        delete pNotifier;
        delete pUpdatingInfo;
        delete pParticipantInfo;
        delete pReason;
    }
};

TEST_F(MtcUiNotifierTest, SendPreIncomingCallReceived)
{
    MockIMtcService objService;
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    MockIJniMtcServiceThread objMockServiceThread;

    ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(nullptr));
    EXPECT_CALL(objMockServiceThread, OnPreIncomingCallReceived(CALL_KEY)).Times(0);
    pNotifier->SendPreIncomingCallReceived();

    ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(&objMockServiceThread));
    EXPECT_CALL(objMockServiceThread, OnPreIncomingCallReceived(CALL_KEY)).Times(1);
    pNotifier->SendPreIncomingCallReceived();
}

TEST_F(MtcUiNotifierTest, SendIncomingCallReceived)
{
    EXPECT_CALL(objMockCallThread, OnIncomingCallReceived(_, _, _, _, _, _)).Times(1);

    pNotifier->SendIncomingCallReceived();

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnIncomingCallReceived(_, _, _, _, _, _)).Times(0);
    pNotifier->SendIncomingCallReceived();
}

TEST_F(MtcUiNotifierTest, SendStarted)
{
    EXPECT_CALL(objMockCallThread, OnStarted(_, _, _)).Times(1);

    pNotifier->SendStarted();

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnStarted(_, _, _)).Times(0);

    pNotifier->SendStarted();
}

TEST_F(MtcUiNotifierTest, SendStartFailed)
{
    EXPECT_CALL(objMockCallThread, OnStartFailed(_)).Times(1);

    pNotifier->SendStartFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnStartFailed(_)).Times(0);

    pNotifier->SendStartFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendProgressing)
{
    EXPECT_CALL(objMockCallThread, OnProgressing(_, _, _, IMS_TRUE)).Times(1);

    pNotifier->SendProgressing(IMS_TRUE);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnProgressing(_, _, _, _)).Times(0);

    pNotifier->SendProgressing(IMS_TRUE);
}

TEST_F(MtcUiNotifierTest, SendHeld)
{
    EXPECT_CALL(objMockCallThread, OnHeld(_, _, _)).Times(1);

    pNotifier->SendHeld();

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnHeld(_, _, _)).Times(0);

    pNotifier->SendHeld();
}

TEST_F(MtcUiNotifierTest, SendHoldFailed)
{
    EXPECT_CALL(objMockCallThread, OnHoldFailed(_)).Times(1);

    pNotifier->SendHoldFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnHoldFailed(_)).Times(0);

    pNotifier->SendHoldFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendResumed)
{
    EXPECT_CALL(objMockCallThread, OnResumed(_, _, _)).Times(1);

    pNotifier->SendResumed();

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnResumed(_, _, _)).Times(0);

    pNotifier->SendResumed();
}

TEST_F(MtcUiNotifierTest, SendResumeFailed)
{
    EXPECT_CALL(objMockCallThread, OnResumeFailed(_)).Times(1);

    pNotifier->SendResumeFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnResumeFailed(_)).Times(0);

    pNotifier->SendResumeFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendHeldBy)
{
    EXPECT_CALL(objMockCallThread, OnHeldBy(_, _, _)).Times(1);

    pNotifier->SendHeldBy();

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnHeldBy(_, _, _)).Times(0);

    pNotifier->SendHeldBy();
}

TEST_F(MtcUiNotifierTest, SendResumedBy)
{
    EXPECT_CALL(
            objMockCallThread, OnResumedBy(_, objContext.GetUpdatingInfo().GetModifiedInfo(), _))
            .Times(1);

    pNotifier->SendResumedBy();

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(
            objMockCallThread, OnResumedBy(_, objContext.GetUpdatingInfo().GetModifiedInfo(), _))
            .Times(0);

    pNotifier->SendResumedBy();
}

TEST_F(MtcUiNotifierTest, SendTerminated)
{
    EXPECT_CALL(objMockCallThread, OnTerminated(_)).Times(1);

    pNotifier->SendTerminated(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnTerminated(_)).Times(0);

    pNotifier->SendTerminated(*pReason);
}

TEST_F(MtcUiNotifierTest, SendIncomingResume)
{
    EXPECT_CALL(objMockCallThread, OnIncomingResume(_, _, _)).Times(1);

    pNotifier->SendIncomingResume();

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnIncomingResume(_, _, _)).Times(0);

    pNotifier->SendIncomingResume();
}

TEST_F(MtcUiNotifierTest, SendIncomingUpdate)
{
    objContext.GetUpdatingInfo().SetTargetCallType(CallType::UNKNOWN);

    EXPECT_CALL(objMockCallThread,
            OnIncomingUpdate(_, objContext.GetUpdatingInfo().GetModifiedInfo(), _))
            .Times(1);

    pNotifier->SendIncomingUpdate(CallType::VOIP);

    objContext.GetUpdatingInfo().SetTargetCallType(CallType::VOIP);

    EXPECT_CALL(objMockCallThread,
            OnIncomingUpdate(_, objContext.GetUpdatingInfo().GetAlertingInfo(), _))
            .Times(1);

    pNotifier->SendIncomingUpdate(CallType::VOIP);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnIncomingUpdate(_, _, _)).Times(0);

    pNotifier->SendIncomingUpdate(CallType::VOIP);
}

TEST_F(MtcUiNotifierTest, SendUpdated)
{
    EXPECT_CALL(objMockCallThread, OnUpdated(_, _, _)).Times(1);

    pNotifier->SendUpdated();

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnUpdated(_, _, _)).Times(0);

    pNotifier->SendUpdated();
}

TEST_F(MtcUiNotifierTest, SendUpdateFailed)
{
    EXPECT_CALL(objMockCallThread, OnUpdateFailed(_)).Times(1);

    pNotifier->SendUpdateFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnUpdateFailed(_)).Times(0);

    pNotifier->SendUpdateFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendUpdatedBy)
{
    EXPECT_CALL(objMockCallThread, OnUpdatedBy(_, _, _)).Times(1);

    pNotifier->SendUpdatedBy();

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnUpdatedBy(_, _, _)).Times(0);

    pNotifier->SendUpdatedBy();
}

TEST_F(MtcUiNotifierTest, SendNotifyInfo)
{
    const AString strValue = "str";
    EXPECT_CALL(objMockCallThread, OnInformationNotificationReceived(0, strValue, -1, IMS_TRUE))
            .Times(1);

    pNotifier->SendNotifyInfo(0, strValue, -1, IMS_TRUE);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnInformationNotificationReceived(_, _, _, _)).Times(0);

    pNotifier->SendNotifyInfo(0, strValue, -1, IMS_TRUE);
}

TEST_F(MtcUiNotifierTest, SendExpanded)
{
    // TODO: implement logic.
    pNotifier->SendExpanded();

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    pNotifier->SendExpanded();
}

TEST_F(MtcUiNotifierTest, SendExpandFailed)
{
    // TODO: implement logic.
    pNotifier->SendExpandFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    pNotifier->SendExpandFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendExpandedBy)
{
    // TODO: implement logic.
    pNotifier->SendExpandedBy(0);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    pNotifier->SendExpandedBy(0);
}

TEST_F(MtcUiNotifierTest, SendMerged)
{
    ImsList<ConfUser*> objUsers;
    EXPECT_CALL(objMockCallThread, OnMerged(_, _, _, objUsers)).Times(1);

    pNotifier->SendMerged(objUsers);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnMerged(_, _, _, _)).Times(0);

    pNotifier->SendMerged(objUsers);
}

TEST_F(MtcUiNotifierTest, SendMergeFailed)
{
    EXPECT_CALL(objMockCallThread, OnMergeFailed(_)).Times(1);

    pNotifier->SendMergeFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnMergeFailed(_)).Times(0);

    pNotifier->SendMergeFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendJoined)
{
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantAdded()).Times(1);

    pNotifier->SendJoined(IMS_SUCCESS, *pReason);

    EXPECT_CALL(objMockCallThread, OnConferenceParticipantAddFailed(_)).Times(1);

    pNotifier->SendJoined(IMS_FAILURE, *pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantAdded()).Times(0);

    pNotifier->SendJoined(IMS_SUCCESS, *pReason);
}

TEST_F(MtcUiNotifierTest, SendDropped)
{
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantRemoved()).Times(1);

    pNotifier->SendDropped(IMS_SUCCESS, *pReason);

    EXPECT_CALL(objMockCallThread, OnConferenceParticipantRemoveFailed(_)).Times(1);

    pNotifier->SendDropped(IMS_FAILURE, *pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantRemoved()).Times(0);

    pNotifier->SendDropped(IMS_SUCCESS, *pReason);
}

TEST_F(MtcUiNotifierTest, SendNotifyUsersInfo)
{
    ImsList<ConfUser*> objUsers;
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantsInfoChanged(objUsers)).Times(1);

    pNotifier->SendNotifyUsersInfo(objUsers);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantsInfoChanged(_)).Times(0);

    pNotifier->SendNotifyUsersInfo(objUsers);
}

TEST_F(MtcUiNotifierTest, SendNotifyConfInfo)
{
    const AString strDisplayText = "text";
    const AString strSubject = "subject";
    const IMS_SINT32 nUser = 1;
    const IMS_SINT32 nMaxUser = 6;
    const AString strHost = "host";

    ImsList<ConfUser*> objUsers;
    EXPECT_CALL(objMockCallThread,
            OnConferenceInfoChanged(strDisplayText, strSubject, nUser, nMaxUser, strHost))
            .Times(1);

    pNotifier->SendNotifyConfInfo(strDisplayText, strSubject, nUser, nMaxUser, strHost);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnConferenceInfoChanged(_, _, _, _, _)).Times(0);

    pNotifier->SendNotifyConfInfo(strDisplayText, strSubject, nUser, nMaxUser, strHost);
}

TEST_F(MtcUiNotifierTest, SendReplacedBy)
{
    // TODO: implement logic
    pNotifier->SendReplacedBy(1, 2);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    pNotifier->SendReplacedBy(1, 2);
}

TEST_F(MtcUiNotifierTest, SendEctCompleted)
{
    EXPECT_CALL(objMockCallThread, OnEctCompleted(IMS_SUCCESS, *pReason)).Times(1);

    pNotifier->SendEctCompleted(IMS_SUCCESS, *pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    EXPECT_CALL(objMockCallThread, OnEctCompleted(_, _)).Times(0);

    pNotifier->SendEctCompleted(IMS_SUCCESS, *pReason);
}

TEST_F(MtcUiNotifierTest, SendCallPushCompleted)
{
    // TODO: implement logic
    pNotifier->SendCallPushCompleted(IMS_SUCCESS, *pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL, CALL_KEY);
    pNotifier->SendCallPushCompleted(IMS_SUCCESS, *pReason);
}

}  // namespace android
