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

#ifndef MOCK_MTC_CALL_STATE_MACHINE_H_
#define MOCK_MTC_CALL_STATE_MACHINE_H_

#include "call/IMtcCall.h"
#include "call/state/MtcCallStateMachine.h"
#include <gmock/gmock.h>

class IMtcCallContext;
class IMtcCallState;

class MockIMtcCallStateFactory : public IMtcCallStateFactory
{
public:
    MOCK_METHOD(IMtcCallState*, CreateState,
            (IN IMtcCall::State eState, IN IMtcCallContext& objContext), (override));
};

class MockIMtcCallStateWatcher : public IMtcCallStateWatcher
{
public:
    MOCK_METHOD(void, OnStateTransition, (IN IMtcCall::State eState), (override));
};

#endif
