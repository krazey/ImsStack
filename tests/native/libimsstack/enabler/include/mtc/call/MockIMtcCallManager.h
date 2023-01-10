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

#ifndef MOCK_I_MTC_CALL_MANAGER_H_
#define MOCK_I_MTC_CALL_MANAGER_H_

#include "IMtcService.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallManager.h"
#include <gmock/gmock.h>

class MockIMtcCallManager : public IMtcCallManager
{
public:
    ~MockIMtcCallManager() {}

    MOCK_METHOD(IMtcCall*, CreateCall, (IN ServiceType eServiceType, IN CallInfo& objCallInfo),
            (override));
    MOCK_METHOD(void, RemoveCall, (IN CallKey nCallKey), (override));
    MOCK_METHOD(IMtcCall*, GetCallByCallKey, (IN CallKey nCallKey), (override));
    MOCK_METHOD(ImsList<IMtcCall*>, GetCalls, (), (override));
    MOCK_METHOD(ImsList<IMtcCall*>, GetCallsExcluding, (IN CallKey nExcludingCallKey), (override));
    MOCK_METHOD(ImsList<IMtcCall*>, GetCallsByType, (IN CallType eCallType), (override));
    MOCK_METHOD(
            ImsList<IMtcCall*>, GetCallsByServiceType, (IN ServiceType eServiceType), (override));
    MOCK_METHOD(ImsList<IMtcCall*>, GetCallsInConference, (), (override));
    MOCK_METHOD(ImsList<IMtcCall*>, GetCallsByState, (IN State eState), (override));
};

#endif
