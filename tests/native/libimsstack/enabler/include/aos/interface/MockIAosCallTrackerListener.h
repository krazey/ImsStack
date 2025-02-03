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

#ifndef MOCK_I_AOS_CALL_TRACKER_LISTENER_H_
#define MOCK_I_AOS_CALL_TRACKER_LISTENER_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosCallTrackerListener.h"

class MockIAosCallTrackerListener : public IAosCallTrackerListener
{
public:
    MOCK_METHOD(
            void, CallTracker_StateChanged, (IN IMS_UINT32 nType, IN CallState eState), (override));
    MOCK_METHOD(void, CallTracker_ECallSessionReleased, (IN IMS_BOOL bEstablished), (override));
};

#endif  // MOCK_I_AOS_CALL_TRACKER_LISTENER_H_
