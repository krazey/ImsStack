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

#ifndef MOCK_ECT_CONTROLLER_H_
#define MOCK_ECT_CONTROLLER_H_

#include "AString.h"
#include "IMtcContext.h"
#include "ITimer.h"
#include "call/IMtcCall.h"
#include "ect/EctController.h"
#include "ect/EctFactory.h"
#include <gmock/gmock.h>

class MockEctController : public EctController
{
public:
    explicit MockEctController(IN IMtcContext& objContext, IN CallKey nCallKey,
            IN IEctControllerListener& objListener, IN EctFactory& objFactory) :
            EctController(objContext, nCallKey, objListener, objFactory)
    {
    }
    ~MockEctController() {}

    MOCK_METHOD(void, OnReferenceStarted, (), (override));
    MOCK_METHOD(void, OnReferenceStartFailed, (), (override));
    MOCK_METHOD(void, OnReferenceUpdated, (IN IMS_SINT32), (override));
    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer*), (override));
    MOCK_METHOD(void, Transfer, (IN const AString&), (override));
    MOCK_METHOD(void, Transfer, (), (override));
};

#endif
