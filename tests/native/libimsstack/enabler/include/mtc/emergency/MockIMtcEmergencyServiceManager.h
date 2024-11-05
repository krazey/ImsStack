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

#ifndef MOCK_INTERFACE_MTC_EMERGENCY_SERVICE_MANAGER_
#define MOCK_INTERFACE_MTC_EMERGENCY_SERVICE_MANAGER_

#include "ImsTypeDef.h"
#include "emergency/IMtcEmergencyServiceManager.h"
#include <gmock/gmock.h>

class MockIMtcEmergencyServiceManager : public IMtcEmergencyServiceManager
{
public:
    MOCK_METHOD(void, StartOpen, (IN ServiceType), (override));
    MOCK_METHOD(void, StopOpen, (IN IMS_BOOL), (override));
};

#endif
