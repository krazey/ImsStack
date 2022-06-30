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
 * distributed under the License is distributed on an "AS IS" B ASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "conferencecall/ConferenceController.h"
#include "call/MockIMtcCallManager.h"
#include "call/IMtcCall.h"
#include "conferencecall/CallConnectionIdManager.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "helper/MockICallStateProxy.h"
#include "../../../engine/interface/core/MockICoreService.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

LOCAL CallKey CONFERENCE_CALL_KEY = 100;

class ConferenceControllerTest : public ::testing::Test
{
public:
    MockIMtcContext objMockContext;
    MockIMtcCallManager objMockCallManager;
    MockICallStateProxy objMockCallStateProxy;
    MockIMtcService objMtcService;
    MockICoreService objCoreService;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockContext, GetCallManager).WillByDefault(ReturnRef(objMockCallManager));
        ON_CALL(objMockContext, GetCallStateProxy).WillByDefault(ReturnRef(objMockCallStateProxy));

        ON_CALL(objMtcService, GetICoreService).WillByDefault(Return(&objCoreService));
    }

    virtual void TearDown() override {}

    MockIMtcCall* CreateMockIMtcCall(IN CallKey nKey)
    {
        MockIMtcCall* pCall = new MockIMtcCall();
        ON_CALL(*pCall, GetKey()).WillByDefault(Return(nKey));
        return pCall;
    }

    ConferenceController* CreateController(IN MockIMtcCall* piConferenceCall)
    {
        CallConnectionIdManager objConnectionManager(objMockContext);

        MockIMtcCallContext objMockCallContext;
        ON_CALL(*piConferenceCall, GetCallContext()).WillByDefault(ReturnRef(objMockCallContext));
        ON_CALL(objMockCallContext, GetService()).WillByDefault(ReturnRef(objMtcService));
        ON_CALL(objMockCallManager, GetCallByCallKey(piConferenceCall->GetKey()))
                .WillByDefault(Return(piConferenceCall));

        return new ConferenceController(
                piConferenceCall->GetKey(), objMockContext, objConnectionManager);
    }
};

TEST_F(ConferenceControllerTest, GetIndividualCallStateReturnsHost)
{
    MockIMtcCall* piMockConferenceCall = CreateMockIMtcCall(CONFERENCE_CALL_KEY);
    ConferenceController* pController = CreateController(piMockConferenceCall);
    EXPECT_EQ(
            pController->GetCallStatusInConference(CONFERENCE_CALL_KEY), IndividualCallState::HOST);
    delete piMockConferenceCall;
    delete pController;
}

TEST_F(ConferenceControllerTest, GetIndividualCallStateReturnsIdle)
{
    const CallKey IDLE_CALL_KEY = 0;
    MockIMtcCall* piMockConferenceCall = CreateMockIMtcCall(CONFERENCE_CALL_KEY);
    ConferenceController* pController = CreateController(piMockConferenceCall);
    EXPECT_EQ(pController->GetCallStatusInConference(IDLE_CALL_KEY), IndividualCallState::IDLE);
    delete piMockConferenceCall;
    delete pController;
}

}  // namespace android
