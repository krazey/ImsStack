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
#include "TestEngine.h"
#include "TestRegistrationContext.h"
#include "TestServiceContext.h"

class TestEnginePrivate
{
public:
    TestEnginePrivate() = default;
    ~TestEnginePrivate() = default;

public:
    inline TestServiceContext& GetServiceContext() { return m_objServiceContext; }
    inline TestRegistrationContext& GetRegistrationContext() { return m_objRegistrationContext; }

private:
    TestServiceContext m_objServiceContext;
    TestRegistrationContext m_objRegistrationContext;
};

TestEngine::TestEngine() :
        m_pPrivate(new TestEnginePrivate())
{
}

TestEngine::~TestEngine()
{
    delete m_pPrivate;
}

MockIConfiguration& TestEngine::GetMockConfiguration()
{
    return m_pPrivate->GetServiceContext().GetMockConfiguration();
}

MockIRegistrationManager& TestEngine::GetMockRegistrationManager()
{
    return m_pPrivate->GetRegistrationContext().GetMockRegistrationManager();
}
