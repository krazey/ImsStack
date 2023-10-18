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
#ifndef TEST_SYSTEM_TIME_SERVICE_H_
#define TEST_SYSTEM_TIME_SERVICE_H_

#include "MockISystemTime.h"
#include "ServiceSystemTime.h"

class TestSystemTimeService : public SystemTimeService
{
public:
    inline TestSystemTimeService() :
            SystemTimeService(),
            m_piSystemTime(&m_objSystemTime)
    {
    }

    inline ISystemTime* GetSystemTime() override { return m_piSystemTime; }

    inline MockISystemTime& GetMockSystemTime() { return m_objSystemTime; }
    inline void SetSystemTime(IN ISystemTime* piSystemTime) { m_piSystemTime = piSystemTime; }

private:
    MockISystemTime m_objSystemTime;

    ISystemTime* m_piSystemTime;
};

#endif
