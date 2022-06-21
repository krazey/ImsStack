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

#ifndef INTERFACE_MTC_CALL_STATE_LISTENER_H_
#define INTERFACE_MTC_CALL_STATE_LISTENER_H_

#include "IMtcService.h"
#include "call/IMtcCall.h"

class IMtcCallStateListener
{
public:
    using State = IMtcCall::State;
    using Type = CallType;

    virtual void OnCallStateChanged(IN CallKey nCallKey, IN State eState, IN Type eType,
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason) = 0;
    virtual void OnTotalCallStateChanged(IN State eState) = 0;
    inline virtual IMS_BOOL IsSynchronousCallRequired() { return IMS_FALSE; }
};

#endif
