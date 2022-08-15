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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/state/EstablishedState.h"
#include "media/MockIMtcMediaManager.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class EstablishedStateTest : public ::testing::Test
{
public:
    EstablishedState* pEstablishedState;
    MockIMtcCallContext objMockCallContext;
    MockIMtcMediaManager objMockMediaManager;
    MockIMtcSession objMockMtcSession;
    MtcUiNotifier* pUiNotifier;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockCallContext, GetMediaManager).WillByDefault(ReturnRef(objMockMediaManager));
        ON_CALL(objMockCallContext, GetSession()).WillByDefault(Return(&objMockMtcSession));

        pUiNotifier = new MtcUiNotifier(objMockCallContext);
        ON_CALL(objMockCallContext, GetUiNotifier).WillByDefault(ReturnRef(*pUiNotifier));

        pEstablishedState = new EstablishedState(objMockCallContext);
    }

    virtual void TearDown() override
    {
        delete pUiNotifier;
        delete pEstablishedState;
    }
};

TEST_F(EstablishedStateTest, TerminateByUserActionWhenNoReceivingAudioPackets)
{
    CallInfo objCallInfo;
    ON_CALL(objMockCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    EXPECT_CALL(objMockMediaManager, IsAudioInactive)
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_CALL(objMockMtcSession, Terminate(_, _)).Times(2);

    CallReasonInfo objReason(CODE_USER_TERMINATED);
    pEstablishedState->Terminate(objReason);
    pEstablishedState->Terminate(objReason);
}

}  // namespace android
