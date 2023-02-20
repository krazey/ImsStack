/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef MOCK_I_IMS_RADIO_LISTENER_H_
#define MOCK_I_IMS_RADIO_LISTENER_H_

#include <gmock/gmock.h>

#include "IImsRadio.h"

class MockIImsRadioConnectionListener : public IImsRadioConnectionListener
{
public:
    inline MockIImsRadioConnectionListener() = default;
    inline ~MockIImsRadioConnectionListener() = default;

    MOCK_METHOD(void, ImsRadio_OnConnectionFailed,
            (IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis),
            (override));
    MOCK_METHOD(void, ImsRadio_OnConnectionSetupPrepared, (), (override));
};

class MockIImsRadioSsacListener : public IImsRadioSsacListener
{
public:
    inline MockIImsRadioSsacListener() = default;
    inline ~MockIImsRadioSsacListener() = default;

    MOCK_METHOD(void, ImsRadio_OnSsacChanged, (IN const SsacInfo& objSsacInfo), (override));
};

class MockIImsRadioTrafficPriorityListener : public IImsRadioTrafficPriorityListener
{
public:
    inline MockIImsRadioTrafficPriorityListener() = default;
    inline ~MockIImsRadioTrafficPriorityListener() = default;

    MOCK_METHOD(void, ImsRadio_OnTrafficPriorityChanged, (), (override));
};

#endif