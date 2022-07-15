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

#ifndef MOCK_I_IMS_AOS_H_
#define MOCK_I_IMS_AOS_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"

#include "IImsAos.h"
#include "ImsAosParameter.h"

class IImsAosListener;
class IImsAosMonitor;

class MockIImsAos : public IImsAos
{
public:
    MOCK_METHOD(IMS_BOOL, Control, (IN IMS_UINT32), (override));
    MOCK_METHOD(IImsAosInfo*, GetAosInfo, (), (override));
    MOCK_METHOD(IMS_UINT32, GetFeatures, (), (override));
    MOCK_METHOD(IMS_UINT32, GetSuspendedReason, (), (override));
    MOCK_METHOD(IMS_BOOL, IsFeatureConnected, (IN IMS_UINT32), (override));
    MOCK_METHOD(IMS_BOOL, IsImsConnected, (), (override));
    MOCK_METHOD(IMS_BOOL, IsImsSuspended, (), (override));
    MOCK_METHOD(void, SetListener, (IN IImsAosListener*), (override));
    MOCK_METHOD(void, SetMonitor, (IN IImsAosMonitor*), (override));
    MOCK_METHOD(IMS_BOOL, SetReady, (IN IMS_BOOL, IN IMS_UINT32), (override));
    MOCK_METHOD(void, UpdateFeature, (IN IMS_UINT32), (override));
    MOCK_METHOD(void, UpdateFeature, (IN IMSList<ImsAosFeatureTag*>&), (override));
};

#endif  // MOCK_I_IMS_AOS_H_
