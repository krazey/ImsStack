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
#include "call/state/IncomingState.h"
#include "core/MockISession.h"
#include "helper/ISrvccStateListener.h"
#include "media/MockIMtcMediaManager.h"
#include "precondition/MockIMtcPreconditionManager.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class IncomingStateTest : public ::testing::Test
{
public:
    IncomingState* pIncomingState;
    MockIMtcCallContext objCallContext;
    MockIMtcPreconditionManager objPreconditionManager;
    MtcUiNotifier* pUiNotifier;
    MockIMtcSession objMtcSession;
    MockISession objMockISession;
    MockIMtcMediaManager objMockMediaManager;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objCallContext, GetPreconditionManager)
                .WillByDefault(ReturnRef(objPreconditionManager));

        ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objMockISession));

        pUiNotifier = new MtcUiNotifier(objCallContext);
        ON_CALL(objCallContext, GetUiNotifier).WillByDefault(ReturnRef(*pUiNotifier));

        ON_CALL(objCallContext, GetMediaManager).WillByDefault(ReturnRef(objMockMediaManager));

        pIncomingState = new IncomingState(objCallContext);
    }

    virtual void TearDown() override
    {
        delete pUiNotifier;
        delete pIncomingState;
    }
};

TEST_F(IncomingStateTest, OnMediaFailed)
{
    EXPECT_CALL(objPreconditionManager, FormPreconditionSdp).Times(0);

    EXPECT_CALL(objMtcSession, Reject(CallReasonInfo(CODE_MEDIA_INIT_FAILED))).Times(1);

    pIncomingState->OnMediaFailed(CallReasonInfo(CODE_MEDIA_INIT_FAILED));
}

TEST_F(IncomingStateTest, SendUpdateBySrvccByCanceled)
{
    ON_CALL(objMockMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_CANCEL)).Times(1);

    EXPECT_EQ(CallStateName::INCOMING, pIncomingState->OnSrvccStateUpdated(SrvccState::CANCELED));
}

TEST_F(IncomingStateTest, SendUpdateBySrvccByFailed)
{
    ON_CALL(objMockMediaManager, GetNegotiationState(_))
            .WillByDefault(Return(NegotiationState::STATE_NEGOTIATED));
    EXPECT_CALL(objMtcSession, SendEarlyUpdate(UpdateType::SRVCC_RECOVERED_FAILURE)).Times(1);

    EXPECT_EQ(CallStateName::INCOMING, pIncomingState->OnSrvccStateUpdated(SrvccState::FAILED));
}

}  // namespace android
