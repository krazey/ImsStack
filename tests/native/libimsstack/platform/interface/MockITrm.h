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

#ifndef MOCK_I_TRM_H_
#define MOCK_I_TRM_H_

#include <gmock/gmock.h>

#include "IMSList.h"
#include "ImsMessageDef.h"
#include "ServiceMessage.h"
#include "ServiceThread.h"
#include "ITrm.h"

class MockITrm : public ITrm
{
public:
    MOCK_METHOD(void, Enable, (IN IMS_UINT32 nSlotId), (override));
    MOCK_METHOD(void, Disable, (IN IMS_UINT32 nSlotId), (override));
    MOCK_METHOD(
            IMS_BOOL, IsServiceAvailable, (IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType), (override));
    MOCK_METHOD(IMS_BOOL, IsTrmSupported, (), (override));
    MOCK_METHOD(void, SetEmergencyService,
            (IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType, IN IMS_UINT32 nMode), (override));
    MOCK_METHOD(void, SetIpcan, (IN IMS_UINT32 nSlotId, IN IMS_UINT32 nCategory), (override));
    MOCK_METHOD(IMS_BOOL, SetService,
            (IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType, IN IMS_UINT32 nMode), (override));
};

#endif  // MOCK_I_TRM_H_
