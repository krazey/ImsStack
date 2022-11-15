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
#ifndef MOCK_I_IMS_RADIO_H_
#define MOCK_I_IMS_RADIO_H_

#include <gmock/gmock.h>

#include "IImsRadio.h"
#include "ImsTypeDef.h"

class IImsRadioTrafficPriorityListener;

class MockIImsRadio : public IImsRadio
{
public:
    inline MockIImsRadio() = default;
    inline ~MockIImsRadio() = default;

    MOCK_METHOD(IMS_BOOL, IsImsTrafficAllowed, (IN IMS_UINT32), (override));
    MOCK_METHOD(void, StartImsTraffic,
            (IN IMS_UINT32, IN IMS_UINT32, IN IMS_UINT32, IN IImsRadioConnectionListener*),
            (override));
    MOCK_METHOD(void, StopImsTraffic, (IN IImsRadioConnectionListener*), (override));
    MOCK_METHOD(void, TriggerEpsFallback, (IN IMS_UINT32), (override));
    MOCK_METHOD(const SsacInfo&, GetSsacInfo, (), (const, override));
    MOCK_METHOD(void, AddListenerForSsac, (IN IImsRadioSsacListener*), (override));
    MOCK_METHOD(void, RemoveListenerForSsac, (IN IImsRadioSsacListener*), (override));
    MOCK_METHOD(void, AddListenerForTrafficPriority, (IN IImsRadioTrafficPriorityListener*),
            (override));
    MOCK_METHOD(void, RemoveListenerForTrafficPriority, (IN IImsRadioTrafficPriorityListener*),
            (override));
};

#endif
