/*
 * Copyright (C) 2025 The Android Open Source Project
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
#ifndef MOCK_I_REFRESHABLE_H_
#define MOCK_I_REFRESHABLE_H_

#include <gmock/gmock.h>

#include "util/IRefreshable.h"

class MockIRefreshable : public IRefreshable
{
public:
    MockIRefreshable() = default;
    ~MockIRefreshable() override = default;

    MOCK_METHOD(void, Refreshable_RefreshCompleted,
            (IN ISipClientConnection * piScc, IN IMS_SINT32 nCode), (override));
    MOCK_METHOD(IMS_BOOL, Refreshable_RefreshStarted, (), (override));
    MOCK_METHOD(void, Refreshable_RefreshTerminated, (), (override));
};

#endif
