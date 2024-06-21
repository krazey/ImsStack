/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TEST_SERVICE_CONTEXT_H_
#define TEST_SERVICE_CONTEXT_H_

#include "IServiceContext.h"

#include "MockIConfiguration.h"
#include "MockIServiceManager.h"

class TestServiceContext : public IServiceContext
{
public:
    TestServiceContext();
    ~TestServiceContext() override;

    inline IConfiguration* GetConfiguration() const override
    {
        return const_cast<MockIConfiguration*>(&m_objConfiguration);
    }
    inline IServiceManager* GetServiceManager() override { return &m_objServiceManager; }

    inline MockIConfiguration& GetMockConfiguration() { return m_objConfiguration; }
    inline MockIServiceManager& GetMockServiceManager() { return m_objServiceManager; }

private:
    MockIConfiguration m_objConfiguration;
    MockIServiceManager m_objServiceManager;
};

#endif
