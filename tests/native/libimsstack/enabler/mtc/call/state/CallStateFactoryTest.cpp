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

#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/state/CallStateFactory.h"
#include "helper/MockICallStateProxy.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::ReturnRef;

class CallStateFactoryTest : public ::testing::Test
{
public:
    MockIMtcCallContext objMockCallContext;
    MockICallStateProxy objCallStateProxy;
    CallStateFactory* pStateFactory;
    CallInfo objCallInfo;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockCallContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objMockCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
        pStateFactory = new CallStateFactory();
    }

    virtual void TearDown() override { delete pStateFactory; }
};

TEST_F(CallStateFactoryTest, CreateStateReturnsNotNull)
{
    IMtcCallState* pState = nullptr;

    pState = pStateFactory->CreateState(CallStateName::IDLE, objMockCallContext);
    EXPECT_NE(nullptr, pState);
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::OUTGOING, objMockCallContext);
    EXPECT_NE(nullptr, pState);
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::INCOMING, objMockCallContext);
    EXPECT_NE(nullptr, pState);
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::ALERTING, objMockCallContext);
    EXPECT_NE(nullptr, pState);
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::ESTABLISHED, objMockCallContext);
    EXPECT_NE(nullptr, pState);
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::UPDATING, objMockCallContext);
    EXPECT_NE(nullptr, pState);
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::TERMINATING, objMockCallContext);
    EXPECT_NE(nullptr, pState);
    delete pState;
}

TEST_F(CallStateFactoryTest, CreateStateReturnsCorrespondingState)
{
    IMtcCallState* pState = nullptr;

    pState = pStateFactory->CreateState(CallStateName::IDLE, objMockCallContext);
    EXPECT_EQ(CallStateName::IDLE, pState->GetStateName());
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::OUTGOING, objMockCallContext);
    EXPECT_EQ(CallStateName::OUTGOING, pState->GetStateName());
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::INCOMING, objMockCallContext);
    EXPECT_EQ(CallStateName::INCOMING, pState->GetStateName());
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::ALERTING, objMockCallContext);
    EXPECT_EQ(CallStateName::ALERTING, pState->GetStateName());
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::ESTABLISHED, objMockCallContext);
    EXPECT_EQ(CallStateName::ESTABLISHED, pState->GetStateName());
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::UPDATING, objMockCallContext);
    EXPECT_EQ(CallStateName::UPDATING, pState->GetStateName());
    delete pState;

    pState = pStateFactory->CreateState(CallStateName::TERMINATING, objMockCallContext);
    EXPECT_EQ(CallStateName::TERMINATING, pState->GetStateName());
    delete pState;
}
