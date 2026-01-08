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

#include "call/MockIMtcCallContext.h"
#include "call/state/MockIMtcCallState.h"
#include "call/state/MockMtcCallStateMachine.h"
#include "call/state/MtcCallStateMachine.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using ::testing::Return;
using State = IMtcCall::State;

class TestCallStateFactory : public IMtcCallStateFactory
{
public:
    IMtcCallState* CreateState(
            IN IMtcCall::State eState, IN IMtcCallContext& /* objContext */) override
    {
        MockIMtcCallState* pState = new MockIMtcCallState();

        ON_CALL(*pState, GetStateName).WillByDefault(Return(eState));
        EXPECT_CALL(*pState, OnEnter).Times(1);

        return pState;
    }
};

class MtcCallStateMachineTest : public ::testing::Test
{
public:
    // cppcheck-suppress unusedStructMember
    MockIMtcCallContext objContext;
    std::unique_ptr<IMtcCallStateFactory> pStateFactory;

protected:
    virtual void SetUp() override { pStateFactory = std::make_unique<TestCallStateFactory>(); }

    virtual void TearDown() override {}
};

TEST_F(MtcCallStateMachineTest, GetStateReturnsInitialStateName)
{
    const State eInitialState = State::TERMINATING;
    MtcCallStateMachine objStateMachine(objContext, eInitialState, std::move(pStateFactory));

    EXPECT_EQ(eInitialState, objStateMachine.GetState());
}

TEST_F(MtcCallStateMachineTest, GetStateReturnsChangedStateName)
{
    const State eInitialState = State::TERMINATING;
    const State eChangedState = State::ESTABLISHED;

    MtcCallStateMachine objStateMachine(objContext, eInitialState, std::move(pStateFactory));
    objStateMachine.RunStateOperation(
            [](IMtcCallState* /* pState */)
            {
                return eChangedState;
            });

    EXPECT_EQ(eChangedState, objStateMachine.GetState());
}

TEST_F(MtcCallStateMachineTest, RunStateOperationRunsOnInitialState)
{
    const State eInitialState = State::TERMINATING;

    MtcCallStateMachine objStateMachine(objContext, eInitialState, std::move(pStateFactory));
    objStateMachine.RunStateOperation(
            [&](const IMtcCallState* pState)
            {
                EXPECT_EQ(eInitialState, pState->GetStateName());
                return eInitialState;
            });
}

TEST_F(MtcCallStateMachineTest, RunStateOperationRunsOnChangedState)
{
    const State eInitialState = State::TERMINATING;
    const State eChangedState = State::ESTABLISHED;

    MtcCallStateMachine objStateMachine(objContext, eInitialState, std::move(pStateFactory));
    objStateMachine.RunStateOperation(
            [&](IMtcCallState* /* pState */)
            {
                return eChangedState;
            });

    objStateMachine.RunStateOperation(
            [&](const IMtcCallState* pState)
            {
                EXPECT_EQ(eChangedState, pState->GetStateName());
                return eChangedState;
            });
}

TEST_F(MtcCallStateMachineTest, NotifiesWatcherInitially)
{
    const State eInitialState = State::TERMINATING;

    MockIMtcCallStateWatcher objWatcher;
    EXPECT_CALL(objWatcher, OnStateTransition(eInitialState)).Times(1);

    MtcCallStateMachine objStateMachine(
            objContext, eInitialState, std::move(pStateFactory), &objWatcher);
}

TEST_F(MtcCallStateMachineTest, NotNotifiesWatcherWhenTransitionToSameState)
{
    const State eInitialState = State::TERMINATING;

    MockIMtcCallStateWatcher objWatcher;
    EXPECT_CALL(objWatcher, OnStateTransition(eInitialState)).Times(1);

    MtcCallStateMachine objStateMachine(
            objContext, eInitialState, std::move(pStateFactory), &objWatcher);
    objStateMachine.RunStateOperation(
            [&](IMtcCallState* /* pState */)
            {
                return eInitialState;
            });
}

TEST_F(MtcCallStateMachineTest, NotifiesWatcherWhenTransitionToAnotherState)
{
    const State eInitialState = State::TERMINATING;
    const State eChangedState = State::ESTABLISHED;

    MockIMtcCallStateWatcher objWatcher;
    EXPECT_CALL(objWatcher, OnStateTransition(eInitialState)).Times(1);
    EXPECT_CALL(objWatcher, OnStateTransition(eChangedState)).Times(1);

    MtcCallStateMachine objStateMachine(
            objContext, eInitialState, std::move(pStateFactory), &objWatcher);
    objStateMachine.RunStateOperation(
            [&](IMtcCallState* /* pState */)
            {
                return eChangedState;
            });
}
