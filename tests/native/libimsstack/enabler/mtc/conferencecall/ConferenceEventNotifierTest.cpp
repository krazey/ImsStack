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
#include "JniEnablerConnector.h"
#include "MockIJniEnabler.h"
#include "MockIJniMtcCallThread.h"
#include "MockISession.h"
#include "MtcDef.h"
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceEventNotifier.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "conferencecall/MockConferenceParticipantList.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include <gtest/gtest.h>

MATCHER(IsEmpty, "")
{
    return arg.IsEmpty();
}

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL CallKey CONFERENCE_CALL_KEY = 999;
LOCAL IMS_SINT32 SLOT_ID = 0;
LOCAL IMS_SINT32 MAX_COUNT = 6;
LOCAL IMS_UINT32 CONF_SIZE = 1;

namespace android
{

class ConferenceEventNotifierTest : public ::testing::Test
{
public:
    MockIMtcCallManager objCallManager;
    MockIMtcCall objConferenceCall;
    MockIMtcCallContext objContext;
    MockIMtcSession objMtcSession;
    MockIJniEnabler objMockJniEnabler;
    MockIJniMtcCallThread objMockCallThread;
    MockISession objISession;
    MockCallConnectionIdManager* pIdManager;
    MockIMtcMediaManager objMediaManager;
    MockICallStateProxy objCallStateProxy;
    MockConferenceParticipantList* pParticipantList;
    ConferenceParticipantList::ConferenceParticipant* pParticipant;

    JniEnablerConnector* pConnector;
    ConferenceEventNotifier* pNotifier;
    ImsList<ConfUser*> objUsers;
    ConfUser objUser;
    CallReasonInfo* pAnyReason;
    MediaInfo objMediaInfo;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objCallManager, GetCallByCallKey(CONFERENCE_CALL_KEY))
                .WillByDefault(Return(&objConferenceCall));
        ON_CALL(objConferenceCall, GetKey).WillByDefault(Return(CONFERENCE_CALL_KEY));
        ON_CALL(objConferenceCall, GetCallContext).WillByDefault(ReturnRef(objContext));

        pConnector = &JniEnablerConnector::GetInstance();
        pConnector->SetJniEnabler(
                SLOT_ID, EnablerType::MTC_CALL, &objMockJniEnabler, CONFERENCE_CALL_KEY);
        ON_CALL(objMockJniEnabler, GetJniThread).WillByDefault(Return(&objMockCallThread));

        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objCallStateProxy, AddListener(_)).WillByDefault(Return());

        pIdManager = new MockCallConnectionIdManager(objContext);
        ON_CALL(*pIdManager, OnConferenceParticipantDisconnected(_)).WillByDefault(Return());
        ON_CALL(objContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objISession));
        ON_CALL(objMediaManager, GetMediaInfo(&objISession)).WillByDefault(ReturnRef(objMediaInfo));

        SetUpMockParticipantList();

        pAnyReason = new CallReasonInfo(CODE_UNSPECIFIED);

        pNotifier = new ConferenceEventNotifier(objCallManager, CONFERENCE_CALL_KEY, *pIdManager);
    }

    virtual void TearDown() override
    {
        delete pIdManager;
        delete pNotifier;
        delete pAnyReason;
        delete pParticipantList;
        delete pParticipant;
        delete pConnector;
    }

    void SetUpMockParticipantList()
    {
        objUsers.Append(&objUser);
        pParticipantList = new MockConferenceParticipantList();

        ON_CALL(*pParticipantList, GetMaxUserCount).WillByDefault(Return(MAX_COUNT));
        ON_CALL(*pParticipantList, GetSize).WillByDefault(Return(CONF_SIZE));
        ON_CALL(*pParticipantList, GetConfUsers).WillByDefault(Return(objUsers));

        pParticipant = new ConferenceParticipantList::ConferenceParticipant();
        pParticipant->SetConfUser(&objUser);
        ON_CALL(*pParticipantList, GetAt(0)).WillByDefault(Return(pParticipant));
        ON_CALL(*pParticipantList, RemoveUser(_)).WillByDefault(Return());
    }
};

