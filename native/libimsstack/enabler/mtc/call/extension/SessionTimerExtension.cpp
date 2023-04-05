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
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/SessionTimerExtension.h"

PUBLIC
SessionTimerExtension::SessionTimerExtension(IN IMtcCallContext& objContext) :
        MtcExtension(objContext, MtcExtensionSet::OPTION_TAG_SESSION_TIMER)
{
}

PUBLIC
SessionTimerExtension::SessionTimerExtension(IN const SessionTimerExtension& objRhs) :
        MtcExtension(objRhs)
{
}

PUBLIC VIRTUAL SessionTimerExtension::~SessionTimerExtension() {}

PUBLIC VIRTUAL IMtcExtension* SessionTimerExtension::Clone() const
{
    return new SessionTimerExtension(*this);
}
