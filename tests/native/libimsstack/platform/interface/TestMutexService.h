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
#ifndef TEST_MUTEX_SERVICE_H_
#define TEST_MUTEX_SERVICE_H_

#include "MockIMutex.h"
#include "ServiceMutex.h"

class TestMutexService : public MutexService
{
public:
    inline IMutex* CreateMutex(IN const AString& /*strName*/) override { return &m_objMutex; }
    inline void DestroyMutex(IN IMutex*& /*piMutex*/) override {}

    inline MockIMutex& GetMockMutex() { return m_objMutex; }

private:
    MockIMutex m_objMutex;
};

#endif
