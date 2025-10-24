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

#ifndef CALL_STATE_FACTORY_H_
#define CALL_STATE_FACTORY_H_

#include "ImsTypeDef.h"
#include "call/state/IMtcCallState.h"
#include "call/state/MtcCallStateMachine.h"

class IMtcCallContext;

class CallStateFactory final : public IMtcCallStateFactory
{
public:
    explicit CallStateFactory();
    virtual ~CallStateFactory() override;
    CallStateFactory(IN const CallStateFactory&) = delete;
    CallStateFactory& operator=(IN const CallStateFactory&) = delete;

    IMtcCallState* CreateState(IN CallStateName eState, IN IMtcCallContext& objContext) override;
};

#endif
