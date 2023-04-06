
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
#include "call/MockIMtcCallContext.h"
#include "call/MtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "conferencecall/ConferenceDef.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcSupplementaryService.h"
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
    MockIMtcCallContext objMockContext;
    JniCallInfo objJniCallInfo;

    MockIJniEnabler objMockJniEnabler;
    MockIJniMtcCallThread objMockCallThread;

    JniEnablerConnector* pConnector;
    MtcUiNotifier* pNotifier;

    CallInfo objCallInfo;
    MediaInfo objMediaInfo;
    ParticipantInfo* pParticipantInfo;
    CallReasonInfo* pReason;

    ImsMap<SuppType, SuppService*> objSuppServices;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockContext, CreateJniCallInfo).WillByDefault(Return(objJniCallInfo));
        ON_CALL(objMockContext, GetSlotId).WillByDefault(Return(SLOT_ID));

        ON_CALL(objMockJniEnabler, GetJniThread).WillByDefault(Return(&objMockCallThread));

        pConnector = &JniEnablerConnector::GetInstance();
        pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, &objMockJniEnabler);

        pReason = new CallReasonInfo(CODE_UNSPECIFIED);
        pParticipantInfo = new ParticipantInfo(objMockContext);
        pNotifier = new MtcUiNotifier(objMockContext);
    }

    virtual void TearDown() override
    {
        objSuppServices.Clear();
        delete pConnector;
        delete pNotifier;
        delete pParticipantInfo;
    }
};

TEST_F(MtcUiNotifierTest, SendPreIncomingCallReceived)
{
    MockIMtcService objService;
    ON_CALL(objMockContext, GetService).WillByDefault(ReturnRef(objService));

    MockIJniMtcServiceThread objMockServiceThread;

    ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(nullptr));
    EXPECT_CALL(objMockServiceThread, OnPreIncomingCallReceived(CALL_KEY)).Times(0);  // no meaning
    pNotifier->SendPreIncomingCallReceived(CALL_KEY);

    ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(&objMockServiceThread));
    EXPECT_CALL(objMockServiceThread, OnPreIncomingCallReceived(CALL_KEY)).Times(1);
    pNotifier->SendPreIncomingCallReceived(CALL_KEY);
}

TEST_F(MtcUiNotifierTest, SendIncomingCallReceived)
{
    MtcConfigurationProxy objConfigurationProxy(new MockIMtcConfigurationManager());
    ON_CALL(objMockContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

    MtcSupplementaryService objSupplementaryService(objMockContext, objConfigurationProxy);
    ON_CALL(objMockContext, GetSupplementaryService)
            .WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_CALL(objMockCallThread, OnIncomingCallReceived(_, _, _, _, _, _)).Times(1);

    pNotifier->SendIncomingCallReceived(
            CALL_KEY, objCallInfo, objMediaInfo, objSuppServices, *pParticipantInfo);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnIncomingCallReceived(_, _, _, _, _, _)).Times(0);
    pNotifier->SendIncomingCallReceived(
            CALL_KEY, objCallInfo, objMediaInfo, objSuppServices, *pParticipantInfo);
}

TEST_F(MtcUiNotifierTest, SendStarted)
{
    EXPECT_CALL(objMockCallThread, OnStarted(_, _, _)).Times(1);

    pNotifier->SendStarted(&objCallInfo, objMediaInfo, objSuppServices);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnStarted(_, _, _)).Times(0);

    pNotifier->SendStarted(&objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(MtcUiNotifierTest, SendStartFailed)
{
    EXPECT_CALL(objMockCallThread, OnStartFailed(_)).Times(1);

    pNotifier->SendStartFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnStartFailed(_)).Times(0);

    pNotifier->SendStartFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendProgressing)
{
    EXPECT_CALL(objMockCallThread, OnProgressing(_, _, _, _)).Times(1);

    pNotifier->SendProgressing(&objCallInfo, objMediaInfo, objSuppServices, IMS_TRUE);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnProgressing(_, _, _, _)).Times(0);

    pNotifier->SendProgressing(&objCallInfo, objMediaInfo, objSuppServices, IMS_TRUE);
}

