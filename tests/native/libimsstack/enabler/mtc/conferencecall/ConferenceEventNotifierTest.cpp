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
#include "MtcDef.h"
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcUiNotifier.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceEventNotifier.h"
#include "conferencecall/ConferenceParticipantList.h"
#include "conferencecall/MockConferenceParticipantList.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MtcSupplementaryService.h"
#include "media/MockIMtcMediaManager.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL IMS_SINT32 MAX_COUNT = 6;
LOCAL IMS_UINT32 CONF_SIZE = 1;

namespace android
{

class ConferenceEventNotifierTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcUiNotifier objUiNotifier;
    MockCallConnectionIdManager* pIdManager;
    MockIMtcMediaManager objMediaManager;
    MockICallStateProxy objCallStateProxy;
    MockConferenceParticipantList* pParticipantList;
    ConferenceParticipantList::ConferenceParticipant* pParticipant;

    ConferenceEventNotifier* pNotifier;
    ImsList<ConfUser*> objUsers;
    ConfUser objUser;
    CallReasonInfo* pAnyReason;
    MediaInfo objMediaInfo;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objCallStateProxy, AddListener(_)).WillByDefault(Return());

        pIdManager = new MockCallConnectionIdManager(objContext);
        ON_CALL(*pIdManager, OnConferenceParticipantDisconnected(_)).WillByDefault(Return());
        ON_CALL(objContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objMediaManager, GetMediaInfo()).WillByDefault(ReturnRef(objMediaInfo));

        ON_CALL(objContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));

        SetUpMockParticipantList();

        pAnyReason = new CallReasonInfo(CODE_UNSPECIFIED);

        pNotifier = new ConferenceEventNotifier(objContext, *pIdManager);
    }

    virtual void TearDown() override
    {
        delete pIdManager;
        delete pNotifier;
        delete pAnyReason;
        delete pParticipantList;
        delete pParticipant;
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

TEST_F(ConferenceEventNotifierTest, NotifyMerged)
{
    MtcConfigurationProxy objConfigurationProxy(new MockIMtcConfigurationManager());
    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_CALL(objUiNotifier, SendMerged(_, _, _, objUsers));
    pNotifier->NotifyMerged(*pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyMergeFailed)
{
    EXPECT_CALL(objUiNotifier, SendMergeFailed(*pAnyReason));
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
    EXPECT_CALL(objUiNotifier, SendDropped(IMS_TRUE, *pAnyReason));
    EXPECT_CALL(objUiNotifier, SendNotifyUsersInfo(objUsers));
    pNotifier->NotifyDropped(*pAnyReason, *pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyDropFailed)
{
    EXPECT_CALL(objUiNotifier, SendDropped(IMS_FALSE, *pAnyReason));
    EXPECT_CALL(objUiNotifier, SendNotifyUsersInfo(objUsers));
    pNotifier->NotifyDropFailed(*pAnyReason, *pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyJoined)
{
    EXPECT_CALL(objUiNotifier, SendJoined(IMS_TRUE, *pAnyReason));
    EXPECT_CALL(objUiNotifier, SendNotifyUsersInfo(objUsers));
    pNotifier->NotifyJoined(*pAnyReason, *pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyJoinFailed)
{
    EXPECT_CALL(objUiNotifier, SendJoined(IMS_FALSE, *pAnyReason));
    EXPECT_CALL(objUiNotifier, SendNotifyUsersInfo(objUsers));
    pNotifier->NotifyJoinFailed(*pAnyReason, *pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyConferenceInfo)
{
    AString strEmpty("");
    EXPECT_CALL(
            objUiNotifier, SendNotifyConfInfo(strEmpty, strEmpty, MAX_COUNT, CONF_SIZE, strEmpty));
    pNotifier->NotifyConferenceInfo(*pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyUsersInfo)
{
    EXPECT_CALL(objUiNotifier, SendNotifyUsersInfo(objUsers));
    pNotifier->NotifyUsersInfo(*pParticipantList);
}

TEST_F(ConferenceEventNotifierTest, NotifyIndividualCallTerminated)
{
    CallKey n1To1Key = 1000;
    MockIMtcCallManager objCallManager;
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
