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

#include "call/IMtcCallContext.h"
#include "call/MtcUiNotifier.h"
#include "call/state/TerminatingState.h"
#include "helper/MtcTimerWrapper.h"
#include "define/MtcStringDef.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
TerminatingState::TerminatingState(IN IMtcCallContext& objContext) :
        MtcCallState(CallStateName::TERMINATING, objContext)
{
}

PUBLIC VIRTUAL TerminatingState::~TerminatingState() {}