TEST_F(ConferenceEventNotifierTest, NotifyMergedIfNotSubscribed)
{
    MtcConfigurationProxy objConfigurationProxy;
    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_CALL(objMockCallThread, OnMerged(_, _, _, IsEmpty())).Times(1);
    pNotifier->NotifyMerged(*pParticipantList, IMS_FALSE);
}

TEST_F(ConferenceEventNotifierTest, NotifyMergedIfSubscribed)
{
    MtcConfigurationProxy objConfigurationProxy;
    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_CALL(objMockCallThread, OnMerged(_, _, _, objUsers)).Times(1);
    pNotifier->NotifyMerged(*pParticipantList, IMS_TRUE);
}

TEST_F(ConferenceEventNotifierTest, NotifyMergeFailed)
{
    EXPECT_CALL(objMockCallThread, OnMergeFailed(_)).Times(1);
    pNotifier->NotifyMergeFailed(*pAnyReason);
}

TEST_F(ConferenceEventNotifierTest, NotifyGroupCallStarted)
{
    // not yet implemented
}

TEST_F(ConferenceEventNotifierTest, NotifyGroupCallFailed)
{
    // not yet implemented
}

TEST_F(ConferenceEventNotifierTest, NotifyExpanded)
{
    // not yet implemented
}

TEST_F(ConferenceEventNotifierTest, NotifyExpandFailed)
{
    // not yet implemented
}

TEST_F(ConferenceEventNotifierTest, NotifyDropped)
{
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantRemoved()).Times(1);
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantsInfoChanged(objUsers)).Times(1);
    pNotifier->NotifyDropped(*pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyDropFailed)
{
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantRemoveFailed(_)).Times(1);
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantsInfoChanged(objUsers)).Times(1);
    pNotifier->NotifyDropFailed(*pAnyReason, *pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyJoined)
{
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantAdded()).Times(1);
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantsInfoChanged(objUsers)).Times(1);
    pNotifier->NotifyJoined(*pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyJoinFailed)
{
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantAddFailed(_)).Times(1);
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantsInfoChanged(objUsers)).Times(1);
    pNotifier->NotifyJoinFailed(*pAnyReason, *pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyConferenceInfo)
{
    AString strEmpty("");
    EXPECT_CALL(objMockCallThread,
            OnConferenceInfoChanged(strEmpty, strEmpty, CONF_SIZE, MAX_COUNT, strEmpty))
            .Times(1);
    pNotifier->NotifyConferenceInfo(*pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyUsersInfo)
{
    EXPECT_CALL(objMockCallThread, OnConferenceParticipantsInfoChanged(objUsers)).Times(1);
    pNotifier->NotifyUsersInfo(*pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, DoesNotNotifyIfConferenceCallDoesNotExist)
{
    // GetCallByCallKey Returns a NullCall
    ON_CALL(objConferenceCall, GetKey).WillByDefault(Return(IMtcCall::CALL_KEY_INVALID));

    EXPECT_CALL(objMockCallThread, OnConferenceParticipantsInfoChanged(_)).Times(0);
    pNotifier->NotifyUsersInfo(*pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyIndividualCallTerminated)
{
    CallKey n1To1Key = 1000;
    MockIMtcCall obj1To1Call;
    MockIMtcCallContext obj1To1CallContext;
    MockIMtcUiNotifier obj1To1CallNotifier;

    ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
    ON_CALL(objCallManager, GetCallByCallKey(n1To1Key)).WillByDefault(Return(&obj1To1Call));
    ON_CALL(obj1To1Call, GetKey).WillByDefault(Return(n1To1Key));
    ON_CALL(obj1To1Call, GetCallContext).WillByDefault(ReturnRef(obj1To1CallContext));
    ON_CALL(obj1To1CallContext, GetUiNotifier).WillByDefault(ReturnRef(obj1To1CallNotifier));

    EXPECT_CALL(obj1To1CallNotifier,
            SendTerminated(CallReasonInfo(
                    CODE_USER_TERMINATED_BY_REMOTE, CODE_USER_TERMINATED_BY_REMOTE)));
    pNotifier->NotifyIndividualCallTerminated(n1To1Key);
}

}  // namespace android
