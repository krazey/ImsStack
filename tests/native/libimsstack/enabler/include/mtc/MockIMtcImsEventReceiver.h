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

#ifndef MOCK_I_MTC_IMS_EVENT_RECEIVER_
#define MOCK_I_MTC_IMS_EVENT_RECEIVER_

#include "IMtcImsEventReceiver.h"
#include "ImsTypeDef.h"
#include <gmock/gmock.h>

class IMtcImsEventListener;

class MockIMtcImsEventReceiver : public IMtcImsEventReceiver
{
public:
    MOCK_METHOD(IMS_UINT32, GetWParam, (IN ImsEvent nEvent), (override));
    MOCK_METHOD(IMS_UINT32, GetLParam, (IN ImsEvent nEvent), (override));
    MOCK_METHOD(void, AddListener, (IN IMtcImsEventListener* pListener, IN ImsEvent nEvent),
            (override));
    MOCK_METHOD(void, RemoveListener, (IN IMtcImsEventListener* pListener, IN ImsEvent nEvent),
            (override));
};

class MockIMtcImsEventListener : public IMtcImsEventListener
{
public:
    MOCK_METHOD(void, OnImsEventNotified,
            (IN ImsEvent nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam), (override));
};

#endif
