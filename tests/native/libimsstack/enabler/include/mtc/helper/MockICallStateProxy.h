/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_I_CALL_STATE_PROXY_H_
#define MOCK_I_CALL_STATE_PROXY_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "helper/ICallStateProxy.h"
#include <gmock/gmock.h>

class IMtcCallStateListener;

class MockICallStateProxy : public ICallStateProxy
{
public:
    MOCK_METHOD(void, AddListener, (IN IMtcCallStateListener* pListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IMtcCallStateListener* pListener), (override));
    MOCK_METHOD(void, UpdateCallState,
            (IN CallKey nCallkey, IN IMtcCall::State eState, IN CallType eCallType,
                    IN IMS_BOOL bEmergency, IN IMS_SINT32 nReason),
            (override));
    MOCK_METHOD(void, NotifyCallSessionReleased,
            (IN CallKey nCallkey, IN IMS_BOOL bEmergency, IN IMS_BOOL bEstablished), (override));
};

#endif
