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
#ifndef MOCK_I_THREAD_H_
#define MOCK_I_THREAD_H_

#include <gmock/gmock.h>

#include "IThread.h"

class MockIThread : public IThread
{
public:
    inline MockIThread() {}
    inline virtual ~MockIThread() {}

    MOCK_METHOD(IMS_BOOL, Activate, (), (override));
    MOCK_METHOD(void, Deactivate, (), (override));
    MOCK_METHOD(IMS_BOOL, Equals, (IN const IThread* piThread), (const, override));
    MOCK_METHOD(const AString&, GetName, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRunning, (), (const, override));
    MOCK_METHOD(IMS_BOOL, PostMessageI, (IN ImsMessage & objMsg), (override));
    MOCK_METHOD(IMS_BOOL, PostMessageI,
            (IN IMS_UINT32 nMsg, IN IMS_UINTP nWparam, IN IMS_UINTP nLparam), (override));
    MOCK_METHOD(void, SetRunnable, (IN IRunnable * piRunnable), (override));
};

#endif
