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

#ifndef MOCK_MTC_EMERGENCY_SERVICE_MANAGER_
#define MOCK_MTC_EMERGENCY_SERVICE_MANAGER_

#include "ImsTypeDef.h"
#include "MtcEmergencyServiceManager.h"
#include "helper/IMtcAosStateListener.h"
#include <gmock/gmock.h>

class IMtcContext;
class IMtcService;

class MockMtcEmergencyServiceManager : public MtcEmergencyServiceManager
{
public:
    explicit MockMtcEmergencyServiceManager(IN IMtcContext& objContext) :
            MtcEmergencyServiceManager(objContext)
    {
    }

    MOCK_METHOD(void, OpenEmergencyService, (), (override));
    MOCK_METHOD(
            void, OnAosStateChanged, (IN IMtcService&, IN MtcAosState, IN IMS_UINT32), (override));
    MOCK_METHOD(void, OnIpcanChanged, (IN IMtcService&, IN IMS_UINT32), (override));
};

#endif
