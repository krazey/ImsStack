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

#include "call/state/AlertingState.h"
#include "call/state/CallStateFactory.h"
#include "call/state/EstablishedState.h"
#include "call/state/IdleState.h"
#include "call/state/IncomingState.h"
#include "call/state/OutgoingState.h"
#include "call/state/TerminatingState.h"
#include "call/state/UpdatingState.h"

PUBLIC CallStateFactory::CallStateFactory() {}

PUBLIC VIRTUAL CallStateFactory::~CallStateFactory() {}

PUBLIC VIRTUAL IMtcCallState* CallStateFactory::CreateState(
        IN CallStateName eState, IN IMtcCallContext& objContext)
{
    switch (eState)
    {
        case CallStateName::IDLE:
            return new IdleState(objContext);
        case CallStateName::OUTGOING:
            return new OutgoingState(objContext);
        case CallStateName::INCOMING:
            return new IncomingState(objContext);
        case CallStateName::ALERTING:
            return new AlertingState(objContext);
        case CallStateName::ESTABLISHED:
            return new EstablishedState(objContext);
        case CallStateName::UPDATING:
            return new UpdatingState(objContext);
        default:  // CallStateName::TERMINATING
            return new TerminatingState(objContext);
    }
}
