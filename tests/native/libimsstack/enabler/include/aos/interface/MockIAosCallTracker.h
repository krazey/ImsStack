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

#ifndef MOCK_I_AOS_CALL_TRACKER_H_
#define MOCK_I_AOS_CALL_TRACKER_H_

#include <gmock/gmock.h>

#include "interface/IAosCallTracker.h"

class MockIAosCallTracker : public IAosCallTracker {
public:
    MOCK_METHOD(IMS_BOOL, SetMtcReady, (), (override));
    MOCK_METHOD(IMS_BOOL, IsCsCallActive, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsNormalCallActive, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyCallActive, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoCallingActive, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(CallState, GetCallState, (IN IMS_UINT32 nType), (const, override));
    MOCK_METHOD(void, SetCsCallStateWatchMode, (), (override));
    MOCK_METHOD(void, SetActiveCsCallState, (IN CallState eActiveCsState), (override));
    MOCK_METHOD(void, SetListener, (IN IAosCallTrackerListener* piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosCallTrackerListener* piListener), (override));
};

#endif //MOCK_I_AOS_CALL_TRACKER_H_
