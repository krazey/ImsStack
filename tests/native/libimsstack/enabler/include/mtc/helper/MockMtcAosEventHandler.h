/**
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

#ifndef MOCK_MTC_AOS_EVENT_HANDLER_H_
#define MOCK_MTC_AOS_EVENT_HANDLER_H_

#include "helper/MtcAosEventHandler.h"
#include <gmock/gmock.h>

class IMtcAosStateListener;
class IMtcCallController;
class IMtcRadioChecker;
class IMtcService;
class MtcConfigurationProxy;

class MockMtcAosEventHandler : public MtcAosEventHandler
{
public:
    explicit MockMtcAosEventHandler(
            IN IMtcService& objService, IN MtcConfigurationProxy& objConfiguration) :
            MtcAosEventHandler(objService, objConfiguration)
    {
    }
    ~MockMtcAosEventHandler() {}

    MOCK_METHOD(void, AddListener, (IN IMtcAosStateListener*), (override));
    MOCK_METHOD(void, RemoveListener, (IN IMtcAosStateListener*), (override));
    MOCK_METHOD(void, OnConnected, (IN IMS_UINT32), (override));
    MOCK_METHOD(void, OnDisconnecting, (IN IMS_UINT32), (override));
    MOCK_METHOD(void, OnDisconnected, (IN IMS_UINT32), (override));
    MOCK_METHOD(void, OnSuspended, (IN IMS_UINT32), (override));
    MOCK_METHOD(void, OnResumed, (), (override));
    MOCK_METHOD(void, OnServiceConnected, (IN IMS_UINT32, IN IMS_UINT32), (override));
    MOCK_METHOD(void, OnEventNotify, (IN IMS_UINT32, IN IMS_UINT32), (override));
};

#endif
