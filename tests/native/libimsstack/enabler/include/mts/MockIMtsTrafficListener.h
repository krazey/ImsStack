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

#ifndef MOCK_I_MTS_TRAFFIC_LISTENER_H_
#define MOCK_I_MTS_TRAFFIC_LISTENER_H_

#include <gmock/gmock.h>
#include "IMtsTrafficListener.h"
#include "ImsTypeDef.h"

class MockIMtsTrafficListener : public IMtsTrafficListener
{
public:
    MOCK_METHOD(void, Traffic_OnConnectionFailed,
            (IN IMS_UINT32 nType, IN IMS_UINT32 nDirection, IN IMS_UINT32 nFailureReason,
                    IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis),
            (override));
    MOCK_METHOD(void, Traffic_OnConnectionSetupPrepared,
            (IN IMS_UINT32 nType, IN IMS_UINT32 nDirection), (override));
    MOCK_METHOD(void, Traffic_GuardTimerExpired, (IN IMS_UINT32 nType, IN IMS_UINT32 nDirection),
            (override));
};

#endif