TEST_F(MtcUiNotifierTest, SendHeld)
{
    EXPECT_CALL(objMockCallThread, OnHeld(_, _, _)).Times(1);

    pNotifier->SendHeld(&objCallInfo, objMediaInfo, objSuppServices);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnHeld(_, _, _)).Times(0);

    pNotifier->SendHeld(&objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(MtcUiNotifierTest, SendHoldFailed)
{
    EXPECT_CALL(objMockCallThread, OnHoldFailed(_)).Times(1);

    pNotifier->SendHoldFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnHoldFailed(_)).Times(0);

    pNotifier->SendHoldFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendResumed)
{
    EXPECT_CALL(objMockCallThread, OnResumed(_, _, _)).Times(1);

    pNotifier->SendResumed(&objCallInfo, objMediaInfo, objSuppServices);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnResumed(_, _, _)).Times(0);

    pNotifier->SendResumed(&objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(MtcUiNotifierTest, SendResumeFailed)
{
    EXPECT_CALL(objMockCallThread, OnResumeFailed(_)).Times(1);

    pNotifier->SendResumeFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnResumeFailed(_)).Times(0);

    pNotifier->SendResumeFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendHeldBy)
{
    EXPECT_CALL(objMockCallThread, OnHeldBy(_, _, _)).Times(1);

    pNotifier->SendHeldBy(&objCallInfo, objMediaInfo, objSuppServices);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnHeldBy(_, _, _)).Times(0);

    pNotifier->SendHeldBy(&objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(MtcUiNotifierTest, SendResumedBy)
{
    EXPECT_CALL(objMockCallThread, OnResumedBy(_, _, _)).Times(1);

    pNotifier->SendResumedBy(&objCallInfo, objMediaInfo, objSuppServices);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnResumedBy(_, _, _)).Times(0);

    pNotifier->SendResumedBy(&objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(MtcUiNotifierTest, SendTerminated)
{
    EXPECT_CALL(objMockCallThread, OnTerminated(_)).Times(1);

    pNotifier->SendTerminated(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnTerminated(_)).Times(0);

    pNotifier->SendTerminated(*pReason);
}

TEST_F(MtcUiNotifierTest, SendIncomingResume)
{
    EXPECT_CALL(objMockCallThread, OnIncomingResume(_, _, _)).Times(1);

    pNotifier->SendIncomingResume(&objCallInfo, objMediaInfo, objSuppServices);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnIncomingResume(_, _, _)).Times(0);

    pNotifier->SendIncomingResume(&objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(MtcUiNotifierTest, SendIncomingUpdate)
{
    EXPECT_CALL(objMockCallThread, OnIncomingUpdate(_, _, _)).Times(1);

    pNotifier->SendIncomingUpdate(CallType::VOIP, &objCallInfo, objMediaInfo, objSuppServices);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnIncomingUpdate(_, _, _)).Times(0);

    pNotifier->SendIncomingUpdate(CallType::VOIP, &objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(MtcUiNotifierTest, SendUpdated)
{
    EXPECT_CALL(objMockCallThread, OnUpdated(_, _, _)).Times(1);

    pNotifier->SendUpdated(&objCallInfo, objMediaInfo, objSuppServices);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnUpdated(_, _, _)).Times(0);

    pNotifier->SendUpdated(&objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(MtcUiNotifierTest, SendUpdateFailed)
{
    EXPECT_CALL(objMockCallThread, OnUpdateFailed(_)).Times(1);

    pNotifier->SendUpdateFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnUpdateFailed(_)).Times(0);

    pNotifier->SendUpdateFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendUpdatedBy)
{
    EXPECT_CALL(objMockCallThread, OnUpdatedBy(_, _, _)).Times(1);

    pNotifier->SendUpdatedBy(&objCallInfo, objMediaInfo, objSuppServices);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnUpdatedBy(_, _, _)).Times(0);

    pNotifier->SendUpdatedBy(&objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(MtcUiNotifierTest, SendNotifyInfo)
{
    EXPECT_CALL(objMockCallThread, OnInformationNotificationReceived(_, _, _, _)).Times(1);

    pNotifier->SendNotifyInfo(0, "", -1, IMS_FALSE);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnInformationNotificationReceived(_, _, _, _)).Times(0);

    pNotifier->SendNotifyInfo(0, "", -1, IMS_FALSE);
}

TEST_F(MtcUiNotifierTest, SendExpanded)
{
    // TODO: implement logic.
    pNotifier->SendExpanded(&objCallInfo, objMediaInfo, objSuppServices);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    pNotifier->SendExpanded(&objCallInfo, objMediaInfo, objSuppServices);
}

TEST_F(MtcUiNotifierTest, SendExpandFailed)
{
    // TODO: implement logic.
    pNotifier->SendExpandFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    pNotifier->SendExpandFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendExpandedBy)
{
    // TODO: implement logic.
    pNotifier->SendExpandedBy(&objCallInfo, objMediaInfo, objSuppServices, 0);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    pNotifier->SendExpandedBy(&objCallInfo, objMediaInfo, objSuppServices, 0);
}

TEST_F(MtcUiNotifierTest, SendMerged)
{
    ImsList<ConfUser*> objUsers;
    EXPECT_CALL(objMockCallThread, OnMerged(_, _, _, _)).Times(1);

    pNotifier->SendMerged(&objCallInfo, objMediaInfo, objSuppServices, objUsers);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnMerged(_, _, _, _)).Times(0);

    pNotifier->SendMerged(&objCallInfo, objMediaInfo, objSuppServices, objUsers);
}

TEST_F(MtcUiNotifierTest, SendMergeFailed)
{
    EXPECT_CALL(objMockCallThread, OnMergeFailed(_)).Times(1);

    pNotifier->SendMergeFailed(*pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnMergeFailed(_)).Times(0);

    pNotifier->SendMergeFailed(*pReason);
}

TEST_F(MtcUiNotifierTest, SendJoined)
{
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantAdded()).Times(1);

    pNotifier->SendJoined(IMS_TRUE, *pReason);

    EXPECT_CALL(objMockCallThread, OnConferenceParticipantAddFailed(_)).Times(1);

    pNotifier->SendJoined(IMS_FALSE, *pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantAdded()).Times(0);

    pNotifier->SendJoined(IMS_TRUE, *pReason);
}

TEST_F(MtcUiNotifierTest, SendDropped)
{
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantRemoved()).Times(1);

    pNotifier->SendDropped(IMS_TRUE, *pReason);

    EXPECT_CALL(objMockCallThread, OnConferenceParticipantRemoveFailed(_)).Times(1);

    pNotifier->SendDropped(IMS_FALSE, *pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantRemoved()).Times(0);

    pNotifier->SendDropped(IMS_TRUE, *pReason);
}

TEST_F(MtcUiNotifierTest, SendNotifyUsersInfo)
{
    ImsList<ConfUser*> objUsers;
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantsInfoChanged(_)).Times(1);

    pNotifier->SendNotifyUsersInfo(objUsers);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantsInfoChanged(_)).Times(0);

    pNotifier->SendNotifyUsersInfo(objUsers);
}

