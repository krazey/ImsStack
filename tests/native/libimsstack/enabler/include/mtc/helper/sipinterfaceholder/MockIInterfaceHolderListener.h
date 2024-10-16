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

#ifndef MOCK_I_INTERFACE_HOLDER_LISTENER_H_
#define MOCK_I_INTERFACE_HOLDER_LISTENER_H_

#include "call/IMtcCall.h"
#include "helper/sipinterfaceholder/IInterfaceHolderListener.h"
#include <gmock/gmock.h>

class MockIInterfaceHolderListener : public IInterfaceHolderListener
{
public:
    ~MockIInterfaceHolderListener() {}
    MOCK_METHOD(void, OnSessionInterfaceReleased, (IN CallKey), (override));
    MOCK_METHOD(void, OnReferenceInterfaceCleared, (), (override));
    MOCK_METHOD(void, OnSubscriptionInterfaceCleared, (), (override));
};

#endif
