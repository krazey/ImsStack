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

#ifndef INTERFACE_CALL_STATE_PROXY_H_
#define INTERFACE_CALL_STATE_PROXY_H_

#include "CallReasonInfo.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class IMtcCallStateListener;

class ICallStateProxy
{
public:
    virtual ~ICallStateProxy() {}

public:
    virtual void AddListener(IN IMtcCallStateListener* pListener) = 0;
    virtual void RemoveListener(IN IMtcCallStateListener* pListener) = 0;

    virtual void UpdateCallState(IN CallKey nCallkey, IN IMtcCall::State eState,
            IN CallType eCallType, IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason = CODE_NONE) = 0;
};

#endif