TEST_F(MtcUiNotifierTest, SendNotifyConfInfo)
{
    ImsList<ConfUser*> objUsers;
    EXPECT_CALL(objMockCallThread, OnConferenceInfoChanged(_, _, _, _, _)).Times(1);

    pNotifier->SendNotifyConfInfo("", "", 6, 1, "");

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnConferenceInfoChanged(_, _, _, _, _)).Times(0);

    pNotifier->SendNotifyConfInfo("", "", 6, 1, "");
}

TEST_F(MtcUiNotifierTest, SendReplacedBy)
{
    // TODO: implement logic
    pNotifier->SendReplacedBy(&objCallInfo, objMediaInfo, objSuppServices, 1, 1);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    pNotifier->SendReplacedBy(&objCallInfo, objMediaInfo, objSuppServices, 1, 1);
}

TEST_F(MtcUiNotifierTest, SendEctCompleted)
{
    EXPECT_CALL(objMockCallThread, OnEctCompleted(_, _)).Times(1);

    pNotifier->SendEctCompleted(IMS_TRUE, *pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    EXPECT_CALL(objMockCallThread, OnEctCompleted(_, _)).Times(0);

    pNotifier->SendEctCompleted(IMS_TRUE, *pReason);
}

TEST_F(MtcUiNotifierTest, SendCallPushCompleted)
{
    // TODO: implement logic
    pNotifier->SendCallPushCompleted(IMS_TRUE, *pReason);

    pConnector->SetJniEnabler(SLOT_ID, EnablerType::MTC_CALL, IMS_NULL);
    pNotifier->SendCallPushCompleted(IMS_TRUE, *pReason);
}

}  // namespace android
