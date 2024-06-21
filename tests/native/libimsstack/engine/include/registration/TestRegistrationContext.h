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
#ifndef TEST_REGISTRATION_CONTEXT_H_
#define TEST_REGISTRATION_CONTEXT_H_

#include "IRegistrationContext.h"

#include "MockIRegInfoManager.h"
#include "MockIRegistrationManager.h"

class TestRegistrationContext : public IRegistrationContext
{
public:
    TestRegistrationContext();
    ~TestRegistrationContext() override;

    inline IRegistrationManager* GetRegistrationManager() override
    {
        return &m_objRegistrationManager;
    }
    inline IRegInfoManager* GetRegInfoManager() override { return &m_objRegInfoManager; };

    inline MockIRegistrationManager& GetMockRegistrationManager()
    {
        return m_objRegistrationManager;
    }
    inline MockIRegInfoManager& GetMockRegInfoManager() { return m_objRegInfoManager; }

private:
    MockIRegistrationManager m_objRegistrationManager;
    MockIRegInfoManager m_objRegInfoManager;
};

#endif
